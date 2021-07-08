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
* @file cpssDxChPortTxDba.c
*
* @brief CPSS implementation for configuring, gathering info and statistics
* for the DBA (Dynamic Buffer Allocation) FW
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxDba.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTxDba.h>

#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>

#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>

/* Timeout for message reply after 100uS*/
#define MSG_REPLY_TIMEOUT_IN_US_CNS 100
#define MSG_REPLY_TIMEOUT_IN_NS_CNS (MSG_REPLY_TIMEOUT_IN_US_CNS*1000)

/* 500nS(=0.5uS) delay period*/
#define DELAY_500_NS_CNS 500

/* Timeout for nessage reply in (loop) cycles */
#define MSG_REPLY_TIMEOUT_CYCLES_CNS (MSG_REPLY_TIMEOUT_IN_NS_CNS/DELAY_500_NS_CNS)

/* Convert CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT to HW value */
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_HW_CONVERT_MAC(alpha, hwValue)\
    switch (alpha)                                                            \
    {                                                                         \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                            \
            hwValue = 0x0;                                                    \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E:                        \
            if(PRV_CPSS_PP_MAC(devNum)->devFamily ==                          \
                CPSS_PP_FAMILY_DXCH_ALDRIN2_E)                                \
            {                                                                 \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);\
            }                                                                 \
            hwValue = 0x20;                                                   \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                         \
            hwValue = 0x40;                                                   \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                          \
            hwValue = 0x80;                                                   \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                            \
            hwValue = 0x100;                                                  \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                            \
            hwValue = 0x200;                                                  \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                            \
            hwValue = 0x400;                                                  \
            break;                                                            \
        case CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E:                            \
            if(PRV_CPSS_PP_MAC(devNum)->devFamily ==                          \
                CPSS_PP_FAMILY_DXCH_ALDRIN2_E)                                \
            {                                                                 \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);\
            }                                                                 \
            hwValue = 0x800;                                                  \
            break;                                                            \
        default:                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);    \
    }

/* Convert HW value to CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT */
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_HW_TO_ALPHA_CONVERT_MAC(hwValue, alpha) \
    switch (hwValue)                                                          \
    {                                                                         \
        case 0x0:                                                             \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;                     \
            break;                                                            \
        case 0x20:                                                            \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E;                 \
            break;                                                            \
        case 0x40:                                                            \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E;                  \
            break;                                                            \
        case 0x80:                                                            \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E;                   \
            break;                                                            \
        case 0x100:                                                           \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;                     \
            break;                                                            \
        case 0x200:                                                           \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E;                     \
            break;                                                            \
        case 0x400:                                                           \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;                     \
            break;                                                            \
        case 0x800:                                                           \
            alpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E;                     \
            break;                                                            \
        default:                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);    \
    }

/**
* @internal prvCpssDxChPortTxDbaMessageSend function
* @endinternal
*
* @brief   Create the message toward the DBA feature and send it.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in,out] messageDataPtr           - (pointer to) message content to send.
* @param[in,out] messageLengthPtr         - (pointer to) message content length.
* @param[in,out] messageDataPtr           - (pointer to) message retreived data (due to get
*                                      parameters request or statistics gathering).
* @param[in,out] messageLengthPtr         - (pointer to) the retreived data length.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
static GT_STATUS prvCpssDxChPortTxDbaMessageSend
(
    IN      GT_U8   devNum,
    INOUT   GT_U32  *messageDataPtr,
    INOUT   GT_U32  *messageLengthPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 ii;

    CPSS_NULL_PTR_CHECK_MAC(messageDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(messageLengthPtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->MG.confiProcessor.memoryBase +
                PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).confiProcessorMemorySize -
                                PRV_CPSS_DXCH_DBA_SHM_OFFSET_FROM_SRAM_END;

    /* check previous message processing already finished */
    rc = prvCpssDrvHwPpResourceReadRegister(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            CPSS_DRV_HW_RESOURCE_MG1_CORE_E,
                                            regAddr,
                                            &regData);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Check owner bit state*/
    if( U32_GET_FIELD_MAC(regData,31,1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* write message data */
    for( ii = 0 ; ii < *messageLengthPtr ; ii++ ) {
        rc = prvCpssDrvHwPpResourceWriteRegister(devNum,
                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                             CPSS_DRV_HW_RESOURCE_MG1_CORE_E,
                                             regAddr + (ii+1)*0x4,
                                             messageDataPtr[ii]);
        if( GT_OK != rc )
        {
            return rc;
        }
    }

    /* trigger message processing */
    regData = 0x0;
    U32_SET_FIELD_MAC(regData, 31, 1, 1);
    U32_SET_FIELD_MAC(regData, 18, 13, *messageLengthPtr);

    rc = prvCpssDrvHwPpResourceWriteRegister(devNum,
                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                             CPSS_DRV_HW_RESOURCE_MG1_CORE_E,
                                             regAddr,
                                             regData);
    if( GT_OK != rc )
    {
        return rc;
    }

    ii = 0;
    do
    {
        cpssOsDelay(DELAY_500_NS_CNS);
        rc = prvCpssDrvHwPpResourceReadRegister(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            CPSS_DRV_HW_RESOURCE_MG1_CORE_E,
                                            regAddr,
                                            &regData);
        if( GT_OK != rc )
        {
            return rc;
        }

        if( U32_GET_FIELD_MAC(regData,31,1) == 0 )
            break;

        if( ii > MSG_REPLY_TIMEOUT_CYCLES_CNS )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
        }
        ii++;
    } while(ii);

    /* check message return code */
    switch ( U32_GET_FIELD_MAC(regData,0,8) )
    {
        case PRV_CPSS_DXCH_DBA_RC_CODE_OK_E: rc = GT_OK;
            break;

        case PRV_CPSS_DXCH_DBA_RC_CODE_FAIL_E: rc = GT_FAIL;
            break;

        case PRV_CPSS_DXCH_DBA_RC_CODE_INVALID_OP_CODE_E: rc = GT_NO_SUCH;
            break;

        case PRV_CPSS_DXCH_DBA_RC_CODE_INVALID_IN_PARAMS_E: rc =  GT_BAD_VALUE;
            break;

        case PRV_CPSS_DXCH_DBA_RC_CODE_INVALID_IN_SIZE_E: rc = GT_BAD_SIZE;
            break;

        default: rc = GT_FAIL;
            break;
    }

    if(GT_OK != rc)
    {
        CPSS_LOG_INFORMATION_MAC("return code is %d, message failed with op code %d",
                                 rc, U32_GET_FIELD_MAC(regData,0,8));
        return rc;
    }

    /* retreive data if required */
    if( U32_GET_FIELD_MAC(regData,18,13) > 1 )
    {
        for( ii = 0 ; ii < U32_GET_FIELD_MAC(regData,18,13) ; ii++ )
        {
            rc = prvCpssDrvHwPpResourceReadRegister(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            CPSS_DRV_HW_RESOURCE_MG1_CORE_E,
                                            regAddr + (ii+1)*0x4,
                                            &messageDataPtr[ii]);
            if( GT_OK != rc )
            {
                return rc;
            }
        }
        *messageLengthPtr = U32_GET_FIELD_MAC(regData,18,13);
    } else {
        *messageLengthPtr = 0;
    }

    return GT_OK;
}

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
)
{
    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_SET_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_SET_MSG_LENGTH;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_FALCON_E
                                              | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(profileParamsPtr);

    msgData[0] = PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_SET_E;
    msgData[1] = profileSet;
    msgData[2] = trafficClass;
    msgData[3] = profileParamsPtr->dp0MaxBuffNum;
    msgData[4] = profileParamsPtr->dp1MaxBuffNum;
    msgData[5] = profileParamsPtr->dp2MaxBuffNum;
    PRV_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_HW_CONVERT_MAC(
                                profileParamsPtr->dp0QueueAlpha, msgData[6]);
    PRV_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_HW_CONVERT_MAC(
                                profileParamsPtr->dp1QueueAlpha, msgData[7]);
    PRV_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_HW_CONVERT_MAC(
                                profileParamsPtr->dp2QueueAlpha, msgData[8]);

    return prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
}

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
)
{
    GT_STATUS rc;
    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_GET_RET_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_GET_MSG_LENGTH;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_FALCON_E
                                              | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(profileParamsPtr);

    msgData[0] = PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_GET_E;
    msgData[1] = profileSet;
    msgData[2] = trafficClass;

    rc = prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
    if( GT_OK != rc)
    {
        return rc;
    }

    if( (msgData[0] != PRV_CPSS_DXCH_DBA_OP_CODE_PROFILE_TC_GET_E) ||
        (msgData[1] != (GT_U32)profileSet) ||
        (msgData[2] != trafficClass) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    profileParamsPtr->dp0MaxBuffNum = msgData[3];
    profileParamsPtr->dp1MaxBuffNum = msgData[4];
    profileParamsPtr->dp2MaxBuffNum = msgData[5];

    PRV_CPSS_PORT_TX_TAIL_DROP_DBA_HW_TO_ALPHA_CONVERT_MAC(msgData[6],
                                             profileParamsPtr->dp0QueueAlpha);
    PRV_CPSS_PORT_TX_TAIL_DROP_DBA_HW_TO_ALPHA_CONVERT_MAC(msgData[7],
                                             profileParamsPtr->dp1QueueAlpha);
    PRV_CPSS_PORT_TX_TAIL_DROP_DBA_HW_TO_ALPHA_CONVERT_MAC(msgData[8],
                                             profileParamsPtr->dp2QueueAlpha);

    return GT_OK;
}

/**
* @internal prvCpssDxChPortTxDbaProxyTxqSet function
* @endinternal
*
* @brief   Set Txq address values through the DBA acting as a proxy.
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
static GT_STATUS prvCpssDxChPortTxDbaProxyTxqSet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        entryAddr,
    IN  GT_U32                        numOfWords,
    IN  PRV_CPSS_DXCH_DBA_OP_CODE_ENT otherOrTable,
    IN  GT_U32                        *entryDataPtr
)
{
    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_SET_MAX_MSG_LENGTH];
    GT_U32    msgLen;
    GT_U32    ii;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E
                                              | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(entryDataPtr);

    if( numOfWords > DBA_TXQ_PROXY_MAX_DATA_ACCESS_LENGTH )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if( (otherOrTable != PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_PROFILE_TABLE_SET_E) &&
        (otherOrTable != PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_OTHER_SET_E) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }
    msgData[0] = otherOrTable;
    msgData[1] = entryAddr;
    msgData[2] = numOfWords;
    for( ii = 0 ; ii < numOfWords ; ii++ )
    {
        msgData[3+ii] = entryDataPtr[ii];
    }

    msgLen = 3 + numOfWords;

    return prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
}

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
)
{
    return prvCpssDxChPortTxDbaProxyTxqSet(
        devNum, entryAddr, numOfWords,
        PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_PROFILE_TABLE_SET_E,
        entryDataPtr);
}

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
)
{
    /* temporary code copied from prvCpssDxChPortTxDbaProxyTxqTableSet */
    /* must be updated                                                 */
    return prvCpssDxChPortTxDbaProxyTxqSet(
        devNum, entryAddr, numOfWords,
        PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_OTHER_SET_E,
        entryDataPtr);
}

/**
* @internal prvCpssDxChPortTxDbaProxyTxqGet function
* @endinternal
*
* @brief   Get Txq address values through the DBA acting as a proxy.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] entryAddr                - address to read from
* @param[in] numOfWords               - number of words to read (0..4)
* @param[in] otherOrTable             - distinguish between specific tables and other TxQ registers.
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
*/
static GT_STATUS prvCpssDxChPortTxDbaProxyTxqGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  entryAddr,
    IN  GT_U32  numOfWords,
    IN  PRV_CPSS_DXCH_DBA_OP_CODE_ENT otherOrTable,
    OUT GT_U32  *entryDataPtr
)
{
    GT_STATUS rc;
    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_GET_MAX_RET_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_GET_MSG_LENGTH;
    GT_U32    ii;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E
                                              | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(entryDataPtr);

    if( numOfWords > DBA_TXQ_PROXY_MAX_DATA_ACCESS_LENGTH )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if( (otherOrTable != PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_PROFILE_TABLE_GET_E) &&
        (otherOrTable != PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_OTHER_GET_E) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    msgData[0] = otherOrTable;
    msgData[1] = entryAddr;
    msgData[2] = numOfWords;

    rc = prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
    if( GT_OK != rc)
    {
        return rc;
    }

    if( (msgData[0] != (GT_U32)otherOrTable) ||
        (msgData[1] != entryAddr) ||
        (msgData[2] != numOfWords) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for( ii = 0 ; ii < numOfWords ; ii++ )
    {
        entryDataPtr[ii] = msgData[3+ii];
    }

    return GT_OK;
}

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
)
{
    return prvCpssDxChPortTxDbaProxyTxqGet(devNum,
                                           entryAddr,
                                           numOfWords,
                       PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_PROFILE_TABLE_GET_E,
                                           entryDataPtr);
}

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
)
{
    return prvCpssDxChPortTxDbaProxyTxqGet(devNum,
                                           entryAddr,
                                           numOfWords,
                               PRV_CPSS_DXCH_DBA_OP_CODE_TXQ_PROXY_OTHER_GET_E,
                                           entryDataPtr);
}

/**
* @internal internal_cpssDxChPortTxDbaEnableSet function
* @endinternal
*
* @brief   Enabling DBA feature
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - GT_TRUE  - enable DBA feature
*                                       GT_FALSE - disable DBA feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortTxDbaEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
{
    GT_STATUS rc;
    GT_U32    regAddr;

    if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) && PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.
            tailDropConfig.globalTailDropConfig;
        return prvCpssHwPpSetRegField(devNum, regAddr, 17, 1,
            BOOL2BIT_MAC(enable));
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsServiceCpuDbaEnable(devNum[%d], enable[%d])", devNum, enable);
        rc = mvHwsServiceCpuDbaEnable(devNum, enable);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortTxDbaEnableSet function
* @endinternal
*
* @brief   Enabling DBA feature
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - GT_TRUE  - enable DBA feature
*                                       GT_FALSE - disable DBA feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxDbaEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxDbaEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortTxDbaEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxDbaEnableGet function
* @endinternal
*
* @brief   Indicates if DBA feature is enabled
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE  - DBA feature enabled
*                                        GT_FALSE - no DBA feature support (or disabled)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortTxDbaEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regData;
    GT_U32    regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.
            tailDropConfig.globalTailDropConfig;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 17, 1, &regData);
        if(GT_OK != rc)
        {
            return rc;
        }
        *enablePtr = BIT2BOOL_MAC(regData);
        return rc;
    }

    *enablePtr = mvHwsServiceCpuDbaEnableGet(devNum);

    return rc;
}

/**
* @internal cpssDxChPortTxDbaEnableGet function
* @endinternal
*
* @brief   Indicates if DBA feature is enabled
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - (pointer to) GT_TRUE  - DBA feature enabled
*                                        GT_FALSE - no DBA feature support (or disabled)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - enabledPtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxDbaEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxDbaEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortTxDbaEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxDbaDebugControl function
* @endinternal
*
* @brief   Start or stop DBA operation and\or gathering statistics
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] startStopDba             - GT_TRUE  - start DBA operation, i.e. profile thresholds updates.
*                                      GT_FALSE - stop DBA operation, i.e. no profile thresholds updates.
* @param[in] startStopStatistics      - GT_TRUE  - start statistics gathering
*                                      GT_FALSE - stop statistics gathering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
GT_STATUS internal_cpssDxChPortTxDbaDebugControl
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  startStopDba,
    IN  GT_BOOL  startStopStatistics
)
{
    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_CONTROL_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_DBA_OP_CODE_CONTROL_MSG_LENGTH;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_FALCON_E
                                              | CPSS_FALCON_E);

    msgData[0] = PRV_CPSS_DXCH_DBA_OP_CODE_CONTROL_E;
    msgData[1] = (GT_TRUE == startStopDba) ? 1 : 0 ;
    msgData[2] = (GT_TRUE == startStopStatistics) ? 1 : 0 ;

    return prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
}

/**
* @internal cpssDxChPortTxDbaDebugControl function
* @endinternal
*
* @brief   Start or stop DBA operation and\or gathering statistics
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] startStopDba             - GT_TRUE  - start DBA operation, i.e. profile thresholds updates.
*                                      GT_FALSE - stop DBA operation, i.e. no profile thresholds updates.
* @param[in] startStopStatistics      - GT_TRUE  - start statistics gathering
*                                      GT_FALSE - stop statistics gathering
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
GT_STATUS cpssDxChPortTxDbaDebugControl
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  startStopDba,
    IN  GT_BOOL  startStopStatistics
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxDbaDebugControl);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startStopDba, startStopStatistics));

    rc = internal_cpssDxChPortTxDbaDebugControl(devNum, startStopDba, startStopStatistics);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startStopDba, startStopStatistics));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxDbaAvailBuffSet function
* @endinternal
*
* @brief   Set maximal available buffers for allocation.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] maxBuff                  - Maximal available buffers for allocation.
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on out of range number of buffers
*                                      (APPLICABLE RANGES: Aldrin2; Falcon: 0..0xFFFFF)
*/
GT_STATUS internal_cpssDxChPortTxDbaAvailBuffSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  maxBuff
)
{
    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_SET_AVAIL_BUFFERS_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_DBA_OP_CODE_SET_AVAIL_BUFFERS_MSG_LENGTH;
    GT_U32    regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E);

    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        if( maxBuff >= BIT_20 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.
            tailDropConfig.dynamicAvailableBuffers;
        return prvCpssHwPpSetRegField(devNum, regAddr, 0, 20, maxBuff);
    }
    else
    {
        if( maxBuff >= BIT_16 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    msgData[0] = PRV_CPSS_DXCH_DBA_OP_CODE_SET_AVAIL_BUFFERS_E;
    msgData[1] = maxBuff;

    return prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
}

/**
* @internal cpssDxChPortTxDbaAvailBuffSet function
* @endinternal
*
* @brief   Set maximal available buffers for allocation.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] maxBuff                  - Maximal available buffers for allocation.
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on out of range number of buffers
*                                      (APPLICABLE RANGES: Aldrin2; Falcon: 0..0xFFFFF)
*/
GT_STATUS cpssDxChPortTxDbaAvailBuffSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  maxBuff
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxDbaAvailBuffSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxBuff));

    rc = internal_cpssDxChPortTxDbaAvailBuffSet(devNum, maxBuff);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxBuff));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxDbaAvailBuffGet function
* @endinternal
*
* @brief   Get maximal available buffers for allocation.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] maxBuffPtr               - (pointer to) Maximal available buffers for allocation.
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
GT_STATUS internal_cpssDxChPortTxDbaAvailBuffGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *maxBuffPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;

    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_GET_AVAIL_BUFFERS_RET_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_DBA_OP_CODE_GET_AVAIL_BUFFERS_MSG_LENGTH;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(maxBuffPtr);

    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.
            tailDropConfig.dynamicAvailableBuffers;
        return prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, maxBuffPtr);
    }

    msgData[0] = PRV_CPSS_DXCH_DBA_OP_CODE_GET_AVAIL_BUFFERS_E;

    rc = prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
    if( GT_OK != rc)
    {
        return rc;
    }

    if( msgData[0] != PRV_CPSS_DXCH_DBA_OP_CODE_GET_AVAIL_BUFFERS_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    *maxBuffPtr = msgData[1];

    return GT_OK;
}

/**
* @internal cpssDxChPortTxDbaAvailBuffGet function
* @endinternal
*
* @brief   Get maximal available buffers for allocation.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] maxBuffPtr               - (pointer to) Maximal available buffers for allocation.
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
GT_STATUS cpssDxChPortTxDbaAvailBuffGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *maxBuffPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxDbaAvailBuffGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxBuffPtr));

    rc = internal_cpssDxChPortTxDbaAvailBuffGet(devNum, maxBuffPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxBuffPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxDbaDebugClearGlobalCounters function
* @endinternal
*
* @brief   Clear the statistics collected related to number of free buffers
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
GT_STATUS internal_cpssDxChPortTxDbaDebugClearGlobalCounters
(
    IN  GT_U8   devNum
)
{
    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_CLEAR_GLOBAL_COUNTERS_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_DBA_OP_CODE_CLEAR_GLOBAL_COUNTERS_MSG_LENGTH;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_FALCON_E
                                              | CPSS_FALCON_E);

    msgData[0] = PRV_CPSS_DXCH_DBA_OP_CODE_CLEAR_GLOBAL_COUNTERS_E;

    return prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
}

/**
* @internal cpssDxChPortTxDbaDebugClearGlobalCounters function
* @endinternal
*
* @brief   Clear the statistics collected related to number of free buffers
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
*/
GT_STATUS cpssDxChPortTxDbaDebugClearGlobalCounters
(
    IN  GT_U8   devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxDbaDebugClearGlobalCounters);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChPortTxDbaDebugClearGlobalCounters(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxDbaDebugGlobalCountersGet function
* @endinternal
*
* @brief   Get Min and Max values of free buffers during DBA run & threshold
*         updates from last values clearing.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] minFreeBuffersPtr        - (pointer to) Minimum value of free buffers.
* @param[out] maxFreeBuffersPtr        - (pointer to) Maximum value of free buffers.
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
*
* @note Previous values clearing is done using API
*       cpssDxChPortTxDbaDebugClearGlobalCounters.
*
*/
GT_STATUS internal_cpssDxChPortTxDbaDebugGlobalCountersGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *minFreeBuffersPtr,
    OUT GT_U32  *maxFreeBuffersPtr
)
{
    GT_STATUS rc;

    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_GET_GLOBAL_COUNTERS_RET_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_DBA_OP_CODE_GET_GLOBAL_COUNTERS_MSG_LENGTH;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_FALCON_E
                                              | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(minFreeBuffersPtr);
    CPSS_NULL_PTR_CHECK_MAC(maxFreeBuffersPtr);

    msgData[0] = PRV_CPSS_DXCH_DBA_OP_CODE_GET_GLOBAL_COUNTERS_E;

    rc = prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
    if( GT_OK != rc)
    {
        return rc;
    }

    if( msgData[0] != PRV_CPSS_DXCH_DBA_OP_CODE_GET_GLOBAL_COUNTERS_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    *minFreeBuffersPtr = msgData[2];
    *maxFreeBuffersPtr = msgData[3];

    return GT_OK;
}

/**
* @internal cpssDxChPortTxDbaDebugGlobalCountersGet function
* @endinternal
*
* @brief   Get Min and Max values of free buffers during DBA run & threshold
*         updates from last values clearing.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] minFreeBuffersPtr        - (pointer to) Minimum value of free buffers.
* @param[out] maxFreeBuffersPtr        - (pointer to) Maximum value of free buffers.
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
*
* @note Previous values clearing is done using API
*       cpssDxChPortTxDbaDebugClearGlobalCounters.
*
*/
GT_STATUS cpssDxChPortTxDbaDebugGlobalCountersGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *minFreeBuffersPtr,
    OUT GT_U32  *maxFreeBuffersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxDbaDebugGlobalCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, minFreeBuffersPtr, maxFreeBuffersPtr));

    rc = internal_cpssDxChPortTxDbaDebugGlobalCountersGet(devNum, minFreeBuffersPtr, maxFreeBuffersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, minFreeBuffersPtr, maxFreeBuffersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxDbaDebugProfileCountersEnable function
* @endinternal
*
* @brief   Enable collecting statistical values of <profile,dp,queue> triplet
*         threshold during DBA run & threshold updates, the enable operation
*         clears previous data.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] profileSet               - the Profile Set to which the Traffic
*                                      Class Drop Parameters are associated.
* @param[in] dpLevel                  - Drop Precedence level
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters.
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on out of range tc
*/
GT_STATUS internal_cpssDxChPortTxDbaDebugProfileCountersEnable
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  CPSS_DP_LEVEL_ENT                   dpLevel,
    IN  GT_U8                               trafficClass
)
{
    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_ENABLE_PROFILE_COUNTERS_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_DBA_OP_CODE_ENABLE_PROFILE_COUNTERS_MSG_LENGTH;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_FALCON_E
                                              | CPSS_FALCON_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(trafficClass);

    msgData[0] = PRV_CPSS_DXCH_DBA_OP_CODE_ENABLE_PROFILE_COUNTERS_E;
    msgData[1] = profileSet;
    msgData[2] = dpLevel;
    msgData[3] = trafficClass;

    return prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
}

/**
* @internal cpssDxChPortTxDbaDebugProfileCountersEnable function
* @endinternal
*
* @brief   Enable collecting statistical values of <profile,dp,queue> triplet
*         threshold during DBA run & threshold updates, the enable operation
*         clears previous data.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] profileSet               - the Profile Set to which the Traffic
*                                      Class Drop Parameters are associated.
* @param[in] dpLevel                  - Drop Precedence level
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters.
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NO_SUCH               - on unrecognized message op code
* @retval GT_BAD_VALUE             - on wrong message parameters
* @retval GT_BAD_SIZE              - on wrong message size
* @retval GT_OUT_OF_RANGE          - on out of range tc
*/
GT_STATUS cpssDxChPortTxDbaDebugProfileCountersEnable
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  CPSS_DP_LEVEL_ENT                   dpLevel,
    IN  GT_U8                               trafficClass
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxDbaDebugProfileCountersEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, dpLevel, trafficClass));

    rc = internal_cpssDxChPortTxDbaDebugProfileCountersEnable(devNum, profileSet, dpLevel, trafficClass);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, dpLevel, trafficClass));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxDbaDebugProfileCountersGet function
* @endinternal
*
* @brief   Get Min and Max values of <profile,dp,queue> triplet threshold during
*         DBA run & threshold updates from last values clearing.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] minThresholdPtr          - (pointer to) Minimum value of threshold.
* @param[out] maxThresholdPtr          - (pointer to) Maximum value of threshold.
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
*
* @note Previous values clearing and triplet selection is done using API
*       cpssDxChPortTxDbaDebugProfileCountersEnable.
*
*/
GT_STATUS internal_cpssDxChPortTxDbaDebugProfileCountersGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *minThresholdPtr,
    OUT GT_U32  *maxThresholdPtr
)
{
    GT_STATUS rc;

    GT_U32    msgData[PRV_CPSS_DXCH_DBA_OP_CODE_GET_PROFILE_COUNTERS_RET_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_DBA_OP_CODE_GET_PROFILE_COUNTERS_MSG_LENGTH;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_FALCON_E
                                              | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(minThresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(maxThresholdPtr);

    msgData[0] = PRV_CPSS_DXCH_DBA_OP_CODE_GET_PROFILE_COUNTERS_E;

    rc = prvCpssDxChPortTxDbaMessageSend(devNum, &msgData[0], &msgLen);
    if( GT_OK != rc)
    {
        return rc;
    }

    if( msgData[0] != PRV_CPSS_DXCH_DBA_OP_CODE_GET_PROFILE_COUNTERS_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    *minThresholdPtr = msgData[2];
    *maxThresholdPtr = msgData[3];

    return GT_OK;
}

/**
* @internal cpssDxChPortTxDbaDebugProfileCountersGet function
* @endinternal
*
* @brief   Get Min and Max values of <profile,dp,queue> triplet threshold during
*         DBA run & threshold updates from last values clearing.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] minThresholdPtr          - (pointer to) Minimum value of threshold.
* @param[out] maxThresholdPtr          - (pointer to) Maximum value of threshold.
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
*
* @note Previous values clearing and triplet selection is done using API
*       cpssDxChPortTxDbaDebugProfileCountersEnable.
*
*/
GT_STATUS cpssDxChPortTxDbaDebugProfileCountersGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *minThresholdPtr,
    OUT GT_U32  *maxThresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxDbaDebugProfileCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, minThresholdPtr, maxThresholdPtr));

    rc = internal_cpssDxChPortTxDbaDebugProfileCountersGet(devNum, minThresholdPtr, maxThresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, minThresholdPtr, maxThresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
