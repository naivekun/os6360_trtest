/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssTmCtl.h
*
* DESCRIPTION: TM configuration library private utilities
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#ifndef __prvCpssTmCtlh
#define __prvCpssTmCtlh

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/tm/cpssTmPublicDefs.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
/* for CPSS_TM_CTL_LAD_INF_PARAM_STC definition */
#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/prvCpssTmDefs.h>


/**
* @struct PRV_CPSS_TM_HW_PARAMS_STC
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

    /** max number of TM Ports. */
    GT_U16 maxPorts;

    /** Default number of Queues which are connected to A node. */
    GT_U8 queuesToAnode;

    /** Default number of A nodes which are connected to B nodes. */
    GT_U8 aNodesToBnode;

    /** Default number of B nodes which are connected to C nodes. */
    GT_U8 bNodesToCnode;

    /** Default number of C nodes which are connected to TM ports. */
    GT_U8 cNodesToPort;

    /** @brief Default number of Queues which are installed on
     *  each TM port.
     */
    GT_U8 installedQueuesPerPort;

} PRV_CPSS_TM_HW_PARAMS_STC;


extern GT_U8 prvCpssMinDramInfArr[PRV_CPSS_TM_DEV_LAST_E+1];
extern GT_U8 prvCpssMaxDramInfArr[PRV_CPSS_TM_DEV_LAST_E+1];

/* macro to verify Lad interfaces range */
    /*PRV_CPSS_PP_MAC(devNum)->tmInfo.prvCpssTmDevId can not be used as this macro is called befor initSystem) */
#define PRV_CPSS_TM_CTL_CHECK_DRAM_INF_NUM_MAC(devNum, dramInfNum) \
    if(dramInfNum < prvCpssMinDramInfArr[prvCpssTmGetInternalDevId(devNum)]) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG); \
    if(dramInfNum > prvCpssMaxDramInfArr[prvCpssTmGetInternalDevId(devNum)]) \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);


/**
* @internal prvCpssTmGetInternalDevId function
* @endinternal
*
* @brief   Get TM Internal Device ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @retval PRV_CPSS_TM_DEVS_ENT != PRV_CPSS_TM_DEV_LAST_E - on success
*/
PRV_CPSS_TM_DEVS_ENT prvCpssTmGetInternalDevId
(
    IN  GT_U8   devNum
);


/**
* @internal prvCpssTmDumpPort function
* @endinternal
*
* @brief   print configured tree under a specific port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portIndex                - port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssTmDumpPort
(
    IN GT_U8    devNum,
    IN GT_U32   portIndex
);

/**
* @internal prvCpssTmCtlDumpPortHW function
* @endinternal
*
* @brief   print from HW configured tree under a specific port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] portIndex                - port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssTmCtlDumpPortHW
(
    IN GT_U8                   devNum,
    IN GT_U32                  portIndex
);

/**
* @internal prvCpssTmCtlLadParamsGetHW function
* @endinternal
*
* @brief   Get TM LAD parameters from HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] numOfLadsPtr             - (out pointer of)LAD number.
* @param[in] ladParamsPtr             - (out pointer of) CPSS_TM_CTL_LAD_INF_PARAM_STC.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or configuration parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssTmCtlLadParamsGetHW
(
    IN  GT_U8                         devNum,
    OUT GT_U32                        *numOfLadsPtr,
    OUT CPSS_TM_CTL_LAD_INF_PARAM_STC *ladParamsPtr
);


/**
* @internal prvCpssTmCtlHwInit function
* @endinternal
*
* @brief   Initialize the TM HW configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
* @param[in] numOfLad                 - Nubmer of lads.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssTmCtlHwInit
(
    IN GT_U8                   devNum,
    IN GT_U8                   numOfLad
);

/**
* @internal prvCpssTmCtlDevInit function
* @endinternal
*
* @brief   Initialize TM Device Info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssTmCtlDevInit
(
	IN GT_U8	devNum
);

/**
* @internal prvCpssDxChTMFreqGet function
* @endinternal
*
* @brief   Get TM Dram Frequency.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X; Puma2; Puma3; ExMx.
*
* @param[in] devNum                   - Device number.
*
* @param[out] tmFreqPtr                - (pointer of) tmFreqency
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on bad params.
*
* @note tmFreqency 0: tm not supported.
*       tmFreqency 1: CPSS_DRAM_FREQ_800_MHZ_E.
*       tmFreqency 2: CPSS_DRAM_FREQ_933_MHZ_E.
*       tmFreqency 3: CPSS_DRAM_FREQ_667_MHZ_E.
*
*/
GT_STATUS prvCpssDxChTMFreqGet
(
    IN  GT_U8    devNum,
    OUT GT_U32 * tmFreqPtr
);




#endif 	    /* __prvCpssTmCtlh */

