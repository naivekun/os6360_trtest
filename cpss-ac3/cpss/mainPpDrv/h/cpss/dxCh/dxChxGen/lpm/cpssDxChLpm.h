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
* @file cpssDxChLpm.h
*
* @brief LPM APIs
*
* @version   8
********************************************************************************
*/

#ifndef __cpssDxChLpmh
#define __cpssDxChLpmh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>

/**
* @enum CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT
 *
 * @brief LPM Exception Status
*/
typedef enum{

    /** Hit: No exceptions */
    CPSS_DXCH_LPM_EXCEPTION_HIT_E                           = 0,

    /** @brief An non recoverable
     *  ECC error was found in the LPM database
     */
    CPSS_DXCH_LPM_EXCEPTION_LPM_ECC_E                       = 1,

    /** @brief An non recoverable ECC error was found
     *  in the ECMP database and the leaf is from
     *  ECMP/Qos type
     */
    CPSS_DXCH_LPM_EXCEPTION_ECMP_ECC_E                      = 2,

    /** @brief Policy Base routing lookup and
     *  the entry fetch from the memory is from type
     *  Bucket (LPM<PointerType> = Bucket)
     */
    CPSS_DXCH_LPM_EXCEPTION_PBR_BUCKET_E                    = 3,

    /** @brief Unicast lookup IPv4/IPv6/FCoE
     *  that reach a bucket with <PointToLookup1> set
     */
    CPSS_DXCH_LPM_EXCEPTION_CONTINUE_TO_LOOKUP_E            = 4,

    /** @brief After lookup in LPM stage7
     *  the received LPM<PointerType> is Bucket
     *  (pass the next DIP LPM stages transparently)
     */
    CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_0_E              = 5,

    /** @brief After lookup in LPM stage7
     *  the received LPM<PointerType> is Bucket
     *  (pass the next SIP LPM stages transparently)
     */
    CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_1_E              = 6,

    /** @brief After lookup in LPM stage7
     *  the received LPM<PointerType> is Bucket and
     *  <PointToSIP> is unset (pass the next DIP LPM
     *  stages transparently)
     */
    CPSS_DXCH_LPM_EXCEPTION_DST_G_IPV4_PACKETS_E            = 7,

    /** @brief After lookup in LPM stage7
     *  the received LPM<PointerType> is Bucket
     *  (pass the next SIP LPM stages transparently)
     */
    CPSS_DXCH_LPM_EXCEPTION_SRC_G_IPV4_PACKETS_E            = 8,

    /** @brief After lookup in
     *  LPM stage31 the received LPM<PointerType>
     */
    CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_0_IPV6_PACKETS_E = 9,

    /** @brief After lookup in
     *  LPM stage31 the received LPM<PointerType> is Bucket
     */
    CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_1_IPV6_PACKETS_E = 10,

    /** @brief After lookup in LPM stage31
     *  the received LPM<PointerType> is Bucket and
     *  <PointToSIP> is unset
     */
    CPSS_DXCH_LPM_EXCEPTION_DST_G_IPV6_PACKETS_E            = 11,

    /** @brief After lookup in LPM stage31
     *  the received LPM<PointerType> is Bucket
     */
    CPSS_DXCH_LPM_EXCEPTION_SRC_G_IPV6_PACKETS_E            = 12,

    /** @brief After lookup in LPM stage5
     *  the received LPM<PointerType> is Bucket
     *  (pass the next DIP LPM stages transparently)
     */
    CPSS_DXCH_LPM_EXCEPTION_FCOE_D_ID_LOOKUP_E              = 13,

    /** @brief After lookup in LPM stage5 the
     *  received LPM<PointerType> is Bucket
     *  (pass the next SIP LPM stages transparently)
     */
    CPSS_DXCH_LPM_EXCEPTION_FCOE_S_ID_LOOKUP_E              = 14

} CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT;

/**
* @internal cpssDxChLpmNodeWrite function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node (Bucket) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmLineOffset            - offset from the LPM base in LPM lines
* @param[in] nodeType                 - type of node (bucket)
* @param[in] rangeSelectSecPtr        - the range selection (bitmap) section
* @param[in] numOfRanges              - number of next pointers in nextPointerArrayPtr
* @param[in] nextPointerArrayPtr      - next pointers array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmNodeWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   lpmLineOffset,
    IN CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      nodeType,
    IN CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT   *rangeSelectSecPtr,
    IN GT_U32                                   numOfRanges,
    IN CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC      *nextPointerArrayPtr
);

/*******************************************************************************
* cpssDxChLpmNodeRead
*
* @brief   Function Relevant mode : Low Level API modes
*         Read an LPM Node (Bucket) from the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmLineOffset            - offset from the LPM base in LPM lines
* @param[in] nodeType                 - type of node (bucket)
*
* @param[out] rangeSelectSecPtr        - the range selection (bitmap) section
* @param[out] numOfRangesPtr           - number of next pointers in nextPointerArrayPtr
* @param[out] nextPointerArrayPtr      - next pointers array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmNodeRead
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  lpmLineOffset,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     nodeType,
    OUT CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT  *rangeSelectSecPtr,
    OUT GT_U32                                  *numOfRangesPtr,
    OUT CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC     *nextPointerArrayPtr
);

/**
* @internal cpssDxChLpmNodeNextPointersWrite function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node's (Bucket) Next Pointer entry (range) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] lpmLineOffset            - offset from the LPM base in LPM lines
* @param[in] numOfNextPointers        - number of next pointers in nextPointerArrayPtr
* @param[in] nextPointerArrayPtr      - next pointers array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmNodeNextPointersWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   lpmLineOffset,
    IN GT_U32                                   numOfNextPointers,
    IN CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC      *nextPointerArrayPtr
);

/**
* @internal cpssDxChLpmLastLookupStagesBypassEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Set the enabling status of bypassing lookup stages 8-31.
*         bypassing the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
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
GT_STATUS cpssDxChLpmLastLookupStagesBypassEnableSet
(
    IN GT_U8                                    devNum,
    IN GT_BOOL                                  bypassEnabled
);

/**
* @internal cpssDxChLpmLastLookupStagesBypassEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Get the enabling status of bypassing lookup stages 8-31.
*         bypassing the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
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
GT_STATUS cpssDxChLpmLastLookupStagesBypassEnableGet
(
    IN  GT_U8                                   devNum,
    OUT GT_BOOL                                 *bypassEnabledPtr
);



/**
* @internal cpssDxChLpmPortSipLookupEnableSet function
* @endinternal
*
* @brief   Enable/Disable SIP lookup for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
GT_STATUS cpssDxChLpmPortSipLookupEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
);

/**
* @internal cpssDxChLpmPortSipLookupEnableGet function
* @endinternal
*
* @brief   Return the SIP Lookup status for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
GT_STATUS cpssDxChLpmPortSipLookupEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal cpssDxChLpmLeafEntryWrite function
* @endinternal
*
* @brief   Write an LPM leaf entry, which is used for policy based routing, to the
*         HW
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
* @param[in] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
GT_STATUS cpssDxChLpmLeafEntryWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   leafIndex,
    IN CPSS_DXCH_LPM_LEAF_ENTRY_STC             *leafPtr
);

/**
* @internal cpssDxChLpmLeafEntryRead function
* @endinternal
*
* @brief   Read an LPM leaf entry, which is used for policy based routing, from the
*         HW
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
*
* @param[out] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
GT_STATUS cpssDxChLpmLeafEntryRead
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  leafIndex,
    OUT CPSS_DXCH_LPM_LEAF_ENTRY_STC            *leafPtr
);

/**
* @internal cpssDxChLpmPbrBaseAndSizeGet function
* @endinternal
*
* @brief   Return the base address and size of the policy based routing area within
*         the LPM memory
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] baseOffsetPtr            - (pointer to) PBR base offset within the LPM memory
* @param[out] sizePtr                  - (pointer to) number of PBR leaf entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS cpssDxChLpmPbrBaseAndSizeGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *baseOffsetPtr,
    OUT GT_U32  *sizePtr
);

/**
* @internal cpssDxChLpmExceptionStatusGet function
* @endinternal
*
* @brief   Return the Lpm Exception Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number 
* @param[in] lpmEngine                - LPM engine index 
*       (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon )
*       (APPLICABLE RANGES: Bobcat3: 0..3, Aldrin2: 0..1, Falcon: 0..7) 
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

GT_STATUS cpssDxChLpmExceptionStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              lpmEngine,
    OUT CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT  *hitExceptionPtr
);

/**
* @internal cpssDxChLpmMemoryModeGet function
* @endinternal
*
* @brief   This function gets LPM memory mode
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; Lion2.
*
* @param[in] devNum                   -  the device number
*                                      lpmMemMode           -  lpm memory mode
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_NOT_SUPPORTED         - in case DFX unit is not supported
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS cpssDxChLpmMemoryModeGet
(
    IN    GT_U8                                     devNum,
    OUT    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT       *lpmMemoryModePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChLpmh */


