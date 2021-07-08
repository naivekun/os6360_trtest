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
* @file tgfConfigGen.h
*
* @brief Generic Config API
*
* @version   4
********************************************************************************
*/
#ifndef __tgfConfigGenh
#define __tgfConfigGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @struct PRV_TGF_CFG_GLOBAL_EPORT_STC
 *
 * @brief Global ePort range configuration.
*/
typedef struct{

    /** @brief GT_TRUE :enable Global ePort configuration based on
     *  the <pattern> & <mask> fields.
     *  GT_FALSE: no Global ePort configured. HW values of all 0's
     *  and all 1's will be used for <mask> and < pattern>
     *  respectively.
     */
    GT_BOOL enable;

    /** @brief Global ePort range value. All bits which are not set to 1 in
     *  <mask> field must be cleared.
     *  (relevant only if enable == GT_TRUE).
     */
    GT_U32 pattern;

    /** @brief A bitmap that determines which bits in pattern are used for
     *  calculating Global ePort value.
     *  (relevant only if enable == GT_TRUE).
     */
    GT_U32 mask;

} PRV_TGF_CFG_GLOBAL_EPORT_STC;

/**
* @enum PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_ENT
 *
 * @brief Ingress Drop Counter Mode types.
*/
typedef enum{

    /** count all ingress dropped packets. */
    PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E,

    /** @brief Count all ingress dropped packets assigned with
     *  a specific VLAN id.
     */
    PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E,

    /** @brief Count all ingress dropped packets received
     *  on a specific port
     */
    PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E

} PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_ENT;


/**
* @internal prvTgfCfgGlobalEportSet function
* @endinternal
*
* @brief   Set Global ePorts ranges configuration.
*         Global ePorts are ePorts that are global in the entire system, and are
*         not unique per device as regular ePorts are.
*         Global ePorts are used for Representing:
*         1. a Trunk.(map target ePort to trunkId by the E2PHY)
*         2. a ePort ECMP Group.(map 'primary' target ePort to 'secondary' target ePort)
*         3. an ePort Replication Group (eVIDX).(map target ePort to eVIDX group)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] globalPtr                - (pointer to) Global EPorts for the Bridge,L2Mll to use.
*                                      In the Bridge Used for:
*                                      a) Whether the bridge engine looks at the device number
*                                      when performing MAC SA lookup or local switching check.
*                                      b) That the local device ID (own device) is learned in the
*                                      FDB entry with the global ePort
*                                      In the L2MLL Used for:
*                                      Enabling the L2MLL replication engine to ignore the
*                                      device ID when comparing source and target interfaces
*                                      for source filtering.
*                                      Typically configured to include global ePorts
*                                      representing a trunk or an ePort ECMP group
*                                      NOTE: For defining the EPorts that used for 'Multi-Target ePort to eVIDX Mapping'
*                                      use API cpssDxChL2MllMultiTargetPortSet(...)
*                                      (see cpssDxChL2Mll.h file for more related functions)
* @param[in] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
*                                      Used for: The 'Primary ePorts' that need to be converted to
*                                      one of their 'Secondary ePorts'.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgGlobalEportSet
(
    IN GT_U8                          devNum,
    IN  PRV_TGF_CFG_GLOBAL_EPORT_STC  *globalPtr,
    IN  PRV_TGF_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr
);

/**
* @internal prvTgfCfgGlobalEportGet function
* @endinternal
*
* @brief   Get Global ePorts ranges configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] globalPtr                - (pointer to) Global EPorts for the Bridge,L2MLL to use.
* @param[out] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgGlobalEportGet
(
    IN  GT_U8                         devNum,
    OUT PRV_TGF_CFG_GLOBAL_EPORT_STC  *globalPtr,
    OUT PRV_TGF_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr
);

/**
* @internal prvTgfCfgIngressDropCntrModeSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter Mode.
*
* @param[in] mode                     - Ingress Drop Counter Mode.
*                                      port      - port number monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E
*                                      counter mode.
* @param[in] vlan                     - VLAN ID monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E
*                                      counter mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong mode, portNum or vlan.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgIngressDropCntrModeSet
(
    IN  PRV_TGF_CFG_INGRESS_DROP_COUNTER_MODE_ENT   mode,
    IN  GT_PORT_NUM                                 portNum,
    IN  GT_U16                                      vlan
);

/**
* @internal prvTgfCfgIngressDropCntrSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter value.
*
* @param[in] counter                  - Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgIngressDropCntrSet
(
    IN  GT_U32      counter
);

/**
* @internal prvTgfCfgIngressDropCntrGet function
* @endinternal
*
* @brief   Get the Ingress Drop Counter value.
*
* @param[out] counterPtr               - (pointer to) Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgIngressDropCntrGet
(
    OUT GT_U32      *counterPtr
);

/**
* @internal prvTgfCfgPortDefaultSourceEportNumberSet function
* @endinternal
*
* @brief   Set Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ePort                    -  number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Default value is physical port number. In Multi-core
*       architecture it is the global physical port number (i.e. Local Core
*       Port number bitwise ORed with LocalCoreID<<4)
*       2. Can be overriden by TTI Action/RBridge
*
*/
GT_STATUS prvTgfCfgPortDefaultSourceEportNumberSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         ePort
);


/**
* @internal prvTgfCfgPortDefaultSourceEportNumberGet function
* @endinternal
*
* @brief   Get Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ePortPtr                 - pointer to ePort number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Default value is physical port number. In Multi-core
*       architecture it is the global physical port number (i.e. Local Core
*       Port number bitwise ORed with LocalCoreID<<4)
*       2. Can be overriden by TTI Action/RBridge
*
*/
GT_STATUS prvTgfCfgPortDefaultSourceEportNumberGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *ePortPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfConfigGenh */


