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
* @file prvCpssDxCh3Policer.h
*
* @brief Private CPSS DxCh3 Policing Engine declarations.
*
* @version   4
********************************************************************************
*/
#ifndef __prvCpssDxCh3Policerh
#define __prvCpssDxCh3Policerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxCh3/policer/cpssDxCh3Policer.h>

/* Check Policer stage number, for DxCh3 it is always 0 */
#define PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(_devNum, _stage)                                  \
if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(_devNum))                                       \
{                                                                                       \
    switch ((_stage))                                                                   \
    {                                                                                   \
        case CPSS_DXCH_POLICER_STAGE_INGRESS_0_E:                                       \
            if (PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.policersNum == 0)   \
            {                                                                           \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                    \
            }                                                                           \
            break;                                                                      \
        case CPSS_DXCH_POLICER_STAGE_INGRESS_1_E:                                       \
            if (PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.featureInfo.iplrSecondStageSupported == GT_FALSE) \
            {                                                                           \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                    \
            }                                                                           \
            break;                                                                      \
        case CPSS_DXCH_POLICER_STAGE_EGRESS_E:                                          \
            if (PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.egressPolicersNum == 0) \
            {                                                                           \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                    \
            }                                                                           \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                        \
    }                                                                                   \
}                                                                                       \
else                                                                                    \
    (_stage) = 0; /* the CH3 ignores stage, use stage 0 */


/**
* @enum PRV_CPSS_DXCH3_POLICER_ACCESS_CTRL_CMD_ENT
 *
 * @brief Enumeration for Type of triggered action that the Policer
 * needs to perform. It using upon accessing the metering and
 * counting table entries for read, write and update operations.
*/
typedef enum{

    /** @brief The counter entry will be
     *  read to the IPLR Table
     *  Access Data Reg. Then reset
     *  and written back to memory.
     */
    PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_RESET_E = 0,

    /** @brief The counter entry will be
     *  read to the IPLR Table
     *  Access Data Register. No
     *  write back is performed.
     */
    PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E      = 1,

    /** @brief Flush the Counter
     *  writeback buffer.
     *  NOTE: in sip5.20 not in same register as others
     */
    PRV_CPSS_DXCH3_POLICER_CNTR_WBF_FLUSH_E      = 2,

    /** @brief Sets metering entry fields,
     *  according to the information
     *  placed in the IPLR Table
     *  Access Data Register.
     */
    PRV_CPSS_DXCH3_POLICER_METERING_UPDATE_E     = 4,

    /** @brief Refresh the metering entry
     *  status fields.
     */
    PRV_CPSS_DXCH3_POLICER_METERING_REFRESH_E    = 5,

    /** @brief Management counters
     *  are read to the Policer Table Access Data Registers
     */
    PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_READ_E = 8,

    /** @brief Management counters
     *  are reset
     */
    PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_RESET_E = 9,

    /** @brief Management counters
     *  are read to the Policer Table Access Data Registers , and Management counters
     *  are reset
     */
    PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_READ_AND_RESET_E = 10,

    /** @brief The counter entry will be
     *  loaded to the PLR Table
     *  Access Data Regs.
     *  And content of PLR Table Access
     *  Data Regs is written to memory.
     *  Supported for Lion and above
     *  devices.
     */
    PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E = 0x40,

    /** @brief The counter entry will be written
     *  from the IPLR Table Access Data Reg.
     *  NOTE: in sip5.20 added instead of 'WBF_FLUSH'
     *  NOTE: code support 8 bits to allow value of 0x102 to become 0x2
     */
    PRV_CPSS_SIP_5_20_POLICER_CNTR_WRITE_E       = 0x102

} PRV_CPSS_DXCH3_POLICER_ACCESS_CTRL_CMD_ENT;

/* ====== Envelopes support (MEF10.3) ===== */

/* maximal size of MEF 10.3 Envelope */
#define PRV_CPSS_DXCH_POLICER_MAX_ENVELOPE_SIZE_CNS 8

/**
* @enum PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_ENT
 *
 * @brief Enumeration for Bucket Color in Bucket Envelope.
*/
typedef enum{

    /** Green Bucket. */
    PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E,

    /** Yellow Bucket. */
    PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_YELLOW_E,

} PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_ENT;

/**
* @struct PRV_CPSS_DXCH_POLICER_ENVELOPE_MEMBER_CFG_STC
 *
 * @brief Structure for MEF 10.3 backet pair parameters in the
 * Policer Metering Entry, related only to MEF1.3.
 * (APPLICABLE DEVICES: Bobcat2).
*/
typedef struct{

    /** @brief amount entries in MEF 10.3 Envelope.
     *  the same value should be configured in all entries of envelope
     *  (APPLICABLE RANGES: 1..8)
     */
    GT_U32 envelopeSize;

    /** @brief rank of bucket0 in the Envelope
     *  (APPLICABLE RANGES: 1..8)
     */
    GT_U32 bucket0Rank;

    /** @brief rank of bucket1 in the Envelope
     *  (APPLICABLE RANGES: 1..8)
     */
    GT_U32 bucket1Rank;

    /** color of bucket0 in the Envelope */
    PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_ENT bucket0Color;

    /** color of bucket1 in the Envelope */
    PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_ENT bucket1Color;

    /** @brief The coupling flag selects between these options.
     *  GT_FALSE = SPILL TO NEXT: The extra tokens of the CIR bucket
     *  are spilled to the CIR bucket of the next rank.
     *  GT_TRUE = SPILL TO SAME: The extra tokens of the CIR bucket
     *  are spilled to the EIR bucket of the same rank.
     */
    GT_BOOL couplingFlag;

    /** @brief in MEF 10.3, the CIR bucket of the bottom rank in the
     *  MEF 10.3 envelope may either spill the overflow to
     *  the EIR bucket of the highest rank or not, according
     *  to coupling flag 0.
     *  GT_FALSE = DONT SPILL: The extra tokens of the CIR bucket
     *  are not spilled to the EIR bucket.
     *  GT_TRUE = SPILL: The extra tokens of the CIR bucket
     *  are spilled to the EIR bucket.
     */
    GT_BOOL couplingFlag0;

    /** @brief index in Max Rate table entry used to limit the actual rate
     *  of backets0-1.
     *  (APPLICABLE RANGES: 0..127)
     */
    GT_U32 maxRateIndex;

} PRV_CPSS_DXCH_POLICER_ENVELOPE_MEMBER_CFG_STC;


/**
* @internal prvCpssDxCh3PolicerInternalTableAccess function
* @endinternal
*
* @brief   Triggers access operation in policer internal table.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupId              - the portGroupId. to support multi-port-groups device-port-groups device
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
* @param[in] entryOffset              - The offset from the base address of the
*                                      accessed entry.
* @param[in] triggerCmd               - Triggered command that the Policer needs to perform.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
*/
GT_STATUS prvCpssDxCh3PolicerInternalTableAccess
(
    IN GT_U8                                        devNum,
    IN  GT_U32                                      portGroupId,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN GT_U32                                       entryOffset,
    IN PRV_CPSS_DXCH3_POLICER_ACCESS_CTRL_CMD_ENT   triggerCmd
);

/**
* @internal prvCpssDxCh3PolicerAccessControlBusyWait function
* @endinternal
*
* @brief   Do busy wait on Policer Access Control Register.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
*/
GT_STATUS prvCpssDxCh3PolicerAccessControlBusyWait
(
    IN GT_U8                                        devNum,
    IN GT_PORT_GROUPS_BMP                           portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage
);

/**
* @internal prvCpssPolicerIngressRamIndexInfoGet function
* @endinternal
*
* @brief   Function returns Policer metering,counting RAM info
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - PP device number
*
* @param[out] numEntriesRam0Ptr        - (pointer to) number of entries in RAM 0.
* @param[out] numEntriesRam1Ptr        - (pointer to) number of entries in RAM 1.
* @param[out] numEntriesRam2Ptr        - (pointer to) number of entries in RAM 2.
* @param[out] iplr0StartRamPtr         - (pointer to) the stating RAM index that IPLR_stage_0 using.
* @param[out] iplr0NumRamsUsedPtr      - (pointer to) the number of consecutive RAMs that the IPLR_stage_0 using.
*                                      NOTE: value is 0..3
* @param[out] iplr1StartRamPtr         - (pointer to) the stating RAM index that IPLR_stage_1 using.
* @param[out] iplr1NumRamsUsedPtr      - (pointer to) the number of consecutive RAMs that the IPLR_stage_1 using.
*                                      NOTE: value is 3 - iplr0
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPolicerIngressRamIndexInfoGet
(
    IN  GT_U8                               devNum,
    OUT  GT_U32                             *numEntriesRam0Ptr,
    OUT  GT_U32                             *numEntriesRam1Ptr,
    OUT  GT_U32                             *numEntriesRam2Ptr,
    OUT  GT_U32                             *iplr0StartRamPtr,
    OUT  GT_U32                             *iplr0NumRamsUsedPtr,
    OUT  GT_U32                             *iplr1StartRamPtr,
    OUT  GT_U32                             *iplr1NumRamsUsedPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxCh3Policerh */

