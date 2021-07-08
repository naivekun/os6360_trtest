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
* @file cpssTmNodesCtl.h
*
* @brief TM nodes control APIs
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmNodesCtlh
#define __cpssTmNodesCtlh

#include <cpss/generic/tm/cpssTmPublicDefs.h>


/**
* @internal cpssTmNodesCtlQueueInstall function
* @endinternal
*
* @brief   Install queue by a given index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] queueInd                 - Queue index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Makes a queue capable of getting traffic.
*
*/
GT_STATUS cpssTmNodesCtlQueueInstall
(
    IN GT_U8    devNum,
    IN GT_U32   queueInd
);


/**
* @internal cpssTmNodesCtlQueueUninstall function
* @endinternal
*
* @brief   Uninstall queue by a given index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] queueInd                 - Queue index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note - Makes a queue incapable of getting traffic. Queue must be not in use!
*       - User should flush Q before this call.
*
*/
GT_STATUS cpssTmNodesCtlQueueUninstall
(
    IN GT_U8    devNum,
    IN GT_U32   queueInd
);

/**
* @internal cpssTmNodesCtlQueueInstallStatusGet function
* @endinternal
*
* @brief   Retrieves install status if the queue by a given index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] queueInd                 - Queue index (APPLICABLE RANGES: 0..16383).
*
* @param[out] statusPtr                - (Pointer to) queue install status.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_PTR               - on NULL statusPtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmNodesCtlQueueInstallStatusGet
(
    IN GT_U8    devNum,
    IN GT_U32   queueInd,
    OUT GT_U32  *statusPtr
);

/**
* @internal cpssTmNodesCtlNodeDelete function
* @endinternal
*
* @brief   Delete node from scheduling hierarchy.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Scheduling level: Queue/A/B/C-node/Port.-node/Port.
* @param[in] index                    - Queue/node index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_STATE             - on not free node's subtree.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmNodesCtlNodeDelete
(
    IN GT_U8                devNum,
    IN CPSS_TM_LEVEL_ENT    level,
    IN GT_U32               index
);


/**
* @internal cpssTmNodesCtlTransPortDelete function
* @endinternal
*
* @brief   Delete port and all its subtree from scheduling hierarchy.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmNodesCtlTransPortDelete
(
    IN GT_U8    devNum,
    IN GT_U32   index
);


/**
* @internal cpssTmQueueFlush function
* @endinternal
*
* @brief   Flush queue by a given index. All packets dequeued from this queue are dropped.
*         Wait till flush queue has completed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Queue index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Flush queue blocking API.
*
*/
GT_STATUS cpssTmQueueFlush
(
    IN GT_U8                    devNum,
    IN GT_U32                   index
);


/**
* @internal cpssTmQueueFlushTriggerActionStart function
* @endinternal
*
* @brief   Activate Flush queue by a given index.
*         All packets dequeued from this queue are dropped.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Queue index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Flush queue non blocking API.
*
*/
GT_STATUS cpssTmQueueFlushTriggerActionStart
(
    IN GT_U8                    devNum,
    IN GT_U32                   index
);


/**
* @internal cpssTmQueueFlushTriggerActionStatusGet function
* @endinternal
*
* @brief   Get whether Flush queue has completed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Queue index.
*
* @param[out] actFinishedPtr           - GT_FALSE - flush is active (packets are dropped on dequeue).
*                                      - GT_TRUE  - flush off /completed.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_BAD_PTR               - on NULL actFinishedPtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmQueueFlushTriggerActionStatusGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   index,
    OUT GT_BOOL                 *actFinishedPtr
);


/**
* @internal cpssTmPortFlush function
* @endinternal
*
* @brief   Flush port by a given index. All packets dequeued from this port (and all its queues)
*         are dropped. Wait till flush port has completed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Flush port blocking API.
*
*/
GT_STATUS cpssTmPortFlush
(
    IN GT_U8                    devNum,
    IN GT_U32                   index
);


/**
* @internal cpssTmPortFlushTriggerActionStart function
* @endinternal
*
* @brief   Flush port by a given index. All packets dequeued from this port (and all its queues)
*         are dropped.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Flush port non blocking API.
*
*/
GT_STATUS cpssTmPortFlushTriggerActionStart
(
    IN GT_U8                    devNum,
    IN GT_U32                   index
);


/**
* @internal cpssTmPortFlushTriggerActionStatusGet function
* @endinternal
*
* @brief   Get whether Flush port has completed.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
*
* @param[out] actFinishedPtr           - GT_FALSE - flush is active (packets are dropped on dequeue).
*                                      - GT_TRUE  - flush off /completed.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or index value or if index not in use.
* @retval GT_BAD_PTR               - on NULL actFinishedPtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmPortFlushTriggerActionStatusGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   index,
    OUT GT_BOOL                 *actFinishedPtr
);


/**
* @internal cpssTmNodesCtlReadNextChange function
* @endinternal
*
* @brief   Read next tree index/range change after reshuffling.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] changePtr                - Change structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_BAD_PTR               - on NULL changePtr.
* @retval GT_NO_RESOURCE           - on empty list.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmNodesCtlReadNextChange
(
    IN GT_U8                        devNum,
    OUT CPSS_TM_TREE_CHANGE_STC     *changePtr
);


/**
* @internal cpssTmNodesCtlCleanList function
* @endinternal
*
* @brief   Empty list of reshuffling changes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmNodesCtlCleanList
(
    IN GT_U8                        devNum
);


#endif 	    /* __cpssTmNodesCtlh */


