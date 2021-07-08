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
* @file tgfLpmGen.h
*
* @brief Generic API for LPM
*
* @version   2
********************************************************************************
*/
#ifndef __tgfLpmGenh
#define __tgfLpmGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum PRV_TGF_LPM_EXCEPTION_STATUS_ENT
 *
 * @brief LPM Exception Status
*/
typedef enum{

    /** Hit: No exceptions */
    PRV_TGF_LPM_EXCEPTION_HIT_E                           = 0,

    /** @brief An non recoverable
     *  ECC error was found in the LPM database
     */
    PRV_TGF_LPM_EXCEPTION_LPM_ECC_E                       = 1,

    /** @brief An non recoverable ECC error was found
     *  in the ECMP database and the leaf is from
     *  ECMP/Qos type
     */
    PRV_TGF_LPM_EXCEPTION_ECMP_ECC_E                      = 2,

    /** @brief Policy Base routing lookup and
     *  the entry fetch from the memory is from type
     *  Bucket (LPM<PointerType> = Bucket)
     */
    PRV_TGF_LPM_EXCEPTION_PBR_BUCKET_E                    = 3,

    /** @brief Unicast lookup IPv4/IPv6/FCoE
     *  that reach a bucket with <PointToLookup1> set
     */
    PRV_TGF_LPM_EXCEPTION_CONTINUE_TO_LOOKUP_E            = 4,

    /** @brief After lookup in LPM stage7
     *  the received LPM<PointerType> is Bucket
     *  (pass the next DIP LPM stages transparently)
     */
    PRV_TGF_LPM_EXCEPTION_UNICAST_LOOKUP_0_E              = 5,

    /** @brief After lookup in LPM stage7
     *  the received LPM<PointerType> is Bucket
     *  (pass the next SIP LPM stages transparently)
     */
    PRV_TGF_LPM_EXCEPTION_UNICAST_LOOKUP_1_E              = 6,

    /** @brief After lookup in LPM stage7
     *  the received LPM<PointerType> is Bucket and
     *  <PointToSIP> is unset (pass the next DIP LPM
     *  stages transparently)
     */
    PRV_TGF_LPM_EXCEPTION_DST_G_IPV4_PACKETS_E            = 7,

    /** @brief After lookup in LPM stage7
     *  the received LPM<PointerType> is Bucket
     *  (pass the next SIP LPM stages transparently)
     */
    PRV_TGF_LPM_EXCEPTION_SRC_G_IPV4_PACKETS_E            = 8,

    /** @brief After lookup in
     *  LPM stage31 the received LPM<PointerType>
     */
    PRV_TGF_LPM_EXCEPTION_UNICAST_LOOKUP_0_IPV6_PACKETS_E = 9,

    /** @brief After lookup in
     *  LPM stage31 the received LPM<PointerType> is Bucket
     */
    PRV_TGF_LPM_EXCEPTION_UNICAST_LOOKUP_1_IPV6_PACKETS_E = 10,

    /** @brief After lookup in LPM stage31
     *  the received LPM<PointerType> is Bucket and
     *  <PointToSIP> is unset (pass the next DIP LPM
     *  stages transparently)
     */
    PRV_TGF_LPM_EXCEPTION_DST_G_IPV6_PACKETS_E            = 11,

    /** @brief After lookup in LPM stage31
     *  the received LPM<PointerType> is Bucket
     *  (pass the next SIP LPM stages transparently)
     */
    PRV_TGF_LPM_EXCEPTION_SRC_G_IPV6_PACKETS_E            = 12,

    /** @brief After lookup in LPM stage5
     *  the received LPM<PointerType> is Bucket
     *  (pass the next DIP LPM stages transparently)
     */
    PRV_TGF_LPM_EXCEPTION_FCOE_D_ID_LOOKUP_E              = 13,

    /** @brief After lookup in LPM stage5 the
     *  received LPM<PointerType> is Bucket
     *  (pass the next SIP LPM stages transparently)
     */
    PRV_TGF_LPM_EXCEPTION_FCOE_S_ID_LOOKUP_E              = 14

} PRV_TGF_LPM_EXCEPTION_STATUS_ENT;

/* number of memories blocks used for IP LPM Management */
#define PRV_TGF_LPM_RAM_MAX_NUM_OF_MEMORY_BLOCKS_CNS 20

/* number of entries in each RAM blocks in lines */
#define PRV_TGF_LPM_RAM_MAX_NUM_OF_ENTRIES_IN_BLOCK_CNS 0x4000

/**
* @internal prvTgfLpmLastLookupStagesBypassEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Set the enabling status of bypassing lookup stages 8-31.
*         Bypsdding the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
* @param[in] devNum                   - the device number
* @param[in] bypassEnabled            - the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS prvTgfLpmLastLookupStagesBypassEnableSet
(
    IN GT_U8                                    devNum,
    IN GT_BOOL                                  bypassEnabled
);

/**
* @internal prvTgfLpmLastLookupStagesBypassEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Get the enabling status of bypassing lookup stages 8-31.
*         Bypsdding the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
* @param[in] devNum                   - the device number
*
* @param[out] bypassEnabledPtr         - (pointer to) the bypass enabling status:
*                                      GT_TRUE  - enable bypassing of lookup stages 8-31
*                                      GT_FALSE - disable bypassing of lookup stages 8-31
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS prvTgfLpmLastLookupStagesBypassEnableGet
(
    IN  GT_U8                                   devNum,
    OUT GT_BOOL                                 *bypassEnabledPtr
);

/**
* @internal prvTgfLpmPortSipLookupEnableSet function
* @endinternal
*
* @brief   Enable/Disable SIP lookup for packets received from the given port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
* @param[in] enable                   - GT_TRUE:    SIP Lookup on the port
*                                      GT_FALSE:  disable SIP Lookup on the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_BAD_STATE             - on invalid hardware value set
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When this attribute is set to "False", SIP Lookup is never performed for
*       the source ePort, regardless of other SIP related configurations
*       (i.e SIP/SA check, SIP filtering etc')
*
*/
GT_STATUS prvTgfLpmPortSipLookupEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
);

/**
* @internal prvTgfLpmPortSipLookupEnableGet function
* @endinternal
*
* @brief   Return the SIP Lookup status for packets received from the given port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE:   enable SIP Lookup on the port
*                                      GT_FALSE:  disable SIP Lookup on the port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note When this attribute is set to "False", SIP Lookup is never performed for
*       the source ePort, regardless of other SIP related configurations
*       (i.e SIP/SA check, SIP filtering etc')
*
*/
GT_STATUS prvTgfLpmPortSipLookupEnableGet
(
   IN  GT_U8                        devNum,
   IN  GT_PORT_NUM                  portNum,
   OUT GT_BOOL                      *enablePtr
);

/**
* @internal prvTgfLpmExceptionStatusGet function
* @endinternal
*
* @brief   Return the Lpm Exception Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmEngine                - LPM engine index (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon )(APPLICABLE RANGES: 0..3)
*
* @param[out] hitExceptionPtr          - (pointer to) hit Exception happened in LPM Stages
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_STATE             - wrong value returned from hw
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/

GT_STATUS prvTgfLpmExceptionStatusGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            lpmEngine,
    OUT PRV_TGF_LPM_EXCEPTION_STATUS_ENT  *hitExceptionPtr
);
/**
* @internal prvTgfLpmEngineGet function
* @endinternal
*
* @brief   Get LPM engine
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - port number
* @param[in] isSipLookup              - GT_TRUE if this is SIP lookup,GT_FALSE otherwise
*
* @param[out] lpmEnginePtr             - lpm engine number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_VALUE             - failure to calculate lpm engine
*/

GT_STATUS prvTgfLpmEngineGet
(
   IN  GT_U8                        devNum,
   IN  GT_PORT_NUM                  portNum,
   IN  GT_BOOL                      isSipLookup,
   OUT GT_U32                        *lpmEnginePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfLpmGenh */


