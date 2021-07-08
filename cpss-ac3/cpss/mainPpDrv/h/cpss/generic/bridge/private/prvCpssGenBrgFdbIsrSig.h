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
* @file prvCpssGenBrgFdbIsrSig.h
*
* @brief Includes ISR and FDB signalling mechanism create/signal/wait/delete API
* This file provide the devices ability to get indication about 2 events:
* 1. FDB AU-message from CPU action or query action ended (NA/QA/QI)
* 2. FDB mac trigger action ended (flush/transplant/trig aging)
*
* 1. When the CPSS try to operate FDB manipulation via the "AU messages
* mechanism" of the PP, the CPSS first check that the last operation was
* ended - this is done by waiting for the event of
* CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E
* 2. When the CPSS try to operate FDB mac trigger action
* (Flush/transplant/trigger aging) in the PP , the CPSS trigger the
* action and wait for the action to end. - this is done by waiting for
* the event of CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E
*
* @version   3
********************************************************************************
*/
#ifndef __prvCpssGenBrgFdbIsrSigh
#define __prvCpssGenBrgFdbIsrSigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
/**
* @enum CPSS_FDB_ISR_SIG_ENT
 *
 * @brief Enumeration that defines the FDB signalling app type.
*/
typedef enum{

    /** @brief Query operation signal.
     *  relate to the event of CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E
     */
    CPSS_QUERY_E,

    /** @brief MAC trigger Action Signal.
     *  relate to the event of CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E
     */
    CPSS_MAC_TRIG_ACTION_E

} CPSS_FDB_ISR_SIG_ENT;


/**
* @internal prvCpssFdbIsrSigCreate function
* @endinternal
*
* @brief   This routine creates a signalling mechanism for FDB ISR.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] sigType                  - The event signal type.
*                                       None.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFdbIsrSigCreate
(
    IN CPSS_FDB_ISR_SIG_ENT       sigType
);


/**
* @internal prvCpssFdbIsrSigDelete function
* @endinternal
*
* @brief   This routine deletes a signalling mechanism for FDB ISR.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] sigType                  - The event signal type.
*                                       None.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFdbIsrSigDelete
(
    IN CPSS_FDB_ISR_SIG_ENT       sigType
);


/**
* @internal prvCpssFdbIsrSigSignal function
* @endinternal
*
* @brief   This routine signals a waiting task.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] sigType                  - The event signal type.
*                                       None.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFdbIsrSigSignal
(
    IN CPSS_FDB_ISR_SIG_ENT       sigType
);


/**
* @internal prvCpssFdbIsrSigWait function
* @endinternal
*
* @brief   This routine waits for a signal for the event to occur.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] sigType                  - The event signal type.
* @param[in] timeout                  - Time-out in milliseconds or 0 to wait forever.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFdbIsrSigWait
(
    IN CPSS_FDB_ISR_SIG_ENT sigType,
    IN GT_U32               timeout
);

/**
* @internal prvCpssFdbIsrSigHaModeSet function
* @endinternal
*
* @brief   function to set CPU High Availability mode of operation.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] mode                     - active or standby
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFdbIsrSigHaModeSet
(
    IN  GT_U8   devNum,
    IN  CPSS_SYS_HA_MODE_ENT mode
);


/**
* @internal prvCpssFdbIsrSigUnmaskDevice function
* @endinternal
*
* @brief   This routine unmasks the unified events of "mac update done" /
*         "mac trigger action done" on specific device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sigType                  - The event signal type.
*                                       None.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFdbIsrSigUnmaskDevice
(
    IN GT_U8                      devNum,
    IN CPSS_FDB_ISR_SIG_ENT       sigType
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenBrgFdbIsrSigh */

