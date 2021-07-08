/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChBrgFdbAu.h
*
* @brief MAC hash struct implementation.
*
* @version   8
********************************************************************************
*/
#ifndef __prvCpssDxChBrgFdbAuh
#define __prvCpssDxChBrgFdbAuh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>


/**
* @enum MESSAGE_QUEUE_ENT
 *
 * @brief hold the different types of message queues that cpss SW uses
*/
typedef enum{

    /** the primary AUQ */
    MESSAGE_QUEUE_PRIMARY_AUQ_E,

    /** the primary FUQ */
    MESSAGE_QUEUE_PRIMARY_FUQ_E,

    /** the secondary AUQ */
    MESSAGE_QUEUE_SECONDARY_AUQ_E

} MESSAGE_QUEUE_ENT;

/**
* @internal auqFuqClearUnreadMessages function
* @endinternal
*
* @brief   The function scan the AU/FU queues, clear all unread
*         AU/FU messages in the queue and calculate number of messages
*         that remained till the end of the queue.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which AU are counted
* @param[in] portGroupId              - the  - for multi-port-groups support
* @param[in] queueType                - AUQ or FUQ. FUQ valid for DxCh2 and above.
*
* @param[out] numOfAuPtr               - (pointer to) number of AU messages processed in the specified queue.
* @param[out] numMsgTillQueueEndPtr    - (pointer to) number of AU messages remained to fill the queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, queueType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on not initialized queue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In order to have the accurate number of entries application should
*       protect Mutual exclusion between HW access to the AUQ/FUQ
*       SW access to the AUQ/FUQ and calling to this API.
*       i.e. application should stop the PP from sending AU messages to CPU.
*       and should not call the api's
*       cpssDxChBrgFdbFuMsgBlockGet, cpssDxChBrgFdbFuMsgBlockGet
*
*/
GT_STATUS auqFuqClearUnreadMessages
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  MESSAGE_QUEUE_ENT             queueType,
    OUT GT_U32                       *numOfAuPtr,
    OUT GT_U32                       *numMsgTillQueueEndPtr
);

/**
* @internal auqFuqMessagesNumberGet function
* @endinternal
*
* @brief   The function scan the AU/FU queues and returns the number of
*         AU/FU messages in the queue.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which AU are counted
* @param[in] portGroupId              - the  - for multi-port-groups support
* @param[in] queueType                - AUQ or FUQ. FUQ valid for DxCh2 and above.
*
* @param[out] numOfAuPtr               - (pointer to) number of AU messages in the specified queue.
* @param[out] endOfQueueReachedPtr     - (pointer to) GT_TRUE: The queue reached to the end.
*                                      GT_FALSE: else
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, queueType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on not initialized queue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In order to have the accurate number of entries application should
*       protect Mutual exclusion between HW access to the AUQ/FUQ
*       SW access to the AUQ/FUQ and calling to this API.
*       i.e. application should stop the PP from sending AU messages to CPU.
*       and should not call the api's
*       cpssDxChBrgFdbFuMsgBlockGet, cpssDxChBrgFdbFuMsgBlockGet
*
*/
GT_STATUS auqFuqMessagesNumberGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  MESSAGE_QUEUE_ENT             queueType,
    OUT GT_U32                       *numOfAuPtr,
    OUT GT_BOOL                      *endOfQueueReachedPtr
);

/**
* @internal auDesc2UpdMsg function
* @endinternal
*
* @brief   This function is called whenever an address update message is received.
*         It translates the descriptor into CPSS_MAC_UPDATE_MSG_EXT_STC format
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - number of device whose descriptor queue is processed
* @param[in] portGroupId              - the  - for multi-port-groups support
* @param[in] auDescExtPtr             - (pointer to) the AU descriptor filled by PP.
* @param[in] auDescrFromDma           - AU descriptor is from DMA queue
*                                      GT_TRUE - AU descriptor is from DMA queue
*                                      GT_FALSE - AU descriptor is from FIFO but not DMA queue
*
* @param[out] addrUpMsgPtr             - MAC format of entry
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS auDesc2UpdMsg
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  PRV_CPSS_AU_DESC_EXT_8_STC    *auDescExtPtr,
    IN  GT_BOOL                       auDescrFromDma,
    OUT CPSS_MAC_UPDATE_MSG_EXT_STC   *addrUpMsgPtr
);

/**
* @internal prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableSet function
* @endinternal
*
* @brief   Set the field mask for 'MAC entry does NOT reside on the local port group'
*         When Enabled - AU/FU messages are not sent to the CPU if the MAC entry
*         does NOT reside on the local portgroup, i.e.
*         the entry port[5:4] != LocalPortGroup
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - mask enable/disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on bad device number.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device (device is not multi-port groups device)
*
* @note none
*
*/
GT_STATUS prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableGet function
* @endinternal
*
* @brief   Get the field mask for 'MAC entry does NOT reside on the local port group'
*         When Enabled - AU/FU messages are not sent to the CPU if the MAC entry
*         does NOT reside on the local core, i.e.
*         the entry port[5:4] != LocalPortGroup
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) mask enable/disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on bad device number.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device (device is not multi-port groups device)
*
* @note none
*
*/
GT_STATUS prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgFdbAuh */

