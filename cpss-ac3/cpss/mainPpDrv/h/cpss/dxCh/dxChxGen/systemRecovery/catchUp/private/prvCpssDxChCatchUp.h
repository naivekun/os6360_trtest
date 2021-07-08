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
* @file prvCpssDxChCatchUp.h
*
* @brief private CPSS DxCh catch up data.
*
* @version   8
********************************************************************************
*/

#ifndef __prvCpssDxChCatchUph
#define __prvCpssDxChCatchUph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbAu.h>

#ifdef ASIC_SIMULATION
    #define PRV_CPSS_DXCH_CATCHUP_RETURN_IF_DEV_IS_BOBK_OR_ALDRIN_SIMULATION_MAC(_devNum)                  \
        if( (PRV_CPSS_PP_MAC(_devNum)->appDevFamily) & (CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E) ) \
            return GT_OK /* ; in caller */
#else
    #define PRV_CPSS_DXCH_CATCHUP_RETURN_IF_DEV_IS_BOBK_OR_ALDRIN_SIMULATION_MAC(_devNum)
#endif

/**
* @struct PRV_CPSS_DXCH_AUQ_ENABLE_DATA_STC
 *
 * @brief This struct containes AUQ enable/disable status for different message types.
*/
typedef struct{

    GT_BOOL *naToCpuPerPortPtr;

    GT_BOOL *naStormPreventPortPtr;

    /** @brief enable/disable status of NA messages to the CPU indicating that
     *  the device cannot learn a new SA.
     */
    GT_BOOL naToCpuLearnFail;

    /** enable/disable status of AA and TA address update messages to the CPU. */
    GT_BOOL aaTaToCpu;

    /** @brief enable/disable status of AA messages to the CPU for the Storm Prevention
     *  entries when those entries reach an aged out status.
     */
    GT_BOOL spAaMsgToCpu;

    /** @brief enable/disable status of Tag1 VLAN Id assignment in vid1 field of the NA AU
     *  message CPSS_MAC_UPDATE_MSG_EXT_STC.
     */
    GT_BOOL naTag1VLANassignment;

    GT_BOOL AUMessageToCPUStop;

} PRV_CPSS_DXCH_AUQ_ENABLE_DATA_STC;

/**
* @struct PRV_CPSS_DXCH_FUQ_ACTION_DATA_STC
 *
 * @brief This struct containes FUQ enable/disable statuses of FDB upload action.
*/
typedef struct{

    /** enable/disable fdb upload state. */
    GT_BOOL fdbUploadState;

    /** action active vlan. */
    GT_U16 currentVid;

    /** action active vlan mask. */
    GT_U16 currentVidMask;

    /** action active HW device number. */
    GT_U32 actDev;

    /** ection active device mask. */
    GT_U32 actDevMask;

    /** action active interface (port/trunk). */
    GT_U32 actTrunkPort;

    GT_U32 actTrunkPortMask;

    /** determines if the interface is port or trunk. */
    GT_U32 actIsTrunk;

    /** @brief action active interface (port/trunk) mask.
     *  actIsTrunkMask       - action active trunk mask.
     */
    GT_U32 actIsTrunkMask;

    /** fdb action mode. */
    CPSS_FDB_ACTION_MODE_ENT actionMode;

    /** trigger mode. */
    CPSS_MAC_ACTION_MODE_ENT triggerMode;

    /** action enable state. */
    GT_BOOL actionEnable;

    /** @brief enable/disable sending AU/FU messages to the CPU if the MAC entry
     *  does NOT reside on the local portgroup.
     */
    GT_BOOL maskAuFuMsg2CpuOnNonLocal;

} PRV_CPSS_DXCH_FUQ_ACTION_DATA_STC;

/*******************************************************************************
* prvCpssCatchUpFuncPtr
*
* DESCRIPTION:
*       Define type of functions pointer performing synchronization of sw DB
*       with hw contents.
*
* APPLICABLE DEVICES:
*       Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* INPUTS:
*       devNum - device number.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_FAIL                  - on failure
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*prvCpssCatchUpFuncPtr)(GT_U8 devNum);

/**
* @internal prvCpssDxChIsPortOob function
* @endinternal
*
* @brief   Check if given Port Mac is an OOB Port (On BobK devices only, on other devices output is GT_FALSE).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMac                  - port Mac
*
* @param[out] isPortOobPtr             - (pointer to) is port OOB:
*                                      GT_TRUE - port is OOB.
*                                      GT_FALSE - port is not OOB.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChIsPortOob
(
    IN GT_U8     devNum,
    IN GT_U32    portMac,
    OUT GT_BOOL *isPortOobPtr
);

/**
* @internal prvCpssDxChSystemRecoveryCatchUpHandle function
* @endinternal
*
* @brief   Perform synchronization of hardware data and software DB after special init sequence.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSystemRecoveryCatchUpHandle
(
   GT_VOID
);

/**
* @internal prvCpssDxChHwDevNumCatchUp function
* @endinternal
*
* @brief   Synchronize hw device number in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwDevNumCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChCpuPortModeCatchUp function
* @endinternal
*
* @brief   Synchronize cpu port mode in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCpuPortModeCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChCpuSdmaPortGroupCatchUp function
* @endinternal
*
* @brief   Synchronize cpu sdma port group in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCpuSdmaPortGroupCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChDevTableCatchUp function
* @endinternal
*
* @brief   Synchronize device table BMP in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDevTableCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChFdbHashParamsModeCatchUp function
* @endinternal
*
* @brief   Synchronize fdb hash params in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFdbHashParamsModeCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChFdbActionHwDevNumActionHwDevNumMaskCatchUp function
* @endinternal
*
* @brief   Synchronize Active device number and active device number mask
*         in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFdbActionHwDevNumActionHwDevNumMaskCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChPrePendTwoBytesCatchUp function
* @endinternal
*
* @brief   Synchronize enable/disable pre-pending a two-byte header
*         in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPrePendTwoBytesCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChPolicerMemorySizeModeCatchUp function
* @endinternal
*
* @brief   Synchronize Policer Memory Size Mode in software DB by its hw values.
*
* @note   APPLICABLE DEVICES:      Lion;  Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPolicerMemorySizeModeCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChSecurBreachPortDropCntrModeAndPortGroupCatchUp function
* @endinternal
*
* @brief   Synchronize Secure Breach Port Drop Counter Mode in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSecurBreachPortDropCntrModeAndPortGroupCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChPortEgressCntrModeCatchUp function
* @endinternal
*
* @brief   Synchronize Port Egress Counters Mode in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortEgressCntrModeCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChBridgeIngressCntrModeCatchUp function
* @endinternal
*
* @brief   Synchronize Bridge Ingress Counters Mode in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBridgeIngressCntrModeCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChBridgeIngressDropCntrModeCatchUp function
* @endinternal
*
* @brief   Synchronize Ingress Drop Counters Mode in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBridgeIngressDropCntrModeCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChBridgeTag1VidFdbEnCatchUp function
* @endinternal
*
* @brief   Synchronize tag1VidFdbEn in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBridgeTag1VidFdbEnCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChBridgeEgrVlanPortFltTabAccessModeCatchUp function
* @endinternal
*
* @brief   Synchronize vlanfltTabAccessMode in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBridgeEgrVlanPortFltTabAccessModeCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChPortModeParamsCatchUp function
* @endinternal
*
* @brief   Synchronize Port Mode parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortModeParamsCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChPortResourcesCatchUp function
* @endinternal
*
* @brief   Synchronize Port resources parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortResourcesCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChSerdesPolarityCatchUp function
* @endinternal
*
* @brief   Synchronize Serdes polarity parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; xCat3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSerdesPolarityCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChPortMacCounterOffsetCatchUp function
* @endinternal
*
* @brief   Set port Mac counter offset parameters in software
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortMacCounterOffsetCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChCfgPpLpmLogicalInitCatchUp function
* @endinternal
*
* @brief   Synchronize LPM index of start entry for PBR and LPM memory mode
*         parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCfgPpLpmLogicalInitCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChSharedCncUnitsCatchUp function
* @endinternal
*
* @brief   Synchronize info about the shared CNC units between port groups control
*         in SW DB.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat2; xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSharedCncUnitsCatchUp
(
    IN GT_U8    devNum
);
/**
* @internal prvCpssDxChHaAuqNonContinuousMsgModeHandle function
* @endinternal
*
* @brief   This function performs AUQ workaround after HA event. It makes PP to consider that
*         queue is full and to be ready for reprogramming.
*         The workaround should be used when application can't guarantee the same memory
*         allocated for AUQ before and after HA event.
*         Before calling this function application should disable access CPU memory by device.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHaAuqNonContinuousMsgModeHandle
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChHaFuqNonContinuousMsgModeHandle function
* @endinternal
*
* @brief   This function performs FUQ workaround after HA event. It makes PP to consider that
*         queue is full and to be ready for reprogramming.
*         The workaround should be used when application can't guarantee the same memory
*         allocated for FUQ before and after HA event.
*         Before calling this function application should disable access of device to CPU memory.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHaFuqNonContinuousMsgModeHandle
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChDisableAuqAndSaveAuqCurrentStatus function
* @endinternal
*
* @brief   This function disable AUQ for messages and save current AUQ messages enable status.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDisableAuqAndSaveAuqCurrentStatus
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssDxChRestoreAuqCurrentStatus function
* @endinternal
*
* @brief   This function retieve AUQ enable/disable message status
*         and apply it on the device.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChRestoreAuqCurrentStatus
(
    IN GT_U8    devNum
);


/**
* @internal prvCpssDxChStopAuq function
* @endinternal
*
* @brief   AUQ stop sequence
*
* @note   APPLICABLE DEVICES:      Bobcat2; Catus; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - the port group Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_RESOURCE           - no place to save the request
*/
GT_STATUS prvCpssDxChStopAuq
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);


/**
* @internal prvCpssDxChEnableAuq function
* @endinternal
*
* @brief   Re-enable AUQ
*
* @note   APPLICABLE DEVICES:      Bobcat2; Catus; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - the port group Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_RESOURCE           - no place to save the request
*/
GT_STATUS prvCpssDxChEnableAuq
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHsuh */


