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
* @file prvCpssDrvErrataMng.h
*
* @brief Cpss Driver manager for the Errata issues.
*
*
* @version   3
********************************************************************************
*/

#ifndef __prvCpssDrvErrataMngh
#define __prvCpssDrvErrataMngh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/* number GT_U32 bmps needed to hold the known FEr errata bmp */
#define PRV_CPSS_DRV_ERRATA_BMP_MAX_NUM_CNS \
                (1 + (((PRV_CPSS_DRV_ERRATA_MAX_NUM_E)+1) >> 5))

/* get if the erratum need to be WA by CPSS
    devNum - the device id of the cpss device
    FErNum -- id of the erratum
*/
#define PRV_CPSS_DRV_ERRATA_GET_MAC(devNum,FErNum) \
  ((prvCpssDrvPpConfig[devNum]->errata.FErArray[(FErNum)>>5] &(1<<((FErNum)&0x1f)))? \
      GT_TRUE : GT_FALSE)

/* set that the erratum need to be WA by CPSS
    devNum - the device id of the cpss device
    FErNum -- 1 based number
*/
#define PRV_CPSS_DRV_ERRATA_SET_MAC(devNum,FErNum) \
    prvCpssDrvPpConfig[devNum]->errata.FErArray[(FErNum)>>5] |= (1<<((FErNum)&0x1f))

/**
* @enum PRV_CPSS_DRV_ERRATA_ENT
 *
 * @brief enumerator for the numbers of Erratum that handles in the CPSS
 * driver code
*/
typedef enum{

    /** @brief WA for Errata of PPs that
     *  require to SYNC their write actions before doing a
     *  read action
     *  (Twist-D FEr#1).
     */
    PRV_CPSS_DRV_ERRATA_PCI_READ_AFTER_WRITE_E,

    /** @brief WA needed
     *  for Device PCI slave that is unable to abort
     *  read/write transaction .
     *  (Twist-D FEr#68 , Tiger FEr#17 , ).
     */
    PRV_CPSS_DRV_ERRATA_PCI_SLAVE_UNABLE_TO_ABORT_TRANSACTION_E,

    /** @brief If we manage to push burst of transactions from
     *  PEX, it may cause a stuck on the SERDESes
     *  registers SM. (Lion FEr#2799)
     *  Note: although xcat/xcat2 have same SERDESes HW
     *  design confirmed that just Lion has this
     *  problem
     */
    PRV_CPSS_DRV_ERRATA_SERDES_INTERNAL_REGS_ACCESS_WA_E,

    PRV_CPSS_DRV_ERRATA_MAX_NUM_E

} PRV_CPSS_DRV_ERRATA_ENT;

/**
* @enum PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_ENT
 *
 * @brief enumerator for the descripton of the event caused to call
 * the errata callback function.
*/
typedef enum{
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_MASK_MEMORY_E    = 0x000F,
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_MASK_STAGE_E     = 0x00F0,
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_MASK_OPERATION_E = 0xFF00,
    /* HW type 0x0000000F */
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_REGISTER_E       = 0x0000,
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_MEMORY_E         = 0x0001,
    /* stage type 0x000000F0 */
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_PREPARE_E        = 0x0000,
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_COMPLETE_E       = 0x0010,
    /* operation type 0x00000F00 */
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_READ_E           = 0x0000,
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_WRITE_E          = 0x0100,
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_READ_MASK_E      = 0x0200,
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_WRITE_MASK_E     = 0x0300,
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_SET_FIELD_E      = 0x0400,
    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_GET_FIELD_E      = 0x0500
} PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_ENT;

/*******************************************************************************
* PRV_CPSS_DRV_ERRATA_ADDR_CALLBACK_FUNC
*
* DESCRIPTION:
*       Prepare and complete read/write access operations for special cases.
*
* APPLICABLE DEVICES:
*       NA
*
* INPUTS:
*       devNum        - The device number
*       portGroupId   - The port group Id. relevant only to 'multi-port-groups' devices,
*                        supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS.
*       anchorDataPtr - (pointer to) data used by callback
*                        passed with callback function address binding callback
*       addr          - Start offset address to check
*       length        - The length of the memory to check (the legth is in words).
*       param0        - additional info - field offset in bits or bitmask
*       param0        - additional info - field length in bits
*       inoutDataPtr  - write data pointers (for write operations)
*
* OUTPUTS:
*       inoutDataPtr  - read data pointers (for read operations)
*       skipHwAccessPtr - (pointer to) GT_TRUE - skip HW access, GT_FALSE - do HW access.
*                     Calls of this callback prevent and completes HW access in CPSS
*                     low level driver.
*
* RETURNS:
*       GT_OK         - on success
*       GT_HW_ERROR   - on hardware error.
*       GT_BAD_PARAM  - on bad devNum or restricted address
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DRV_ERRATA_CALLBACK_FUNC_PTR)
(
    IN    GT_U8                                  devNum,
    IN    GT_U32                                 portGroupId,
    IN    GT_VOID                                *anchorDataPtr,
    IN    PRV_CPSS_DRV_ERRATA_CALLBACK_EVENT_ENT callbackEvent,
    IN    GT_U32                                 addr,
    IN    GT_U32                                 length,
    IN    GT_U32                                 param0,
    IN    GT_U32                                 param1,
    INOUT GT_VOID                                *inoutDataPtr,
    OUT   GT_BOOL                                *skipHwAccessPtr
);

/*******************************************************************************
* PRV_CPSS_DRV_ERRATA_ADDR_CHECK_FUNC
*
* DESCRIPTION:
*       Check address range comprises restricted addresses.
*
* APPLICABLE DEVICES:
*       NA
*
* INPUTS:
*       devNum      - The device number
*       portGroupId - The port group Id. relevant only to 'multi-port-groups' devices,
*                        supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS.
*       addr        - Start offset address to check
*       length      - The length of the memory to check (the legth is in words).
*
* OUTPUTS:
*       none.

* RETURNS:
*       GT_OK         - on success
*       GT_HW_ERROR   - on hardware error.
*       GT_BAD_PARAM  - on bad devNum or restricted address
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_DRV_ERRATA_ADDR_CHECK_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 addr,
    IN GT_U32 length
);

/**
* @struct PRV_CPSS_DRV_ERRATA_STC
 *
 * @brief A structure to hold all Errata info that CPSS need to fix
 * (workaround) for the PP .
*/
typedef struct{

    GT_U32 FErArray[PRV_CPSS_DRV_ERRATA_BMP_MAX_NUM_CNS];

    /** pointer to Address Checking callback */
    PRV_CPSS_DRV_ERRATA_ADDR_CHECK_FUNC addrCheckFuncPtr;

    /** @brief pointer to prepare/complete special read/write operations function
     *  callbackDataPtr  - pointer to prepare/complete special read/write operations anchor data
     */
    PRV_CPSS_DRV_ERRATA_CALLBACK_FUNC_PTR callbackFuncPtr;

    GT_VOID *callbackDataPtr;

} PRV_CPSS_DRV_ERRATA_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDrvErrataMngh */



