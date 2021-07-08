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
* @file prvCpssDxChNetIfDbg.h
*
* @brief Header file for debug utilities for network interface.
*
*
* @version   2
********************************************************************************
*/
#ifndef __prvCpssDxChNetIfDbgh
#define __prvCpssDxChNetIfDbgh

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>

/**
* @enum CPSS_TX_SDMA_GENERATOR_MEM_CHECK_ENT
 *
 * @brief Define Tx SDMA Generator memory check result status.
*/
typedef enum{

    /** @brief memory check passed
     *  successfully.
     */
    CPSS_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E,

    /** @brief wrong value of buffer
     *  pointer in descriptor.
     */
    CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_BUFF_PTR_E,

    /** @brief wrong value of next
     *  descriptor pointed.
     */
    CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_NEXT_DESC_E,

    /** the queue memory is empty. */
    CPSS_TX_SDMA_GENERATOR_MEM_CHECK_EMPTY_QUEUE_E,

    /** @brief memory scanning exceed
     *  the expected number of queue descriptors.
     */
    CPSS_TX_SDMA_GENERATOR_MEM_CHECK_DESC_EXCEED_E

} CPSS_TX_SDMA_GENERATOR_MEM_CHECK_ENT;


/**
* @internal cpssDxChNetIfSdmaTxGeneratorMemCheck function
* @endinternal
*
* @brief   This function verify that descriptors and buffers memory is arranged
*         to support optimal performance.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @param[out] checkResultPtr           - (pointer to) memory check result status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note memory check can be performed while queue is enabled.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorMemCheck
(
    IN  GT_U8                   devNum,
    IN  GT_U8                   txQueue,
    OUT CPSS_TX_SDMA_GENERATOR_MEM_CHECK_ENT *checkResultPtr
);

#ifdef __cplusplus
}
#endif

#endif  /* __prvCpssDxChNetIfDbgh */

