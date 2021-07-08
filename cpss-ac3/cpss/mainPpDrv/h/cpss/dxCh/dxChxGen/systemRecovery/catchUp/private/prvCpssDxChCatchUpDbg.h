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
* @file prvCpssDxChCatchUpDbg.h
*
* @brief CPSS DxCh CatchUp debug utilities
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChCatchUpDbgh
#define __prvCpssDxChCatchUpDbgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/generic/bridge/cpssGenBrgSecurityBreachTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUp.h>

/* Dummy value to indicate that we are during catchup validity check. Used for debug purpose to
   verify that cathcup changed the value of this variable */
#define PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS       0xABCDABCD
#define PRV_CPSS_DXCH_CATCHUP_8_BIT_PATTERN_CNS 0xCD

#ifdef ASIC_SIMULATION
    #define PRV_CPSS_DXCH_CATCHUP_DBG_IS_DEV_NOT_BOBK_OR_ALDRIN_SIMULATION_CHECK_MAC(_devNum)                  \
       (!((PRV_CPSS_PP_MAC(devNum)->appDevFamily) & (CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E)))
#else
    #define PRV_CPSS_DXCH_CATCHUP_DBG_IS_DEV_NOT_BOBK_OR_ALDRIN_SIMULATION_CHECK_MAC(_devNum) GT_TRUE
#endif

typedef struct
{
    GT_HW_DEV_NUM                                   hwDevNum;
    CPSS_NET_CPU_PORT_MODE_ENT                      cpuPortMode;
    CPSS_MAC_VL_ENT                                 vlanMode;
    CPSS_MAC_HASH_FUNC_MODE_ENT                     hashMode;
    GT_U32                                          actionHwDevNum;
    GT_U32                                          actionHwDevNumMask;
    GT_BOOL                                         prePendTwoBytesHeader;
    GT_U32                                          policerInfoMemSize[2];
    CPSS_BRG_SECUR_BREACH_DROP_COUNT_MODE_ENT       securBreachDropCounterInfoCounterMode;
    GT_U32                                          securBreachDropCounterInfoPortGroupId;
    GT_U32                                          portEgressCntrModeInfoPortGroupId[2];
    GT_U32                                          bridgeIngressCntrModePortGroupId[2];
    GT_U32                                          cfgIngressDropCntrModePortGroupId;
    CPSS_PORT_INTERFACE_MODE_ENT                    portIfMode[CPSS_MAX_PORTS_NUM_CNS];
    CPSS_PORT_SPEED_ENT                             portSpeed[CPSS_MAX_PORTS_NUM_CNS];
    PRV_CPSS_PORT_TYPE_ENT                          portType[CPSS_MAX_PORTS_NUM_CNS];
    GT_U8                                          *portGtMacCounterOffset;

    /* Lion, Lion2 */
    GT_U32                                          netifSdmaPortGroupId;

    /* xCat, xCat2, xCat3, Lion, Lion2 */
    GT_U32                                          devTable;

    /* SIP 5 */
    GT_BOOL                                         tag1VidFdbEn;
    CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT portVlanfltTabAccessMode;
    GT_U32                                          usedDescCredits[MAX_DP_CNS];
    GT_U32                                          usedHeaderCredits[MAX_DP_CNS];
    GT_U32                                          usedPayloadCredits[MAX_DP_CNS];
    GT_U32                                          lpmIndexForPbr;

    /* SIP 5.15 */
    GT_U32                                          coreOverallSpeedSummary[MAX_DP_CNS];
    GT_U32                                          coreOverallSpeedSummaryTemp[MAX_DP_CNS];
    PRV_DXCH_PORT_SERDES_POLARITY_CONFIG_STC       *serdesPolarityPtr;
    GT_BOOL                                        *isSerdesPolarityLaneNumValidPtr;
    GT_BOOL                                         isValidityCheckEnabled;

    /* SIP 5.20 */
    PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT              lpmMemMode;
} PRV_CPSS_DXCH_CATCHUP_PARAMS_STC;

PRV_CPSS_DXCH_CATCHUP_PARAMS_STC ppConfigCatchUpParams[PRV_CPSS_MAX_PP_DEVICES_CNS];

/**
* @internal prvCpssDxChSystemRecoveryCatchUpValidityCheckEnable function
* @endinternal
*
* @brief   Enable CatchUp validity check
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note Pre-condition for using this function:
*       CatchUp parameters in PP data structure must be equal to the HW values
*
*/
GT_STATUS prvCpssDxChSystemRecoveryCatchUpValidityCheckEnable
(
    IN  GT_U8   devNum
);

/**
* @internal prvCpssDxChSystemRecoveryCatchUpValidityCheck function
* @endinternal
*
* @brief   Validates that SW params hold the correct value from HW after CatchUp
*         process.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; xCat; xCat3; xCat2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note To use this API, cpssDxChCatchUpValidityCheckEnable must be called before
*       performing CatchUp
*
*/
GT_STATUS prvCpssDxChSystemRecoveryCatchUpValidityCheck
(
    IN  GT_U8   devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChCatchUpDbgh */


