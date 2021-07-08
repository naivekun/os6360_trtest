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
* @file cpssTmLogicalLayer.h
*
* @brief API for creation TM nodes with persistent names
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmLogicalLayerh
#define __cpssTmLogicalLayerh

#include <cpss/generic/tm/cpssTmPublicDefs.h>

/***************************************************************************
 * Port Creation
 ***************************************************************************/

/**
* @internal cpssTmNamedPortCreate function
* @endinternal
*
* @brief   Create NamedPort and download its parameters to HW. This port can be accessed by its index and by it's name
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
* @param[in] portName                 - Name for port to be created.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_BAD_SIZE              - on too large port's min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_VALUE             - portName is NULL or already occupied.
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
GT_STATUS cpssTmNamedPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr,
    IN GT_U32                       cNodesNum,
    IN GT_U32                       bNodesNum,
    IN GT_U32                       aNodesNum,
    IN GT_U32                       queuesNum,
	IN const GT_8_PTR               portName
);


/**
* @internal cpssTmNamedAsymPortCreate function
* @endinternal
*
* @brief   Create Named Port with assymetric sub-tree and download its parameters to HW. This port can be accessed by its index and by it's name
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port parameters structure pointer.
* @param[in] portName                 - Name for port to be created.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_ALREADY_EXIST         - on portInd already in use.
* @retval GT_BAD_SIZE              - on too large port's min/max token.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_VALUE             - portName is NULL or already occupied.
*
* @note To indicate that no shaping is needed for port, set
*       cirBw to CPSS_TM_INVAL_CNS, in this case other shaping
*       parameters will not be considered.
*
*/
GT_STATUS cpssTmNamedAsymPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr,
	IN const GT_8_PTR               portName
);






/**
* @internal cpssTmNamedQueueToAnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to A-node. The created queue has a name and can be accessed also by name or by it's index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] aNodeInd                 - A-Node index.
* @param[in] qParamsPtr               - Queue parameters structure pointer.
* @param[in] queueName                - Name assigned to created Queue.
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
* @retval GT_BAD_VALUE             - queueName is NULL or already occupied.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmNamedQueueToAnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      aNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC    *qParamsPtr,
	IN  const GT_8_PTR              queueName,
    OUT GT_U32                      *queueIndPtr
);



/**
* @internal cpssTmNamedAnodeToBnodeCreate function
* @endinternal
*
* @brief   Create path from A-node to B-node. The created A-node has "persistent" name and can be accessed by it always , even if
*         physical index of this node is changed as a result of reshuffling process.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] bNodeInd                 - B-Node index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] nodeName                 - name assigned to created A-Node
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
* @retval GT_BAD_VALUE             - nodeName is NULL or already occupied on for it's level.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmNamedAnodeToBnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      bNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
	IN  const GT_8_PTR                     nodeName,
    OUT GT_U32                      *aNodeIndPtr
);


/**
* @internal cpssTmNamedBnodeToCnodeCreate function
* @endinternal
*
* @brief   Create path from B-node to C-node. The created B-node has "persistent" name and can be accessed by it always , even if
*         physical index of this node is changed as a result of reshuffling process.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] cNodeInd                 - C-Node index.
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] nodeName                 - name assigned to created B-Node
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
* @retval GT_BAD_VALUE             - nodeName is NULL or already occupied on for it's level.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmNamedBnodeToCnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      cNodeInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC   *bParamsPtr,
	IN  const GT_8_PTR              nodeName,
    OUT GT_U32                      *bNodeIndPtr
);

/**
* @internal cpssTmNamedCnodeToPortCreate function
* @endinternal
*
* @brief   Create path from C-node to Port. This C-node can be accessed by its index and by it's name
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portInd                  - Port index.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
* @param[in] nodeName                 - name assigned to created C-Node
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
* @retval GT_BAD_VALUE             - nodeName is NULL or already occupied on for it's level.
*
* @note To indicate that no shaping is needed to the queue/node, set
*       shapingProfilePtr to CPSS_TM_INVAL_CNS.
*
*/
GT_STATUS cpssTmNamedCnodeToPortCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portInd,
    IN  CPSS_TM_C_NODE_PARAMS_STC   *cParamsPtr,
	IN  const GT_8_PTR              nodeName,
    OUT GT_U32                      *cNodeIndPtr
);

/***************************************************************************
 * mapping functions
 ***************************************************************************/
/**
* @internal cpssTmGetLogicalNodeIndex function
* @endinternal
*
* @brief   Returns physical index of node with given logical name
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - node  ( Port / C- / B- /A- /Queue
* @param[in] nodeName                 - name of interesting node
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NO_RESOURCE           - index not found.
*/

GT_STATUS		cpssTmGetLogicalNodeIndex
(
    IN  GT_U8                    devNum,
    IN  CPSS_TM_LEVEL_ENT        level,
 	IN  const GT_8_PTR           nodeName,
	OUT GT_32                    *nodeIndexPtr
);
/**
* @internal cpssTmGetNodeLogicalName function
* @endinternal
*
* @brief   Returns logical name of node with given index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - node  ( Port / C- / B- /A- /Queue
* @param[in] nodeIndex                - physical index of the node in it's layer
*
* @param[out] nodeNamePtr              - (pointer to) The name of node with given index (if call successful)
*                                      NULL if call failed;
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NO_RESOURCE           - name not found.
*/

GT_STATUS	cpssTmGetNodeLogicalName
(
    IN  GT_U8                    devNum,
    IN  CPSS_TM_LEVEL_ENT        level,
 	IN  GT_U32                   nodeIndex,
	OUT GT_8_PTR                 *nodeNamePtr
);
#endif 	    /* __cpssTmLogicalLayerh */


