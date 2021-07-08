/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssTmNodesCreate.c
*
* DESCRIPTION:
*       TM nodes creation APIs
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/tm/cpssTmLogicalLayer.h>
#include <tm_logical_layer_interface.h>
#include <tm_nodes_create.h>

/**
* @internal internal_cpssTmNamedPortCreate function
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
static GT_STATUS internal_cpssTmNamedPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr,
    IN GT_U32                       cNodesNum,
    IN GT_U32                       bNodesNum,
    IN GT_U32                       aNodesNum,
    IN GT_U32                       queuesNum,
    IN const GT_8_PTR               portName
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_port_params core_params;
    int i;
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
    core_params.cir_bw = paramsPtr->cirBw;
    core_params.eir_bw = paramsPtr->eirBw;
    core_params.cbs = paramsPtr->cbs;
    core_params.ebs = paramsPtr->ebs;
    for(i=0; i<8; i++) {
        core_params.quantum[i] = (uint16_t)paramsPtr->quantumArr[i];
        core_params.dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    }
    core_params.wred_profile_ref = (uint8_t)paramsPtr->dropProfileInd;
    core_params.elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    core_params.num_of_children = (uint16_t)paramsPtr->numOfChildren;

    ret = tm_create_logical_port(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                 (uint8_t)portInd,
                                 &core_params,
                                 (uint16_t)cNodesNum,
                                 (uint16_t)bNodesNum,
                                 (uint16_t)aNodesNum,
                                 queuesNum,
                                 (const char *)portName);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmNamedPortCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum, portName));

    rc = internal_cpssTmNamedPortCreate(devNum, portInd, paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum, portName);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum, portName));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssTmNamedAsymPortCreate function
* @endinternal
*
* @brief   Create Named Port with asymmetric sub-tree and download its parameters to HW. This port can be accessed by its index and by it's name
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
static GT_STATUS internal_cpssTmNamedAsymPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       portInd,
    IN CPSS_TM_PORT_PARAMS_STC      *paramsPtr,
    IN const GT_8_PTR               portName
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_port_params core_params;
    int i;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    core_params.cir_bw = paramsPtr->cirBw;
    core_params.eir_bw = paramsPtr->eirBw;
    core_params.cbs = paramsPtr->cbs;
    core_params.ebs = paramsPtr->ebs;
    for(i=0; i<8; i++) {
        core_params.quantum[i] = (uint16_t)paramsPtr->quantumArr[i];
        core_params.dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    }
    core_params.wred_profile_ref = (uint8_t)paramsPtr->dropProfileInd;
    core_params.elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    core_params.num_of_children = (uint16_t)paramsPtr->numOfChildren;
    ret = tm_create_logical_asym_port(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                              (uint8_t)portInd,
                              &core_params,
                              (const char *)portName
                              );
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

/**
* @internal cpssTmNamedAsymPortCreate function
* @endinternal
*
* @brief   Create Named Port with asymmetric sub-tree and download its parameters to HW. This port can be accessed by its index and by it's name
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmNamedAsymPortCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, paramsPtr, portName));

    rc = internal_cpssTmNamedAsymPortCreate(devNum, portInd, paramsPtr, portName);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, paramsPtr, portName));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmNamedQueueToAnodeCreate function
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
static GT_STATUS internal_cpssTmNamedQueueToAnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      aNodeInd,
    IN  CPSS_TM_QUEUE_PARAMS_STC    *qParamsPtr,
    IN  const GT_8_PTR              queueName,
    OUT GT_U32                      *queueIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_queue_params core_params;
    uint32_t    node_index;

    CPSS_NULL_PTR_CHECK_MAC(qParamsPtr);
    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );


    core_params.shaping_profile_ref = qParamsPtr->shapingProfilePtr;
    core_params.quantum = (uint16_t)qParamsPtr->quantum;
    core_params.wred_profile_ref = (uint16_t)qParamsPtr->dropProfileInd;
    core_params.elig_prio_func_ptr = (uint8_t)qParamsPtr->eligiblePrioFuncId;

    ret=tm_create_logical_queue_to_a_node (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                         (uint32_t) aNodeInd,
                                         &core_params,
                                         (const char *)queueName,
                                         &node_index);

    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(!rc) *queueIndPtr=node_index;
    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmNamedQueueToAnodeCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, aNodeInd, qParamsPtr, queueName, queueIndPtr));

    rc = internal_cpssTmNamedQueueToAnodeCreate(devNum, aNodeInd, qParamsPtr, queueName, queueIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, aNodeInd, qParamsPtr, queueName, queueIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssTmNamedAnodeToBnodeCreate function
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
static GT_STATUS internal_cpssTmNamedAnodeToBnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      bNodeInd,
    IN  CPSS_TM_A_NODE_PARAMS_STC   *aParamsPtr,
    IN  const GT_8_PTR              nodeName,
    OUT GT_U32                      *aNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    int i;
    struct tm_a_node_params core_params;
    uint32_t  node_index;

    CPSS_NULL_PTR_CHECK_MAC(aParamsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    core_params.shaping_profile_ref = aParamsPtr->shapingProfilePtr;
    core_params.quantum = (uint16_t)aParamsPtr->quantum;
    for(i=0; i<8; i++)
        core_params.dwrr_priority[i] = (uint8_t)aParamsPtr->schdModeArr[i];
    core_params.wred_profile_ref = (uint16_t)aParamsPtr->dropProfileInd;
    core_params.elig_prio_func_ptr = (uint8_t)aParamsPtr->eligiblePrioFuncId;
    core_params.num_of_children = (uint16_t)aParamsPtr->numOfChildren;

    ret=tm_create_logical_a_node_to_b_node (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                         (uint32_t) bNodeInd,
                                         &core_params,
                                         (const char *)nodeName,
                                         &node_index);

    rc = XEL_TO_CPSS_ERR_CODE(ret);

    if(!rc) *aNodeIndPtr=node_index;
    return rc;
}

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
    IN  const GT_8_PTR              nodeName,
    OUT GT_U32                      *aNodeIndPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmNamedAnodeToBnodeCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bNodeInd, aParamsPtr, nodeName, aNodeIndPtr));

    rc = internal_cpssTmNamedAnodeToBnodeCreate(devNum, bNodeInd, aParamsPtr, nodeName, aNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bNodeInd, aParamsPtr, nodeName, aNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssTmNamedBnodeToCnodeCreate function
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
static GT_STATUS internal_cpssTmNamedBnodeToCnodeCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      cNodeInd,
    IN  CPSS_TM_B_NODE_PARAMS_STC   *bParamsPtr,
    IN  const GT_8_PTR              nodeName,
    OUT GT_U32                      *bNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_b_node_params core_params;
    int i;
    uint32_t  node_index;

    CPSS_NULL_PTR_CHECK_MAC(bParamsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    core_params.shaping_profile_ref = bParamsPtr->shapingProfilePtr;
    core_params.quantum = (uint16_t)bParamsPtr->quantum;
    for(i=0; i<8; i++)
        core_params.dwrr_priority[i] = (uint8_t)bParamsPtr->schdModeArr[i];
    core_params.wred_profile_ref = (uint16_t)bParamsPtr->dropProfileInd;
    core_params.elig_prio_func_ptr = (uint8_t)bParamsPtr->eligiblePrioFuncId;
    core_params.num_of_children = (uint16_t)bParamsPtr->numOfChildren;

    ret=tm_create_logical_b_node_to_c_node (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                         (uint32_t) cNodeInd,
                                         &core_params,
                                         (const char *)nodeName,
                                         &node_index);

    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(!rc) *bNodeIndPtr=node_index;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmNamedBnodeToCnodeCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cNodeInd, bParamsPtr, nodeName, bNodeIndPtr));

    rc = internal_cpssTmNamedBnodeToCnodeCreate(devNum, cNodeInd, bParamsPtr, nodeName, bNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cNodeInd, bParamsPtr, nodeName, bNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssTmNamedCnodeToPortCreate function
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
static GT_STATUS internal_cpssTmNamedCnodeToPortCreate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portInd,
    IN  CPSS_TM_C_NODE_PARAMS_STC   *cParamsPtr,
    IN  const GT_8_PTR              nodeName,
    OUT GT_U32                      *cNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_c_node_params core_params;
    int i;
    uint32_t  node_index;

    CPSS_NULL_PTR_CHECK_MAC(cParamsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );

    core_params.shaping_profile_ref = cParamsPtr->shapingProfilePtr;
    core_params.quantum = (uint16_t)cParamsPtr->quantum;
    for(i=0; i<8; i++)
        core_params.dwrr_priority[i] = (uint8_t)cParamsPtr->schdModeArr[i];
    core_params.wred_cos = (uint8_t)cParamsPtr->dropCosMap;
    for(i=0; i<8; i++)
        core_params.wred_profile_ref[i] = (uint8_t)cParamsPtr->dropProfileIndArr[i];
    core_params.elig_prio_func_ptr = (uint8_t)cParamsPtr->eligiblePrioFuncId;

    ret=tm_create_logical_c_node_to_port (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                         (uint8_t) portInd,
                                         &core_params,
                                         (const char *)nodeName,
                                         &node_index);
   rc = XEL_TO_CPSS_ERR_CODE(ret);

    if(!rc) *cNodeIndPtr=node_index;
    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmNamedCnodeToPortCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portInd, cParamsPtr, nodeName, cNodeIndPtr));

    rc = internal_cpssTmNamedCnodeToPortCreate(devNum, portInd, cParamsPtr, nodeName, cNodeIndPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portInd, cParamsPtr, nodeName, cNodeIndPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/***************************************************************************
 * mapping functions
 ***************************************************************************/

/**
* @internal internal_cpssTmGetLogicalNodeIndex function
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
static GT_STATUS internal_cpssTmGetLogicalNodeIndex
(
    IN  GT_U8                   devNum,
    IN  CPSS_TM_LEVEL_ENT       level,
    IN  const GT_8_PTR          nodeName,
    OUT GT_32                   *nodeIndexPtr
)
{
    int         nodeNumber;
    GT_STATUS   rc = GT_OK;
    CPSS_NULL_PTR_CHECK_MAC(nodeName);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
    nodeNumber=tm_get_node_number(  PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    level,
                                    (const char *) nodeName
                                 );
    *nodeIndexPtr=nodeNumber;
    if (nodeNumber==-1) rc=GT_NO_RESOURCE;
    return rc;
}

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

GT_STATUS       cpssTmGetLogicalNodeIndex
(
    IN  GT_U8                   devNum,
    IN  CPSS_TM_LEVEL_ENT       level,
    IN  const GT_8_PTR          nodeName,
    OUT GT_32                   *nodeIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmGetLogicalNodeIndex);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, level, nodeName, nodeIndexPtr));

    rc = internal_cpssTmGetLogicalNodeIndex(devNum, level, nodeName, nodeIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, level, nodeName, nodeIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssTmGetNodeLogicalName function
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
static GT_STATUS internal_cpssTmGetNodeLogicalName
(
    IN  GT_U8                   devNum,
    IN  CPSS_TM_LEVEL_ENT       level,
    IN  GT_U32                  nodeIndex,
    OUT GT_8_PTR                *nodeNamePtr
)
{
    GT_STATUS   rc = GT_OK;
    const char *  name=NULL;
    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
    name=tm_get_node_logical_name(  PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    level,
                                    (uint32_t) nodeIndex
                                 );
    if (!name) rc=GT_NO_RESOURCE;
    *nodeNamePtr=(GT_8_PTR)name;
    return rc;
}

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

GT_STATUS   cpssTmGetNodeLogicalName
(
    IN  GT_U8                   devNum,
    IN  CPSS_TM_LEVEL_ENT       level,
    IN  GT_U32                  nodeIndex,
    OUT GT_8_PTR                *nodeNamePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmGetNodeLogicalName);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, level, nodeIndex, nodeNamePtr));

    rc = internal_cpssTmGetNodeLogicalName(devNum, level, nodeIndex, nodeNamePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, level, nodeIndex, nodeNamePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

