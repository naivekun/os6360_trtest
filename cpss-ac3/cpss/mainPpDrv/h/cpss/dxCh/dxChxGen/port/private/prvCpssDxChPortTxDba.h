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
* @file prvCpssDxChPortTxDba.h
*
* @brief Includes types and values definition and initialization for the use of
* CPSS DxCh DBA feature.
*
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChPortTxDbah
#define __prvCpssDxChPortTxDbah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef DBA_FW
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/port/cpssPortTx.h>
#endif /*DBA_FW*/

/**
* @enum PRV_CPSS_DXCH_DBA_OP_CODE_ENT
 *
 * @brief OP codes for messages towards DBA feature.
*/
typedef enum{

    /** Reserved */
    PRV_CPSS_DXCH_DBA_OP_CODE_RESERVED_1_E = 1,

    /** Global Enable\Disable DBA and Enable\Disable gathering statistics. */
    PRV_CPSS_DXCH_DBA_OP_CODE_CONTROL_E,

    /** Reserved */
    PRV_CPSS_DXCH_DBA_OP_CODE_RESERVED_3_E,

    /** Reserved */
    PRV_CPSS_DXCH_DBA_OP_CODE_RESERVED_4_E,

    /** Clear free buffers statistics. */
    PRV_CPSS_DXCH_DBA_OP_CODE_CLEAR_GLOBAL_COUNTERS_E,

    /** Get free buffers statistics. */
    PRV_CPSS_DXCH_DBA_OP_CODE_GET_GLOBAL_COUNTERS_E,

    /** Configure on which <profile,dp,queue> triplet to gather statistics. */
    PRV_CPSS_DXCH_DBA_OP_CODE_ENABLE_PROFILE_COUNTERS_E,

    /** Get the <profile,dp,queue> triplet statistics. */
    PRV_CPSS_DXCH_DBA_OP_CODE_GET_PROFILE_COUNTERS_E,

    /** Set maximal buffers available. */
    PRV_CPSS_DXCH_DBA_OP_CODE_SET_AVAIL_BUFFERS_E,

    /** Get maximal buffers available. */
    PRV_CPSS_DXCH_DBA_OP_CODE_GET_AVAIL_BUFFERS_E,

    /** Reserved */
    PRV_CPSS_DXCH_DBA_OP_CODE_RESERVED_11_E,

    /** Reserved */
    PRV_CPSS_DXCH_DBA_OP_CODE_RESERVED_12_E,

    /** @brief Configure DP0, DP1 & DP2 gaurantee & weight values for specified
     *  <profile, queue>.
     */
    PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_SET_E,

    /** @brief Get DP0, DP1 & DP2 gaurantee & weight values for specified
     *  <profile, queue>.
     */
    PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_GET_E,

    /** @brief Indirect write access to the switch core TxQ tables which require
     *  the special WA.
     */
    PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_PROFILE_TABLE_SET_E,

    /** @brief Indirect read access to the switch core TxQ tables which require
     *  the special WA.
     */
    PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_PROFILE_TABLE_GET_E,

    PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_OTHER_SET_E,

    /** @brief Indirect read access to the switch core TxQ beside the tables which
     *  require the special WA.
     */
    PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_OTHER_GET_E

} PRV_CPSS_DXCH_DBA_OP_CODE_ENT;

/* Message length definitions */
#define PRV_CPSS_DXCH_DBA_OP_CODE_RESERVED_1_MSG_LENGTH 1
#define PRV_CPSS_DXCH_DBA_OP_CODE_CONTROL_MSG_LENGTH 3
#define PRV_CPSS_DXCH_DBA_OP_CODE_CLEAR_GLOBAL_COUNTERS_MSG_LENGTH 1
#define PRV_CPSS_DXCH_DBA_OP_CODE_GET_GLOBAL_COUNTERS_MSG_LENGTH 1
#define PRV_CPSS_DXCH_DBA_OP_CODE_ENABLE_PROFILE_COUNTERS_MSG_LENGTH 4
#define PRV_CPSS_DXCH_DBA_OP_CODE_GET_PROFILE_COUNTERS_MSG_LENGTH 1
#define PRV_CPSS_DXCH_DBA_OP_CODE_SET_AVAIL_BUFFERS_MSG_LENGTH 2
#define PRV_CPSS_DXCH_DBA_OP_CODE_GET_AVAIL_BUFFERS_MSG_LENGTH 1
#define PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_SET_MSG_LENGTH 9
#define PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_GET_MSG_LENGTH 3

#define DBA_TXQ_PROXY_MAX_DATA_ACCESS_LENGTH 4
#define PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_SET_MAX_MSG_LENGTH \
                                    (DBA_TXQ_PROXY_MAX_DATA_ACCESS_LENGTH + 3)
#define PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_GET_MSG_LENGTH 3


/* Return message length definitions */
#define PRV_CPSS_DXCH_DBA_OP_CODE_RESERVED_1_RET_MSG_LENGTH 0
#define PRV_CPSS_DXCH_DBA_OP_CODE_CONTROL_RET_MSG_LENGTH 0
#define PRV_CPSS_DXCH_DBA_OP_CODE_CLEAR_GLOBAL_COUNTERS_RET_MSG_LENGTH 0
#define PRV_CPSS_DXCH_DBA_OP_CODE_GET_GLOBAL_COUNTERS_RET_MSG_LENGTH 4
#define PRV_CPSS_DXCH_DBA_OP_CODE_ENABLE_PROFILE_COUNTERS_RET_MSG_LENGTH 0
#define PRV_CPSS_DXCH_DBA_OP_CODE_GET_PROFILE_COUNTERS_RET_MSG_LENGTH 4
#define PRV_CPSS_DXCH_DBA_OP_CODE_SET_AVAIL_BUFFERS_RET_MSG_LENGTH 0
#define PRV_CPSS_DXCH_DBA_OP_CODE_GET_AVAIL_BUFFERS_RET_MSG_LENGTH 2
#define PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_SET_RET_MSG_LENGTH 0
#define PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_GET_RET_MSG_LENGTH 9

#define PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_SET_RET_MSG_LENGTH 0
#define PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_GET_MAX_RET_MSG_LENGTH \
                                    (DBA_TXQ_PROXY_MAX_DATA_ACCESS_LENGTH + 3)

/**
* @enum PRV_CPSS_DXCH_DBA_RC_CODE_ENT
 *
 * @brief Return code due to messages towards DBA feature.
*/
typedef enum{

    /** operation passed ok */
    PRV_CPSS_DXCH_DBA_RC_CODE_OK_E,

    /** generic failure */
    PRV_CPSS_DXCH_DBA_RC_CODE_FAIL_E,

    /** unrecognized op code */
    PRV_CPSS_DXCH_DBA_RC_CODE_INVALID_OP_CODE_E,

    /** wrong message parameters */
    PRV_CPSS_DXCH_DBA_RC_CODE_INVALID_IN_PARAMS_E,

    /** wrong message size */
    PRV_CPSS_DXCH_DBA_RC_CODE_INVALID_IN_SIZE_E

} PRV_CPSS_DXCH_DBA_RC_CODE_ENT;

/* Offset of shared memory (where the message data is expected to be written) */
/* from the CM3 SRAM end.*/
#define PRV_CPSS_DXCH_DBA_SHM_OFFSET_FROM_SRAM_END 0x1000

#define PRV_CPSS_DXCH_DBA_NUMBER_OF_TX_PROFILES 2

/* Convert CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT to HW value */
#define PRV_CPSS_PORT_TX_TAIL_DROP_SIP6_ALPHA_TO_HW_CONVERT_MAC(alpha, hwValue) \
    switch (alpha)                                                             \
    {                                                                          \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                             \
            hwValue = 0x0;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E:                         \
            hwValue = 0x3;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                          \
            hwValue = 0x4;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                           \
            hwValue = 0x5;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                             \
            hwValue = 0x6;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                             \
            hwValue = 0x7;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                             \
            hwValue = 0x8;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E:                       \
            hwValue = 0x1;                                                     \
            break;                                                             \
       case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E:                         \
            hwValue = 0x2;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E:                             \
            hwValue = 0x9;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E:                            \
            hwValue = 0xa;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E:                            \
            hwValue = 0xb;                                                     \
            break;                                                             \
        default:                                                               \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);     \
     }

    /* Convert  HW to  CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT value */
#define PRV_CPSS_PORT_TX_TAIL_DROP_SIP6_HW_TO_ALPHA_CONVERT_MAC(alpha, hwValue) \
    switch (hwValue)                                                             \
    {                                                                          \
        case 0x0:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;                      \
            break;                                                             \
        case 0x3:                                                              \
            alpha =  CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E;                 \
            break;                                                             \
        case 0x4 :                                                             \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E;                   \
            break;                                                             \
        case 0x5:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E;                    \
            break;                                                             \
        case 0x6:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;                      \
            break;                                                             \
        case 0x7:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E;                      \
            break;                                                             \
        case 0x8:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;                      \
            break;                                                             \
        case 0x1 :                                                             \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E;                \
            break;                                                             \
       case 0x2 :                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E;                 \
            break;                                                             \
        case 0x9 :                                                             \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E;                      \
            break;                                                             \
        case 0xa :                                                             \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E;                     \
            break;                                                             \
        case 0xb :                                                             \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E;                     \
            break;                                                             \
        default:                                                               \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);     \
     }

/* Convert CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT to HW value */
#define PRV_CPSS_PORT_TX_TAIL_DROP_QCN_DBA_ALPHA_TO_HW_CONVERT_MAC(alpha, hwValue) \
    switch (alpha)                                                             \
    {                                                                          \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                             \
            hwValue = 0x0;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E:                         \
            if(PRV_CPSS_PP_MAC(devNum)->devFamily ==                           \
                CPSS_PP_FAMILY_DXCH_ALDRIN2_E)                                 \
            {                                                                  \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG); \
            }                                                                  \
            hwValue = 0x6;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                          \
            hwValue = 0x1;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                           \
            hwValue = 0x2;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                             \
            hwValue = 0x3;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                             \
            hwValue = 0x4;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                             \
            hwValue = 0x5;                                                     \
            break;                                                             \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E:                             \
            if(PRV_CPSS_PP_MAC(devNum)->devFamily ==                           \
                CPSS_PP_FAMILY_DXCH_ALDRIN2_E)                                 \
            {                                                                  \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG); \
            }                                                                  \
            hwValue = 0x7;                                                     \
            break;                                                             \
        default:                                                               \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);     \
    }

/* Convert HW value to CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT */
#define PRV_CPSS_PORT_TX_TAIL_DROP_QCN_DBA_HW_TO_ALPHA_CONVERT_MAC(hwValue, alpha) \
    switch (hwValue)                                                           \
    {                                                                          \
        case 0x0:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;                      \
            break;                                                             \
        case 0x6:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E;                  \
            break;                                                             \
        case 0x1:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E;                   \
            break;                                                             \
        case 0x2:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E;                    \
            break;                                                             \
        case 0x3:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;                      \
            break;                                                             \
        case 0x4:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E;                      \
            break;                                                             \
        case 0x5:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;                      \
            break;                                                             \
        case 0x7:                                                              \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E;                      \
            break;                                                             \
        default:                                                               \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);     \
    }

#ifndef DBA_FW
/**
* @internal prvCpssDxChPortTxDbaProfileTcSet function
* @endinternal
*
* @brief   Set per profile and queue (TC) the various DPs guarantee & weight values
*         for DBA feature.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..7).
* @param[in] profileParamsPtr         - (pointer to) the Drop Profile Parameters to
*                                      associate with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
GT_STATUS prvCpssDxChPortTxDbaProfileTcSet
(
    IN  GT_U8   devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN  GT_U8                                   trafficClass,
    IN  CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS *profileParamsPtr
);

/**
* @internal prvCpssDxChPortTxDbaProfileTcGet function
* @endinternal
*
* @brief   Get per profile and queue (TC) the various DPs guarantee & weight values
*         for DBA feature.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..7).
*
* @param[out] profileParamsPtr         - (pointer to) the Drop Profile Parameters to
*                                      associate with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
GT_STATUS prvCpssDxChPortTxDbaProfileTcGet
(
    IN  GT_U8   devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN  GT_U8                                   trafficClass,
    OUT CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS *profileParamsPtr
);

/**
* @internal prvCpssDxChPortTxDbaProxyTxqTableSet function
* @endinternal
*
* @brief   Set Txq specified table (by address) through the DBA acting as a proxy.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] entryAddr                - address of the table entry to write
* @param[in] numOfWords               - number of entry words to write (0..4)
* @param[in] entryDataPtr             - (pointer to) the values to write to the table entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on num of words out of range
*/
GT_STATUS prvCpssDxChPortTxDbaProxyTxqTableSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  entryAddr,
    IN  GT_U32  numOfWords,
    IN  GT_U32  *entryDataPtr
);

/**
* @internal prvCpssDxChPortTxDbaProxyTxqOtherSet function
* @endinternal
*
* @brief   Set Txq registers or tables (beside specified tables) through the DBA
*         acting as a proxy.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] entryAddr                - address to read from
* @param[in] numOfWords               - number of words to read (0..4)
* @param[in] entryDataPtr             - (pointer to) the values read.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on num of words out of range
*
* @note To write values of the specified tables use the (excluded from this API)
*       use the "prvCpssDxChPortTxDbaProxyTxqTableSet" API.
*
*/
GT_STATUS prvCpssDxChPortTxDbaProxyTxqOtherSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  entryAddr,
    IN  GT_U32  numOfWords,
    IN  GT_U32  *entryDataPtr
);

/**
* @internal prvCpssDxChPortTxDbaProxyTxqTableGet function
* @endinternal
*
* @brief   Get Txq specified table (by address) through the DBA acting as a proxy.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] entryAddr                - address of the table entry to read
* @param[in] numOfWords               - number of entry words to read (0..4)
*
* @param[out] entryDataPtr             - (pointer to) the values read from the table entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on num of words out of range
*/
GT_STATUS prvCpssDxChPortTxDbaProxyTxqTableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  entryAddr,
    IN  GT_U32  numOfWords,
    OUT GT_U32  *entryDataPtr
);

/**
* @internal prvCpssDxChPortTxDbaProxyTxqOtherGet function
* @endinternal
*
* @brief   Get Txq registers or tables (beside specified tables) through the DBA
*         acting as a proxy.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] entryAddr                - address to read from
* @param[in] numOfWords               - number of words to read (0..4)
*
* @param[out] entryDataPtr             - (pointer to) the values read.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on num of words out of range
*
* @note To read values of the specified tables use the (excluded from this API)
*       use the "prvCpssDxChPortTxDbaProxyTxqTableGet" API.
*
*/
GT_STATUS prvCpssDxChPortTxDbaProxyTxqOtherGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  entryAddr,
    IN  GT_U32  numOfWords,
    OUT GT_U32  *entryDataPtr
);

#endif /*DBA_FW*/

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __prvCpssDxChPortTxDbah */


