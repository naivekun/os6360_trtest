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
* @file prvCpssDxChTxqPdq.c
*
* @brief CPSS SIP6 TXQ PDQ low level configurations.
*
* @version   1
********************************************************************************
*/

/*Synced to \Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_PR_IP \TXQ_PR_IP {7.0.5} \TXQ_PSI \TXQ_PDQ */
#include <cpssCommon/cpssPresteraDefs.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedCtl.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCtlInternal.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegs.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSched.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedShaping.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesTree.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesCreate.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <errno.h>

#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqPdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqSdq.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesRead.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesUpdate.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrors.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>


#define PRV_CPSS_DXCH_TXQ_SCHED_MINIMAL_PLEVEL_SPEED_IN_MB_MAC (10*1000) /*10 Gyga*/

extern GT_STATUS prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32      * tileNumPtr,
    OUT GT_U32      * dpNumPtr,
    OUT GT_U32      * localdpPortNumPtr,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT * mappingTypePtr
);

static const char* elig_func_node_names_arry[64] = {
       "TM_ELIG_N_PRIO1",
       "TM_ELIG_N_PRIO5",
       "TM_ELIG_N_SHP",
       "TM_ELIG_N_MIN_SHP",
       "TM_ELIG_N_PPA" ,
       "TM_ELIG_N_PPA_SP",
       "TM_ELIG_N_PPA_SHP",
       "TM_ELIG_N_PPA_SP_MIN_SHP" ,
       "TM_ELIG_N_PPA_SHP_IGN" ,
       "TM_ELIG_N_PPA_MIN_SHP_SP_IGN" ,
       "TM_ELIG_N_FP0" ,
       "TM_ELIG_N_FP1" ,
       "TM_ELIG_N_FP2" ,
       "TM_ELIG_N_FP3" ,
       "TM_ELIG_N_FP4" ,
       "TM_ELIG_N_FP5" ,
       "TM_ELIG_N_FP6" ,
       "TM_ELIG_N_FP7" ,
       "TM_ELIG_N_MIN_SHP_FP0",
       "TM_ELIG_N_MIN_SHP_FP1",
       "TM_ELIG_N_MIN_SHP_FP2",
       "TM_ELIG_N_MIN_SHP_FP3",
       "TM_ELIG_N_MIN_SHP_FP4",
       "TM_ELIG_N_MIN_SHP_FP5",
       "TM_ELIG_N_MIN_SHP_FP6",
       "TM_ELIG_N_MIN_SHP_FP7",
       "TM_ELIG_N_MAX_INC_MIN_SHP" ,
       "TM_ELIG_N_PP" ,
       "TM_ELIG_N_MIN_SHP_PP",
       "TM_ELIG_N_PP_SHP",
       "TM_ELIG_N_SHP_4P_MIN_4P_MAX",
       "TM_ELIG_N_SHP_PP_TB",
       "TM_ELIG_N_SHP_PP_MAX_TB_0",
       "TM_ELIG_N_MAX_LIM_SHP_FP0"
   };

   static const char* elig_func_queue_names_arry[] = {
       "TM_ELIG_Q_PRIO0",
       "TM_ELIG_Q_PRIO1",
       "TM_ELIG_Q_PRIO2",
       "TM_ELIG_Q_PRIO3",
       "TM_ELIG_Q_MIN_SHP_PRIO0",
       "TM_ELIG_Q_MIN_SHP_PRIO1",
       "TM_ELIG_Q_MIN_SHP_PRIO2",
       "TM_ELIG_Q_MIN_SHP_PRIO3",
       "TM_ELIG_Q_PRIO4",
       "TM_ELIG_Q_PRIO5",
       "TM_ELIG_Q_PRIO6",
       "TM_ELIG_Q_PRIO7",
       "TM_ELIG_Q_MIN_SHP_PRIO4",
       "TM_ELIG_Q_MIN_SHP_PRIO5",
       "TM_ELIG_Q_MIN_SHP_PRIO6",
       "TM_ELIG_Q_MIN_SHP_PRIO7",
       "TM_ELIG_Q_SHP",
       "TM_ELIG_Q_MAX_INC_MIN_SHP",
       "TM_ELIG_Q_SHP_SCHED00_PROP00",
       "TM_ELIG_Q_SHP_SCHED10_PROP10",
       "TM_ELIG_Q_SHP_SCHED20_PROP20",
       "TM_ELIG_Q_SHP_SCHED30_PROP30",
       "TM_ELIG_Q_SHP_SCHED40_PROP40",
       "TM_ELIG_Q_SHP_SCHED50_PROP50",
       "TM_ELIG_Q_SHP_SCHED60_PROP60",
       "TM_ELIG_Q_SHP_SCHED70_PROP70"
   };

GT_STATUS schedToCpssErrCodes[TM_CONF_MAX_ERROR+1] = {GT_NOT_INITIALIZED};
GT_STATUS schedToCpssErrCodesNeg[140] = {GT_NOT_INITIALIZED};

#define FALCON_XEL_TO_CPSS_ERR_CODE(x,pdqNum)\
    falcon_xel_to_cpss_err_code(devNum,x,pdqNum,__FUNCNAME__,__LINE__)

/* convert errors from xel code to CPSS error code */
/* and support for the CPSS ERROR LOG */
static GT_STATUS falcon_xel_to_cpss_err_code(IN GT_U8 devNum,IN int retCode_xel,IN  GT_U32 pdqNum,IN const char * funcName,IN GT_U32    lineNum)
{
    GT_STATUS rc = ( (PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedLibIsInitilized[pdqNum] == GT_FALSE) ?
    GT_NOT_INITIALIZED : ( (retCode_xel) < 0) ?  schedToCpssErrCodesNeg[-(retCode_xel)]: schedToCpssErrCodes[(retCode_xel)]);

    if(rc != GT_OK)
    {
#ifndef CPSS_LOG_ENABLE
    funcName = funcName;/*avoid warning of unused parameter */
    lineNum  = lineNum;
#endif/*!CPSS_LOG_ENABLE*/

        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "[%s , line[%d]] failed : in pdqNum[%d] (retCode_xel[%d])",
            funcName,lineNum,pdqNum,retCode_xel);
    }

    return rc;
}

static GT_VOID prvCpssSchedPortParamsCpy
(
    IN  PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC     *paramsPtr,
    OUT struct tm_port_params       *prmsPtr
);

static GT_VOID prvCpssSchedCNodeParamsCpy
(
    IN  PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC      *paramsPtr,
    OUT struct tm_c_node_params        *prmsPtr
);

static GT_VOID prvCpssSchedANodeParamsCpy
(
    IN  PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC      *paramsPtr,
    OUT struct tm_a_node_params        *prmsPtr
);
static GT_VOID prvCpssSchedBNodeParamsCpy
(
    IN  PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC      *paramsPtr,
    OUT struct tm_b_node_params        *prmsPtr
);

static GT_VOID prvCpssSchedQueueParamsCpy
(
    IN  PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC      *paramsPtr,
    OUT struct tm_queue_params        *prmsPtr
);


/**
* @struct PRV_CPSS_SCHED_HW_PARAMS_STC
 *
 * @brief TM HW properties and Limits.
*/
typedef struct{

    /** max number of Queue Nodes. */
    GT_U16 maxQueues;

    /** max number of A Nodes. */
    GT_U16 maxAnodes;

    /** max number of B Nodes. */
    GT_U16 maxBnodes;

    /** max number of C Nodes. */
    GT_U16 maxCnodes;

    /** @brief max number of TM Ports.
     *  queuesToAnode - Default number of Queues which are connected to A node.
     *  aNodesToBnode - Default number of A nodes which are connected to B nodes.
     *  bNodesToCnode - Default number of B nodes which are connected to C nodes.
     *  cNodesToPort - Default number of C nodes which are connected to TM ports.
     *  installedQueuesPerPort - Default number of Queues which are installed on
     *  each TM port.
     */
    GT_U16 maxPorts;

} PRV_CPSS_SCHED_HW_PARAMS_STC;



static PRV_CPSS_SCHED_HW_PARAMS_STC pdqSchedHwParamsFalcon =
{
    2048,           /* maxQueues */
    512,           /* maxAnodes */
    128,           /* maxBnodes */
    128, /* maxCnodes */
    128  /* maxPorts  */
};

static GT_VOID prvSchedToCpssErrCodesInit
(
    GT_VOID
)
{
    int       i = 0;

    /* Positive errors */
    /* 0 on Success */
    schedToCpssErrCodes[0] = GT_OK;

    /* HW errors */
    for (i=1; i<=TM_HW_MAX_ERROR; i++) {
        schedToCpssErrCodes[i] = GT_FAIL;
    }

    /* SW (configuration) errors */
    for (i=TM_HW_MAX_ERROR+1; i<=TM_CONF_PER_RATE_L_K_N_NOT_FOUND; i++) {
        schedToCpssErrCodes[i] = GT_BAD_VALUE;
    }

    for (i=TM_CONF_PORT_IND_OOR; i<=TM_CONF_TM2TM_PORT_FOR_CTRL_PKT_OOR; i++) {
        schedToCpssErrCodes[i] = GT_OUT_OF_RANGE;
    }

    for (i=TM_CONF_PORT_BW_OUT_OF_SPEED; i<=TM_CONF_REORDER_NODES_NOT_ADJECENT; i++) {
        schedToCpssErrCodes[i] = GT_BAD_PARAM;
    }

    for (i=TM_CONF_BURST_TOO_SMALL_FOR_GIVEN_RATE; i<=TM_CONF_REORDER_CHILDREN_NOT_AVAIL; i++) {
        schedToCpssErrCodes[i] = GT_BAD_SIZE;
    }

    for (i=TM_CONF_PORT_IND_NOT_EXIST; i<=TM_CONF_C_NODE_IND_NOT_EXIST; i++) {
        schedToCpssErrCodes[i] = GT_BAD_STATE;
    }

    schedToCpssErrCodes[TM_CONF_CANNT_GET_LAD_FREQUENCY] = GT_GET_ERROR; /* Possibly not relevant for cpss */

    for (i=TM_CONF_UPD_RATE_NOT_CONF_FOR_LEVEL; i<=TM_CONF_TM2TM_CHANNEL_NOT_CONFIGURED; i++) {
        schedToCpssErrCodes[i] = GT_NOT_INITIALIZED;
    }

    schedToCpssErrCodes[TM_CONF_PORT_IND_USED] = GT_ALREADY_EXIST;
    schedToCpssErrCodes[TM_CONF_SHAPING_PROF_REF_OOR]=GT_BAD_VALUE;
    schedToCpssErrCodes[TM_WRONG_SHP_PROFILE_LEVEL]=GT_BAD_VALUE;
    /* new periodic errors */
    schedToCpssErrCodes[TM_BW_OUT_OF_RANGE]=GT_OUT_OF_RANGE;
    schedToCpssErrCodes[TM_BW_UNDERFLOW]=GT_OUT_OF_RANGE;
    /* logical layer errors */
    schedToCpssErrCodes[TM_CONF_WRONG_LOGICAL_NAME] = GT_BAD_VALUE;
    schedToCpssErrCodes[TM_CONF_NULL_LOGICAL_NAME] = GT_BAD_VALUE;


    /* Negative errors */
    schedToCpssErrCodesNeg[EINVAL]  = GT_BAD_PTR;
    schedToCpssErrCodesNeg[EBADF]   = GT_BAD_PARAM;
    schedToCpssErrCodesNeg[ENOBUFS] = GT_NO_RESOURCE;
    schedToCpssErrCodesNeg[ENOMEM]  = GT_NO_RESOURCE;
    schedToCpssErrCodesNeg[EBADMSG] = GT_NOT_INITIALIZED;
    schedToCpssErrCodesNeg[EACCES]  = GT_BAD_PARAM;
    schedToCpssErrCodesNeg[EADDRNOTAVAIL] = GT_BAD_PARAM;
    schedToCpssErrCodesNeg[EDOM]    = GT_BAD_PARAM;
    schedToCpssErrCodesNeg[EFAULT]  = GT_BAD_PARAM;
    schedToCpssErrCodesNeg[ENODATA] = GT_BAD_PARAM;
    schedToCpssErrCodesNeg[ENODEV]  = GT_BAD_PARAM;
    schedToCpssErrCodesNeg[ERANGE]  = GT_BAD_PARAM;
    schedToCpssErrCodesNeg[EPERM]   = GT_BAD_PARAM;
    schedToCpssErrCodesNeg[EBUSY]   = GT_BAD_STATE;
    schedToCpssErrCodesNeg[ENOSPC]  = GT_FULL;
}

/**
* @internal prvCpssFalconTxqPdqInit function
* @endinternal
*
* @brief   Initialize the PDQ scheduler configuration library.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssFalconTxqPdqInit
(
    IN GT_U8 devNum,
    IN  GT_U32 pdqNum,
    IN GT_U32 mtu
)
{
    GT_STATUS                   rc = GT_OK;
    struct PRV_CPSS_SCHED_LIB_INIT_PARAMS_STC   tm_lib_init_params = {0};
    struct PRV_CPSS_SCHED_TREE_MAX_SIZES_STC    pdqSchedTreeParams;
    PRV_CPSS_SCHED_HW_PARAMS_STC   *pdqSchedHwParamsPtr;
    int                         ret = 0;

    PRV_CPSS_DEV_CHECK_MAC(devNum);



    if (PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedLibIsInitilized[pdqNum] == GT_TRUE)
    {
        /*TM_DBG_INFO(("---- cpssTmInit:tm hndl already exist and tm_lib was invoked, ignoring ...\n"));*/
        return GT_OK;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.regDataBaseInitialized == GT_FALSE)
    {

         /* init tm registers */
        prvSchedInitAdressStruct();

        prvSchedToCpssErrCodesInit();

        PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.regDataBaseInitialized = GT_TRUE;
    }




    tm_lib_init_params.schedLibMtu = mtu;

    tm_lib_init_params.schedLibPdqNum = pdqNum;

    pdqSchedHwParamsPtr = &pdqSchedHwParamsFalcon;


    pdqSchedTreeParams.numOfQueues = pdqSchedHwParamsPtr->maxQueues;
    pdqSchedTreeParams.numOfAnodes = pdqSchedHwParamsPtr->maxAnodes;
    pdqSchedTreeParams.numOfBnodes = pdqSchedHwParamsPtr->maxBnodes;
    pdqSchedTreeParams.numOfCnodes = pdqSchedHwParamsPtr->maxCnodes;
    pdqSchedTreeParams.numOfPorts  = pdqSchedHwParamsPtr->maxPorts;



    /* Scheduler Initialization */
    ret = prvSchedLibOpenExt(devNum, &pdqSchedTreeParams,  &tm_lib_init_params, &PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum]);

    if (ret == 0)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedLibIsInitilized[pdqNum] = GT_TRUE;
    }

    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    return rc;
}

/**
* @internal prvCpssFalconTxqPdqPeriodicSchemeConfig function
* @endinternal
*
* @brief   Configure Periodic Scheme.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
* @param[in] paramsPtr                - (pointer to) scheduling parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The API may be invoked once in a system lifetime.
*       2. To indicate the shaping is disabled for a level the
*       periodicState field must be set to GT_FALSE.
*       3 paramsPtr is handled as 5 elements array including also a
*       port level scheduling configuration.
*
*/
GT_STATUS prvCpssFalconTxqPdqPeriodicSchemeConfig
(
    IN GT_U8                                                devNum,
    IN GT_U32                                               pdqNum,
    IN PRV_CPSS_PDQ_SCHED_LEVEL_PERIODIC_PARAMS_STC        *paramsPtr
)
{
    int         i;
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    struct schedPerLevelPeriodicParams prms[P_LEVEL+1];


    for(i=Q_LEVEL; i<=P_LEVEL; i++)
    {
        prms[i].per_state = (uint8_t)(paramsPtr[i].periodicState);
        prms[i].shaper_dec = (uint8_t)(paramsPtr[i].shaperDecoupling);
    }

    ret = prvCpssTxqSchedulerConfigurePeriodicScheme(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], prms);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    return rc;
}

/**
* @internal prvCpssFalconTxqPdqShapingProfileCreate function
* @endinternal
*
* @brief   Create a Shaping Profile.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] level                    - Level to configure the profile for.
* @param[in] profileStrPtr            - Shaping profile configuration struct pointer.
*
* @param[out] profileIndPtr            - (pointer to) The created Shaping profile index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssFalconTxqPdqShapingProfileCreate
(
    IN  GT_U8                                   devNum,
    IN GT_U32                                   pdqNum,
    IN  CPSS_PDQ_SCHED_LEVEL_ENT                       level,
    IN  CPSS_PDQ_SCHED_SHAPING_PROFILE_PARAMS_STC      *profileStrPtr,
    OUT GT_U32                                  *profileIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    struct tm_shaping_profile_params prf;
    uint32_t prof_index;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                           | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_PUMA_E | CPSS_PUMA3_E  );
    CPSS_NULL_PTR_CHECK_MAC(profileStrPtr);
    CPSS_NULL_PTR_CHECK_MAC(profileIndPtr);

    prf.cir_bw = profileStrPtr->cirBw;
    prf.cbs = profileStrPtr->cbs;
    prf.eir_bw = profileStrPtr->eirBw;
    prf.ebs = profileStrPtr->ebs;
    ret = prvCpssSchedShapingCreateShapingProfile(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                    level,
                                    &prf,
                                    &prof_index);

    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
    if(rc)
        return rc;

    *profileIndPtr = prof_index;
    return rc;
}

GT_STATUS prvCpssFalconTxqPdqTreePlevelDwrrEnableSet
(
    IN GT_U8               devNum,
    IN GT_U32              pdqNum,
    IN GT_BOOL            enable
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    uint8_t prio[8];
    int i;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    /*Enable/disable for all levels*/
    for(i=0; i<8 ;i++)
        prio[i] = enable;

    ret = prvCpssSchedNodesTreeSetDwrrPrio(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], prio);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    return rc;
}


/**
* @internal prvCpssFalconTxqPdqAsymPortCreate function
* @endinternal
*
* @brief   Create Port with assymetric sub-tree and download its parameters to HW.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
* @param[in] portInd                  - Port index.
* @param[in] paramsPtr                - Port parameters structure pointer.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note To indicate that no shaping is needed for port, set
*       cirBw to CPSS_PDQ_SCHED_INVAL_DATA_CNS, in this case other shaping
*       parameters will not be considered.
*
* @param[in]  fixedMappingToCNode - if equal GT_TRUE then P[i] is mapped to C[i]
*/

GT_STATUS prvCpssFalconTxqPdqAsymPortCreate
(
    IN GT_U8                        devNum,
    IN GT_U32                       pdqNum,
    IN GT_U32                       portInd,
    IN  GT_BOOL                  fixedMappingToCNode,
    IN PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC      *paramsPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_port_params prms;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);



    /* check that the port index is valid */
    if(portInd >71 && portInd<128)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssSchedPortParamsCpy(paramsPtr, &prms);
    ret = prvCpssSchedNodesCreateAsymPort(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                              (uint8_t)portInd,
                              &prms,fixedMappingToCNode);

     rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    return rc;
}


/**
* @internal prvCpssFalconTxqPdqAnodeToPortCreate function
* @endinternal
*
* @brief   Configure Periodic Scheme.
*
 *
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - tile number.(APPLICABLE RANGES:0..3)
* @param[in] portInd                  - Port index.
* @param[in] aParamsPtr               - A-Node parameters structure pointer.
* @param[in] explicitQueueStart       - if GT_TRUE then queue number is scpecified by the caller,otherwise first free queue number is given
* @param[in] queueStart               - relevant if explicitQueueStart is equal GT_TRUE.Queue number requested by the caller
* @param[in] fixedPortToBnodeMapping - if equal GT_TRUE then P[i] is mapped to C[i] , mapped to B[i]
* @param[in] bParamsPtr               - B-Node parameters structure pointer.
* @param[in] cParamsPtr               - C-Node parameters structure pointer.
* @param[in] addToExisting            - if equal GT_TRUE then A node is added to already existing P->C->B branch.
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

GT_STATUS prvCpssFalconTxqPdqAnodeToPortCreate
(
    IN  GT_U8                           devNum,
    IN GT_U32                           pdqNum,
    IN  GT_U32                          portInd,
    IN  PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC       *aParamsPtr,
    IN  GT_BOOL explicitQueueStart,
    IN  GT_U32  queueStart,
    IN GT_BOOL fixedPortToBnodeMapping,
    IN  PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC       *bParamsPtr,
    IN  PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC       *cParamsPtr,
    IN GT_BOOL                           addToExisting,
    OUT GT_U32                          *aNodeIndPtr,
    OUT GT_U32                          *bNodeIndPtr,
    OUT GT_U32                          *cNodeIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_a_node_params a_prms;
    struct tm_b_node_params b_prms;
    struct tm_c_node_params c_prms;
    uint32_t a_index;
    uint32_t b_index;
    uint32_t c_index;


    CPSS_NULL_PTR_CHECK_MAC(aParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(bParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(cParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(aNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(bNodeIndPtr);
    CPSS_NULL_PTR_CHECK_MAC(cNodeIndPtr);


    prvCpssSchedANodeParamsCpy(aParamsPtr, &a_prms);
    prvCpssSchedBNodeParamsCpy(bParamsPtr, &b_prms);
    prvCpssSchedCNodeParamsCpy(cParamsPtr, &c_prms);

    if((fixedPortToBnodeMapping == GT_TRUE)&&(addToExisting == GT_TRUE))
    {
        b_index = portInd;
     }

    ret = prvCpssSchedNodesCreateANodeToPort(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                   (uint8_t)portInd,
                                   &a_prms,
                                   explicitQueueStart,
                                   queueStart,
                                   fixedPortToBnodeMapping,
                                   &b_prms,
                                   &c_prms,
                                   &a_index,
                                   &b_index,
                                   &c_index,addToExisting);

    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
    if(rc)
        return rc;

    *aNodeIndPtr = a_index;
    *bNodeIndPtr = b_index;
    *cNodeIndPtr = c_index;
    return rc;
}

/**
* @internal prvCpssFalconTxqPdqQueueToAnodeCreate function
* @endinternal
*
* @brief   Create path from Queue to A-node.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
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
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/

GT_STATUS prvCpssFalconTxqPdqQueueToAnodeCreate
(
    IN  GT_U8                           devNum,
    IN GT_U32                           pdqNum,
    IN  GT_U32                          aNodeInd,
    IN  PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC        *qParamsPtr,
    IN  GT_BOOL                     explicitNumber,
    OUT GT_U32                          *queueIndPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_queue_params q_prms;
    uint32_t q_index;

    CPSS_NULL_PTR_CHECK_MAC(qParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(queueIndPtr);

    prvCpssSchedQueueParamsCpy(qParamsPtr, &q_prms);

    if(GT_FALSE == explicitNumber)
    {
         q_index = (uint32_t) -1;
    }
    else
    {
        q_index = *queueIndPtr;
    }


    ret = prvCpssSchedNodesCreateQueueToANode(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                    aNodeInd,
                                    &q_prms,
                                    &q_index);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
    if(rc)
        return rc;

    *queueIndPtr = q_index;
    return rc;
}



/**
* @internal prvCpssSchedPortParamsCpy function
* @endinternal
*
* @brief   Copy CPSS Port structure to TM Port structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] paramsPtr                - CPSS Port parameters structure pointer.
*                                       None.
*/
GT_VOID prvCpssSchedPortParamsCpy
(
    IN  PRV_CPSS_PDQ_SCHD_PORT_PARAMS_STC     *paramsPtr,
    OUT struct tm_port_params       *prmsPtr
)
{
    int i;

    prmsPtr->cir_bw = paramsPtr->cirBw;
    prmsPtr->eir_bw = paramsPtr->eirBw;
    prmsPtr->cbs = paramsPtr->cbs;
    prmsPtr->ebs = paramsPtr->ebs;
    for(i=0; i<8; i++) {
        prmsPtr->quantum[i] = (uint16_t)paramsPtr->quantumArr[i];
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    }

    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children = (uint16_t)paramsPtr->numOfChildren;
}


/**
* @internal prvCpssSchedCNodeParamsCpy function
* @endinternal
*
* @brief   Copy CPSS C-Node structure to TM C-Node structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] paramsPtr                - CPSS C-Node parameters structure pointer.
*                                       None.
*/
GT_VOID prvCpssSchedCNodeParamsCpy
(
    IN  PRV_CPSS_PDQ_SCHD_C_NODE_PARAMS_STC      *paramsPtr,
    OUT struct tm_c_node_params        *prmsPtr
)
{
    int i;

    prmsPtr->shaping_profile_ref = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum = (uint16_t)paramsPtr->quantum;
    for(i=0; i<8; i++)
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];

    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children = (uint16_t)paramsPtr->numOfChildren;
}


/**
* @internal prvCpssSchedBNodeParamsCpy function
* @endinternal
*
* @brief   Copy CPSS B-Node structure to TM B-Node structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] paramsPtr                - CPSS B-Node parameters structure pointer.
*                                       None.
*/
GT_VOID prvCpssSchedBNodeParamsCpy
(
    IN  PRV_CPSS_PDQ_SCHD_B_NODE_PARAMS_STC      *paramsPtr,
    OUT struct tm_b_node_params        *prmsPtr
)
{
    int i;

    prmsPtr->shaping_profile_ref = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum = (uint16_t)paramsPtr->quantum;
    for(i=0; i<8; i++)
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children = (uint16_t)paramsPtr->numOfChildren;
}


/**
* @internal prvCpssSchedANodeParamsCpy function
* @endinternal
*
* @brief   Copy CPSS A-Node structure to TM A-Node structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] paramsPtr                - CPSS A-Node parameters structure pointer.
*                                       None.
*/
GT_VOID prvCpssSchedANodeParamsCpy
(
    IN  PRV_CPSS_PDQ_SCHD_A_NODE_PARAMS_STC      *paramsPtr,
    OUT struct tm_a_node_params        *prmsPtr
)
{
    int i;

    prmsPtr->shaping_profile_ref = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum = (uint16_t)paramsPtr->quantum;
    for(i=0; i<8; i++)
        prmsPtr->dwrr_priority[i] = (uint8_t)paramsPtr->schdModeArr[i];

    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
    prmsPtr->num_of_children = (uint16_t)paramsPtr->numOfChildren;
}


/**
* @internal prvCpssSchedQueueParamsCpy function
* @endinternal
*
* @brief   Copy CPSS Queue structure to TM Queue structure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] paramsPtr                - CPSS Queue parameters structure pointer.
*                                       None.
*/
GT_VOID prvCpssSchedQueueParamsCpy
(
    IN  PRV_CPSS_PDQ_SCHD_QUEUE_PARAMS_STC      *paramsPtr,
    OUT struct tm_queue_params        *prmsPtr
)
{
    prmsPtr->shaping_profile_ref = paramsPtr->shapingProfilePtr;
    prmsPtr->quantum = (uint16_t)paramsPtr->quantum;
    prmsPtr->elig_prio_func_ptr = (uint8_t)paramsPtr->eligiblePrioFuncId;
}

/**
* @internal prvCpssFalconTxqPdqPrintNodeFunc function
* @endinternal
*
* @brief   This function print node
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NO_RESOURCE           - on memory allocation fail.
* @retval GT_BAD_STATE             - on portInd not in use.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/

static int prvCpssFalconTxqPdqPrintNodeFunc(int bBeginNodeEval,
                             void * vpEnv,
                             unsigned int level,
                             unsigned int node_index,
                             unsigned int quantum,
                             unsigned int node_parent_index ,
                             unsigned int elig_fun ,
                             void * vpUserData)
{
    GT_U8    devNum;
    devNum = *((GT_U8*)(vpEnv));

    (void)vpUserData;
    (void)node_parent_index;
    (void)devNum;
    (void)quantum;
    elig_func_node_names_arry[63] = "undefind";

    switch(level)
    {
        case CPSS_PDQ_SCHED_LEVEL_P_E:
        {
            if (bBeginNodeEval) cpssOsPrintf("port :  index = %d    quantum[0] %d elig_fun = %d     func_name =  %s\n",node_index,quantum,elig_fun,elig_func_node_names_arry[elig_fun]);
            break;
        }
        case CPSS_PDQ_SCHED_LEVEL_C_E:
        {
            if (bBeginNodeEval) cpssOsPrintf("      C_node : index = %d    elig_fun = %d     func_name =  %s\n",node_index, elig_fun,elig_func_node_names_arry[elig_fun]);
            break;
        }
        case CPSS_PDQ_SCHED_LEVEL_B_E:
        {
            if (bBeginNodeEval) cpssOsPrintf("            B_node :  index = %d    elig_fun = %d     func_name =  %s\n",node_index, elig_fun,elig_func_node_names_arry[elig_fun]);
            break;
        }
        case CPSS_PDQ_SCHED_LEVEL_A_E:
        {
            if (bBeginNodeEval) cpssOsPrintf("                  A_node :  index = %d   quantum %d  elig_fun=%d     func_name =  %s\n",node_index, quantum,elig_fun,elig_func_node_names_arry[elig_fun]);
            break;
        }
        case CPSS_PDQ_SCHED_LEVEL_Q_E:
        {
            /* queue callback is called once */
                                cpssOsPrintf("                        Queue :  index = %d  quantum %d  elig_fun = %d     func_name =  %s\n",node_index,quantum, elig_fun, elig_func_queue_names_arry[elig_fun]);
            break;
        }
        default :               cpssOsPrintf(" unknown level=%d index=%d\n",level,node_index); return 1;
    }

    return 0;
}

/**
* @internal prvCpssFalconTxqPdqDumpPortSw function
* @endinternal
*
* @brief   print the configure tree under a specific port (SW data base ,no read from HW)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] portIndex                - Port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqPdqDumpPortSw
(
    IN GT_U8                   devNum,
    IN GT_U32                  pdqNum,
    IN GT_U32                  portIndex
)
{
    GT_STATUS                  rc = GT_OK;
    int                        ret;


    ret = prvCpssSchedNodesReadTraversePortTreeSw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                    portIndex,
                                    prvCpssFalconTxqPdqPrintNodeFunc,
                                    &devNum,
                                    NULL);
    if (ret) rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
    return rc;
}
/**
* @internal prvCpssFalconTxqPdqDumpPortHw function
* @endinternal
*
* @brief   print the configure tree under a specific port ( read from HW)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] portIndex                - Port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqPdqDumpPortHw
(
    IN GT_U8                   devNum,
    IN GT_U32                  pdqNum,
    IN GT_U32                  portIndex
)
{
    GT_STATUS                  rc = GT_OK;
    int                        ret;


    ret = prvCpssSchedNodesReadTraversePortTreeHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                    portIndex,
                                    prvCpssFalconTxqPdqPrintNodeFunc,
                                    &devNum,
                                    NULL);

    if (ret) rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
    return rc;
}
/**
* @internal prvCpssFalconTxqPdqGetShapingOnNode function
* @endinternal
*
* @brief   Read shaping parameters of specific node( read from SW)
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] nodeInd                  - Node index.
* @param[in] isQueueNode              - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] burstSizePtr             - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[out] maxRatePtr               - Requested Rate in Kbps
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/


GT_STATUS prvCpssFalconTxqPdqGetShapingOnNode
(
    IN      GT_U8                        devNum,
    IN      GT_U32                       pdqNum,
    IN      GT_U32                       nodeInd,
    IN      GT_BOOL                       isQueueNode,
    OUT  GT_U16    *burstSizePtr,
    OUT  GT_U32    *maxRatePtr

)
{

    int                        ret;
    GT_STATUS                  rc = GT_OK ;
    GT_U16          profileNum;
    struct tm_shaping_profile_params           params;

    ret = prvCpssSchedShapingReadNodeShaping(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
        isQueueNode?Q_LEVEL:A_LEVEL,
        nodeInd,&profileNum,&params);

    if (ret)
    {   rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
        return rc;
    }

    *burstSizePtr=(GT_U16)(params.cbs/4)/*In 4K bytes units*/;
    *maxRatePtr = params.cir_bw;

    return rc;

}

/**
* @internal prvCpssFalconTxqPdqSetShapingOnNode function
* @endinternal
*
* @brief   Write shaping parameters to specific node
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] nodeInd                  - Node index.
* @param[in] isQueueNode              - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[in,out] maxRatePtr               - (pointer to)Requested Rate in Kbps
* @param[in,out] maxRatePtr               -(pointer to) the actual Rate value in Kbps.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqPdqSetShapingOnNode
(
    IN      GT_U8                        devNum,
    IN      GT_U32                       pdqNum,
    IN      GT_U32                       nodeInd,
    IN      GT_BOOL                       isQueueNode,
    IN      GT_U16    burstSize,
    INOUT GT_U32    *maxRatePtr

)
{

    int                        ret;
    GT_STATUS                  rc = GT_OK ;
    struct tm_shaping_profile_params           params;

    params.cir_bw = *maxRatePtr;
   /* burst size in units of 4K bytes*/
   params.cbs = burstSize*4;
   params.eir_bw = 0;
   params.ebs = 0;

    ret = prvCpssSchedShapingUpdateNodeShaping(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
        isQueueNode?Q_LEVEL:A_LEVEL,
        nodeInd,&params,maxRatePtr);

    if (ret)
    {   rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
        return rc;
    }

    return rc;

}

static GT_U8 prvCpssFalconTxqPdqCalculateEligPrioFuncForAlevelShaping(GT_BOOL enable)
{
        if(enable == GT_TRUE)
        {
            return CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_PP_E;
        }
        else
        {
            return CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP1_E;
        }
}


static GT_BOOL prvCpssFalconTxqPdqShapingStatusForAlevelGet(GT_U32 eligPrioFunc)
{
        if(eligPrioFunc == CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_PP_E)
        {
            return GT_TRUE;
        }
        else
        {
            return GT_FALSE;
        }
}

static GT_BOOL prvCpssFalconTxqPdqShapingStatusForQlevelGet(GT_U32 eligPrioFunc)
{
         switch(eligPrioFunc)
         {
            case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO0_E:
            case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO1_E:
            case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO2_E:
            case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO3_E:
            case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO4_E:
            case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO5_E:
            case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO6_E:
            case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO7_E:
                return GT_TRUE;
                break;
             default:
               return GT_FALSE;
               break;
         }
}




static GT_U8 prvCpssFalconTxqPdqCalculateEligPrioFuncForQlevelShaping(GT_BOOL enable,GT_U8  oldEligPrioFunc)
{
        GT_U8 result ;

        if(enable == GT_TRUE)
        {
            switch(oldEligPrioFunc)
            {
                    case     CPSS_PDQ_SCHED_ELIG_Q_PRIO0_E:
                        result= CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO0_E;
                        break;
                    case     CPSS_PDQ_SCHED_ELIG_Q_PRIO1_E:
                        result= CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO1_E;
                        break;
                    case     CPSS_PDQ_SCHED_ELIG_Q_PRIO2_E:
                        result= CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO2_E;
                        break;
                    case     CPSS_PDQ_SCHED_ELIG_Q_PRIO3_E:
                        result= CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO3_E;
                        break;
                    case     CPSS_PDQ_SCHED_ELIG_Q_PRIO4_E:
                         result= CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO4_E;
                        break;
                    case     CPSS_PDQ_SCHED_ELIG_Q_PRIO5_E:
                         result= CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO5_E;
                        break;
                    case     CPSS_PDQ_SCHED_ELIG_Q_PRIO6_E:
                        result= CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO6_E;
                        break;
                    case     CPSS_PDQ_SCHED_ELIG_Q_PRIO7_E:
                        result= CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO7_E;
                        break;
                     default:
                        result = oldEligPrioFunc;
                        break;
            }
        }
        else
        {
                     switch(oldEligPrioFunc)
                     {
                        case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO0_E:
                            result= CPSS_PDQ_SCHED_ELIG_Q_PRIO0_E;
                            break;
                         case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO1_E:
                            result= CPSS_PDQ_SCHED_ELIG_Q_PRIO1_E;
                            break;
                         case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO2_E:
                            result= CPSS_PDQ_SCHED_ELIG_Q_PRIO2_E;
                            break;
                         case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO3_E:
                            result= CPSS_PDQ_SCHED_ELIG_Q_PRIO3_E;
                            break;
                        case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO4_E:
                            result= CPSS_PDQ_SCHED_ELIG_Q_PRIO4_E;
                            break;
                         case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO5_E:
                            result= CPSS_PDQ_SCHED_ELIG_Q_PRIO5_E;
                            break;
                         case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO6_E:
                            result= CPSS_PDQ_SCHED_ELIG_Q_PRIO6_E;
                            break;
                         case     CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO7_E:
                            result= CPSS_PDQ_SCHED_ELIG_Q_PRIO7_E;
                            break;
                         default:
                            result = oldEligPrioFunc;
                            break;
                     }
          }

        return result;
}

/**
* @internal prvCpssFalconTxqPdqSetShapingEnable function
* @endinternal
*
* @brief   Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] nodeInd                  - Node index.
* @param[in] isQueueNode              - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqPdqSetShapingEnable
(
    IN      GT_U8                         devNum,
    IN      GT_U32                       pdqNum,
    IN      GT_U32                       nodeInd,
    IN      GT_BOOL                   isQueueNode,
    IN      GT_BOOL                   enable
)
{

    int                        ret;
    GT_STATUS                  rc = GT_OK ;
    GT_U8 eligFunc;
    struct tm_queue_params  params;

    if(isQueueNode == GT_FALSE)
     {
        eligFunc = prvCpssFalconTxqPdqCalculateEligPrioFuncForAlevelShaping(enable);
     }
    else
     {
            /*Read old elig prio func in order to get old priority*/
            ret = prvCpssSchedNodesReadQueueConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                nodeInd,&params);

          if (ret)
         {   rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
            return rc;
         }

        eligFunc =  prvCpssFalconTxqPdqCalculateEligPrioFuncForQlevelShaping(enable,params.elig_prio_func_ptr);

     }


    ret = prvCpssSchedUpdateEligFunc(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
        isQueueNode?Q_LEVEL:A_LEVEL, nodeInd,eligFunc);

    if (ret)
    {   rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
        return rc;
    }

    return rc;

}

/**
* @internal prvCpssFalconTxqPdqGetShapingEnable function
* @endinternal
*
* @brief   Get Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] nodeInd                  - Node index.
* @param[in] isQueueNode              - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] enablePtr                - GT_TRUE, enable Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqPdqGetShapingEnable
(
    IN      GT_U8                         devNum,
    IN      GT_U32                       pdqNum,
    IN      GT_U32                       nodeInd,
    IN      GT_BOOL                   isQueueNode,
    OUT      GT_BOOL                   *enablePtr
)
{

    int                        ret;
    GT_STATUS                  rc = GT_OK ;
    struct tm_a_node_params  aNodeParams;
    struct tm_queue_params queueParams;



    if(isQueueNode == GT_FALSE)
    {
        ret = prvCpssSchedNodesReadANodeConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                nodeInd,&aNodeParams);

        if (ret)
        {   rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
            return rc;
        }

        *enablePtr =  prvCpssFalconTxqPdqShapingStatusForAlevelGet(aNodeParams.elig_prio_func_ptr);


    }
    else
    {
        ret = prvCpssSchedNodesReadQueueConfiguration(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                nodeInd,&queueParams);

        if (ret)
        {   rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
            return rc;
        }

        *enablePtr = prvCpssFalconTxqPdqShapingStatusForQlevelGet(queueParams.elig_prio_func_ptr);

    }

    return GT_OK;


}

/**
* @internal prvCpssFalconTxqPdqUpdateTypeANodeSchedulingAttributes function
* @endinternal
*
* @brief   Update scheduler HW with shadow scheduling profile attributes
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                   Device number.
* @param[in] pdqNum                                     - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] schedAtrributesPtr                  - (pointer to)Scheduling attributes
* @param[in] lowLatencyQueueIsPresent     if equal GT_TRUE then it mean that there is at least one queue in SP group
* @param[in] firstPnodeQueue                -      First queue in corresponding Pnode
* @param[in] lastPnodeQueue                -      First queue in corresponding Pnode
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqPdqUpdateTypeANodeSchedulingAttributes
(
    IN      GT_U8                                   devNum,
    IN      GT_U32                                  pdqNum,
    IN      PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE          *aNodePtr,
    IN      PRV_CPSS_DXCH_TXQ_SIP_6_SCHED_PROFILE   *schedAtrributesPtr,
    IN      GT_BOOL                                 lowLatencyQueueIsPresent,
    IN      GT_U32                                  firstPnodeQueue,
    IN      GT_U32                                  lastPnodeQueue
)
{
        struct tm_queue_params params;
        int                        ret;
        GT_STATUS                  rc = GT_OK ;
        GT_U32 i,localIndex =0;
        GT_BOOL lowLatency;

        GT_U32 dummy,localPort;
        /*GT_U32  dwrrGroupSize = 0;*/
        PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE range;
       uint8_t eligFuncArray []= {TM_ELIG_Q_PRIO0,TM_ELIG_Q_PRIO1,TM_ELIG_Q_PRIO2,TM_ELIG_Q_PRIO3,
            TM_ELIG_Q_PRIO4,TM_ELIG_Q_PRIO5,TM_ELIG_Q_PRIO6,TM_ELIG_Q_PRIO7};

        GT_U32 currentSpFunction =0;
        GT_U32 wrrGroup0Function =0;
        GT_U32 wrrGroup1Function =0;


        GT_BOOL shapingEnabled;

        params.shaping_profile_ref = TM_INVAL;
        /*First check the profile*/

        if(schedAtrributesPtr->wrrEnableBmp)
        {
                if(schedAtrributesPtr->wrrGroupBmp ==0)
                {
                    /*all wrr group 0*/
                    currentSpFunction = 1;

                }
                else if(schedAtrributesPtr->wrrGroupBmp ==0xFF)
                {
                     /*all wrr group 1*/
                    currentSpFunction = 1;
                }
                else
                 {
                    /*both  groups are present */
                    wrrGroup0Function = 0;
                    wrrGroup1Function = 1;
                    currentSpFunction = 2;
                 }
        }
        else
        {
            /*all sp*/
            currentSpFunction=0;
        }



        for(i=aNodePtr->queuesData.pdqQueueFirst;i<=aNodePtr->queuesData.pdqQueueLast;i++,localIndex++)
        {

          params.quantum = (uint16_t)schedAtrributesPtr->weights[localIndex];

          lowLatency = GT_FALSE;

                /*DWRR group*/
              if(schedAtrributesPtr->wrrEnableBmp&(1<<localIndex))
              {
                     /*dwrrGroupSize++;*/

                    /*DWRR group 1*/
                     if(schedAtrributesPtr->wrrGroupBmp&(1<<localIndex))
                     {
                            params.elig_prio_func_ptr = eligFuncArray[wrrGroup1Function];
                     }
                     else
                        /*DWRR group 0*/
                     {
                           params.elig_prio_func_ptr = eligFuncArray[wrrGroup0Function];
                     }
              }
              else
               /*SP group*/
              {
                    params.elig_prio_func_ptr = eligFuncArray[currentSpFunction++];

                    if(currentSpFunction == 8)
                    {
                        currentSpFunction=7;/*In case there are more then 8 queues mapped*/
                    }

                    lowLatency  = (schedAtrributesPtr->lowLatencyQueue == localIndex)?GT_TRUE:GT_FALSE;
              }

            /*set SDQ related configuration*/
            rc = prvCpssFalconTxqSdqQueueStrictPrioritySet(devNum,pdqNum,aNodePtr->queuesData.dp,
                                aNodePtr->queuesData.queueBase+localIndex,lowLatency);


            if(rc)
            {
                return rc;
             }

            /*Check if shaping was configured*/

            rc = prvCpssFalconTxqPdqGetShapingEnable(devNum,pdqNum,i,GT_TRUE,&shapingEnabled);

            if(rc)
             {
                return rc;
              }

            /*If shaping was configured then change elig prio func to contain shapibg*/

            if(GT_TRUE==shapingEnabled)
            {
                 params.elig_prio_func_ptr =  prvCpssFalconTxqPdqCalculateEligPrioFuncForQlevelShaping(GT_TRUE,params.elig_prio_func_ptr);
            }

            ret = prvCpssSchedUpdateQ(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                            i,&params);

            if (ret)
            {   rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
                return rc;
            }
        }

        /*Need a local port number*/
        rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,aNodePtr->physicalPort,&dummy,&dummy,&localPort,NULL);
        if(rc)
        {
            return rc;
         }

        /* keep this code if we decide to support more than one queue at low latency


         if(schedAtrributesPtr->wrrEnableBmp ==0)
         {
                range.prio1LowLimit =  aNodePtr->queuesData.pdqQueueFirst;
                range.prio1HighLimit =  aNodePtr->queuesData.pdqQueueLast;
                range.prio0LowLimit = 0;
                range.prio0HighLimit = 0;
         }


         else if(dwrrGroupSize == localIndex)
        {
                range.prio1LowLimit = 0;
                range.prio1HighLimit =  0;
                range.prio0LowLimit =  aNodePtr->queuesData.pdqQueueFirst;
                range.prio0HighLimit = aNodePtr->queuesData.pdqQueueLast;
         }
         else
         {
            range.prio0LowLimit = aNodePtr->queuesData.pdqQueueFirst;
            range.prio1LowLimit = range.prio0LowLimit + dwrrGroupSize-1;
            range.prio0HighLimit = range.prio1LowLimit +1;
            range.prio1HighLimit =  aNodePtr->queuesData.pdqQueueLast;
         }

         */

        /*currently support only one low latency queue*/

        range.prio0LowLimit = firstPnodeQueue;

        if(lowLatencyQueueIsPresent == GT_FALSE)
        {
          range.prio0HighLimit =  lastPnodeQueue;
          range.prio1LowLimit =  0;
          range.prio1HighLimit = 0;
        }
        else
        {
         range.prio0HighLimit = lastPnodeQueue-1;
         range.prio1LowLimit =  lastPnodeQueue;
         range.prio1HighLimit = lastPnodeQueue;
        }

         /*set SDQ related configuration*/
        rc = prvCpssFalconTxqSdqSelectListSet(devNum,pdqNum,aNodePtr->queuesData.dp,localPort,&range);
        if(rc)
        {
            return rc;
        }
        return GT_OK;
}

/**
* @internal prvCpssFalconTxqGlobalTxEnableSet function
* @endinternal
 *
* @brief   Enable/Disable transmission of specified tile on specific device
*
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] enable           -
*                                      GT_TRUE, enable transmission
*                                      GT_FALSE, disable transmission
*/
GT_STATUS   prvCpssFalconTxqPdqGlobalTxEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_BOOL  enable
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    ret = prvCpssSchedNodesTreeChangeStatus(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], enable);
      rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    return rc;
}

/**
* @internal prvCpssFalconTxqGlobalTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission of specified tile on specific device (Enable/Disable).
*                Note this function check the status in all availeble tiles (The status should be the SAME)
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE, enable transmission
*                                      GT_FALSE, disable transmission
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGlobalTxEnableGet
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_BOOL  * enablePtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    uint8_t status;

    ret = prvCpssSchedNodesTreeGetDeqStatus(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], &status);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    if(rc == GT_OK)
    {
        *enablePtr = (status==0)?GT_FALSE:GT_TRUE;
    }

    return rc;
}
/**
* @internal prvCpssFalconTxqPdqGetErrorStatus function
* @endinternal
*
* @brief   Get the status of scheduler errors
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
*
* @param[out] errorCounterPtr                - (pointer to) error counter
* @param[out] exceptionCounterPtr                - (pointer to) exception counter
* @param[out] errorStatusPtr                - (pointer to) error status
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGetErrorStatus
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   * errorCounterPtr,
    IN GT_U32   * exceptionCounterPtr,
    IN GT_U32   * errorStatusPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct tm_error_info info;


    ret = prvSchedGetErrors(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], &info);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    if(rc == GT_OK)
    {
        *errorCounterPtr = info.error_counter;
        *exceptionCounterPtr = info.exception_counter;

         ret = prvSchedGetErrorStatus(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum], errorStatusPtr);
         rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);
    }

    return rc;
}

/**
* @internal prvCpssFalconTxqPdqGetQueueStatus function
* @endinternal
*
* @brief   Read queue token bucket level and deficit
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] queueNum                   - queue number .(APPLICABLE RANGES:0..2K-1)

* @param[out] levelPtr                - (pointer to)  token bucket level
* @param[out] deficitPtr                - (pointer to) deficit
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS   prvCpssFalconTxqPdqGetQueueStatus
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   queueNum,
    OUT GT_32   *  levelPtr,
    OUT GT_U32   * dificitPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct schedNodeStatus  status;

    ret = prvSchedLowLevelGetQueueStatus(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],queueNum, &status);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    if(rc == GT_OK)
    {
        *levelPtr = status.min_bucket_level;

        if(status.min_bucket_sign == 1)
        {
            (*levelPtr )*= (-1);
        }

        *dificitPtr = status.deficit;
    }

    return rc;
}

/**
* @internal prvCpssFalconTxqPdqGetAlevelStatus function
* @endinternal
*
* @brief   Read A level token bucket level and deficit
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] alevelNum                   - A level number .(APPLICABLE RANGES:0..511)

* @param[out] levelPtr                - (pointer to)  token bucket level
* @param[out] deficitPtr                - (pointer to) deficit
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGetAlevelStatus
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   alevelNum,
    OUT GT_32   *  levelPtr,
    OUT GT_U32   * dificitPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct schedNodeStatus  status;

    ret = prvSchedLowLevelGetANodeStatus(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],alevelNum, &status);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    if(rc == GT_OK)
    {
        *levelPtr = status.min_bucket_level;

        if(status.min_bucket_sign == 1)
        {
            (*levelPtr )*= (-1);
        }

        *dificitPtr = status.deficit;
    }

    return rc;
}

/**
* @internal prvCpssFalconTxqPdqGetQueueShapingParameters function
* @endinternal
*
* @brief   Read Q level shaping configurations
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] queueNum                   - Q level number .(APPLICABLE RANGES:0..2047)

* @param[out] divExpPtr                - (pointer to)  divider exponent
* @param[out] tokenPtr                - (pointer to) number of tokens
* @param[out] resPtr                - (pointer to)  resolution exponent
* @param[out] burstPtr                - (pointer to) burst size
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGetQueueShapingParameters
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   queueNum,
    OUT GT_U32   *  divExpPtr,
    OUT GT_U32   * tokenPtr,
    OUT GT_U32   * resPtr,
    OUT GT_U32   * burstPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct prvCpssDxChTxqSchedShapingProfile profile;

    ret = prvSchedLowLevelGetNodeShapingParametersFromHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                                                                                                                Q_LEVEL,queueNum, &profile);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    if(rc == GT_OK)
    {
        *divExpPtr = profile.min_div_exp;
         *tokenPtr = profile.min_token;
         *resPtr = profile.min_token_res;
         *burstPtr = profile.min_burst_size;
    }

    return rc;
}
/**
* @internal prvCpssFalconTxqPdqGetPerLevelShapingParameters function
* @endinternal
*
* @brief   Read per  level shaping configurations
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] queueLevel                   - equal GT_TRUE if request is for Q level ,else A level

* @param[out] shaperDecPtr                - (pointer to) shaper decopling
* @param[out] perInterPtr                - (pointer to) periodic interval
* @param[out] perEnPtr                - (pointer to)  per level shaping enabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGetPerLevelShapingParameters
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_BOOL  queueLevel,
    OUT GT_U32   *  shaperDecPtr,
    OUT GT_U32   * perInterPtr,
    OUT GT_U32   * perEnPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct schedPerLevelPeriodicParams  periodicParams;

    ret = prvSchedLowLevelGetPerLevelShapingStatusFromHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                                                                                                                queueLevel?Q_LEVEL:A_LEVEL, &periodicParams);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    if(rc == GT_OK)
    {
        *shaperDecPtr = periodicParams.shaper_dec;
        *perInterPtr = periodicParams.per_interval;
        *perEnPtr = periodicParams.per_state;
    }

    return rc;
}
/**
* @internal prvCpssFalconTxqPdqGetQueueShapingParameters function
* @endinternal
*
* @brief   Read A level shaping configurations
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2;Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] pdqNum                   - Tile number.(APPLICABLE RANGES:0..3)
* @param[in] aNodeNum                   - A level number .(APPLICABLE RANGES:0..511)

* @param[out] divExpPtr                - (pointer to)  divider exponent
* @param[out] tokenPtr                - (pointer to) number of tokens
* @param[out] resPtr                - (pointer to)  resolution exponent
* @param[out] burstPtr                - (pointer to) burst size
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqPdqGetAnodeShapingParameters
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32  aNodeNum,
    OUT GT_U32   *  divExpPtr,
    OUT GT_U32   * tokenPtr,
    OUT GT_U32   * resPtr,
    OUT GT_U32   * burstPtr
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct prvCpssDxChTxqSchedShapingProfile profile;

    ret = prvSchedLowLevelGetNodeShapingParametersFromHw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                                                                                                                A_LEVEL,aNodeNum, &profile);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    if(rc == GT_OK)
    {
        *divExpPtr = profile.min_div_exp;
         *tokenPtr = profile.min_token;
         *resPtr = profile.min_token_res;
         *burstPtr = profile.min_burst_size;
    }

    return rc;
}

GT_STATUS prvCpssFalconTxqPdqConvertSpeedToQuantum
(
        IN CPSS_PORT_SPEED_ENT speed ,
        IN GT_BOOL isRemotePort,
        OUT GT_U32 * quantum
)
{
        GT_STATUS rc = GT_OK;
        GT_U32 chunks = 0;

        switch(speed)
        {
                case CPSS_PORT_SPEED_10000_E:
                    chunks = 10000/PRV_CPSS_DXCH_TXQ_SCHED_MINIMAL_PLEVEL_SPEED_IN_MB_MAC;
                    break;
                case CPSS_PORT_SPEED_50000_E:
                    chunks = 50000/PRV_CPSS_DXCH_TXQ_SCHED_MINIMAL_PLEVEL_SPEED_IN_MB_MAC;
                    break;
                case CPSS_PORT_SPEED_100G_E:
                    chunks = 100000/PRV_CPSS_DXCH_TXQ_SCHED_MINIMAL_PLEVEL_SPEED_IN_MB_MAC;
                    break;

               case CPSS_PORT_SPEED_200G_E:
                    chunks = 200000/PRV_CPSS_DXCH_TXQ_SCHED_MINIMAL_PLEVEL_SPEED_IN_MB_MAC;
                    break;
               case CPSS_PORT_SPEED_400G_E:
                    chunks = 400000/PRV_CPSS_DXCH_TXQ_SCHED_MINIMAL_PLEVEL_SPEED_IN_MB_MAC;
                    break;

               default :
                   rc = GT_FAIL;
                  break;
        }

        if(rc == GT_OK)
        {
            if(isRemotePort == GT_FALSE)
            {
                *quantum = chunks*CPSS_PDQ_SCHED_MIN_PORT_QUANTUM_CNS;
            }
            else
            {
                *quantum = chunks*CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS;
            }
        }

        return rc;
}


GT_STATUS   prvCpssFalconTxqPdqSetPortQuantum
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   portIndex,
    IN GT_U32   quantum
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    GT_U16 quantumArr[8];
    GT_U8 dummy[8];
    GT_U32 i;

    for(i = 0;i<8;i++)
    {
        quantumArr[i] = (GT_U16)quantum;
    }

    ret = prvCpssSchedUpdatePortScheduling(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],
                                                                                                                                (uint8_t)portIndex, 0 /*don't care*/,quantumArr,dummy,GT_TRUE);
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);


    return rc;
}

GT_STATUS   prvCpssFalconTxqPdqSetAlevelQuantum
(
    IN GT_U8    devNum,
    IN GT_U32  pdqNum,
    IN GT_U32   aNodeIndex,
    IN GT_U32   quantum
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

     struct tm_a_node_params params;

    params.quantum = (uint16_t)quantum;

    ret = prvCpssSchedUpdateNodeTypeA(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],aNodeIndex,&params,GT_TRUE);

    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);


    return rc;
}

/**
* @internal prvCpssFalconTxqPdqClose function
* @endinternal
*
* @brief  Release dynamic memory allocation for tile specific TxQ scheduler
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             -physical device number
* @param[in] pdqNum             -number of PDQ[0..3]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqPdqClose
(
    IN GT_U8   devNum,
    IN GT_U32  pdqNum
)
{
    int             ret = 0;
    GT_STATUS       rc;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum]==NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
     /* Scheduler destructor*/

    ret = prvSchedLibCloseSw(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum]);

    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,pdqNum);

    return rc;
}

