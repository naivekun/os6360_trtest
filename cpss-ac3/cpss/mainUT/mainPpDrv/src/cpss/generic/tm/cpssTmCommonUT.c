/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssTmSchedUT.c
*
* DESCRIPTION:
*       Common Functions for TM unit tests.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/

#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesTree.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/cpssTmUtDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/**
* @internal cpssTmTestTmCtlLibInit function
* @endinternal
*
* @brief   common test API to open TM LIB after verify TM lib is closed
*
* @param[in] devNum                   - Device number.
*                                       GT_STATUS
*/
extern GT_STATUS cpssTmTestTmCtlLibInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS st;
    cpssTmClose(devNum);
    st = cpssTmInit(devNum);
    return st;
}

/**
* @internal cpssTmTestPeriodicSchemeConfiguration function
* @endinternal
*
* @brief   common test API to configure periodic scheme
*
* @param[in] devNum                   - Device number.
*                                       GT_STATUS
*/
GT_STATUS cpssTmTestPeriodicSchemeConfiguration
(
    IN GT_U8 devNum
)
{
    GT_STATUS   rc = GT_OK;
   
    
    CPSS_TM_LEVEL_PERIODIC_PARAMS_STC periodicSchemeArray[CPSS_TM_LEVEL_P_E+1];
 
    periodicSchemeArray[CPSS_TM_LEVEL_Q_E].periodicState = GT_FALSE;
    periodicSchemeArray[CPSS_TM_LEVEL_Q_E].shaperDecoupling = GT_FALSE;

    periodicSchemeArray[CPSS_TM_LEVEL_A_E].periodicState = GT_TRUE;
    periodicSchemeArray[CPSS_TM_LEVEL_A_E].shaperDecoupling = GT_FALSE;

    periodicSchemeArray[CPSS_TM_LEVEL_B_E].periodicState = GT_TRUE;
    periodicSchemeArray[CPSS_TM_LEVEL_B_E].shaperDecoupling = GT_FALSE;

    periodicSchemeArray[CPSS_TM_LEVEL_C_E].periodicState = GT_TRUE;
    periodicSchemeArray[CPSS_TM_LEVEL_C_E].shaperDecoupling = GT_FALSE;

    periodicSchemeArray[CPSS_TM_LEVEL_P_E].periodicState = GT_TRUE;
    periodicSchemeArray[CPSS_TM_LEVEL_P_E].shaperDecoupling = GT_FALSE;

    rc = cpssTmSchedPeriodicSchemeConfig(devNum, periodicSchemeArray);

    return rc; 
}

/**
* @internal cpssTmTestTreeBuild function
* @endinternal
*
* @brief   common test API to create single thread tree:
*         port(1)
*         \
*         cnode
*         \
*         bnode
*         \
*         anode
*         \
*         queue
* @param[in] devNum                   - Device number.
*
* @param[out] queueIndPtr              - (pointer of) queue index.
* @param[out] aNodeIndPtr              - (pointer of) anode index.
* @param[out] bNodeIndPtr              - (pointer of) bnode index.
* @param[out] cNodeIndPtr              - (pointer of) cnode index.
*                                       GT_STATUS
*/
GT_STATUS cpssTmTestTreeBuild
(
    IN  GT_U8   devNum,
    OUT GT_U32  *queueIndPtr,
    OUT GT_U32  *aNodeIndPtr,
    OUT GT_U32  *bNodeIndPtr,
    OUT GT_U32  *cNodeIndPtr
)
{
    GT_STATUS               st = GT_OK;

    GT_U32                       portInd;
    CPSS_TM_QUEUE_PARAMS_STC     qParamsPtr;
    CPSS_TM_A_NODE_PARAMS_STC    aParamsPtr;
    CPSS_TM_B_NODE_PARAMS_STC    bParamsPtr;
    CPSS_TM_C_NODE_PARAMS_STC    cParamsPtr;
    CPSS_TM_PORT_PARAMS_STC      paramsPtr;
    GT_U32                       cNodesNum;
    GT_U32                       bNodesNum;
    GT_U32                       aNodesNum;
    GT_U32                       queuesNum;
    GT_U32                       ii;

    cpssOsBzero((GT_VOID*)&qParamsPtr, sizeof(qParamsPtr));
    cpssOsBzero((GT_VOID*)&aParamsPtr, sizeof(aParamsPtr));
    cpssOsBzero((GT_VOID*)&bParamsPtr, sizeof(bParamsPtr));
    cpssOsBzero((GT_VOID*)&cParamsPtr, sizeof(cParamsPtr));
    cpssOsBzero((GT_VOID*)&paramsPtr, sizeof(paramsPtr));

    /* Configure parameters for node A */
    aParamsPtr.shapingProfilePtr = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    aParamsPtr.quantum = 0x40;
    for (ii=0; ii<8; ii++)
        aParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured */
    aParamsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
    aParamsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_FP5_E;

    /* Configure parameters for node B */
    bParamsPtr.shapingProfilePtr = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    bParamsPtr.quantum = 0x40;
    for (ii=0; ii<8; ii++)
        bParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E; /* No DWRR configured */
    bParamsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
    bParamsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_FP7_E;

    /* Configure parameters for node C */
    cParamsPtr.shapingProfilePtr = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    cParamsPtr.quantum = 0x40;
    cParamsPtr.dropCosMap = 1;
    cParamsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_FP6_E;
    for (ii=0; ii<8; ii++){
        cParamsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
        cParamsPtr.dropProfileIndArr[ii] = CPSS_TM_NO_DROP_PROFILE_CNS;
    }

    /* profile for Q level */
    qParamsPtr.shapingProfilePtr = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    qParamsPtr.quantum = 0x40;
    qParamsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;
    qParamsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_Q_PRIO1_E;

    /* Configure parameters for Port */
    paramsPtr.cirBw = 1000000;
    paramsPtr.eirBw = 0;
    paramsPtr.cbs = 8; /* KBytes */
    paramsPtr.ebs = 0x1FFFF;
    paramsPtr.eligiblePrioFuncId = CPSS_TM_ELIG_N_MIN_SHP_E;
    for(ii=0; ii<8; ii++){
        paramsPtr.schdModeArr[ii] = CPSS_TM_SCHD_MODE_RR_E;
        paramsPtr.quantumArr[ii] = UT_TM_MIN_PORT_QUANTUM_CNS;
    }
    paramsPtr.dropProfileInd = CPSS_TM_NO_DROP_PROFILE_CNS;

    portInd = 1;
    cNodesNum = 1;
    bNodesNum = 1;
    aNodesNum = 1;
    queuesNum = 1;

    st = cpssTmPortCreate(devNum, portInd, &paramsPtr, cNodesNum, bNodesNum, aNodesNum, queuesNum);
    if(st)
        return st;

    st = cpssTmQueueToPortCreate(devNum, 
                                 portInd, 
                                 &qParamsPtr, 
                                 &aParamsPtr, 
                                 &bParamsPtr, 
                                 &cParamsPtr,
                                 queueIndPtr, 
                                 aNodeIndPtr, 
                                 bNodeIndPtr, 
                                 cNodeIndPtr);

    return st;
}

/**
* @internal cpssTmTestTmClose function
* @endinternal
*
* @brief   common test API to Close TM LIB
*
* @param[in] devNum                   - Device number.
*                                       GT_STATUS
*/
extern GT_STATUS cpssTmTestTmClose
(
    IN GT_U8 devNum
)
{
    GT_STATUS st;
    CPSS_TM_LEVEL_ENT     level;

    for (level = CPSS_TM_LEVEL_Q_E; level <= CPSS_TM_LEVEL_P_E; level++)
    {
        st = cpssTmShapingPeriodicUpdateEnableSet(devNum, level, GT_FALSE);
        if (st != GT_OK)
        {
            return st;
        }
    }

    st = cpssTmClose(devNum);
    return st;
}



