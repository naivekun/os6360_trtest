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
* @file cpssDxChTam.c
*
* @brief CPSS definitions for configuring, gathering info and statistics
*        for the TAM (Telemetry Analytics and Monitoring) feature
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/tam/private/prvCpssDxChTamLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>
#include <cpss/dxCh/dxChxGen/tam/private/prvCpssDxChTam.h>
#include <cpss/dxCh/dxChxGen/tam/cpssDxChTam.h>

#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpssCommon/private/prvCpssMath.h>

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>

/* Timeout for message reply after 100uS*/
#define MSG_REPLY_TIMEOUT_IN_US_CNS 100
#define MSG_REPLY_TIMEOUT_IN_NS_CNS (MSG_REPLY_TIMEOUT_IN_US_CNS*1000)

/* 500nS(=0.5uS) delay period*/
#define DELAY_500_NS_CNS 500

/* Timeout for nessage reply in (loop) cycles */
#define MSG_REPLY_TIMEOUT_CYCLES_CNS (MSG_REPLY_TIMEOUT_IN_NS_CNS/DELAY_500_NS_CNS)

GT_U32 tamSnapshotDbStartAddr; /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC */
GT_U32 tamCountersDbStartAddr; /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_COUNTER_DB_STC */
GT_U32 tamMicroburstDbStartAddr; /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC */
GT_U32 tamHistogramDbStartAddr; /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC */
GT_U32 tamThresholdDbStartAddr; /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_THRESHOLD_DB_STC */
GT_U32 tamThresholdNotifStartAddr;
GT_U32 tamThresholdNotifBmpStartAddr;
GT_U32 tamFwDebugCounterAddr;
GT_U32 tamFwDebugStatCounterAddr;

GT_U32 freeCounterFirstAddr;
GT_U32 freeCountersNum;

/* defines to work with DB PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC */
#define SNAPSHOT_DB_FIRST_COUNTER_MASK      0x0000FFFF
#define SNAPSHOT_DB_STAT_BUFF_MASK          0x00FF0000

/* defines to work with DB PRV_CPSS_DXCH_TAM_COUNTER_DB_STC */
#define COUNTER_DB_REG_ADDR_OFFSET          0 /* offset in bytes (for specific counter entry) */
#define COUNTER_DB_STAT_0_OFFSET            4
#define STATISTICS_DB_SIZE                  sizeof(PRV_CPSS_DXCH_TAM_COUNTER_STATISTICS_DB_STC) /* in bytes */
#define STATISTICS_SIZE_IN_WORDS            STATISTICS_DB_SIZE >> 2 /* == STATISTICS_DB_SIZE / 4 */
#define ALL_STATISTICS_SIZE_IN_WORDS        STATISTICS_DB_SIZE >> 1 /* == STATISTICS_DB_SIZE * 2 / 4 */
#define COUNTER_DB_STAT_1_OFFSET            COUNTER_DB_STAT_0_OFFSET + STATISTICS_DB_SIZE
#define COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET COUNTER_DB_STAT_1_OFFSET + STATISTICS_DB_SIZE
/*#define COUNTER_DB_NEXT_COUNTER_ADDR_MASK   0x0000FFFF*/

/* defines to work with DB PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC */
#define MICROBURST_DB_COUNTER_MASK            0x0000FFFF
#define MICROBURST_DB_STAT_BUFF_MASK          0x00FF0000
#define MICROBURST_DB_SNAPSHOT_ID_MASK        0xFF000000
#define MICROBURST_DB_HIGH_WATERMARK_OFFSET   4 /* offset in bytes */
#define MICROBURST_DB_LOW_WATERMARK_OFFSET    8
#define MICROBURST_DB_TIME_START_OFFSET       12
#define MICROBURST_DB_STAT_0_OFFSET           16
#define MICROBURST_DB_STAT_1_OFFSET           MICROBURST_DB_STAT_0_OFFSET + STATISTICS_DB_SIZE
#define MICROBURST_DB_CALC_HISTOGRAM_TBD      MICROBURST_DB_STAT_1_OFFSET + STATISTICS_DB_SIZE
#define MICROBURST_DB_TIME_STATUS_TBD         MICROBURST_DB_STAT_1_OFFSET + STATISTICS_DB_SIZE
#define MICROBURST_DB_CALC_HISTOGRAM_MASK     0xFFFF0000
#define MICROBURST_DB_TIME_STATUS_MASK        0x0000FFFF
/*#define MICROBURST_DB_HISTOGRAM_OFFSET        MICROBURST_DB_TIME_STATUS_TBD + 4*/

/* defines to work with DB PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC */
#define HISTOGRAM_DB_COUNTER_MASK            0x0000FFFF
#define HISTOGRAM_DB_STAT_BUFF_MASK          0x00FF0000
#define HISTOGRAM_DB_TYPE_MASK               0xFF000000
#define HISTOGRAM_DB_BIN_LEVELS_OFFSET       4 /* offset in bytes */
#define HISTOGRAM_DB_BIN_LEVEL_BYTES         4 /* number of bytes for each bin in the DB */
#define HISTOGRAM_DB_BIN_VAL_STAT_0_OFFSET   HISTOGRAM_DB_BIN_LEVELS_OFFSET + 12 /* 12 == 4*3 == (HISTOGRAM_DB_BIN_LEVEL_BYTES * PRV_CPSS_DXCH_TAM_FW_BIN_LEVELS) */
#define HISTOGRAM_DB_BIN_VAL_BYTES           8 /* number of bytes for each bin value in the DB */
#define HISTOGRAM_DB_BIN_VAL_STAT_1_OFFSET   HISTOGRAM_DB_BIN_VAL_STAT_0_OFFSET + 32 /* 32 == 8*4 == (HISTOGRAM_DB_BIN_VAL_BYTES * PRV_CPSS_DXCH_TAM_FW_BINS)*/
#define HISTOGRAM_DB_SNAPSHOT_OFFSET         HISTOGRAM_DB_BIN_VAL_STAT_1_OFFSET + 32 /* 32 == 8*4 == (HISTOGRAM_DB_BIN_VAL_BYTES * PRV_CPSS_DXCH_TAM_FW_BINS)*/

/* defines to work with DB PRV_CPSS_DXCH_TAM_THRESHOLD_DB_STC */
#define THRESHOLD_DB_COUNTER_MASK            0x0000FFFF
#define THRESHOLD_DB_SNAPSHOT_MASK           0xFFFF0000
#define THRESHOLD_DB_LEVEL_OFFSET            4 /* offset in bytes */

typedef struct CPSS_DXCH_TAM_SNAPSHOT_SHADOW_DB
{
    GT_U8                           numOfCounters;

} CPSS_DXCH_TAM_SNAPSHOT_SHADOW_DB;

typedef struct CPSS_DXCH_TAM_COUNTER_SHADOW_DB
{
    CPSS_DXCH_TAM_COUNTER_ID_STC    counter;
    GT_U8                           snapshotId; /* default value is 0xFF */
    GT_U8                           histogramId; /* default value is 0xFF */

} CPSS_DXCH_TAM_COUNTER_SHADOW_DB;

typedef struct CPSS_DXCH_TAM_MICROBURST_SHADOW_DB
{
    GT_U8 histogramId;  /* default value is 0xFF */

} CPSS_DXCH_TAM_MICROBURST_SHADOW_DB;

CPSS_DXCH_TAM_SNAPSHOT_SHADOW_DB    snapshotShadowDb[PRV_CPSS_DXCH_TAM_FW_SNAPSHOTS];
CPSS_DXCH_TAM_COUNTER_SHADOW_DB     countersShadowDb[PRV_CPSS_DXCH_TAM_FW_COUNTERS];
CPSS_DXCH_TAM_MICROBURST_SHADOW_DB  microburstShadowDb[PRV_CPSS_DXCH_TAM_FW_MICROBURSTS];

static char* tamCounterTypeStr[12]=
{
    "",
    "ING_GLOBAL_BUF",
    "ING_GLOBAL_MIRR_BUF",
    "ING_PORT_BUF",
    "ING_PG_TC_BUF",
    "EGR_GLOBAL_BUF",
    "EGR_GLOBAL_MULT_BUF",
    "EGR_GLOBAL_MIRR_BUF",
    "EGR_PORT_BUF",
    "EGR_QUEUE_BUF",
    "EGR_SHARED_POOL_BUF",
    "MICROBURST_DURATION"
};

static CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC *notifyFuncPtr = NULL;

/**
* @internal prvCpssDxChTamSharedMemRead function
* @endinternal
*
* @brief   Read word (32 bit) of memory from the fw shared memory.
*          The address should be aligned.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in]  devNum                - physical device number.
* @param[in]  address               - address to read (from the memory base).
* @param[out] value                 - (pointer to) memory value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChTamSharedMemRead
(
    IN  GT_U8   devNum,
    IN  GT_U32  address,
    OUT GT_U32  *value
)
{
    GT_STATUS rc;

    rc = prvCpssDrvHwPpResourceReadRegister(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            CPSS_DRV_HW_RESOURCE_MG1_CORE_E,
                                            PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->MG.confiProcessor.memoryBase +
                                            address,
                                            value);
    return rc;
}

/**
* @internal prvCpssDxChTamSharedMemWrite function
* @endinternal
*
* @brief   Write word (32 bit) to the fw shared memory.
*          The address should be aligned.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in]  devNum                - physical device number.
* @param[in]  address               - address to read (from the memory base).
* @param[in]  value                 - value to write.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChTamSharedMemWrite
(
    IN  GT_U8   devNum,
    IN  GT_U32  address,
    IN  GT_U32  value
)
{
    GT_STATUS rc;

    rc = prvCpssDrvHwPpResourceWriteRegister(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            CPSS_DRV_HW_RESOURCE_MG1_CORE_E,
                                            PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->MG.confiProcessor.memoryBase +
                                            address,
                                            value);
    return rc;
}

/**
* @internal prvCpssDxChTamMessageSend function
* @endinternal
*
* @brief   Create the message toward the TAM feature and send it.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in,out] messageDataPtr    - (pointer to) message content to send.
* @param[in,out] messageLengthPtr  - (pointer to) message content length.
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
static GT_STATUS prvCpssDxChTamMessageSend
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

    regAddr = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).confiProcessorMemorySize -
              PRV_CPSS_DXCH_TAM_SHM_OFFSET_FROM_SRAM_END;

    /* check previous message processing already finished */
    rc = prvCpssDxChTamSharedMemRead(devNum, regAddr, &regData);
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
    for( ii = 0 ; ii < *messageLengthPtr ; ii++ )
    {
        rc = prvCpssDxChTamSharedMemWrite(devNum, regAddr + (ii+1)*0x4, messageDataPtr[ii]);
        if( GT_OK != rc )
        {
            return rc;
        }
    }

    /* trigger message processing */
    regData = 0x0;
    U32_SET_FIELD_MAC(regData, 31, 1, 1);
    U32_SET_FIELD_MAC(regData, 18, 13, *messageLengthPtr);
    rc = prvCpssDxChTamSharedMemWrite(devNum, regAddr, regData);
    if( GT_OK != rc )
    {
        return rc;
    }

    ii = 0;
    do
    {
        cpssOsDelay(DELAY_500_NS_CNS);
        rc = prvCpssDxChTamSharedMemRead(devNum, regAddr, &regData);
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
        case PRV_CPSS_DXCH_TAM_RC_CODE_OK_E: rc = GT_OK;
            break;

        case PRV_CPSS_DXCH_TAM_RC_CODE_FAIL_E: rc = GT_FAIL;
            break;

        case PRV_CPSS_DXCH_TAM_RC_CODE_INVALID_OP_CODE_E: rc = GT_NO_SUCH;
            break;

        case PRV_CPSS_DXCH_TAM_RC_CODE_INVALID_IN_PARAMS_E: rc =  GT_BAD_VALUE;
            break;

        case PRV_CPSS_DXCH_TAM_RC_CODE_INVALID_IN_SIZE_E: rc = GT_BAD_SIZE;
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
            rc = prvCpssDxChTamSharedMemRead(devNum, regAddr + (ii+1)*0x4, &messageDataPtr[ii]);
            if( GT_OK != rc )
            {
                return rc;
            }
        }
        *messageLengthPtr = U32_GET_FIELD_MAC(regData,18,13);
    }
    else
    {
        *messageLengthPtr = 0;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChTamInit function
* @endinternal
*
* @brief   init process for TAM feature.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChTamInit
(
    IN  GT_U8    devNum
)
{
    GT_STATUS rc;
    GT_U32    address, snapshotId, counterId, microburstId;

    GT_U32    msgData[PRV_CPSS_DXCH_TAM_OP_CODE_INIT_RET_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_TAM_OP_CODE_INIT_MSG_LENGTH;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    /*  */
    /* PFC Trigger Global Configuration Register 0x42000000 */
    /* PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum).regsAddrPtr->TXQ.pfc.PFCTriggerGlobalConfig; */
    rc = cpssDxChPortPfcCountingModeSet(devNum, CPSS_DXCH_PORT_PFC_COUNT_BUFFERS_MODE_E);
    if(GT_OK != rc)
    {
        return rc;
    }

    msgData[0] = PRV_CPSS_DXCH_TAM_OP_CODE_INIT_E;
    rc = prvCpssDxChTamMessageSend(devNum, &msgData[0], &msgLen);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(msgData[0] != PRV_CPSS_DXCH_TAM_OP_CODE_INIT_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    tamSnapshotDbStartAddr = msgData[1];
    tamCountersDbStartAddr = msgData[2];
    tamMicroburstDbStartAddr = msgData[3];
    tamHistogramDbStartAddr = msgData[4];
    tamThresholdDbStartAddr = msgData[5];
    tamThresholdNotifStartAddr = msgData[6];
    tamThresholdNotifBmpStartAddr = msgData[7];
    tamFwDebugCounterAddr = msgData[8];
    tamFwDebugStatCounterAddr = msgData[9];

    freeCounterFirstAddr = tamCountersDbStartAddr;
    freeCountersNum = PRV_CPSS_DXCH_TAM_FW_COUNTERS;

    /* set for all the snapshots: firstCounterAddr=0 and statBuff=0 */
    address = tamSnapshotDbStartAddr;
    for (snapshotId = 0; snapshotId < PRV_CPSS_DXCH_TAM_FW_SNAPSHOTS; snapshotId++)
    {
        rc = prvCpssDxChTamSharedMemWrite(devNum, address, 0);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        address += 4;

        snapshotShadowDb[snapshotId].numOfCounters = 0;
    }

    /* set for all the counters the next counter field and default min value */
    address = tamCountersDbStartAddr;
    for (counterId = 0; counterId < (PRV_CPSS_DXCH_TAM_FW_COUNTERS - 1); counterId++)
    {
        /* set default min value to 0xFFFFFFFF. (the min is in the second word of statistics) */
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          address + COUNTER_DB_STAT_0_OFFSET + 4,
                                          0xFFFFFFFF);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          address + COUNTER_DB_STAT_1_OFFSET + 4,
                                          0xFFFFFFFF);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* set the next counter field and calculate the next counter address */
        address += COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET;
        rc = prvCpssDxChTamSharedMemWrite(devNum, address, address + 4);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        address += 4;

        /* countersShadowDb[counterId].counter.type ==0 i.e:not valid */
        cpssOsMemSet(&countersShadowDb[counterId], 0, sizeof(CPSS_DXCH_TAM_COUNTER_SHADOW_DB));
        countersShadowDb[counterId].snapshotId = 0xFF;
        countersShadowDb[counterId].histogramId = 0xFF;
    }

    /* set the last in the counters list to 0 */
    address += COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET;
    rc = prvCpssDxChTamSharedMemWrite(devNum, address, 0);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    cpssOsMemSet(&countersShadowDb[counterId], 0, sizeof(CPSS_DXCH_TAM_COUNTER_SHADOW_DB));
    countersShadowDb[counterId].snapshotId = 0xFF;
    countersShadowDb[counterId].histogramId = 0xFF;

    /*  */
    address = tamMicroburstDbStartAddr;
    for (microburstId = 0; microburstId < PRV_CPSS_DXCH_TAM_FW_MICROBURSTS; microburstId++)
    {
        /* set default min value to 0xFFFFFFFF */
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          address + MICROBURST_DB_STAT_0_OFFSET + 4,
                                          0xFFFFFFFF);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          address + MICROBURST_DB_STAT_1_OFFSET + 4,
                                          0xFFFFFFFF);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        address += (sizeof(PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC));

        /* set shadow default value */
        microburstShadowDb[microburstId].histogramId = 0xFF;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTamInit function
* @endinternal
*
* @brief   init process for TAM feature.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                   - physical device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamInit
(
    IN  GT_U8    devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChTamInit(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTamDebugControl function
* @endinternal
*
* @brief  enable/disable TAM statistics measurements by FW.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] startStopStatistics      - GT_TRUE  - enable statistics measurements
*                                       GT_FALSE - disable statistics measurements
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChTamDebugControl
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  startStopStatistics
)
{
    GT_U32    msgData[PRV_CPSS_DXCH_TAM_OP_CODE_CONTROL_MSG_LENGTH];
    GT_U32    msgLen = PRV_CPSS_DXCH_TAM_OP_CODE_CONTROL_MSG_LENGTH;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    msgData[0] = PRV_CPSS_DXCH_TAM_OP_CODE_CONTROL_E;
    msgData[1] = 0;
    msgData[2] = (GT_TRUE == startStopStatistics) ? 1 : 0 ;

    return prvCpssDxChTamMessageSend(devNum, &msgData[0], &msgLen);
}

/**
* @internal cpssDxChTamDebugControl function
* @endinternal
*
* @brief  enable/disable TAM statistics measurements by FW.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] startStopStatistics      - GT_TRUE  - enable statistics measurements
*                                       GT_FALSE - disable statistics measurements
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTamDebugControl
(
    IN  GT_U8    devNum,
    IN  GT_BOOL  startStopStatistics
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamDebugControl);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startStopStatistics));

    rc = internal_cpssDxChTamDebugControl(devNum, startStopStatistics);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startStopStatistics));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTamFwDebugCountersGet function
* @endinternal
*
* @brief  can be used to check if fw running ok.
*         if the counters value is incremented between calls it means the fw enabled
*         and statistics calculated.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[out] fwCounter            - this counter incremented if the tam task started.
* @param[out] fwStatCounter        - this counter incremented if tam statistics enabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChTamFwDebugCountersGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *fwCounter,
    OUT GT_U32   *fwStatCounter
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    rc = prvCpssDxChTamSharedMemRead(devNum, tamFwDebugCounterAddr, fwCounter);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    rc = prvCpssDxChTamSharedMemRead(devNum, tamFwDebugStatCounterAddr, fwStatCounter);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal cpssDxChTamFwDebugCountersGet function
* @endinternal
*
* @brief  can be used to check if fw running ok.
*         if the counters value is incremented between calls it means the fw enabled
*         and statistics calculated.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[out] fwCounter            - this counter incremented if the tam task started.
* @param[out] fwStatCounter        - this counter incremented if tam statistics enabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTamFwDebugCountersGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *fwCounter,
    OUT GT_U32   *fwStatCounter
)
{
    GT_STATUS rc;
    /*CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamFwDebugCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fwCounter, fwStatCounter));*/

    rc = internal_cpssDxChTamFwDebugCountersGet(devNum, fwCounter, fwStatCounter);

    /*CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fwCounter, fwStatCounter));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);*/

    return rc;
}

/**
 *  return the switch hw register address (from cm3 view) for specified counter type
 *  dbgPrintEn - used for debug. enable debug printing of the addresses.
 */
static GT_U32 getHwRegAddrForTamCounter
(
    IN   GT_U8                          devNum,
    IN   CPSS_DXCH_TAM_COUNTER_ID_STC   counter,
    IN   GT_BOOL                        dbgPrintEn
)
{
    GT_U32 hwRegAddr, txqPortNum, portRxdmaNum;

    switch (counter.type)
    {
        case CPSS_DXCH_TAM_INGRESS_GLOBAL_BUFFER_COUNTER_E:
            /* BM Global Buffer Counter Register 0x40000200 */
            hwRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->BM.BMCntrs.BMGlobalBufferCntr;
            if(dbgPrintEn) cpssOsPrintf("hwRegAddr=0x%08x\n",hwRegAddr);
            hwRegAddr &= (~EXT_REMAP_WIN_0_MASK);
            hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_0_BASE);

            break;
        case CPSS_DXCH_TAM_INGRESS_GLOBAL_MIRROR_BUFFER_COUNTER_E:
            /* Ingress Sniffer Buffers Counter Register 0x550A9204 */
            hwRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.muliticastFilterCntrs.ingrSnifferBuffersCntr;
            if(dbgPrintEn) cpssOsPrintf("hwRegAddr=0x%08x\n",hwRegAddr);
            hwRegAddr &= (~EXT_REMAP_WIN_1_MASK);
            hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_1_BASE);

            break;
        case CPSS_DXCH_TAM_INGRESS_PORT_BUFFER_COUNTER_E:
            /* SCDMA %n Buffers Allocation Counter 0x19001E00 */
            PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, counter.port, portRxdmaNum);
            hwRegAddr = PRV_DXCH_REG1_UNIT_RXDMA_MAC(devNum).scdmaDebug.portBuffAllocCnt[portRxdmaNum];
            if(dbgPrintEn) cpssOsPrintf("hwRegAddr=0x%08x\n",hwRegAddr);
            if ((hwRegAddr & 0xFF000000) == 0x99000000)
            {
                hwRegAddr &= (~EXT_REMAP_WIN_3_MASK);
                hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_3_BASE);
            }
            else
            {
                hwRegAddr &= (~EXT_REMAP_WIN_2_MASK);
                hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_2_BASE);
            }

            break;
        case CPSS_DXCH_TAM_INGRESS_PG_TC_BUFFER_COUNTER_E:
            /* PFC Group of Ports <p> TC <t> Counter Register (p=0-0, t=0-7) 0x42000D00 */
            hwRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.pfc.PFCGroupOfPortsTCCntr[0][counter.queue];
            if(dbgPrintEn) cpssOsPrintf("hwRegAddr=0x%08x\n",hwRegAddr);
            hwRegAddr &= (~EXT_REMAP_WIN_0_MASK);
            hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_0_BASE);

            break;
        case CPSS_DXCH_TAM_EGRESS_GLOBAL_BUFFER_COUNTER_E:
            /* Total Buffers Counter Register 0x550A4004 */
            hwRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.tailDropCntrs.totalBuffersCntr;
            if(dbgPrintEn) cpssOsPrintf("hwRegAddr=0x%08x\n",hwRegAddr);
            hwRegAddr &= (~EXT_REMAP_WIN_1_MASK);
            hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_1_BASE);

            break;
        case CPSS_DXCH_TAM_EGRESS_GLOBAL_MULTI_BUFFER_COUNTER_E:
            /* Multicast Buffers Counter Register 0x550A9210 */
            hwRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.muliticastFilterCntrs.mcBuffersCntr;
            if(dbgPrintEn) cpssOsPrintf("hwRegAddr=0x%08x\n",hwRegAddr);
            hwRegAddr &= (~EXT_REMAP_WIN_1_MASK);
            hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_1_BASE);

            break;
        case CPSS_DXCH_TAM_EGRESS_GLOBAL_MIRROR_BUFFER_COUNTER_E:
            /* Egress Sniffer Buffers Counter Register 0x550A9208 */
            hwRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.muliticastFilterCntrs.egrSnifferBuffersCntr;
            if(dbgPrintEn) cpssOsPrintf("hwRegAddr=0x%08x\n",hwRegAddr);
            hwRegAddr &= (~EXT_REMAP_WIN_1_MASK);
            hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_1_BASE);

            break;
        case CPSS_DXCH_TAM_EGRESS_PORT_BUFFER_COUNTER_E:
            /* Port<n> Buffers Counter Register (n=0-71) 0x550A4130 (n=72-575) 0x550B2800 */
            PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXQ_PORT_GET_MAC(devNum, counter.port, txqPortNum);
            hwRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.tailDropCntrs.portBuffersCntr[txqPortNum];
            if(dbgPrintEn) cpssOsPrintf("hwRegAddr=0x%08x\n",hwRegAddr);
            hwRegAddr &= (~EXT_REMAP_WIN_1_MASK);
            hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_1_BASE);

            break;
        case CPSS_DXCH_TAM_EGRESS_QUEUE_BUFFER_COUNTER_E:
            /* Queue Maintenance Buffers Tail Drop Queue Buffer Counters 0x550D0000 */
            /* There are 576 counters (lines), selected by {port[6:0], prio[2:0]}, port=0-71, prio=0-7 */
            hwRegAddr = 0x000D0000;/*prvCpssDxChTableBaseAddrGet(devNum, PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E, NULL);*/
            PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXQ_PORT_GET_MAC(devNum, counter.port, txqPortNum);
            hwRegAddr += 0x4 * ((txqPortNum << 3) + counter.queue);
            if(dbgPrintEn) cpssOsPrintf("hwRegAddr=0x%08x\n",hwRegAddr);
            hwRegAddr &= (~EXT_REMAP_WIN_1_MASK);
            hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_1_BASE);


            break;
        case CPSS_DXCH_TAM_EGRESS_SHARED_POOL_BUFFER_COUNTER_E:
            /* Priority<t> Shared Buffers Counter Register (t=0-7) 0x550A4290 */
            hwRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.tailDropCntrs.priorityBuffersCntr[counter.id];
            if(dbgPrintEn) cpssOsPrintf("hwRegAddr=0x%08x\n",hwRegAddr);
            hwRegAddr &= (~EXT_REMAP_WIN_1_MASK);
            hwRegAddr += (EXT_REMAP_BASE+EXT_REMAP_WIN_1_BASE);

            break;
        case CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E: /* this is not a hw counter */
        default:
            return 0; /* GT_BAD_PARAM */
    }

    if(dbgPrintEn) cpssOsPrintf("hwRegAddr(from cm3 view)=0x%08x\n",hwRegAddr);
    return hwRegAddr;
}

#if 0
/**
 *  return the counter type for specified switch hw register address
 */
static CPSS_DXCH_TAM_COUNTER_ID_STC getTamCounterForHwRegAddr
(
    IN   GT_U8                          devNum,
    IN   GT_U32                         regAddr
)
{
    CPSS_DXCH_TAM_COUNTER_ID_STC counter;

    counter.id = 0;
    counter.port = 0;
    counter.queue = 0;

    if(regAddr == (PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->BM.BMCntrs.BMGlobalBufferCntr + FW_WIN_0))
        /* BM Global Buffer Counter Register 0x40000200 */
        counter.type = CPSS_DXCH_TAM_INGRESS_GLOBAL_BUFFER_COUNTER_E;

    else if(regAddr == (PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.muliticastFilterCntrs.ingrSnifferBuffersCntr + FW_WIN_1))
        /* Ingress Sniffer Buffers Counter Register 0x550A9204 */
        counter.type = CPSS_DXCH_TAM_INGRESS_GLOBAL_MIRROR_BUFFER_COUNTER_E;

    /* TBD
        counter.type = CPSS_DXCH_TAM_INGRESS_PORT_BUFFER_COUNTER_E;

        break;
    */

    else if((regAddr & 0xfffffff0) ==
            (FW_WIN_0 + 0x02000000 + (PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.pfc.PFCGroupOfPortsTCCntr[0][0]))){
        /* PFC Group of Ports <p> TC <t> Counter Register (p=0-0, t=0-7) 0x42000D00 */
        /* 00000d00+0x20*p + 0x4*t*/
        counter.type = CPSS_DXCH_TAM_INGRESS_PG_TC_BUFFER_COUNTER_E;
        counter.queue = (regAddr & 0x000000ff) / 4;
    }

    else if(regAddr == (PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.tailDropCntrs.totalBuffersCntr + FW_WIN_1))
        /* Total Buffers Counter Register 0x550A4004 */
        counter.type = CPSS_DXCH_TAM_EGRESS_GLOBAL_BUFFER_COUNTER_E;

    else if(regAddr == (PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.muliticastFilterCntrs.mcBuffersCntr + FW_WIN_1))
        /* Multicast Buffers Counter Register 0x550A9210 */
        counter.type = CPSS_DXCH_TAM_EGRESS_GLOBAL_MULTI_BUFFER_COUNTER_E;

    else if(regAddr == (PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.muliticastFilterCntrs.egrSnifferBuffersCntr + FW_WIN_1))
        /* Egress Sniffer Buffers Counter Register 0x550A9208 */
        counter.type = CPSS_DXCH_TAM_EGRESS_GLOBAL_MIRROR_BUFFER_COUNTER_E;

    else if () {
        /* Port<n> Buffers Counter Register (n=0-71) 0x550A4130 (n=72-575) 0x550B2800 */
        /* 0-71: 0x000a4130+n*0x4
           72-575: 0x000b2800+n*0x4
        */
                hwRegAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.tailDropCntrs.portBuffersCntr[counter.port];
                hwRegAddr &= FW_WIN_1_MASK;
                hwRegAddr += FW_WIN_1;

                counter.type = CPSS_DXCH_TAM_EGRESS_PORT_BUFFER_COUNTER_E;
                counter.port = ;
    }

    else if((regAddr & FFFF FC00) == (0x000D0000 + FW_WIN_1)){
            /* Queue Maintenance Buffers Tail Drop Queue Buffer Counters 0x550D0000 */
            /* There are 576 counters (lines), selected by {port[6:0], prio[2:0]}, port=0-71, prio=0-7 */
            counter.type = CPSS_DXCH_TAM_EGRESS_QUEUE_BUFFER_COUNTER_E;
            counter.port = ((regAddr >> 3) & 0x0000007f);
            counter.queue = (regAddr & 0x00000007);
    }
    else if((regAddr & 0xffffff00) ==
            ((PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.queue.tailDrop.tailDropCntrs.priorityBuffersCntr[0] + FW_WIN_1) & 0xffffff00)) {
        /* Priority<t> Shared Buffers Counter Register (t=0-7) 0x550A4290 */
        /* 0x000a4290+t*0x4 */
        counter.type = CPSS_DXCH_TAM_EGRESS_SHARED_POOL_BUFFER_COUNTER_E;
        counter.queue = (regAddr - 0x000A4290 - FW_WIN_1) / 4;
        }

    return counter;
}
#endif

/**
* @internal internal_cpssDxChTamSnapshotSet function
* @endinternal
*
* @brief  Create snaphot of measured resources for atomic operation.
*         Snapshot #0 has high priority.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] snapshotId            - snapshot id.
* @param[in] listSize              - number of counters in countersArray
*                                    0 - delete snapshot
* @param[in] countersPtr           - array of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_VALUE             - on invalid snapshotId
* @retval GT_NO_RESOURCE           - when listSize > freeCounters
* @retval GT_ALREADY_EXIST         - when snapshotId already exists
*                                    or When try to delete snapshot with existing histogram or microburst
*
* @note   Release restriction: CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E cannot be a snapshot counter.
*
*/
static GT_STATUS internal_cpssDxChTamSnapshotSet
(
    IN   GT_U8                          devNum,
    IN   GT_U32                         snapshotId,
    IN   GT_U32                         listSize,
    IN   CPSS_DXCH_TAM_COUNTER_ID_STC   *counters
)
{
    GT_STATUS   rc;
    GT_U32      ii, word, address, value, counterIdx;
    GT_U32      snapshotDbAddr, counterDbAddr, microburstDbAddr;
    GT_U32      firstCounterAddr, lastCounterAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    /* check if it is a valid snapshot id */
    if (snapshotId >= PRV_CPSS_DXCH_TAM_FW_SNAPSHOTS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* calculate the address for this snapshot */
    snapshotDbAddr = tamSnapshotDbStartAddr + (snapshotId * sizeof(PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC));

    /* if(listSize==0) ==> delete the snapshot */
    /* (zero the counter db fields and counter shadow. and snapshot fields and snapshots shadow. */
    if (listSize == 0)
    {
        /* read the snapshot firstCounter value */
        rc = prvCpssDxChTamSharedMemRead(devNum, snapshotDbAddr, &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        firstCounterAddr = (value & SNAPSHOT_DB_FIRST_COUNTER_MASK) * 4;
        if (firstCounterAddr == 0)
        {
            /* the snapshot already free. nothing to do */
            return GT_OK;
        }

        /* check if some counter in the snapshot has histogram for it */
        for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_COUNTERS; ii++)
        {
            if ((countersShadowDb[ii].snapshotId == snapshotId) && (countersShadowDb[ii].histogramId != 0xFF))
            {
                /* cannot delete the snapshot before the histogram will be deleted */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
            }
        }

        /* check if some microburst defined for counter from this snapshot */
        for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_MICROBURSTS; ii++)
        {
            /* calculate the address for microburst */
            microburstDbAddr = tamMicroburstDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC));

            /* read the microburst counter value */
            rc = prvCpssDxChTamSharedMemRead(devNum, microburstDbAddr, &value);
            if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            if ((value & MICROBURST_DB_COUNTER_MASK) != 0) /* check if microburst valid*/
            {
                if ((value >> 24) == snapshotId) /* MICROBURST_DB_SNAPSHOT_ID_MASK*/
                {
                    /* cannot delete the snapshot before the microburst will be deleted */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
                }
            }
        }

        /* now delete the snapshot */

        /* update snapshot db */
        /* zero the firstCounter field & statBufNum field */
        rc = prvCpssDxChTamSharedMemWrite(devNum, snapshotDbAddr, 0);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* go over all the snapshot counters list, zero each counter db and counter shadow */
        value = firstCounterAddr;
        do
        {
            /* take the counter db address */
            counterDbAddr = value;

            counterIdx = (counterDbAddr - tamCountersDbStartAddr) / sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC);

            /* zero the counter shadow */
            cpssOsMemSet(&countersShadowDb[counterIdx].counter, 0, sizeof(CPSS_DXCH_TAM_COUNTER_ID_STC));
            countersShadowDb[counterIdx].snapshotId = 0xFF;
            countersShadowDb[counterIdx].histogramId = 0xFF;

            /* zero the counter register address in the counterDb entry */
            rc = prvCpssDxChTamSharedMemWrite(devNum, counterDbAddr, 0);
            if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            /* zero all the statistics */
            /* the size of staistics should be in full words */
            address = counterDbAddr + COUNTER_DB_STAT_0_OFFSET;
            for (word = 0; word < ALL_STATISTICS_SIZE_IN_WORDS; word++)
            {
                value = 0;

                /* the default value for min field is 0xFFFFFFFF */
                if ((word == 1) || (word == 7)) value = 0xFFFFFFFF;

                rc = prvCpssDxChTamSharedMemWrite(devNum, address, value);
                if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                address += 4;
            }

            /* read the next counter address (will be used in the next loop) */
            rc = prvCpssDxChTamSharedMemRead(devNum,
                                             counterDbAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                             &value);
            if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        } while (value != 0); /* if(value==0) it means that it is the end of the snapshot counters list */

        /* update snapshot shadow */
        snapshotShadowDb[snapshotId].numOfCounters = 0;

        /* add the snapshot list of counters to the start of the free counters list */
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          counterDbAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                          freeCounterFirstAddr);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        freeCounterFirstAddr = firstCounterAddr;

        /* update the free counters number */
        freeCountersNum = (freeCountersNum + ii + 1);

        return GT_OK;
    } /* end of delete */

    /* check if this snapshot id is free. read the snapshot firstCounter value */
    rc = prvCpssDxChTamSharedMemRead(devNum, snapshotDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    value &= SNAPSHOT_DB_FIRST_COUNTER_MASK;
    if (value != 0)
    {
        /* snapshot not free */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    /* check if the pointer is valid */
    CPSS_NULL_PTR_CHECK_MAC(counters);

    /* check if we have enought space for all counters */
    if (listSize > freeCountersNum)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);

    /* save the first free counter to write it later to the snapshot table */
    firstCounterAddr = freeCounterFirstAddr;

    /*  */
    counterDbAddr = freeCounterFirstAddr;
    lastCounterAddr = freeCounterFirstAddr; /*initialize it*/

    /* add counters to the DB */
    for (ii = 0; ii < listSize; ii++)
    {
        counterIdx = (counterDbAddr - tamCountersDbStartAddr) / sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC);
        /* save the counter in the shadow */
        cpssOsMemCpy(&countersShadowDb[counterIdx].counter, &counters[ii], sizeof(CPSS_DXCH_TAM_COUNTER_ID_STC));
        countersShadowDb[counterIdx].snapshotId = (GT_U8)snapshotId;

        /* find the counter register address for given counter type */
        value = getHwRegAddrForTamCounter(devNum, counters[ii], GT_FALSE);
        /* write the counter register address to the counterDb entry */
        rc = prvCpssDxChTamSharedMemWrite(devNum, counterDbAddr, value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* if it is the last counter - save the counter address to update the next pointer after the loop */
        if (ii == (listSize - 1)) lastCounterAddr = counterDbAddr;

        /* read the next counter address */
        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         counterDbAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                         &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* go to next counter address */
        counterDbAddr = value;

    } /* for (ii = 0; ii < listSize; ii++) */

    /* the last counter in snapshot points to NULL */
    rc = prvCpssDxChTamSharedMemWrite(devNum,
                                      lastCounterAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                      0);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* update the first free idx */
    freeCounterFirstAddr = counterDbAddr;

    /* update the number of free counters */
    freeCountersNum -= listSize;

    /* update snapshot db */
    /* zero the statBufNum field and update the firstCounter field */
    value = (firstCounterAddr / 4) & SNAPSHOT_DB_FIRST_COUNTER_MASK;
    rc = prvCpssDxChTamSharedMemWrite(devNum, snapshotDbAddr, value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* update snapshot shadow */
    snapshotShadowDb[snapshotId].numOfCounters = (GT_U8)listSize;

    return GT_OK;
}

/**
* @internal cpssDxChTamSnapshotSet function
* @endinternal
*
* @brief  Create snaphot of measured resources for atomic operation.
*         Snapshot #0 has high priority.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] snapshotId            - snapshot id.
* @param[in] listSize              - number of counters in countersArray
*                                    0 - delete snapshot
* @param[in] countersArray         - array of counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_VALUE             - on invalid snapshotId
* @retval GT_NO_RESOURCE           - when listSize > freeCounters
* @retval GT_ALREADY_EXIST         - when snapshotId already exists
*                                    or When try to delete snapshot with existing histogram or microburst
*
* @note   Release restriction: CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E cannot be a snapshot counter.
*
*/
GT_STATUS cpssDxChTamSnapshotSet
(
    IN   GT_U8                          devNum,
    IN   GT_U32                         snapshotId,
    IN   GT_U32                         listSize,
    IN   CPSS_DXCH_TAM_COUNTER_ID_STC   countersArray[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamSnapshotSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, snapshotId, listSize, countersArray));

    rc = internal_cpssDxChTamSnapshotSet(devNum, snapshotId, listSize, countersArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, snapshotId, listSize, countersArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTamSnapshotGet function
* @endinternal
*
* @brief  Get snapshot configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] snapshotId            - snapshot id.
* @param[in,out] listSize          - in - countersPtr array size
*                                    out - number of counters for this snapshot
* @param[out] countersPtr          - array of counters. Pointer to preallocated list (by the caller).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - if listSize==0
* @retval GT_BAD_VALUE             - on not valid snapshotId
* @retval GT_NO_SUCH               - if snapshot free
* @retval GT_BAD_SIZE              - if listSize < number of counters in this snapshot
*
* @note   Release restriction: CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E cannot be a snapshot counter.
*
*/
static GT_STATUS internal_cpssDxChTamSnapshotGet
(
    IN      GT_U8                          devNum,
    IN      GT_U32                         snapshotId,
    INOUT   GT_U32                         *listSize,
    OUT     CPSS_DXCH_TAM_COUNTER_ID_STC   *countersPtr
)
{
    GT_STATUS   rc;
    GT_U32      ii, value, snapshotDbAddr, firstCounterAddr, counterDbAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(listSize);
    CPSS_NULL_PTR_CHECK_MAC(countersPtr);

    /* check if it is valid listSize */
    if (*listSize == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* check if it is a valid snapshot id */
    if (snapshotId >= PRV_CPSS_DXCH_TAM_FW_SNAPSHOTS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* check if input array size is long enougth */
    if (snapshotShadowDb[snapshotId].numOfCounters > *listSize)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);

    /* calculate the address for this snapshot */
    snapshotDbAddr = tamSnapshotDbStartAddr + (snapshotId * sizeof(PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC));

    /* read the snapshot firstCounter value */
    rc = prvCpssDxChTamSharedMemRead(devNum, snapshotDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    firstCounterAddr = (value & SNAPSHOT_DB_FIRST_COUNTER_MASK) * 4;
    if (firstCounterAddr == 0)
    {
        /* the snapshot free */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }

    /*  */
    counterDbAddr = firstCounterAddr;

    for (ii = 0; ii < (*listSize); ii++)
    {
        /* copy the counter from the shadow to the output */
        cpssOsMemCpy(&countersPtr[ii],
                     &countersShadowDb[(counterDbAddr - tamCountersDbStartAddr) / sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC)].counter,
                     sizeof(CPSS_DXCH_TAM_COUNTER_ID_STC));

        /* read the next counter address */
        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         counterDbAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                         &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        if (value == 0)
        {
            /* this is the last counter in this snapshot */
            /* update the real list size and return from function */
            *listSize = ii + 1;
            return GT_OK;
        }

        /* go to next counter address */
        counterDbAddr = value;
    } /* for (ii = 0; ii < (*listSize); ii++) */

    /* if arrive to this place so ii==*listSize
     * it means that list is full but we dont arrive to the last counter in the snapshot
     * we should not arrive to here
     */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChTamSnapshotGet function
* @endinternal
*
* @brief  Get snapshot configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] snapshotId            - snapshot id.
* @param[in,out] listSize          - in - countersPtr array size
*                                    out - number of counters for this snapshot
* @param[out] countersArray        - array of counters. Pointer to preallocated list (by the caller).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - if listSize==0
* @retval GT_BAD_VALUE             - on not valid snapshotId
* @retval GT_NO_SUCH               - if snapshot free
* @retval GT_BAD_SIZE              - if listSize < number of counters in this snapshot
*
* @note   Release restriction: CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E cannot be a snapshot counter.
*
*/
GT_STATUS cpssDxChTamSnapshotGet
(
    IN      GT_U8                          devNum,
    IN      GT_U32                         snapshotId,
    INOUT   GT_U32                         *listSize,
    OUT     CPSS_DXCH_TAM_COUNTER_ID_STC   countersArray[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamSnapshotGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, snapshotId, listSize, countersArray));

    rc = internal_cpssDxChTamSnapshotGet(devNum, snapshotId, listSize, countersArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, snapshotId, listSize, countersArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTamSnapshotStatGet function
* @endinternal
*
* @brief  Get snaphot data of measured resources for atomic operation.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] snapshotId            - snapshot id.
* @param[in,out] numOfStats        - in - statsDataPtr array size
*                                    out - number of counters for this snapshot.
* @param[out] statsDataPtr         - array of counters data. Pointer to preallocated list (by the caller).
* @param[in,out] numOfHistograms   - in - histValuePtr array size
*                                    out - number of histograms for this snapshot.
* @param[out] histValuePtr         - array of histogram data. Pointer to preallocated list (by the caller).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - if numOfStats==0
* @retval GT_BAD_VALUE             - on wrong snapshotId
* @retval GT_NO_SUCH               - if snapshot free
* @retval GT_BAD_SIZE              - if numOfStats < number of counters in this snapshot
*
*/
static GT_STATUS internal_cpssDxChTamSnapshotStatGet
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               snapshotId,
    INOUT  GT_U32                               *numOfStats,
    OUT    CPSS_DXCH_TAM_COUNTER_STATISTICS_STC *statsDataPtr,
    INOUT  GT_U32                               *numOfHistograms,
    OUT    CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC    *histValuePtr
)
{
    GT_STATUS   rc;
    GT_U32      ii, jj, word, address, value;
    GT_U32      snapshotDbAddr, histogramDbAddr, firstCounterAddr, counterDbAddr;
    GT_U32      statBufNum, inNumOfHist, counterId;
    GT_U64      statSum, binSum, val0, val1;
    GT_U64      binValue[CPSS_DXCH_TAM_HISTOGRAM_BIN_VALUES];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(numOfStats);
    CPSS_NULL_PTR_CHECK_MAC(statsDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfHistograms);
    if (*numOfHistograms != 0) CPSS_NULL_PTR_CHECK_MAC(histValuePtr);

    /* save the input histograms array size for future use */
    inNumOfHist = *numOfHistograms;
    *numOfHistograms = 0;

    /* check if it is valid numOfStats */
    if (*numOfStats == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* check if it is a valid snapshot id */
    if (snapshotId >= PRV_CPSS_DXCH_TAM_FW_SNAPSHOTS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* check if statsDataPtr array size is long enougth */
    if (snapshotShadowDb[snapshotId].numOfCounters > *numOfStats)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);

    /* calculate the address for this snapshot */
    snapshotDbAddr = tamSnapshotDbStartAddr + (snapshotId * sizeof(PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC));

    /* read the snapshot firstCounter value and the statBuff field */
    rc = prvCpssDxChTamSharedMemRead(devNum, snapshotDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    firstCounterAddr = (value & SNAPSHOT_DB_FIRST_COUNTER_MASK) * 4;
    if (firstCounterAddr == 0)
    {
        /* the snapshot free */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }

    statBufNum = value & SNAPSHOT_DB_STAT_BUFF_MASK;

    /* update the statBufNum */
    if (statBufNum) value &= 0x0000FFFF; /* zero the statBufNum field (bits 16-23) */
    else            value |= 0x00010000; /* write value 1 to statBufNum field (bits 16-23) */

    /* write the snapshot statBuff field & firstCounter value */
    rc = prvCpssDxChTamSharedMemWrite(devNum, snapshotDbAddr, value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* go over all the counters in snapshot */
    counterDbAddr = firstCounterAddr;
    for (ii = 0; ii < (*numOfStats); ii++)
    {
        /* calculate the address of statistic 0 or 1 */
        if (statBufNum == 0)    address = counterDbAddr + COUNTER_DB_STAT_0_OFFSET;
        else                    address = counterDbAddr + COUNTER_DB_STAT_1_OFFSET;

        /* read the statistics */
        /* read the "last" field */
        rc = prvCpssDxChTamSharedMemRead(devNum, address, &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        statsDataPtr[ii].current.l[0] = value;

        /* read the "min" field */
        rc = prvCpssDxChTamSharedMemRead(devNum, address + 4, &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        statsDataPtr[ii].minimum.l[0] = value;

        /* read the "max" field */
        rc = prvCpssDxChTamSharedMemRead(devNum, address + 8, &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        statsDataPtr[ii].peak.l[0] = value;

        /* read the "count" field */
        rc = prvCpssDxChTamSharedMemRead(devNum, address + 12, &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* read the "sum" two fields */
        /*  read u64 lsb part */
        rc = prvCpssDxChTamSharedMemRead(devNum, address + 16, &statSum.l[0]);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /*  read u64 msb part */
        rc = prvCpssDxChTamSharedMemRead(devNum, address + 20, &statSum.l[1]);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* calculate the average */
        statsDataPtr[ii].average = prvCpssMathDiv64By32(statSum, value, NULL);

        /* zero the statistics (that was already read) */
        for (word = 0; word < STATISTICS_SIZE_IN_WORDS; word++)
        {
            value = 0;

            /* set the default nim value to maximal value */
            if (word == 1) value = 0xFFFFFFFF;

            rc = prvCpssDxChTamSharedMemWrite(devNum, address, value);
            if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            address += 4;
        }

        /* calculate the counterId to check the counters shadow */
        counterId = (counterDbAddr - tamCountersDbStartAddr) / sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC);

        /* if input histValuePtr valid and histogram for this counter exists -
         * read the histogram value and update the output */
        if ((histValuePtr != NULL) &&
            (countersShadowDb[counterId].histogramId != 0xFF) &&
            (*numOfHistograms < inNumOfHist))
        {
            /* take the histogramId from shadow & update the output */
            histValuePtr[*numOfHistograms].histogramId = countersShadowDb[counterId].histogramId;

            /* calculate the address for this histogram */
            histogramDbAddr = tamHistogramDbStartAddr + (countersShadowDb[counterId].histogramId * sizeof(PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC));

            /* read the histogram statBuff field */
            rc = prvCpssDxChTamSharedMemRead(devNum, histogramDbAddr, &value);
            if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            statBufNum = value & HISTOGRAM_DB_STAT_BUFF_MASK;
            /*TBD maybe recheck if the histogram type and counter address is valid?*/

            /* update the statBufNum */
            if (statBufNum) value &= 0xFF00FFFF; /* zero the statBufNum field (bits 16-23) */
            else            value |= 0x00010000; /* write value 1 to statBufNum field (bits 16-23) */

            /* write the histogram statBuff field & counterPtr value & counterType */
            rc = prvCpssDxChTamSharedMemWrite(devNum, histogramDbAddr, value);
            if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            /* calculate the address of statistic 0 or 1 */
            if (statBufNum == 0)    address = histogramDbAddr + HISTOGRAM_DB_BIN_VAL_STAT_0_OFFSET;
            else                    address = histogramDbAddr + HISTOGRAM_DB_BIN_VAL_STAT_1_OFFSET;

            /* read the statistics from histogram db  and save the values */
            binSum.l[0] = binSum.l[1] = 0;
            for (jj = 0; jj < CPSS_DXCH_TAM_HISTOGRAM_BIN_VALUES; jj++)
            {
                rc = prvCpssDxChTamSharedMemRead(devNum,
                                                 address + (jj * HISTOGRAM_DB_BIN_VAL_BYTES),
                                                 &value);
                if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                binValue[jj].l[0] = value;

                rc = prvCpssDxChTamSharedMemRead(devNum,
                                                 address + (jj * HISTOGRAM_DB_BIN_VAL_BYTES) + 4,
                                                 &value);
                if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                binValue[jj].l[1] = value;

                /* calculate the sum of the binValues to calculate later the precents */
                binSum = prvCpssMathAdd64(binSum, binValue[jj]);
            }

            /* calculate the precents and update the output values */
            for (jj = 0; jj < CPSS_DXCH_TAM_HISTOGRAM_BIN_VALUES; jj++)
            {
                if ((binSum.l[0] != 0) || (binSum.l[1] != 0))
                {
                    /* multiple the binValue by 100 */
                    val1 = prvCpssMathMul64(binValue[jj].l[1], 100);
                    val0 = prvCpssMathMul64(binValue[jj].l[0], 100);
                    val0.l[1] += val1.l[0];

                    /* divide val0 by the sum of binValues */
                    rc = prvCpssMathDiv64(val0, binSum, &histValuePtr[*numOfHistograms].value[jj], NULL);
                    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);                                                  
                }
                else
                {
                    histValuePtr[*numOfHistograms].value[jj].l[0] = 0;
                    histValuePtr[*numOfHistograms].value[jj].l[1] = 0;
                }
            }

            /* update the output */
            *numOfHistograms = (*numOfHistograms + 1);

            /* zero the histogram bin values (that was already read) */
            for (word = 0; word < (PRV_CPSS_DXCH_TAM_FW_BINS * 2); word++)
            {
                rc = prvCpssDxChTamSharedMemWrite(devNum, address, 0);
                if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                address += 4;
            }
        } /* if (histValuePtr != NULL) */

        /* read the next counter address */
        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         counterDbAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                         &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        if (value == 0)
        {
            /* this is the last counter in this snapshot */
            /* update the real list size and return from function */
            *numOfStats = ii + 1;
            return GT_OK;
        }

        /* go to next counter address */
        counterDbAddr = value;

    } /* for (ii = 0; ii < (*numOfStats); ii++) */

    /* if arrive to this place so ii==*listSize
     * it means that list is full but we don't arrive to the last counter in the snapshot
     * dont should to arrive to here
     */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChTamSnapshotStatGet function
* @endinternal
*
* @brief  Get snaphot data of measured resources for atomic operation.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] snapshotId            - snapshot id.
* @param[in,out] numOfStats        - in - statsDataPtr array size
*                                    out - number of counters for this snapshot.
* @param[out] statsDataArray       - array of counters data. Pointer to preallocated list (by the caller).
* @param[in,out] numOfHistograms   - in - histValuePtr array size
*                                    out - number of histograms for this snapshot.
* @param[out] histValueArray       - array of histogram data. Pointer to preallocated list (by the caller).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - if numOfStats==0
* @retval GT_BAD_VALUE             - on wrong snapshotId
* @retval GT_NO_SUCH               - if snapshot free
* @retval GT_BAD_SIZE              - if numOfStats < number of counters in this snapshot
*
*/
GT_STATUS cpssDxChTamSnapshotStatGet
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               snapshotId,
    INOUT  GT_U32                               *numOfStats,
    OUT    CPSS_DXCH_TAM_COUNTER_STATISTICS_STC statsDataArray[],
    INOUT  GT_U32                               *numOfHistograms,
    OUT    CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC    histValueArray[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamSnapshotStatGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, snapshotId, numOfStats, statsDataArray, numOfHistograms, histValueArray));

    rc = internal_cpssDxChTamSnapshotStatGet(devNum, snapshotId, numOfStats, statsDataArray, numOfHistograms, histValueArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, snapshotId, numOfStats, statsDataArray, numOfHistograms, histValueArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTamMicroburstSet function
* @endinternal
*
* @brief  Create microburst. The microburst measurements will start after creation.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] microburstId          - microburst id.
* @param[in] tamCounterPtr         - Pointer to base counter for the microburst.
*                                    if null - delete the microburst
* @param[in] highWatermarkLevel    - Breach high watermark level for this counter.
* @param[in] lowWatermarkLevel     - Breach low watermark level for this counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note   Release restriction: we cannot create microburst on CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E.
*
*/
static GT_STATUS internal_cpssDxChTamMicroburstSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       microburstId,
    IN CPSS_DXCH_TAM_COUNTER_ID_STC *tamCounterPtr,
    IN GT_U32                       highWatermarkLevel,
    IN GT_U32                       lowWatermarkLevel
)
{
    GT_STATUS   rc;
    GT_U32      ii, value, word, address;
    GT_U32      microburstDbAddr, counterDbAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    /* check if it is a valid microburst id */
    if (microburstId >= PRV_CPSS_DXCH_TAM_FW_MICROBURSTS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* calculate the address for this microburst */
    microburstDbAddr = tamMicroburstDbStartAddr + (microburstId * sizeof(PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC));

    /* if(tamCounterPtr==0) ==> delete the microburst */
    /* zero the microburst db and shadow. */
    if (tamCounterPtr == NULL)
    {
        if (microburstShadowDb[microburstId].histogramId != 0xFF)
        {
            /* cannot delete the microburst before the histogram will be deleted */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
        }

        /* check if this microburst id is free. read the microburst counter value */
        rc = prvCpssDxChTamSharedMemRead(devNum, microburstDbAddr, &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        if ((value & MICROBURST_DB_COUNTER_MASK) == 0)
        {
            /* nothing to do microburst already free */
            return GT_OK;
        }

        /* zero the microburst counter value and the statBuff value. set snapshotId to default (0xFF). */
        value = 0xFF000000;
        rc = prvCpssDxChTamSharedMemWrite(devNum, microburstDbAddr, value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* zero the highwatermark field */
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          microburstDbAddr + MICROBURST_DB_HIGH_WATERMARK_OFFSET,
                                          0);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* zero the lowwatermark field */
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          microburstDbAddr + MICROBURST_DB_LOW_WATERMARK_OFFSET,
                                          0);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* zero the time field */
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          microburstDbAddr + MICROBURST_DB_TIME_START_OFFSET,
                                          0);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* zero the statistics */
        address = microburstDbAddr + MICROBURST_DB_STAT_0_OFFSET;
        for (word = 0; word < ALL_STATISTICS_SIZE_IN_WORDS; word++)
        {
            rc = prvCpssDxChTamSharedMemWrite(devNum, address, 0);
            if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            address += 4;
        }

        /* zero the timeStatus field and calcHistogram field */
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          microburstDbAddr + MICROBURST_DB_TIME_STATUS_TBD,
                                          0);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        return GT_OK;
    } /* end of delete */

    /* check if the watermark values valid */
    if(highWatermarkLevel < lowWatermarkLevel)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* check if this microburst id is free. read the microburst counter value */
    rc = prvCpssDxChTamSharedMemRead(devNum, microburstDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    value &= MICROBURST_DB_COUNTER_MASK;
    if (value != 0)
    {
        /* microburst not free */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    /* search the counters shadow for needed counter */
    for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_COUNTERS; ii++)
    {
        if(countersShadowDb[ii].counter.type == tamCounterPtr->type)
        {
            if ((countersShadowDb[ii].counter.id == tamCounterPtr->id) &&
                (countersShadowDb[ii].counter.port == tamCounterPtr->port) &&
                (countersShadowDb[ii].counter.queue == tamCounterPtr->queue))
            {
                /* we find counter equal to needed counter */
                break;
            }
        }
    }
    if (ii == PRV_CPSS_DXCH_TAM_FW_COUNTERS)
    {
        /* no such counter in countersDb. cannot create microburst */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* calculate the counterDb address to write it later to the microburstDb */
    counterDbAddr = tamCountersDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC));

    /* update microburst db */
    /* update the highwatermark field */
    rc = prvCpssDxChTamSharedMemWrite(devNum,
                                      microburstDbAddr + MICROBURST_DB_HIGH_WATERMARK_OFFSET,
                                      highWatermarkLevel);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* update the lowwatermark field */
    rc = prvCpssDxChTamSharedMemWrite(devNum,
                                      microburstDbAddr + MICROBURST_DB_LOW_WATERMARK_OFFSET,
                                      lowWatermarkLevel);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* zero the time field */
    rc = prvCpssDxChTamSharedMemWrite(devNum,
                                      microburstDbAddr + MICROBURST_DB_TIME_START_OFFSET,
                                      0);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* zero the statBufNum field and update the counterPtr field and set snapshotId field */
    value = (countersShadowDb[ii].snapshotId << 24) | ((counterDbAddr / 4) & MICROBURST_DB_COUNTER_MASK);
    rc = prvCpssDxChTamSharedMemWrite(devNum, microburstDbAddr, value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal cpssDxChTamMicroburstSet function
* @endinternal
*
* @brief  Create microburst. The microburst measurements will start after creation.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] microburstId          - microburst id.
* @param[in] tamCounterPtr         - Pointer to base counter for the microburst.
*                                    if null - delete the microburst
* @param[in] highWatermarkLevel    - Breach high watermark level for this counter.
* @param[in] lowWatermarkLevel     - Breach low watermark level for this counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note   Release restriction: we cannot create microburst on CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E.
*
*/
GT_STATUS cpssDxChTamMicroburstSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       microburstId,
    IN CPSS_DXCH_TAM_COUNTER_ID_STC *tamCounterPtr,
    IN GT_U32                       highWatermarkLevel,
    IN GT_U32                       lowWatermarkLevel
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamMicroburstSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, microburstId, tamCounterPtr, highWatermarkLevel, lowWatermarkLevel));

    rc = internal_cpssDxChTamMicroburstSet(devNum, microburstId, tamCounterPtr, highWatermarkLevel, lowWatermarkLevel);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, microburstId, tamCounterPtr, highWatermarkLevel, lowWatermarkLevel));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTamMicroburstGet function
* @endinternal
*
* @brief  Get microburst configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] microburstId          - microburst id.
* @param[out] tamCounterPtr        - Pointer to base counter for the microburst.
* @param[out] highWatermarkLevel   - Breach high watermark level for this counter.
* @param[out] lowWatermarkLevel    - Breach low watermark level for this counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChTamMicroburstGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          microburstId,
    OUT CPSS_DXCH_TAM_COUNTER_ID_STC    *tamCounterPtr,
    OUT GT_U32                          *highWatermarkLevel,
    OUT GT_U32                          *lowWatermarkLevel
)
{
    GT_STATUS   rc;
    GT_U32      microburstDbAddr, counterDbAddr, value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(tamCounterPtr);

    /* check if it is a valid microburst id */
    if (microburstId >= PRV_CPSS_DXCH_TAM_FW_MICROBURSTS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* calculate the address for this microburst */
    microburstDbAddr = tamMicroburstDbStartAddr + (microburstId * sizeof(PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC));

    /* read the microburst counter address */
    rc = prvCpssDxChTamSharedMemRead(devNum, microburstDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    counterDbAddr = (value & MICROBURST_DB_COUNTER_MASK) * 4;

    if (counterDbAddr == 0)
    {
        /* the microburst free */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }

    /* copy the counter from the shadow to the output */
    cpssOsMemCpy(tamCounterPtr,
                 &countersShadowDb[(counterDbAddr - tamCountersDbStartAddr) / sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC)].counter,
                 sizeof(CPSS_DXCH_TAM_COUNTER_ID_STC));

    /* read the highwatermark from microburstDb */
    rc = prvCpssDxChTamSharedMemRead(devNum,
                                     microburstDbAddr + MICROBURST_DB_HIGH_WATERMARK_OFFSET,
                                     highWatermarkLevel);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* read the lowwatermark from microburstDb */
    rc = prvCpssDxChTamSharedMemRead(devNum,
                                     microburstDbAddr + MICROBURST_DB_LOW_WATERMARK_OFFSET,
                                     lowWatermarkLevel);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal cpssDxChTamMicroburstGet function
* @endinternal
*
* @brief  Get microburst configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] microburstId          - microburst id.
* @param[out] tamCounterPtr        - Pointer to base counter for the microburst.
* @param[out] highWatermarkLevel   - Breach high watermark level for this counter.
* @param[out] lowWatermarkLevel    - Breach low watermark level for this counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamMicroburstGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          microburstId,
    OUT CPSS_DXCH_TAM_COUNTER_ID_STC    *tamCounterPtr,
    OUT GT_U32                          *highWatermarkLevel,
    OUT GT_U32                          *lowWatermarkLevel
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamMicroburstGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, microburstId, tamCounterPtr, highWatermarkLevel, lowWatermarkLevel));

    rc = internal_cpssDxChTamMicroburstGet(devNum, microburstId, tamCounterPtr, highWatermarkLevel, lowWatermarkLevel);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, microburstId, tamCounterPtr, highWatermarkLevel, lowWatermarkLevel));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTamMicroburstStatGet function
* @endinternal
*
* @brief  Get microburst data of measured resources.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] microburstId          - snapshot id.
* @param[out] statsDataPtr         - microburst statistics data.
* @param[out] histValuePtr         - histogram statistics data.
*                                    NULL - if no histogram for this microburst.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChTamMicroburstStatGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  microburstId,
    OUT CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC *statsDataPtr,
    OUT CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC       *histValuePtr
)
{
    GT_STATUS   rc;
    GT_U32      word, address, statBufNum, microburstDbAddr, histogramDbAddr, value, ii;
    GT_U64      statSum, statAverage;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(statsDataPtr);

    /* check if it is a valid microburst id */
    if (microburstId >= PRV_CPSS_DXCH_TAM_FW_MICROBURSTS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* calculate the address for this microburst */
    microburstDbAddr = tamMicroburstDbStartAddr + (microburstId * sizeof(PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC));

    /* read the microburst statBuff field */
    rc = prvCpssDxChTamSharedMemRead(devNum, microburstDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    statBufNum = value & MICROBURST_DB_STAT_BUFF_MASK;

    /* update the statBufNum */
    if (statBufNum) value &= 0xFF00FFFF; /* zero the statBufNum field (bits 16-23) */
    else            value |= 0x00010000; /* write value 1 to statBufNum field (bits 16-23) */

    /* write the microburst statBuff field & counterPtr value & snapshotId*/
    rc = prvCpssDxChTamSharedMemWrite(devNum, microburstDbAddr, value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* calculate the address of statistic 0 or 1 */
    if (statBufNum == 0)    address = microburstDbAddr + MICROBURST_DB_STAT_0_OFFSET;
    else                    address = microburstDbAddr + MICROBURST_DB_STAT_1_OFFSET;

    /* read the statistics */
    /* read the "last" field */
    rc = prvCpssDxChTamSharedMemRead(devNum, address, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    statsDataPtr->lastDuration = value;

    /* read the "min" field */
    rc = prvCpssDxChTamSharedMemRead(devNum, address + 4, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    statsDataPtr->shortestDuration = value;

    /* read the "max" field */
    rc = prvCpssDxChTamSharedMemRead(devNum, address + 8, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    statsDataPtr->longestDuration = value;

    /* read the "count" field */
    rc = prvCpssDxChTamSharedMemRead(devNum, address + 12, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    statsDataPtr->numMicrobursts = value;

    /* read the "sum" two fields */
    /*  read u64 lsb part */
    rc = prvCpssDxChTamSharedMemRead(devNum, address + 16, &statSum.l[0]);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /*  read u64 msb part */
    rc = prvCpssDxChTamSharedMemRead(devNum, address + 20, &statSum.l[1]);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* calculate the average */
    statAverage = prvCpssMathDiv64By32(statSum, statsDataPtr->numMicrobursts, NULL);
    statsDataPtr->averageDuration = statAverage.l[0];

    /* zero the statistics (that was already read) */
    for (word = 0; word < STATISTICS_SIZE_IN_WORDS; word++)
    {
        if((word == 1) || (word == 7)) value = 0xFFFFFFFF;
        else                           value = 0;
        rc = prvCpssDxChTamSharedMemWrite(devNum, address, value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        address += 4;
    }

    /* if histogramPtr not valid - not need to read histogram */
    if (histValuePtr == NULL)
    {
        return GT_OK;
    }

    /* check if we have any histogram for this microburst */
    if(microburstShadowDb[microburstId].histogramId == 0xFF)
    {
        /* no histogram defined. nothing to do */
        histValuePtr->histogramId = 0xFF;
        return GT_OK;
    }

    /* take the histogramId from shadow & update the output */
    histValuePtr->histogramId = microburstShadowDb[microburstId].histogramId;

    /* calculate the address for this histogram */
    histogramDbAddr = tamHistogramDbStartAddr + (microburstShadowDb[microburstId].histogramId * sizeof(PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC));

    /* read the histogram statBuff field */
    rc = prvCpssDxChTamSharedMemRead(devNum, histogramDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    statBufNum = value & HISTOGRAM_DB_STAT_BUFF_MASK;

    /* update the statBufNum */
    if (statBufNum) value &= 0xFF00FFFF; /* zero the statBufNum field (bits 16-23) */
    else            value |= 0x00010000; /* write value 1 to statBufNum field (bits 16-23) */

    /* write the histogram statBuff field & counterPtr value & counterType */
    rc = prvCpssDxChTamSharedMemWrite(devNum, histogramDbAddr, value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* calculate the address of statistic 0 or 1 */
    if (statBufNum == 0)    address = histogramDbAddr + HISTOGRAM_DB_BIN_VAL_STAT_0_OFFSET;
    else                    address = histogramDbAddr + HISTOGRAM_DB_BIN_VAL_STAT_1_OFFSET;

    /* read the statistics and update output values */
    for (ii = 0; ii < CPSS_DXCH_TAM_HISTOGRAM_BIN_VALUES; ii++)
    {
        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         address + (ii * HISTOGRAM_DB_BIN_VAL_BYTES),
                                         &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        histValuePtr->value[ii].l[0] = value;
        histValuePtr->value[ii].l[1] = 0;
    }

    /* zero the statistics (that was already read) */
    for (word = 0; word < (PRV_CPSS_DXCH_TAM_FW_BINS * 2); word++)
    {
        rc = prvCpssDxChTamSharedMemWrite(devNum, address, 0);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        address += 4;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTamMicroburstStatGet function
* @endinternal
*
* @brief  Get microburst data of measured resources.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] microburstId          - snapshot id.
* @param[out] statsDataPtr         - microburst statistics data.
* @param[out] histValuePtr         - histogram statistics data.
*                                    NULL - if no histogram for this microburst.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamMicroburstStatGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  microburstId,
    OUT CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC *statsDataPtr,
    OUT CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC       *histValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamMicroburstStatGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, microburstId, statsDataPtr, histValuePtr));

    rc = internal_cpssDxChTamMicroburstStatGet(devNum, microburstId, statsDataPtr, histValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, microburstId, statsDataPtr, histValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTamHistogramSet function
* @endinternal
*
* @brief  Configure a histogram bins and clear values.  The histogram measurements will start after creation.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] histogramId           - histogram id.
* @param[in] counterPtr            - counter to calculate the histogram.
*                                    if null - delete the histogram
* @param[in] binsPtr               - histogram configuration structure. bin3 > bin2 > bin1 > 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note   If the counter type is: CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E -
*         The histogram counts microburst time values. (Microburst defined by cpssDxChTamMicroburstSePI).
*         In this case the histogram statistics need to be get by cpssDxChTamMicroburstStatGet API.
* @note   For other counter types - The histogram counts counter values.
*         (Counters that were defined in cpssDxChTamSnapshotSet API).
*         In this case the histogram statistics need to be get by cpssDxChTamSnapshotStatGet API.
*
*/
static GT_STATUS internal_cpssDxChTamHistogramSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           histogramId,
    IN CPSS_DXCH_TAM_COUNTER_ID_STC     *counterPtr,
    IN CPSS_DXCH_TAM_HISTOGRAM_BIN_STC  *binsPtr
)
{
    GT_STATUS   rc;
    GT_U32      value, ii;
    GT_U32      histogramDbAddr, microOrCountDbAddr, histogramType, snapshotDbAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    /* check if it is a valid histogram id */
    if (histogramId >= PRV_CPSS_DXCH_TAM_FW_HISTOGRAMS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* calculate the address for this histogram */
    histogramDbAddr = tamHistogramDbStartAddr + (histogramId * sizeof(PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC));

    /* read the counter/microburst address */
    rc = prvCpssDxChTamSharedMemRead(devNum, histogramDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    microOrCountDbAddr = (value & HISTOGRAM_DB_COUNTER_MASK) * 4;
    histogramType = value >> 24;

    /* if(counterPtr==0) ==> delete the histogram */
    /* (don't zero the counter db fields and counter shadow. it will be zeroed/updated when will add a new snapshot) */
    if (counterPtr == NULL)
    {
        if(microOrCountDbAddr == 0)
        {
            /* this histogram id is already free - nothing to do */
            return GT_OK;
        }

        if (histogramType == PRV_CPSS_DXCH_TAM_HISTOGRAM_TYPE_MICROBURST_E)
        {
            /* calculate microburst index */
            ii = ((microOrCountDbAddr << 2) - tamMicroburstDbStartAddr) / sizeof(PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC);

            /* update the microburst shadow */
            microburstShadowDb[ii].histogramId = 0xFF;
        }
        else
        {
            /* calculate counter index */
            ii = ((microOrCountDbAddr << 2) - tamCountersDbStartAddr) / sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC);

            /* update the counter shadow */
            countersShadowDb[ii].histogramId = 0xFF;
        }

        /* zero snapshot address field */
        value = 0x0;
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          histogramDbAddr + HISTOGRAM_DB_SNAPSHOT_OFFSET,
                                          value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* zero the statBufNum field and counterPtr field and histogramType field */
        value = 0x0;
        rc = prvCpssDxChTamSharedMemWrite(devNum, histogramDbAddr, value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        return GT_OK;
    } /* end of delete */

    CPSS_NULL_PTR_CHECK_MAC(binsPtr);

    if (microOrCountDbAddr != 0)
    {
        /* histogram not free */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    /* check if the bin values valid */
    if((binsPtr->bin[2].l[0] < binsPtr->bin[1].l[0]) || (binsPtr->bin[1].l[0] < binsPtr->bin[0].l[0]))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* check if this histogram defined on microburst */
    if (counterPtr->type == CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E)
    {
        /* check if it is a valid microburst id */
        if (counterPtr->id >= PRV_CPSS_DXCH_TAM_FW_MICROBURSTS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

        /* calculate the address for this microburst */
        microOrCountDbAddr = tamMicroburstDbStartAddr + (counterPtr->id * sizeof(PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC));

        /* check if this microburst valid and used */
        rc = prvCpssDxChTamSharedMemRead(devNum, microOrCountDbAddr, &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        if((value & MICROBURST_DB_COUNTER_MASK) == 0)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

        /* check in the microburst shadow if this microburst not already used by another histogram */
        if( microburstShadowDb[counterPtr->id].histogramId != 0xFF)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

        /* update the microburst shadow */
        microburstShadowDb[counterPtr->id].histogramId = (GT_U8)histogramId;

        snapshotDbAddr = 0;
        histogramType = PRV_CPSS_DXCH_TAM_HISTOGRAM_TYPE_MICROBURST_E;
    } /* end of "histogram defined on microburst" */
    else
    {
        /* the histogram need to be defined on regular counter type */

        /* search the counters shadow for needed counter */
        for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_COUNTERS; ii++)
        {
            if(countersShadowDb[ii].counter.type == counterPtr->type)
            {
                if ((countersShadowDb[ii].counter.id == counterPtr->id) &&
                    (countersShadowDb[ii].counter.port == counterPtr->port) &&
                    (countersShadowDb[ii].counter.queue == counterPtr->queue))
                {
                    /* we find counter equal to needed counter */
                    break;
                }
            }
        }

        /* if such counter not found return error */
        if (ii == PRV_CPSS_DXCH_TAM_FW_COUNTERS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

        /* check in the counters shadow if this counter not already used by another histogram */
        if( countersShadowDb[ii].histogramId != 0xFF)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

        /* update the counters shadow */
        countersShadowDb[ii].histogramId = (GT_U8)histogramId;

        /* calculate the counterDb address to write it later to the microburstDb */
        microOrCountDbAddr = tamCountersDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC));
        snapshotDbAddr = tamSnapshotDbStartAddr + (countersShadowDb[ii].snapshotId * sizeof(PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC));
        histogramType = PRV_CPSS_DXCH_TAM_HISTOGRAM_TYPE_COUNTER_E;
    } /* end of "histogram defined on counter" */

    /* update histogram db */
    for (ii = 0; ii < CPSS_DXCH_TAM_HISTOGRAM_BIN_LEVELS; ii++)
    {
        /* update the bins fields */
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          histogramDbAddr + HISTOGRAM_DB_BIN_LEVELS_OFFSET +
                                          (ii * HISTOGRAM_DB_BIN_LEVEL_BYTES),
                                          binsPtr->bin[ii].l[0]);

        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* clean the statistics */

    /* update snapshot address field */
    rc = prvCpssDxChTamSharedMemWrite(devNum,
                                      histogramDbAddr + HISTOGRAM_DB_SNAPSHOT_OFFSET,
                                      snapshotDbAddr);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* zero the statBufNum field and update the counterPtr field and update the histogramType field */
    value = (((histogramType << 24) & HISTOGRAM_DB_TYPE_MASK) | ((microOrCountDbAddr / 4) & HISTOGRAM_DB_COUNTER_MASK));
    rc = prvCpssDxChTamSharedMemWrite(devNum, histogramDbAddr, value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal cpssDxChTamHistogramSet function
* @endinternal
*
* @brief  Configure a histogram bins and clear values.  The histogram measurements will start after creation.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] histogramId           - histogram id.
* @param[in] counterPtr            - counter to calculate the histogram.
*                                    if null - delete the histogram
* @param[in] binsPtr               - histogram configuration structure. bin[2] > bin[1] > bin[0] > 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note   If the counter type is: CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E -
*         The histogram counts microburst time values. (Microburst defined by cpssDxChTamMicroburstSePI).
*         In this case the histogram statistics need to be get by cpssDxChTamMicroburstStatGet API.
* @note   For other counter types - The histogram counts counter values.
*         (Counters that were defined in cpssDxChTamSnapshotSet API).
*         In this case the histogram statistics need to be get by cpssDxChTamSnapshotStatGet API.
*
*/
GT_STATUS cpssDxChTamHistogramSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           histogramId,
    IN CPSS_DXCH_TAM_COUNTER_ID_STC     *counterPtr,
    IN CPSS_DXCH_TAM_HISTOGRAM_BIN_STC  *binsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamHistogramSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, histogramId, counterPtr, binsPtr));

    rc = internal_cpssDxChTamHistogramSet(devNum, histogramId, counterPtr, binsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, histogramId, counterPtr, binsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChTamHistogramSetTemp function
* @endinternal
*
* @brief  Temporary API to use in LUA instead of cpssDxChTamHistogramSet untill the problem with U64 will be resolved.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] histogramId           - histogram id.
* @param[in] counterPtr            - counter to calculate the histogram. NULL to disable.
* @param[in] binsPtr               - histogram configuration structure. bin[2] > bin[1] > bin[0] > 0.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamHistogramSetTemp
(
    IN GT_U8                            devNum,
    IN GT_U32                           histogramId,
    IN CPSS_DXCH_TAM_COUNTER_ID_STC     *counterPtr,
    IN CPSS_DXCH_TAM_HISTOGRAM_BIN_TEMP_STC  *binsPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_TAM_HISTOGRAM_BIN_STC  bins;

    bins.bin[0].l[0] = binsPtr->bin0;
    bins.bin[1].l[0] = binsPtr->bin1;
    bins.bin[2].l[0] = binsPtr->bin2;
    bins.bin[0].l[1] = 0;
    bins.bin[1].l[1] = 0;
    bins.bin[2].l[1] = 0;

    rc = cpssDxChTamHistogramSet(devNum, histogramId, counterPtr, &bins);

    return rc;
}

/**
* @internal internal_cpssDxChTamHistogramGet function
* @endinternal
*
* @brief  Get configuration of the histogram.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] histogramId           - histogram id.
* @param[out] counterPtr           - counter to calculate the histogram.
* @param[out] binsPtr              - histogram configuration structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChTamHistogramGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          histogramId,
    OUT CPSS_DXCH_TAM_COUNTER_ID_STC    *counterPtr,
    OUT CPSS_DXCH_TAM_HISTOGRAM_BIN_STC *binsPtr
)
{
    GT_STATUS   rc;
    GT_U32      histogramType, microOrCountDbAddr, histogramDbAddr, counterId;
    GT_U32      value, ii;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);
    CPSS_NULL_PTR_CHECK_MAC(binsPtr);

    /* check if it is a valid histogram id */
    if (histogramId >= PRV_CPSS_DXCH_TAM_FW_HISTOGRAMS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* calculate the address for this histogram */
    histogramDbAddr = tamHistogramDbStartAddr + (histogramId * sizeof(PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC));

    /* read the histogram counter address */
    rc = prvCpssDxChTamSharedMemRead(devNum, histogramDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    microOrCountDbAddr = (value & HISTOGRAM_DB_COUNTER_MASK) * 4;
    histogramType = value >> 24;

    if (microOrCountDbAddr == 0)
    {
        /* the histogram free */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }

    /* check if the histogram defined on microburst */
    if (histogramType == PRV_CPSS_DXCH_TAM_HISTOGRAM_TYPE_MICROBURST_E)
    {
        /* set the output to type == microburst */
        counterPtr->type = CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E;

        /* calculate the microburstId and update the output */
        counterPtr->id = (microOrCountDbAddr - tamMicroburstDbStartAddr) / sizeof(PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC);
    }
    else /* histogram defined on counter */
    {
        /* calculate the counterId */
        counterId = (microOrCountDbAddr - tamCountersDbStartAddr) / sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC);

        /* update the output with counter data */
        cpssOsMemCpy(counterPtr, &countersShadowDb[counterId].counter, sizeof(CPSS_DXCH_TAM_COUNTER_ID_STC));
    }

    /* read the bins fields */
    for (ii = 0; ii < CPSS_DXCH_TAM_HISTOGRAM_BIN_LEVELS; ii++)
    {
        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         histogramDbAddr + HISTOGRAM_DB_BIN_LEVELS_OFFSET +
                                         (ii * HISTOGRAM_DB_BIN_LEVEL_BYTES),
                                         &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        binsPtr->bin[ii].l[0] = value;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTamHistogramGet function
* @endinternal
*
* @brief  Get configuration of the histogram.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] histogramId           - histogram id.
* @param[out] counterPtr           - counter to calculate the histogram.
* @param[out] binsPtr              - histogram configuration structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamHistogramGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          histogramId,
    OUT CPSS_DXCH_TAM_COUNTER_ID_STC    *counterPtr,
    OUT CPSS_DXCH_TAM_HISTOGRAM_BIN_STC *binsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamHistogramGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, histogramId, counterPtr, binsPtr));

    rc = internal_cpssDxChTamHistogramGet(devNum, histogramId, counterPtr, binsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, histogramId, counterPtr, binsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTamThresholdSet function
* @endinternal
*
* @brief  Create and configure a statistics treshold. Set Breach level for this
*         statistics. The statistic should be active. If specified, a threshold
*         breach event will be recorded when the buffer usage goes beyond the level.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] thresholdId           - threshold id.
* @param[in] statisticPtr          - the statistic to monitor.
*                                    if null - delete the histogram
* @param[in] thresholdLevel        - breach watermark level for this statistic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChTamThresholdSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       thresholdId,
    IN CPSS_DXCH_TAM_STATISTIC_STC  *statisticPtr,
    IN GT_U64                       thresholdLevel
)
{
    GT_STATUS   rc;
    GT_U32      thresholdDbAddr, counterDbAddr, snapshotDbAddr, value, ii;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);

    /* check if it is a valid threshold id */
    if (thresholdId >= PRV_CPSS_DXCH_TAM_FW_THRESHOLDS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* calculate the address for this threshold */
    thresholdDbAddr = tamThresholdDbStartAddr + (thresholdId * sizeof(PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC));

    /* read the threshold counter value */
    rc = prvCpssDxChTamSharedMemRead(devNum, thresholdDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    value &= THRESHOLD_DB_COUNTER_MASK;

    /* if(statisticPtr==0) ==> delete the threshold */
    /* zero the threshold db */
    if (statisticPtr == NULL)
    {
        /* check if this threshold id is free */
        if (value == 0)
        {
            /* nothing to do threshold already free */
            return GT_OK;
        }

        /* zero the snapshotPtr and counterPtr in the thresholdsDb */
        rc = prvCpssDxChTamSharedMemWrite(devNum, thresholdDbAddr, 0);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* zero the thresholdLevel in the thresholdsDb */
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          thresholdDbAddr + THRESHOLD_DB_LEVEL_OFFSET,
                                          0);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* zero the threshold notification value */
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                          tamThresholdNotifStartAddr + (sizeof(GT_U32) * thresholdId),
                                          0);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        
        /* zero the threshold notification bit */
        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         tamThresholdNotifBmpStartAddr,
                                         &value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        value &= (~(0x1 << thresholdId));
        rc = prvCpssDxChTamSharedMemWrite(devNum,
                                         tamThresholdNotifBmpStartAddr,
                                         value);
        if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        return GT_OK;
    } 

    /* check if this threshold id is free. */
    if (value != 0)
    {
        /* threshold not free */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    /* check input statistic mode */
    if (statisticPtr->mode != CPSS_DXCH_TAM_TRACKING_MODE_CURRENT_E)
    {
        /* just "current" mode supported at this time */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* search the counters shadow for needed counter */
    for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_COUNTERS; ii++)
    {
        if(countersShadowDb[ii].counter.type == statisticPtr->counter.type)
        {
            if ((countersShadowDb[ii].counter.id == statisticPtr->counter.id) &&
                (countersShadowDb[ii].counter.port == statisticPtr->counter.port) &&
                (countersShadowDb[ii].counter.queue == statisticPtr->counter.queue))
            {
                /* we find counter equal to needed counter */
                break;
            }
        }
    }
    if (ii == PRV_CPSS_DXCH_TAM_FW_COUNTERS)
    {
        /* no such counter in countersDb. cannot create threshold */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* calculate the counterDb address to write it later to the thresholdDb */
    counterDbAddr = tamCountersDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC));

    /* calculate the counter snapshotDbAddr to write it later to the thresholdDb */
    snapshotDbAddr = tamSnapshotDbStartAddr + ((countersShadowDb[ii].snapshotId) * sizeof(PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC));

    /* write to the thresholdsDb the counterAddr and the snapshotAddr */
    value = (counterDbAddr / 4) & THRESHOLD_DB_COUNTER_MASK;
    value |= (snapshotDbAddr << 16);
    rc = prvCpssDxChTamSharedMemWrite(devNum, thresholdDbAddr, value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    /* write the thresholdLevel to the thresholdsDb */
    rc = prvCpssDxChTamSharedMemWrite(devNum,
                                      thresholdDbAddr + THRESHOLD_DB_LEVEL_OFFSET,
                                      thresholdLevel.l[0]);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    return rc;
}

/**
* @internal cpssDxChTamThresholdSet function
* @endinternal
*
* @brief  Create and configure a statistics treshold. Set Breach level for this
*         statistics. The statistic should be active. If specified, a threshold
*         breach event will be recorded when the buffer usage goes beyond the level.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] thresholdId           - threshold id.
* @param[in] statisticPtr          - the statistic to monitor.
*                                    if null - delete the histogram
* @param[in] thresholdLevel        - breach watermark level for this statistic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamThresholdSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       thresholdId,
    IN CPSS_DXCH_TAM_STATISTIC_STC  *statisticPtr,
    IN GT_U64                       thresholdLevel
)
{
    GT_STATUS rc;
    /*CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamThresholdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, thresholdId, statisticPtr, thresholdLevel));
*/
    rc = internal_cpssDxChTamThresholdSet(devNum, thresholdId, statisticPtr, thresholdLevel);

    /*CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, thresholdId, statisticPtr, thresholdLevel));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
*/
    return rc;
}

/**
* @internal internal_cpssDxChTamThresholdGet function
* @endinternal
*
* @brief  Get threshold configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] thresholdId           - threshold id.
* @param[out] statisticPtr         - the statistic to monitor that was defined at set API.
* @param[out] thresholdLevel       - breach watermark level for this statistic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChTamThresholdGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      thresholdId,
    OUT CPSS_DXCH_TAM_STATISTIC_STC *statisticPtr,
    OUT GT_U64                      *thresholdLevel
)
{
    GT_STATUS   rc;
    GT_U32      thresholdDbAddr, counterDbAddr, value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(statisticPtr);
    CPSS_NULL_PTR_CHECK_MAC(thresholdLevel);

    /* check if it is a valid threshold id */
    if (thresholdId >= PRV_CPSS_DXCH_TAM_FW_THRESHOLDS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    /* calculate the address for this threshold */
    thresholdDbAddr = tamThresholdDbStartAddr + (thresholdId * sizeof(PRV_CPSS_DXCH_TAM_THRESHOLD_DB_STC));

    /* read the threshold counter address */
    rc = prvCpssDxChTamSharedMemRead(devNum, thresholdDbAddr, &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    counterDbAddr = (value & THRESHOLD_DB_COUNTER_MASK) * 4;

    if (counterDbAddr == 0)
    {
        /* the threshold free */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }

    /* copy the counter from the counters-shadow to the output */
    cpssOsMemCpy(&statisticPtr->counter,
                 &countersShadowDb[(counterDbAddr - tamCountersDbStartAddr) / sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC)].counter,
                 sizeof(CPSS_DXCH_TAM_COUNTER_ID_STC));

    /* read the statistics mode from thresholdDb and update the output */
    /* for now just current mode supported */
    statisticPtr->mode = CPSS_DXCH_TAM_TRACKING_MODE_CURRENT_E;

    /* read the thresholdLevel from thresholdDb and update the output */
    rc = prvCpssDxChTamSharedMemRead(devNum,
                                     thresholdDbAddr + THRESHOLD_DB_LEVEL_OFFSET,
                                     &value);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    thresholdLevel->l[0] = value;
    thresholdLevel->l[1] = 0;

    return GT_OK;
}

/**
* @internal cpssDxChTamThresholdGet function
* @endinternal
*
* @brief  Get threshold configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] thresholdId           - threshold id.
* @param[out] statisticPtr         - the statistic to monitor that was defined at set API.
* @param[out] thresholdLevel       - breach watermark level for this statistic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamThresholdGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      thresholdId,
    OUT CPSS_DXCH_TAM_STATISTIC_STC *statisticPtr,
    OUT GT_U64                      *thresholdLevel
)
{
    GT_STATUS rc;
    
    /*CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamThresholdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, thresholdId, statisticPtr, thresholdLevel));
*/
    rc = internal_cpssDxChTamThresholdGet(devNum, thresholdId, statisticPtr, thresholdLevel);

    /*CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, thresholdId, statisticPtr, thresholdLevel));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
*/
    return rc;
}

/**
* @internal internal_cpssDxChTamThresholdNotificationsGet function
* @endinternal
*
* @brief  Get list of thresholds that exedded its thresholdLevel.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in,out] numOfThresholds   - in - thresholdsArray size
*                                    out - number of thresholds.
* @param[out] thresholdsArray      - array of thresholds that were crossed its thresholdLevel. Pointer to preallocated list (by the caller).
*  
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChTamThresholdNotificationsGet
(
    IN    GT_U8                                    devNum,
    INOUT GT_U32                                   *numOfThresholds,
    OUT   CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC *thresholdsArray
)
{
    GT_STATUS   rc;
    GT_U32      bmp, value, ii, inNumOfThresholds;
    
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                              | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E
                                              | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E
                                              | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                              | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E);
    CPSS_NULL_PTR_CHECK_MAC(numOfThresholds);
    CPSS_NULL_PTR_CHECK_MAC(thresholdsArray);
    
    if ((*numOfThresholds) > PRV_CPSS_DXCH_TAM_FW_THRESHOLDS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    inNumOfThresholds = *numOfThresholds;
    *numOfThresholds = 0;

    /* read the address */
    rc = prvCpssDxChTamSharedMemRead(devNum, tamThresholdNotifBmpStartAddr, &bmp);
    if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    
    if ((bmp & 0x80000000) == 0) 
    {
        /* no notifications. nothing to do. */
        return GT_OK;
    }

    /* go on all thresholdIds */
    for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_THRESHOLDS; ii++) 
    {
        /* check if this thresholdId enabled in bitmap (i.e. this threshold was crossed) */
        if((bmp >> ii) && 0x1)
        {
            rc = prvCpssDxChTamSharedMemRead(devNum, 
                                             tamThresholdNotifStartAddr + (sizeof(GT_U32) * ii), 
                                             &value);
            if (rc != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            thresholdsArray[*numOfThresholds].thresholdId = ii;
            thresholdsArray[*numOfThresholds].value.l[0] = value;
            
            *numOfThresholds = (*numOfThresholds) + 1;

            /* if no place in input array. no need to read the rest thresholds. */
            if ((*numOfThresholds) == inNumOfThresholds) 
                break;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChTamThresholdNotificationsGet function
* @endinternal
*
* @brief  Get list of thresholds that exedded its thresholdLevel.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in,out] numOfThresholds   - in - thresholdsArray size
*                                    out - number of thresholds.
* @param[out] thresholdsArray      - array of thresholds that were crossed its thresholdLevel. Pointer to preallocated list (by the caller).
*  
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamThresholdNotificationsGet
(
    IN    GT_U8                                    devNum,
    INOUT GT_U32                                   *numOfThresholds,
    OUT   CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC thresholdsArray[]
)
{
    GT_STATUS rc;
    
    /*CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamThresholdNotificationsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfThresholds, thresholdArray));
*/
    rc = internal_cpssDxChTamThresholdNotificationsGet(devNum, numOfThresholds, thresholdsArray);
/*
    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfThresholds, thresholdArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
*/
    return rc;
}

/**
* @internal internal_cpssDxChTamEventNotifyBind function
* @endinternal
*
* @brief  Register a notify callback function.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] notifyFunc            - function pointer.

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChTamEventNotifyBind
(
    IN   GT_U8                                      devNum,
    IN   CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC   *notifyFunc

)
{
    devNum = devNum;

    notifyFuncPtr = notifyFunc;

    return GT_OK;
}

/**
* @internal cpssDxChTamEventNotifyBind function
* @endinternal
*
* @brief  Register a notify callback function.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon.
*
* @param[in] devNum                - physical device number.
* @param[in] notifyFunc            - function pointer.

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_INITIALIZED       - on un-initialized package
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChTamEventNotifyBind
(
    IN   GT_U8                                      devNum,
    IN   CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC   *notifyFunc

)
{
    GT_STATUS rc;

    /*CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTamEventNotifyBind);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, notifyFunc));
*/
    rc = internal_cpssDxChTamEventNotifyBind(devNum, notifyFunc);

    /*CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, notifyFunc));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
*/
    return rc;
}

GT_STATUS prvDebugFwDebugCountersGet(GT_U8 devNum)
{
    GT_STATUS   rc;
    GT_U32      fwCounter, fwStatCounter;

    rc = cpssDxChTamFwDebugCountersGet(devNum, &fwCounter, &fwStatCounter);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    cpssOsPrintf("tamFwDebugCounterAddr=0x%08x tamFwDebugStatCounterAddr=0x%08x\n",tamFwDebugCounterAddr,tamFwDebugStatCounterAddr);

    cpssOsPrintf("fwCounter     = %d\n", fwCounter);
    cpssOsPrintf("fwStatCounter = %d\n", fwStatCounter);

    return GT_OK;
}

GT_STATUS prvDebugPrintTamCounterAddr(GT_U8 devNum, CPSS_DXCH_TAM_COUNTER_TYPE_ENT counterType, GT_U32 id, GT_U32 port, GT_U32 queue)
{
    CPSS_DXCH_TAM_COUNTER_ID_STC   counter;
    GT_U32                         regAddr;

    counter.type = counterType;
    counter.id = id;
    counter.port = port;
    counter.queue = queue;

    regAddr = getHwRegAddrForTamCounter(devNum, counter, GT_TRUE);
    cpssOsPrintf("regAddr=0x%08x\n", regAddr);

    return GT_OK;
}

GT_STATUS prvDebugSnapshotSet(GT_U8 devNum, GT_U32 snapshotId, CPSS_DXCH_TAM_COUNTER_TYPE_ENT counterType, GT_U32 id, GT_U32 port, GT_U32 queue)
{
    GT_STATUS                      rc;
    CPSS_DXCH_TAM_COUNTER_ID_STC   counters[1];

    cpssOsPrintf("snapshotId=%d\n", snapshotId);
    cpssOsPrintf("counterType=%d=%s\n", counterType, tamCounterTypeStr[counterType]);
    cpssOsPrintf("id=%d port=%d queue=%d\n", id, port, queue);

    counters[0].type = counterType;
    counters[0].id = id;
    counters[0].port = port;
    counters[0].queue = queue;

    rc = cpssDxChTamSnapshotSet(devNum, snapshotId, 1, counters);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    return GT_OK;
}

GT_STATUS prvDebugSnapshotSetListSize(GT_U8 devNum, GT_U32 snapshotId, GT_U32 listSize)
{
    GT_STATUS                      rc;
    CPSS_DXCH_TAM_COUNTER_ID_STC   counters[PRV_CPSS_DXCH_TAM_FW_COUNTERS];
    GT_U32                         ii;

    cpssOsPrintf("snapshotId=%d\n", snapshotId);

    cpssOsMemSet(counters, 0, listSize * sizeof(CPSS_DXCH_TAM_COUNTER_ID_STC));
    for (ii = 0; ii < listSize; ii++) {
        counters[ii].type = CPSS_DXCH_TAM_INGRESS_GLOBAL_BUFFER_COUNTER_E;/*CPSS_DXCH_TAM_EGRESS_PORT_BUFFER_COUNTER_E;
        counters[ii].port = ii;*/
    }
    rc = cpssDxChTamSnapshotSet(devNum, snapshotId, listSize, counters);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    return GT_OK;
}

GT_STATUS prvDebugSnapshotSetEgressPort(GT_U8 devNum, GT_U32 snapshotId, GT_U32 port1, GT_U32 port2)
{
    GT_STATUS                      rc;
    GT_U32                         listSize = 2;
    CPSS_DXCH_TAM_COUNTER_ID_STC   counters[PRV_CPSS_DXCH_TAM_FW_COUNTERS];
    /*GT_U32                         ii;*/

    cpssOsPrintf("snapshotId=%d port1=%d port2=%d\n", snapshotId, port1, port2);

    cpssOsMemSet(counters, 0, listSize * sizeof(CPSS_DXCH_TAM_COUNTER_ID_STC));
    counters[0].type = CPSS_DXCH_TAM_EGRESS_PORT_BUFFER_COUNTER_E;
    counters[0].port = port1;
    counters[1].type = CPSS_DXCH_TAM_EGRESS_PORT_BUFFER_COUNTER_E;
    counters[1].port = port2;

    rc = cpssDxChTamSnapshotSet(devNum, snapshotId, listSize, counters);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    return GT_OK;
}

GT_STATUS prvDebugSnapshotDelete(GT_U8 devNum, GT_U32 snapshotId)
{
    GT_STATUS                      rc;
    GT_U32                         listSize = 0;
    CPSS_DXCH_TAM_COUNTER_ID_STC   counters[PRV_CPSS_DXCH_TAM_FW_COUNTERS];

    cpssOsPrintf("snapshotId=%d\n", snapshotId);

    rc = cpssDxChTamSnapshotSet(devNum, snapshotId, listSize, counters);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    return GT_OK;
}

GT_STATUS prvDebugSnapshotGet(GT_U8 devNum, GT_U32 snapshotId, GT_U32 size)
{
    GT_STATUS                    rc;
    GT_U32                       listSize, ii;
    CPSS_DXCH_TAM_COUNTER_ID_STC counters[PRV_CPSS_DXCH_TAM_FW_COUNTERS];

    cpssOsPrintf("snapshotId=%d inputSize=%d\n", snapshotId, size);

    cpssOsMemSet(counters, 0, size * sizeof(CPSS_DXCH_TAM_COUNTER_ID_STC));
    listSize = size;
    rc = cpssDxChTamSnapshotGet(devNum, snapshotId, &listSize, counters);
    if (rc != GT_OK)
    {
        cpssOsPrintf("rc=%d\n", rc);
        return rc;
    }

    cpssOsPrintf("outputSize=%d\n", listSize);
    cpssOsPrintf("   ii  type             typeStr         id       port      queue\n");
    for (ii = 0; ii < listSize; ii++) {
        cpssOsPrintf("%5d %5d %19s %10d %10d %10d\n",
                     ii,
                     counters[ii].type,
                     tamCounterTypeStr[counters[ii].type],
                     counters[ii].id, counters[ii].port, counters[ii].queue);
    }

    return GT_OK;
}

GT_STATUS prvDebugSnapshotStatGet(GT_U8 devNum, GT_U32 snapshotId, GT_U32 size)
{
    GT_STATUS                               rc;
    GT_U32                                  numOfStats, numOfHistograms, ii;
    CPSS_DXCH_TAM_COUNTER_STATISTICS_STC    statsData[PRV_CPSS_DXCH_TAM_FW_COUNTERS];
    CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC       histValue[PRV_CPSS_DXCH_TAM_FW_HISTOGRAMS];

    cpssOsPrintf("snapshotId=%d inputStatSize=%d inputHistSize=%d\n", snapshotId, size, PRV_CPSS_DXCH_TAM_FW_HISTOGRAMS);

    cpssOsMemSet(statsData, 0, sizeof(statsData));
    cpssOsMemSet(histValue, 0, sizeof(histValue));
    numOfStats = size;
    numOfHistograms = PRV_CPSS_DXCH_TAM_FW_HISTOGRAMS;
    rc = cpssDxChTamSnapshotStatGet(devNum, snapshotId, &numOfStats, statsData, &numOfHistograms, histValue);
    if (rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    cpssOsPrintf("outputStatSize=%d outputHistSize=%d\n", numOfStats, numOfHistograms);
    cpssOsPrintf("        ii    current    minimum       peak    average\n");
    for (ii = 0; ii < numOfStats; ii++)
    {
        cpssOsPrintf("%10u %10u %10u %10u %10u\n",
                     ii,
                     statsData[ii].current.l[0],
                     statsData[ii].minimum.l[0],
                     statsData[ii].peak.l[0],
                     statsData[ii].average.l[0]);
    }

    cpssOsPrintf("\n        ii histogramId     value0     value1     value2     value3\n");
    for (ii = 0; ii < numOfHistograms; ii++)
    {
        cpssOsPrintf("%10u  %10u %10u %10u %10u %10u\n",
                     ii,
                     histValue[ii].histogramId,
                     histValue[ii].value[0].l[0],
                     histValue[ii].value[1].l[0],
                     histValue[ii].value[2].l[0],
                     histValue[ii].value[3].l[0]);
    }

    return GT_OK;
}

GT_STATUS prvDebugPrintCountersDb(GT_U8 devNum, GT_U16 firstIdx, GT_U16 entries)
{
    GT_U32                       rc, value, counterDbAddr;
    GT_U16                       ii, jj;

    cpssOsPrintf("tamCountersDbStartAddr = 0x%08x\n", tamCountersDbStartAddr);
    cpssOsPrintf("freeCounterFirstAddr = 0x%08x freeCountersNum = %d\n", freeCounterFirstAddr, freeCountersNum);

    cpssOsPrintf("address       idx switchRegAddr      last0       min0       max0      coun0       sum0       sum0      last1       min1       max1      coun1       sum1       sum1 nextCountAddr\n");
    for (ii = firstIdx; ii < (firstIdx + entries); ii++)
    {
        counterDbAddr = tamCountersDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC));
        cpssOsPrintf("0x%08x: ", counterDbAddr);
        cpssOsPrintf("%5d ", (counterDbAddr - tamCountersDbStartAddr)/sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC));

        rc = prvCpssDxChTamSharedMemRead(devNum, counterDbAddr, &value);
        if (rc != GT_OK)    cpssOsPrintf("0xxxxxxxxx    ");
        else                cpssOsPrintf("0x%08x    ", value);

        for (jj = 0; jj < ALL_STATISTICS_SIZE_IN_WORDS; jj++)
        {
            rc = prvCpssDxChTamSharedMemRead(devNum,
                                             counterDbAddr + COUNTER_DB_STAT_0_OFFSET + (4 * jj),
                                             &value);
            if (rc != GT_OK)    cpssOsPrintf("     xxxxx ");
            else                cpssOsPrintf("%10u ", value);
        }

        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         counterDbAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                         &value);
        if (rc != GT_OK)    cpssOsPrintf("0xxxxxxxxx ");
        else                cpssOsPrintf("0x%08x ", value);

        cpssOsPrintf("\n");
    }

    return GT_OK;
}

GT_STATUS prvDebugPrintCountersShadow(GT_U8 devNum, GT_U16 firstIdx, GT_U16 entries)
{
    GT_U32   counterDbAddr;
    GT_U16    ii;

    cpssOsPrintf("   ii    dbAddress   idx  type             typeStr   id  port queue snapId histId\n");
    for (ii = firstIdx; ii < (firstIdx + entries); ii++)
    {
        cpssOsPrintf("%5d ", ii);
        counterDbAddr = tamCountersDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC));
        cpssOsPrintf("(0x%08x): ", counterDbAddr);
        cpssOsPrintf("%5d ", (counterDbAddr - tamCountersDbStartAddr)/sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC));

        cpssOsPrintf("%5d ", countersShadowDb[ii].counter.type);
        cpssOsPrintf("%19s ", tamCounterTypeStr[countersShadowDb[ii].counter.type]);
        cpssOsPrintf("%5d ", countersShadowDb[ii].counter.id);
        cpssOsPrintf("%5d ", countersShadowDb[ii].counter.port);
        cpssOsPrintf("%5d ", countersShadowDb[ii].counter.queue);
        cpssOsPrintf("%6d ", countersShadowDb[ii].snapshotId);
        cpssOsPrintf("%6d ", countersShadowDb[ii].histogramId);
        cpssOsPrintf("\n");
    }

    (void)devNum;

    return GT_OK;
}

GT_STATUS prvDebugPrintSnapshotsDb(GT_U8 devNum)
{
    GT_U8           ii;
    GT_U32          rc, value, snapshotDbAddr;

    cpssOsPrintf("tamSnapshotDbStartAddr = 0x%08x\n", tamSnapshotDbStartAddr);

    cpssOsPrintf("DbAddress  snapshotId statBuff(0/1) countersDbFirstPtr/4 countersDbFirstPtr\n");
    for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_SNAPSHOTS; ii++)
    {
        snapshotDbAddr = tamSnapshotDbStartAddr + (ii * 4);
        cpssOsPrintf("0x%08x %10d ",snapshotDbAddr, ii);

        rc = prvCpssDxChTamSharedMemRead(devNum, snapshotDbAddr, &value);
        if (rc != GT_OK)    cpssOsPrintf("xxxxxxxxxx ");
        else
        {
            cpssOsPrintf("%13d ", ((value & SNAPSHOT_DB_STAT_BUFF_MASK) >> 16));
            cpssOsPrintf("          0x%08x ", (value & SNAPSHOT_DB_FIRST_COUNTER_MASK));
            cpssOsPrintf("        0x%08x ", ((value & SNAPSHOT_DB_FIRST_COUNTER_MASK)*4));
        }
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

GT_STATUS prvDebugPrintSnapshotCounters(GT_U8 devNum, GT_U32 snapshotId)
{
    GT_U32 rc, jj, value, snapshotDbAddr, counterDbAddr;

    cpssOsPrintf("tamSnapshotDbStartAddr = 0x%08x\n", tamSnapshotDbStartAddr);
    cpssOsPrintf("tamCountersDbStartAddr = 0x%08x\n", tamCountersDbStartAddr);
    cpssOsPrintf("freeCounterFirstAddr = 0x%08x freeCountersNum = %d\n", freeCounterFirstAddr, freeCountersNum);

    cpssOsPrintf("DbAddress  snapshotId statBuff(0/1) countersDbFirstPtr/4 countersDbFirstPtr\n");

    snapshotDbAddr = tamSnapshotDbStartAddr + (snapshotId * 4);
    cpssOsPrintf("0x%08x %10d ",snapshotDbAddr, snapshotId);

    rc = prvCpssDxChTamSharedMemRead(devNum, snapshotDbAddr, &value);
    if (rc != GT_OK)
    {
        cpssOsPrintf("rc = %d\n", rc);
        return rc;
    }
    cpssOsPrintf("%13d ", ((value & SNAPSHOT_DB_STAT_BUFF_MASK) >> 16));
    cpssOsPrintf("     0x%08x ", (value & SNAPSHOT_DB_FIRST_COUNTER_MASK));
    cpssOsPrintf("   0x%08x ", ((value & SNAPSHOT_DB_FIRST_COUNTER_MASK)*4));
    cpssOsPrintf("\n\n");

    cpssOsPrintf("address       idx switchRegAddr      last0       min0       max0      coun0       sum0       sum0      last1       min1       max1      coun1       sum1       sum1 nextCountAddr\n");
    counterDbAddr = ((value & SNAPSHOT_DB_FIRST_COUNTER_MASK)*4);
    while (counterDbAddr != 0)
    {
        cpssOsPrintf("0x%08x: ", counterDbAddr);
        cpssOsPrintf("%5d ", (counterDbAddr - tamCountersDbStartAddr)/sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC));

        rc = prvCpssDxChTamSharedMemRead(devNum, counterDbAddr, &value);
        if (rc != GT_OK)    cpssOsPrintf("0xxxxxxxxx    ");
        else                cpssOsPrintf("0x%08x    ", value);

        for (jj = 0; jj < ALL_STATISTICS_SIZE_IN_WORDS; jj++)
        {
            rc = prvCpssDxChTamSharedMemRead(devNum,
                                             counterDbAddr + COUNTER_DB_STAT_0_OFFSET + (4 * jj),
                                             &value);
            if (rc != GT_OK)    cpssOsPrintf("     xxxxx ");
            else                cpssOsPrintf("%10u ", value);
        }

        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         counterDbAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                         &value);
        if (rc != GT_OK)    cpssOsPrintf("0xxxxxxxxx ");
        else                cpssOsPrintf("0x%08x ", value);
        cpssOsPrintf("\n");

        counterDbAddr = value;
    }

    return GT_OK;
}

GT_STATUS prvDebugPrintSnapshotCountersShadow(GT_U8 devNum, GT_U32 snapshotId)
{
    GT_U32 rc, ii, value, snapshotDbAddr, counterDbAddr;

    cpssOsPrintf("DbAddress  snapshotId statBuff(0/1) countersDbFirstPtr/4 countersDbFirstPtr\n");
    snapshotDbAddr = tamSnapshotDbStartAddr + (snapshotId * 4);
    cpssOsPrintf("0x%08x %10d ",snapshotDbAddr, snapshotId);

    rc = prvCpssDxChTamSharedMemRead(devNum, snapshotDbAddr, &value);
    if (rc != GT_OK)
    {
        cpssOsPrintf("rc = %d\n", rc);
        return rc;
    }
    cpssOsPrintf("%13d ", ((value & SNAPSHOT_DB_STAT_BUFF_MASK) >> 16));
    cpssOsPrintf("          0x%08x ", (value & SNAPSHOT_DB_FIRST_COUNTER_MASK));
    cpssOsPrintf("        0x%08x ", ((value & SNAPSHOT_DB_FIRST_COUNTER_MASK)*4));
    cpssOsPrintf("\n\n");

    cpssOsPrintf("    dbAddress   idx  type             typeStr   id  port queue snaId hisId nextDbAddr\n");
    counterDbAddr = ((value & SNAPSHOT_DB_FIRST_COUNTER_MASK)*4);
    while (counterDbAddr != 0)
    {
        ii = (counterDbAddr - tamCountersDbStartAddr)/sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC);
        cpssOsPrintf("(0x%08x): ", counterDbAddr);
        cpssOsPrintf("%5d ", ii);

        cpssOsPrintf("%5d ", countersShadowDb[ii].counter.type);
        cpssOsPrintf("%19s ", tamCounterTypeStr[countersShadowDb[ii].counter.type]);
        cpssOsPrintf("%5d ", countersShadowDb[ii].counter.id);
        cpssOsPrintf("%5d ", countersShadowDb[ii].counter.port);
        cpssOsPrintf("%5d ", countersShadowDb[ii].counter.queue);
        cpssOsPrintf("%5d ", countersShadowDb[ii].snapshotId);
        cpssOsPrintf("%5d ", countersShadowDb[ii].histogramId);

        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         counterDbAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                         &value);
        if (rc != GT_OK)    cpssOsPrintf("0xxxxxxxxx ");
        else                cpssOsPrintf("0x%08x ", value);
        cpssOsPrintf("\n");

        counterDbAddr = value;
    }

    return GT_OK;
}

GT_STATUS prvDebugMicroburstSet(GT_U8 devNum, GT_U32 microburstId,
                                CPSS_DXCH_TAM_COUNTER_TYPE_ENT counterType, GT_U32 id, GT_U32 port, GT_U32 queue,
                                GT_U32 highWatermark, GT_U32 lowWatermark)
{
    GT_STATUS                      rc;
    CPSS_DXCH_TAM_COUNTER_ID_STC   counter;

    cpssOsPrintf("microburstId=%d\n", microburstId);
    cpssOsPrintf("counterType=%d=%s\n", counterType, tamCounterTypeStr[counterType]);
    cpssOsPrintf("id=%d port=%d queue=%d\n", id, port, queue);
    cpssOsPrintf("highWatermark=%d lowWatermark=%d\n", highWatermark, lowWatermark);

    counter.type = counterType;
    counter.port = port;
    counter.queue = queue;
    counter.id = id;
    rc = cpssDxChTamMicroburstSet(devNum, microburstId, &counter, highWatermark, lowWatermark);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    return GT_OK;
}

GT_STATUS prvDebugMicroburstSetEgressPort(GT_U8 devNum, GT_U32 microburstId, GT_U32 port1, GT_U32 highWatermark, GT_U32 lowWatermark)
{
    GT_STATUS                      rc;
    CPSS_DXCH_TAM_COUNTER_ID_STC   counter;

    cpssOsPrintf("microburstId=%d port1=%d highWatermark=%d lowWatermark=%d\n", microburstId, port1, highWatermark, lowWatermark);

    counter.type = CPSS_DXCH_TAM_EGRESS_PORT_BUFFER_COUNTER_E;
    counter.port = port1;
    counter.queue = 0;
    counter.id = 0;
    rc = cpssDxChTamMicroburstSet(devNum, microburstId, &counter, highWatermark, lowWatermark);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    return GT_OK;
}

GT_STATUS prvDebugMicroburstDelete(GT_U8 devNum, GT_U32 microburstId)
{
    GT_STATUS                      rc;

    cpssOsPrintf("microburstId=%d\n", microburstId);

    rc = cpssDxChTamMicroburstSet(devNum, microburstId, NULL, 0, 0);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    return GT_OK;
}

GT_STATUS prvDebugMicroburstGetAll(GT_U8 devNum)
{
    GT_STATUS                       rc;
    GT_U32                          highWatermark, lowWatermark;
    CPSS_DXCH_TAM_COUNTER_ID_STC    counter;
    GT_U32                          microburstId;

    cpssOsPrintf("micId  type             typeStr         id       port      queue highWatermark lowWatermark\n");

    for (microburstId = 0; microburstId < PRV_CPSS_DXCH_TAM_FW_MICROBURSTS; microburstId++) {

        counter.type = 0;
        counter.port = 0;
        counter.queue = 0;
        counter.id = 0;
        rc = cpssDxChTamMicroburstGet(devNum, microburstId, &counter, &highWatermark, &lowWatermark);
        if (rc == GT_NO_SUCH)
        {
            cpssOsPrintf("%5d GT_NO_SUCH\n", microburstId);
        }
        else if (rc != GT_OK)
        {
            cpssOsPrintf("rc=%d\n", rc);
        }
        else
        {
            cpssOsPrintf("%5d %5d %19s %10d %10d %10d %13u %12u\n",
                         microburstId,
                         counter.type,
                         tamCounterTypeStr[counter.type],
                         counter.id, counter.port, counter.queue,
                         highWatermark, lowWatermark);
        }
    }

    return GT_OK;
}

GT_STATUS prvDebugMicroburstStatGet(GT_U8 devNum, GT_U32 microburstId)
{
    GT_STATUS                               rc;
    CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC statsData;
    CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC       histValue;

    cpssOsPrintf("microburstId=%d\n", microburstId);

    cpssOsMemSet(&statsData, 0, sizeof(statsData));
    cpssOsMemSet(&histValue, 0, sizeof(histValue));

    rc = cpssDxChTamMicroburstStatGet(devNum, microburstId, &statsData, &histValue);
    if (rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    cpssOsPrintf("    current    minimum       peak    average\n");
    cpssOsPrintf("%10u %10u %10u %10u\n",
                 statsData.lastDuration,
                 statsData.shortestDuration,
                 statsData.longestDuration,
                 statsData.averageDuration);

    cpssOsPrintf("\n\nhistogramId     value0     value1     value2     value3\n");
    cpssOsPrintf("%11u %10u %10u %10u %10u\n",
                 histValue.histogramId,
                 histValue.value[0].l[0],
                 histValue.value[1].l[0],
                 histValue.value[2].l[0],
                 histValue.value[3].l[0]);

    return GT_OK;
}

GT_STATUS prvDebugMicroburstStatGetAll(GT_U8 devNum)
{
    GT_STATUS                                rc;
    CPSS_DXCH_TAM_MICROBURST_STATISTICS_STC  statsData;
    CPSS_DXCH_TAM_HISTOGRAM_VALUE_STC        histValue;
    GT_U32                                   microburstId;

    cpssOsPrintf("microburstId    current    minimum       peak    average ");
    cpssOsPrintf("histogramId     value0     value1     value2     value3\n");

    for (microburstId = 0; microburstId < PRV_CPSS_DXCH_TAM_FW_MICROBURSTS; microburstId++)
    {
        cpssOsMemSet(&statsData, 0, sizeof(statsData));
        cpssOsMemSet(&histValue, 0, sizeof(histValue));
        rc = cpssDxChTamMicroburstStatGet(devNum, microburstId, &statsData, &histValue);
        if (rc == GT_NO_SUCH){
            cpssOsPrintf("%12d GT_NO_SUCH\n", microburstId);
        }
        else if (rc != GT_OK)
        {
            cpssOsPrintf("rc=%d\n", rc);
        }
        else
        {
            cpssOsPrintf("%12u %10u %10u %10u %10u ",
                         microburstId,
                         statsData.lastDuration,
                         statsData.shortestDuration,
                         statsData.longestDuration,
                         statsData.averageDuration);

            cpssOsPrintf("%11u %10u %10u %10u %10u\n",
                         histValue.histogramId,
                         histValue.value[0].l[0],
                         histValue.value[1].l[0],
                         histValue.value[2].l[0],
                         histValue.value[3].l[0]);
        }
    }

    return GT_OK;
}

GT_STATUS prvDebugPrintMicroburstDb(GT_U8 devNum)
{
    GT_U8           ii, jj;
    GT_U32          rc, value, microburstDbAddr;

    cpssOsPrintf("tamMicroburstDbStartAddr = 0x%08x\n", tamMicroburstDbStartAddr);

    cpssOsPrintf(" DbAddress microburId statBuff(0/1) snapshotId countersDbPtr/4 countersDbPtr ");
    cpssOsPrintf("hiWatermark loWatermark  timeStart calcHistog timeStatus\n");
    for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_MICROBURSTS; ii++)
    {
        microburstDbAddr = tamMicroburstDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC));
        cpssOsPrintf("0x%08x %10d ",microburstDbAddr, ii);

        rc = prvCpssDxChTamSharedMemRead(devNum, microburstDbAddr, &value);
        if (rc != GT_OK)    cpssOsPrintf("xxxxxxxxxx ");
        else
        {
            cpssOsPrintf("%13d ", ((value & MICROBURST_DB_STAT_BUFF_MASK) >> 16));
            cpssOsPrintf("%10d ", ((value & MICROBURST_DB_SNAPSHOT_ID_MASK) >> 24));
            cpssOsPrintf("     0x%08x ", (value & MICROBURST_DB_COUNTER_MASK));
            cpssOsPrintf("   0x%08x ", ((value & MICROBURST_DB_COUNTER_MASK)*4));
        }

        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         microburstDbAddr + MICROBURST_DB_HIGH_WATERMARK_OFFSET,
                                         &value);
        if (rc != GT_OK)    cpssOsPrintf("xxxxxxxxxxx ");
        else                cpssOsPrintf(" 0x%08x ", value);

        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         microburstDbAddr + MICROBURST_DB_LOW_WATERMARK_OFFSET,
                                         &value);
        if (rc != GT_OK)    cpssOsPrintf("xxxxxxxxxxxx ");
        else                cpssOsPrintf("  0x%08x ", value);

        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         microburstDbAddr + MICROBURST_DB_TIME_START_OFFSET,
                                         &value);
        if (rc != GT_OK)    cpssOsPrintf("xxxxxxxxxx");
        else                cpssOsPrintf(" 0x%08x ", value);

        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         microburstDbAddr + MICROBURST_DB_TIME_STATUS_TBD,
                                         &value);
        if (rc != GT_OK)    
        {
            cpssOsPrintf("xxxxxxxxxx xxxxxxxxxx");
        }
        else
        {
            cpssOsPrintf(" 0x%08x ", ((value & MICROBURST_DB_CALC_HISTOGRAM_MASK) >> 16));
            cpssOsPrintf(" 0x%08x", (value & MICROBURST_DB_TIME_STATUS_MASK));
        }

        cpssOsPrintf("\n");
    }

    cpssOsPrintf("\n");
    cpssOsPrintf("microburId     last0       min0       max0      coun0    sum0[0]    sum0[1]      last1       min1       max1      coun1    sum1[0]    sum1[1]\n");
    for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_MICROBURSTS; ii++)
    {
        microburstDbAddr = tamMicroburstDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC));

        cpssOsPrintf("%10d ", ii);

        for (jj = 0; jj < ALL_STATISTICS_SIZE_IN_WORDS; jj++)
        {
            rc = prvCpssDxChTamSharedMemRead(devNum,
                                             microburstDbAddr + MICROBURST_DB_STAT_0_OFFSET + (4 * jj),
                                             &value);
            if (rc != GT_OK)    cpssOsPrintf("     xxxxx ");
            else                cpssOsPrintf("%10u ", value);
        }

        cpssOsPrintf("\n");
    }

    cpssOsPrintf("\n");
    cpssOsPrintf("microburId shadowHistogramId\n");
    for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_MICROBURSTS; ii++)
    {
        cpssOsPrintf("%10d %16d", ii, microburstShadowDb[ii].histogramId);
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

GT_STATUS prvDebugHistogramOnCounterSet(GT_U8 devNum,
                                        GT_U32 histogramId,
                                        CPSS_DXCH_TAM_COUNTER_TYPE_ENT counterType,
                                        GT_U32 id, GT_U32 port, GT_U32 queue,
                                        GT_U32 bin0, GT_U32 bin1, GT_U32 bin2)
{
    GT_STATUS                           rc;
    CPSS_DXCH_TAM_COUNTER_ID_STC        counter;
    CPSS_DXCH_TAM_HISTOGRAM_BIN_STC     bins;

    cpssOsPrintf("histogramId=%d\n", histogramId);
    cpssOsPrintf("counterType=%d=%s\n", counterType, tamCounterTypeStr[counterType]);
    cpssOsPrintf("id=%d port=%d queue=%d\n", id, port, queue);
    cpssOsPrintf("bin0=%d bin1=%d bin2=%d\n", bin0, bin1, bin2);

    counter.type = counterType;
    counter.id = id;
    counter.port = port;
    counter.queue = queue;
    bins.bin[0].l[0] = bin0;
    bins.bin[1].l[0] = bin1;
    bins.bin[2].l[0] = bin2;
    bins.bin[0].l[1] = 0;
    bins.bin[1].l[1] = 0;
    bins.bin[2].l[1] = 0;
    rc = cpssDxChTamHistogramSet(devNum, histogramId, &counter, &bins);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    return GT_OK;
}

GT_STATUS prvDebugHistogramOnMicroburstSet(GT_U8 devNum, GT_U32 histogramId, GT_U32 microburstId, GT_U32 bin0, GT_U32 bin1, GT_U32 bin2)
{
    GT_STATUS                           rc;
    CPSS_DXCH_TAM_COUNTER_ID_STC        counter;
    CPSS_DXCH_TAM_HISTOGRAM_BIN_STC     bins;

    cpssOsPrintf("histogramId=%d microburstId=%d\n", histogramId, microburstId);
    cpssOsPrintf("bin0=%d bin1=%d bin2=%d\n", bin0, bin1, bin2);

    counter.type = CPSS_DXCH_TAM_MICROBURST_DURATION_COUNTER_E;
    counter.port = 0;
    counter.queue = 0;
    counter.id = microburstId;
    bins.bin[0].l[0] = bin0;
    bins.bin[1].l[0] = bin1;
    bins.bin[2].l[0] = bin2;
    bins.bin[0].l[1] = 0;
    bins.bin[1].l[1] = 0;
    bins.bin[2].l[1] = 0;
    rc = cpssDxChTamHistogramSet(devNum, histogramId, &counter, &bins);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    return GT_OK;
}

GT_STATUS prvDebugHistogramDelete(GT_U8 devNum, GT_U32 histogramId)
{
    GT_STATUS                      rc;

    cpssOsPrintf("histogramId=%d\n", histogramId);

    rc = cpssDxChTamHistogramSet(devNum, histogramId, NULL, NULL);
    if(rc != GT_OK) cpssOsPrintf("rc=%d\n", rc);

    return GT_OK;
}

GT_STATUS prvDebugHistogramGetAll(GT_U8 devNum)
{
    GT_STATUS                       rc;
    GT_U32                          histogramId;
    CPSS_DXCH_TAM_COUNTER_ID_STC    counter;
    CPSS_DXCH_TAM_HISTOGRAM_BIN_STC bins;

    cpssOsPrintf("hisId  type             typeStr         id       port      queue       bin0       bin1       bin2\n");

    for (histogramId = 0; histogramId < PRV_CPSS_DXCH_TAM_FW_HISTOGRAMS; histogramId++)
    {
        counter.type = 0;
        counter.port = 0;
        counter.queue = 0;
        counter.id = 0;
        rc = cpssDxChTamHistogramGet(devNum, histogramId, &counter, &bins);
        if (rc == GT_NO_SUCH)
        {
            cpssOsPrintf("%5d GT_NO_SUCH\n", histogramId);
        }
        else if (rc != GT_OK)
        {
            cpssOsPrintf("rc=%d\n", rc);
        }
        else
        {
            cpssOsPrintf("%5d %5d %19s %10d %10d %10d %10u %10u %10u\n",
                         histogramId,
                         counter.type,
                         tamCounterTypeStr[counter.type],
                         counter.id, counter.port, counter.queue,
                         bins.bin[0].l[0], bins.bin[1].l[0], bins.bin[2].l[0]);
        }
    }

    return GT_OK;
}

GT_STATUS prvDebugPrintHistogramDb(GT_U8 devNum)
{
    GT_U8           ii, jj;
    GT_U32          rc, value, histogramDbAddr;

    cpssOsPrintf("tamHistogramDbStartAddr = 0x%08x\n", tamHistogramDbStartAddr);

    cpssOsPrintf(" DbAddress histograId   cnt/micr(0/1) statBuff(0/1) countersDbPtr/4 countersDbPtr snapshotDbPtr ");
    cpssOsPrintf("      bin0       bin1       bin2\n");
    for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_HISTOGRAMS; ii++)
    {
        histogramDbAddr = tamHistogramDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC));
        cpssOsPrintf("0x%08x %10d ",histogramDbAddr, ii);

        rc = prvCpssDxChTamSharedMemRead(devNum, histogramDbAddr, &value);
        if (rc != GT_OK)    cpssOsPrintf("xxxxxxxxxx ");
        else
        {
            cpssOsPrintf("%13d ", ((value & HISTOGRAM_DB_TYPE_MASK) >> 24));
            cpssOsPrintf("%13d ", ((value & HISTOGRAM_DB_STAT_BUFF_MASK) >> 16));
            cpssOsPrintf("     0x%08x ", (value & HISTOGRAM_DB_COUNTER_MASK));
            cpssOsPrintf("   0x%08x ", ((value & HISTOGRAM_DB_COUNTER_MASK)*4));
        }

        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         histogramDbAddr + HISTOGRAM_DB_SNAPSHOT_OFFSET,
                                         &value);
        if (rc != GT_OK)    cpssOsPrintf("xxxxxxxxxxx  ");
        else                cpssOsPrintf("   0x%08x ", value);

        for (jj = 0; jj < CPSS_DXCH_TAM_HISTOGRAM_BIN_LEVELS; jj++)
        {
            rc = prvCpssDxChTamSharedMemRead(devNum,
                                             histogramDbAddr + HISTOGRAM_DB_BIN_LEVELS_OFFSET +
                                             (jj * HISTOGRAM_DB_BIN_LEVEL_BYTES),
                                             &value);
            if (rc != GT_OK)    cpssOsPrintf("xxxxxxxxxxx ");
            else                cpssOsPrintf(" 0x%08x ", value);
        }

        cpssOsPrintf("\n");
    }

    cpssOsPrintf("\n");
    cpssOsPrintf("histograId     val00      val00      val01      val01     val02      val02      val03      val03\n");
    for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_HISTOGRAMS; ii++)
    {
        histogramDbAddr = tamHistogramDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC));

        cpssOsPrintf("%10d ", ii);

        for (jj = 0; jj < 8; jj++)
        {
            rc = prvCpssDxChTamSharedMemRead(devNum,
                                             histogramDbAddr + HISTOGRAM_DB_BIN_VAL_STAT_0_OFFSET + (4 * jj),
                                             &value);
            if (rc != GT_OK)    cpssOsPrintf("     xxxxx ");
            else                cpssOsPrintf("0x%08x ", value);
        }

        cpssOsPrintf("\n");
    }

    cpssOsPrintf("\n");
    cpssOsPrintf("histograId     val10      val10      val11      val11     val12      val12      val13      val13\n");
    for (ii = 0; ii < PRV_CPSS_DXCH_TAM_FW_HISTOGRAMS; ii++)
    {
        histogramDbAddr = tamHistogramDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC));

        cpssOsPrintf("%10d ", ii);

        for (jj = 0; jj < 8; jj++)
        {
            rc = prvCpssDxChTamSharedMemRead(devNum,
                                             histogramDbAddr + HISTOGRAM_DB_BIN_VAL_STAT_1_OFFSET + (4 * jj),
                                             &value);
            if (rc != GT_OK)    cpssOsPrintf("     xxxxx ");
            else                cpssOsPrintf("0x%08x ", value);
        }

        cpssOsPrintf("\n");
    }

    return GT_OK;
}

GT_STATUS prvDebugThresholdSet(GT_U8 devNum,
                               GT_U32 thresholdId,
                               GT_U32 level,
                               CPSS_DXCH_TAM_COUNTER_TYPE_ENT counterType,
                               GT_U32 id, GT_U32 port, GT_U32 queue)
{
    GT_STATUS rc;
    CPSS_DXCH_TAM_STATISTIC_STC  statisticPtr;
    GT_U64                       thresholdLevel;

    thresholdLevel.l[1] = 0;
    thresholdLevel.l[0] = level;
    statisticPtr.mode = CPSS_DXCH_TAM_TRACKING_MODE_CURRENT_E;
    statisticPtr.counter.type = counterType;
    statisticPtr.counter.id = id;
    statisticPtr.counter.port = port;
    statisticPtr.counter.queue = queue;

    rc = cpssDxChTamThresholdSet(devNum, thresholdId, &statisticPtr, thresholdLevel);
    if (rc != GT_OK)    cpssOsPrintf("rc = %d", rc);

    return GT_OK;
}

GT_STATUS prvDebugThresholdsNotificationsGet(GT_U8 devNum)
{
    GT_STATUS                                   rc;
    GT_U32                                      numOfThresholds = PRV_CPSS_DXCH_TAM_FW_THRESHOLDS;
    CPSS_DXCH_TAM_THRESHOLD_BREACH_EVENT_STC    thresholdsArray[PRV_CPSS_DXCH_TAM_FW_THRESHOLDS];
    GT_U32                                      ii;

    cpssOsMemSet(thresholdsArray, 0, sizeof(thresholdsArray));
    rc = cpssDxChTamThresholdNotificationsGet(devNum, &numOfThresholds, thresholdsArray);
    if (rc != GT_OK)    cpssOsPrintf("rc = %d", rc);

    cpssOsPrintf("out numOfThresholds=%d\n", numOfThresholds);

    for (ii = 0; ii < numOfThresholds; ii++) 
    {
        cpssOsPrintf("thfresholdId=%3d val=0x%x=%d\n", 
                     thresholdsArray[ii].thresholdId,
                     thresholdsArray[ii].value.l[0],
                     thresholdsArray[ii].value.l[0]);
    }

    return GT_OK;
}

GT_STATUS ddd(GT_U8 devNum, GT_U8 h)
{
    GT_U8           ii, jj;
    GT_U32          rc, value, histogramDbAddr;

    for (ii = 0; ii < h; ii++)
    {
        histogramDbAddr = tamHistogramDbStartAddr + (ii * sizeof(PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC));

        cpssOsPrintf("ii=%d histogramDbAddr=0x%08x\n", ii, histogramDbAddr);

        for (jj = 0; jj < 8; jj++)
        {
            cpssOsPrintf("jj=%d histogramDbAddr=0x%08x off=%d ", jj, histogramDbAddr, HISTOGRAM_DB_BIN_VAL_STAT_0_OFFSET);
            cpssOsPrintf("add=0x%08x ", histogramDbAddr + HISTOGRAM_DB_BIN_VAL_STAT_0_OFFSET + (4 * jj));

            rc = prvCpssDxChTamSharedMemRead(devNum,
                                             histogramDbAddr + HISTOGRAM_DB_BIN_VAL_STAT_0_OFFSET + (4 * jj),
                                             &value);
            if (rc != GT_OK)    cpssOsPrintf("     xxxxx ");
            else                cpssOsPrintf("0x%08x ", value);
            cpssOsPrintf("\n");
        }

        for (jj = 0; jj < 8; jj++)
        {
            cpssOsPrintf("jj=%d histogramDbAddr=0x%08x off=%d ", jj, histogramDbAddr, HISTOGRAM_DB_BIN_VAL_STAT_0_OFFSET);
            cpssOsPrintf("add=0x%08x ", histogramDbAddr + HISTOGRAM_DB_BIN_VAL_STAT_1_OFFSET + (4 * jj));

            rc = prvCpssDxChTamSharedMemRead(devNum,
                                             histogramDbAddr + HISTOGRAM_DB_BIN_VAL_STAT_1_OFFSET + (4 * jj),
                                             &value);
            if (rc != GT_OK)    cpssOsPrintf("     xxxxx ");
            else                cpssOsPrintf("0x%08x ", value);
            cpssOsPrintf("\n");
        }

        cpssOsPrintf("\n");
    }

    return GT_OK;
}

GT_STATUS prvDebugPrintFreeCountersList(GT_U8 devNum, GT_U16 entries)
{
    GT_U32                       rc, value, counterDbAddr;
    GT_U16                       ii;

    cpssOsPrintf("freeCounterFirstAddr = 0x%08x freeCountersNum = %d\n", freeCounterFirstAddr, freeCountersNum);

    cpssOsPrintf("address            idx nextCounterAddr\n");
    counterDbAddr = freeCounterFirstAddr;
    ii = 0;
    while (counterDbAddr != 0)
    {
        cpssOsPrintf("0x%08x: ", counterDbAddr);
        cpssOsPrintf("%10d ", (counterDbAddr - tamCountersDbStartAddr)/sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC));

        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         counterDbAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                         &value);
        if (rc != GT_OK)    cpssOsPrintf("0xxxxxxxxx ");
        else                cpssOsPrintf("0x%08x ", value);
        cpssOsPrintf("\n");

        counterDbAddr = value;

        ii++;
        if (ii == entries) break;
    }

    return GT_OK;
}

GT_STATUS prvDebugPrintFreeCountersListWithData(GT_U8 devNum, GT_U16 entries)
{
    GT_U32                       rc, value, counterDbAddr;
    GT_U16                       ii, jj;

    cpssOsPrintf("freeCounterFirstAddr = 0x%08x freeCountersNum = %d\n", freeCounterFirstAddr, freeCountersNum);

    cpssOsPrintf("address       idx switchRegAddr      last0       min0       max0      coun0       sum0       sum0      last1       min1       max1      coun1       sum1       sum1 nextCountAddr\n");
    counterDbAddr = freeCounterFirstAddr;
    ii = 0;
    while (counterDbAddr != 0)
    {
        cpssOsPrintf("0x%08x: ", counterDbAddr);
        cpssOsPrintf("%5d ", (counterDbAddr - tamCountersDbStartAddr)/sizeof(PRV_CPSS_DXCH_TAM_COUNTER_DB_STC));

        rc = prvCpssDxChTamSharedMemRead(devNum, counterDbAddr, &value);
        if (rc != GT_OK)    cpssOsPrintf("0xxxxxxxxx    ");
        else                cpssOsPrintf("0x%08x    ", value);

        for (jj = 0; jj < ALL_STATISTICS_SIZE_IN_WORDS; jj++)
        {
            rc = prvCpssDxChTamSharedMemRead(devNum,
                                             counterDbAddr + COUNTER_DB_STAT_0_OFFSET + (4 * jj),
                                             &value);
            if (rc != GT_OK)    cpssOsPrintf("     xxxxx ");
            else                cpssOsPrintf("%10u ", value);
        }

        rc = prvCpssDxChTamSharedMemRead(devNum,
                                         counterDbAddr + COUNTER_DB_NEXT_COUNTER_ADDR_OFFSET,
                                         &value);
        if (rc != GT_OK)    cpssOsPrintf("0xxxxxxxxx ");
        else                cpssOsPrintf("0x%08x ", value);
        cpssOsPrintf("\n");

        counterDbAddr = value;

        ii++;
        if (ii == entries) break;
    }

    return GT_OK;
}

GT_STATUS prvDebugPrintFwMemory(GT_U8 devNum, GT_U32 addr, GT_U16 words, GT_U8 numWordsInLine)
{
    GT_U16                       ii;
    GT_U32                       rc, value;

    if (numWordsInLine <= 0) numWordsInLine = 8;
    cpssOsPrintf("numWordsInLine=%d\n", numWordsInLine);

    for (ii = 0; ii < words; ii++)
    {
        if ((ii % numWordsInLine) == 0) cpssOsPrintf("0x%08x: ", addr + (ii * 4));
        rc = prvCpssDxChTamSharedMemRead(devNum, addr + (ii * 4), &value);
        if (rc != GT_OK) cpssOsPrintf("xxxxxxxx ");
        else             cpssOsPrintf("%08x ", value);

        if (((ii + 1) % numWordsInLine) == 0) cpssOsPrintf("\n");
    }

    cpssOsPrintf("\n");

    return GT_OK;
}

GT_STATUS prv_size(GT_VOID)
{
    cpssOsPrintf("size=%d\n", sizeof(PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC));

    return GT_OK;
}

#if 0
GT_U8 packetData[64] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

GT_STATUS prvDebugSendTraffic(GT_U8 devNum, GT_U16 vid, GT_U32 dstPortNum)
{
    GT_STATUS   rc;
    CPSS_DXCH_NET_TX_PARAMS_STC pcktParams;
    GT_U8                       *buffList[1];
    GT_U32                      buffLenList[1];
    GT_HW_DEV_NUM               hwDevNum; /* HW device number */

    cpssOsMemSet(&pcktParams, '\0', sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
    pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
    pcktParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    pcktParams.dsaParam.commonParams.vid = vid;
    pcktParams.packetIsTagged = GT_TRUE;
    rc = cpssDxChCfgHwDevNumGet(0,&hwDevNum);
    if (rc != GT_OK) {
        cpssOsPrintf("cpssDxChCfgHwDevNumGet rc=%d\n", rc);
        return rc;
    }
    pcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = hwDevNum;
    pcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.portNum = dstPortNum;
    pcktParams.dsaParam.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
    pcktParams.dsaParam.dsaInfo.forward.srcHwDev = hwDevNum;
    pcktParams.dsaParam.dsaInfo.forward.srcIsTrunk = GT_FALSE;
    pcktParams.dsaParam.dsaInfo.forward.source.portNum = 63;
    pcktParams.dsaParam.dsaInfo.forward.isTrgPhyPortValid = GT_TRUE;
    pcktParams.dsaParam.dsaInfo.forward.dstEport = dstPortNum;
    pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_FALSE;
    pcktParams.dsaParam.dsaInfo.forward.origSrcPhy.portNum = 0;
    pcktParams.sdmaInfo.recalcCrc = GT_TRUE;
    pcktParams.sdmaInfo.txQueue = 0;

    buffLenList[0] = 64;
    buffList[0] = cpssOsCacheDmaMalloc(64);
    cpssOsMemCpy(buffList[0],packetData,buffLenList[0]);

    rc = cpssDxChNetIfSdmaSyncTxPacketSend(devNum,
                                            &pcktParams,
                                            buffList,
                                            buffLenList,
                                            1);
    if (rc != GT_OK) {
        cpssOsPrintf("cpssDxChNetIfSdmaSyncTxPacketSend rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}

GT_STATUS prvDebugSendTraffic2(GT_U8 devNum, GT_U16 vid, GT_U32 dstPortNum)
{
    GT_STATUS   rc;
    CPSS_DXCH_NET_TX_PARAMS_STC pcktParams;
    GT_U8                       *buffList[1];
    GT_U32                      buffLenList[1];
    GT_HW_DEV_NUM               hwDevNum; /* HW device number */

    cpssOsMemSet(&pcktParams, '\0', sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
    pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
    pcktParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    pcktParams.dsaParam.commonParams.vid = vid;
    pcktParams.packetIsTagged = GT_TRUE;
    rc = cpssDxChCfgHwDevNumGet(0,&hwDevNum);
    if (rc != GT_OK) {
        cpssOsPrintf("cpssDxChCfgHwDevNumGet rc=%d\n", rc);
        return rc;
    }
    pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;
    pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = dstPortNum;

    pcktParams.dsaParam.dsaInfo.fromCpu.tc = 0;
    pcktParams.dsaParam.dsaInfo.fromCpu.dp =  CPSS_DP_GREEN_E;

    pcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged = GT_TRUE;
    pcktParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;
    pcktParams.dsaParam.dsaInfo.fromCpu.dstEport = dstPortNum;

    pcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = hwDevNum;
    pcktParams.dsaParam.dsaInfo.fromCpu.srcHwDev = hwDevNum;

    buffLenList[0] = 64;
    buffList[0] = cpssOsCacheDmaMalloc(64);
    cpssOsMemCpy(buffList[0],packetData,buffLenList[0]);

    rc = cpssDxChNetIfSdmaSyncTxPacketSend(devNum,
                                            &pcktParams,
                                            buffList,
                                            buffLenList,
                                            1);
    if (rc != GT_OK) {
        cpssOsPrintf("cpssDxChNetIfSdmaSyncTxPacketSend rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}
#endif
#if 0
GT_STATUS prvDebugTraffic(GT_U8 devNum)
{
    GT_STATUS   rc;
    GT_U64      actualRateValue;
    GT_U64      rateValue;/* the rate in nanoseconds */
    CPSS_DXCH_NET_TX_PARAMS_STC pcktParams;
    GT_U8                       *buff;
    GT_U32                      buffLen;
    GT_HW_DEV_NUM               hwDevNum; /* HW device number */
    GT_U32                      packetId; /* returned packet identification number */

    rateValue.l[0] = 3300000;
    rateValue.l[1] = 0;
    rc = cpssDxChNetIfSdmaTxGeneratorRateSet(devNum,
                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                             0,
                                             CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E,
                                             rateValue,
                                             &actualRateValue);
    if (rc != GT_OK) {
        cpssOsPrintf("cpssDxChNetIfSdmaTxGeneratorRateSet rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChNetIfSdmaTxGeneratorEnable(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            0,
                                            1,
                                            100);
    if (rc != GT_OK) {
        cpssOsPrintf("cpssDxChNetIfSdmaTxGeneratorEnable rc=%d\n", rc);
        return rc;
    }

    cpssOsMemSet(&pcktParams, '\0', sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));
    pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
    pcktParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    pcktParams.dsaParam.commonParams.vid = 1;
    pcktParams.packetIsTagged = GT_TRUE;
    rc = cpssDxChCfgHwDevNumGet(0,&hwDevNum);
    if (rc != GT_OK) {
        cpssOsPrintf("cpssDxChCfgHwDevNumGet rc=%d\n", rc);
        return rc;
    }
    pcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = hwDevNum;
    pcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.portNum = 0;
    pcktParams.dsaParam.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
    pcktParams.dsaParam.dsaInfo.forward.srcHwDev = hwDevNum+1;
    pcktParams.dsaParam.dsaInfo.forward.srcIsTrunk = GT_FALSE;
    pcktParams.dsaParam.dsaInfo.forward.source.portNum = 63;
    pcktParams.dsaParam.dsaInfo.forward.isTrgPhyPortValid = GT_TRUE;
    pcktParams.dsaParam.dsaInfo.forward.dstEport = 0;
    pcktParams.dsaParam.dsaInfo.forward.origSrcPhyIsTrunk = GT_FALSE;
    pcktParams.dsaParam.dsaInfo.forward.origSrcPhy.portNum = 0;
    pcktParams.sdmaInfo.recalcCrc = GT_TRUE;
    pcktParams.sdmaInfo.txQueue = 0;

    buffLen = 64;
    buff = cpssOsCacheDmaMalloc(64);
    cpssOsMemCpy(buff,packetData,buffLen);

    rc = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum,
                                           CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                           &pcktParams,
                                           buff,
                                           buffLen,
                                           &packetId);
    if (rc != GT_OK) {
        cpssOsPrintf("cpssDxChNetIfSdmaSyncTxPacketSend rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}
#endif
