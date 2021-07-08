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
* @file cpssTmNodesCreate.h
*
* @brief TM nodes creation APIs
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmNodesCreateh
#define __cpssTmNodesCreateh

#include <cpss/generic/tm/cpssTmPublicDefs.h>

/***************************************************************************
 * Port Creation
 ***************************************************************************/

/**
* @internal cpssTmPortCreate function
* @endinternal
*
* @brief   Create Port and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port parameters structure pointer.
* @param[in] cNodesNum                - Number of C-nodes under port.
* @param[in] bNodesNum                - Number of B-nodes under port.
* @param[in] aNodesNum                - Number of A-nodes under port.
* @param[in] queuesNum                - Number of Queues under port.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_BAD_SIZE              - on too large port's min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. To indicate that no shaping is needed for port, set
*       cirBw to CPSS_TM_INVAL_CNS, in this case other shaping
*       parameters will not be considered.
*       2. Valid number of nodes per level must be multiple of number nodes
*       on upper level (parent nodes) in case of equal distribution. And
*       in case of unequal distribution all parent nodes except the last
*       one must have the same children range and the last parent node -
*       number of children less that the range. In case of not valid
*       number will be returned GT_BAD_PARAM error code.
*       3. If port will be used for transparent queues, set the c/b/aNodesNum = 1.
*
*/
GT_STATUS cpssTmPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr,
    IN GT_U32                       cNodesNum,
    IN GT_U32                       bNodesNum,
    IN GT_U32                       aNodesNum,
    IN GT_U32                       queuesNum
);


/**
* @internal cpssTmAsymPortCreate function
* @endinternal
*
* @brief   Create Port with assymetric sub-tree and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_BAD_SIZE              - on too large port's min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed for port, set
*       cirBw to CPSS_TM_INVAL_CNS, in this case other shaping
*       parameters will not be considered.
*
*/
GT_STATUS cpssTmAsymPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr
);


/**
* @internal cpssTmPortDropPerCosConfig function
* @endinternal
*
* @brief   Configure Port's Drop per Cos and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port Drop per Cos parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmPortDropPerCosConfig
(
    IN GT_U8                         devNum,
    IN GT_U32                        portInd,
    IN CPSS_TM_PORT_DROP_PER_COS_STC *paramsPtr
);


/***************************************************************************
 * Queue Creation
 ***************************************************************************/

/**
* @internal cpssTmQueueToPortCreate function
* @endinternal
*
* @brief   Create path from Queue to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmQueueToPortCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC    *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC   *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC   *cParamsPtr,
    OUT GT_U32                      *queueIndPtr,
    OUT GT_U32                      *aNodeIndPtr,
    OUT GT_U32                      *bNodeIndPtr,
    OUT GT_U32                      *cNodeIndPtr
);


/**
* @internal cpssTmTransQueueToPortCreate function
* @endinternal
*
* @brief   Create transparent path from Queue to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. This API introduces 'transparent' concept to TM nodes
*       structure that deals with Queues and Ports only. In this case no
*       configuration is needed for A,B,C level nodes, this nodes
*       are created automatically (one C-node, one B-node and one
*       A-node) and they are 'transparent' from the system point of
*       view. Transparent path can be created under symmetric port
*       only. To delete Queue from the structure - use
*       'cpssTmNodesCtlNodeDelete' API with level CPSS_TM_LEVEL_Q_E. To update queue
*       parameters - use 'cpssTmQueueUpdate' API. To delete Port from
*       the structure use 'cpssTmNodesCtlTransPortDelete' API. Applying any
*       other APIs on nodes underlying the port can cause unexpected
*       behavior of the system.
*       2. To indicate that no shaping is needed to the queue, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmTransQueueToPortCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC    *qParamsPtr,
    OUT GT_U32                      *queueIndPtr
);


/**
* @internal cpssTmQueueToCnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to C-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] cNodeInd                 - C-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on cNodeIndex not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmQueueToCnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      cNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC    *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC   *bParamsPtr,
    OUT GT_U32                      *queueIndPtr,
    OUT GT_U32                      *aNodeIndPtr,
    OUT GT_U32                      *bNodeIndPtr
);


/**
* @internal cpssTmQueueToBnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to B-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] bNodeInd                 - B-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on bNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmQueueToBnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      bNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC    *qParamsPtr,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
    OUT GT_U32                      *queueIndPtr,
    OUT GT_U32                      *aNodeIndPtr
);


/**
* @internal cpssTmQueueToAnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to A-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] aNodeInd                 - A-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
*
* @param[out] queueIndPtr              - (pointer to) The created Queue index (APPLICABLE RANGES: 0..65535).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on aNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmQueueToAnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      aNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC    *qParamsPtr,
    OUT GT_U32                      *queueIndPtr
);

/**
* @internal cpssTmQueueToAnodeByIndexCreate function
* @endinternal
*
* @brief   Create path from selected Queue to A-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] aNodeInd                 - A-Node parent index.
* @param[in] qParamsPtr               - pointer to Queue parameters structure.
* @param[in] queueInd                 - Index of Queue to init (APPLICABLE RANGES: 0..65535).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on aNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1.To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*       2. The selected queue must previously allocated to A-node parent
*
*/
GT_STATUS cpssTmQueueToAnodeByIndexCreate
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          aNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  GT_U32                          queueInd
);


/***************************************************************************
 * A-node Creation
 ***************************************************************************/

/**
* @internal cpssTmAnodeToPortCreate function
* @endinternal
*
* @brief   Create path from A-node to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmAnodeToPortCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC   *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC   *cParamsPtr,
    OUT GT_U32                      *aNodeIndPtr,
    OUT GT_U32                      *bNodeIndPtr,
    OUT GT_U32                      *cNodeIndPtr
);


/**
* @internal cpssTmAnodeToCnodeCreate function
* @endinternal
*
* @brief   Create path from A-node to C-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] cNodeInd                 - C-Node index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on cNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmAnodeToCnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      cNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
    IN  CPSS_TM_B_NODE_PARAMS_STC   *bParamsPtr,
    OUT GT_U32                      *aNodeIndPtr,
    OUT GT_U32                      *bNodeIndPtr
);


/**
* @internal cpssTmAnodeToBnodeCreate function
* @endinternal
*
* @brief   Create path from A-node to B-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] bNodeInd                 - B-Node index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on bNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmAnodeToBnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      bNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
    OUT GT_U32                      *aNodeIndPtr
);


/**
* @internal cpssTmAnodeToBnodeWithQueuePoolCreate function
* @endinternal
*
* @brief   Create path from A-node to B-node & allocates user defined queue children pool
*         Not applicable for symmetric port tree creation
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] bNodeInd                 - B-Node parent index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] firstQueueInRange        - index of first queue in required queue range
* @param[in] queueRangeSize           - queue range size
*
* @param[out] aNodeIndPtr              - (pointer to) The created A-Node index (APPLICABLE RANGES: 0..16383).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on resource allocation fail.
* @retval GT_BAD_STATE             - on bNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmAnodeToBnodeWithQueuePoolCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      bNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
	IN  GT_U32                      firstQueueInRange,
	IN  GT_U32                      queueRangeSize,
    OUT GT_U32                      *aNodeIndPtr
);



/***************************************************************************
 * B-node Creation
 ***************************************************************************/

/**
* @internal cpssTmBnodeToPortCreate function
* @endinternal
*
* @brief   Create path from B-node to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmBnodeToPortCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC   *bParamsPtr,
    IN  CPSS_TM_C_NODE_PARAMS_STC   *cParamsPtr,
    OUT GT_U32                      *bNodeIndPtr,
    OUT GT_U32                      *cNodeIndPtr
);


/**
* @internal cpssTmBnodeToCnodeCreate function
* @endinternal
*
* @brief   Create path from B-node to C-node.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] cNodeInd                 - C-Node index.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
*
* @param[out] bNodeIndPtr              - (pointer to) The created B-Node index (APPLICABLE RANGES: 0..4095).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on cNodeInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmBnodeToCnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      cNodeInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC   *bParamsPtr,
    OUT GT_U32                      *bNodeIndPtr
);


/***************************************************************************
 * C-node Creation
 ***************************************************************************/

/**
* @internal cpssTmCnodeToPortCreate function
* @endinternal
*
* @brief   Create path from C-node to Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
*
* @param[out] cNodeIndPtr              - (pointer to) The created C-Node index (APPLICABLE RANGES: 0..511).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmCnodeToPortCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portInd,
    IN  CPSS_TM_C_NODE_PARAMS_STC   *cParamsPtr,
    OUT GT_U32                      *cNodeIndPtr
);


#endif 	    /* __cpssTmNodesCreateh */


