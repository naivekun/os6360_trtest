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
* @file cpssDxCh3Policer.c
*
* @brief CPSS DxCh3 Policing Engine API.
* The Policing Engine (or Traffic Conditioner) is responsible for:
* - Ingress Metering - implemented using SrTCM Token Bucket,
* TrTCM Token Bucket and Two Color Mode algorithms.
* - Egress Metering - implemented for XCAT devices
* - Ingress Billing - byte/packet counters that keep track of amount
* of traffic belonging to a flow.
* Re-Marking - QoS assigned to the packet according to metering
* results.
* - Egress Billing - implemented for XCAT and above devices
* Re-Marking - QoS assigned to the packet according to metering
* results.
*
* @version   83
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/policer/cpssGenPolicerTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxCh3/policer/cpssDxCh3Policer.h>
#include <cpss/dxCh/dxCh3/policer/private/prvCpssDxCh3Policer.h>
#include <cpss/dxCh/dxCh3/policer/private/prvCpssDxChPolicerLog.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>

/* from sip 5.15 (bobk) the IPLR 0,1          are sharing metering in with 'base address' between them */
/* from sip 5.20 (bc3)  the IPLR 0,1 and EPLR are sharing metering with 'base address' between them */
/* legacy devices shared IPLR 0,1 metering on 2 or 3 memories 'partition' */
#define PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL /* empty */

/* from sip 5.20 (bc3)  the IPLR 0,1 and EPLR are sharing counting with 'base address' between them */
/* legacy devices shared IPLR 0,1 counting on 2 or 3 memories 'partition' */
#define PLR_COUNTING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL /* empty */

/* the number of entries allocated for IPLR0 to use */
#define PLR0_SIZE_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]

/* the number of entries allocated for IPLR1 to use */
#define PLR1_SIZE_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]

/* the number of entries allocated for EPLR to use */
#define PLR2_SIZE_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[CPSS_DXCH_POLICER_STAGE_EGRESS_E]

/* falgs for Metering Configuration table. Each PLR unit get 2 flags:
   1) shared table - is the Metering Configuration table is shared or not.
   2) offsetIndex - does the entry index should include the base address offset from register "Policer Metering Base Address".
   */
#define IPLR0_IS_SHARED_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.isMeterConfigTableShared[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]

#define IPLR1_IS_SHARED_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.isMeterConfigTableShared[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]

#define EPLR_IS_SHARED_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.isMeterConfigTableShared[CPSS_DXCH_POLICER_STAGE_EGRESS_E]

#define IPLR0_ADD_BASE_ADDR_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.meterConfigTableAddBaseAddr[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]

#define IPLR1_ADD_BASE_ADDR_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.meterConfigTableAddBaseAddr[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]

#define EPLR_ADD_BASE_ADDR_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.meterConfigTableAddBaseAddr[CPSS_DXCH_POLICER_STAGE_EGRESS_E]



/* both meter entry and counting entry in the same width */
#define METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS  0x20

/* Number of DXCH3 Policer Metering/Counting Entries */
#define PRV_CPSS_DXCH3_POLICER_ENTRY_NUM_CNS                    1024

/* Max Rate ratio applied for CIR, PIR calculation for DxCh3 */
#define PRV_CPSS_DXCH3_POLICER_RATE_RATIO_CNS                   1023


/* Max Rate ratio applied for CIR, PIR calculation, for, xCat, xCat3 and Lion2*/
#define PRV_CPSS_DXCHXCAT_POLICER_RATE_RATIO_CNS                   102300

/* Max Rate ratio applied for CIR, PIR calculation for SIP5 and above*/
#define PRV_CPSS_SIP5_POLICER_RATE_RATIO_CNS                   131071

/* Max Burst Size ratio applied for CBS, PBS calculation */
#define PRV_CPSS_DXCH3_POLICER_BURST_SIZE_RATIO_CNS             65535

/* Number of rate type supported by DxCh3 Policer */
#define PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS                6

/* Check that Policer stage is Ingress #0 or Ingress #1 */
#define PRV_CPSS_DXCH_POLICER_INGRESS_STAGE_CHECK_MAC(_stage)              \
    (CPSS_DXCH_POLICER_STAGE_EGRESS_E - (_stage))

/* Policer counting entry size in words */
#define PRV_CPSS_DXCH_POLICER_COUNT_ENTRY_SIZE_CNS             8

/* Alignment of Policy Counters indexes in the Counting entry.
   Index alignment is 8 but only first 6 indexes are valid   */
#define PRV_CPSS_DXCH_POLICER_POLICY_CNTR_INDEX_ALIGNMENT_CNS   8

static GT_STATUS prvCpssDxChPolicerTriggerEntryToHwConvert
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr,
    OUT GT_U32                                  *hwDataPtr
);

static GT_STATUS prvCpssDxChPolicerTriggerEntryToSwConvert
(
    IN  GT_U8                                    devNum,
    IN   GT_U32                                  *hwDataPtr,
    OUT  CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
);

/* define for sip5 devices : the 3 memories that uses for partitions between
    iplr0 and iplr1 */
#define MEM_0   BIT_0
#define MEM_1   BIT_1
#define MEM_2   BIT_2

/************************************************************************/
/*                Metering Rate & Burst Ranges  for DxCh3, xCat, xCat3 and Lion2      */
/************************************************************************/
/*        |   Rate(Kbps) - CIR, PIR | Burst size(B) - CBS, EBS     |
--------------------------------------------------------------------------
|rateType | min    | max = min*1023 |     min    | max = min*65535 |
--------------------------------------------------------------------------
|   0     | 1      |       1,023    |     1      |       65,535    |
|   1     | 10     |      10,230    |     8      |      524,280    |
|   2     | 100    |     102,300    |     64     |    4,194,240    |
|   3     | 1000   |   1,023,000    |     512    |   33,553,920    |
|   4     | 10000  |  10,230,000    |     4096   |  268,431,360    |
|   5     | 100000 | 102,300,000    |     32768  | 2,147,450,880   |
*************************************************************************/

/************************************************************************/
/*         Metering Rate & Burst Ranges  for SIP5 and above         */
/************************************************************************/
/*        |   Rate(Kbps) - CIR, PIR | Burst size(B) - CBS, EBS     |
--------------------------------------------------------------------------
|rateType | min    | max = min*102300  |     min    | max = min*65535 |
--------------------------------------------------------------------------
|   0     | 1      |         65,535    |     1      |       65,535    |
|   1     | 10     |      1,310,710    |     8      |      524,280    |
|   2     | 100    |     13,107,100    |     64     |    4,194,240    |
|   3     | 1000   |    131,071,000    |     512    |   33,553,920    |
|   4     | 10000  |  1,310,710,000    |     4096   |  268,431,360    |
|   5     | 100000 | 13,107,100,000    |     32768  | 2,147,450,880   |
*************************************************************************/

static const GT_U32 prvMeterMinRateBurstRangesArray
    [PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS][2] = {{1, 1},
                                                     {10, 8},
                                                     {100, 64},
                                                     {1000, 512},
                                                     {10000, 4096},
                                                     {100000, 32768}
                                                    };

/*array of rate ratio. The array size is (PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS +1) 
  because prvCpssDxCh3PolicerHwTbCbsCalculate use rateType = PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS as the highest rateType */
static GT_U32 prvMeterMaxHwRate_Xcat_Array[PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS+1] =
{
    PRV_CPSS_DXCHXCAT_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCHXCAT_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCHXCAT_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCHXCAT_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCHXCAT_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCHXCAT_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCHXCAT_POLICER_RATE_RATIO_CNS
};/* Rate Ratio */

/*array of rate ratio. The array size is (PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS +1) 
  because prvCpssDxCh3PolicerHwTbCbsCalculate use rateType = PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS as the highest rateType */
static GT_U32 prvMeterMaxHwRateArray[PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS+1] =
{
    PRV_CPSS_DXCH3_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCH3_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCH3_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCH3_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCH3_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCH3_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_DXCH3_POLICER_RATE_RATIO_CNS
};/* Rate Ratio */

/*array of rate ratio for SIP5 and above. The array size is (PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS +1) 
  because prvCpssDxCh3PolicerHwTbCbsCalculate use rateType = PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS as the highest rateType */
static GT_U32 prvMeterMaxHwRate_SIP_5_Array[PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS+1] =
{
    PRV_CPSS_DXCH3_POLICER_BURST_SIZE_RATIO_CNS,
    PRV_CPSS_SIP5_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_SIP5_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_SIP5_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_SIP5_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_SIP5_POLICER_RATE_RATIO_CNS,
    PRV_CPSS_SIP5_POLICER_RATE_RATIO_CNS
};/* Rate Ratio for sip5*/

/* convert HW to SW */
#define GET_SW_PACKET_CMD_FROM_HW_MAC(_hwValue,swValue)                                 \
        switch(_hwValue)                                                                \
        {                                                                               \
            case 0: swValue = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;break;     \
            case 1: swValue = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;break;          \
            case 2: swValue = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E;break;        \
            case 3: swValue = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E;break;\
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);                                                    \
        }

/* number of bits in the Bobk - Metering configuration entry */
#define BOBK_POLICER_METERING_CONFIG_BITS_SIZE_CNS 51
#define BOBK_POLICER_METERING_CONFIG_WORDS_CNS BITS_TO_WORDS_MAC(BOBK_POLICER_METERING_CONFIG_BITS_SIZE_CNS)

/* convert enum CPSS_DXCH_POLICER_STORM_TYPE_ENT to start bit in sip6 */
#define SIP6_STORM_TYPE_TO_START_BIT_CONVERT_MAC(stormType,startBit)                         \
    switch (stormType)    {                                                                  \
        case CPSS_DXCH_POLICER_STORM_TYPE_UC_KNOWN_E:       startBit = 1;            break;  \
        case CPSS_DXCH_POLICER_STORM_TYPE_UC_UNKNOWN_E:     startBit = 3;            break;  \
        case CPSS_DXCH_POLICER_STORM_TYPE_MC_REGISTERED_E:  startBit = 5;            break;  \
        case CPSS_DXCH_POLICER_STORM_TYPE_MC_UNREGISTERED_E:startBit = 7;            break;  \
        case CPSS_DXCH_POLICER_STORM_TYPE_BC_E:             startBit = 9;            break;  \
        case CPSS_DXCH_POLICER_STORM_TYPE_TCP_SYN_E:        startBit = 11;           break;  \
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad storm type[%d]" , stormType ); \
    }


/**
* @internal prvCpssDxCh3PolicerHwTbCirCalculate function
* @endinternal
*
* @brief   Calculate the HW values of Token Bucket and Rate Type.
*         The calculation is CIR oriented which means that it looks for
*         the range with the highest granularity the CIR can fit into.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] swRate                   - Rate (CIR or PIR) in the SW format.
* @param[in] swBurstSize0             - Burst Size (CBS or PBS) in the SW format.
* @param[in] swBurstSize1             - Burst Size (EBS) in the SW format.
*                                      This parameter is used only upon SrTCM.
*
* @param[out] hwRatePtr                - Rate (CIR or PIR) in the HW format.
* @param[out] hwBurstSize0Ptr          - Burst Size (CBS or PBS) in the HW format.
* @param[out] hwBurstSize1Ptr          - Burst Size (EBS) in the HW format.
*                                      This parameter is used only upon SrTCM.
*                                      Upon TrTCM it should be NULL.
* @param[out] rateTypePtr              - pointer to the Rate Type.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong input parameters.
*
* @note Upon TrTCM the hwBurstSize1Ptr parameter should be NULL.
*
*/
static GT_STATUS prvCpssDxCh3PolicerHwTbCirCalculate
(
    IN  GT_U8   devNum,
    IN  GT_U32  swRate,
    IN  GT_U32  swBurstSize0,
    IN  GT_U32  swBurstSize1,
    OUT GT_U32  *hwRatePtr,
    OUT GT_U32  *hwBurstSize0Ptr,
    OUT GT_U32  *hwBurstSize1Ptr,
    OUT GT_U32  *rateTypePtr
)
{
    GT_U32  rateType;   /* Rate Type */
    GT_U32  *rateRatioArr;  /* Rate Ratio */

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        rateRatioArr = &prvMeterMaxHwRate_SIP_5_Array[0];
    }
    else
    {
        rateRatioArr = &prvMeterMaxHwRateArray[0];
    }

    /*******************************/
    /* Calculate CIR(PIR) HW value */
    /*******************************/
    for(rateType = 0 ; rateType < PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS;
        rateType++)
    {
        if(swRate <= (prvMeterMinRateBurstRangesArray[rateType][0] *
                                rateRatioArr[rateType]))
        {
            *hwRatePtr = swRate / prvMeterMinRateBurstRangesArray[rateType][0];
            break;
        }
    }

    if(rateType == PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS)
    {
        /* the user rate is over the max supported */
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            *hwRatePtr = BIT_17 - 1;  /* 17 bits max value */
        }
        else
        {
            *hwRatePtr = BIT_10 - 1;  /* 10 bits max value */
        }
        rateType = PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS - 1;
    }

    /***********************/
    /* Store the Rate Type */
    /***********************/
    *rateTypePtr = rateType;

    /*******************************/
    /* Calculate CBS(PBS) HW value */
    /*******************************/
    if(swBurstSize0 > (prvMeterMinRateBurstRangesArray[rateType][1] *
                            PRV_CPSS_DXCH3_POLICER_BURST_SIZE_RATIO_CNS))
    {
        *hwBurstSize0Ptr = 0xFFFF;     /* 16 bits */
    }
    else
    {
        *hwBurstSize0Ptr = swBurstSize0 /
            prvMeterMinRateBurstRangesArray[rateType][1];
    }

    /*************************************************/
    /* Calculate EBS HW value (SrTCM only parameter) */
    /*************************************************/
    if (hwBurstSize1Ptr != NULL)
    {
        if(swBurstSize1 > (prvMeterMinRateBurstRangesArray[rateType][1] *
                            PRV_CPSS_DXCH3_POLICER_BURST_SIZE_RATIO_CNS))
        {
            *hwBurstSize1Ptr = 0xFFFF;      /* 16 bits */
        }
        else
        {
            *hwBurstSize1Ptr = swBurstSize1 /
                prvMeterMinRateBurstRangesArray[rateType][1];
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxCh3PolicerHwTbCbsCalculate function
* @endinternal
*
* @brief   Calculate the HW values of Token Bucket and Rate Type.
*         The calculation is CBS/PBS oriented which means that it looks for
*         the range where the CBS/PBS value will be most accurate.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for DxChXcat and above devices
*                                      and ignored by DxCh3.
* @param[in] swRate                   - Rate (CIR/PIR) in the SW format.
* @param[in] swBurstSize0             - Burst Size (CBS/PBS) in the SW format.
* @param[in] swBurstSize1             - Burst Size (EBS) in the SW format.
*                                      This parameter is used only upon SrTCM.
*
* @param[out] hwRatePtr                - Rate (CIR/PIR) in the HW format.
* @param[out] hwBurstSize0Ptr          - Burst Size (CBS/PBS) in the HW format.
* @param[out] hwBurstSize1Ptr          - Burst Size (EBS) in the HW format.
*                                      This parameter is used only upon SrTCM.
*                                      Upon TrTCM it should be NULL.
* @param[out] rateTypePtr              - pointer to the Rate Type.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong input parameters.
*
* @note Upon TrTCM the hwBurstSize1Ptr parameter should be NULL.
*       rate\burst HW limitation: Due to HW limitation, the result must satisfy
*       the following relation -
*       <rate_value_in_HW> <= <burst_value_in_HW><burst_granularity>
*       Or in HW fields notion:
*       The value configured in <Rate0> must be configured to be less than or
*       equal to <Max Burst Size0>Burst Granularity - MRU.
*       The value configured in <Rate1> must be configured to be less than or
*       equal to <Max Burst Size1>Burst Granularity - MRU.
*       The Burst Granularity is a function of <Rate Type0>/<Rate Type1>.
*
*/
static GT_STATUS prvCpssDxCh3PolicerHwTbCbsCalculate
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32  swRate,
    IN  GT_U32  swBurstSize0,
    IN  GT_U32  swBurstSize1,
    OUT GT_U32  *hwRatePtr,
    OUT GT_U32  *hwBurstSize0Ptr,
    OUT GT_U32  *hwBurstSize1Ptr,
    OUT GT_U32  *rateTypePtr
)
{
    GT_U32  cirPirAllowedDeviation; /* rate allowed deviation */
    GT_U32  rateType;   /* Rate Type */
    GT_U32  rateTypeRateGranularity; /* Rate Type Rate granularity */
    GT_U32  rateTypeBurstGranularity = 0; /* Rate Type Burst granularity */
    GT_U32  swHighRate; /* Highest rate based on the allowed deviation */
    GT_U32  hwLowRate;  /* Lowest rate in HW due to granularity that still conforms with SW low rate */
    GT_U32  hwHighRate; /* Highest rate in HW due to granularity that still conforms with SW high rate */
    GT_U32  *policerRateRatioArr;/* rate range ratio between minimum (also granularity) and maximum */
    GT_U32  lastQualifiedCirRateType; /* the last rate type which qualified with the CIR limitations */
    GT_U32  lastQualifiedHwLowRate;   /* Lowest rate in HW due to granularity for the last rate type */
                                      /* which qualified with the CIR limitations */
    GT_BOOL lowRateIsZero; /* flag indicating if the lowest allowed rate is 0 */
    GT_U32  mru;           /* Policer MRU */
    GT_U32  actualMaxBucketSize; /* actual maximal bucket size. MRU is taken into account */
    GT_STATUS rc;                /* return status */

    /* get MRU */
    rc = cpssDxCh3PolicerMruGet(devNum,stage,&mru);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* For all devices the granularity of all rate types is the same, but DxCh3 has different */
    /* range (100 times smaller) for the same rate type.                                      */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        policerRateRatioArr = &prvMeterMaxHwRate_SIP_5_Array[0];
    }
    else
    {
        policerRateRatioArr = &prvMeterMaxHwRate_Xcat_Array[0];
    }

    cirPirAllowedDeviation = PRV_CPSS_DXCH_PP_MAC(devNum)->policer.cirPirAllowedDeviation;
    lowRateIsZero = GT_FALSE;
    if( cirPirAllowedDeviation >= 100 )
    {
        /* If allowed deviation is 100% or more, lowest acceptable\actual rate is 0 */
        /* and not negative one. */
        lowRateIsZero = GT_TRUE;
    }

    if( swRate > (0xFFFFFFFF/(cirPirAllowedDeviation + 100))*100 )
    {
        /* Maximal allowed rate is set to 0xFFFFFFFF (32 bits maximal value) in case due to the */
        /* allowed deviation the highest acceptable rate will be more then 0xFFFFFFFF.          */
        swHighRate = 0xFFFFFFFF;
    }
    else
    {
        /* The calculation for range high rate is: rate*(100+deviation)/100. In case the result of */
        /* (rate*(100+deviation)) will overflow above 0xFFFFFFFF (but still the final result is    */
        /* below 0xFFFFFFFF otherwise the if case had already set the high rate to 0xFFFFFFFF)     */
        /* some modulo arithmetic is done. Otherwise the straight forward formula is used.         */
        if( swRate >= 0xFFFFFFFF/(cirPirAllowedDeviation + 100) )
        {
            swHighRate = (swRate/100)*(cirPirAllowedDeviation + 100)
                         + ((swRate%100)*cirPirAllowedDeviation)/100;
        }
        else
        {
            swHighRate = (swRate * (cirPirAllowedDeviation + 100))/100;
        }
    }

    lastQualifiedCirRateType = PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS;
    lastQualifiedHwLowRate = 0;

    for(rateType = 0 ; rateType < PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS;
        rateType++)
    {
        rateTypeRateGranularity  = prvMeterMinRateBurstRangesArray[rateType][0];
        rateTypeBurstGranularity = prvMeterMinRateBurstRangesArray[rateType][1];

        /* hwLowRate & hwHighRate are the quantized range edges due to the rate type granularity. */

        if( GT_TRUE == lowRateIsZero )
        {
            /* If the lowest allowed rate value is 0 then HW value is 0 as well. */
            hwLowRate = 0;
        }
        else if ( ((swRate/policerRateRatioArr[rateType])*(100 - cirPirAllowedDeviation) +
                   ((swRate%policerRateRatioArr[rateType])*(100 - cirPirAllowedDeviation))/policerRateRatioArr[rateType]) >
                                                                        (rateTypeRateGranularity * 100) )
        {
            /* If the lowest allowed rate value is greater than the rate type maximal rate go to the    */
            /* next rate type. The straight forward theoretical check that the maximal rate is breached  */
            /* could be: SW_low_rate > Maximal_rate_of_rate_type                                        */
            /* or respectively: sw_rate * (100 - deviation)/100 > rate_granularity_of_rate_type * rate_ratio */
            /* Due to the fact that the product (sw_rate * (100 - deviation)) might be more than 32 bits  */
            /* and the fact that division operation in integer and not fractional (i.e. (x/y)*y might not */
            /* be equal x, the calculation above use some modulo arithmetic manipulations.                */
            continue;
        }
        else
        {
            /* The calculation for range low rate is: rate*(100-deviation)/100/granularity. Since the   */
            /* rate is not continuous but quantized by the granularity, its value is rounded up to the  */
            /* nearest value. This cause the modulo arithmetic in the second part of the formulas here  */
            /* below.                                                                                   */
            /* The distinction (if case) is required for cases in which the rate value is large and the */
            /* product (rate*(100-deviation)) exceed 0xFFFFFFFF. Otherwise the straight forward formula */
            /* can be used.                                                                             */
            if( swRate > 0xFFFFFFFF/(100 - cirPirAllowedDeviation))
            {
                hwLowRate = ((swRate/100)*(100 - cirPirAllowedDeviation) +
                             ((swRate%100)*(100 - cirPirAllowedDeviation)/100))/rateTypeRateGranularity +
                            ((((swRate/100)*(100 - cirPirAllowedDeviation) +
                               ((swRate%100)*(100 - cirPirAllowedDeviation)/100))
                                                                %rateTypeRateGranularity) == 0 ? 0 : 1 );
            }
            else
            {
                hwLowRate = swRate * (100 - cirPirAllowedDeviation) / (100 * rateTypeRateGranularity) +
                            (((swRate * (100 - cirPirAllowedDeviation)) % (100 * rateTypeRateGranularity)) == 0 ? 0 : 1);
            }
        }

        if( swHighRate < rateTypeRateGranularity )
        {
            /* Allowed high rate is below the rate type granularity. */
            hwHighRate = 0;
        }
        else if ( swHighRate/policerRateRatioArr[rateType] > rateTypeRateGranularity )
        {
            /* Allowed high rate is above the rate type maximal rate. */
            hwHighRate = policerRateRatioArr[rateType];
        }
        else
        {
            hwHighRate = swHighRate/rateTypeRateGranularity;
        }

        /* In case the quantized range (quantized low range edge to quantized high range edge) */
        /* is an empty group (contains no value) go to the next rate type.                     */
        if( hwLowRate > hwHighRate )
        {
            continue;
        }

        if( hwLowRate <= (0xFFFF * rateTypeBurstGranularity) )
        {
            /* This rate type can fulfill the rate requirements and the rate\burst HW limitation */
            /* without taking into consideration the burst constraint.                           */
            lastQualifiedCirRateType = rateType;
            lastQualifiedHwLowRate = hwLowRate;
        }
        else
        {
            /* Setting any allowed rate from the requested one will in this rate type will */
            /* violate the rate\burst HW limitation.                                       */
            /* Go to the next rate type.                                                   */
            continue;
        }

        *hwBurstSize0Ptr = swBurstSize0/rateTypeBurstGranularity;

        /* Check if required burst can be matched in the current rate type. */
        if( *hwBurstSize0Ptr <= 0xFFFF )
        {
            /* the MRU value should be decreased from bucket size in order to
              get actual size of bucket. The bucket size should be more then MRU.
              The bucket size less then MRU is used for debug purpose. */
            actualMaxBucketSize = *hwBurstSize0Ptr * rateTypeBurstGranularity;
            if (actualMaxBucketSize >= mru)
            {
                actualMaxBucketSize -= mru;
            }

            if( hwLowRate <= actualMaxBucketSize )
            {
                /* This rate type can be used to fulfill all requirements:  */
                /* - rate in requested range.                               */
                /* - burst as requested (up to granularity rounding).       */
                /* - rate\burst HW limitation obeyed.                       */

                /* Set selected rate type. */
                *rateTypePtr = rateType;

                /* Calculate naive HW rate */
                *hwRatePtr = swRate / rateTypeRateGranularity;

                if( *hwRatePtr < hwLowRate )
                {
                    /* If naive HW rate calculated lower then quantized range lowest rate, */
                    /* set selected HW rate to quantized range lowest rate.                */
                    *hwRatePtr = hwLowRate;
                }
                else if( *hwRatePtr > actualMaxBucketSize )
                {
                    /* If naive HW rate calculated violate rate\burst HW limitation, set HW rate */
                    /* to the maximal value conforming the limitation.                           */
                    *hwRatePtr = actualMaxBucketSize;
                }
                break;
            }

            /* If next rate type burst granularity is higher that the requested burst, no need to go */
            /* to next rate type. If the flow arrived here a rate type which fulfill the rate        */
            /* constraints was already been found.                                                   */
            if ( (rateType < (PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS - 1)) &&
                 (swBurstSize0 < prvMeterMinRateBurstRangesArray[rateType+1][1]) )
            {
                rateType = PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS;
                break;
            }
        }
    }

    if( PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS == lastQualifiedCirRateType )
    {
        /* Rate type which fulfill the rate constraints was not found. */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    else if( PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS == rateType )
    {
        /* No rate type which fulfill all requirements (rate, burst, rate\burst HW limitation was found. */
        /* Rate constraints and rate\burst HW limitation can be fulfilled.                               */
        if( GT_TRUE == PRV_CPSS_DXCH_PP_MAC(devNum)->policer.cbsPbsCalcOnFail )
        {
            /* Configure with burst best effort calculation. */

            /* Set selected rate type. */
            *rateTypePtr = lastQualifiedCirRateType;

            /* Calculate naive HW rate */
            *hwRatePtr = swRate / prvMeterMinRateBurstRangesArray[*rateTypePtr][0];

            if( *hwRatePtr > policerRateRatioArr[rateType] )
            {
                /* If naive HW rate calculated highest than rate type maximal rate set */
                /* set selected HW rate to rate type maximal rate.                     */
                *hwRatePtr = policerRateRatioArr[rateType];
            }
            else if (*hwRatePtr < lastQualifiedHwLowRate)
            {
                /* If naive HW rate calculated lower then quantized range lowest rate, */
                /* set selected HW rate to quantized range lowest rate.                */
                *hwRatePtr = lastQualifiedHwLowRate;
            }

            rateTypeBurstGranularity = prvMeterMinRateBurstRangesArray[*rateTypePtr][1];

            /* Calculate best effort burst. <Rate0> must be configured to be less than or
               equal to <Max Burst Size0>*Burst Granularity - MRU. */
            actualMaxBucketSize = swBurstSize0;
            if (actualMaxBucketSize >= mru)
            {
                actualMaxBucketSize -= mru;
            }

            if( (actualMaxBucketSize/rateTypeBurstGranularity) <= (*hwRatePtr/rateTypeBurstGranularity) )
            {
                /* Naive calcualtion of burst will give lower value than required by the rate\burst */
                /* HW limitation. Find the minimal value that will obey the limitation.             */
                if (actualMaxBucketSize != swBurstSize0)
                {
                    /* MRU should be taken in to account */
                    actualMaxBucketSize = *hwRatePtr + mru;
                }
                else
                {
                    /* burst size is less then MRU. Ignore MRU for calculation */
                    actualMaxBucketSize = *hwRatePtr;
                }

                *hwBurstSize0Ptr = actualMaxBucketSize/rateTypeBurstGranularity +
                                    (actualMaxBucketSize%rateTypeBurstGranularity == 0 ? 0: 1 );
            }
            else
            {
                /* Maximum burst required. */
                *hwBurstSize0Ptr = 0xFFFF;
            }
        }
        else /* GT_FALSE == PRV_CPSS_DXCH_PP_MAC(devNum)->policer.cbsPbsCalcOnFail */
        {
            /* No configuration with burst best effort calculation requested. */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

    }

    /*************************************************/
    /* Calculate EBS HW value (SrTCM only parameter) */
    /*************************************************/
    if (hwBurstSize1Ptr != NULL)
    {
        if(swBurstSize1 > (rateTypeBurstGranularity * PRV_CPSS_DXCH3_POLICER_BURST_SIZE_RATIO_CNS))
        {
            *hwBurstSize1Ptr = 0xFFFF;      /* 16 bits */
        }
        else
        {
            *hwBurstSize1Ptr = swBurstSize1 / rateTypeBurstGranularity;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxCh3PolicerHwTbCalculate function
* @endinternal
*
* @brief   Calculate the HW values of Token Bucket and Rate Type.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for DxChXcat and above devices
*                                      and ignored by DxCh3.
* @param[in] swRate                   - Rate (CIR or PIR) in the SW format.
* @param[in] swBurstSize0             - Burst Size (CBS or PBS) in the SW format.
* @param[in] swBurstSize1             - Burst Size (EBS) in the SW format.
*                                      This parameter is used only upon SrTCM.
*
* @param[out] hwRatePtr                - Rate (CIR or PIR) in the HW format.
* @param[out] hwBurstSize0Ptr          - Burst Size (CBS or PBS) in the HW format.
* @param[out] hwBurstSize1Ptr          - Burst Size (EBS) in the HW format.
*                                      This parameter is used only upon SrTCM.
*                                      Upon TrTCM it should be NULL.
* @param[out] rateTypePtr              - pointer to the Rate Type.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong input parameters.
*
* @note Upon TrTCM the hwBurstSize1Ptr parameter should be NULL.
*
*/
static GT_STATUS prvCpssDxCh3PolicerHwTbCalculate
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32  swRate,
    IN  GT_U32  swBurstSize0,
    IN  GT_U32  swBurstSize1,
    OUT GT_U32  *hwRatePtr,
    OUT GT_U32  *hwBurstSize0Ptr,
    OUT GT_U32  *hwBurstSize1Ptr,
    OUT GT_U32  *rateTypePtr
)
{
    if( CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E ==
        PRV_CPSS_DXCH_PP_MAC(devNum)->policer.meteringCalcMethod)
    {
        return prvCpssDxCh3PolicerHwTbCirCalculate(devNum,
                                                   swRate,
                                                   swBurstSize0,
                                                   swBurstSize1,
                                                   hwRatePtr,
                                                   hwBurstSize0Ptr,
                                                   hwBurstSize1Ptr,
                                                   rateTypePtr);
    }
    else /* CPSS_DXCH_POLICER_METERING_CALC_MODE_CBS_E */
    {
        return prvCpssDxCh3PolicerHwTbCbsCalculate(devNum,
                                                   stage,
                                                   swRate,
                                                   swBurstSize0,
                                                   swBurstSize1,
                                                   hwRatePtr,
                                                   hwBurstSize0Ptr,
                                                   hwBurstSize1Ptr,
                                                   rateTypePtr);
    }

}

/**
* @internal prvCpssDxChPolicerHwTbEnvelopeCalculate function
* @endinternal
*
* @brief   Calculate the HW values of Token Bucket and Rate Type.
*         The calculation oriented to tripple of rate, maxRate and burst size
*         which means that it looks for
*         the range where the values will be most accurate.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] swRate                   - Rate in the SW format.
* @param[in] swBurstSize              - Burst Size in the SW format.
* @param[in] swMaxRate                - Maximal Rate in the SW format.
*
* @param[out] hwRatePtr                - Rate in the HW format.
* @param[out] hwBurstSizePtr           - Burst Size in the HW format.
* @param[out] rateTypePtr              - pointer to the Rate Type.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS prvCpssDxChPolicerHwTbEnvelopeCalculate
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                   swRate,
    IN  GT_U32                                   swBurstSize,
    IN  GT_U32                                   swMaxRate,
    OUT GT_U32                                   *hwRatePtr,
    OUT GT_U32                                   *hwBurstSizePtr,
    OUT GT_U32                                   *rateTypePtr
)
{
    GT_U32   rateType;        /* Rate Type                  */
    GT_U32   matcRateType;    /* Matching Rate Type         */
    GT_U32   hwRateUnit;      /* HW Rate unit               */
    GT_U32   hwBurstUnit;     /* HW Burst unit              */
    GT_U32   hwRate;          /* HW Rate                    */
    GT_U32   hwBurst;         /* HW Burst                   */
    GT_U32   maxHwRate;       /* Maximal HW Rate value      */
    GT_U32   maxHwBurst;      /* Maximal HW Burst value     */
    GT_U32   lastRateType1;   /* last restriction Rate Type */
    GT_U32  mru;              /* Policer MRU                */
    GT_U32  actualMaxBucketSize; /* actual maximal bucket size. MRU is taken into account */
    GT_STATUS rc;             /* return status              */

    /* get MRU */
    rc = cpssDxCh3PolicerMruGet(devNum, stage, &mru);
    if (rc != GT_OK)
    {
        return rc;
    }

    maxHwRate  = BIT_17 - 1;
    maxHwBurst = BIT_16 - 1;

    /* search for the least rate type matcing swRate, swBurstSize, swMaxRate */
    matcRateType  = 0xFFFFFFFF; /* stamp */
    lastRateType1 = 0xFFFFFFFF; /* stamp */
    for (rateType = (PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS - 1);
         ((rateType + 1) > 0); rateType--) /* last loop when rateType == 0 */
    {
        hwRateUnit  = prvMeterMinRateBurstRangesArray[rateType][0];
        hwBurstUnit = prvMeterMinRateBurstRangesArray[rateType][1];

        if (swMaxRate > (hwRateUnit * maxHwRate))
        {
            break; /* unit is too litle */
        }
        if (swRate > (hwRateUnit * maxHwRate))
        {
            break; /* unit is too litle */
        }
        if (swBurstSize > (hwBurstUnit * maxHwBurst))
        {
            break; /* unit is too litle */
        }
        if ((swMaxRate != 0) && (swMaxRate < hwRateUnit))
        {
            continue; /* unit is too big */
        }
        if ((swRate != 0) && (swRate < hwRateUnit))
        {
            continue; /* unit is too big */
        }
        if ((swBurstSize != 0) && (swBurstSize < hwBurstUnit))
        {
            continue; /* unit is too big */
        }
        matcRateType = rateType;

        /* the HW resriction:                                                        */
        /* <rate_value_in_HW> <= ((<burst_value_in_HW> * <burst_granularity>) - MRU) */
        hwRate      = swRate / hwRateUnit;
        hwBurst     = swBurstSize / hwBurstUnit;
        actualMaxBucketSize = hwBurst * hwBurstUnit;
        if (actualMaxBucketSize > mru)
        {
            actualMaxBucketSize -= mru;
            if (hwRate <= actualMaxBucketSize)
            {
                lastRateType1 = rateType;
            }
        }
        /* when actualMaxBucketSize <= 0 we do not save the rateType */
        /* it not better the that least of matchings                 */
    }

    if (matcRateType == 0xFFFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (lastRateType1 != 0xFFFFFFFF)
    {
        /* Fix "hwRate <= hwBurst" resriction decreasing currency */
        matcRateType = lastRateType1;
    }

    hwRateUnit  = prvMeterMinRateBurstRangesArray[matcRateType][0];
    hwBurstUnit = prvMeterMinRateBurstRangesArray[matcRateType][1];
    hwRate      = swRate / hwRateUnit;
    hwBurst     = swBurstSize / hwBurstUnit;

    *hwRatePtr        = hwRate;
    *hwBurstSizePtr   = hwBurst;
    *rateTypePtr      = matcRateType;

    return GT_OK;
}

/**
* @internal prvCpssDxCh3PolicerSwTbCalculate function
* @endinternal
*
* @brief   Convert Token Bucket from the HW format to the SW format.
*
* @note   APPLICABLE DEVICES:      None.
* @param[in] meterMode                - Meter mode (SrTCM or TrTCM).
* @param[in] rateType0                - SrTCM Bucket Rate Type (or TrTCM Bucket 1 Rate Type).
* @param[in] rateType1                - TrTCM Bucket 2 Rate Type.
* @param[in] tbHwParamsPtr            - pointer to the HW Token Bucket format.
*
* @param[out] tbSwParamsPtr            - pointer to the SW Token Bucket format.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong input parameters.
*/
static GT_STATUS prvCpssDxCh3PolicerSwTbCalculate
(
    IN  CPSS_DXCH3_POLICER_METER_MODE_ENT       meterMode,
    IN  GT_U32                                  rateType0,
    IN  GT_U32                                  rateType1,
    IN  CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbHwParamsPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbSwParamsPtr
)
{
    if ((rateType0 > PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS - 1) |
        (rateType1 > PRV_CPSS_DXCH3_POLICER_RATE_TYPE_MAX_CNS - 1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
            tbSwParamsPtr->srTcmParams.cir = tbHwParamsPtr->srTcmParams.cir *
                prvMeterMinRateBurstRangesArray[rateType0][0];

            tbSwParamsPtr->srTcmParams.cbs = tbHwParamsPtr->srTcmParams.cbs *
                prvMeterMinRateBurstRangesArray[rateType0][1];

            tbSwParamsPtr->srTcmParams.ebs = tbHwParamsPtr->srTcmParams.ebs *
                prvMeterMinRateBurstRangesArray[rateType0][1];
            break;
        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
            tbSwParamsPtr->trTcmParams.cir = tbHwParamsPtr->trTcmParams.cir *
                prvMeterMinRateBurstRangesArray[rateType0][0];

            tbSwParamsPtr->trTcmParams.pir = tbHwParamsPtr->trTcmParams.pir *
                prvMeterMinRateBurstRangesArray[rateType1][0];

            tbSwParamsPtr->trTcmParams.cbs = tbHwParamsPtr->trTcmParams.cbs *
                prvMeterMinRateBurstRangesArray[rateType0][1];

            tbSwParamsPtr->trTcmParams.pbs = tbHwParamsPtr->trTcmParams.pbs *
                prvMeterMinRateBurstRangesArray[rateType1][1];
            break;
        case  CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E:
        case  CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E:
            tbSwParamsPtr->envelope.cir = tbHwParamsPtr->envelope.cir *
                prvMeterMinRateBurstRangesArray[rateType1][0];
            tbSwParamsPtr->envelope.cbs = tbHwParamsPtr->envelope.cbs *
                prvMeterMinRateBurstRangesArray[rateType1][1];
            tbSwParamsPtr->envelope.eir = tbHwParamsPtr->envelope.eir *
                prvMeterMinRateBurstRangesArray[rateType1][0];
            tbSwParamsPtr->envelope.ebs = tbHwParamsPtr->envelope.ebs *
                prvMeterMinRateBurstRangesArray[rateType1][1];
            tbSwParamsPtr->envelope.maxCir = tbHwParamsPtr->envelope.maxCir *
                prvMeterMinRateBurstRangesArray[rateType1][0];
            tbSwParamsPtr->envelope.maxEir = tbHwParamsPtr->envelope.maxEir *
                prvMeterMinRateBurstRangesArray[rateType1][0];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal sip5_20IndirectCountingTableAccessWa function
* @endinternal
*
* @brief   implement the WA for :
*         The Policer Billing counting table expected to support indirect 'read'
*         and 'read and reset' to support
*         atomic operation and counters coherency.
*         However, due to this erratum those actions are not applicable when
*         <counting mode> is 'disabled'.
*         The WA should do :
*         a. Instead of indirect 'read' - do direct read.
*         b. Instead of indirect 'read and reset' - do direct read , indirect write of ZERO content.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupId              - the portGroupId. to support multi-port-groups device
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryOffset              - The offset from the base address of the
*                                      accessed entry.
* @param[in] triggerCmd               - Triggered command that the Policer needs to perform.
*                                      one of : 'read only' / 'read and reset' / 'read and write'
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
*/
static GT_STATUS sip5_20IndirectCountingTableAccessWa
(
    IN GT_U8                                        devNum,
    IN  GT_U32                                      portGroupId,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN GT_U32                                       entryOffset,
    IN PRV_CPSS_DXCH3_POLICER_ACCESS_CTRL_CMD_ENT   triggerCmd
)
{
    GT_STATUS rc;
    GT_U32    directReadHwData[8];     /* buffer for direct read  */
    GT_U32    policerTableAccessDataAddr;/* addr for indirect write data */
    GT_U32    policerTableAccessCtrlAddr;/* addr for indirect control register */
    CPSS_DXCH_TABLE_ENT tableType; /* table for direct read   */

    /****************************************/
    /* the indirect action is not supported */
    /****************************************/

    /* Get Ingress Policer Table Access Data 0 Register Address */
    policerTableAccessDataAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                policerTableAccessData[0];

    tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E;

    if (stage == CPSS_DXCH_POLICER_STAGE_INGRESS_1_E)
    {
        entryOffset += PLR0_SIZE_MAC(devNum);
    }
    else if (stage == CPSS_DXCH_POLICER_STAGE_EGRESS_E)
    {
        entryOffset += PLR0_SIZE_MAC(devNum) + PLR1_SIZE_MAC(devNum);
    }

    /***************************/
    /* do direct read of entry */
    /***************************/
    rc = prvCpssDxChPortGroupReadTableEntry(devNum, portGroupId ,tableType ,
        entryOffset, &directReadHwData[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(triggerCmd != PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E)
    {
        /*******************************************************************/
        /* do flush cache , so HW entry will hold information from 'cache' */
        /*******************************************************************/
        rc = prvCpssDxCh3PolicerInternalTableAccess(devNum,portGroupId,stage,
            entryOffset,/* not used for WBF flush */
            PRV_CPSS_DXCH3_POLICER_CNTR_WBF_FLUSH_E);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* NOTE : update of the 'Shadow DB' (for SER) is expected to be regardless
           to this WA ! */

        if(triggerCmd == PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_RESET_E)
        {
            static GT_U32 resetHwData[8] = {0,0,0,0,0,0,0,0};/* buffer for reset entry */

            /* wait for bit 0 to clear */
            /* before we can update Policer Table Access Data registers */
            policerTableAccessCtrlAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerTableAccessCtrl;
            rc = prvCpssPortGroupBusyWait(devNum,portGroupId,policerTableAccessCtrlAddr,0,GT_FALSE);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Write 'reset values' data to Policer Table Access Data registers */
            rc = prvCpssHwPpPortGroupWriteRam(devNum, portGroupId,  policerTableAccessDataAddr, 8, &resetHwData[0]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else /* PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E */
        {
            /* the caller already set the needed info in 'Policer Table Access Data registers' */
        }

        /***************************************/
        /* trigger indirect write to the entry */
        /***************************************/
        rc = prvCpssDxCh3PolicerInternalTableAccess(devNum,portGroupId,stage,
            entryOffset,
            PRV_CPSS_SIP_5_20_POLICER_CNTR_WRITE_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* wait for bit 0 to clear */
    /* before we can update Policer Table Access Data registers */
    policerTableAccessCtrlAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerTableAccessCtrl;
    rc = prvCpssPortGroupBusyWait(devNum,portGroupId,policerTableAccessCtrlAddr,0,GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* write the 'read buffer' from 'direct' to Policer Table Access Data registers */
    /* so the caller of this function continues ... 'as usual' */
    rc = prvCpssHwPpPortGroupWriteRam(devNum, portGroupId , policerTableAccessDataAddr, 8 , &directReadHwData[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


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
* @param[in] portGroupId              - the portGroupId. to support multi-port-groups device
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
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regValue;       /* register value */
    GT_U32      regMask;        /* mask for selecting bits in the register */
    GT_STATUS   retStatus;      /* generic return status code */
    CPSS_DXCH_POLICER_COUNTING_MODE_ENT countingMode;/* global counting mode */
    GT_U32      hw_triggerCmd;  /* HW value for 'triggerCmd' */

#ifdef GM_USED
    if(triggerCmd == PRV_CPSS_DXCH3_POLICER_CNTR_WBF_FLUSH_E &&
       PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) &&
       !PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))/* in sip5.20 the 'WBF_FLUSH' moved to registers without 'trigger' */
    {
        /* currently the GM not do 'self clear' to the trigger bit when operating
           this function ... so to not damage other functionality ...

           anyway this functionality relevant only to real HW.
        */
        CPSS_TBD_BOOKMARK_BOBCAT2_BOBK
        return GT_OK;
    }
    else
    if(triggerCmd == PRV_CPSS_SIP_5_20_POLICER_CNTR_WRITE_E)
    {
        /* currently the GM not do 'self clear' to the trigger bit when operating
           this function ... so to not damage other functionality ...
        */
        CPSS_TBD_BOOKMARK_BOBCAT3
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }
#endif

    if(((triggerCmd == PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_RESET_E) ||
        (triggerCmd == PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E) ||
        (triggerCmd == PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E)) &&
        PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
        PRV_CPSS_DXCH_BOBCAT3_PLR_INDIRECT_READ_COUNTERS_ON_DISABLED_MODE_WA_E))
    {
        /* get the <counting mode> */
        retStatus = cpssDxChPolicerCountingModeGet(devNum,stage,&countingMode);
        if(retStatus != GT_OK)
        {
            return retStatus;
        }
#ifdef GM_USED
        /* the GM not supports operation PRV_CPSS_SIP_5_20_POLICER_CNTR_WRITE_E
            that is part of the WA (see above GM code for PRV_CPSS_SIP_5_20_POLICER_CNTR_WRITE_E)
        */
        countingMode = CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E;
#endif

        /* check if <counting mode> is disabled */
        if(countingMode == CPSS_DXCH_POLICER_COUNTING_DISABLE_E)
        {
            /* implement the WA */
            return sip5_20IndirectCountingTableAccessWa(devNum,portGroupId,
                stage,entryOffset,triggerCmd);
        }

        /* continue as usual ... */
    }

    if(triggerCmd == PRV_CPSS_DXCH3_POLICER_CNTR_WBF_FLUSH_E &&
       PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl1;

        /* Flushing the cache by writing 0 into bit[13] */
        retStatus = prvCpssHwPpSetRegField(devNum, regAddr, 13, 1, 0);
        if(retStatus != GT_OK)
        {
            return retStatus;
        }

        /* Reset bit[13] to it's default value */
        retStatus = prvCpssHwPpSetRegField(devNum, regAddr, 13, 1, 1);
        if(retStatus != GT_OK)
        {
            return retStatus;
        }

        return GT_OK;
    }

    /* Check Triggered command */
    switch(triggerCmd)
    {
        case PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_RESET_E:
        case PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E:
        case PRV_CPSS_DXCH3_POLICER_METERING_UPDATE_E:
        case PRV_CPSS_DXCH3_POLICER_METERING_REFRESH_E:
        case PRV_CPSS_DXCH3_POLICER_CNTR_WBF_FLUSH_E:
            break;
        case PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E:
            /* check that the device is Lion B0 and above */
            if(0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_READ_E:
        case PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_RESET_E:
        case PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_READ_AND_RESET_E:
        case PRV_CPSS_SIP_5_20_POLICER_CNTR_WRITE_E:
            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                policerTableAccessCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].policerTblAccessControlReg;
    }

    /* In case of read&write the busyWait should be done before writing to Access Data registers */
    /* same for 'write counters' */
    if (triggerCmd != PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E &&
        triggerCmd != PRV_CPSS_SIP_5_20_POLICER_CNTR_WRITE_E)
    {
        /* wait for bit 0 to clear */
        retStatus = prvCpssPortGroupBusyWait(devNum,portGroupId,regAddr,0,GT_FALSE);
        if(retStatus != GT_OK)
        {
            return retStatus;
        }
    }

    {
        /***********************************************************/
        /* NO change needed for INDIRECT access to counting tables */
        /***********************************************************/
        PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
        PLR_COUNTING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
    }


    regMask = 0xFFFF008F; /* Mask [3:0], [7] and [31:16] ranges */
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regMask |= BIT_4;
    }

    hw_triggerCmd = triggerCmd & 0xFF;/* support 8 bits to allow value of 0x102
                                         to become 0x2
                                      */

    /* If triggerCmd is "READ_AND_WRITE" bit 7 should be set,
       thus (triggerCmd << 1) will set this bit*/

    /* Set the Policer Table Access Control fields: */
    /* <TriggerCommand>, <Access Trigger>, <Update Billing Counter mode>
       and <EntryOffset> */
    regValue = 0x1 | (hw_triggerCmd << 1) | (entryOffset << 16);

    /* Now we can write to the register and ask for command implementation */
    retStatus =
        prvCpssHwPpPortGroupWriteRegBitMask(devNum, portGroupId, regAddr,
                                               regMask, regValue);
    if(retStatus != GT_OK)
    {
        return retStatus;
    }

    if ((PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_RESET_E == triggerCmd) ||
        (PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E == triggerCmd) ||
        (PRV_CPSS_SIP_5_20_POLICER_CNTR_WRITE_E  == triggerCmd) ||
        (PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E == triggerCmd) ||
        (PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_READ_E  == triggerCmd) ||
        (PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_RESET_E  == triggerCmd) ||
        (PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_READ_AND_RESET_E  == triggerCmd)
        )
    {
        /* wait for bit 0 to clear */
        retStatus = prvCpssPortGroupBusyWait(devNum,portGroupId,regAddr,0,GT_FALSE);
        if(retStatus != GT_OK)
        {
            return retStatus;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPolicerBillingCountersModeSet function
* @endinternal
*
* @brief   Sets the Billing Counters resolution.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] cntrMode                 - Billing Counters resolution: 1 Byte,
*                                      16 Byte or packet.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or cntrMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS prvCpssDxChPolicerBillingCountersModeSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT     cntrMode
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */

    /* Convert Billing Counters resolution to the Reg Value */
    switch(cntrMode)
    {
        case CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E:
            regValue = 0;
            break;
        case CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E:
            regValue = 1;
            break;
        case CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E:
            regValue = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControlReg;

    /* Set Ingress Policer Control register, <Billing_Counters_Mode> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 1 , 2, regValue);
}


/**
* @internal prvCpssDxChPolicerBillingCountersModeGet function
* @endinternal
*
* @brief   Gets the Billing Counters resolution.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] cntrModePtr              - pointer to the Billing Counters resolution:
*                                      1 Byte, 16 Byte or packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS prvCpssDxChPolicerBillingCountersModeGet
(
    IN GT_U8                                        devNum,
    OUT CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT    *cntrModePtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControlReg;

    /* Get Ingress Policer Control register, <Billing_Counters_Mode> field */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 1, 2, &regValue);

    if (GT_OK == retStatus)
    {
        /* Convert HW Value to the Billing Counters resolution */
        switch(regValue)
        {
            case 0:
                *cntrModePtr = CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E;
                break;
            case 1:
                *cntrModePtr = CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E;
                break;
            case 2:
                *cntrModePtr = CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return retStatus;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables metering per device.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] enable                   - Enable/disable metering:
*                                      GT_TRUE  - metering is enabled on the device.
*                                      GT_FALSE - metering is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note If metering is enabled, it can be triggered
*       either by Policy engine or per port.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_BOOL                              enable
)
{
    GT_U32    regAddr;    /* register address */
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
        PRV_CPSS_DXCH_CAELUM_RM_PLR_METERING_INACCURACY_IN_START_TRAFFIC_WA_E))
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).debug_PrngControl1;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1,
            BOOL2BIT_MAC(enable));
        if(rc != GT_OK)
        {
            return rc;
        }
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).debug_PrngControl2;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1,
                BOOL2BIT_MAC(enable));
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Set Policer Control register, <Metering_Mode> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxCh3PolicerMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables metering per device.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] enable                   - Enable/disable metering:
*                                      GT_TRUE  - metering is enabled on the device.
*                                      GT_FALSE - metering is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note If metering is enabled, it can be triggered
*       either by Policy engine or per port.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_BOOL                              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enable));

    rc = internal_cpssDxCh3PolicerMeteringEnableSet(devNum, stage, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringEnableGet function
* @endinternal
*
* @brief   Gets device metering status (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] enablePtr                - pointer to Enable/Disable metering:
*                                      GT_TRUE  - metering is enabled on the device.
*                                      GT_FALSE - metering is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note If metering is enabled, it can be triggered
*       either by Policy engine or per port.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Get Policer Control register, <Metering_Mode> field */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &regValue);

    if (GT_OK == retStatus)
    {
        *enablePtr = BIT2BOOL_MAC(regValue);
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerMeteringEnableGet function
* @endinternal
*
* @brief   Gets device metering status (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] enablePtr                - pointer to Enable/Disable metering:
*                                      GT_TRUE  - metering is enabled on the device.
*                                      GT_FALSE - metering is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note If metering is enabled, it can be triggered
*       either by Policy engine or per port.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enablePtr));

    rc = internal_cpssDxCh3PolicerMeteringEnableGet(devNum, stage, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerBillingCountingEnableSet function
* @endinternal
*
* @brief   Enables or disables Billing Counting.
*         Billing is the process of counting the amount of traffic that
*         belongs to a flow and can be activated only by metering.
*         When metering is activated per source port, Billing counting
*         cannot be enabled.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - enable/disable Billing Counting:
*                                      GT_TRUE  - Billing Counting is enabled on the device.
*                                      GT_FALSE - Billing Counting is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerBillingCountingEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControlReg;

    /* Set Ingress Policer Control register, <Counting_Mode> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 3, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxCh3PolicerBillingCountingEnableSet function
* @endinternal
*
* @brief   Enables or disables Billing Counting.
*         Billing is the process of counting the amount of traffic that
*         belongs to a flow and can be activated only by metering.
*         When metering is activated per source port, Billing counting
*         cannot be enabled.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - enable/disable Billing Counting:
*                                      GT_TRUE  - Billing Counting is enabled on the device.
*                                      GT_FALSE - Billing Counting is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerBillingCountingEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerBillingCountingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxCh3PolicerBillingCountingEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerBillingCountingEnableGet function
* @endinternal
*
* @brief   Gets device Billing Counting status (Enable/Disable).
*         Billing is the process of counting the amount of traffic that
*         belongs to a flow and can be activated only by metering.
*         When metering is activated per source port, Billing counting
*         cannot be enabled.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - pointer to Enable/Disable Billing Counting:
*                                      GT_TRUE  - Billing Counting is enabled on the device.
*                                      GT_FALSE - Billing Counting is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerBillingCountingEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControlReg;

    /* Get Ingress Policer Control register, <Counting_Mode> field */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 3, 1, &regValue);

    if (GT_OK == retStatus)
    {
        *enablePtr = BIT2BOOL_MAC(regValue);
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerBillingCountingEnableGet function
* @endinternal
*
* @brief   Gets device Billing Counting status (Enable/Disable).
*         Billing is the process of counting the amount of traffic that
*         belongs to a flow and can be activated only by metering.
*         When metering is activated per source port, Billing counting
*         cannot be enabled.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - pointer to Enable/Disable Billing Counting:
*                                      GT_TRUE  - Billing Counting is enabled on the device.
*                                      GT_FALSE - Billing Counting is disabled on the device.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerBillingCountingEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerBillingCountingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxCh3PolicerBillingCountingEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerCountingModeSet function
* @endinternal
*
* @brief   Configures counting mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
* @param[in] mode                     -  counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerCountingModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_COUNTING_MODE_ENT      mode
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldValue;     /* field value */
    GT_STATUS   rc;             /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    switch (mode)
    {
        case CPSS_DXCH_POLICER_COUNTING_DISABLE_E:
            fieldValue = 0;
            break;
        case CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E:
            fieldValue = 1;
            break;
        case CPSS_DXCH_POLICER_COUNTING_POLICY_E:
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                /* removed feature !!! (use CNC for counting) */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            fieldValue = 2;
            break;
        case CPSS_DXCH_POLICER_COUNTING_VLAN_E:
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                /* removed feature !!! (use CNC for counting) */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
            {
                /* Not supported in xCat2 */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }

            fieldValue = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* set EPCL EPLR Global Configuration register before EPLR register
       when disable counting */
    if ((stage == CPSS_DXCH_POLICER_STAGE_EGRESS_E) &&
        (mode == CPSS_DXCH_POLICER_COUNTING_DISABLE_E) &&
        (GT_FALSE == PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.eplrGlobalConfig;

        /* Set Egress Policer Global Configuration register,
          <Counting Mode > field */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 2,
                                         fieldValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Set Policer Control register, <Metering_Mode> field */
    rc = prvCpssHwPpSetRegField(
        devNum, regAddr, 1, 2, fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set EPCL EPLR Global Configuration register after EPLR register
       when enable counting */
    if ((stage == CPSS_DXCH_POLICER_STAGE_EGRESS_E) &&
        (mode != CPSS_DXCH_POLICER_COUNTING_DISABLE_E) &&
        (GT_FALSE == PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.eplrGlobalConfig;

        /* Set Egress Policer Global Configuration register,
          <Counting Mode > field */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 2,
                                         fieldValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal cpssDxChPolicerCountingModeSet function
* @endinternal
*
* @brief   Configures counting mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
* @param[in] mode                     -  counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerCountingModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_COUNTING_MODE_ENT      mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerCountingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, mode));

    rc = internal_cpssDxChPolicerCountingModeSet(devNum, stage, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerCountingModeGet function
* @endinternal
*
* @brief   Gets the couning mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
*
* @param[out] modePtr                  - pointer to Counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerCountingModeGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_COUNTING_MODE_ENT     *modePtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      fieldValue;     /* field value */
    GT_STATUS   retStatus;      /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Set Policer Control register, <Metering_Mode> field */
    retStatus = prvCpssHwPpGetRegField(
        devNum, regAddr, 1, 2, &fieldValue);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    switch (fieldValue)
    {
        case 0:
            *modePtr = CPSS_DXCH_POLICER_COUNTING_DISABLE_E;
            break;
        case 1:
            *modePtr = CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E;
            break;
        case 2:
            *modePtr = CPSS_DXCH_POLICER_COUNTING_POLICY_E;
            break;
        case 3:
            *modePtr = CPSS_DXCH_POLICER_COUNTING_VLAN_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPolicerCountingModeGet function
* @endinternal
*
* @brief   Gets the couning mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
*
* @param[out] modePtr                  - pointer to Counting mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerCountingModeGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_COUNTING_MODE_ENT     *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerCountingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, modePtr));

    rc = internal_cpssDxChPolicerCountingModeGet(devNum, stage, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerBillingCountersModeSet function
* @endinternal
*
* @brief   Sets the Billing Counters resolution.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] cntrMode                 - Billing Counters resolution: 1 Byte,
*                                      16 Byte or packet.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or cntrMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerBillingCountersModeSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT     cntrMode
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    return prvCpssDxChPolicerBillingCountersModeSet(devNum,
                                                    cntrMode);
}

/**
* @internal cpssDxCh3PolicerBillingCountersModeSet function
* @endinternal
*
* @brief   Sets the Billing Counters resolution.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] cntrMode                 - Billing Counters resolution: 1 Byte,
*                                      16 Byte or packet.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or cntrMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerBillingCountersModeSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT     cntrMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerBillingCountersModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrMode));

    rc = internal_cpssDxCh3PolicerBillingCountersModeSet(devNum, cntrMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerBillingCountersModeGet function
* @endinternal
*
* @brief   Gets the Billing Counters resolution.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] cntrModePtr              - pointer to the Billing Counters resolution:
*                                      1 Byte, 16 Byte or packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerBillingCountersModeGet
(
    IN GT_U8                                        devNum,
    OUT CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT    *cntrModePtr
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(cntrModePtr);

    return prvCpssDxChPolicerBillingCountersModeGet(devNum, cntrModePtr);
}

/**
* @internal cpssDxCh3PolicerBillingCountersModeGet function
* @endinternal
*
* @brief   Gets the Billing Counters resolution.
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] cntrModePtr              - pointer to the Billing Counters resolution:
*                                      1 Byte, 16 Byte or packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerBillingCountersModeGet
(
    IN GT_U8                                        devNum,
    OUT CPSS_DXCH3_POLICER_BILLING_CNTR_MODE_ENT    *cntrModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerBillingCountersModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrModePtr));

    rc = internal_cpssDxCh3PolicerBillingCountersModeGet(devNum, cntrModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerPacketSizeModeSet function
* @endinternal
*
* @brief   Sets metered Packet Size Mode that metering and billing is done
*         according to.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] packetSize               - Type of packet size:
*                                      - L3 datagram size only (this does not include
*                                      the L2 header size and packets CRC).
*                                      - L2 packet length including.
*                                      - L1 packet length including
*                                      (preamble + IFG + FCS).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or packetSize.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerPacketSizeModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_POLICER_PACKET_SIZE_MODE_ENT    packetSize
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */
    GT_U32  length;     /* length of bit field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* Convert metering and counting packet size to the Reg Value */
    regValue = 0;
    switch(packetSize)
    {
        case CPSS_POLICER_PACKET_SIZE_L3_ONLY_E:
            length = 1;
            break;
        case CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E:
        case CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E:
            if (packetSize == CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E)
            {
                regValue = 0x2;
            }
            regValue |= 0x1;
            length = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Set the following fields of Ingress Policer Control register:    */
    /* <Packet_Size_Mode> and <Include_Layer_1_Overhead>, if required.  */
    return prvCpssHwPpSetRegField(devNum, regAddr, 16, length, regValue);
}

/**
* @internal cpssDxCh3PolicerPacketSizeModeSet function
* @endinternal
*
* @brief   Sets metered Packet Size Mode that metering and billing is done
*         according to.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] packetSize               - Type of packet size:
*                                      - L3 datagram size only (this does not include
*                                      the L2 header size and packets CRC).
*                                      - L2 packet length including.
*                                      - L1 packet length including
*                                      (preamble + IFG + FCS).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or packetSize.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPacketSizeModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_POLICER_PACKET_SIZE_MODE_ENT    packetSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerPacketSizeModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, packetSize));

    rc = internal_cpssDxCh3PolicerPacketSizeModeSet(devNum, stage, packetSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, packetSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerPacketSizeModeGet function
* @endinternal
*
* @brief   Gets metered Packet Size Mode that metering and billing is done
*         according to.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] packetSizePtr            - pointer to the Type of packet size:
*                                      - L3 datagram size only (this does not include
*                                      the L2 header size and packets CRC).
*                                      - L2 packet length including.
*                                      - L1 packet length including (preamble + IFG + FCS).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerPacketSizeModeGet
(
    IN  GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT      stage,
    OUT CPSS_POLICER_PACKET_SIZE_MODE_ENT    *packetSizePtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(packetSizePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Get the following fields of Ingress Policer Control register: */
    /* <Packet_Size_Mode> and <Include_Layer_1_Overhead>             */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 16, 2, &regValue);

    if (GT_OK == retStatus)
    {
        switch(regValue)
        {
            case 0:
            case 2:
                *packetSizePtr = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;
                break;
            case 1:
                *packetSizePtr = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
                break;
            case 3:
                *packetSizePtr = CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerPacketSizeModeGet function
* @endinternal
*
* @brief   Gets metered Packet Size Mode that metering and billing is done
*         according to.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] packetSizePtr            - pointer to the Type of packet size:
*                                      - L3 datagram size only (this does not include
*                                      the L2 header size and packets CRC).
*                                      - L2 packet length including.
*                                      - L1 packet length including (preamble + IFG + FCS).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPacketSizeModeGet
(
    IN  GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT      stage,
    OUT CPSS_POLICER_PACKET_SIZE_MODE_ENT    *packetSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerPacketSizeModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, packetSizePtr));

    rc = internal_cpssDxCh3PolicerPacketSizeModeGet(devNum, stage, packetSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, packetSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMeterResolutionSet function
* @endinternal
*
* @brief   Sets metering algorithm resolution: packets per
*         second or bytes per second.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] resolution               - packet/Byte based Meter resolution.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or resolution.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerMeterResolutionSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT  resolution
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* Convert Meter resolution to the Reg Value */
    switch(resolution)
    {
        case CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E:
            regValue = 0;
            break;
        case CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E:
            regValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerControlReg;

    /* Set Ingress Policer Control reg, <Packet_Based_Meter_Enable> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 4, 1, regValue);
}

/**
* @internal cpssDxCh3PolicerMeterResolutionSet function
* @endinternal
*
* @brief   Sets metering algorithm resolution: packets per
*         second or bytes per second.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] resolution               - packet/Byte based Meter resolution.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or resolution.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeterResolutionSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT  resolution
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeterResolutionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, resolution));

    rc = internal_cpssDxCh3PolicerMeterResolutionSet(devNum, stage, resolution);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, resolution));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMeterResolutionGet function
* @endinternal
*
* @brief   Gets metering algorithm resolution: packets per
*         second or bytes per second.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] resolutionPtr            - pointer to the Meter resolution: packet or Byte based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerMeterResolutionGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT *resolutionPtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(resolutionPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerControlReg;

    /* Get Ingress Policer Control reg, <Packet_Based_Meter_Enable> field */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 4, 1, &regValue);

    if (GT_OK == retStatus)
    {
        /* Convert HW Value to the Meter resolution */
        *resolutionPtr = (0 == regValue) ?
            CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E :
            CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E;
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerMeterResolutionGet function
* @endinternal
*
* @brief   Gets metering algorithm resolution: packets per
*         second or bytes per second.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] resolutionPtr            - pointer to the Meter resolution: packet or Byte based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeterResolutionGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT *resolutionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeterResolutionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, resolutionPtr));

    rc = internal_cpssDxCh3PolicerMeterResolutionGet(devNum, stage, resolutionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, resolutionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerDropTypeSet function
* @endinternal
*
* @brief   Sets the Policer out-of-profile drop command type.
*         This setting controls if non-conforming dropped packets
*         (Red or Yellow) will be SOFT DROP or HARD DROP.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] dropType                 - Policer Drop Type: Soft or Hard.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, dropType or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerDropTypeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_DROP_MODE_TYPE_ENT              dropType
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /*Egress Trap/Mirror Supported from Falcon*/
    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (stage > 1)
        {
            /* not supported for Egress Policer */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    /* Convert Policer Drop Type to the Reg Value */
    switch(dropType)
    {
        case CPSS_DROP_MODE_SOFT_E:
            regValue = 0;
            break;
        case CPSS_DROP_MODE_HARD_E:
            regValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Set IPLR Control register, <Non_Conforming_Packet_Drop_Mode> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 5, 1, regValue);
}

/**
* @internal cpssDxCh3PolicerDropTypeSet function
* @endinternal
*
* @brief   Sets the Policer out-of-profile drop command type.
*         This setting controls if non-conforming dropped packets
*         (Red or Yellow) will be SOFT DROP or HARD DROP.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] dropType                 - Policer Drop Type: Soft or Hard.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, dropType or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerDropTypeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_DROP_MODE_TYPE_ENT              dropType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerDropTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, dropType));

    rc = internal_cpssDxCh3PolicerDropTypeSet(devNum, stage, dropType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, dropType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerDropTypeGet function
* @endinternal
*
* @brief   Gets the Policer out-of-profile drop command type.
*         This setting controls if non-conforming dropped packets
*         (Red or Yellow) will be SOFT DROP or HARD DROP.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] dropTypePtr              - pointer to the Policer Drop Type: Soft or Hard.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerDropTypeGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DROP_MODE_TYPE_ENT             *dropTypePtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(dropTypePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if (stage > 1)
    {
        /* not supported for Egress Policer */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Get IPLR Control register, <Non_Conforming_Packet_Drop_Mode> field */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 5, 1, &regValue);

    if (GT_OK == retStatus)
    {
        /* Convert HW Value to the Meter resolution */
        *dropTypePtr = (0 == regValue) ? CPSS_DROP_MODE_SOFT_E :
                                         CPSS_DROP_MODE_HARD_E;
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerDropTypeGet function
* @endinternal
*
* @brief   Gets the Policer out-of-profile drop command type.
*         This setting controls if non-conforming dropped packets
*         (Red or Yellow) will be SOFT DROP or HARD DROP.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] dropTypePtr              - pointer to the Policer Drop Type: Soft or Hard.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerDropTypeGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DROP_MODE_TYPE_ENT             *dropTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerDropTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, dropTypePtr));

    rc = internal_cpssDxCh3PolicerDropTypeGet(devNum, stage, dropTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, dropTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerCountingColorModeSet function
* @endinternal
*
* @brief   Sets the Policer color counting mode.
*         The Color Counting can be done according to the packet's
*         Drop Precedence or Conformance Level.
*         This affects both the Billing and Management counters.
*         If the packet was subject to remarking, the drop precedence used
*         here is AFTER remarking.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mode                     - Color counting mode: Drop Precedence or
*                                      Conformance Level.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerCountingColorModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT  mode
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* Convert Color counting mode to the Reg Value */
    switch(mode)
    {
        case CPSS_DXCH3_POLICER_COLOR_COUNT_CL_E:
            regValue = 0;
            break;
        case CPSS_DXCH3_POLICER_COLOR_COUNT_DP_E:
            regValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Set Ingress Policer Control register, <Counter_Color_Mode> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 7, 1, regValue);
}

/**
* @internal cpssDxCh3PolicerCountingColorModeSet function
* @endinternal
*
* @brief   Sets the Policer color counting mode.
*         The Color Counting can be done according to the packet's
*         Drop Precedence or Conformance Level.
*         This affects both the Billing and Management counters.
*         If the packet was subject to remarking, the drop precedence used
*         here is AFTER remarking.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mode                     - Color counting mode: Drop Precedence or
*                                      Conformance Level.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerCountingColorModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerCountingColorModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, mode));

    rc = internal_cpssDxCh3PolicerCountingColorModeSet(devNum, stage, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerCountingColorModeGet function
* @endinternal
*
* @brief   Gets the Policer color counting mode.
*         The Color Counting can be done according to the packet's
*         Drop Precedence or Conformance Level.
*         This affects both the Billing and Management counters.
*         If the packet was subject to remarking, the drop precedence used
*         here is AFTER remarking.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] modePtr                  - pointer to the color counting mode:
*                                      Drop Precedence or Conformance Level.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerCountingColorModeGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT     *modePtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Get Ingress Policer Control register, <Counter_Color_Mode> field */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 7, 1, &regValue);

    if (GT_OK == retStatus)
    {
        /* Convert HW Value to the Meter resolution */
        *modePtr = (0 == regValue) ? CPSS_DXCH3_POLICER_COLOR_COUNT_CL_E :
                                     CPSS_DXCH3_POLICER_COLOR_COUNT_DP_E;
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerCountingColorModeGet function
* @endinternal
*
* @brief   Gets the Policer color counting mode.
*         The Color Counting can be done according to the packet's
*         Drop Precedence or Conformance Level.
*         This affects both the Billing and Management counters.
*         If the packet was subject to remarking, the drop precedence used
*         here is AFTER remarking.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] modePtr                  - pointer to the color counting mode:
*                                      Drop Precedence or Conformance Level.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerCountingColorModeGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH3_POLICER_COLOR_COUNT_MODE_ENT     *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerCountingColorModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, modePtr));

    rc = internal_cpssDxCh3PolicerCountingColorModeGet(devNum, stage, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerManagementCntrsResolutionSet function
* @endinternal
*
* @brief   Sets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] cntrSet                  - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] cntrResolution           - Management Counters resolution: 1 or 16 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or
*                                       Management Counters Set or cntrResolution.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerManagementCntrsResolutionSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT          cntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT   cntrResolution
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */
    GT_U32  offset;     /* bit offset in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* Convert Management Counters Set to the bit offset */
    switch(cntrSet)
    {
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E:
            offset = 8;
            break;
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E:
            offset = 9;
            break;
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E:
            offset = 10;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Convert Management counters resolution to the Reg Value */
    switch(cntrResolution)
    {
        case CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_1B_E:
            regValue = 0;
            break;
        case CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_16B_E:
            regValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Set one of the following Ingress Policer Control register */
    /* fields: <Counter_Set_0_DU_Mode>, <Counter_Set_1_DU_Mode>  */
    /* or <Counter_Set_2_DU_Mode>.                               */
    return prvCpssHwPpSetRegField(devNum, regAddr, offset , 1, regValue);
}

/**
* @internal cpssDxCh3PolicerManagementCntrsResolutionSet function
* @endinternal
*
* @brief   Sets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] cntrSet                  - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] cntrResolution           - Management Counters resolution: 1 or 16 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or
*                                       Management Counters Set or cntrResolution.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerManagementCntrsResolutionSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT          cntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT   cntrResolution
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerManagementCntrsResolutionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, cntrSet, cntrResolution));

    rc = internal_cpssDxCh3PolicerManagementCntrsResolutionSet(devNum, stage, cntrSet, cntrResolution);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, cntrSet, cntrResolution));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerManagementCntrsResolutionGet function
* @endinternal
*
* @brief   Gets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] cntrSet                  - Management Counters Set (APPLICABLE RANGES: 0..2).
*
* @param[out] cntrResolutionPtr        - pointer to the Management Counters
*                                      resolution: 1 or 16 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or Mng Counters Set.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerManagementCntrsResolutionGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT         cntrSet,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT  *cntrResolutionPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      offset;     /* bit offset in the register */
    GT_STATUS   retStatus;  /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(cntrResolutionPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* Convert Management Counters Set to the bit offset */
    switch(cntrSet)
    {
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E:
            offset = 8;
            break;
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E:
            offset = 9;
            break;
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E:
            offset = 10;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Get one of the following Ingress Policer Control register */
    /* fields: <Counter_Set_0_DU_Mode>, <Counter_Set_1_DU_Mode>  */
    /* or <Counter_Set_2_DU_Mode>.                               */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr,
                                                    offset, 1, &regValue);
    if (GT_OK == retStatus)
    {
        *cntrResolutionPtr = (regValue)?
                            CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_16B_E :
                            CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_1B_E;
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerManagementCntrsResolutionGet function
* @endinternal
*
* @brief   Gets the Management counters resolution (either 1 Byte or 16 Bytes).
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] cntrSet                  - Management Counters Set (APPLICABLE RANGES: 0..2).
*
* @param[out] cntrResolutionPtr        - pointer to the Management Counters
*                                      resolution: 1 or 16 Bytes.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or Mng Counters Set.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerManagementCntrsResolutionGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT         cntrSet,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_ENT  *cntrResolutionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerManagementCntrsResolutionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, cntrSet, cntrResolutionPtr));

    rc = internal_cpssDxCh3PolicerManagementCntrsResolutionGet(devNum, stage, cntrSet, cntrResolutionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, cntrSet, cntrResolutionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerPacketSizeModeForTunnelTermSet function
* @endinternal
*
* @brief   Sets size mode for metering and counting of tunnel terminated packets.
*         The Policer provides the following modes to define packet size:
*         - Regular packet metering and counting. The packet size is defined
*         by the cpssDxCh3PolicerPacketSizeModeSet().
*         - Passenger packet metering and counting.
*         Metering and counting of TT packets is performed according
*         to L3 datagram size only. This mode does not include the tunnel
*         header size, the L2 header size, and the packet CRC in the metering
*         and counting.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] ttPacketSizeMode         - Tunnel Termination Packet Size Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or ttPacketSizeMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerPacketSizeModeForTunnelTermSet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT  ttPacketSizeMode
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* Convert Tunnel Termination Packet Size Mode to the Reg Value */
    switch(ttPacketSizeMode)
    {
        case CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E:
            regValue = 0;
            break;
        case CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E:
            regValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Set IPLR Control register, <Tunnel_Termination_Packet_Size_Mode> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 24, 1, regValue);
}

/**
* @internal cpssDxCh3PolicerPacketSizeModeForTunnelTermSet function
* @endinternal
*
* @brief   Sets size mode for metering and counting of tunnel terminated packets.
*         The Policer provides the following modes to define packet size:
*         - Regular packet metering and counting. The packet size is defined
*         by the cpssDxCh3PolicerPacketSizeModeSet().
*         - Passenger packet metering and counting.
*         Metering and counting of TT packets is performed according
*         to L3 datagram size only. This mode does not include the tunnel
*         header size, the L2 header size, and the packet CRC in the metering
*         and counting.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] ttPacketSizeMode         - Tunnel Termination Packet Size Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or ttPacketSizeMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPacketSizeModeForTunnelTermSet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT  ttPacketSizeMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerPacketSizeModeForTunnelTermSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, ttPacketSizeMode));

    rc = internal_cpssDxCh3PolicerPacketSizeModeForTunnelTermSet(devNum, stage, ttPacketSizeMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, ttPacketSizeMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerPacketSizeModeForTunnelTermGet function
* @endinternal
*
* @brief   Gets size mode for metering and counting of tunnel terminated packets.
*         The Policer provides the following modes to define packet size:
*         - Regular packet metering and counting. The packet size is defined
*         by the cpssDxCh3PolicerPacketSizeModeSet().
*         - Passenger packet metering and counting.
*         Metering and counting of TT packets is performed according
*         to L3 datagram size only. This mode does not include the tunnel
*         header size, the L2 header size, and the packet CRC in the metering
*         and counting.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] ttPacketSizeModePtr      - pointer to the Tunnel Termination
*                                      Packet Size Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerPacketSizeModeForTunnelTermGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT  *ttPacketSizeModePtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(ttPacketSizeModePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Get IPLR Control register, <Tunnel_Termination_Packet_Size_Mode> field */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 24, 1, &regValue);
    if (GT_OK == retStatus)
    {
        *ttPacketSizeModePtr = (regValue)?
            CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E :
            CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E;
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerPacketSizeModeForTunnelTermGet function
* @endinternal
*
* @brief   Gets size mode for metering and counting of tunnel terminated packets.
*         The Policer provides the following modes to define packet size:
*         - Regular packet metering and counting. The packet size is defined
*         by the cpssDxCh3PolicerPacketSizeModeSet().
*         - Passenger packet metering and counting.
*         Metering and counting of TT packets is performed according
*         to L3 datagram size only. This mode does not include the tunnel
*         header size, the L2 header size, and the packet CRC in the metering
*         and counting.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] ttPacketSizeModePtr      - pointer to the Tunnel Termination
*                                      Packet Size Mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPacketSizeModeForTunnelTermGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT  *ttPacketSizeModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerPacketSizeModeForTunnelTermGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, ttPacketSizeModePtr));

    rc = internal_cpssDxCh3PolicerPacketSizeModeForTunnelTermGet(devNum, stage, ttPacketSizeModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, ttPacketSizeModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet function
* @endinternal
*
* @brief   Enables or disables the metering Auto Refresh Scan mechanism.
*         If no traffic passes through a meter (long periods of "silence") the
*         meter state variables do not get refreshed. This can cause a problem of
*         state misinterpreted once the flow traffic is restarted, because the
*         timers can have already wrapped around.
*         To avoid this problem the software must initiate a refresh transaction
*         on the meter every ten minutes.
*         This CPU intensive metering refresh maintanance can be offloaded by
*         Auto Refresh Scan mechanism.
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - Enable/disable Auto Refresh Scan mechanism:
*                                      GT_TRUE  - enable Auto Refresh scan mechanism on
*                                      the device.
*                                      GT_FALSE - disable Auto Refresh scan mechanism on
*                                      the device.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The CPU is still capable to perform any Refresh Transactions during
*       the Auto Refresh Scan operation.
*       Once the CPU triggers Refresh transaction, the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControlReg;

    /* Set IPLR Control register, <Metering_Refresh_Scan_Enable> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 11, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet function
* @endinternal
*
* @brief   Enables or disables the metering Auto Refresh Scan mechanism.
*         If no traffic passes through a meter (long periods of "silence") the
*         meter state variables do not get refreshed. This can cause a problem of
*         state misinterpreted once the flow traffic is restarted, because the
*         timers can have already wrapped around.
*         To avoid this problem the software must initiate a refresh transaction
*         on the meter every ten minutes.
*         This CPU intensive metering refresh maintanance can be offloaded by
*         Auto Refresh Scan mechanism.
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - Enable/disable Auto Refresh Scan mechanism:
*                                      GT_TRUE  - enable Auto Refresh scan mechanism on
*                                      the device.
*                                      GT_FALSE - disable Auto Refresh scan mechanism on
*                                      the device.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The CPU is still capable to perform any Refresh Transactions during
*       the Auto Refresh Scan operation.
*       Once the CPU triggers Refresh transaction, the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet function
* @endinternal
*
* @brief   Gets metering Auto Refresh Scan mechanism status (Enabled/Disabled).
*         If no traffic passes through a meter (long periods of "silence") the
*         meter state variables do not get refreshed. This can cause a problem of
*         state misinterpreted once the flow traffic is restarted, because the
*         timers can have already wrapped around.
*         To avoid this problem the software must initiate a refresh transaction
*         on the meter every ten minutes.
*         This CPU intensive metering refresh maintanance can be offloaded by
*         Auto Refresh Scan mechanism.
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - pointer on Auto Refresh Scan mechanism status
*                                      (Enable/Disable) :
*                                      GT_TRUE  - Auto Refresh scan mechanism is enabled.
*                                      GT_FALSE - Auto Refresh scan mechanism is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The CPU is still capable to perform any Refresh Transactions during
*       the Auto Refresh Scan operation.
*       Once the CPU triggers Refresh transaction, the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControlReg;

    /* Get IPLR Control register, <Metering_Refresh_Scan_Enable> field */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 11, 1, &regValue);

    if (GT_OK == retStatus)
    {
        *enablePtr = BIT2BOOL_MAC(regValue);
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet function
* @endinternal
*
* @brief   Gets metering Auto Refresh Scan mechanism status (Enabled/Disabled).
*         If no traffic passes through a meter (long periods of "silence") the
*         meter state variables do not get refreshed. This can cause a problem of
*         state misinterpreted once the flow traffic is restarted, because the
*         timers can have already wrapped around.
*         To avoid this problem the software must initiate a refresh transaction
*         on the meter every ten minutes.
*         This CPU intensive metering refresh maintanance can be offloaded by
*         Auto Refresh Scan mechanism.
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - pointer on Auto Refresh Scan mechanism status
*                                      (Enable/Disable) :
*                                      GT_TRUE  - Auto Refresh scan mechanism is enabled.
*                                      GT_FALSE - Auto Refresh scan mechanism is disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The CPU is still capable to perform any Refresh Transactions during
*       the Auto Refresh Scan operation.
*       Once the CPU triggers Refresh transaction, the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxCh3PolicerMeteringAutoRefreshScanEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringAutoRefreshRangeSet function
* @endinternal
*
* @brief   Sets Metering Refresh Scan address range (Start and Stop addresses).
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] startAddress             - beginning of the address range to be scanned
*                                      by the Auto Refresh Scan mechanism (APPLICABLE RANGES: 0..1023).
* @param[in] stopAddress              - end of the address range to be scanned by the
*                                      Auto Refresh Scan mechanism (APPLICABLE RANGES: 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or illegal values
*                                       of startAddress and stopAddress.
* @retval GT_OUT_OF_RANGE          - on out of range of startAddress
*                                       or stopAddress.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringAutoRefreshRangeSet
(
    IN GT_U8    devNum,
    IN GT_U32   startAddress,
    IN GT_U32   stopAddress
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    if(startAddress >= BIT_16 || stopAddress >= BIT_16)  /*16 bits in HW */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Check pointed startAddressPtr value less than stopAddressPtr value */
    if(startAddress > stopAddress)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[0].policerRefreshScanRangeReg;

    /* Create HW data should be set to the register */
    regValue = startAddress | (stopAddress << 16);

    /* Set IPLR Metering refresh scan address range register: */
    /* <Metering_Scan_Start_Address> and <Metering_Scan_Stop_Address> fields */
    return prvCpssHwPpWriteRegister(devNum, regAddr, regValue);
}

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshRangeSet function
* @endinternal
*
* @brief   Sets Metering Refresh Scan address range (Start and Stop addresses).
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] startAddress             - beginning of the address range to be scanned
*                                      by the Auto Refresh Scan mechanism (APPLICABLE RANGES: 0..1023).
* @param[in] stopAddress              - end of the address range to be scanned by the
*                                      Auto Refresh Scan mechanism (APPLICABLE RANGES: 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or illegal values
*                                       of startAddress and stopAddress.
* @retval GT_OUT_OF_RANGE          - on out of range of startAddress
*                                       or stopAddress.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshRangeSet
(
    IN GT_U8    devNum,
    IN GT_U32   startAddress,
    IN GT_U32   stopAddress
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringAutoRefreshRangeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startAddress, stopAddress));

    rc = internal_cpssDxCh3PolicerMeteringAutoRefreshRangeSet(devNum, startAddress, stopAddress);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startAddress, stopAddress));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringAutoRefreshRangeGet function
* @endinternal
*
* @brief   Gets Metering Refresh Scan address range (Start and Stop addresses).
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] startAddressPtr          - pointer to the beginning address of Refresh
*                                      Scan address range.
* @param[out] stopAddressPtr           - pointer to the end address of Refresh Scan
*                                      address range.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringAutoRefreshRangeGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *startAddressPtr,
    OUT GT_U32  *stopAddressPtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(startAddressPtr);
    CPSS_NULL_PTR_CHECK_MAC(stopAddressPtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[0].policerRefreshScanRangeReg;

    /* Get IPLR Metering refresh scan address range register */
    retStatus = prvCpssHwPpReadRegister(devNum, regAddr, &regValue);

    if (GT_OK == retStatus)
    {
        /* Extract <Metering_Scan_Start_Address> and
           <Metering_Scan_Stop_Address> fields from the HW register value */
        *startAddressPtr    = regValue & 0xFFFF;
        *stopAddressPtr     = regValue >> 16;
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshRangeGet function
* @endinternal
*
* @brief   Gets Metering Refresh Scan address range (Start and Stop addresses).
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The Auto Refresh Scan Rate can be controlled by setting time interval
*         between two refresh access to metering table. The time interval is
*         configured by the cpssDxCh3PolicerMeteringAutoRefreshIntervalSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] startAddressPtr          - pointer to the beginning address of Refresh
*                                      Scan address range.
* @param[out] stopAddressPtr           - pointer to the end address of Refresh Scan
*                                      address range.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointers.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshRangeGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *startAddressPtr,
    OUT GT_U32  *stopAddressPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringAutoRefreshRangeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startAddressPtr, stopAddressPtr));

    rc = internal_cpssDxCh3PolicerMeteringAutoRefreshRangeGet(devNum, startAddressPtr, stopAddressPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startAddressPtr, stopAddressPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringAutoRefreshIntervalSet function
* @endinternal
*
* @brief   Sets the time interval between two refresh access to metering table.
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] interval                 - time  between refresh of two entries
*                                      in micro seconds. Upon 270 Mhz core clock value
*                                      of DxCh3, (APPLICABLE RANGES: 0..15907286) microS.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on out of range of interval value.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringAutoRefreshIntervalSet
(
    IN GT_U8    devNum,
    IN GT_U32   interval
)
{
    GT_U32  regAddr;            /* register address */
    GT_U32  regValue;           /* register value */
    GT_U32  maxIntervalValue;   /* maximum interval value in microS */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    /* Calculate maximum interval value in microS */
    maxIntervalValue = 0xFFFFFFFF / PRV_CPSS_PP_MAC(devNum)->coreClock;

    /* Check time interval range */
    if(maxIntervalValue < interval)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[0].policerRefreshScanRateReg;

    /* Create HW data should be set to the register */
    regValue = interval * PRV_CPSS_PP_MAC(devNum)->coreClock;

    /* Set Ingress Policer Metering refresh Scan Rate Limit register */
    return prvCpssHwPpWriteRegister(devNum, regAddr, regValue);
}

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshIntervalSet function
* @endinternal
*
* @brief   Sets the time interval between two refresh access to metering table.
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] interval                 - time  between refresh of two entries
*                                      in micro seconds. Upon 270 Mhz core clock value
*                                      of DxCh3, (APPLICABLE RANGES: 0..15907286) microS.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on out of range of interval value.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshIntervalSet
(
    IN GT_U8    devNum,
    IN GT_U32   interval
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringAutoRefreshIntervalSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, interval));

    rc = internal_cpssDxCh3PolicerMeteringAutoRefreshIntervalSet(devNum, interval);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, interval));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringAutoRefreshIntervalGet function
* @endinternal
*
* @brief   Gets the time interval between two refresh access to metering table.
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] intervalPtr              - pointer to the time interval between refresh of two
*                                      entries in micro seconds. Upon 270 Mhz core clock
*                                      value of DxCh3, (APPLICABLE RANGES: 0..15907286) microS.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringAutoRefreshIntervalGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *intervalPtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(intervalPtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[0].policerRefreshScanRateReg;

    /* Get Ingress Policer Metering refresh Scan Rate Limit register */
    retStatus = prvCpssHwPpReadRegister(devNum, regAddr, &regValue);

    if (GT_OK == retStatus)
    {
        /* Calculate Rate limit HW value in micro second */
        *intervalPtr = regValue / PRV_CPSS_PP_MAC(devNum)->coreClock;
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerMeteringAutoRefreshIntervalGet function
* @endinternal
*
* @brief   Gets the time interval between two refresh access to metering table.
*         The Auto Refresh Scan mechanism is enabled by the
*         cpssDxCh3PolicerMeteringAutoRefreshScanEnableSet().
*         The range of Auto Refresh Scan is configurable by the
*         cpssDxCh3PolicerMeteringAutoRefreshRangeSet().
*
* @note   APPLICABLE DEVICES:      DxCh3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] intervalPtr              - pointer to the time interval between refresh of two
*                                      entries in micro seconds. Upon 270 Mhz core clock
*                                      value of DxCh3, (APPLICABLE RANGES: 0..15907286) microS.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMeteringAutoRefreshIntervalGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *intervalPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringAutoRefreshIntervalGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, intervalPtr));

    rc = internal_cpssDxCh3PolicerMeteringAutoRefreshIntervalGet(devNum, intervalPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, intervalPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringEntryRefresh function
* @endinternal
*
* @brief   Refresh the Policer Metering Entry.
*         Meter Bucket's state parameters refreshing are needed in order to
*         prevent mis-behavior due to wrap around of timers.
*         The wrap around problem can occur when there are long periods of
*         'silence' on the flow and the bucket's state parameters do not get
*         refreshed (meter is in the idle state). This causes a problem once the
*         flow traffic is re-started since the timers might have already wrapped
*         around which can cause a situation in which the bucket's state is
*         mis-interpreted and the incoming packet is marked as non-conforming
*         even though the bucket was actually supposed to be full.
*         To prevent this from happening the CPU needs to trigger a meter
*         refresh transaction at least once every 10 minutes per meter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Once the CPU triggers Refresh transaction the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringEntryRefresh
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              entryIndex
)
{
    return cpssDxChPolicerPortGroupMeteringEntryRefresh(
                                            devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            stage,
                                            entryIndex);
}

/**
* @internal cpssDxCh3PolicerMeteringEntryRefresh function
* @endinternal
*
* @brief   Refresh the Policer Metering Entry.
*         Meter Bucket's state parameters refreshing are needed in order to
*         prevent mis-behavior due to wrap around of timers.
*         The wrap around problem can occur when there are long periods of
*         'silence' on the flow and the bucket's state parameters do not get
*         refreshed (meter is in the idle state). This causes a problem once the
*         flow traffic is re-started since the timers might have already wrapped
*         around which can cause a situation in which the bucket's state is
*         mis-interpreted and the incoming packet is marked as non-conforming
*         even though the bucket was actually supposed to be full.
*         To prevent this from happening the CPU needs to trigger a meter
*         refresh transaction at least once every 10 minutes per meter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above
*                                      devices and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Once the CPU triggers Refresh transaction the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringEntryRefresh
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              entryIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringEntryRefresh);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, entryIndex));

    rc = internal_cpssDxCh3PolicerMeteringEntryRefresh(devNum, stage, entryIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, entryIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssDxChPolicerPortMeteringEnablerDataGet function
* @endinternal
*
* @brief   Get registers and bits offset for per port metering enabler.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] portNum                  - port number (including the CPU port).
*
* @param[out] portGroupIdPtr           - pointer to port group
* @param[out] plrRegAddrPtr            - pointer to PLR register address
* @param[out] plrRegBitOffsetPtr       - pointer to PLR register offset
* @param[out] epclRegAddrPtr           - pointer to PCL register address, may be NULL
* @param[out] epclRegBitOffsetPtr      - pointer to PCL register offset, may be NULL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS prvCpssDxChPolicerPortMeteringEnablerDataGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    OUT GT_U32                             *portGroupIdPtr,
    OUT GT_U32                             *plrRegAddrPtr,
    OUT GT_U32                             *plrRegBitOffsetPtr,
    OUT GT_U32                             *epclRegAddrPtr,
    OUT GT_U32                             *epclRegBitOffsetPtr
)
{
    GT_U32  regOffset;  /* register offset      */
    GT_U32   localPort;  /* local port - support multi-port-groups device */
    GT_U32  startBit;   /* the word's bit at which the field starts */

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    *portGroupIdPtr = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* should not get here */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "registers replaced with table");
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        *plrRegAddrPtr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                           policerPortMeteringReg[OFFSET_TO_WORD_MAC(portNum)];
        *plrRegBitOffsetPtr = OFFSET_TO_BIT_MAC(portNum);

        /* write it to all pipes of BC3 */
        *portGroupIdPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }
    else
    {
        if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum) &&
            (stage == CPSS_DXCH_POLICER_STAGE_EGRESS_E))
        {
            /* EPLR use 6 bits of global port */
            localPort = (GT_U8)(portNum & 0x3F);
        }
        else
        {
            localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
        }

        regOffset = 0;

        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat and above use two registers for per port enabler */
            if (localPort > 31)
                regOffset = 1;
        }

        /* Remap CPU Port to the offset in the Policer Port Metering Enable register */
        if(CPSS_CPU_PORT_NUM_CNS == localPort)
        {
            localPort = 31;
        }

        *plrRegAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerPortMeteringEnReg[regOffset];

        /* for Lion2 EPLR local port has 6 bits. Need to apply 5 bit mask */
        *plrRegBitOffsetPtr = OFFSET_TO_BIT_MAC(localPort);
    }

    if (epclRegAddrPtr == NULL)
    {
        return GT_OK;
    }

    /* xCat2 and above devices have enabler bits in EPCL unit for Egress Policer */
    if (PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(devNum) &&
        (stage == CPSS_DXCH_POLICER_STAGE_EGRESS_E) &&
        (GT_FALSE == PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum)))
    {
        /* EPCL use local port */
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);

        startBit = (localPort == CPSS_CPU_PORT_NUM_CNS) ?
                 PRV_CPSS_DXCH_PP_HW_INFO_HA_CPU_PORT_BIT_INDEX_MAC(devNum) :
                 localPort;

        /* EPCL - Port Based Metering Register */
        *epclRegAddrPtr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.
                                            eplrPortBasedMetering[OFFSET_TO_WORD_MAC(startBit)];

        *epclRegBitOffsetPtr = OFFSET_TO_BIT_MAC(startBit);
    }
    else
    {
        /* mark address as not set */
        *epclRegAddrPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        *epclRegBitOffsetPtr = 0;
    }

    return GT_OK;
}


/**
* @internal internal_cpssDxCh3PolicerPortMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables a port metering trigger for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] enable                   - Enable/Disable per-port metering for packets arriving
*                                      on this port:
*                                      GT_TRUE  - Metering is triggered on specified port.
*                                      GT_FALSE - Metering isn't triggered on specified port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerPortMeteringEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_BOOL                              enable
)
{
    GT_U32  plrRegAddr;    /* the PLR register address */
    GT_U32  plrRegOffset;  /* PLR register offset      */
    GT_U32  pclRegAddr;    /* the PCL register address */
    GT_U32  pclRegOffset;  /* PCL register offset      */
    GT_U32  regValue;      /* register value       */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    CPSS_DXCH_TABLE_ENT tableType; /* table for direct read/write   */

    GT_STATUS rc;       /* return code          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    regValue = (enable == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        tableType = stage + CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E;

        return prvCpssDxChWriteTableEntryField(devNum,tableType,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0,1,regValue);
    }

    /* check parameters and get register addresses and bit offsets */
    rc = prvCpssDxChPolicerPortMeteringEnablerDataGet(devNum, stage, portNum,
                                   &portGroupId, &plrRegAddr, &plrRegOffset,
                                   &pclRegAddr, &pclRegOffset);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* Enable/Disables Metering on this port by updating */
    /* Policer Port Metering Enable register.               */
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, plrRegAddr,
                                            plrRegOffset, 1, regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* configure PCL register if need */
    if (pclRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, pclRegAddr,
                                                pclRegOffset, 1, regValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxCh3PolicerPortMeteringEnableSet function
* @endinternal
*
* @brief   Enables or disables a port metering trigger for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] enable                   - Enable/Disable per-port metering for packets arriving
*                                      on this port:
*                                      GT_TRUE  - Metering is triggered on specified port.
*                                      GT_FALSE - Metering isn't triggered on specified port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPortMeteringEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_BOOL                              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerPortMeteringEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, portNum, enable));

    rc = internal_cpssDxCh3PolicerPortMeteringEnableSet(devNum, stage, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerPortMeteringEnableGet function
* @endinternal
*
* @brief   Gets port status (Enable/Disable) of metering for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] portNum                  - port number (including the CPU port).
*
* @param[out] enablePtr                - pointer on per-port metering status (Enable/Disable)
*                                      for packets arriving on specified port:
*                                      GT_TRUE  - Metering is triggered on specified port.
*                                      GT_FALSE - Metering isn't triggered on specified port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerPortMeteringEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32  plrRegAddr;    /* the PLR register address */
    GT_U32  plrRegOffset;  /* PLR register offset      */
    GT_U32  regValue;      /* register value       */
    GT_STATUS retStatus;   /* return code          */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    CPSS_DXCH_TABLE_ENT tableType; /* table for direct read/write   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        tableType = stage + CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E;

        retStatus = prvCpssDxChReadTableEntryField(devNum,tableType,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0,1,&regValue);

        if(GT_OK == retStatus)
        {
            *enablePtr = (regValue == 1) ? GT_TRUE : GT_FALSE;
        }

        return retStatus;
    }

    /* check parameters and get register addresses and bit offsets */
    retStatus = prvCpssDxChPolicerPortMeteringEnablerDataGet(devNum, stage, portNum,
                                   &portGroupId, &plrRegAddr, &plrRegOffset,
                                   NULL, NULL);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /* Get port metering status for arrived packets (Enable/Disable) */
    /* by reading IPLR Port Metering Enable register.                */
    retStatus = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,plrRegAddr,
                                                   plrRegOffset, 1, &regValue);
    if(GT_OK == retStatus)
    {
        *enablePtr = (regValue == 1) ? GT_TRUE : GT_FALSE;
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerPortMeteringEnableGet function
* @endinternal
*
* @brief   Gets port status (Enable/Disable) of metering for packets
*         arriving on this port.
*         When feature is enabled the meter entry index is a port number.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] portNum                  - port number (including the CPU port).
*
* @param[out] enablePtr                - pointer on per-port metering status (Enable/Disable)
*                                      for packets arriving on specified port:
*                                      GT_TRUE  - Metering is triggered on specified port.
*                                      GT_FALSE - Metering isn't triggered on specified port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerPortMeteringEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerPortMeteringEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, portNum, enablePtr));

    rc = internal_cpssDxCh3PolicerPortMeteringEnableGet(devNum, stage, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMruSet function
* @endinternal
*
* @brief   Sets the Policer Maximum Receive Unit size.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mruSize                  - Policer MRU value in bytes, (APPLICABLE RANGES: 0..0xFFFF).
*                                      This value is used in the metering algorithm.
*                                      When the number of bytes in the bucket is lower than
*                                      this value a packet is marked as non conforming.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_OUT_OF_RANGE          - on mruSize out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerMruSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               mruSize
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  numOfBits;  /* width (number of bits) of the MRU field in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /* all 32 bits supported */
        numOfBits = 32;
    }
    else
    {
        /* Check MRU value */
        if(mruSize > 0xFFFF)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        numOfBits = 16;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerMRU;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerMRUReg;
    }

    /* Set Ingress Policer MRU register */
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, numOfBits, mruSize);
}

/**
* @internal cpssDxCh3PolicerMruSet function
* @endinternal
*
* @brief   Sets the Policer Maximum Receive Unit size.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mruSize                  - Policer MRU value in bytes, (APPLICABLE RANGES: 0..0xFFFF).
*                                      This value is used in the metering algorithm.
*                                      When the number of bytes in the bucket is lower than
*                                      this value a packet is marked as non conforming.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_OUT_OF_RANGE          - on mruSize out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMruSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               mruSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMruSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, mruSize));

    rc = internal_cpssDxCh3PolicerMruSet(devNum, stage, mruSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, mruSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerMruGet function
* @endinternal
*
* @brief   Gets the Policer Maximum Receive Unit size.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] mruSizePtr               - pointer to the Policer MRU value in bytes.
*                                      This value is used in the metering algorithm.
*                                      When the number of bytes in the bucket is lower than
*                                      this value a packet is marked as non conforming.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerMruGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *mruSizePtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32  numOfBits;  /* width (number of bits) of the MRU field in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(mruSizePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /* all 32 bits supported */
        numOfBits = 32;
    }
    else
    {
        numOfBits = 16;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerMRU;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerMRUReg;
    }

    /* Get Ingress Policer MRU value */
    return prvCpssHwPpGetRegField(devNum, regAddr, 0, numOfBits, mruSizePtr);
}

/**
* @internal cpssDxCh3PolicerMruGet function
* @endinternal
*
* @brief   Gets the Policer Maximum Receive Unit size.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] mruSizePtr               - pointer to the Policer MRU value in bytes.
*                                      This value is used in the metering algorithm.
*                                      When the number of bytes in the bucket is lower than
*                                      this value a packet is marked as non conforming.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerMruGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *mruSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMruGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, mruSizePtr));

    rc = internal_cpssDxCh3PolicerMruGet(devNum, stage, mruSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, mruSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerErrorGet function
* @endinternal
*
* @brief   Gets address and type of Policer Entry that had an ECC error.
*         This information available if error is happened and not read till now.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] entryTypePtr             - pointer to the Type of Entry (Metering or Counting)
*                                      that had an error.
* @param[out] entryAddrPtr             - pointer to the Policer Entry that had an error.
*
* @retval GT_OK                    - on success.
* @retval GT_EMPTY                 - on missing error information.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerErrorGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT   *entryTypePtr,
    OUT GT_U32                              *entryAddrPtr
)
{
    return cpssDxChPolicerPortGroupErrorGet(devNum,
                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                             stage,
                                             entryTypePtr,
                                             entryAddrPtr);
}

/**
* @internal cpssDxCh3PolicerErrorGet function
* @endinternal
*
* @brief   Gets address and type of Policer Entry that had an ECC error.
*         This information available if error is happened and not read till now.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] entryTypePtr             - pointer to the Type of Entry (Metering or Counting)
*                                      that had an error.
* @param[out] entryAddrPtr             - pointer to the Policer Entry that had an error.
*
* @retval GT_OK                    - on success.
* @retval GT_EMPTY                 - on missing error information.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerErrorGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT   *entryTypePtr,
    OUT GT_U32                              *entryAddrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerErrorGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, entryTypePtr, entryAddrPtr));

    rc = internal_cpssDxCh3PolicerErrorGet(devNum, stage, entryTypePtr, entryAddrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, entryTypePtr, entryAddrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerErrorCounterGet function
* @endinternal
*
* @brief   Gets the value of the Policer ECC Error Counter.
*         The Error Counter is a free-running non-sticky 8-bit read-only
*         counter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] cntrValuePtr             - pointer to the Policer ECC Error counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerErrorCounterGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *cntrValuePtr
)
{
    return cpssDxChPolicerPortGroupErrorCounterGet(
                                            devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            stage,
                                            cntrValuePtr);
}

/**
* @internal cpssDxCh3PolicerErrorCounterGet function
* @endinternal
*
* @brief   Gets the value of the Policer ECC Error Counter.
*         The Error Counter is a free-running non-sticky 8-bit read-only
*         counter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] cntrValuePtr             - pointer to the Policer ECC Error counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerErrorCounterGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *cntrValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerErrorCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, cntrValuePtr));

    rc = internal_cpssDxCh3PolicerErrorCounterGet(devNum, stage, cntrValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, cntrValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerManagementCountersSet function
* @endinternal
*
* @brief   Sets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
* @param[in] mngCntrPtr               - pointer to the Management Counters Entry must be set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType
*                                       or Management Counters Set number.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerManagementCountersSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT      mngCntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT     mngCntrType,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC    *mngCntrPtr
)
{
    return cpssDxChPolicerPortGroupManagementCountersSet(
                                        devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        stage,
                                        mngCntrSet,
                                        mngCntrType,
                                        mngCntrPtr);
}

/**
* @internal cpssDxCh3PolicerManagementCountersSet function
* @endinternal
*
* @brief   Sets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
* @param[in] mngCntrPtr               - pointer to the Management Counters Entry must be set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType
*                                       or Management Counters Set number.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*
*/
GT_STATUS cpssDxCh3PolicerManagementCountersSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT      mngCntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT     mngCntrType,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC    *mngCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerManagementCountersSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, mngCntrSet, mngCntrType, mngCntrPtr));

    rc = internal_cpssDxCh3PolicerManagementCountersSet(devNum, stage, mngCntrSet, mngCntrType, mngCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, mngCntrSet, mngCntrType, mngCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerManagementCountersGet function
* @endinternal
*
* @brief   Gets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
*
* @param[out] mngCntrPtr               - pointer to the requested Management Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType
*                                       or Management Counters Set number.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerManagementCountersGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCntrSet,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    mngCntrType,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC   *mngCntrPtr
)
{
    return cpssDxChPolicerPortGroupManagementCountersGet(
                                            devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            stage,
                                            mngCntrSet,
                                            mngCntrType,
                                            mngCntrPtr);
}

/**
* @internal cpssDxCh3PolicerManagementCountersGet function
* @endinternal
*
* @brief   Gets the value of specified Management Counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
*
* @param[out] mngCntrPtr               - pointer to the requested Management Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType
*                                       or Management Counters Set number.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*
*/
GT_STATUS cpssDxCh3PolicerManagementCountersGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCntrSet,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    mngCntrType,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC   *mngCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerManagementCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, mngCntrSet, mngCntrType, mngCntrPtr));

    rc = internal_cpssDxCh3PolicerManagementCountersGet(devNum, stage, mngCntrSet, mngCntrType, mngCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, mngCntrSet, mngCntrType, mngCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPolicerMeteringEntrySet function
* @endinternal
*
* @brief   Sets Metering Policer Entry starting from third word
*         for xCat and above.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] entryPtr                 - pointer to the metering policer entry going to be set.
* @param[in] envelopeCfgPtr           - pointer to data fo bucket envelope configuration. Relevant only
*                                      for MEF10.3 entries. Otherwize ignored and can be specified NULL
*
* @param[out] tbParamsPtr              - pointer to actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex or
*                                       entry parameters.
* @retval GT_OUT_OF_RANGE          - on Billing Entry Index out of range.
*
* @note Fills indirect writing data, but does not write control access register.
*       Needs prvCpssDxCh3PolicerInternalTableAccess call after it.
*
*/
static GT_STATUS prvCpssDxChPolicerMeteringEntrySet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT              stage,
    IN  GT_U32                                        entryIndex,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC         *entryPtr,
    IN  PRV_CPSS_DXCH_POLICER_ENVELOPE_MEMBER_CFG_STC *envelopeCfgPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT        *tbParamsPtr
)
{
    GT_U32              hwFormatArray[8]; /* 8 words of entry format */
    GT_U32              hwFormatMeterConfigArray[BOBK_POLICER_METERING_CONFIG_WORDS_CNS];/* 'meter config' entry. sip5_15 only*/
    CPSS_DXCH_TABLE_ENT tableType;
    GT_U32              hwData;         /* data to write to HW */
    GT_U32              regAddr;        /* register address */
    GT_U32              hwRate0;        /* HW rate 0 value */
    GT_U32              hwRate1;        /* HW rate 1 value */
    GT_U32              hwBurstSize0;   /* HW Max Burst Size 0 value */
    GT_U32              hwBurstSize1;   /* HW Max Burst Size 1 value */
    GT_U32              hwRateType0;    /* HW rate type 0 value */
    GT_U32              hwRateType1;    /* HW rate type 1 value */
    GT_U32              hwBucketSize0;  /* HW Bucket Size 0 value */
    GT_U32              hwBucketSize1;  /* HW Bucket Size 1 value */
    GT_U32              redPcktCmdVal;      /* Hw value for red packet command */
    GT_U32              yellowPcktCmdVal;   /* Hw value for yellow packet command */
    GT_U32              meterColorModeVal;  /* Hw value for meter color mode */
    GT_U32              meterModeVal;       /* Hw value for meter mode */
    GT_U32              mngCounterSetVal;   /* Hw value for management counter set */
    GT_U32              modifyDscpVal = 0;  /* Hw value for modify dscp operation */
    GT_U32              modifyUpVal = 0;    /* Hw value for modify up operation */
    GT_U32              modifyDpVal = 0;    /* Hw value for modufy dp */
    GT_U32              remarkModeVal;      /* Hw value for remark mode */
    GT_U32              addOffset;      /* additional offset for field entry */
                                        /* when creating eArch entry */
    GT_STATUS           retStatus;      /* generic return status code */
    GT_U32              envelopeCfg_envelopeSize;  /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_bucket0Rank;   /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_bucket1Rank;   /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_bucket0Color;  /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_bucket1Color;  /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_couplingFlag;  /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_couplingFlag0; /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_maxRateIndex;  /* field from envelopeCfg in GT_U32 form */
    GT_U32              configTblIndex;            /* metering configuration table index */
    GT_U32              shadowTokenBucketTblIndex; /* token bucket table index in shadow */
    GT_U32              redMirrorToAnalyzerEnable;  /* Hw value for meter red mirror to analyzer enabled */
    GT_U32              yellowMirrorToAnalyzerEnable;  /* Hw value for meter yellow mirror to analyzer enabled */
    GT_U32              greenMirrorToAnalyzerEnable;  /* Hw value for meter green mirror to analyzer enabled */

    envelopeCfg_envelopeSize   = 0;
    envelopeCfg_bucket0Rank    = 0;
    envelopeCfg_bucket1Rank    = 0;
    envelopeCfg_bucket0Color   = 0;
    envelopeCfg_bucket1Color   = 1; /* for not MEF10.3 should alwyais be 1 */
    envelopeCfg_couplingFlag   = 0;
    envelopeCfg_couplingFlag0  = 0;
    envelopeCfg_maxRateIndex   = 0;

    switch(entryPtr->meterMode)
    {
        default: /* envelopeCfg already cleared */
            break;
        case  CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E:
        case  CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E:
            CPSS_NULL_PTR_CHECK_MAC(envelopeCfgPtr);
            if (! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            if ((envelopeCfgPtr->envelopeSize < 2)|| (envelopeCfgPtr->envelopeSize > 8))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            if ((envelopeCfgPtr->bucket0Rank < 1) || (envelopeCfgPtr->bucket0Rank > 8))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            if ((envelopeCfgPtr->bucket1Rank < 1) || (envelopeCfgPtr->bucket1Rank > 8))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            envelopeCfg_envelopeSize   = envelopeCfgPtr->envelopeSize - 1;
            envelopeCfg_bucket0Rank    = envelopeCfgPtr->bucket0Rank - 1;
            envelopeCfg_bucket1Rank    = envelopeCfgPtr->bucket1Rank - 1;
            switch (envelopeCfgPtr->bucket0Color)
            {
                case PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E:
                    envelopeCfg_bucket0Color   = 0;
                    break;
                case PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_YELLOW_E:
                    envelopeCfg_bucket0Color   = 1;
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            switch (envelopeCfgPtr->bucket1Color)
            {
                case PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E:
                    envelopeCfg_bucket1Color   = 0;
                    break;
                case PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_YELLOW_E:
                    envelopeCfg_bucket1Color   = 1;
                    break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            envelopeCfg_couplingFlag   = BOOL2BIT_MAC(envelopeCfgPtr->couplingFlag);
            envelopeCfg_couplingFlag0  = BOOL2BIT_MAC(envelopeCfgPtr->couplingFlag0);
            envelopeCfg_maxRateIndex   = envelopeCfgPtr->maxRateIndex;
            break;
    }

    /* Calculate Token Bucket  */
    switch(entryPtr->meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
            retStatus = prvCpssDxCh3PolicerHwTbCalculate(
                           devNum,
                           stage,
                           entryPtr->tokenBucketParams.srTcmParams.cir,
                           entryPtr->tokenBucketParams.srTcmParams.cbs,
                           entryPtr->tokenBucketParams.srTcmParams.ebs,
                           &(tbParamsPtr->srTcmParams.cir), /* Rate0 = Rate1 */
                           &(tbParamsPtr->srTcmParams.cbs), /* MaxBurstSize0 */
                           &(tbParamsPtr->srTcmParams.ebs), /* MaxBurstSize1 */
                           &hwRateType0);      /* RateType0 = RateType1 */
            if (retStatus != GT_OK)
            {
                return retStatus;
            }
            hwRateType1 = hwRateType0;          /* RateType0 = RateType1 */
            hwRate0 = tbParamsPtr->srTcmParams.cir;
            hwRate1 = hwRate0;                              /* Rate0 = Rate1 */
            hwBurstSize0 = tbParamsPtr->srTcmParams.cbs;    /* MaxBurstSize0 */
            hwBurstSize1 = tbParamsPtr->srTcmParams.ebs;    /* MaxBurstSize1 */
            break;

        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
            retStatus = prvCpssDxCh3PolicerHwTbCalculate(
                           devNum,
                           stage,
                           entryPtr->tokenBucketParams.trTcmParams.cir,
                           entryPtr->tokenBucketParams.trTcmParams.cbs,
                           0,                   /* Not exists in the TrTCM */
                           &(tbParamsPtr->trTcmParams.cir), /* Rate0 */
                           &(tbParamsPtr->trTcmParams.cbs), /* MaxBurstSize0 */
                           NULL,                /* Not exists in the TrTCM */
                           &hwRateType0);       /* RateType0 */
            if (retStatus != GT_OK)
            {
                return retStatus;
            }
            hwRate0 = tbParamsPtr->trTcmParams.cir;         /* Rate0 */
            hwBurstSize0 = tbParamsPtr->trTcmParams.cbs;    /* MaxBurstSize0 */

            retStatus = prvCpssDxCh3PolicerHwTbCalculate(
                           devNum,
                           stage,
                           entryPtr->tokenBucketParams.trTcmParams.pir,
                           entryPtr->tokenBucketParams.trTcmParams.pbs,
                           0,                   /* Not exists in the TrTCM */
                           &(tbParamsPtr->trTcmParams.pir), /* Rate1 */
                           &(tbParamsPtr->trTcmParams.pbs), /* MaxBurstSize1 */
                           NULL,                /* Not exists in the TrTCM */
                           &hwRateType1);       /* RateType1 */
            if (retStatus != GT_OK)
            {
                return retStatus;
            }
            hwRate1 = tbParamsPtr->trTcmParams.pir;         /* Rate1 */
            hwBurstSize1 = tbParamsPtr->trTcmParams.pbs;    /* MaxBurstSize1 */

            break;
        case  CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E:
        case  CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E:

            retStatus = prvCpssDxChPolicerHwTbEnvelopeCalculate(
                devNum, stage,
                entryPtr->tokenBucketParams.envelope.cir,
                entryPtr->tokenBucketParams.envelope.cbs,
                entryPtr->tokenBucketParams.envelope.maxCir,
                &(tbParamsPtr->envelope.cir) /*hwRatePtr*/,
                &(tbParamsPtr->envelope.cbs) /*hwBurstSizePtr*/,
                &hwRateType0 /*rateTypePtr*/);
            if (retStatus != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            hwRate0 = tbParamsPtr->envelope.cir;         /* Rate0 */
            hwBurstSize0 = tbParamsPtr->envelope.cbs;    /* MaxBurstSize0 */

            retStatus = prvCpssDxChPolicerHwTbEnvelopeCalculate(
                devNum, stage,
                entryPtr->tokenBucketParams.envelope.eir,
                entryPtr->tokenBucketParams.envelope.ebs,
                entryPtr->tokenBucketParams.envelope.maxEir,
                &(tbParamsPtr->envelope.eir) /*hwRatePtr*/,
                &(tbParamsPtr->envelope.ebs) /*hwBurstSizePtr*/,
                &hwRateType1 /*rateTypePtr*/);
            if (retStatus != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            hwRate1 = tbParamsPtr->envelope.eir;         /* Rate1 */
            hwBurstSize1 = tbParamsPtr->envelope.ebs;    /* MaxBurstSize1 */

            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*************************************************/
    /* Convert actual TB parameters to the SW format */
    /*************************************************/
    retStatus = prvCpssDxCh3PolicerSwTbCalculate(entryPtr->meterMode,
                                                 hwRateType0, hwRateType1,
                                                 tbParamsPtr, tbParamsPtr);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /* get bucket size values in bytes as Maximal Burst size of token bucket */
    switch(entryPtr->meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
                        hwBucketSize0 = tbParamsPtr->srTcmParams.cbs;
                        hwBucketSize1 = tbParamsPtr->srTcmParams.ebs;
                        break;
        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
                        hwBucketSize0 = tbParamsPtr->trTcmParams.cbs;
                        hwBucketSize1 = tbParamsPtr->trTcmParams.pbs;
                        break;
        case  CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E:
        case  CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E:
                        hwBucketSize0 = tbParamsPtr->envelope.cbs;
                        hwBucketSize1 = tbParamsPtr->envelope.ebs;
                        break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* Calculate hw value for Policer Red Packet command  */
    switch(entryPtr->redPcktCmd)
    {
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
            redPcktCmdVal = 0;
            break;
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E:
            redPcktCmdVal = 1;
            break;
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E:
            redPcktCmdVal = 2;
            break;
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E:
            redPcktCmdVal = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Calculate hw value for Policer Yellow Packet command  */
    switch(entryPtr->yellowPcktCmd)
    {
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
            yellowPcktCmdVal = 0;
            break;
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E:
            yellowPcktCmdVal = 1;
            break;
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E:
            yellowPcktCmdVal = 2;
            break;
        case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E:
            yellowPcktCmdVal = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Calculate hw value for Color Mode */
    switch(entryPtr->meterColorMode)
    {
        case CPSS_POLICER_COLOR_BLIND_E:
            meterColorModeVal = 0;
            break;
        case CPSS_POLICER_COLOR_AWARE_E:
            meterColorModeVal = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Calculate hw value for Meter Mode */
    switch(entryPtr->meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
            meterModeVal = 0;
            break;
        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
            meterModeVal = 1;
            break;
        case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
            meterModeVal = 2;
            break;
        case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
            meterModeVal = 3;
            break;
        case  CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E:
            meterModeVal = 4;
            break;
        case  CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E:
            meterModeVal = 5;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Calculate hw value for Management Counter Set */
    switch(entryPtr->mngCounterSet)
    {
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E:
            mngCounterSetVal = 0;
            break;
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E:
            mngCounterSetVal = 1;
            break;
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E:
            mngCounterSetVal = 2;
            break;
        case CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E:
            mngCounterSetVal = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Check index of Policer Billing Entry.
       Maximum index is in the Policy counting mode where each counting entry is
       responsible for 8 indexes. */
    if(entryPtr->countingEntryIndex >= (PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[stage] * 8))
    {
        /* although 16 bits for the field but it is ERROR to point to
           'out of range' counter */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    policerTableAccessData[0];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].policerTblAccessDataReg;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        cpssOsMemSet(hwFormatArray,0,sizeof(hwFormatArray));
        cpssOsMemSet(hwFormatMeterConfigArray,0,sizeof(hwFormatMeterConfigArray));

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            /* Set 'Bucket 1 color' to be 'yellow' , this will set ''Bucket 1' as
               'EBS' .

               NOTE: when MEF 10.3 will be supported this value should not be
                    'HARD CODED' to 1
            */
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_COLOR_E,
                1);
        }

        /* set Bucket size 0 */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE0_E,
            hwBucketSize0);

        /* set Bucket size 1 */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE1_E,
            hwBucketSize1);

        /* Set RateType0 value */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_RATE_TYPE0_E,
            hwRateType0);

        /* Set RateType1 value */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_RATE_TYPE1_E,
            hwRateType1);

        /* Set Rate 0 (CIR) */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_RATE0_E,
            hwRate0);
        /* Set Rate 1 (CIR) */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_RATE1_E,
            hwRate1);

        /* Set Maximum burst size for bucket 0 */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE0_E,
            hwBurstSize0);
        /* Set Maximum burst size for bucket 1 */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE1_E,
            hwBurstSize1);

        /* Set Color Mode */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_COLOR_MODE_E,
            meterColorModeVal);

        /* Set Meter Mode */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_POLICER_MODE_E,
            meterModeVal);

        /* Set Management Counter Set */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_MG_COUNTERS_SET_EN_E,
            mngCounterSetVal);
        /* Set pointer to related counting entry */
        SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_BILLING_PTR_E,
            entryPtr->countingEntryIndex);


        switch(entryPtr->byteOrPacketCountingMode)
        {
            case CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E:
                break;
            case CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E:
                /* Set byte or packet metering mode */
                SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                    SIP5_PLR_METERING_TABLE_FIELDS_BYTE_OR_PACKET_COUNTING_MODE_E,
                    1);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


        /* Set packet size mode & include layer1 overhead */
        switch(entryPtr->packetSizeMode)
        {
            case CPSS_POLICER_PACKET_SIZE_L3_ONLY_E:
            case CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E:
            case CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(entryPtr->packetSizeMode == CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E ||
           entryPtr->packetSizeMode == CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E )
        {
            /* L2 counting */
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_PACKET_SIZE_MODE_E,
                1);
        }

        if(entryPtr->packetSizeMode == CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E)
        {
            /* + L1 overhead */
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_INCLUDE_LAYER1_OVERHEAD_E,
                1);
        }

        /* Set TT/TS packet size mode */
        switch(entryPtr->tunnelTerminationPacketSizeMode)
        {
            case CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E:
                break;
            case CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E:
                SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                    SIP5_PLR_METERING_TABLE_FIELDS_TUNNEL_TERMINATION_PACKET_SIZE_MODE_E,
                    1);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Set DSA Tag metering mode */
        switch(entryPtr->dsaTagCountingMode)
        {
            case CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E:
                break;
            case CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E:
                SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                    SIP5_PLR_METERING_TABLE_FIELDS_DSA_TAG_COUNTING_MODE_E,
                    1);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Set Timestamp Tag metering mode */
        switch(entryPtr->timeStampCountingMode)
        {
            case CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E:
                break;
            case CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E:
                SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                    SIP5_PLR_METERING_TABLE_FIELDS_TIMESTAMP_TAG_COUNTING_MODE_E,
                    1);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            if (entryPtr->redMirrorToAnalyzerEnable == GT_TRUE)
            {
                redMirrorToAnalyzerEnable = 1;
            }
            else
            {
                redMirrorToAnalyzerEnable = 0;
            }
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_RED_MIRROR_EN___EMULATION_ONLY_E,
                redMirrorToAnalyzerEnable);
            if (entryPtr->yellowMirrorToAnalyzerEnable == GT_TRUE)
            {
                yellowMirrorToAnalyzerEnable = 1;
            }
            else
            {
                yellowMirrorToAnalyzerEnable = 0;
            }
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_YELLOW_MIRROR_EN___EMULATION_ONLY_E,
                yellowMirrorToAnalyzerEnable);
            if (entryPtr->greenMirrorToAnalyzerEnable == GT_TRUE)
            {
                greenMirrorToAnalyzerEnable = 1;
            }
            else
            {
                greenMirrorToAnalyzerEnable = 0;
            }
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_GREEN_MIRROR_EN___EMULATION_ONLY_E,
                greenMirrorToAnalyzerEnable);
        }

        if (PRV_CPSS_DXCH_POLICER_INGRESS_STAGE_CHECK_MAC(stage))
        { /* IPLR */

            /* Calculate hw value for Modify DSCP operation */
            switch(entryPtr->modifyDscp)
            {
                case CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E:
                    modifyDscpVal = 0;
                    break;
                case CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E:
                    modifyDscpVal = 1;
                    break;
                case CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E:
                    modifyDscpVal = 2;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_MODIFY_DSCP_E,
                modifyDscpVal);

            /* Calculate hw value for Modify UP operation */
            switch(entryPtr->modifyUp)
            {
                case CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E:
                    modifyUpVal = 0;
                    break;
                case CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E:
                    modifyUpVal = 1;
                    break;
                case CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E:
                    modifyUpVal = 2;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_MODIFY_UP_E,
                modifyUpVal);


            /* Set Policer commands for Red Packet */
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_RED_CMD_E,
                redPcktCmdVal);
            /* Set Policer commands hw value for Yellow Packet */
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_YELLOW_CMD_E,
                yellowPcktCmdVal);

            switch(entryPtr->greenPcktCmd)
            {
                case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
                    hwData = 0;
                    break;
                case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E:
                    hwData = 1;
                    break;
                case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E:
                    hwData = 2;
                    break;
                case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E:
                    hwData = 3;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* Set Policer commands hw value for Green Packet */
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_GREEN_CMD_E,
                hwData);

            /* Set Qos Profile */
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_QOS_PROFILE_E,
                entryPtr->qosProfile);
        }
        else    /*EPLR*/
        {

            /* Calculate hw value for remark mode */
            switch (entryPtr->remarkMode)
            {
                case CPSS_DXCH_POLICER_REMARK_MODE_L2_E:
                    remarkModeVal = 0;
                    break;
                case CPSS_DXCH_POLICER_REMARK_MODE_L3_E:
                    remarkModeVal = 1;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_REMARK_MODE_E,
                remarkModeVal);

            /* Calculate hw value for non conforming packet command */
            switch(entryPtr->redPcktCmd)
            {
                case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
                    redPcktCmdVal = 0;
                    break;
                case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E:
                    redPcktCmdVal = 1;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_DROP_RED_E,
                redPcktCmdVal);

            /* Calculate hw value for Modify DP operation */
            switch(entryPtr->modifyDp)
            {
                case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                    modifyDpVal = 0;
                    break;
                case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                    modifyDpVal = 1;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_DP_E,
                modifyDpVal);

            /* Calculate hw value for Modify DSCP operation */
            switch(entryPtr->modifyDscp)
            {
                case  CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E:
                    modifyDscpVal = 0;
                    break;
                case CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E:
                    modifyDscpVal = 1;
                    break;
                case CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_INNER_E:
                    modifyDscpVal = 2;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_DSCP_E,
                modifyDscpVal);

            /* Calculate hw value for Modify UP operation */
            switch(entryPtr->modifyUp)
            {
                case CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E:
                    modifyUpVal = 0;
                    break;
                case CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E:
                    modifyUpVal = 1;
                    break;
                case CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_TAG0_E:
                    modifyUpVal = 2;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_UP_E,
                modifyUpVal);

            /* Set Modify EXP operation */
            switch(entryPtr->modifyExp)
            {
                case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                    break;
                case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                    SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                        SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_EXP_E,
                        1);
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* Set Modify TC operation */
            switch(entryPtr->modifyTc)
            {
                case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                    break;
                case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                    SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                        SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_TC_E,
                        1);
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* Set enable ECN marking */
            if(entryPtr->yellowEcnMarkingEnable)
            {
                SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                    SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_YELLOW_ECN_MARKING_E,
                    1);
            }
        }

        if ((entryPtr->meterMode == CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E)
           || (entryPtr->meterMode == CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E))
        {
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_MEF_10_3_ENV_SIZE_E___EMULATION_ONLY_E,
                envelopeCfg_envelopeSize);
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_0_PRIORITY_E,
                envelopeCfg_bucket0Rank);
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_PRIORITY_E,
                envelopeCfg_bucket1Rank);
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_0_COLOR_E,
                envelopeCfg_bucket0Color);
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_COLOR_E,
                envelopeCfg_bucket1Color);
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_COUPLING_FLAG_E,
                envelopeCfg_couplingFlag);
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_COUPLING_FLAG_0_E,
                envelopeCfg_couplingFlag0);
            SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_EIR_AND_CIR_MAX_INDEX_E,
                envelopeCfg_maxRateIndex);
        }

        shadowTokenBucketTblIndex = entryIndex;
        if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
        {
            tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
        }
        else if (CPSS_DXCH_POLICER_STAGE_INGRESS_1_E == stage )
        {
            tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E;

            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                /* direct access to Metering Token Bucket table is done by IPLR0 */
                tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
                shadowTokenBucketTblIndex += PLR0_SIZE_MAC(devNum);
            }
        }
        else /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
        {
            tableType = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E;
        }

        /* update HW table shadow (if supported and requested) */
        retStatus = prvCpssDxChPortGroupShadowEntryWrite(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  tableType,
                                                  shadowTokenBucketTblIndex, hwFormatArray);
        if (retStatus != GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(retStatus, "update hw table shadow failed");
        }

        /* write the 'meter' entry into the indirect data registers*/
        retStatus = prvCpssHwPpWriteRam(devNum, regAddr, 8 , hwFormatArray);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            configTblIndex = entryIndex;

            if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
            {
                tableType = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E;
            }
            else if (CPSS_DXCH_POLICER_STAGE_INGRESS_1_E == stage )
            {
                if (IPLR1_IS_SHARED_MAC(devNum))
                {
                    PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                    /* access to Metering Configuration table is done by IPLR0 */
                    tableType = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E;
                }
                else
                {
                    /* in sip 6 each PLR unit has it's own instance of the metering configuration table
                       the entry poiner is still the same as in Metering Token Bucket table*/
                    tableType = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E;
                }
                if (IPLR1_ADD_BASE_ADDR_MAC(devNum))
                {
                    configTblIndex += PLR0_SIZE_MAC(devNum);
                }
            }
            else /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
            {
                 if (EPLR_IS_SHARED_MAC(devNum))
                 {
                    PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                    /* access to Metering Configuration table is done by IPLR0 */
                    tableType = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E;
                 }
                 else
                 {
                     tableType = CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E;
                 }
                 if (EPLR_ADD_BASE_ADDR_MAC(devNum))
                 {
                     configTblIndex += PLR0_SIZE_MAC(devNum) + PLR1_SIZE_MAC(devNum);
                 }
            }

            /* write the 'meter config' entry using the generic table write mechanism */
            retStatus =  prvCpssDxChWriteTableEntry(
                            devNum,
                            tableType,
                            configTblIndex,
                            &hwFormatMeterConfigArray[0]);
            if (retStatus != GT_OK)
            {
                return retStatus;
            }
        }

        return GT_OK;
    }

    /**********/
    /* Word 1 */
    /**********/
    /* set Bucket size 0 - LSB 6 bits to bits 26:31 */

    regAddr += 0x4;

    /* Set Ingress Policer Table Access Data 1 Register */
    retStatus = prvCpssHwPpSetRegField(devNum, regAddr, 26, 6,
                                          (hwBucketSize0 & 0x3f));
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /**********/
    /* Word 2 */
    /**********/
    /* set Bucket size 0 - MSB 26 bits to bits 0:25,
       set Bucket size 1 - LSB 6 bits to bits 26:31 */
    hwData = (hwBucketSize0 >> 6) | ((hwBucketSize1 & 0x3f) << 26);

    regAddr += 0x4;

    /* Set Ingress Policer Table Access Data 1 Register */
    retStatus = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /**********/
    /* Word 3 */
    /**********/
    /* set Bucket size 1 - MSB 26 bits to bits 0:25 */
    hwData = (hwBucketSize1 >> 6);

    /* Set RateType0 value */
    hwData |= (hwRateType0 << 26);
    /* Set RateType1 value */
    hwData |= (hwRateType1 << 29);

    regAddr += 0x4;

    /* Set Policer Table Access Data 3 Register */
    retStatus = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /**********/
    /* Word 4 */
    /**********/

    /* Set Rate 0 (CIR) */
    /* bits 0:16 */
    hwData = (hwRate0 & 0x1FFFF);
    /* Set 15 LSB of Rate 1 (CIR) */
    /* bits 17:31 */
    hwData |= ((hwRate1 & 0x7FFF) << 17);

    regAddr += 0x4;

    /* Set Policer Table Access Data 4 Register */
    retStatus = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /**********/
    /* Word 5 */
    /**********/

    /* Set 2 MSB of Rate 1 (CIR) */
    /* bits 0:1 */
    hwData = ((hwRate1 >> 15) & 0x3);
    /* Set Maximum burst size for bucket 0 */
    /* bits 2:17 */
    hwData |= (hwBurstSize0 << 2);
    /* Set 14 LSB of Maximum burst size for bucket 1 */
    /* bits 18:31 */
    hwData |= ((hwBurstSize1 & 0x3FFF) << 18);

    regAddr += 0x4;

    /* Set Policer Table Access Data 5 Register */
    retStatus = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /**********/
    /* Word 6 */
    /**********/

    /* Set 2 MSB of Maximum burst size for bucket 1 */
    /* bits 0:1 */
    hwData = ((hwBurstSize1 >> 14) & 0x3);
    /* Set Color Mode */
    /* bit 2 */
    hwData |= (meterColorModeVal << 2);
    /* Set Meter Mode */
    /* bit 3 or bits 3:4 for eArch */
    hwData |= (meterModeVal << 3);

    addOffset = 0;

    /* Set Management Counter Set */
    /* bits 4:5 or bits 5:6 for eArch*/
    hwData |= (mngCounterSetVal << (4+addOffset));
    /* Set pointer to related counting entry */
    /* bits 6:21 or bits 7:22 for eArch */
    hwData |= (entryPtr->countingEntryIndex << (6+addOffset));

    /* check Policer stage: Ingress or Egress */
    if (PRV_CPSS_DXCH_POLICER_INGRESS_STAGE_CHECK_MAC(stage))
    { /* IPLR */

        /* check Qos Profile range */
        if (entryPtr->qosProfile >= BIT_7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* Calculate hw value for Modify DSCP operation */
        switch(entryPtr->modifyDscp)
        {
            case CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E:
                modifyDscpVal = 0;
                break;
            case CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E:
                modifyDscpVal = 1;
                break;
            case CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E:
                modifyDscpVal = 2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Calculate hw value for Modify UP operation */
        switch(entryPtr->modifyUp)
        {
            case CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E:
                modifyUpVal = 0;
                break;
            case CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E:
                modifyUpVal = 1;
                break;
            case CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E:
                modifyUpVal = 2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Set Policer commands for Red Packet */
        /* bits 22:23 */
        hwData |= (redPcktCmdVal << 22);
        /* Set Policer commands for Yellow Packet */
        /* bits 24:25 */
        hwData |= (yellowPcktCmdVal << 24);
        /* Set Modify DSCP operation */
        /* bits 26:27 */
        hwData |= (modifyDscpVal << 26);
        /* Set Modify UP operation */
        /* bits 28:29 */
        hwData |= (modifyUpVal << 28);
        /* Set 2 LSB of Qos Profile */
        /* bits 30:31 */
        hwData |= ((entryPtr->qosProfile & 0x3) << 30);
    }
    else
    { /* EPLR */

        /* Calculate hw value for remark mode */
        switch (entryPtr->remarkMode)
        {
            case CPSS_DXCH_POLICER_REMARK_MODE_L2_E:
                remarkModeVal = 0;
                break;
            case CPSS_DXCH_POLICER_REMARK_MODE_L3_E:
                remarkModeVal = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Calculate hw value for non conforming packet command */
        switch(entryPtr->redPcktCmd)
        {
            case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
                redPcktCmdVal = 0;
                break;
            case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E:
                redPcktCmdVal = 1;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Calculate hw value for Modify DP operation */
        switch(entryPtr->modifyDp)
        {
            case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                modifyDpVal = 0;
                break;
            case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                modifyDpVal = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Calculate hw value for Modify DSCP operation */
        switch(entryPtr->modifyDscp)
        {
            case  CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E:
                modifyDscpVal = 0;
                break;
            case CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E:
                modifyDscpVal = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Calculate hw value for Modify UP operation */
        switch(entryPtr->modifyUp)
        {
            case CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E:
                modifyUpVal = 0;
                break;
            case CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E:
                modifyUpVal = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Set Remark mode */
        /* bit 22 */
        hwData |= (remarkModeVal << 22);

        /* Set MX Non Conforming Packet Command */
        /* bit 23 */
        hwData |= (redPcktCmdVal << 23);

        /* Set Modify DSCP operation */
        /* bit 25 */
        hwData |= (modifyDscpVal << 25);

        /* Set Modify UP operation */
        /* bit 27 */
        hwData |= (modifyUpVal << 27);

        /* Set Modify DP operation */
        /* bit 28 */
        hwData |= (modifyDpVal << 28);
    }

    regAddr += 0x4;

    /* Set Policer Table Access Data 6 Register */
    retStatus = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /**********/
    /* Word 7 */
    /**********/

    regAddr += 0x4;

    /* for xCat and above Ingress Policers */
    if (PRV_CPSS_DXCH_POLICER_INGRESS_STAGE_CHECK_MAC(stage))
    {
        /* Set 5 MSB of Qos Profile */
        /* bits 0:4 */
        hwData = ((entryPtr->qosProfile >> 2) & 0x1F);

        /* Set Ingress Policer Table Access Data 7 Register */
        retStatus = prvCpssHwPpSetRegField(devNum, regAddr, 0, 5, hwData);
    }

    return retStatus;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringEntrySet function
* @endinternal
*
* @brief   Sets Metering Policer Entry.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] entryPtr                 - pointer to the metering policer entry going to be set.
*
* @param[out] tbParamsPtr              - pointer to actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex or
*                                       entry parameters.
* @retval GT_OUT_OF_RANGE          - on Billing Entry Index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Should not be used to update member of MEF 10.3 Envelope.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringEntrySet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbParamsPtr
)
{
    return cpssDxChPolicerPortGroupMeteringEntrySet(
                                            devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            stage,
                                            entryIndex,
                                            entryPtr,
                                            tbParamsPtr);
}

/**
* @internal cpssDxCh3PolicerMeteringEntrySet function
* @endinternal
*
* @brief   Sets Metering Policer Entry.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] entryPtr                 - pointer to the metering policer entry going to be set.
*
* @param[out] tbParamsPtr              - pointer to actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex or
*                                       entry parameters.
* @retval GT_OUT_OF_RANGE          - on Billing Entry Index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Should not be used to update member of MEF 10.3 Envelope.
*
*/
GT_STATUS cpssDxCh3PolicerMeteringEntrySet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, entryIndex, entryPtr, tbParamsPtr));

    rc = internal_cpssDxCh3PolicerMeteringEntrySet(devNum, stage, entryIndex, entryPtr, tbParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, entryIndex, entryPtr, tbParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal sip5PolicerMeteringEntryGet function
* @endinternal
*
* @brief   Gets Metering Policer Entry for eArch devices.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3 ;xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] hwFormatArray[]          - pointer to 'meter' entry (in 5_15 'token bucket' entry)
* @param[in] hwFormatMeterConfigArray[] - pointer to 'meter config' entry (relevant only to 5_15)
*
* @param[out] entryPtr                 - pointer to the SW format of the metering policer entry.
* @param[out] envelopeCfgPtr           - pointer to data fo bucket envelope configuration. Relevant only
*                                      for MEF10.3 entries. Otherwize ignored.
*                                      Any case can be specified NULL,
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
static GT_STATUS sip5PolicerMeteringEntryGet
(
    IN   GT_U8                                          devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT               stage,
    OUT  CPSS_DXCH3_POLICER_METERING_ENTRY_STC          *entryPtr,
    OUT  PRV_CPSS_DXCH_POLICER_ENVELOPE_MEMBER_CFG_STC  *envelopeCfgPtr,
    IN   GT_U32                                         hwFormatArray[],
    IN   GT_U32                                         hwFormatMeterConfigArray[]
)
{
    GT_STATUS           rc;      /* generic return status code */
    GT_U32              hwValue;
    GT_U32              hwRateType0;    /* HW rate type 0 value */
    GT_U32              hwRateType1;    /* HW rate type 1 value */
    GT_U32              hwRate0;        /* HW rate 0 value */
    GT_U32              hwRate1;        /* HW rate 1 value */
    GT_U32              hwBurstSize0;   /* HW Max Burst Size 0 value */
    GT_U32              hwBurstSize1;   /* HW Max Burst Size 1 value */
    GT_U32              redPcktCmdVal;      /* Hw value for red packet command */
    GT_U32              yellowPcktCmdVal;   /* Hw value for yellow packet command */
    GT_U32              meterColorModeVal;  /* Hw value for meter color mode */
    GT_U32              meterModeVal;       /* Hw value for meter mode */
    GT_U32              mngCounterSetVal;   /* Hw value for management counter set */
    GT_U32              modifyDscpVal;      /* Hw value for modify dscp */
    GT_U32              modifyUpVal;        /* Hw value for modify up */
    GT_U32              modifyDpVal;        /* Hw value for modufy dp */
    GT_U32              remarkModeVal;      /* Hw value for remark mode */
    GT_U32              envelopeCfg_envelopeSize;  /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_bucket0Rank;   /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_bucket1Rank;   /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_bucket0Color;  /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_bucket1Color;  /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_couplingFlag;  /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_couplingFlag0; /* field from envelopeCfg in GT_U32 form */
    GT_U32              envelopeCfg_maxRateIndex;  /* field from envelopeCfg in GT_U32 form */

    /* get RateType0 value */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_RATE_TYPE0_E,
        hwRateType0);

    /* get RateType1 value */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_RATE_TYPE1_E,
        hwRateType1);

    /* get Rate 0 (CIR) */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_RATE0_E,
        hwRate0);
    /* get Rate 1 (CIR) */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_RATE1_E,
        hwRate1);

    /* get Maximum burst size for bucket 0 */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE0_E,
        hwBurstSize0);
    /* get Maximum burst size for bucket 1 */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE1_E,
        hwBurstSize1);

    /* get Color Mode */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_COLOR_MODE_E,
        meterColorModeVal);

    /* Get Color Mode */
    entryPtr->meterColorMode = (meterColorModeVal) ?
                                            CPSS_POLICER_COLOR_AWARE_E :
                                            CPSS_POLICER_COLOR_BLIND_E;

    /* get Meter Mode */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_POLICER_MODE_E,
        meterModeVal);

    /* Get Meter Mode */
    switch(meterModeVal)
    {
        case 0:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E;
            break;
        case 1:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E;
            break;
        case 2:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_MEF0_E;
            break;
        case 3:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_MEF1_E;
            break;
        case 4:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E;
            break;
        case 5:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG); /* never should be reached*/
    }

    /* get Management Counter Set */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_MG_COUNTERS_SET_EN_E,
        mngCounterSetVal);

    /* Get Management Counter Set */
    switch(mngCounterSetVal)
    {
        case 0:
            entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E;
            break;
        case 1:
            entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E;
            break;
        case 2:
            entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E;
            break;
        case 3:
            entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG); /* never should be reached*/
    }

    /* get pointer to related counting entry */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_BILLING_PTR_E,
        entryPtr->countingEntryIndex);

    /* get byte or packet metering mode */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_BYTE_OR_PACKET_COUNTING_MODE_E,
        hwValue);
    entryPtr->byteOrPacketCountingMode =
         hwValue == 0 ?
            CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E :
            CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E;

    /* L2 counting */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_PACKET_SIZE_MODE_E,
        hwValue);

    if(hwValue)
    {
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_INCLUDE_LAYER1_OVERHEAD_E,
            hwValue);

        entryPtr->packetSizeMode = (hwValue ?
            CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E :
            CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
    }
    else
    {
        entryPtr->packetSizeMode = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;
    }


    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_TUNNEL_TERMINATION_PACKET_SIZE_MODE_E,
        hwValue);

    /* get TT/TS packet size mode */
    entryPtr->tunnelTerminationPacketSizeMode = hwValue ?
        CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E :
        CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E;

    /* get DSA Tag metering mode */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_DSA_TAG_COUNTING_MODE_E,
        hwValue);
    entryPtr->dsaTagCountingMode = hwValue ?
        CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E :
        CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E;

    /* get Timestamp Tag metering mode */
    SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
        SIP5_PLR_METERING_TABLE_FIELDS_TIMESTAMP_TAG_COUNTING_MODE_E,
        hwValue);
    entryPtr->timeStampCountingMode = hwValue ?
        CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E :
        CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* get Red mirror to analyzer enabled */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_RED_MIRROR_EN___EMULATION_ONLY_E,
            hwValue);
        entryPtr->redMirrorToAnalyzerEnable = hwValue ?
            GT_TRUE :
            GT_FALSE;

        /* get Yellow mirror to analyzer enabled */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_YELLOW_MIRROR_EN___EMULATION_ONLY_E,
            hwValue);
        entryPtr->yellowMirrorToAnalyzerEnable = hwValue ?
            GT_TRUE :
            GT_FALSE;

        /* get Green mirror to analyzer enabled */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_GREEN_MIRROR_EN___EMULATION_ONLY_E,
            hwValue);
        entryPtr->greenMirrorToAnalyzerEnable = hwValue ?
            GT_TRUE :
            GT_FALSE;
    }

    if (PRV_CPSS_DXCH_POLICER_INGRESS_STAGE_CHECK_MAC(stage))
    { /* IPLR */

        /* Calculate hw value for Modify DSCP operation */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_MODIFY_DSCP_E,
            modifyDscpVal);

        switch(modifyDscpVal)
        {
            case 0:
                entryPtr->modifyDscp = CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E;
                break;
            case 1:
                entryPtr->modifyDscp = CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E;
                break;
            case 2:
                entryPtr->modifyDscp = CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* Calculate hw value for Modify UP operation */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_MODIFY_UP_E,
            modifyUpVal);

        switch(modifyUpVal)
        {
            case 0:
                entryPtr->modifyUp = CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E;
                break;
            case 1:
                entryPtr->modifyUp = CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E;
                break;
            case 2:
                entryPtr->modifyUp = CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* get Policer commands for Red Packet */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_RED_CMD_E,
            redPcktCmdVal);
        GET_SW_PACKET_CMD_FROM_HW_MAC(redPcktCmdVal,entryPtr->redPcktCmd);

        /* get Policer commands hw value for Yellow Packet */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_YELLOW_CMD_E,
            yellowPcktCmdVal);
        GET_SW_PACKET_CMD_FROM_HW_MAC(yellowPcktCmdVal,entryPtr->yellowPcktCmd);

        /* get Policer commands hw value for Green Packet */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_GREEN_CMD_E,
            hwValue);
        GET_SW_PACKET_CMD_FROM_HW_MAC(hwValue,entryPtr->greenPcktCmd);


        /* get Qos Profile */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_QOS_PROFILE_E,
            entryPtr->qosProfile);

    }
    else    /*EPLR*/
    {

        /* Calculate hw value for remark mode */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_REMARK_MODE_E,
            remarkModeVal);

        entryPtr->remarkMode = remarkModeVal ?
            CPSS_DXCH_POLICER_REMARK_MODE_L3_E :
            CPSS_DXCH_POLICER_REMARK_MODE_L2_E;

        /* Calculate hw value for non conforming packet command */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_DROP_RED_E,
            redPcktCmdVal);

        entryPtr->redPcktCmd = redPcktCmdVal ?
            CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E :
            CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;

        /* Calculate hw value for Modify DP operation */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_DP_E,
            modifyDpVal);
        entryPtr->modifyDp = modifyDpVal ?
            CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E :
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

        /* Calculate hw value for Modify UP operation */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_DSCP_E,
            modifyDscpVal);
        switch(modifyDscpVal)
        {
            case 0:
                entryPtr->modifyDscp = CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E;
                break;
            case 1:
                entryPtr->modifyDscp = CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E;
                break;
            case 2:
                entryPtr->modifyDscp = CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_INNER_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* Calculate hw value for Modify UP operation */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_UP_E,
            modifyUpVal);
        switch(modifyUpVal)
        {
            case 0:
                entryPtr->modifyUp = CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E;
                break;
            case 1:
                entryPtr->modifyUp = CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E;
                break;
            case 2:
                entryPtr->modifyUp = CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_TAG0_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* get Modify EXP operation */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_EXP_E,
            hwValue);
        entryPtr->modifyExp = hwValue ?
            CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;


        /* get Modify TC operation */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_TC_E,
            hwValue);
        entryPtr->modifyTc = hwValue ?
            CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E :
            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

        /* get enable ECN marking */
        SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
            SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_YELLOW_ECN_MARKING_E,
            hwValue);
        entryPtr->yellowEcnMarkingEnable = hwValue ? GT_TRUE : GT_FALSE;
    }


    /****************************************************/
    /* Convert Token Bucket Parameters in the SW format */
    /****************************************************/
    switch(entryPtr->meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
            entryPtr->tokenBucketParams.srTcmParams.cir = hwRate0;
            entryPtr->tokenBucketParams.srTcmParams.cbs = hwBurstSize0;
            entryPtr->tokenBucketParams.srTcmParams.ebs = hwBurstSize1;

            rc = prvCpssDxCh3PolicerSwTbCalculate(
                entryPtr->meterMode,
                hwRateType0,
                0,
                &(entryPtr->tokenBucketParams),
                &(entryPtr->tokenBucketParams));

            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
            entryPtr->tokenBucketParams.trTcmParams.cir = hwRate0;
            entryPtr->tokenBucketParams.trTcmParams.pir = hwRate1;
            entryPtr->tokenBucketParams.trTcmParams.cbs = hwBurstSize0;
            entryPtr->tokenBucketParams.trTcmParams.pbs = hwBurstSize1;

            rc = prvCpssDxCh3PolicerSwTbCalculate(
                entryPtr->meterMode,
                hwRateType0,
                hwRateType1,
                &(entryPtr->tokenBucketParams),
                &(entryPtr->tokenBucketParams));

            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case  CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E:
        case  CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E:
            entryPtr->tokenBucketParams.envelope.cir = hwRate0;
            entryPtr->tokenBucketParams.envelope.eir = hwRate1;
            entryPtr->tokenBucketParams.envelope.cbs = hwBurstSize0;
            entryPtr->tokenBucketParams.envelope.ebs = hwBurstSize1;
            /* maximal possible value of hwRate, maxRates used for scaling only */
            entryPtr->tokenBucketParams.envelope.maxCir = (BIT_17 - 1);
            entryPtr->tokenBucketParams.envelope.maxEir = (BIT_17 - 1);

            rc = prvCpssDxCh3PolicerSwTbCalculate(
                entryPtr->meterMode,
                hwRateType0,
                hwRateType1,
                &(entryPtr->tokenBucketParams),
                &(entryPtr->tokenBucketParams));
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG); /* never should be reached*/
    }

    switch(entryPtr->meterMode)
    {
        default: /* envelopeCfg already cleared */
            break;
        case  CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E:
        case  CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E:
            if (! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            if (envelopeCfgPtr == NULL)
            {
                break;
            }
            SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_MEF_10_3_ENV_SIZE_E___EMULATION_ONLY_E,
                envelopeCfg_envelopeSize);
            SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_0_PRIORITY_E,
                envelopeCfg_bucket0Rank);
            SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_PRIORITY_E,
                envelopeCfg_bucket1Rank);
            SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_0_COLOR_E,
                envelopeCfg_bucket0Color);
            SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_COLOR_E,
                envelopeCfg_bucket1Color);
            SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_COUPLING_FLAG_E,
                envelopeCfg_couplingFlag);
            SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_COUPLING_FLAG_0_E,
                envelopeCfg_couplingFlag0);
            SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwFormatArray,hwFormatMeterConfigArray,
                SIP5_15_PLR_METERING_TABLE_FIELDS_EIR_AND_CIR_MAX_INDEX_E,
                envelopeCfg_maxRateIndex);
            /* convert to SW form */
            envelopeCfgPtr->envelopeSize   = envelopeCfg_envelopeSize + 1;
            envelopeCfgPtr->bucket0Rank    = envelopeCfg_bucket0Rank + 1;
            envelopeCfgPtr->bucket1Rank    = envelopeCfg_bucket1Rank + 1;
            envelopeCfgPtr->bucket0Color   =
                (envelopeCfg_bucket0Color == 0)
                    ? PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E
                    : PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_YELLOW_E;
            envelopeCfgPtr->bucket1Color   =
                (envelopeCfg_bucket1Color == 0)
                    ? PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E
                    : PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_YELLOW_E;
            envelopeCfgPtr->couplingFlag   = BIT2BOOL_MAC(envelopeCfg_couplingFlag);
            envelopeCfgPtr->couplingFlag0  = BIT2BOOL_MAC(envelopeCfg_couplingFlag0);
            envelopeCfgPtr->maxRateIndex   = envelopeCfg_maxRateIndex;
            break;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPolicerMeteringEntryGet function
* @endinternal
*
* @brief   Gets Metering Policer Entry for xCat and above.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] portGroupId              - the portGroupId. to support multi-port-groups device
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @param[out] entryPtr                 - pointer to the requested metering policer entry.
* @param[out] envelopeCfgPtr           - pointer to data fo bucket envelope configuration. Relevant only
*                                      for MEF10.3 entries. Otherwize ignored and can be specified NULL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
static GT_STATUS prvCpssDxChPolicerMeteringEntryGet
(
    IN   GT_U8                                          devNum,
    IN   GT_U32                                         portGroupId,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT               stage,
    IN   GT_U32                                         entryIndex,
    OUT  CPSS_DXCH3_POLICER_METERING_ENTRY_STC          *entryPtr,
    OUT  PRV_CPSS_DXCH_POLICER_ENVELOPE_MEMBER_CFG_STC  *envelopeCfgPtr
)
{
    GT_U32              regAddr;        /* register address */
    GT_U32              *hwData;        /* pointer to data read from HW for parsing */
    GT_U32              hwDataArr[8];   /* data read from HW */
    GT_U32              hwFormatMeterConfigArray[BOBK_POLICER_METERING_CONFIG_WORDS_CNS];/* 'meter config' entry. sip5_15 only*/
    GT_U32              hwRateType0;    /* HW rate type 0 value */
    GT_U32              hwRateType1;    /* HW rate type 1 value */
    GT_U32              hwRate0;        /* HW rate 0 value */
    GT_U32              hwRate1;        /* HW rate 1 value */
    GT_U32              hwBurstSize0;   /* HW Max Burst Size 0 value */
    GT_U32              hwBurstSize1;   /* HW Max Burst Size 1 value */
    GT_U32              redPcktCmdVal;      /* Hw value for red packet command */
    GT_U32              yellowPcktCmdVal;   /* Hw value for yellow packet command */
    GT_U32              meterColorModeVal;  /* Hw value for meter color mode */
    GT_U32              meterModeVal;       /* Hw value for meter mode */
    GT_U32              mngCounterSetVal;   /* Hw value for management counter set */
    GT_U32              modifyDscpVal;      /* Hw value for modify dscp */
    GT_U32              modifyUpVal;        /* Hw value for modify up */
    GT_U32              modifyDpVal;        /* Hw value for modufy dp */
    GT_U32              remarkModeVal;      /* Hw value for remark mode */
    CPSS_DXCH_TABLE_ENT tableType;  /* metering table type */
    GT_U32              addOffset;      /* additional offset for field entry */
                                        /* when parsing eArch entry */
    GT_STATUS           retStatus;      /* generic return status code */

    /*********************************************************/
    /* Read Word 3,4,5,6 of requested Policer Metering Entry */
    /* Read Also word 7 in case of Ingress policer in XCAT and above  */
    /*********************************************************/
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
        {
            tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
        }
        else if (CPSS_DXCH_POLICER_STAGE_INGRESS_1_E == stage )
        {
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                /* access to Metering Token Bucket table is done by IPLR0 */
                tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
                entryIndex += PLR0_SIZE_MAC(devNum);
            }
            else
            {
                tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E;
            }
        }
        else /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
        {
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                /* access to Metering Token Bucket table (and meter config) is done by IPLR0 */
                tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
                entryIndex += PLR0_SIZE_MAC(devNum) + PLR1_SIZE_MAC(devNum);
            }
            else
            {
                tableType = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E;
            }
        }

        retStatus =  prvCpssDxChPortGroupReadTableEntry(
                        devNum,
                        portGroupId,
                        tableType,
                        entryIndex,
                        &hwDataArr[0]);

        if (retStatus != GT_OK)
        {
            return retStatus;
        }
        /* meter config table*/
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                if(CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
                {
                   tableType = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E;
                }
                else if (CPSS_DXCH_POLICER_STAGE_INGRESS_1_E == stage )
                {
                     if (IPLR1_IS_SHARED_MAC(devNum))
                     {
                         PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                        /* access to Metering Configuration table is done by IPLR0 */
                        tableType = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E;
                     }
                     else
                     {
                         tableType = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E;
                     }
                }
                else /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
                {
                    if (EPLR_IS_SHARED_MAC(devNum))
                    {
                         PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                        /* access to Metering Configuration table is done by IPLR0 */
                        tableType = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E;
                     }
                    else
                     {
                         tableType = CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E;
                     }
                }

                /* read the 'meter config' entry using the generic table read mechanism */
                retStatus =  prvCpssDxChPortGroupReadTableEntry(
                                devNum,
                                portGroupId,
                                tableType,
                                entryIndex,
                                &hwFormatMeterConfigArray[0]);
                if (retStatus != GT_OK)
                {
                    return retStatus;
                }
            }

            return sip5PolicerMeteringEntryGet(
                devNum, stage,
                entryPtr, envelopeCfgPtr,
                hwDataArr, hwFormatMeterConfigArray);
        }

        hwData = &hwDataArr[3];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            PLR[stage].policerMeteringCountingTbl + (entryIndex * METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS) + 12;

        retStatus = prvCpssHwPpPortGroupReadRam(devNum, portGroupId, regAddr,
                                                   5, &hwDataArr[0]);
        hwData = &hwDataArr[0];
    }

    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /**********/
    /* Word 3 */
    /**********/

    /* Get RateType0 value */
    hwRateType0 = (hwData[0] >> 26) & 0x7;
    /* Get RateType1 value */
    hwRateType1 = (hwData[0] >> 29);

    /**********/
    /* Word 4 */
    /**********/

    /* Get Rate 0 (CIR) */
    /* bits 0:16 */
    hwRate0 = (hwData[1] & 0x1FFFF);
    /* Set 15 LSB of Rate 1 (CIR) */
    /* bits 17:31 */
    hwRate1 = (hwData[1] >> 17);

    /**********/
    /* Word 5 */
    /**********/

    /* Get 2 MSB of Rate 1 (CIR) */
    /* bits 0:1 */
    hwRate1 |= ((hwData[2] & 0x3) << 15);
    /* Get Maximum burst size for bucket 0 */
    /* bits 2:17 */
    hwBurstSize0 = (hwData[2] >> 2) & 0xFFFF;
    /* Get 14 LSB of Maximum burst size for bucket 1 */
    /* bits 18:31 */
    hwBurstSize1 = (hwData[2] >> 18);

    /**********/
    /* Word 6 */
    /**********/

    /* Get 2 MSB of Maximum burst size for bucket 1 */
    /* bits 0:1 */
    hwBurstSize1 |= ((hwData[3] & 0x3) << 14);
    /* Get Color Mode hw value */
    /* bit 2 */
    meterColorModeVal = (hwData[3] >> 2) & 0x1;
    /* Get Meter Mode hw value */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* bits 3:4 */
        meterModeVal = (hwData[3] >> 3) & 0x3;
        addOffset = 1;
    }
    else
    {
        /* bit 3 */
        meterModeVal = (hwData[3] >> 3) & 0x1;
        addOffset = 0;
    }
    /* Get Management Counter Set hw value */
    /* bits 4:5 or bits 5:6 for eArch */
    mngCounterSetVal = (hwData[3] >> (4+addOffset)) & 0x3;
    /* Get pointer to related counting entry */
    /* bits 6:21 or bits 7:22 for eArch */
    entryPtr->countingEntryIndex = (hwData[3] >> (6+addOffset)) & 0xFFFF;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* Get byte or packet metering mode */
        /* bit 23 */
        entryPtr->byteOrPacketCountingMode =
         (((hwData[3] >> 23) & 0x1) == 0) ?
            CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E :
            CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E;

        /* Get packet size mode & include layer1 overhead */
        /* bits 24 & 26 */
        switch((hwData[3] >> 24) & 0x5)
        {
            case 0:
                entryPtr->packetSizeMode = CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;
                break;
            case 1:
                entryPtr->packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
                break;
            case 5:
                entryPtr->packetSizeMode = CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* Get TT/TS packet size mode */
        /* bits 25 */
        entryPtr->tunnelTerminationPacketSizeMode =
         (((hwData[3] >> 25) & 0x1) == 0) ?
            CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E :
            CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E;

        /* Get DSA Tag metering mode */
        /* bits 27 */
        entryPtr->dsaTagCountingMode =
         (((hwData[3] >> 27) & 0x1) == 0) ?
            CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E :
            CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E;

        /* Get Timestamp Tag metering mode */
        /* bits 28 */
        entryPtr->timeStampCountingMode =
         (((hwData[3] >> 28) & 0x1) == 0) ?
            CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E :
            CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E;
    }

    /* check Policer stage: Ingress or Egress */
    if (PRV_CPSS_DXCH_POLICER_INGRESS_STAGE_CHECK_MAC(stage))
    { /* IPLR */

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            /* Get Policer commands hw value for Red Packet */
            /* bits 29:30 */
            redPcktCmdVal = (hwData[3] >> 29) & 0x3;
            /* Get lsb Policer commands hw value for Yellow Packet */
            /* bit 31 */
            yellowPcktCmdVal = (hwData[3] >> 31) & 0x1;

            /**********/
            /* Word 7 */
            /**********/

            /* Get msb Policer commands hw value for Yellow Packet */
            /* bit 0 */
            yellowPcktCmdVal |= ((hwData[4] & 0x1) << 1);
            /* Get Policer commands hw value for Green Packet */
            /* bits 1:2 */
            switch((hwData[4] >> 1) & 0x3)
            {
                case 0:
                    entryPtr->greenPcktCmd =
                                CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
                    break;
                case 1:
                    entryPtr->greenPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
                    break;
                case 2:
                    entryPtr->greenPcktCmd =
                                CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E;
                    break;
                case 3:
                    entryPtr->greenPcktCmd =
                                CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            /* Get Modify DSCP operation */
            /* bits 3:4 */
            modifyDscpVal = ((hwData[4] >> 3) & 0x3);
            /* Get Modify UP operation */
            /* bits 5:6 */
            modifyUpVal = ((hwData[4] >> 5) & 0x3);
            /* Get Qos Profile */
            /* bits 7:16 */
            entryPtr->qosProfile = (hwData[4] >> 7) & 0x3FF;
        }
        else
        {
            /* Get Policer commands hw value for Red Packet */
            /* bits 22:23 */
            redPcktCmdVal = (hwData[3] >> 22) & 0x3;
            /* Get Policer commands hw value for Yellow Packet */
            /* bits 24:25 */
            yellowPcktCmdVal = (hwData[3] >> 24) & 0x3;
            /* Get Modify DSCP operation */
            /* bits 26:27 */
            modifyDscpVal = ((hwData[3] >> 26) & 0x3);
            /* Get Modify UP operation */
            /* bits 28:29 */
            modifyUpVal = ((hwData[3] >> 28) & 0x3);
            /* Get 2 LSB of Qos Profile */
            /* bits 30:31 */
            entryPtr->qosProfile = (hwData[3] >> 30);

            /**********/
            /* Word 7 */
            /**********/

            /* Get 5 MSB of Qos Profile */
            /* bits 0:4 */
            entryPtr->qosProfile |= ((hwData[4] & 0x1F) << 2);
        }

        switch(modifyDscpVal)
        {
            case 0:
                entryPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                break;
            case 1:
                entryPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
                break;
            case 2:
                entryPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch(modifyUpVal)
        {
            case 0:
                entryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                break;
            case 1:
                entryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
                break;
            case 2:
                entryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    { /* EPLR */

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            /* Get Remark mode */
            /* bit 29 */
            remarkModeVal = ((hwData[3] >> 29) & 0x1);

            /* Get MX Non Conforming Packet Command */
            /* bit 30 */
            redPcktCmdVal = ((hwData[3] >> 30) & 0x1);

            /* Get Modify EXP operation */
            /* bit 31 */
            entryPtr->modifyExp =
                (((hwData[3] >> 31) & 0x1) == 0) ?
                            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E :
                            CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

            /**********/
            /* Word 7 */
            /**********/

            /* Get Modify DSCP operation */
            /* bits 0:1 */
            modifyDscpVal = (hwData[4] & 0x3);

            /* Get Modify TC operation */
            /* bit 2 */
            entryPtr->modifyTc =
                (((hwData[4] >> 2) & 0x1) == 0) ?
                            CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E :
                            CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;

            /* Get Modify UP operation */
            /* bits 3:4 */
            modifyUpVal = ((hwData[4] >> 3) & 0x3);

            /* Get Modify DP operation */
            /* bit 5 */
            modifyDpVal = ((hwData[4] >> 5) & 0x1);

            /* Get enable ECN marking */
            /* bit 6 */
            entryPtr->yellowEcnMarkingEnable =
                (((hwData[4] >> 6) & 0x1) == 0) ? GT_FALSE :GT_TRUE;
        }
        else
        {
            /* Get Remark mode */
            /* bit 22 */
            remarkModeVal = ((hwData[3] >> 22) & 0x1);

            /* Get MX Non Conforming Packet Command */
            /* bit 23 */
            redPcktCmdVal = ((hwData[3] >> 23) & 0x1);

            /* Get Modify DSCP operation */
            /* bit 25 */
            modifyDscpVal = ((hwData[3] >> 25) & 0x1);

            /* Get Modify UP operation */
            /* bit 27 */
            modifyUpVal = ((hwData[3] >> 27) & 0x1);

            /* Get Modify DP operation */
            /* bit 28 */
            modifyDpVal = ((hwData[3] >> 28) & 0x1);
        }

        switch (remarkModeVal)
        {
            case 0:
                entryPtr->remarkMode = CPSS_DXCH_POLICER_REMARK_MODE_L2_E;
                break;
            case 1:
                entryPtr->remarkMode = CPSS_DXCH_POLICER_REMARK_MODE_L3_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* never should be reached*/
        }

        switch(redPcktCmdVal)
        {
            case 0:
            case 1:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* never should be reached*/
        }

        switch(modifyDscpVal)
        {
            case 0: entryPtr->modifyDscp =
                                        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                    break;
            case 1: entryPtr->modifyDscp =
                                        CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
                    break;
            case 2: if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) ==
                                                                      GT_FALSE)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                    }
                    entryPtr->modifyDscp =
                                 CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_INNER_E;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch(modifyUpVal)
        {
            case 0: entryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                    break;
            case 1: entryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
                    break;
            case 2: if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) ==
                                                                      GT_FALSE)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                    }
                    entryPtr->modifyUp =
                                 CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_TAG0_E;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        entryPtr->modifyDp = (modifyDpVal) ?
                                        CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
                                        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;

        /* Not relevant for Egress Policer */
        yellowPcktCmdVal = 0;
    }

    /* Get Policer command for Red Packet */
    switch(redPcktCmdVal)
    {
        case 0:
            entryPtr->redPcktCmd =
                                CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
            break;
        case 1:
            entryPtr->redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
            break;
        case 2:
            entryPtr->redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E;
            break;
        case 3:
            entryPtr->redPcktCmd =
                CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* never should be reached*/
    }

    /* Get Policer command for Yellow Packet */
    switch(yellowPcktCmdVal)
    {
        case 0:
            entryPtr->yellowPcktCmd =
                                CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
            break;
        case 1:
            entryPtr->yellowPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
            break;
        case 2:
            entryPtr->yellowPcktCmd =
                                CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E;
            break;
        case 3:
            entryPtr->yellowPcktCmd =
                                CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* never should be reached*/
    }

    /* Get Color Mode */
    entryPtr->meterColorMode = (meterColorModeVal) ?
                                            CPSS_POLICER_COLOR_AWARE_E :
                                            CPSS_POLICER_COLOR_BLIND_E;

    /* Get Meter Mode */
    switch(meterModeVal)
    {
        case 0:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E;
            break;
        case 1:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E;
            break;
        case 2:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_MEF0_E;
            break;
        case 3:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_MEF1_E;
            break;
        case 4:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E;
            break;
        case 5:
            entryPtr->meterMode = CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* never should be reached*/
    }

    /* Get Management Counter Set */
    switch(mngCounterSetVal)
    {
        case 0:
            entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E;
            break;
        case 1:
            entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E;
            break;
        case 2:
            entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E;
            break;
        case 3:
            entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* never should be reached*/
    }

    /****************************************************/
    /* Convert Token Bucket Parameters in the SW format */
    /****************************************************/
    switch(entryPtr->meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
            entryPtr->tokenBucketParams.srTcmParams.cir = hwRate0;
            entryPtr->tokenBucketParams.srTcmParams.cbs = hwBurstSize0;
            entryPtr->tokenBucketParams.srTcmParams.ebs = hwBurstSize1;

            retStatus = prvCpssDxCh3PolicerSwTbCalculate(
                entryPtr->meterMode,
                hwRateType0,
                0,
                &(entryPtr->tokenBucketParams),
                &(entryPtr->tokenBucketParams));

            if (retStatus != GT_OK)
            {
                return retStatus;
            }
            break;

        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
            entryPtr->tokenBucketParams.trTcmParams.cir = hwRate0;
            entryPtr->tokenBucketParams.trTcmParams.pir = hwRate1;
            entryPtr->tokenBucketParams.trTcmParams.cbs = hwBurstSize0;
            entryPtr->tokenBucketParams.trTcmParams.pbs = hwBurstSize1;

            retStatus = prvCpssDxCh3PolicerSwTbCalculate(
                entryPtr->meterMode,
                hwRateType0,
                hwRateType1,
                &(entryPtr->tokenBucketParams),
                &(entryPtr->tokenBucketParams));

            if (retStatus != GT_OK)
            {
                return retStatus;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* never should be reached*/
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxCh3PolicerMeteringEntryGet function
* @endinternal
*
* @brief   Gets Metering Policer Entry configuration.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @param[out] entryPtr                 - pointer to the requested metering policer entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
static GT_STATUS internal_cpssDxCh3PolicerMeteringEntryGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr
)
{
    return cpssDxChPolicerPortGroupMeteringEntryGet(
                                            devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            stage,
                                            entryIndex,
                                            entryPtr);
}

/**
* @internal cpssDxCh3PolicerMeteringEntryGet function
* @endinternal
*
* @brief   Gets Metering Policer Entry configuration.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @param[out] entryPtr                 - pointer to the requested metering policer entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
GT_STATUS cpssDxCh3PolicerMeteringEntryGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerMeteringEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, entryIndex, entryPtr));

    rc = internal_cpssDxCh3PolicerMeteringEntryGet(devNum, stage, entryIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, entryIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerEntryMeterParamsCalculate function
* @endinternal
*
* @brief   Calculates Token Bucket parameters in the Application format without
*         HW update.
*         The token bucket parameters are returned as output values. This is
*         due to the hardware rate resolution, the exact rate or burst size
*         requested may not be honored. The returned value gives the user the
*         actual parameters configured in the hardware.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] tbInParamsPtr            - pointer to Token bucket input parameters.
* @param[in] meterMode                - Meter mode (SrTCM or TrTCM).
*
* @param[out] tbOutParamsPtr           - pointer to Token bucket output paramters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong devNum or meterMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxCh3PolicerEntryMeterParamsCalculate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  CPSS_DXCH3_POLICER_METER_MODE_ENT       meterMode,
    IN  CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbInParamsPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbOutParamsPtr
)
{
    GT_U32      rateType0;  /* Rate Type 0 */
    GT_U32      rateType1;  /* Rate Type 1 */
    GT_STATUS   retStatus;  /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(tbInParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(tbOutParamsPtr);

    /* Token Bucket Calculation */
    switch(meterMode)
    {
        case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
            retStatus = prvCpssDxCh3PolicerHwTbCalculate(
                      devNum,
                      stage,
                      tbInParamsPtr->srTcmParams.cir,
                      tbInParamsPtr->srTcmParams.cbs,
                      tbInParamsPtr->srTcmParams.ebs,
                      &(tbOutParamsPtr->srTcmParams.cir), /* HW Rate */
                      &(tbOutParamsPtr->srTcmParams.cbs), /* HW MaxBurstSize0 */
                      &(tbOutParamsPtr->srTcmParams.ebs), /* HW MaxBurstSize1 */
                      &rateType0);                        /* HW RateType */

            if (retStatus == GT_OK)
            {
                return prvCpssDxCh3PolicerSwTbCalculate(meterMode,
                                                         rateType0,
                                                         0,
                                                         tbOutParamsPtr,
                                                         tbOutParamsPtr);
            }
            break;

        case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
        case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
            retStatus = prvCpssDxCh3PolicerHwTbCalculate(
                      devNum,
                      stage,
                      tbInParamsPtr->trTcmParams.cir,
                      tbInParamsPtr->trTcmParams.cbs,
                      0,                        /* Not exists in the TrTCM */
                      &(tbOutParamsPtr->trTcmParams.cir), /* HW Rate0 */
                      &(tbOutParamsPtr->trTcmParams.cbs), /* HW MaxBurstSize0 */
                      NULL,                     /* Not exists in the TrTCM */
                      &rateType0);                        /* HW RateType0 */

            if (retStatus != GT_OK)
            {
                return retStatus;
            }

            retStatus = prvCpssDxCh3PolicerHwTbCalculate(
                      devNum,
                      stage,
                      tbInParamsPtr->trTcmParams.pir,
                      tbInParamsPtr->trTcmParams.pbs,
                      0,                        /* Not exists in the TrTCM */
                      &(tbOutParamsPtr->trTcmParams.pir), /* HW Rate1 */
                      &(tbOutParamsPtr->trTcmParams.pbs), /* HW MaxBurstSize1 */
                      NULL,                     /* Not exists in the TrTCM */
                      &rateType1);                      /* HW RateType1 */

            if (retStatus == GT_OK)
            {
                return prvCpssDxCh3PolicerSwTbCalculate(meterMode,
                                                        rateType0,
                                                        rateType1,
                                                        tbOutParamsPtr,
                                                        tbOutParamsPtr);
            }

            break;

        case CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E:
        case CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E:
            /* calculate twice - for cir and for maxCir*/
            retStatus = prvCpssDxChPolicerHwTbEnvelopeCalculate(
                devNum, stage,
                tbInParamsPtr->envelope.cir /*swRate*/,
                tbInParamsPtr->envelope.cbs /*swBurstSize*/,
                tbInParamsPtr->envelope.maxCir /*swMaxRate*/,
                &(tbOutParamsPtr->envelope.cir) /*hwRatePtr*/,
                &(tbOutParamsPtr->envelope.cbs) /*hwBurstSizePtr*/,
                &rateType0 /*rateTypePtr*/);
            if (retStatus != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            retStatus = prvCpssDxChPolicerHwTbEnvelopeCalculate(
                devNum, stage,
                tbInParamsPtr->envelope.maxCir /*swRate*/,
                tbInParamsPtr->envelope.cbs /*swBurstSize*/,
                tbInParamsPtr->envelope.cir /*swMaxRate*/,
                &(tbOutParamsPtr->envelope.maxCir) /*hwRatePtr*/,
                &(tbOutParamsPtr->envelope.cbs) /*hwBurstSizePtr*/,
                &rateType0 /*rateTypePtr*/);
            if (retStatus != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* calculate twice - for eir and for maxEir*/
            retStatus = prvCpssDxChPolicerHwTbEnvelopeCalculate(
                devNum, stage,
                tbInParamsPtr->envelope.eir /*swRate*/,
                tbInParamsPtr->envelope.ebs /*swBurstSize*/,
                tbInParamsPtr->envelope.maxEir /*swMaxRate*/,
                &(tbOutParamsPtr->envelope.eir) /*hwRatePtr*/,
                &(tbOutParamsPtr->envelope.ebs) /*hwBurstSizePtr*/,
                &rateType1 /*rateTypePtr*/);
            if (retStatus != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            retStatus = prvCpssDxChPolicerHwTbEnvelopeCalculate(
                devNum, stage,
                tbInParamsPtr->envelope.maxEir /*swRate*/,
                tbInParamsPtr->envelope.ebs /*swBurstSize*/,
                tbInParamsPtr->envelope.eir /*swMaxRate*/,
                &(tbOutParamsPtr->envelope.maxEir) /*hwRatePtr*/,
                &(tbOutParamsPtr->envelope.ebs) /*hwBurstSizePtr*/,
                &rateType1 /*rateTypePtr*/);
            if (retStatus != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            return prvCpssDxCh3PolicerSwTbCalculate(
                meterMode, rateType0, rateType1, tbOutParamsPtr, tbOutParamsPtr);

            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerEntryMeterParamsCalculate function
* @endinternal
*
* @brief   Calculates Token Bucket parameters in the Application format without
*         HW update.
*         The token bucket parameters are returned as output values. This is
*         due to the hardware rate resolution, the exact rate or burst size
*         requested may not be honored. The returned value gives the user the
*         actual parameters configured in the hardware.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] tbInParamsPtr            - pointer to Token bucket input parameters.
* @param[in] meterMode                - Meter mode (SrTCM or TrTCM).
*
* @param[out] tbOutParamsPtr           - pointer to Token bucket output paramters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong devNum or meterMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxCh3PolicerEntryMeterParamsCalculate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  CPSS_DXCH3_POLICER_METER_MODE_ENT       meterMode,
    IN  CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbInParamsPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbOutParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerEntryMeterParamsCalculate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, meterMode, tbInParamsPtr, tbOutParamsPtr));

    rc = internal_cpssDxCh3PolicerEntryMeterParamsCalculate(devNum, stage, meterMode, tbInParamsPtr, tbOutParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, meterMode, tbInParamsPtr, tbOutParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPolicerBillingCntrToHwConvert function
* @endinternal
*
* @brief   Convert counters part and billing mode of Billing entry
*         of xCat and above to the HW format.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] billingCntrPtr           - pointer to the Billing Counters Entry.
* @param[in] billingCntrModeHwVal     - HW value for counters mode
*
* @param[out] hwDataPtr                - pointer to the HW data.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong packet size mode.
*/
static GT_STATUS prvCpssDxChPolicerBillingCntrToHwConvert
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr,
    IN  GT_U32                                   billingCntrModeHwVal,
    OUT GT_U32                                  *hwDataPtr
)
{
    GT_U32 hwDataTmp;

    hwDataPtr[0] = billingCntrPtr->greenCntr.l[0];
    hwDataPtr[1] = (billingCntrPtr->greenCntr.l[1]          |
                ((billingCntrPtr->yellowCntr.l[0] & 0x3FFFFF) << 10));
    hwDataPtr[2] = (billingCntrPtr->yellowCntr.l[0] >> 22) |
                (billingCntrPtr->yellowCntr.l[1] << 10) |
                ((billingCntrPtr->redCntr.l[0] & 0xFFF) << 20);

    hwDataPtr[3] = (billingCntrPtr->redCntr.l[0] >> 12)    |
                (billingCntrPtr->redCntr.l[1] << 20);

    /* convert counters mode */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        /* bits 126..127 */
        hwDataPtr[3] |= (billingCntrModeHwVal << 30);
    }
    else
    {
        /* bits 140..141 */
        hwDataPtr[4] = (billingCntrModeHwVal << 12);

    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        hwDataTmp = (billingCntrPtr->billingCntrAllEnable == GT_TRUE) ?
            (1 << 14) : 0;

        switch(billingCntrPtr->lmCntrCaptureMode)
        {
            case CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_COPY_E:
                break;
            case CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_INSERT_E:
                hwDataTmp |= 1 << 15;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        }
        /* bit 142...159 */
        hwDataPtr[4] |= hwDataTmp |
                        ((billingCntrPtr->greenCntrSnapshot.l[0] & 0xFFFF) << 16);

        hwDataTmp = (billingCntrPtr->greenCntrSnapshotValid == GT_TRUE) ? 1 : 0;
        /* bit 160...186 */
        hwDataPtr[5]  = ( billingCntrPtr->greenCntrSnapshot.l[0] >> 16 |
                        ((billingCntrPtr->greenCntrSnapshot.l[1] & 0x3FF) << 16) |
                        ( hwDataTmp << 26));

        /* Convert counting packet size to the word */
        switch(billingCntrPtr->packetSizeMode)
        {
            case CPSS_POLICER_PACKET_SIZE_L3_ONLY_E:
                hwDataTmp = 0;
                break;
            case CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E:
            case CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E:
                hwDataTmp = 0;
                if (billingCntrPtr->packetSizeMode == CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E)
                {
                    hwDataTmp |= 1 << 29;
                }
                hwDataTmp |= 1 << 27;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch(billingCntrPtr->tunnelTerminationPacketSizeMode)
        {
            case CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E:
                break;
            case CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E:
                hwDataTmp |= 1 << 28;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch(billingCntrPtr->dsaTagCountingMode)
        {
            case CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E:
                break;
            case CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E:
                hwDataTmp |= 1 << 30;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch(billingCntrPtr->timeStampCountingMode)
        {
            case CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E:
                break;
            case CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E:
                hwDataTmp |= 1 << 31;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* bit 187...191 */
        hwDataPtr[5] |= hwDataTmp;
    }


    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) CPSS_TBD_BOOKMARK_BOBCAT2_BOBK
    {
        GT_U32  tmp_hwDataPtr[2] = {0};
        GT_U32  numBitsToMove = 1+(191-143);

        /* all bits from bit 143 <LM Counter Capture Mo> moved 1 bit */
        /* except for last 2 bits <Counter_Mode> in bit 228 --> that need value 0 */
        /* so we actually need to move bits 143..191 by 1 bit to 144..192 */

        /* copy the needed bits to tmp place */
        copyBits(tmp_hwDataPtr,1,
                 hwDataPtr,143,
                 numBitsToMove);

        /* copy the needed bits to new position */
        copyBits(hwDataPtr,143,
                 tmp_hwDataPtr,0,
                 numBitsToMove+1);
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChPolicerBillingEntrySet function
* @endinternal
*
* @brief   Sets Policer Billing Counters of xCat and above.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out upon counter reset by
*                                       indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*       In order to set Billing entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
static GT_STATUS prvCpssDxChPolicerBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    GT_U32      regAddr;                /* register address */
    GT_U32      billingCntrModeHwVal;   /* Billing counter mode hw value */
    GT_U32      hwData[8];              /* HW data */
    GT_U32      portGroupId;            /* port group id */
    GT_U32      wordAddr;               /* entry word's address */
    GT_U16      portGroupNum;           /* number of port groups      */
    GT_U16      portGroupIndex;         /* index of port group in BMP */
    GT_STATUS   rc;                     /* return code */
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    billingCntr; /* counters values */
    GT_U32      numOfWords;             /* number of words to be written to memory */
    CPSS_DXCH_TABLE_ENT tableType;

    /* Check Billing Counters mode */
    switch (billingCntrPtr->billingCntrMode)
    {
        case CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E:
            billingCntrModeHwVal = 0;
            break;
        case CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E:
            billingCntrModeHwVal = 1;
            break;
        case CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E:
            billingCntrModeHwVal = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Check MSB of word 1 of counters */
    if ((billingCntrPtr->greenCntr.l[1] >> 10) |
        (billingCntrPtr->yellowCntr.l[1] >> 10) |
        (billingCntrPtr->redCntr.l[1] >> 10))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(hwData, 0, sizeof(hwData));

    /* calculate number of port groups */
    rc = prvCpssPortGroupsNumActivePortGroupsInBmpGet(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_IPLR_E,&portGroupNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Check if Policer Counting memory errata is enabled */
    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                               PRV_CPSS_DXCH_LION_POLICER_COUNTING_MEM_ACCESS_WA_E))
    {
        /* check that the previous action is finished in each portGroup, do busy wait */
        rc = prvCpssDxCh3PolicerAccessControlBusyWait(devNum, portGroupsBmp, stage);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Get Ingress Policer Table Access Data 0 Register Address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        PLR[stage].policerTblAccessDataReg;

    }
    else
    {
         if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
         {
             regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCountingTbl;
         }
         else
         {
             regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].policerCountingTbl;
         }

         regAddr += (entryIndex * METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS);
    }

    /* initialize values of local billing counter entry by values from original one */
    cpssOsMemCpy(&billingCntr, billingCntrPtr,
                 sizeof(CPSS_DXCH3_POLICER_BILLING_ENTRY_STC));

    if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
    {
        tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E;
    }
    else if (CPSS_DXCH_POLICER_STAGE_INGRESS_1_E == stage )
    {
        tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E;
    }
    else /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
    {
        tableType = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E;
    }

    /* loop on all port groups to set the counter value */
    portGroupIndex = 0;
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        /* calculate average value for green counter */
        prvCpssPortGroupCounterAverageValueGet(portGroupNum, portGroupIndex,
                                                   billingCntrPtr->greenCntr,
                                                   &billingCntr.greenCntr);

        /* calculate average value for yellow counter */
        prvCpssPortGroupCounterAverageValueGet(portGroupNum, portGroupIndex,
                                                   billingCntrPtr->yellowCntr,
                                                   &billingCntr.yellowCntr);

        /* calculate average value for red counter */
        prvCpssPortGroupCounterAverageValueGet(portGroupNum, portGroupIndex,
                                                   billingCntrPtr->redCntr,
                                                   &billingCntr.redCntr);

        if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
        {
            /* calculate average value for green counter snapshot counter */
            prvCpssPortGroupCounterAverageValueGet(portGroupNum, portGroupIndex,
                                                   billingCntrPtr->greenCntrSnapshot,
                                                   &billingCntr.greenCntrSnapshot);
        }

        /* convert entry */
        rc = prvCpssDxChPolicerBillingCntrToHwConvert(devNum, &billingCntr,
                                                      billingCntrModeHwVal, hwData);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* update HW table shadow (if supported and requested) */
        rc = prvCpssDxChPortGroupShadowEntryWrite(devNum, portGroupId,
                                                  tableType,
                                                  entryIndex, hwData);
        if (rc != GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "update hw table shadow failed");
        }

        /* Check if Policer Counting memory errata is enabled */
        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                  PRV_CPSS_DXCH_LION_POLICER_COUNTING_MEM_ACCESS_WA_E))
        {
            /* Indirect access to counting memory should be used */

            /* Write data to Policer Table Access Data registers */
            rc = prvCpssHwPpPortGroupWriteRam(devNum, portGroupId,
                                                 regAddr, 8, hwData);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Trigger the read & write action */
            rc = prvCpssDxCh3PolicerInternalTableAccess(devNum,
                                  portGroupId,
                                  stage,
                                  entryIndex,
                                  PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                numOfWords = PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)  ? 7 : 6;
            }
            else
            {
                numOfWords = 5;
            }

            /* set words with counters and billing mode */
            rc = prvCpssHwPpWriteRam(
                devNum, regAddr, numOfWords/* words*/, hwData);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* verify that all words were written before write last one.
               because last one triggers write of whole entry */
            GT_SYNC;

            /* set last word 7 - Counter Mode bit#5 is always 0 (billing).
              this write triggers update of whole entry. */
            wordAddr = regAddr + 0x1C;

            rc = prvCpssHwPpPortGroupWriteRegister(devNum, portGroupId, wordAddr, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        portGroupIndex++;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal internal_cpssDxCh3PolicerBillingEntrySet function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out upon counter reset by
*                                       indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*       In order to set Billing entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxChIpfixEntrySet".
*
*/
static GT_STATUS internal_cpssDxCh3PolicerBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    return cpssDxChPolicerPortGroupBillingEntrySet(
                                        devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        stage,
                                        entryIndex,
                                        billingCntrPtr);
}

/**
* @internal cpssDxCh3PolicerBillingEntrySet function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out upon counter reset by
*                                       indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*       In order to set Billing entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxChIpfixEntrySet".
*
*/
GT_STATUS cpssDxCh3PolicerBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerBillingEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, entryIndex, billingCntrPtr));

    rc = internal_cpssDxCh3PolicerBillingEntrySet(devNum, stage, entryIndex, billingCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, entryIndex, billingCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPolicerBillingEntryGet function
* @endinternal
*
* @brief   Gets Policer Billing Counters of xCat and above.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*/
static GT_STATUS prvCpssDxChPolicerBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    GT_U32      regAddr;                /* register address */
    GT_U32      regValue;               /* register hw value */
    GT_U32      billingCntrModeHwVal;   /* Billing counter mode hw value */
    GT_U32      portGroupId;            /* port group id */
    GT_U64      tmp;                    /* temporary variable */
    GT_STATUS   retStatus;              /* generic return status code */
    GT_U32      bitOffset;              /* bit offset between format of sip5 and sip_5_20 */

    /******************************************************************/
    /* Get counter value from the corresponding Access Data Registers */
    /******************************************************************/

    /* Get the first active port group */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp,
                                                        portGroupId);

    /* Get Ingress Policer Table Access Data 0 Register Address */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    policerTableAccessData[0];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].policerTblAccessDataReg;
    }

    /* Get Ingress Policer Table Access Data 7 Register */
    regAddr += 28;

    retStatus = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                                    regAddr, &regValue);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /* Check that counter mode is Billing and not IPFIX */
    if ((regValue >> 4) & 0x1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* Set the register address back to the register 0 */
    regAddr -= 28;

    retStatus = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp,
                                                       regAddr,0,32,
                                                       NULL,
                                                       &billingCntrPtr->greenCntr);
    if (GT_OK != retStatus)
    {
        return retStatus;
    }

    /* Get Ingress Policer Table Access Data 1 Register */
    regAddr += 4;

    retStatus = prvCpssPortGroupsBmpMultiCounterSummary(devNum,
                                portGroupsBmp,regAddr,
                                0,10,
                                &regValue,/* GC: 10 MSbit */
                                10,22,
                                &billingCntrPtr->yellowCntr.l[0],/* YC: 22 LSbit */
                                0,0,NULL);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    billingCntrPtr->greenCntr.l[1] += regValue;

    /* Get Ingress Policer Table Access Data 2 Register */
    regAddr += 4;

    retStatus = prvCpssPortGroupsBmpMultiCounterSummary(devNum,
                                portGroupsBmp, regAddr,
                                0,10,
                                &regValue,/* YC: 10 MSbit of first word */
                                10,10,
                                &billingCntrPtr->yellowCntr.l[1],/*YC:10MSbit*/
                                20,12,
                                &billingCntrPtr->redCntr.l[0]);/*RC:12LSbit*/
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /* the regValue is sum of bits 0..9 from up to all port groups.
       therefore value of the variable may be more then 10 bits. All MSBs after
       10th should be stored in the MSB word of tmp */
    tmp.l[0] = regValue << 22; /*YC: 10 MSbit of first word*/
    tmp.l[1] = regValue >> 10;

    billingCntrPtr->yellowCntr = prvCpssMathAdd64(billingCntrPtr->yellowCntr, tmp);

    /* Get Ingress Policer Table Access Data 3 Register */
    regAddr += 4;

    retStatus = prvCpssPortGroupsBmpMultiCounterSummary(devNum,
                                portGroupsBmp, regAddr,
                                0,20,
                                &regValue,/*RC:20LSbit*/
                                20,10,
                                &billingCntrPtr->redCntr.l[1],/*RC:10MSbit*/
                                0,0,NULL);
    if (GT_OK != retStatus)
    {
        return retStatus;
    }

    /* the regValue is sum of bits 0..19 from up to all port groups.
       therefore value of the variable may be more then 20 bits. All MSBs after
       20th should be stored in the MSB word of tmp */
    tmp.l[0] = regValue << 12; /*RC: 20 MSbits of first word */
    tmp.l[1] = regValue >> 20;

    billingCntrPtr->redCntr = prvCpssMathAdd64(billingCntrPtr->redCntr, tmp);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        /* Ingress Policer Table Access Data 3 Register */
        retStatus = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                                        regAddr, &regValue);
        if (GT_OK != retStatus)
        {
            return retStatus;
        }

        /* 2 bits of Cntr Mode */
        billingCntrModeHwVal = (regValue >> 30);
    }
    else
    {
        /* Get Ingress Policer Table Access Data 4 Register */
        regAddr += 4;

        retStatus = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                                        regAddr, &regValue);
        if (GT_OK != retStatus)
        {
            return retStatus;
        }

        /* 2 bits of Cntr Mode */
        billingCntrModeHwVal = ((regValue >> 12) & 0x3);
    }

    switch (billingCntrModeHwVal)
    {
        case 0:
            billingCntrPtr->billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E;
            break;
        case 1:
            billingCntrPtr->billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E;
            break;
        case 2:
            billingCntrPtr->billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) CPSS_TBD_BOOKMARK_BOBCAT2_BOBK
    {
        bitOffset = 1;
    }
    else
    {
        bitOffset = 0;
    }


    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            billingCntrPtr->billingCntrAllEnable = (((regValue >> 14) & 0x1) == 0) ?
                GT_FALSE : GT_TRUE;
        }
        else
        {
            switch((regValue >> 14) & 0x3)/*2 bits*/
            {
                case 0:
                    billingCntrPtr->billingCntrAllEnable = GT_FALSE;
                    break;
                case 1:
                    billingCntrPtr->billingCntrAllEnable = GT_TRUE;
                    break;
                default:
                    /*value 2 not supported by CPSS API at this stage */
                    /*value 3 not supported by the device */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

        /* LM Counter Capture Mode */
        billingCntrPtr->lmCntrCaptureMode = (((regValue >> (bitOffset+15)) & 0x1) == 0) ?
            CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_COPY_E :
            CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_INSERT_E;

        /* 16 bit of LM Counter Capture Mode */
        retStatus = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp,
                                                    regAddr,(16+bitOffset),32-(bitOffset+16),
                                                    NULL,
                                                    &billingCntrPtr->greenCntrSnapshot);
        if (GT_OK != retStatus)
        {
         return retStatus;
        }

        /* Get Ingress Policer Table Access Data 5 Register */
        regAddr += 4;
        retStatus = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp,
                                                        regAddr,0,(26+bitOffset),
                                                        &regValue,
                                                        NULL);
        if (GT_OK != retStatus)
        {
         return retStatus;
        }


        /* the regValue is sum of bits 0..25 from up to all port groups.
        therefore value of the variable may be more then 26 bits. All MSBs after
        26th should be stored in the MSB word of tmp */
        tmp.l[0] = regValue << (32-(16+bitOffset)); /*RC: 16 MSbits of first word */
        tmp.l[1] = regValue >>     (16+bitOffset);

        billingCntrPtr->greenCntrSnapshot =
         prvCpssMathAdd64(billingCntrPtr->greenCntrSnapshot, tmp);

        billingCntrPtr->greenCntrSnapshotValid = GT_FALSE;
        retStatus = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp,
                                                       regAddr, 26+bitOffset, 1,
                                                       &regValue,NULL);
        if(retStatus != GT_OK)
        {
            return retStatus;
        }

        /* Green Counter Snapshot Valid */
        if(regValue)
        {
            billingCntrPtr->greenCntrSnapshotValid = GT_TRUE;
        }


        retStatus = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                                     regAddr, &regValue);
        if (GT_OK != retStatus)
        {
         return retStatus;
        }

        /* Packet size mode and Include Layer1 Overhead */
        switch((regValue >> (27+bitOffset)) & 0x5)
        {
         case 0:
             billingCntrPtr->packetSizeMode =
                 CPSS_POLICER_PACKET_SIZE_L3_ONLY_E;
             break;
         case 1:
             billingCntrPtr->packetSizeMode =
                 CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
             break;
         case 5:
             billingCntrPtr->packetSizeMode =
                 CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        /* Tunnel Termination Packet Size Mode */
        billingCntrPtr->tunnelTerminationPacketSizeMode =
         (((regValue >> (28+bitOffset)) & 0x1) == 0) ?
            CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E :
            CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E;

        /* DSA Tag Counting Mode */
        billingCntrPtr->dsaTagCountingMode =
         (((regValue >> (30+bitOffset)) & 0x1) == 0) ?
            CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E :
            CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E;

        if(bitOffset == 0)
        {
            /* Timestamp Tag Counting Mode */
            billingCntrPtr->timeStampCountingMode =
             (((regValue >> 31) & 0x1) == 0) ?
                CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E :
                CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E;
        }
        else
        {
            /* Get Ingress Policer Table Access Data 6 Register */
            regAddr += 4;

            retStatus = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                                         regAddr, &regValue);
            if (GT_OK != retStatus)
            {
                return retStatus;
            }


            /* Timestamp Tag Counting Mode */
            billingCntrPtr->timeStampCountingMode =
             (((regValue >> 0) & 0x1) == 0) ?
                CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E :
                CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E;

        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxCh3PolicerBillingEntryGet function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] reset                    -  flag:
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read counters operation only.
*
* @param[out] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  GT_BOOL                                 reset,
    OUT CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    return cpssDxChPolicerPortGroupBillingEntryGet(
                                        devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        stage,
                                        entryIndex,
                                        reset,
                                        billingCntrPtr);
}

/**
* @internal cpssDxCh3PolicerBillingEntryGet function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] reset                    -  flag:
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read counters operation only.
*
* @param[out] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*/
GT_STATUS cpssDxCh3PolicerBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  GT_BOOL                                 reset,
    OUT CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerBillingEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, entryIndex, reset, billingCntrPtr));

    rc = internal_cpssDxCh3PolicerBillingEntryGet(devNum, stage, entryIndex, reset, billingCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, entryIndex, reset, billingCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colors.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Bobcat3, Aldrin, AC3X Qos Remarking Entry contains three indexes of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] qosProfileIndex          - index of Qos Remarking Entry will be set.
* @param[in] greenQosTableRemarkIndex - QoS profile (index in the Qos Table)
*                                      assigned to In-Profile packets with
*                                      the Green color. (APPLICABLE RANGES: 0..1023).
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.).
* @param[in] yellowQosTableRemarkIndex - QoS profile (index in the Qos Table)
*                                      assigned to Out-Of-Profile packets with
*                                      the Yellow color.
*                                      (APPLICABLE RANGES: DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
* @param[in] redQosTableRemarkIndex   - QoS profile (index in the Qos Table)
*                                      assigned to Out-Of-Profile packets with
*                                      the Red color.
*                                      (APPLICABLE RANGES: DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or qosProfileIndex.
* @retval GT_OUT_OF_RANGE          - on greenQosTableRemarkIndex,
*                                       yellowQosTableRemarkIndex and
*                                       redQosTableRemarkIndex out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Supported only for Ingress Policer.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerQosRemarkingEntrySet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               qosProfileIndex,
    IN GT_U32                               greenQosTableRemarkIndex,
    IN GT_U32                               yellowQosTableRemarkIndex,
    IN GT_U32                               redQosTableRemarkIndex
)
{
    GT_U32      regAddr;       /* register address */
    GT_U32      hwDataArr[2];  /* HW data */
    GT_U32      addrAllign; /* address alignment between entries */
    CPSS_DXCH_TABLE_ENT tableType;  /* ingressQosRemarking table type */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, qosProfileIndex);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* check that the Policer is Ingress */
    if (stage > CPSS_DXCH_POLICER_STAGE_INGRESS_1_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* Check QoS profile indexes assigned to Yelloow and Red packets */
        if((greenQosTableRemarkIndex >= BIT_10)  ||
           (yellowQosTableRemarkIndex >= BIT_10) ||
           (redQosTableRemarkIndex >= BIT_10))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
        {
            tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_REMARKING_E;
        }
        else /* CPSS_DXCH_POLICER_STAGE_INGRESS_1_E */
        {
            tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_REMARKING_E;
        }

        hwDataArr[0] = (redQosTableRemarkIndex
            | (yellowQosTableRemarkIndex << 10)
            | (greenQosTableRemarkIndex << 20));

        return prvCpssDxChWriteTableEntry(devNum,
                                          tableType,
                                          qosProfileIndex,
                                          &hwDataArr[0]);
    }

    /* Check QoS profile indexes assigned to Yelloow and Red packets */
    if((yellowQosTableRemarkIndex >= PRV_CPSS_DXCH2_QOS_PROFILE_NUM_MAX_CNS) ||
       (redQosTableRemarkIndex >= PRV_CPSS_DXCH2_QOS_PROFILE_NUM_MAX_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat A1 and above devices has 8 bytes address alignment */
        addrAllign = 8;
    }
    else
    {
        /* CH3 and xCat/Lion A0 devices has 4 bytes address alignment */
        addrAllign = 4;
    }

    /* Calculate an address of Qos Remarking Entry will be set */
    regAddr = (addrAllign * qosProfileIndex) +
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerQosRemarkingTbl;

    hwDataArr[0] = redQosTableRemarkIndex | (yellowQosTableRemarkIndex << 7);

    /* Set Policer Relative Qos Remarking Entry */
    return prvCpssHwPpWriteRam(devNum, regAddr,
                                  (addrAllign >> 2) /* two words should be
                                            written for xCat A1 and above.
                                            Content of second word is
                                            meaningless and ignored and used
                                            only to trigger the write of the
                                            first word */,
                                   hwDataArr);
}

/**
* @internal cpssDxCh3PolicerQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colors.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Bobcat3, Aldrin, AC3X Qos Remarking Entry contains three indexes of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] qosProfileIndex          - index of Qos Remarking Entry will be set.
* @param[in] greenQosTableRemarkIndex - QoS profile (index in the Qos Table)
*                                      assigned to In-Profile packets with
*                                      the Green color. (APPLICABLE RANGES: 0..1023).
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.).
* @param[in] yellowQosTableRemarkIndex - QoS profile (index in the Qos Table)
*                                      assigned to Out-Of-Profile packets with
*                                      the Yellow color.
*                                      (APPLICABLE RANGES: DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
* @param[in] redQosTableRemarkIndex   - QoS profile (index in the Qos Table)
*                                      assigned to Out-Of-Profile packets with
*                                      the Red color.
*                                      (APPLICABLE RANGES: DxCh3, xCat, xCat3, Lion, xCat2, Lion2 0..127).
*                                      (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin2, Falcon, Aldrin, AC3X 0..1023).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or qosProfileIndex.
* @retval GT_OUT_OF_RANGE          - on greenQosTableRemarkIndex,
*                                       yellowQosTableRemarkIndex and
*                                       redQosTableRemarkIndex out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Supported only for Ingress Policer.
*
*/
GT_STATUS cpssDxCh3PolicerQosRemarkingEntrySet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               qosProfileIndex,
    IN GT_U32                               greenQosTableRemarkIndex,
    IN GT_U32                               yellowQosTableRemarkIndex,
    IN GT_U32                               redQosTableRemarkIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerQosRemarkingEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, qosProfileIndex, greenQosTableRemarkIndex, yellowQosTableRemarkIndex, redQosTableRemarkIndex));

    rc = internal_cpssDxCh3PolicerQosRemarkingEntrySet(devNum, stage, qosProfileIndex, greenQosTableRemarkIndex, yellowQosTableRemarkIndex, redQosTableRemarkIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, qosProfileIndex, greenQosTableRemarkIndex, yellowQosTableRemarkIndex, redQosTableRemarkIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxCh3PolicerQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colours.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Bobcat3, Aldrin, AC3X Qos Remarking Entry contains three indexes of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] qosProfileIndex          - index of requested Qos Remarking Entry.
*
* @param[out] greenQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      In-Profile packets with the
*                                      Green color.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.).
* @param[out] yellowQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      Out-Of-Profile packets with the
*                                      Yellow color.
* @param[out] redQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      Out-Of-Profile packets with the Red
*                                      color.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or qosProfileIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Supported only for Ingress Policer.
*
*/
static GT_STATUS internal_cpssDxCh3PolicerQosRemarkingEntryGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               qosProfileIndex,
    OUT GT_U32                              *greenQosTableRemarkIndexPtr,
    OUT GT_U32                              *yellowQosTableRemarkIndexPtr,
    OUT GT_U32                              *redQosTableRemarkIndexPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      hwDataArr[2];  /* HW data */
    GT_STATUS   retStatus;  /* generic return status code */
    GT_U32      addrAllign; /* address alignment between entries */
    CPSS_DXCH_TABLE_ENT tableType;  /* ingressQosRemarking table type */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, qosProfileIndex);
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_NULL_PTR_CHECK_MAC(greenQosTableRemarkIndexPtr);
    }
    CPSS_NULL_PTR_CHECK_MAC(yellowQosTableRemarkIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(redQosTableRemarkIndexPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* check that the Policer is Ingress */
    if (stage > CPSS_DXCH_POLICER_STAGE_INGRESS_1_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
        {
            tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_REMARKING_E;
        }
        else /* CPSS_DXCH_POLICER_STAGE_INGRESS_1_E */
        {
            tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_REMARKING_E;
        }

        retStatus =  prvCpssDxChReadTableEntry(devNum,
                                               tableType,
                                               qosProfileIndex,
                                               hwDataArr);

        if(GT_OK == retStatus)
        {
            *redQosTableRemarkIndexPtr      = hwDataArr[0] & 0x3FF;
            *yellowQosTableRemarkIndexPtr   = (hwDataArr[0] >> 10) & 0x3FF;
            *greenQosTableRemarkIndexPtr    = (hwDataArr[0] >> 20) & 0x3FF;
        }

        return retStatus;
    }

    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat A1 and above devices has 8 bytes address alignment */
        addrAllign = 8;
    }
    else
    {
        /* CH3 and xCat/Lion A0 devices has 4 bytes address alignment */
        addrAllign = 4;
    }

    /* Calculate an address of Qos Remarking Entry */
    regAddr = (addrAllign * qosProfileIndex) +
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerQosRemarkingTbl;

    /* Get Policer Relative Qos Remarking Entry */
    retStatus = prvCpssHwPpReadRam(devNum, regAddr, 1, hwDataArr);

    if(GT_OK == retStatus)
    {
        *redQosTableRemarkIndexPtr      = hwDataArr[0] & 0x7F;
        *yellowQosTableRemarkIndexPtr   = (hwDataArr[0] >> 7) & 0x7F;
    }

    return retStatus;
}

/**
* @internal cpssDxCh3PolicerQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Policer Relative Qos Remarking Entry.
*         The Qos Remarking Entry contains two indexes of Qos Profile
*         assigned to Out of profile packets with the Yellow and Red colours.
*         According to this indexes, new QoS parameters: UP,DP,TC and DSCP,
*         are extracted from the Qos Profile Table Entry.
*         Bobcat2, Caelum, Bobcat3, Aldrin, AC3X Qos Remarking Entry contains three indexes of Qos Profile
*         assigned to In-Profile Green colored and Out-Of-profile packets with the Yellow and Red colors.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] qosProfileIndex          - index of requested Qos Remarking Entry.
*
* @param[out] greenQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      In-Profile packets with the
*                                      Green color.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.).
* @param[out] yellowQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      Out-Of-Profile packets with the
*                                      Yellow color.
* @param[out] redQosTableRemarkIndexPtr - pointer to the QoS profile (index in
*                                      the Qos Table) assigned to
*                                      Out-Of-Profile packets with the Red
*                                      color.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or qosProfileIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Supported only for Ingress Policer.
*
*/
GT_STATUS cpssDxCh3PolicerQosRemarkingEntryGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               qosProfileIndex,
    OUT GT_U32                              *greenQosTableRemarkIndexPtr,
    OUT GT_U32                              *yellowQosTableRemarkIndexPtr,
    OUT GT_U32                              *redQosTableRemarkIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxCh3PolicerQosRemarkingEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, qosProfileIndex, greenQosTableRemarkIndexPtr, yellowQosTableRemarkIndexPtr, redQosTableRemarkIndexPtr));

    rc = internal_cpssDxCh3PolicerQosRemarkingEntryGet(devNum, stage, qosProfileIndex, greenQosTableRemarkIndexPtr, yellowQosTableRemarkIndexPtr, redQosTableRemarkIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, qosProfileIndex, greenQosTableRemarkIndexPtr, yellowQosTableRemarkIndexPtr, redQosTableRemarkIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerEgressQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Egress Policer Re-Marking table Entry.
*         Re-marking is the process of assigning new QoS parameters to the
*         packet, at the end of the metering process, based on the
*         Conformance Level the packet is assigned by metering process.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] remarkTableType          - Remark table type: DSCP, EXP or TC/UP.
* @param[in] remarkParamValue         - QoS parameter value.
* @param[in] confLevel                - Conformance Level: Green, Yellow, Red.
* @param[in] qosParamPtr              - pointer to the Re-Marking Entry going to be set.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
*                                       -------------------
*                                       |QoS param | Range |
* @retval |                        ------------------
*                                       | DSCP  | [0:63] |
*                                       | TC/UP  | [0:7] |
*                                       | EXP   | [0:7] |
*                                       -------------------
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on QoS parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerEgressQosRemarkingEntrySet
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT     remarkTableType,
    IN GT_U32                                       remarkParamValue,
    IN CPSS_DP_LEVEL_ENT                            confLevel,
    IN CPSS_DXCH_POLICER_QOS_PARAM_STC              *qosParamPtr
)
{
    GT_U32      hwData;     /* data will be written to HW */
    GT_U32      offset;     /* bit offset in the specific word of Entry */
    GT_U32      length;     /* length of bit field */
    GT_U32      wordNum;    /* word number inside the entry */
    GT_U32      entryIndex; /* entry index in the memory  */
    GT_U32      dpVal;      /* hw value of dp             */
    GT_STATUS   retStatus;  /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(qosParamPtr);


    /* Calculate entry index in case of correct input parameters. */
    switch(remarkTableType)
    {
        case CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_DSCP_E:
            if (remarkParamValue > 63)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            entryIndex = remarkParamValue;

            break;
        case CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_EXP_E:
            if (remarkParamValue > 7)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            entryIndex = 64 + remarkParamValue;
            break;
        case CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_TC_UP_E:
            if (remarkParamValue > 7)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            entryIndex = 72 + remarkParamValue;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((qosParamPtr->dscp > 63) || (qosParamPtr->up > 7) ||
       (qosParamPtr->exp > 7))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    switch (qosParamPtr->dp)
    {
        case CPSS_DP_GREEN_E:
            dpVal = 0;
            break;
        case CPSS_DP_YELLOW_E:
            dpVal = 1;
            break;
        case CPSS_DP_RED_E:
            dpVal = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Convert Re-Marking Entry data going to be set to the HW format */
    hwData = (qosParamPtr->dscp)| (dpVal << 6) | ((qosParamPtr->up) << 11) |
             ((qosParamPtr->exp) << 14);

    /************************/
    /* Set Re-Marking Entry */
    /************************/
    switch(confLevel)
    {
        case CPSS_DP_YELLOW_E:
            offset = 17;
            length = 15;
            wordNum = 0;

            /* write the 2 MSB bits of yellow conformance level */
            /* word 1, bits 0:1 */
            retStatus = prvCpssDxChWriteTableEntryField(
                                    devNum,
                                    CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E,
                                    entryIndex,
                                    1,  /* wordNum */
                                    0,  /* field offset*/
                                    2,  /* field length */
                                    (hwData >> 15));
            if (GT_OK != retStatus)
            {
                return retStatus;
            }
            break;
        case CPSS_DP_GREEN_E:
            wordNum = 0;
            offset = 0;
            length = 17;
            break;
        case CPSS_DP_RED_E:
            wordNum = 1;
            offset = 2;
            length = 17;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return  prvCpssDxChWriteTableEntryField(devNum,
                                            CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E,
                                            entryIndex,
                                            wordNum, /* wordNum */
                                            offset,  /* field offset*/
                                            length,  /* field length */
                                            hwData);
}

/**
* @internal cpssDxChPolicerEgressQosRemarkingEntrySet function
* @endinternal
*
* @brief   Sets Egress Policer Re-Marking table Entry.
*         Re-marking is the process of assigning new QoS parameters to the
*         packet, at the end of the metering process, based on the
*         Conformance Level the packet is assigned by metering process.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] remarkTableType          - Remark table type: DSCP, EXP or TC/UP.
* @param[in] remarkParamValue         - QoS parameter value.
* @param[in] confLevel                - Conformance Level: Green, Yellow, Red.
* @param[in] qosParamPtr              - pointer to the Re-Marking Entry going to be set.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
*                                       -------------------
*                                       |QoS param | Range |
* @retval |                        ------------------
*                                       | DSCP  | [0:63] |
*                                       | TC/UP  | [0:7] |
*                                       | EXP   | [0:7] |
*                                       -------------------
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on QoS parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEgressQosRemarkingEntrySet
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT     remarkTableType,
    IN GT_U32                                       remarkParamValue,
    IN CPSS_DP_LEVEL_ENT                            confLevel,
    IN CPSS_DXCH_POLICER_QOS_PARAM_STC              *qosParamPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerEgressQosRemarkingEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, remarkTableType, remarkParamValue, confLevel, qosParamPtr));

    rc = internal_cpssDxChPolicerEgressQosRemarkingEntrySet(devNum, remarkTableType, remarkParamValue, confLevel, qosParamPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, remarkTableType, remarkParamValue, confLevel, qosParamPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerEgressQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Egress Policer Re-Marking table Entry.
*         Re-marking is the process of assigning new QoS parameters to the
*         packet, at the end of the metering process, based on the
*         Conformance Level the packet is assigned by metering process.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] remarkTableType          - Remark table type: DSCP, EXP or TC/UP.
* @param[in] remarkParamValue         - QoS parameter value.
* @param[in] confLevel                - Conformance Level: Green, Yellow, Red.
*
* @param[out] qosParamPtr              - pointer to the requested Re-Marking Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
*                                       -------------------
*                                       |QoS param | Range |
* @retval |                        ------------------
*                                       | DSCP  | [0:63] |
*                                       | TC/UP  | [0:7] |
*                                       | EXP   | [0:7] |
*                                       -------------------
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
static GT_STATUS internal_cpssDxChPolicerEgressQosRemarkingEntryGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT     remarkTableType,
    IN  GT_U32                                      remarkParamValue,
    IN  CPSS_DP_LEVEL_ENT                           confLevel,
    OUT CPSS_DXCH_POLICER_QOS_PARAM_STC             *qosParamPtr
)
{
    GT_U32      hwData[2];  /* data will be written to HW */
    GT_U32      entryIndex; /* entry index in the memory */
    GT_U32      entryValue; /* Re-Marking Eentry value */
    GT_U32      dpVal;      /* hw value of dp           */
    GT_STATUS   retStatus;  /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(qosParamPtr);


    /* Calculate entry index in case of correct input parameters. */
    switch(remarkTableType)
    {
        case CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_DSCP_E:
            if (remarkParamValue > 63)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            entryIndex = remarkParamValue;

            break;
        case CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_EXP_E:
            if (remarkParamValue > 7)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            entryIndex = 64 + remarkParamValue;
            break;
        case CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_TC_UP_E:
            if (remarkParamValue > 7)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            entryIndex = 72 + remarkParamValue;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /************************/
    /* Get Re-Marking Entry */
    /************************/
    switch(confLevel)
    {
        case CPSS_DP_YELLOW_E:
            /* write the 2 MSB bits of yellow conformance level */
            /* word 1, bits 0:1 */
            retStatus = prvCpssDxChReadTableEntry(
                                    devNum,
                                    CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E,
                                    entryIndex,
                                    &hwData[0]);
            if (GT_OK != retStatus)
            {
                return retStatus;
            }
            entryValue = (hwData[0] >> 17) | ((hwData[1] & 0x3) << 15);
            break;

        case CPSS_DP_GREEN_E:
            retStatus = prvCpssDxChReadTableEntryField(
                                    devNum,
                                    CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E,
                                    entryIndex,
                                    0,  /* wordNum */
                                    0,  /* field offset*/
                                    17,  /* field length */
                                    &entryValue);
            if (GT_OK != retStatus)
            {
                return retStatus;
            }
            break;

        case CPSS_DP_RED_E:
            retStatus = prvCpssDxChReadTableEntryField(
                                    devNum,
                                    CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E,
                                    entryIndex,
                                    1,  /* wordNum */
                                    2,  /* field offset*/
                                    17,  /* field length */
                                    &entryValue);
            if (GT_OK != retStatus)
            {
                return retStatus;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Gather QoS parameters from the Re-Marking Entry */
    qosParamPtr->dscp   = entryValue & 0x3F;
    qosParamPtr->up     = (entryValue >> 11) & 0x7;
    qosParamPtr->exp    = (entryValue >> 14) & 0x7;
    dpVal               = (entryValue >> 6) & 0x3;

    switch (dpVal)
    {
        case 0:
            qosParamPtr->dp = CPSS_DP_GREEN_E;
            break;
        case 1:
            qosParamPtr->dp = CPSS_DP_YELLOW_E;
            break;
        case 2:
            qosParamPtr->dp = CPSS_DP_RED_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return  GT_OK;
}

/**
* @internal cpssDxChPolicerEgressQosRemarkingEntryGet function
* @endinternal
*
* @brief   Gets Egress Policer Re-Marking table Entry.
*         Re-marking is the process of assigning new QoS parameters to the
*         packet, at the end of the metering process, based on the
*         Conformance Level the packet is assigned by metering process.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] remarkTableType          - Remark table type: DSCP, EXP or TC/UP.
* @param[in] remarkParamValue         - QoS parameter value.
* @param[in] confLevel                - Conformance Level: Green, Yellow, Red.
*
* @param[out] qosParamPtr              - pointer to the requested Re-Marking Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, remarkTableType,
*                                       remarkParamValue or confLevel.
*                                       -------------------
*                                       |QoS param | Range |
* @retval |                        ------------------
*                                       | DSCP  | [0:63] |
*                                       | TC/UP  | [0:7] |
*                                       | EXP   | [0:7] |
*                                       -------------------
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
GT_STATUS cpssDxChPolicerEgressQosRemarkingEntryGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_REMARK_TABLE_TYPE_ENT     remarkTableType,
    IN  GT_U32                                      remarkParamValue,
    IN  CPSS_DP_LEVEL_ENT                           confLevel,
    OUT CPSS_DXCH_POLICER_QOS_PARAM_STC             *qosParamPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerEgressQosRemarkingEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, remarkTableType, remarkParamValue, confLevel, qosParamPtr));

    rc = internal_cpssDxChPolicerEgressQosRemarkingEntryGet(devNum, remarkTableType, remarkParamValue, confLevel, qosParamPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, remarkTableType, remarkParamValue, confLevel, qosParamPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerStageMeterModeSet function
* @endinternal
*
* @brief   Sets Policer Global stage mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - Policer meter mode: FLOW or PORT.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In FLOW mode both the Policy and the port-trigger commands are
*       considered and in case of contention, the Policy command takes
*       precedence.
*
*/
static GT_STATUS internal_cpssDxChPolicerStageMeterModeSet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT      mode
)
{
    GT_U32  hwData;     /* hw data value */
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    switch (mode)
    {
        case CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E:
            hwData = 0;
            break;
        case CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E:
            hwData = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    hierarchicalPolicerCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControl2Reg;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, hwData);
}

/**
* @internal cpssDxChPolicerStageMeterModeSet function
* @endinternal
*
* @brief   Sets Policer Global stage mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - Policer meter mode: FLOW or PORT.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In FLOW mode both the Policy and the port-trigger commands are
*       considered and in case of contention, the Policy command takes
*       precedence.
*
*/
GT_STATUS cpssDxChPolicerStageMeterModeSet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    IN  CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT      mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerStageMeterModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, mode));

    rc = internal_cpssDxChPolicerStageMeterModeSet(devNum, stage, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerStageMeterModeGet function
* @endinternal
*
* @brief   Gets Policer Global stage mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) Policer meter mode: FLOW or PORT.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In FLOW mode both the Policy and the port-trigger commands are
*       considered and in case of contention, the Policy command takes
*       precedence.
*
*/
static GT_STATUS internal_cpssDxChPolicerStageMeterModeGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT      *modePtr
)
{
    GT_U32      hwData;     /* hw data value */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   retStatus;  /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    hierarchicalPolicerCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControl2Reg;
    }

    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &hwData);

    *modePtr = (hwData == 0) ? CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E :
                               CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E;

    return retStatus;
}

/**
* @internal cpssDxChPolicerStageMeterModeGet function
* @endinternal
*
* @brief   Gets Policer Global stage mode.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) Policer meter mode: FLOW or PORT.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In FLOW mode both the Policy and the port-trigger commands are
*       considered and in case of contention, the Policy command takes
*       precedence.
*
*/
GT_STATUS cpssDxChPolicerStageMeterModeGet
(
    IN  GT_U8                                       devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT             stage,
    OUT CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT      *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerStageMeterModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, modePtr));

    rc = internal_cpssDxChPolicerStageMeterModeGet(devNum, stage, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerMeteringOnTrappedPktsEnableSet function
* @endinternal
*
* @brief   Enable/Disable Ingress metering for Trapped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] enable                   - GT_TRUE - policing is executed on all packets (data
*                                      and control).
*                                      GT_FALSE - policing is executed only on packets that
*                                      were asigned Forward (mirrored to analyzer
*                                      is inclusive) or Mirror to CPU commands by
*                                      previous processing stages.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The Ingress Policer considers data traffic as any packet that is not
*       trapped or dropped.
*
*/
static GT_STATUS internal_cpssDxChPolicerMeteringOnTrappedPktsEnableSet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN GT_BOOL                                  enable
)
{
    GT_U32      hwData;     /* hw data value */
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* Check that the stage is not Egress */
    if (stage > CPSS_DXCH_POLICER_STAGE_INGRESS_1_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    hierarchicalPolicerCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControl2Reg;
    }

    hwData = BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(devNum, regAddr, 1, 1, hwData);
}

/**
* @internal cpssDxChPolicerMeteringOnTrappedPktsEnableSet function
* @endinternal
*
* @brief   Enable/Disable Ingress metering for Trapped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] enable                   - GT_TRUE - policing is executed on all packets (data
*                                      and control).
*                                      GT_FALSE - policing is executed only on packets that
*                                      were asigned Forward (mirrored to analyzer
*                                      is inclusive) or Mirror to CPU commands by
*                                      previous processing stages.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The Ingress Policer considers data traffic as any packet that is not
*       trapped or dropped.
*
*/
GT_STATUS cpssDxChPolicerMeteringOnTrappedPktsEnableSet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN GT_BOOL                                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMeteringOnTrappedPktsEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enable));

    rc = internal_cpssDxChPolicerMeteringOnTrappedPktsEnableSet(devNum, stage, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerMeteringOnTrappedPktsEnableGet function
* @endinternal
*
* @brief   Get Ingress metering status (Enabled/Disabled) for Trapped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*
* @param[out] enablePtr                - GT_TRUE - policing is executed on all packets (data
*                                      and control).
*                                      GT_FALSE - policing is executed only on packets that
*                                      were asigned Forward (mirrored to analyzer
*                                      is inclusive) or Mirror to CPU commands by
*                                      previous processing stages.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The Ingress Policer considers data traffic as any packet that is not
*       trapped or dropped.
*
*/
static GT_STATUS internal_cpssDxChPolicerMeteringOnTrappedPktsEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      hwData;     /* hw data value */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   retStatus;  /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Check that the stage is not Egress */
    if (stage > CPSS_DXCH_POLICER_STAGE_INGRESS_1_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    hierarchicalPolicerCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControl2Reg;
    }

    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 1, 1, &hwData);

    *enablePtr = BIT2BOOL_MAC(hwData);

    return retStatus;
}

/**
* @internal cpssDxChPolicerMeteringOnTrappedPktsEnableGet function
* @endinternal
*
* @brief   Get Ingress metering status (Enabled/Disabled) for Trapped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
*
* @param[out] enablePtr                - GT_TRUE - policing is executed on all packets (data
*                                      and control).
*                                      GT_FALSE - policing is executed only on packets that
*                                      were asigned Forward (mirrored to analyzer
*                                      is inclusive) or Mirror to CPU commands by
*                                      previous processing stages.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The Ingress Policer considers data traffic as any packet that is not
*       trapped or dropped.
*
*/
GT_STATUS cpssDxChPolicerMeteringOnTrappedPktsEnableGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMeteringOnTrappedPktsEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enablePtr));

    rc = internal_cpssDxChPolicerMeteringOnTrappedPktsEnableGet(devNum, stage, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal prvCpssPortStormTypeOffsetAndRegisterAddressGet function
* @endinternal
*
* @brief   Get register address and field's offset for specific port
*         and storm type.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] localPort                - local port number (including the CPU port).
* @param[in] stormType                - storm type
*
* @param[out] fieldOffsetPtr           - (pointer to) field's offset
* @param[out] regAddrPtr               - (pointer to) register's address
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong stormType.
*/
static GT_STATUS prvCpssPortStormTypeOffsetAndRegisterAddressGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_U32                               localPort,
    IN CPSS_DXCH_POLICER_STORM_TYPE_ENT     stormType,
    OUT GT_U32                             *fieldOffsetPtr,
    OUT GT_U32                             *regAddrPtr
)
{
    GT_U32  numPortsPerRegister;/* num ports per register */

    /* check storm type and calculate bitoffset */
    switch (stormType)
    {
        case CPSS_DXCH_POLICER_STORM_TYPE_UC_KNOWN_E:
            *fieldOffsetPtr = 0;
            break;
        case CPSS_DXCH_POLICER_STORM_TYPE_UC_UNKNOWN_E:
            *fieldOffsetPtr = 2;
            break;
        case CPSS_DXCH_POLICER_STORM_TYPE_MC_REGISTERED_E:
            *fieldOffsetPtr = 4;
            break;
        case CPSS_DXCH_POLICER_STORM_TYPE_MC_UNREGISTERED_E:
            *fieldOffsetPtr = 6;
            break;
        case CPSS_DXCH_POLICER_STORM_TYPE_BC_E:
            *fieldOffsetPtr = 8;
            break;
        case CPSS_DXCH_POLICER_STORM_TYPE_TCP_SYN_E:
            *fieldOffsetPtr = 10;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        numPortsPerRegister = 1;
    }
    else
    {
        numPortsPerRegister = 2;
        /* each register stores values for 2 ports.
           Even ports' values start from bit#0.
           Odd  ports' values start from bit#12. */
        *fieldOffsetPtr += (12 * (localPort & 1));
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        *regAddrPtr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                            portAndPktTypeTranslationTable[(localPort / numPortsPerRegister)];
    }
    else
    {
        *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                PLR[stage].portMeteringPtrIndexReg + ((localPort >> 1) * 0x4);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPolicerPortStormTypeIndexSet function
* @endinternal
*
* @brief   Associates policing profile with source/target port and storm rate type.
*         There are up to four policers per port and every storm type can be
*         flexibly associated with any of the four policers.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] stormType                - storm type
* @param[in] index                    - policer  (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, stormType ot port.
* @retval GT_OUT_OF_RANGE          - on out of range of index.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerPortStormTypeIndexSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN CPSS_DXCH_POLICER_STORM_TYPE_ENT     stormType,
    IN GT_U32                               index
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      bitOffset;  /* bit offset inside register */
    GT_U32      portGroupId;     /* the port group Id - support multi-port-groups device */
    GT_U32      localPort;  /* local port - support multi-port-groups device */
    GT_STATUS   rc;         /* return status */
    CPSS_DXCH_TABLE_ENT tableType; /* table for direct read/write   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* Global physical port is used */
        portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        localPort = portNum;
    }
    else
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
    }

    /* check policing profile index */
    if (index > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        tableType = stage + CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E;
        SIP6_STORM_TYPE_TO_START_BIT_CONVERT_MAC(stormType,bitOffset);

        return prvCpssDxChWriteTableEntryField(devNum,tableType,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            bitOffset,2,index);
    }

    /* calculate register's address and position within register */
    rc = prvCpssPortStormTypeOffsetAndRegisterAddressGet(devNum, stage,
                                                         localPort, stormType,
                                                         &bitOffset, &regAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,regAddr, bitOffset, 2, index);
}

/**
* @internal cpssDxChPolicerPortStormTypeIndexSet function
* @endinternal
*
* @brief   Associates policing profile with source/target port and storm rate type.
*         There are up to four policers per port and every storm type can be
*         flexibly associated with any of the four policers.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] stormType                - storm type
* @param[in] index                    - policer  (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, stormType ot port.
* @retval GT_OUT_OF_RANGE          - on out of range of index.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortStormTypeIndexSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN CPSS_DXCH_POLICER_STORM_TYPE_ENT     stormType,
    IN GT_U32                               index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortStormTypeIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, portNum, stormType, index));

    rc = internal_cpssDxChPolicerPortStormTypeIndexSet(devNum, stage, portNum, stormType, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, portNum, stormType, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPolicerPortStormTypeIndexGet function
* @endinternal
*
* @brief   Get policing profile for given source/target port and storm rate type.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] stormType                - storm type
*
* @param[out] indexPtr                 - pointer to policer index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, stormType or portNum.
* @retval GT_OUT_OF_RANGE          - on out of range of port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerPortStormTypeIndexGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_POLICER_STORM_TYPE_ENT    stormType,
    OUT GT_U32                              *indexPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      bitOffset;  /* bit offset inside register */
    GT_U32      portGroupId;     /*the port group Id - support multi-port-groups device */
    GT_U32      localPort;  /* local port - support multi-port-groups device */
    GT_STATUS   rc;         /* return status */
    CPSS_DXCH_TABLE_ENT tableType; /* table for direct read/write   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* Global physical port is used */
        portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        localPort = portNum;
    }
    else
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        tableType = stage + CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E;
        SIP6_STORM_TYPE_TO_START_BIT_CONVERT_MAC(stormType,bitOffset);

        return prvCpssDxChReadTableEntryField(devNum,tableType,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            bitOffset,2,indexPtr);
    }

    /* calculate register's address and position within register */
    rc = prvCpssPortStormTypeOffsetAndRegisterAddressGet(devNum, stage,
                                                         localPort, stormType,
                                                         &bitOffset, &regAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,regAddr, bitOffset, 2, indexPtr);
}

/**
* @internal cpssDxChPolicerPortStormTypeIndexGet function
* @endinternal
*
* @brief   Get policing profile for given source/target port and storm rate type.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] portNum                  - port number (including the CPU port).
* @param[in] stormType                - storm type
*
* @param[out] indexPtr                 - pointer to policer index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, stormType or portNum.
* @retval GT_OUT_OF_RANGE          - on out of range of port.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortStormTypeIndexGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_POLICER_STORM_TYPE_ENT    stormType,
    OUT GT_U32                              *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortStormTypeIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, portNum, stormType, indexPtr));

    rc = internal_cpssDxChPolicerPortStormTypeIndexGet(devNum, stage, portNum, stormType, indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, portNum, stormType, indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPolicerEgressL2RemarkModelSet function
* @endinternal
*
* @brief   Sets Egress Policer L2 packets remarking model.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] model                    - L2 packets remarking model. It defines QoS parameter,
*                                      which used as index in the Qos table:
*                                      User Priority or Traffic Class.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerEgressL2RemarkModelSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT    model
)
{
    GT_U32      hwData;     /* hw data value */
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    switch (model)
    {
        case CPSS_DXCH_POLICER_L2_REMARK_MODEL_TC_E:
            hwData = 0;
            break;
        case CPSS_DXCH_POLICER_L2_REMARK_MODEL_UP_E:
            hwData = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,2).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[2].policerControlReg;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, 18, 1, hwData);
}

/**
* @internal cpssDxChPolicerEgressL2RemarkModelSet function
* @endinternal
*
* @brief   Sets Egress Policer L2 packets remarking model.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] model                    - L2 packets remarking model. It defines QoS parameter,
*                                      which used as index in the Qos table:
*                                      User Priority or Traffic Class.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEgressL2RemarkModelSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT    model
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerEgressL2RemarkModelSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, model));

    rc = internal_cpssDxChPolicerEgressL2RemarkModelSet(devNum, model);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, model));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerEgressL2RemarkModelGet function
* @endinternal
*
* @brief   Gets Egress Policer L2 packets remarking model.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] modelPtr                 - L2 packets remarking model. It defines QoS parameter,
*                                      which used as index in the Qos table:
*                                      User Priority or Traffic Class.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerEgressL2RemarkModelGet
(
    IN  GT_U8                                    devNum,
    OUT CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT    *modelPtr
)
{
    GT_U32      hwData;     /* hw data value */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   retStatus;  /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(modelPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,2).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[2].policerControlReg;
    }

    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 18, 1, &hwData);

    switch (hwData)
    {
        case 0:
            *modelPtr = CPSS_DXCH_POLICER_L2_REMARK_MODEL_TC_E;
            break;
        case 1:
            *modelPtr = CPSS_DXCH_POLICER_L2_REMARK_MODEL_UP_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return retStatus;
}

/**
* @internal cpssDxChPolicerEgressL2RemarkModelGet function
* @endinternal
*
* @brief   Gets Egress Policer L2 packets remarking model.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] modelPtr                 - L2 packets remarking model. It defines QoS parameter,
*                                      which used as index in the Qos table:
*                                      User Priority or Traffic Class.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEgressL2RemarkModelGet
(
    IN  GT_U8                                    devNum,
    OUT CPSS_DXCH_POLICER_L2_REMARK_MODEL_ENT    *modelPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerEgressL2RemarkModelGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modelPtr));

    rc = internal_cpssDxChPolicerEgressL2RemarkModelGet(devNum, modelPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modelPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerEgressQosUpdateEnableSet function
* @endinternal
*
* @brief   The function enables or disables QoS remarking of conforming packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - Enable/disable Qos update for conforming packets:
*                                      GT_TRUE  - Remark Qos parameters of conforming packets.
*                                      GT_FALSE - Keep incoming Qos parameters of conforming packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerEgressQosUpdateEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_U32      hwData;     /* hw data value */
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,2).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[2].policerControlReg;
    }

    hwData = BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(devNum, regAddr, 31, 1, hwData);
}

/**
* @internal cpssDxChPolicerEgressQosUpdateEnableSet function
* @endinternal
*
* @brief   The function enables or disables QoS remarking of conforming packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] enable                   - Enable/disable Qos update for conforming packets:
*                                      GT_TRUE  - Remark Qos parameters of conforming packets.
*                                      GT_FALSE - Keep incoming Qos parameters of conforming packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEgressQosUpdateEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerEgressQosUpdateEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPolicerEgressQosUpdateEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerEgressQosUpdateEnableGet function
* @endinternal
*
* @brief   The function get QoS remarking status of conforming packets
*         (enable/disable).
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - Enable/disable Qos update for conforming packets:
*                                      GT_TRUE  - Remark Qos parameters of conforming packets.
*                                      GT_FALSE - Keep incoming Qos parameters of conforming
*                                      packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerEgressQosUpdateEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32      hwData;     /* hw data value */
    GT_U32      regAddr;    /* register address */
    GT_STATUS   retStatus;  /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,2).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[2].policerControlReg;
    }

    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 31, 1, &hwData);

    *enablePtr = BIT2BOOL_MAC(hwData);

    return retStatus;
}

/**
* @internal cpssDxChPolicerEgressQosUpdateEnableGet function
* @endinternal
*
* @brief   The function get QoS remarking status of conforming packets
*         (enable/disable).
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
*
* @param[out] enablePtr                - Enable/disable Qos update for conforming packets:
*                                      GT_TRUE  - Remark Qos parameters of conforming packets.
*                                      GT_FALSE - Keep incoming Qos parameters of conforming
*                                      packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerEgressQosUpdateEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerEgressQosUpdateEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPolicerEgressQosUpdateEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPolicerCountingEntryWordSet function
* @endinternal
*
* @brief   Sets particular word of the counting entry
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
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
* @param[in] entryIndex               - entry index.
* @param[in] entryWord                - entry's word number.
* @param[in] cntrValue                - counter's value.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, entryIndex, entryWord
*                                       or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPolicerCountingEntryWordSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex,
    IN  GT_U32                              entryWord,
    IN  GT_U32                              cntrValue
)
{
    GT_U32  wordAddr;       /* counting entry's word address */
    GT_U32  wordData;       /* counting entry's word data */
    GT_U32  word;           /* counting entry's word */
    GT_U32  portGroupId;    /* port group id */
    GT_U32  regAddr;        /* register address */
    GT_STATUS rc;           /* return status */

    /* check counting entry index and word number */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum, stage, entryIndex);
    if (entryWord >= PRV_CPSS_DXCH_POLICER_COUNT_ENTRY_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Check if Policer Counting memory errata is enabled */

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
             PRV_CPSS_DXCH_LION_POLICER_COUNTING_MEM_ACCESS_WA_E))
    {
        /* Indirect access to counting memory should be used */

        /* Get Ingress Policer Table Access Data 0 Register Address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        PLR[stage].policerTblAccessDataReg;

        /* First read the entry on each portGroup from the bitmap */
        /* loop on all active port groups in the bmp */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
        {
            /* Perform indirect access to the Policer Table */
            rc = prvCpssDxCh3PolicerInternalTableAccess(
                                    devNum,
                                    portGroupId,
                                    stage,
                                    entryIndex,
                                    PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E);
            if (GT_OK != rc)
            {
                return rc;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

        /* Update the desired word in access data registers */
        wordAddr = (regAddr + (entryWord * 4));

        /* set new value of counter */
        rc = prvCpssPortGroupsBmpCounterSet(devNum, portGroupsBmp,
                                                wordAddr, 0,32, cntrValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* In the last word there is a bit that is responsible for
           Billing/IPFIX, it should be set to Billing and not IPFIX
           (the bit should be cleared).
           VLAN and Policy counters do not use the last word for counting */
        wordAddr = regAddr + 0x1C;

        /* loop on all active port groups in the bmp */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
        {
            rc = prvCpssHwPpPortGroupWriteRegister(devNum,
                                                      portGroupId,
                                                      wordAddr,
                                                      0x0);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Trigger the read & write action */
            rc = prvCpssDxCh3PolicerInternalTableAccess(
                                devNum,
                                portGroupId,
                                stage,
                                entryIndex,
                                PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E);
            if (rc != GT_OK)
            {
                return rc;
            }

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    }
    else
    {
        /* Direct access to counting memory */
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            wordAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCountingTbl;
        }
        else
        {
            wordAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].policerCountingTbl;
        }

        wordAddr += (entryIndex * METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS);

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            /* SIP 5_15 and above devices use same internal buffer for read/write memories.
               Each word read access loads whole entry in to a buffer.
               Write of any word but last one updates data in buffer.
               Write to last word writes context of buffer into memory.
               The function should read entry only once before update of targed word.
               After that need to update targed word and write last one to trigger memory update. */

            /* loop on all active port groups in the bmp */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
            {

                /* just read one word. This operation loads whole entry in internal buffer. */
                rc = prvCpssHwPpPortGroupReadRegister(devNum,
                                                         portGroupId,
                                                         wordAddr,
                                                         &wordData);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
        }

        /* The device may write only whole counting memory entry.
           The device triggers write by the last word of the counting entry.
           Copy all words of entry and set given word. */
        for (word = 0; word < PRV_CPSS_DXCH_POLICER_COUNT_ENTRY_SIZE_CNS; word++)
        {
            /* verify that all words were written before write last one.
               because last one triggers write of whole entry */
            if (word == (PRV_CPSS_DXCH_POLICER_COUNT_ENTRY_SIZE_CNS - 1) )
            {
                GT_SYNC;
            }

            if (word == entryWord)
            {
                /* set new value of counter */
                rc = prvCpssPortGroupsBmpCounterSet(devNum, portGroupsBmp,
                                                        wordAddr, 0,32, cntrValue);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                /* Check if it the last word, if yes, clear it.
                   In the last word there is a bit that is responsible for
                   Billing/IPFIX, it should be set to Billing and not IPFIX
                   (the bit should be cleared).
                   VLAN and Policy counters do not use the last word for counting */
                if (word == (PRV_CPSS_DXCH_POLICER_COUNT_ENTRY_SIZE_CNS - 1))
                {

                    /* loop on all active port groups in the bmp */
                    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
                    {
                        rc = prvCpssHwPpPortGroupWriteRegister(devNum,
                                                                  portGroupId,
                                                                  wordAddr,
                                                                  0x0);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
                }
                else
                {
                    if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                    {
                        /* loop on all active port groups in the bmp */
                        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
                        {

                            /* just read word and write it back */
                            rc = prvCpssHwPpPortGroupReadRegister(devNum,
                                                                     portGroupId,
                                                                     wordAddr,
                                                                     &wordData);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }

                            rc = prvCpssHwPpPortGroupWriteRegister(devNum,
                                                                      portGroupId,
                                                                      wordAddr,
                                                                      wordData);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }
                        }
                        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
                    }
                }
            }

            /* increment word's address */
            wordAddr += 4;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPolicerVlanCntrSet function
* @endinternal
*
* @brief   Sets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
*         When working in VLAN Counting Mode each counter counts the number of
*         packets or bytes according to the VLAN counters mode set
*         by cpssDxChPolicerVlanCountingModeSet(). Index of counter is VLAN ID.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] vid                      - VLAN ID. Bits [1:0] of VID are used to select the
*                                      relevant 32 bit counter inside the Counting Entry.
* @param[in] cntrValue                - Packet/Byte counter indexed by the packet's VID.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled VLAN Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or vid (VID range is limited by max
*                                       number of Policer Counting Entries) or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The VLAN counters are free-running no-sticky counters.
*       In order to set Vlan counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
static GT_STATUS internal_cpssDxChPolicerVlanCntrSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U16                              vid,
    IN  GT_U32                              cntrValue
)
{
    GT_U32  countNumber;    /* the number of counter in the Entry */
    GT_U32  entryIndex;     /* entry index in the Internal Table */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E |
            CPSS_BOBCAT3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* each counting entry has 4 VLAN counters */
    entryIndex = (vid >> 2);
    countNumber = (vid & 0x3);

    /* Set counters value */
    return prvCpssDxChPolicerCountingEntryWordSet(
                                            devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            stage, entryIndex, countNumber,
                                            cntrValue);
}

/**
* @internal cpssDxChPolicerVlanCntrSet function
* @endinternal
*
* @brief   Sets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
*         When working in VLAN Counting Mode each counter counts the number of
*         packets or bytes according to the VLAN counters mode set
*         by cpssDxChPolicerVlanCountingModeSet(). Index of counter is VLAN ID.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] vid                      - VLAN ID. Bits [1:0] of VID are used to select the
*                                      relevant 32 bit counter inside the Counting Entry.
* @param[in] cntrValue                - Packet/Byte counter indexed by the packet's VID.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled VLAN Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or vid (VID range is limited by max
*                                       number of Policer Counting Entries) or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The VLAN counters are free-running no-sticky counters.
*       In order to set Vlan counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
GT_STATUS cpssDxChPolicerVlanCntrSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U16                              vid,
    IN  GT_U32                              cntrValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerVlanCntrSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, vid, cntrValue));

    rc = internal_cpssDxChPolicerVlanCntrSet(devNum, stage, vid, cntrValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, vid, cntrValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerVlanCntrGet function
* @endinternal
*
* @brief   Gets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
*         When working in VLAN Counting Mode each counter counts the number of
*         packets or bytes according to the VLAN counters mode set
*         by cpssDxChPolicerVlanCountingModeSet(). Index of counter is VLAN ID.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] vid                      - VLAN ID. Bits [1:0] of VID are used to select the
*                                      relevant 32 bit counter inside the Counting Entry.
*
* @param[out] cntrValuePtr             - Packet/Byte counter indexed by the packet's VID.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled VLAN Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or vid (VID range is limited by max
*                                       number of Policer Counting Entries) or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The VLAN counters are free-running no-sticky counters.
*
*/
static GT_STATUS internal_cpssDxChPolicerVlanCntrGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U16                              vid,
    OUT GT_U32                              *cntrValuePtr
)
{
    GT_U32  regAddr;        /* register address                     */
    GT_U32  countNumber;    /* the number of counter in the Entry   */
    GT_U32  entryIndex;     /* entry index in the Internal Table    */
    GT_STATUS rc;           /* return code                          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E |
            CPSS_BOBCAT3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);

    entryIndex = (vid >> 2);

    /* check allocated number of counters - 4 VLAN counters per Counting Vlan Entry */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum, stage, entryIndex);


    /* Check if Policer Counting memory errata is enabled */
    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                          PRV_CPSS_DXCH_LION_POLICER_COUNTING_MEM_ACCESS_WA_E))
    {
        /* Trigger Indirect Read from Counting memory */
        rc = prvCpssDxCh3PolicerInternalTableAccess(devNum,
                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                              stage,
                              entryIndex,
                              PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Get Ingress Policer Table Access Data 0 Register Address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        PLR[stage].policerTblAccessDataReg;
    }
    else
    {
        /* Direct access to counting memory */
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCountingTbl;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].policerCountingTbl;
        }

        regAddr += (entryIndex * METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS);
    }

    countNumber = (vid & 0x3);

    /* calculate the word address inside the table */
    regAddr += (countNumber * 0x4);


    /* Get counters value */
    return prvCpssPortGroupsCounterSummary(devNum, regAddr,0,32, cntrValuePtr,NULL);
}

/**
* @internal cpssDxChPolicerVlanCntrGet function
* @endinternal
*
* @brief   Gets Policer VLAN Counters located in the Policer Counting Entry upon
*         enabled VLAN counting mode.
*         When working in VLAN Counting Mode each counter counts the number of
*         packets or bytes according to the VLAN counters mode set
*         by cpssDxChPolicerVlanCountingModeSet(). Index of counter is VLAN ID.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] vid                      - VLAN ID. Bits [1:0] of VID are used to select the
*                                      relevant 32 bit counter inside the Counting Entry.
*
* @param[out] cntrValuePtr             - Packet/Byte counter indexed by the packet's VID.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled VLAN Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or vid (VID range is limited by max
*                                       number of Policer Counting Entries) or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The VLAN counters are free-running no-sticky counters.
*
*/
GT_STATUS cpssDxChPolicerVlanCntrGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U16                              vid,
    OUT GT_U32                              *cntrValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerVlanCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, vid, cntrValuePtr));

    rc = internal_cpssDxChPolicerVlanCntrGet(devNum, stage, vid, cntrValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, vid, cntrValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerVlanCountingPacketCmdTriggerSet function
* @endinternal
*
* @brief   Enables or disables VLAN counting triggering according to the
*         specified packet command.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] cmdTrigger               - The packet command trigger.
*                                      The following packet command are supported:
*                                      Forward (CPSS_PACKET_CMD_FORWARD_E),
*                                      Hard Drop (CPSS_PACKET_CMD_DROP_HARD_E),
*                                      Soft Drop (CPSS_PACKET_CMD_DROP_SOFT_E),
*                                      Trap (CPSS_PACKET_CMD_TRAP_TO_CPU_E),
*                                      Mirror to CPU (CPSS_PACKET_CMD_MIRROR_TO_CPU_E).
* @param[in] enable                   - Enable/Disable VLAN Counting according to the packet
*                                      command trigger:
*                                      GT_TRUE  - Enable VLAN counting on packet with
*                                      packet command trigger.
*                                      GT_FALSE - Disable VLAN counting on packet with
*                                      packet command trigger.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported egress direction
*
* @note All packet commands are enabled in the HW by default.
*       For egress policer the default trigger values shouldn't be changed.
*
*/
static GT_STATUS internal_cpssDxChPolicerVlanCountingPacketCmdTriggerSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_PACKET_CMD_ENT                  cmdTrigger,
    IN GT_BOOL                              enable
)
{
    GT_U32      hwData;     /* hw data value */
    GT_U32      regAddr;    /* register address */
    GT_U32      bitOffset;  /* bit offset inside register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* Check that the stage is Ingress */
    if (!PRV_CPSS_DXCH_POLICER_INGRESS_STAGE_CHECK_MAC(stage))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check vlan command trigger and calculate bit offset */
    switch (cmdTrigger)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            bitOffset = 25;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            bitOffset = 26;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            bitOffset = 27;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            bitOffset = 28;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    hwData = BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(devNum, regAddr, bitOffset, 1, hwData);
}

/**
* @internal cpssDxChPolicerVlanCountingPacketCmdTriggerSet function
* @endinternal
*
* @brief   Enables or disables VLAN counting triggering according to the
*         specified packet command.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] cmdTrigger               - The packet command trigger.
*                                      The following packet command are supported:
*                                      Forward (CPSS_PACKET_CMD_FORWARD_E),
*                                      Hard Drop (CPSS_PACKET_CMD_DROP_HARD_E),
*                                      Soft Drop (CPSS_PACKET_CMD_DROP_SOFT_E),
*                                      Trap (CPSS_PACKET_CMD_TRAP_TO_CPU_E),
*                                      Mirror to CPU (CPSS_PACKET_CMD_MIRROR_TO_CPU_E).
* @param[in] enable                   - Enable/Disable VLAN Counting according to the packet
*                                      command trigger:
*                                      GT_TRUE  - Enable VLAN counting on packet with
*                                      packet command trigger.
*                                      GT_FALSE - Disable VLAN counting on packet with
*                                      packet command trigger.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported egress direction
*
* @note All packet commands are enabled in the HW by default.
*       For egress policer the default trigger values shouldn't be changed.
*
*/
GT_STATUS cpssDxChPolicerVlanCountingPacketCmdTriggerSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN CPSS_PACKET_CMD_ENT                  cmdTrigger,
    IN GT_BOOL                              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerVlanCountingPacketCmdTriggerSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, cmdTrigger, enable));

    rc = internal_cpssDxChPolicerVlanCountingPacketCmdTriggerSet(devNum, stage, cmdTrigger, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, cmdTrigger, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerVlanCountingPacketCmdTriggerGet function
* @endinternal
*
* @brief   Gets VLAN counting triggering status (Enable/Disable) according to the
*         specified packet command.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] cmdTrigger               - The packet command trigger.
*                                      The following packetcommand are supported:
*                                      Forward (CPSS_PACKET_CMD_FORWARD_E),
*                                      Hard Drop (CPSS_PACKET_CMD_DROP_HARD_E),
*                                      Soft Drop (CPSS_PACKET_CMD_DROP_SOFT_E),
*                                      Trap (CPSS_PACKET_CMD_TRAP_TO_CPU_E),
*                                      Mirror to CPU (CPSS_PACKET_CMD_MIRROR_TO_CPU_E).
*
* @param[out] enablePtr                - Pointer to Enable/Disable VLAN Counting according
*                                      to the packet command trigger:
*                                      GT_TRUE  - Enable VLAN counting on packet with
*                                      triggered packet command.
*                                      GT_FALSE - Disable VLAN counting on packet with
*                                      triggered packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All packet commands are enabled in the HW by default.
*
*/
static GT_STATUS internal_cpssDxChPolicerVlanCountingPacketCmdTriggerGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  CPSS_PACKET_CMD_ENT                 cmdTrigger,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      hwData;     /* hw data value                */
    GT_U32      regAddr;    /* register address             */
    GT_U32      bitOffset;  /* bit offset inside register   */
    GT_STATUS   retStatus;  /* return code                  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Check that the stage is Ingress */
    if (!PRV_CPSS_DXCH_POLICER_INGRESS_STAGE_CHECK_MAC(stage))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check vlan command trigger and calculate bit offset */
    switch (cmdTrigger)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            bitOffset = 25;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            bitOffset = 26;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            bitOffset = 27;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            bitOffset = 28;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, bitOffset, 1, &hwData);


    *enablePtr = BIT2BOOL_MAC(hwData);

    return retStatus;
}

/**
* @internal cpssDxChPolicerVlanCountingPacketCmdTriggerGet function
* @endinternal
*
* @brief   Gets VLAN counting triggering status (Enable/Disable) according to the
*         specified packet command.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0 or Ingress #1.
* @param[in] cmdTrigger               - The packet command trigger.
*                                      The following packetcommand are supported:
*                                      Forward (CPSS_PACKET_CMD_FORWARD_E),
*                                      Hard Drop (CPSS_PACKET_CMD_DROP_HARD_E),
*                                      Soft Drop (CPSS_PACKET_CMD_DROP_SOFT_E),
*                                      Trap (CPSS_PACKET_CMD_TRAP_TO_CPU_E),
*                                      Mirror to CPU (CPSS_PACKET_CMD_MIRROR_TO_CPU_E).
*
* @param[out] enablePtr                - Pointer to Enable/Disable VLAN Counting according
*                                      to the packet command trigger:
*                                      GT_TRUE  - Enable VLAN counting on packet with
*                                      triggered packet command.
*                                      GT_FALSE - Disable VLAN counting on packet with
*                                      triggered packet command.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All packet commands are enabled in the HW by default.
*
*/
GT_STATUS cpssDxChPolicerVlanCountingPacketCmdTriggerGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  CPSS_PACKET_CMD_ENT                 cmdTrigger,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerVlanCountingPacketCmdTriggerGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, cmdTrigger, enablePtr));

    rc = internal_cpssDxChPolicerVlanCountingPacketCmdTriggerGet(devNum, stage, cmdTrigger, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, cmdTrigger, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerVlanCountingModeSet function
* @endinternal
*
* @brief   Sets the Policer VLAN counters mode as byte or packet based.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - VLAN counters mode: Byte or Packet based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerVlanCountingModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT     mode
)
{
    GT_U32      hwData;     /* hw data value                */
    GT_U32      regAddr;    /* register address             */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    /* Convert VLAN counters mode to the Reg Value */
    switch(mode)
    {
        case CPSS_DXCH_POLICER_VLAN_CNTR_MODE_BYTES_E:
            hwData = 1;
            break;
        case CPSS_DXCH_POLICER_VLAN_CNTR_MODE_PACKETS_E:
            hwData = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    /* Set Policer Control0 register, <VlanCountingMode> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 3, 1, hwData);
}

/**
* @internal cpssDxChPolicerVlanCountingModeSet function
* @endinternal
*
* @brief   Sets the Policer VLAN counters mode as byte or packet based.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - VLAN counters mode: Byte or Packet based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerVlanCountingModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT     mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerVlanCountingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, mode));

    rc = internal_cpssDxChPolicerVlanCountingModeSet(devNum, stage, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerVlanCountingModeGet function
* @endinternal
*
* @brief   Gets the Policer VLAN counters mode as byte or packet based.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) VLAN counters mode: Byte or Packet based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerVlanCountingModeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT     *modePtr
)
{
    GT_U32      hwData;     /* hw data value                */
    GT_U32      regAddr;    /* register address             */
    GT_STATUS   retStatus;  /* return code                  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControlReg;
    }

    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 3, 1, &hwData);

    *modePtr = (hwData == 0) ? CPSS_DXCH_POLICER_VLAN_CNTR_MODE_PACKETS_E:
                                CPSS_DXCH_POLICER_VLAN_CNTR_MODE_BYTES_E;

    return retStatus;
}

/**
* @internal cpssDxChPolicerVlanCountingModeGet function
* @endinternal
*
* @brief   Gets the Policer VLAN counters mode as byte or packet based.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) VLAN counters mode: Byte or Packet based.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerVlanCountingModeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_VLAN_CNTR_MODE_ENT     *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerVlanCountingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, modePtr));

    rc = internal_cpssDxChPolicerVlanCountingModeGet(devNum, stage, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPolicyCntrSet function
* @endinternal
*
* @brief   Sets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
* @param[in] cntrValue                - packets counter.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index (index range is limited by
*                                       max number of Policer Policy counters).
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*       In order to set Policy counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
static GT_STATUS internal_cpssDxChPolicerPolicyCntrSet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    IN  GT_U32                              cntrValue
)
{
    return cpssDxChPolicerPortGroupPolicyCntrSet(
                                            devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            stage,
                                            index,
                                            cntrValue);
}

/**
* @internal cpssDxChPolicerPolicyCntrSet function
* @endinternal
*
* @brief   Sets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
* @param[in] cntrValue                - packets counter.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index (index range is limited by
*                                       max number of Policer Policy counters).
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*       In order to set Policy counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
GT_STATUS cpssDxChPolicerPolicyCntrSet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    IN  GT_U32                              cntrValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPolicyCntrSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, index, cntrValue));

    rc = internal_cpssDxChPolicerPolicyCntrSet(devNum, stage, index, cntrValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, index, cntrValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPolicyCntrGet function
* @endinternal
*
* @brief   Gets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
*
* @param[out] cntrValuePtr             - Pointer to the packet counter.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*
*/
static GT_STATUS internal_cpssDxChPolicerPolicyCntrGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    OUT GT_U32                              *cntrValuePtr
)
{
    return cpssDxChPolicerPortGroupPolicyCntrGet(
                                        devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        stage,
                                        index,
                                        cntrValuePtr);
}

/**
* @internal cpssDxChPolicerPolicyCntrGet function
* @endinternal
*
* @brief   Gets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
*
* @param[out] cntrValuePtr             - Pointer to the packet counter.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*
*/
GT_STATUS cpssDxChPolicerPolicyCntrGet
(
    IN  GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    OUT GT_U32                              *cntrValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPolicyCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, index, cntrValuePtr));

    rc = internal_cpssDxChPolicerPolicyCntrGet(devNum, stage, index, cntrValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, index, cntrValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

#define CASE_PLR_PARTITION_SIZE_GET(/*IN*/minSize,/*IN*/ingressPolicersNum,/*OUT*/iplr0MemSize,/*OUT*/iplr1MemSize)\
            case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E:  \
                iplr1MemSize = MIN(minSize, ingressPolicersNum);              \
                iplr0MemSize = ingressPolicersNum - iplr1MemSize;             \
                break;                                                        \
            case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E:   \
                iplr1MemSize = 0;                                             \
                iplr0MemSize = ingressPolicersNum;                            \
                break;                                                        \
            case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E:   \
                iplr0MemSize = 0;                                             \
                iplr1MemSize = ingressPolicersNum;                            \
                break;                                                        \
            case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E:  \
                iplr0MemSize = MIN(minSize, ingressPolicersNum);              \
                iplr1MemSize = ingressPolicersNum - iplr0MemSize;             \
                break;                                                        \
            case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_4_E:                      \
                iplr0MemSize = MIN(ram0Size, ingressPolicersNum);             \
                iplr1MemSize = ingressPolicersNum - iplr0MemSize;             \
                break;                                                        \
            case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E:                      \
                iplr1MemSize = MIN(ram0Size, ingressPolicersNum);             \
                iplr0MemSize = ingressPolicersNum - iplr1MemSize;             \
                break


/**
* @internal sip5_20policerMemoryPartitionsInfoGet function
* @endinternal
*
* @brief   Sip5.20 : Get Policer partitions info.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     - The  in which internal tables are shared.
*                                      relevant only to 'write'
* @param[in] isWriteOperation         - is the info needed for 'write' operation or for read operation.
*                                      GT_TRUE  - write operation
*                                      GT_FALSE - read operation
* @param[in] numEntriesIngressStage0  - The number of metering/billing counters for
*                                      ingress policer stage 0
*                                      relevant only for 'write' operation with
* @param[in] mode                     == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*                                      (APPLICABLE DEVICES Bobcat3)
* @param[in] numEntriesIngressStage1  - The number of metering/billing counters for
*                                      ingress policer stage 1
*                                      relevant only for 'write' operation with
* @param[in] mode                     == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon)
*                                      NOTE: the egress policer gets the 'rest' of the metering/billing counters
*                                      the that device support.
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*
* @param[out] iplr0MemSizePtr          - pointer to the size of iplr0
* @param[out] iplr1MemSizePtr          - pointer to the size of iplr1
* @param[out] modePtr                  - pointer to the 'mode' of the partitions
*                                      relevant only to 'read'
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip5_20policerMemoryPartitionsInfoGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT      mode,
    IN  GT_BOOL                             isWriteOperation,
    IN  GT_U32                              numEntriesIngressStage0,
    IN  GT_U32                              numEntriesIngressStage1,
    OUT GT_U32                              *iplr0MemSizePtr,
    OUT GT_U32                              *iplr1MemSizePtr,
    OUT CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT      *modePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;        /* register address                  */
    GT_U32      ram0Size;       /* fake ram0 size  */
    GT_U32      minSize;
    GT_U32      ingressPolicersNum;/* number of ingress policers (meters/counter)*/
    GT_U32      tmpMemSize;
    GT_U32      iplr0MemSize;   /* IPLR 0 memory size                */
    GT_U32      iplr1MemSize;   /* IPLR 1 memory size                */
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT  fakeHwMode;
    GT_U32      check_iplr0MemSize;   /* check : IPLR 0 memory size  */
    GT_U32      check_iplr1MemSize;   /* check : IPLR 1 memory size  */

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                /*
                    The BC3 total memory is : 16K (0x4000)
                    we give BWC to BC2 : 8K for ingress and 8K for egress.

                    currently emulate it as :
                    Memory 0 : 4K
                    Memory 1 : 4K - 512
                        this value need to be at least : 4*512 ,  due to :
                        It was in bobk/bc2 :  {<traffic type> 2 MSBits , <portNum> 8 LSBits}
                        In bc3                {<traffic type> 2 MSBits , <portNum> 9 LSBits}
                    Memory 2 : 512
                */
             minSize  = 512;          /* in BC2 256 , but BC3 support 512 physical ports */
             ram0Size = _4K;          /* BWC to BC2 values */
             ingressPolicersNum = _8K;/* BWC to BC2 values */
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                /*
                    The Aldrin2 total memory is : 8K (half from BC3)
                    we give default behavior like BC3/2 : 4K for ingress and 4K for egress.

                    currently emulate it as :
                    Memory 0 : 2K - 128
                    Memory 1 : 2K
                        this value need to be at least : 4*512 ,  due to :
                        It was in bobk/bc2 :  {<traffic type> 2 MSBits , <portNum> 8 LSBits}
                        In bc3                {<traffic type> 2 MSBits , <portNum> 9 LSBits}
                    Memory 2 : 128
                */
             minSize  = 128;          /* Aldrin2 support 128 physical ports */
             ram0Size = _2K-128;      /* NOTE that 2K must be in Memory 1 to support 4*512 : {<traffic type> 2 MSBits , <portNum> 9 LSBits} */
             ingressPolicersNum = _4K;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            /* currently Falcon as BC3 */
            CPSS_TBD_BOOKMARK_FALCON
                /*
                    The Falcon (per pipe) total memory is : 4K (0x1000)
                    we will split 3K for ingress and 1K for egress.

                    currently emulate it as :
                    Memory 0 : 1K
                    Memory 1 : 2K
                        this value need to be at least : 4*512 ,  due to :
                        It was in bobk/bc2 :  {<traffic type> 2 MSBits , <portNum> 8 LSBits}
                        In bc3                {<traffic type> 2 MSBits , <portNum> 9 LSBits}
                    Memory 2 : 128
                */
             minSize  = 128;          /* for 128 physical ports */
             ram0Size = _2K;          /* NOTE that 2K must be in Memory 1 to support 4*512 : {<traffic type> 2 MSBits , <portNum> 9 LSBits} */
             ingressPolicersNum = _3K;
            break;
        default:
            /* ignored for other devices */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if(isWriteOperation == GT_TRUE)
    {
        switch(mode)
        {
            case CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E:
                if((numEntriesIngressStage1 + numEntriesIngressStage0) > PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "(numEntriesIngressStage1[%d] + numEntriesIngressStage0[%d])=[%d] must not be more than [%d]",
                        numEntriesIngressStage0,
                        numEntriesIngressStage1,
                        numEntriesIngressStage0+numEntriesIngressStage1,
                        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum);
                }

                iplr0MemSize = numEntriesIngressStage0;
                iplr1MemSize = numEntriesIngressStage1;
                break;

                CASE_PLR_PARTITION_SIZE_GET(/*IN*/minSize,/*IN*/ingressPolicersNum,/*OUT*/iplr0MemSize,/*OUT*/iplr1MemSize);

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        fakeHwMode = mode;
    }
    else
    {
        PLR_COUNTING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
        /* COUNTING : PLR1 after PLR0 */
        regAddr =  PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_INGRESS_1_E).policerCountingBaseAddr;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, &iplr0MemSize);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* COUNTING : EPLR after PLR1 */
        regAddr =  PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_EGRESS_E).policerCountingBaseAddr;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, &tmpMemSize);
        if (rc != GT_OK)
        {
            return rc;
        }

        iplr1MemSize = tmpMemSize -  iplr0MemSize;

        for(fakeHwMode = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_0_E ;
            fakeHwMode < CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E ;
            fakeHwMode++)
        {
            switch(fakeHwMode)
            {
                CASE_PLR_PARTITION_SIZE_GET(/*IN*/minSize,/*IN*/ingressPolicersNum,/*OUT*/check_iplr0MemSize,/*OUT*/check_iplr1MemSize);
                default:
                    check_iplr0MemSize = 0;
                    check_iplr1MemSize = 0;
                    break;
            }

            if(check_iplr0MemSize == iplr0MemSize &&
               check_iplr1MemSize == iplr1MemSize)
            {
                /* we found fake mode */
                break;
            }
        }

        if(fakeHwMode == CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E)
        {
            /* not found fake mode */
            fakeHwMode = CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E;
        }
    }

    *iplr0MemSizePtr = iplr0MemSize;
    *iplr1MemSizePtr = iplr1MemSize;

    if(modePtr)
    {
        *modePtr = fakeHwMode;
    }

    return GT_OK;
}

/**
* @internal ramSize0Get function
* @endinternal
*
* @brief   Get RAM 0 size
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] numEntriesRamPtr         - pointer to the RAM 0 size
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS ramSize0Get(
    IN GT_U8        devNum ,
    OUT  GT_U32     *numEntriesRamPtr
)
{
    GT_U32 ram0Size;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_XCAT2_E:
            ram0Size = 172;
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                if (PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98DX8332_Z0_CNS)
                {
                    /*    The partitions in AldrinZ are:*/
                    /*    Memory 0 : 1536               */
                    /*    Memory 1 : 256                */
                    /*    Memory 2 : 256                */
                    ram0Size = 1536;

                }
                else
                {
                    /*    The partitions in BOBK are: */
                    /*    Memory 0 : 4888             */
                    /*    Memory 1 : 1000             */
                    /*    Memory 2 : 256              */
                    ram0Size = 4888;
                }
            }
            else
            {
                ram0Size = _4K;
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            /*    The partitions in Aldrin are: */
            /*    Memory 0 : 1536               */
            /*    Memory 1 : 256                */
            /*    Memory 2 : 256                */
            ram0Size = 1536;
            break;
        default:
            /* ignored for other devices */
            ram0Size = _4K;
            break;
    }

    *numEntriesRamPtr = ram0Size;
    return GT_OK;
}

/**
* @internal ramSize1Get function
* @endinternal
*
* @brief   Get RAM 1 size
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
*
* @param[out] numEntriesRamPtr         - pointer to the RAM 1 size
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS ramSize1Get(
    IN GT_U8        devNum ,
    OUT  GT_U32     *numEntriesRamPtr
)
{
    GT_U32 ram1Size;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                if (PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98DX8332_Z0_CNS)
                {
                    /*    The partitions in AldrinZ are:*/
                    /*    Memory 0 : 1536               */
                    /*    Memory 1 : 256                */
                    /*    Memory 2 : 256                */
                    ram1Size = 256;

                }
                else
                {
                    /*    The partitions in BOBK are: */
                    /*    Memory 0 : 4888             */
                    /*    Memory 1 : 1000             */
                    /*    Memory 2 : 256              */
                    ram1Size = 1000;
                }
            }
            else
            {
                ram1Size = _4K-256;
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            /*    The partitions in Aldrin, AC3X are: */
            /*    Memory 0 : 1536               */
            /*    Memory 1 : 256                */
            /*    Memory 2 : 256                */
            ram1Size = 256;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *numEntriesRamPtr = ram1Size;
    return GT_OK;
}


/**
* @internal policerMemoryPartitionsInfoGet function
* @endinternal
*
* @brief   Get Policer partitions info.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     - The  in which internal tables are shared.
*                                      isWriteOperation - is the info needed for 'write' operation or for read operation.
*                                      GT_TRUE  - write operation
*                                      GT_FALSE - read operation
*
* @param[out] regAddrPtr               - pointer to address of the register for 'write mode'
*                                      ignored if NULL
* @param[out] offsetPtr                - pointer to offset of bit in the register for 'write mode'
*                                      ignored if NULL
* @param[out] numOfBitsPtr             - pointer to number of bits in the register for 'write mode'
*                                      ignored if NULL
* @param[out] hwDataPtr                - pointer to HW data to register for 'write mode'
*                                      ignored if NULL
* @param[out] iplr0MemSizePtr          - pointer to the size of iplr0
* @param[out] iplr1MemSizePtr          - pointer to the size of iplr1
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS policerMemoryPartitionsInfoGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT      mode,
    OUT GT_U32                              *regAddrPtr,
    OUT GT_U32                              *offsetPtr,
    OUT GT_U32                              *numOfBitsPtr,
    OUT GT_U32                              *hwDataPtr,
    OUT GT_U32                              *iplr0MemSizePtr,
    OUT GT_U32                              *iplr1MemSizePtr
)
{
    GT_STATUS   rc;
    GT_U32      hwData;         /* hw data value xCat and Lion       */
    GT_U32      hwData1;    /* hw data value for xCat2           */
    GT_U32      hwData2 = 0;    /* hw data value for Lion2           */
    GT_U32      hwData3;    /* hw data value for eArch           */
    GT_U32      regAddr;        /* register address                  */
    GT_U32      offset;         /* field offset in the register      */
    GT_U32      numOfBits;      /* size of field in bits             */
    GT_U32      ram0Size;       /* RAM#0 size, used for device with
                                   3 memories like xCat2, Bobcat2, Caelum */
    GT_U32      minSize = 256;
    GT_U32      ingressPolicersNum;/* number of ingress policers (meters/counter)*/
    GT_U32      iplr0MemSize;   /* IPLR 0 memory size                */
    GT_U32      iplr1MemSize;   /* IPLR 1 memory size                */

    ingressPolicersNum = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum;

    rc = ramSize0Get(devNum,&ram0Size);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch (mode)
    {
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E:
            iplr1MemSize = MIN(minSize, ingressPolicersNum);
            iplr0MemSize = ingressPolicersNum -
                iplr1MemSize;
            hwData = 0;
            hwData1 = 5;
            hwData2 = 2;
            hwData3 = MEM_2;/*0x4*/
            break;
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E:
            iplr1MemSize = 0;
            iplr0MemSize = ingressPolicersNum;
            hwData = 1;
            hwData1 = 0;
            hwData2 = 0;
            hwData3 = 0x0;
            break;
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E:
            /* Check that the second stage is supported, if not, then memory
               size of the single stage will become 0. */
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->
                fineTuning.featureInfo.iplrSecondStageSupported == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            iplr0MemSize = 0;
            iplr1MemSize = ingressPolicersNum;
            hwData = 2;
            hwData1 = 4;
            hwData2 = 3;
            hwData3 = MEM_2 | MEM_1 | MEM_0;/*0x7*/
            break;
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E:
            iplr0MemSize = MIN(minSize, ingressPolicersNum);
            iplr1MemSize = ingressPolicersNum -
                iplr0MemSize;
            hwData = 3;
            hwData1 = 1;
            hwData2 = 1;
            hwData3 = MEM_1 | MEM_0;/*0x3*/
            break;
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_4_E:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->
                fineTuning.featureInfo.iplrSecondStageSupported == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
                (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE))
            {
                iplr0MemSize = MIN(ram0Size, ingressPolicersNum);
                iplr1MemSize = ingressPolicersNum -
                    iplr0MemSize;
                hwData = 0;
                hwData1 = 2;
                hwData3 = MEM_2 | MEM_1;/*0x6*/
                break;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E:
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->
                fineTuning.featureInfo.iplrSecondStageSupported == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E) ||
                (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE))
            {
                iplr1MemSize = MIN(ram0Size, ingressPolicersNum);
                iplr0MemSize = ingressPolicersNum -
                    iplr1MemSize;
                hwData = 0;
                hwData1 = 6;
                hwData3 = MEM_0;/*0x1*/
                break;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,0).
                                                hierarchicalPolicerCtrl;
        offset = 5;
        numOfBits = 3;
        hwData = hwData3;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        /* xCat2 and above use Hierarchical Policer control register
          for memory control. The register is policerControl2Reg in the DB */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControl2Reg;
        offset = 5;
        numOfBits = 3;
        hwData = hwData1;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_DXCH_LION_E)
    {
        /* xCat and Lion use Policer control register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControlReg;
        offset = 11;
        numOfBits = 2;
    }
    else
    {
        /* Lion2 and above use Hierarchical Policer control register
          for memory control. The register is policerControl2Reg in the DB */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControl2Reg;
        offset = 5;
        numOfBits = 2;
        hwData = hwData2;
    }

    if(hwDataPtr)
    {
        *hwDataPtr = hwData;
    }

    if(numOfBitsPtr)
    {
        *numOfBitsPtr = numOfBits;
    }

    if(offsetPtr)
    {
        *offsetPtr = offset;
    }

    if(regAddrPtr)
    {
        *regAddrPtr = regAddr;
    }

    *iplr0MemSizePtr = iplr0MemSize;
    *iplr1MemSizePtr = iplr1MemSize;

    return GT_OK;
}

/**
* @internal internal_cpssDxChPolicerMemorySizeModeSet function
* @endinternal
*
* @brief   Sets internal table sizes and the way they are shared between the Ingress
*         policers.
*         In Bobcat3 the 'flex mode' allow to control partitions also with the egress
*         policer.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     - The  in which internal tables are shared.
* @param[in] numEntriesIngressStage0  - The number of metering/billing counters for
*                                      ingress policer stage 0
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*                                      (APPLICABLE DEVICES Bobcat3)
* @param[in] numEntriesIngressStage1  - The number of metering/billing counters for
*                                      ingress policer stage 1
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon)
*                                      NOTE: the egress policer gets the 'rest' of the metering/billing counters
*                                      the that device support.
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mode change should not be done when meter or counter entries are in use.
*
*/
static GT_STATUS internal_cpssDxChPolicerMemorySizeModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT      mode,
    IN  GT_U32                                      numEntriesIngressStage0,
    IN  GT_U32                                      numEntriesIngressStage1
)
{
    GT_STATUS   rc;
    GT_STATUS   retStatus;      /* return code                       */
    GT_U32      hwData;         /* hw data value xCat and Lion       */
    GT_U32      regAddr;        /* register address                  */
    GT_U32      offset;         /* field offset in the register      */
    GT_U32      numOfBits;      /* size of field in bits             */
    GT_U32      iplr0MemSize;   /* IPLR 0 memory size                */
    GT_U32      iplr1MemSize;   /* IPLR 1 memory size                */
    GT_U32      old_hwData;     /* HW mode before the change */
    GT_U32      modeChanged = 0;/* indication that HW mode changes */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        retStatus = sip5_20policerMemoryPartitionsInfoGet(devNum,
                        /*INPUTS*/
                        mode,
                        GT_TRUE,/*write operation*/
                        numEntriesIngressStage0,
                        numEntriesIngressStage1,
                        /*OUTPUTS*/
                    &iplr0MemSize,&iplr1MemSize,
                    NULL/*modePtr*/
                    );
        if(retStatus != GT_OK)
        {
            return retStatus;
        }
    }
    else
    {
        retStatus = policerMemoryPartitionsInfoGet(devNum,
                        /*INPUTS*/
                        mode,
                        /*OUTPUTS*/
                    &regAddr, &offset, &numOfBits, &hwData,
                    &iplr0MemSize,&iplr1MemSize
                    );
        if(retStatus != GT_OK)
        {
            return retStatus;
        }


        retStatus = prvCpssHwPpGetRegField(devNum, regAddr, offset, numOfBits, &old_hwData);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        if(old_hwData != hwData)
        {
            modeChanged = 1;
        }

        retStatus = prvCpssHwPpSetRegField(devNum, regAddr, offset, numOfBits, hwData);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        if(PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
            PRV_CPSS_DXCH_CAELUM_RM_PLR_UNUSED_IPLR_STAGES_MUST_BE_BYPASSED_WA_E))
        {
            /* All three memories are assigned to IPLR1: IPLR0 must be bypassed */
            GT_BOOL     iPlr0Bypassed = (hwData == ((GT_U32)((1 << numOfBits) - 1)));
            /* All three memories are assigned to IPLR0: IPLR1 must be bypassed */
            GT_BOOL     iPlr1Bypassed = (hwData == 0);

            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, 0).policerCtrl1;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 1,
                BOOL2BIT_MAC(iPlr0Bypassed));
            if(rc != GT_OK)
            {
                return rc;
            }
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, 1).policerCtrl1;
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 1,
                BOOL2BIT_MAC(iPlr1Bypassed));
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
        /* Meter Configuration Table is shared one starting from BobK.
           Need to configure metering base address for stage 1 to be equal size of IPLR0 table. */
        regAddr =  PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_INGRESS_1_E).policerMeteringBaseAddr;
        retStatus = prvCpssHwPpSetRegField(devNum, regAddr, 0, 24, iplr0MemSize);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
        /* Meter Configuration Table is shared one starting from BC3 on ALL units */
        /* Counting Configuration Table is shared one starting from BC3 on ALL units */

        /* METERING : set EPLR after PLR1*/
        regAddr =  PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_EGRESS_E).policerMeteringBaseAddr;
        retStatus = prvCpssHwPpSetRegField(devNum, regAddr, 0, 24, iplr0MemSize+iplr1MemSize);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        PLR_COUNTING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
        /* COUNTING : set PLR1 after PLR0 */
        regAddr =  PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_INGRESS_1_E).policerCountingBaseAddr;
        retStatus = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, iplr0MemSize);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }
        /* COUNTING : set PLR1 after PLR0 */
        regAddr =  PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_EGRESS_E).policerCountingBaseAddr;
        retStatus = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, iplr0MemSize+iplr1MemSize);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        /****************************************/
        /* for DIRECT access to counting tables */
        /****************************************/

        /* update the counting table base address of the IPLR1,and EPLR */
        PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_INGRESS_1_E).policerCountingTbl =
            METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS*iplr0MemSize+
            PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_INGRESS_0_E).policerCountingTbl;

        PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_EGRESS_E).policerCountingTbl =
            METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS*(iplr0MemSize+iplr1MemSize)+
            PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,CPSS_DXCH_POLICER_STAGE_INGRESS_0_E).policerCountingTbl;

        /***********************************************************/
        /* NO change needed for INDIRECT access to counting tables */
        /***********************************************************/
    }

    /* update memory size in the policer db */
    PLR0_SIZE_MAC(devNum) = iplr0MemSize;
    PLR1_SIZE_MAC(devNum) = iplr1MemSize;

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* EPLR gets all the rest */
        PLR2_SIZE_MAC(devNum) = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum - (iplr0MemSize+iplr1MemSize);
    }

    if(modeChanged)
    {
        /* re-synch HW and shadow entries */

        (void)prvCpssDxChDiagDataIntegrityShadowAndHwSynchTable(devNum,
                CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E);
        (void)prvCpssDxChDiagDataIntegrityShadowAndHwSynchTable(devNum,
                CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E);

        (void)prvCpssDxChDiagDataIntegrityShadowAndHwSynchTable(devNum,
                CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E);
        (void)prvCpssDxChDiagDataIntegrityShadowAndHwSynchTable(devNum,
                CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPolicerMemorySizeModeSet function
* @endinternal
*
* @brief   Sets internal table sizes and the way they are shared between the Ingress
*         policers.
*         In Bobcat3 the 'flex mode' allow to control partitions also with the egress
*         policer.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     - The  in which internal tables are shared.
* @param[in] numEntriesIngressStage0  - The number of metering/billing counters for
*                                      ingress policer stage 0
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*                                      (APPLICABLE DEVICES Bobcat3)
* @param[in] numEntriesIngressStage1  - The number of metering/billing counters for
*                                      ingress policer stage 1
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*                                      (APPLICABLE DEVICES Bobcat3; Aldrin2; Falcon)
*                                      NOTE: the egress policer gets the 'rest' of the metering/billing counters
*                                      the that device support.
*                                      relevant only to mode == CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mode change should not be done when meter or counter entries are in use.
*
*/
GT_STATUS cpssDxChPolicerMemorySizeModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT      mode,
    IN  GT_U32                                      numEntriesIngressStage0,
    IN  GT_U32                                      numEntriesIngressStage1
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMemorySizeModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode , numEntriesIngressStage0 , numEntriesIngressStage1));

    rc = internal_cpssDxChPolicerMemorySizeModeSet(devNum, mode, numEntriesIngressStage0 , numEntriesIngressStage1);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode , numEntriesIngressStage0 , numEntriesIngressStage1));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerMemorySizeModeGet function
* @endinternal
*
* @brief   Gets internal table sizes and the way they are shared between the Ingress
*         policers.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - The mode in which internal tables are shared.
* @param[out] numEntriesIngressStage0Ptr - (pointer to) The number of metering/billing counters for
*                                      ingress policer stage 0
*                                      NOTE: ignored if NULL
* @param[out] numEntriesIngressStage1Ptr - (pointer to) The number of metering/billing counters for
*                                      ingress policer stage 1
*                                      NOTE: ignored if NULL
* @param[out] numEntriesEgressStagePtr - (pointer to) The number of metering/billing counters for
*                                      egress policer stage
*                                      NOTE: ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerMemorySizeModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT     *modePtr,
    OUT GT_U32                                     *numEntriesIngressStage0Ptr,
    OUT GT_U32                                     *numEntriesIngressStage1Ptr,
    OUT GT_U32                                     *numEntriesEgressStagePtr
)
{
    GT_STATUS   rc;  /* return code */
    GT_U32      hwData;     /* hw data value */
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* field offset in the register      */
    GT_U32      numOfBits;  /* size of field in bits             */
    GT_U32      iplr0MemSize;   /* IPLR 0 memory size                */
    GT_U32      iplr1MemSize;   /* IPLR 1 memory size                */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = sip5_20policerMemoryPartitionsInfoGet(devNum,
                        /*INPUTS*/
                        0,/* not used on 'read'*/
                        GT_FALSE,/*read operation*/
                        0,/* not used on 'read'*/
                        0,/* not used on 'read'*/
                        /*OUTPUTS*/
                    &iplr0MemSize,&iplr1MemSize,
                    modePtr
                    );
        if (rc != GT_OK)
        {
            return rc;
        }
        if(numEntriesIngressStage0Ptr)/*ignored if NULL*/
        {
            *numEntriesIngressStage0Ptr = iplr0MemSize;
        }

        if(numEntriesIngressStage1Ptr)/*ignored if NULL*/
        {
            *numEntriesIngressStage1Ptr = iplr1MemSize;
        }

        if(numEntriesEgressStagePtr)/*ignored if NULL*/
        {
            /* EPLR gets all the rest */
            *numEntriesEgressStagePtr   = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum - (iplr0MemSize+iplr1MemSize);
        }


        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,0).
                                                hierarchicalPolicerCtrl;
        offset = 5;
        numOfBits = 3;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        /* xCat2 and above use Hierarchical Policer control register
          for memory control. The register is policerControl2Reg in the DB */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControl2Reg;
        offset = 5;
        numOfBits = 3;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_DXCH_LION_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControlReg;
        offset = 11;
        numOfBits = 2;
    }
    else
    {
        /* Lion2 and above use Hierarchical Policer control register
          for memory control. The register is policerControl2Reg in the DB */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerControl2Reg;
        offset = 5;
        numOfBits = 2;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, numOfBits, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        switch (hwData)
        {
            case 0:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E;
                break;
            case 1:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E;
                break;
            case 3:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E;
                break;
            case 4:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E;
                break;
            case 5:
                /* relevant only to SIP5.20 */
                if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
                *modePtr = CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E;
                break;
            case 6:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_4_E;
                break;
            case 7:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        switch (hwData)
        {
            case 0:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E;
                break;
            case 2:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_4_E;
                break;
            case 1:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E;
                break;
            case 4:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E;
                break;
            case 5:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E;
                break;
            case 6:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily <= CPSS_PP_FAMILY_DXCH_LION_E)
    {
        switch (hwData)
        {
            case 0:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E;
                break;
            case 1:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E;
                break;
            case 2:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E;
                break;
            case 3:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch (hwData)
        {
            case 0:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E;
                break;
            case 1:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E;
                break;
            case 2:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E;
                break;
            case 3:
                *modePtr = CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    if(numEntriesIngressStage0Ptr == NULL &&
       numEntriesIngressStage1Ptr == NULL &&
       numEntriesEgressStagePtr == NULL)
    {
        /* no more to do */
        return GT_OK;
    }


    /* NOTE: this function can not use :
        PLR0_SIZE_MAC(devNum)
        PLR1_SIZE_MAC(devNum)
        PLR2_SIZE_MAC(devNum)
        to return values .... because during 'catchup' those values are set to
        'invalid value' see : PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS

        so we must restore sizes from the HW and not from 'CPSS DB'
    */

    rc = policerMemoryPartitionsInfoGet(devNum,
                    /*INPUTS*/
                    *modePtr,
                    /*OUTPUTS*/
                NULL/*regAddr*/,NULL/*offset*/, NULL/*numOfBits*/, NULL/*hwData*/,
                &iplr0MemSize,&iplr1MemSize
                );
    if(rc != GT_OK)
    {
        return rc;
    }

    if(numEntriesIngressStage0Ptr)/*ignored if NULL*/
    {
        *numEntriesIngressStage0Ptr = iplr0MemSize;
    }

    if(numEntriesIngressStage1Ptr)/*ignored if NULL*/
    {
        *numEntriesIngressStage1Ptr = iplr1MemSize;
    }

    if(numEntriesEgressStagePtr)/*ignored if NULL*/
    {
        /* EPLR hold separate memory */
        *numEntriesEgressStagePtr   = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.egressPolicersNum;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPolicerMemorySizeModeGet function
* @endinternal
*
* @brief   Gets internal table sizes and the way they are shared between the Ingress
*         policers.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - The mode in which internal tables are shared.
* @param[out] numEntriesIngressStage0Ptr - (pointer to) The number of metering/billing counters for
*                                      ingress policer stage 0
*                                      NOTE: ignored if NULL
* @param[out] numEntriesIngressStage1Ptr - (pointer to) The number of metering/billing counters for
*                                      ingress policer stage 1
*                                      NOTE: ignored if NULL
* @param[out] numEntriesEgressStagePtr - (pointer to) The number of metering/billing counters for
*                                      egress policer stage
*                                      NOTE: ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerMemorySizeModeGet
(
    IN  GT_U8                                      devNum,
    OUT CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT     *modePtr,
    OUT GT_U32                                     *numEntriesIngressStage0Ptr,
    OUT GT_U32                                     *numEntriesIngressStage1Ptr,
    OUT GT_U32                                     *numEntriesEgressStagePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMemorySizeModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr,
        numEntriesIngressStage0Ptr,numEntriesIngressStage1Ptr,numEntriesEgressStagePtr));

    rc = internal_cpssDxChPolicerMemorySizeModeGet(devNum, modePtr,
        numEntriesIngressStage0Ptr,numEntriesIngressStage1Ptr,numEntriesEgressStagePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr,
        numEntriesIngressStage0Ptr,numEntriesIngressStage1Ptr,numEntriesEgressStagePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerCountingWriteBackCacheFlush function
* @endinternal
*
* @brief   Flush internal Write Back Cache (WBC) of counting entries.
*         The Policer implements internal Write Back Cache for frequent and rapid
*         update of counting entries.
*         Since the WBC holds the policer entries' data, the CPU must clear its
*         content when performing direct write access of the counting entries by
*         one of following functions:
*         - cpssDxCh3PolicerBillingEntrySet
*         - cpssDxChPolicerVlanCntrSet
*         - cpssDxChPolicerPolicyCntrSet
*         The policer counting should be disabled before flush of WBC if direct
*         write access is performed under traffic. And policer counting should be
*         enabled again after finish of write access.
*         The cpssDxChPolicerCountingModeSet may be used to disable/enable
*         the policer counting.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To use the listed above APIs (cpssDxCh3PolicerBillingEntrySet,
*       cpssDxChPolicerVlanCntrSet and cpssDxChPolicerPolicyCntrSet) under
*       traffic, perform the following algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
static GT_STATUS internal_cpssDxChPolicerCountingWriteBackCacheFlush
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    return prvCpssDxCh3PolicerInternalTableAccess(devNum,
                                                  CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  stage,
                                                  0 /* not used for flush */,
                                                  PRV_CPSS_DXCH3_POLICER_CNTR_WBF_FLUSH_E);
}

/**
* @internal cpssDxChPolicerCountingWriteBackCacheFlush function
* @endinternal
*
* @brief   Flush internal Write Back Cache (WBC) of counting entries.
*         The Policer implements internal Write Back Cache for frequent and rapid
*         update of counting entries.
*         Since the WBC holds the policer entries' data, the CPU must clear its
*         content when performing direct write access of the counting entries by
*         one of following functions:
*         - cpssDxCh3PolicerBillingEntrySet
*         - cpssDxChPolicerVlanCntrSet
*         - cpssDxChPolicerPolicyCntrSet
*         The policer counting should be disabled before flush of WBC if direct
*         write access is performed under traffic. And policer counting should be
*         enabled again after finish of write access.
*         The cpssDxChPolicerCountingModeSet may be used to disable/enable
*         the policer counting.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To use the listed above APIs (cpssDxCh3PolicerBillingEntrySet,
*       cpssDxChPolicerVlanCntrSet and cpssDxChPolicerPolicyCntrSet) under
*       traffic, perform the following algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
GT_STATUS cpssDxChPolicerCountingWriteBackCacheFlush
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerCountingWriteBackCacheFlush);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage));

    rc = internal_cpssDxChPolicerCountingWriteBackCacheFlush(devNum, stage);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerMeteringCalcMethodSet function
* @endinternal
*
* @brief   Sets Metering Policer parameters calculation orientation method.
*         Due to the fact that selecting algorithm has effect on how HW metering
*         parameters are set, this API influence on
*         cpssDxCh3PolicerMeteringEntrySet and
*         cpssDxCh3PolicerEntryMeterParamsCalculate APIs.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] meteringCalcMethod       - selecting calculation of HW metering
*                                      parameters between CIR\PIR or CBS\PBS
*                                      orientation.
* @param[in] cirPirAllowedDeviation   - the allowed deviation in percentage from the
*                                      requested CIR\PIR. Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
* @param[in] cbsPbsCalcOnFail         - GT_TRUE: If CBS\PBS constraints cannot be
*                                      matched return to CIR\PIR oriented
*                                      calculation.
*                                      GT_FALSE: If CBS\PBS constraints cannot be
*                                      matched return error. Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or method.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - The default
*       algorithm in which metering parameters are configured in such a way
*       that the selected CIR\PIR is the one which is the nearest possible
*       (under the granularity constraint) to the requested CIR\PIR.
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - In this
*       algorithm a deviation from requested CIR\PIR is defined. At first stage
*       the algorithm tries to find parameters that will complay with both the
*       CBS\PBS requested and the range allowed for the CIR\PIR. If no matching
*       solution is achieved there is an option to perform another stage. At
*       the second (optional) stage CIR\PIR is selected in the allowed range
*       and under this selection CBS\PBS is configured to be as near as
*       possible to the requested one. If second stage is needed and not
*       selected failure (error) indication is reported.
*
*/
static GT_STATUS internal_cpssDxChPolicerMeteringCalcMethodSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT   meteringCalcMethod,
    IN GT_U32                                       cirPirAllowedDeviation,
    IN GT_BOOL                                      cbsPbsCalcOnFail
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);


    switch(meteringCalcMethod)
    {
        case CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E:
            PRV_CPSS_DXCH_PP_MAC(devNum)->policer.meteringCalcMethod =
                CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E;
            break;
        case CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E:
            PRV_CPSS_DXCH_PP_MAC(devNum)->policer.meteringCalcMethod =
                CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E;
            PRV_CPSS_DXCH_PP_MAC(devNum)->policer.cirPirAllowedDeviation =
                    cirPirAllowedDeviation;
            PRV_CPSS_DXCH_PP_MAC(devNum)->policer.cbsPbsCalcOnFail =
                cbsPbsCalcOnFail;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPolicerMeteringCalcMethodSet function
* @endinternal
*
* @brief   Sets Metering Policer parameters calculation orientation method.
*         Due to the fact that selecting algorithm has effect on how HW metering
*         parameters are set, this API influence on
*         cpssDxCh3PolicerMeteringEntrySet and
*         cpssDxCh3PolicerEntryMeterParamsCalculate APIs.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
* @param[in] meteringCalcMethod       - selecting calculation of HW metering
*                                      parameters between CIR\PIR or CBS\PBS
*                                      orientation.
* @param[in] cirPirAllowedDeviation   - the allowed deviation in percentage from the
*                                      requested CIR\PIR. Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
* @param[in] cbsPbsCalcOnFail         - GT_TRUE: If CBS\PBS constraints cannot be
*                                      matched return to CIR\PIR oriented
*                                      calculation.
*                                      GT_FALSE: If CBS\PBS constraints cannot be
*                                      matched return error. Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong devNum or method.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - The default
*       algorithm in which metering parameters are configured in such a way
*       that the selected CIR\PIR is the one which is the nearest possible
*       (under the granularity constraint) to the requested CIR\PIR.
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - In this
*       algorithm a deviation from requested CIR\PIR is defined. At first stage
*       the algorithm tries to find parameters that will complay with both the
*       CBS\PBS requested and the range allowed for the CIR\PIR. If no matching
*       solution is achieved there is an option to perform another stage. At
*       the second (optional) stage CIR\PIR is selected in the allowed range
*       and under this selection CBS\PBS is configured to be as near as
*       possible to the requested one. If second stage is needed and not
*       selected failure (error) indication is reported.
*
*/
GT_STATUS cpssDxChPolicerMeteringCalcMethodSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT   meteringCalcMethod,
    IN GT_U32                                       cirPirAllowedDeviation,
    IN GT_BOOL                                      cbsPbsCalcOnFail
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMeteringCalcMethodSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, meteringCalcMethod, cirPirAllowedDeviation, cbsPbsCalcOnFail));

    rc = internal_cpssDxChPolicerMeteringCalcMethodSet(devNum, meteringCalcMethod, cirPirAllowedDeviation, cbsPbsCalcOnFail);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, meteringCalcMethod, cirPirAllowedDeviation, cbsPbsCalcOnFail));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerMeteringCalcMethodGet function
* @endinternal
*
* @brief   Gets Metering Policer parameters calculation orientation method.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
*
* @param[out] meteringCalcMethodPtr    - (pointer to) selected calculation of HW
*                                      metering parameters between CIR\PIR or
*                                      CBS\PBS orientation.
* @param[out] cirPirAllowedDeviationPtr - (pointer to) the allowed deviation in
*                                      percentage from the requested CIR\PIR.
*                                      Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
* @param[out] cbsPbsCalcOnFailPtr      - (pointer to)
*                                      GT_TRUE: If CBS\PBS constraints cannot be
*                                      matched return to CIR\PIR oriented
*                                      calculation.
*                                      GT_FALSE: If CBS\PBS constraints cannot
*                                      be matched return error.
*                                      Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - wrong devNum or method.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - The default
*       algorithm in which metering parameters are configured in such a way
*       that the selected CIR\PIR is the one which is the nearest possible
*       (under the granularity constraint) to the requested CIR\PIR.
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - In this
*       algorithm a deviation from requested CIR\PIR is defined. At first stage
*       the algorithm tries to find parameters that will complay with both the
*       CBS\PBS requested and the range allowed for the CIR\PIR. If no matching
*       solution is achieved there is an option to perform another stage. At
*       the second (optional) stage CIR\PIR is selected in the allowed range
*       and under this selection CBS\PBS is configured to be as near as
*       possible to the requested one. If second stage is needed and not
*       selected failure (error) indication is reported.
*
*/
static GT_STATUS internal_cpssDxChPolicerMeteringCalcMethodGet
(
    IN GT_U8                                        devNum,
    OUT CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT  *meteringCalcMethodPtr,
    OUT GT_U32                                      *cirPirAllowedDeviationPtr,
    OUT GT_BOOL                                     *cbsPbsCalcOnFailPtr
)
{

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(meteringCalcMethodPtr);

    switch(PRV_CPSS_DXCH_PP_MAC(devNum)->policer.meteringCalcMethod)
    {
        case CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E:
            *meteringCalcMethodPtr = CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E;
            break;
        case CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E:
            CPSS_NULL_PTR_CHECK_MAC(cirPirAllowedDeviationPtr);
            CPSS_NULL_PTR_CHECK_MAC(cbsPbsCalcOnFailPtr);
            *meteringCalcMethodPtr = CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E;
            *cirPirAllowedDeviationPtr =
                PRV_CPSS_DXCH_PP_MAC(devNum)->policer.cirPirAllowedDeviation;
            *cbsPbsCalcOnFailPtr =
                PRV_CPSS_DXCH_PP_MAC(devNum)->policer.cbsPbsCalcOnFail;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPolicerMeteringCalcMethodGet function
* @endinternal
*
* @brief   Gets Metering Policer parameters calculation orientation method.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
*
* @param[out] meteringCalcMethodPtr    - (pointer to) selected calculation of HW
*                                      metering parameters between CIR\PIR or
*                                      CBS\PBS orientation.
* @param[out] cirPirAllowedDeviationPtr - (pointer to) the allowed deviation in
*                                      percentage from the requested CIR\PIR.
*                                      Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
* @param[out] cbsPbsCalcOnFailPtr      - (pointer to)
*                                      GT_TRUE: If CBS\PBS constraints cannot be
*                                      matched return to CIR\PIR oriented
*                                      calculation.
*                                      GT_FALSE: If CBS\PBS constraints cannot
*                                      be matched return error.
*                                      Relevant only for
*                                      CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - wrong devNum or method.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_ONLY_E - The default
*       algorithm in which metering parameters are configured in such a way
*       that the selected CIR\PIR is the one which is the nearest possible
*       (under the granularity constraint) to the requested CIR\PIR.
*       CPSS_DXCH_POLICER_METERING_CALC_METHOD_CIR_AND_CBS_E - In this
*       algorithm a deviation from requested CIR\PIR is defined. At first stage
*       the algorithm tries to find parameters that will complay with both the
*       CBS\PBS requested and the range allowed for the CIR\PIR. If no matching
*       solution is achieved there is an option to perform another stage. At
*       the second (optional) stage CIR\PIR is selected in the allowed range
*       and under this selection CBS\PBS is configured to be as near as
*       possible to the requested one. If second stage is needed and not
*       selected failure (error) indication is reported.
*
*/
GT_STATUS cpssDxChPolicerMeteringCalcMethodGet
(
    IN GT_U8                                        devNum,
    OUT CPSS_DXCH_POLICER_METERING_CALC_METHOD_ENT  *meteringCalcMethodPtr,
    OUT GT_U32                                      *cirPirAllowedDeviationPtr,
    OUT GT_BOOL                                     *cbsPbsCalcOnFailPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMeteringCalcMethodGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, meteringCalcMethodPtr, cirPirAllowedDeviationPtr, cbsPbsCalcOnFailPtr));

    rc = internal_cpssDxChPolicerMeteringCalcMethodGet(devNum, meteringCalcMethodPtr, cirPirAllowedDeviationPtr, cbsPbsCalcOnFailPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, meteringCalcMethodPtr, cirPirAllowedDeviationPtr, cbsPbsCalcOnFailPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChPolicerCountingTriggerByPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Counting when port based metering is triggered for
*         the packet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - GT_TRUE - If counting mode is Billing, counting is performed
*                                      for every packet subject to port based metering.
*                                      If counting mode is not Billing, counting is
*                                      performed for these packets only if the Policy
*                                      engine enabled for Counting.
*                                      GT_FALSE - Counting is performed for these packets only
*                                      if the Policy engine enabled for Counting.
*                                      .
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerCountingTriggerByPortEnableSet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN GT_BOOL                                  enable
)
{
    GT_U32      hwData;         /* hw data value        */
    GT_U32      regAddr;        /* register address     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    hierarchicalPolicerCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControl2Reg;
    }

    hwData = BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(devNum, regAddr, 3, 1, hwData);
}

/**
* @internal cpssDxChPolicerCountingTriggerByPortEnableSet function
* @endinternal
*
* @brief   Enable/Disable Counting when port based metering is triggered for
*         the packet.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - GT_TRUE - If counting mode is Billing, counting is performed
*                                      for every packet subject to port based metering.
*                                      If counting mode is not Billing, counting is
*                                      performed for these packets only if the Policy
*                                      engine enabled for Counting.
*                                      GT_FALSE - Counting is performed for these packets only
*                                      if the Policy engine enabled for Counting.
*                                      .
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerCountingTriggerByPortEnableSet
(
    IN  GT_U8                                   devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN GT_BOOL                                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerCountingTriggerByPortEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enable));

    rc = internal_cpssDxChPolicerCountingTriggerByPortEnableSet(devNum, stage, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerCountingTriggerByPortEnableGet function
* @endinternal
*
* @brief   Get Counting trigger by port status.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - GT_TRUE - If counting mode is Billing, counting is performed
*                                      for every packet subject to port based metering.
*                                      If counting mode is not Billing, counting is
*                                      performed for these packets only if the Policy
*                                      engine enabled for Counting.
*                                      GT_FALSE - Counting is performed for these packets only
*                                      if the Policy engine enabled for Counting.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerCountingTriggerByPortEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_U32      hwData;         /* hw data value        */
    GT_U32      regAddr;        /* register address     */
    GT_STATUS   retStatus;      /* return code          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    hierarchicalPolicerCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControl2Reg;
    }

    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 3, 1, &hwData);

    if (GT_OK == retStatus)
    {
        *enablePtr = BIT2BOOL_MAC(hwData);
    }

    return retStatus;
}

/**
* @internal cpssDxChPolicerCountingTriggerByPortEnableGet function
* @endinternal
*
* @brief   Get Counting trigger by port status.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - GT_TRUE - If counting mode is Billing, counting is performed
*                                      for every packet subject to port based metering.
*                                      If counting mode is not Billing, counting is
*                                      performed for these packets only if the Policy
*                                      engine enabled for Counting.
*                                      GT_FALSE - Counting is performed for these packets only
*                                      if the Policy engine enabled for Counting.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerCountingTriggerByPortEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerCountingTriggerByPortEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enablePtr));

    rc = internal_cpssDxChPolicerCountingTriggerByPortEnableGet(devNum, stage, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPolicerPortModeAddressSelectSet function
* @endinternal
*
* @brief   Configure Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] type                     - Address select type: Full or Compressed.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or type.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerPortModeAddressSelectSet
(
    IN  GT_U8                                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT                     stage,
    IN CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT     type
)
{
    GT_U32      hwData;         /* hw data value        */
    GT_U32      regAddr;        /* register address     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E );

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    hierarchicalPolicerCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControl2Reg;
    }

    switch (type)
    {
        case CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E:
            hwData = 0;
            break;
        case CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E:
            hwData = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, 4, 1, hwData);
}

/**
* @internal cpssDxChPolicerPortModeAddressSelectSet function
* @endinternal
*
* @brief   Configure Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] type                     - Address select type: Full or Compressed.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or type.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortModeAddressSelectSet
(
    IN  GT_U8                                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT                     stage,
    IN CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT     type
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortModeAddressSelectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, type));

    rc = internal_cpssDxChPolicerPortModeAddressSelectSet(devNum, stage, type);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, type));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPolicerPortModeAddressSelectGet function
* @endinternal
*
* @brief   Get Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] typePtr                  - (pointer to) Address select type: Full or Compressed.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerPortModeAddressSelectGet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT                    stage,
    OUT CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT    *typePtr
)
{

    GT_U32      hwData;         /* hw data value        */
    GT_U32      regAddr;        /* register address     */
    GT_STATUS   retStatus;      /* return code          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E );

    CPSS_NULL_PTR_CHECK_MAC(typePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    hierarchicalPolicerCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControl2Reg;
    }

    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 4, 1, &hwData);

    if (GT_OK == retStatus)
    {
        *typePtr = (hwData) ? CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E :
            CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E;
    }

    return retStatus;
}

/**
* @internal cpssDxChPolicerPortModeAddressSelectGet function
* @endinternal
*
* @brief   Get Metering Address calculation type.
*         Relevant when stage mode is CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] typePtr                  - (pointer to) Address select type: Full or Compressed.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortModeAddressSelectGet
(
    IN  GT_U8                                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT                    stage,
    OUT CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT    *typePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortModeAddressSelectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, typePtr));

    rc = internal_cpssDxChPolicerPortModeAddressSelectGet(devNum, stage, typePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, typePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerTriggerEntrySet function
* @endinternal
*
* @brief   Set Policer trigger entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..8191).
* @param[in] entryPtr                 - (pointer to) Policer port trigger entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerTriggerEntrySet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
)
{
    GT_U32                  hwDataArr[2];   /* value to write to hw     */
    CPSS_DXCH_TABLE_ENT tableType;      /* ePort trigger table type */
    GT_STATUS               rc;             /* return code              */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
    {
        tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E;
    }
    else if (CPSS_DXCH_POLICER_STAGE_INGRESS_1_E == stage )
    {
        tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E;
    }
    else /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
    {
        tableType = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E;
    }

    /* convert trigger entry to hw value */
    rc = prvCpssDxChPolicerTriggerEntryToHwConvert(devNum, entryPtr, hwDataArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChPortGroupWriteTableEntry(devNum,
                                               CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                               tableType,
                                               entryIndex,
                                               &hwDataArr[0]);
}

/**
* @internal cpssDxChPolicerTriggerEntrySet function
* @endinternal
*
* @brief   Set Policer trigger entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..8191).
* @param[in] entryPtr                 - (pointer to) Policer port trigger entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerTriggerEntrySet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerTriggerEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, entryIndex, entryPtr));

    rc = internal_cpssDxChPolicerTriggerEntrySet(devNum, stage, entryIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, entryIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerTriggerEntryGet function
* @endinternal
*
* @brief   Get Policer trigger entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..8191).
*
* @param[out] entryPtr                 - (pointer to) Policer port trigger entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerTriggerEntryGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
)
{
    CPSS_DXCH_TABLE_ENT tableType;      /* ePort trigger table type */
    GT_U32                  hwDataArr[2];   /* value to write to hw */
    GT_STATUS               retStatus;      /* return code          */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
                                          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
                                          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
    {
        tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E;
    }
    else if (CPSS_DXCH_POLICER_STAGE_INGRESS_1_E == stage )
    {
        tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E;
    }
    else /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
    {
        tableType = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E;
    }

    retStatus = prvCpssDxChPortGroupReadTableEntry(devNum,
                                                   CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                   tableType,
                                                   entryIndex,
                                                   &hwDataArr[0]);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    /* convert trigger entry to SW value */
    return prvCpssDxChPolicerTriggerEntryToSwConvert(devNum, hwDataArr, entryPtr);
}

/**
* @internal cpssDxChPolicerTriggerEntryGet function
* @endinternal
*
* @brief   Get Policer trigger entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..8191).
*
* @param[out] entryPtr                 - (pointer to) Policer port trigger entry.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or index.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerTriggerEntryGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerTriggerEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, entryIndex, entryPtr));

    rc = internal_cpssDxChPolicerTriggerEntryGet(devNum, stage, entryIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, entryIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPolicerTriggerEntryToHwConvert function
* @endinternal
*
* @brief   Convert Policer trigger entry to hw data.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] entryPtr                 - (pointer to) Policer port trigger entry
*
* @param[out] hwDataPtr                - (pointer to) data to be written to hw
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - on out of range of policerIndex
*                                       or traffic type offsets.
*
* @note In Ingress Dual stage both IPLR_0 and IPLR_1 configure same memory.
*
*/
static GT_STATUS prvCpssDxChPolicerTriggerEntryToHwConvert
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr,
    OUT GT_U32                                  *hwDataPtr
)
{
    GT_U32      tmpValue;        /* temporary value       */
    GT_U32      policerIndexMax; /* policerIndex field <max value>+1 */
    GT_U32      dec;             /* decrement (in bits). */

    /* clear hw data array */
    cpssOsMemSet(hwDataPtr, 0, (sizeof(GT_U32) * 2));

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* length of policerIndex field reduced in sip6 devices from
           16 bits to 12 bits. So fields above policerIndex are shifted. */
        policerIndexMax = BIT_12;
        dec = 4;
    }
    else
    {
        policerIndexMax = BIT_16;
        dec = 0;
    }

    /* check entry parameters validity */
    if ((entryPtr->policerIndex >= policerIndexMax) ||
        ((entryPtr->ucKnownOffset > 6) && entryPtr->ucKnownEnable) ||
        ((entryPtr->ucUnknownOffset > 6) && entryPtr->ucUnknownEnable) ||
        ((entryPtr->mcRegisteredOffset > 6) && entryPtr->mcRegisteredEnable) ||
        ((entryPtr->mcUnregisteredOffset > 6) && entryPtr->mcUnregisteredEnable) ||
        ((entryPtr->bcOffset > 6) && entryPtr->bcEnable) ||
        ((entryPtr->tcpSynOffset > 6) && entryPtr->tcpSynEnable))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,0,1,
                      (BOOL2BIT_MAC(entryPtr->meteringEnable)));

    U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,1,1,
                      (BOOL2BIT_MAC(entryPtr->countingEnable)));


    U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,2,16-dec,entryPtr->policerIndex);

    /* known unicast */
    if (entryPtr->ucKnownEnable)
    {
        tmpValue = entryPtr->ucKnownOffset;
    }
    else
    {
        tmpValue = 0x7;
    }
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,18-dec,3,tmpValue);

    /* unknown unicast */
    if (entryPtr->ucUnknownEnable)
    {
        tmpValue = entryPtr->ucUnknownOffset;
    }
    else
    {
        tmpValue = 0x7;
    }
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,21-dec,3,tmpValue);

    /* registered multicast */
    if (entryPtr->mcRegisteredEnable)
    {
        tmpValue = entryPtr->mcRegisteredOffset;
    }
    else
    {
        tmpValue = 0x7;
    }
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,24-dec,3,tmpValue);

    /* unregistered multicast */
    if (entryPtr->mcUnregisteredEnable)
    {
        tmpValue = entryPtr->mcUnregisteredOffset;
    }
    else
    {
        tmpValue = 0x7;
    }
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,27-dec,3,tmpValue);

    /* broadcast */
    if (entryPtr->bcEnable)
    {
        tmpValue = entryPtr->bcOffset;
    }
    else
    {
        tmpValue = 0x7;
    }
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,30-dec,3,tmpValue);

    /* TCP-SYN */
    if (entryPtr->tcpSynEnable)
    {
        tmpValue = entryPtr->tcpSynOffset;
    }
    else
    {
        tmpValue = 0x7;
    }
    U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr,33-dec,3,tmpValue);

    return GT_OK;
}


/**
* @internal prvCpssDxChPolicerTriggerEntryToSwConvert function
* @endinternal
*
* @brief   Convert Policer trigger hw data to sw entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] hwDataPtr                - (pointer to) data read from hw
*
* @param[out] entryPtr                 - (pointer to) Policer port trigger entry
*                                       GT_OK if successful, or
*                                       GT_BAD_PARAM otherwise.
*
* @note In Ingress Dual stage both IPLR_0 and IPLR_1 configure same memory.
*
*/
static GT_STATUS prvCpssDxChPolicerTriggerEntryToSwConvert
(
    IN  GT_U8                                    devNum,
    IN   GT_U32                                  *hwDataPtr,
    OUT  CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC     *entryPtr
)
{
    GT_U32      tmpValue;       /* temporary value       */
    GT_U32      dec;             /* decrement (in bits). */


    /* clear entry */
    cpssOsMemSet(entryPtr, 0, sizeof(CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC));


    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr,0,1,tmpValue)
    entryPtr->meteringEnable = BIT2BOOL_MAC(tmpValue);
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr,1,1,tmpValue)
    entryPtr->countingEnable = BIT2BOOL_MAC(tmpValue);

    /* length of policerIndex field reduced in sip6 devices from
       16 bits to 12 bits. So fields above policerIndex are shifted. */
    dec =  (PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 4 : 0);

    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr,2,16-dec,tmpValue)
    entryPtr->policerIndex = tmpValue;

    /* known unicast */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr,18-dec,3,tmpValue);
    if (tmpValue == 0x7)
    {
        entryPtr->ucKnownEnable = GT_FALSE;
        /* the value is not relevant */
        entryPtr->ucKnownOffset = 0x0;
    }
    else
    {
        entryPtr->ucKnownEnable = GT_TRUE;
        entryPtr->ucKnownOffset = tmpValue;
    }

    /* unknown unicast */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr,21-dec,3,tmpValue);
    if (tmpValue == 0x7)
    {
        entryPtr->ucUnknownEnable = GT_FALSE;
        /* the value is not relevant */
        entryPtr->ucUnknownOffset = 0x0;
    }
    else
    {
        entryPtr->ucUnknownEnable = GT_TRUE;
        entryPtr->ucUnknownOffset = tmpValue;
    }

    /* registered multicast */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr,24-dec,3,tmpValue);
    if (tmpValue == 0x7)
    {
        entryPtr->mcRegisteredEnable = GT_FALSE;
        /* the value is not relevant */
        entryPtr->mcRegisteredOffset = 0x0;
    }
    else
    {
        entryPtr->mcRegisteredEnable = GT_TRUE;
        entryPtr->mcRegisteredOffset = tmpValue;
    }

    /* unregistered multicast */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr,27-dec,3,tmpValue);
    if (tmpValue == 0x7)
    {
        entryPtr->mcUnregisteredEnable = GT_FALSE;
        /* the value is not relevant */
        entryPtr->mcUnregisteredOffset = 0x0;
    }
    else
    {
        entryPtr->mcUnregisteredEnable = GT_TRUE;
        entryPtr->mcUnregisteredOffset = tmpValue;
    }

    /* broadcast */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr,30-dec,3,tmpValue);
    if (tmpValue == 0x7)
    {
        entryPtr->bcEnable = GT_FALSE;
        /* the value is not relevant */
        entryPtr->bcOffset = 0x0;
    }
    else
    {
        entryPtr->bcEnable = GT_TRUE;
        entryPtr->bcOffset = tmpValue;
    }

    /* TCP-SYN */
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr,33-dec,3,tmpValue);
    if (tmpValue == 0x7)
    {
        entryPtr->tcpSynEnable = GT_FALSE;
        /* the value is not relevant */
        entryPtr->tcpSynOffset = 0x0;
    }
    else
    {
        entryPtr->tcpSynEnable = GT_TRUE;
        entryPtr->tcpSynOffset = tmpValue;
    }

    return GT_OK;
}

/******************************************************************************/
/******************************************************************************/
/******* start of functions with portGroupsBmp parameter **********************/
/******************************************************************************/
/******************************************************************************/


/**
* @internal internal_cpssDxChPolicerPortGroupMeteringEntryRefresh function
* @endinternal
*
* @brief   Refresh the Policer Metering Entry.
*         Meter Bucket's state parameters refreshing are needed in order to
*         prevent mis-behavior due to wrap around of timers.
*         The wrap around problem can occur when there are long periods of
*         'silence' on the flow and the bucket's state parameters do not get
*         refreshed (meter is in the idle state). This causes a problem once the
*         flow traffic is re-started since the timers might have already wrapped
*         around which can cause a situation in which the bucket's state is
*         mis-interpreted and the incoming packet is marked as non-conforming
*         even though the bucket was actually supposed to be full.
*         To prevent this from happening the CPU needs to trigger a meter
*         refresh transaction at least once every 10 minutes per meter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, entryIndex or
*                                       portGroupsBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Once the CPU triggers Refresh transaction the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupMeteringEntryRefresh
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex
)
{
    GT_U32      portGroupId;    /* port group id */
    GT_STATUS   retStatus;      /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Check index of Policer Metering Entry */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum, stage, entryIndex);

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        retStatus =  prvCpssDxCh3PolicerInternalTableAccess(
                                devNum, portGroupId,
                                stage, entryIndex,
                                PRV_CPSS_DXCH3_POLICER_METERING_REFRESH_E);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChPolicerPortGroupMeteringEntryRefresh function
* @endinternal
*
* @brief   Refresh the Policer Metering Entry.
*         Meter Bucket's state parameters refreshing are needed in order to
*         prevent mis-behavior due to wrap around of timers.
*         The wrap around problem can occur when there are long periods of
*         'silence' on the flow and the bucket's state parameters do not get
*         refreshed (meter is in the idle state). This causes a problem once the
*         flow traffic is re-started since the timers might have already wrapped
*         around which can cause a situation in which the bucket's state is
*         mis-interpreted and the incoming packet is marked as non-conforming
*         even though the bucket was actually supposed to be full.
*         To prevent this from happening the CPU needs to trigger a meter
*         refresh transaction at least once every 10 minutes per meter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, entryIndex or
*                                       portGroupsBmp
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note Once the CPU triggers Refresh transaction the policer accesses the
*       metering entry and performs metering refresh with the packet's Byte
*       Count set to zero.
*
*/
GT_STATUS cpssDxChPolicerPortGroupMeteringEntryRefresh
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              entryIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupMeteringEntryRefresh);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, entryIndex));

    rc = internal_cpssDxChPolicerPortGroupMeteringEntryRefresh(devNum, portGroupsBmp, stage, entryIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, entryIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortGroupErrorGet function
* @endinternal
*
* @brief   Gets address and type of Policer Entry that had an ECC error.
*         This information available if error is happened and not read till now.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] entryTypePtr             - pointer to the Type of Entry (Metering or Counting)
*                                      that had an error.
* @param[out] entryAddrPtr             - pointer to the Policer Entry that had an error.
*
* @retval GT_OK                    - on success.
* @retval GT_EMPTY                 - on missing error information.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupErrorGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT   *entryTypePtr,
    OUT GT_U32                              *entryAddrPtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regValue;       /* register value */
    GT_U32      entryTypeBit;   /* The bit number of entryType and the size of
                                   entry address in bits */
    GT_U32      addressBits;     /* The number of bits used for error address */
    GT_U32      portGroupId;    /* port group id */
    GT_STATUS   retStatus;      /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    /* the register  policerError, policerErrorCntr doesn't exists in Falcon
       anymore. Appropriate memories are protected by DFX.*/
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(entryTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(entryAddrPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerError;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                    PLR[stage].policerErrorReg;
    }

    retStatus = GT_EMPTY;

    /* initialize output parameters */
    *entryTypePtr = CPSS_DXCH3_POLICER_ENTRY_METERING_E;
    *entryAddrPtr = 0x0;

    /* Loop on all portGroups from the bitmap, untill first non-empty error is found */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)
    {

        /* Get Ingress Policer Error register */
        retStatus = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                                        regAddr, &regValue);
        if (GT_OK == retStatus)
        {
            if(!((regValue >> 31) & 0x1))
            {
                retStatus = GT_EMPTY;

                /* move to the next portGroupId */
                continue;
            }

            if (PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
            {
                /* Lion and above */
                entryTypeBit = 29;
                addressBits = 24;
            }
            else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                     (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E))
            {
                /* xCat */
                entryTypeBit = 24;
                addressBits = entryTypeBit;
            }
            else
            {
                /* DxCh3 */
                entryTypeBit = 30;
                addressBits = entryTypeBit;
            }

            *entryTypePtr = ((regValue >> entryTypeBit) & 0x1)?
                                        CPSS_DXCH3_POLICER_ENTRY_BILLING_E :
                                        CPSS_DXCH3_POLICER_ENTRY_METERING_E;

            *entryAddrPtr = regValue & (~(0xFFFFFFFF << addressBits));

            /* first non-empty error entry is found */
            break;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)


    return retStatus;
}

/**
* @internal cpssDxChPolicerPortGroupErrorGet function
* @endinternal
*
* @brief   Gets address and type of Policer Entry that had an ECC error.
*         This information available if error is happened and not read till now.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] entryTypePtr             - pointer to the Type of Entry (Metering or Counting)
*                                      that had an error.
* @param[out] entryAddrPtr             - pointer to the Policer Entry that had an error.
*
* @retval GT_OK                    - on success.
* @retval GT_EMPTY                 - on missing error information.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortGroupErrorGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT CPSS_DXCH3_POLICER_ENTRY_TYPE_ENT   *entryTypePtr,
    OUT GT_U32                              *entryAddrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupErrorGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, entryTypePtr, entryAddrPtr));

    rc = internal_cpssDxChPolicerPortGroupErrorGet(devNum, portGroupsBmp, stage, entryTypePtr, entryAddrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, entryTypePtr, entryAddrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortGroupErrorCounterGet function
* @endinternal
*
* @brief   Gets the value of the Policer ECC Error Counter.
*         The Error Counter is a free-running non-sticky 8-bit read-only
*         counter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] cntrValuePtr             - pointer to the Policer ECC Error counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupErrorCounterGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *cntrValuePtr
)
{
    GT_U32      regAddr;        /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* the register  policerError, policerErrorCntr doesn't exists in Falcon
       anymore. Appropriate memories are protected by DFX.*/
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerErrorCntr;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].
                                                            policerErrorCntrReg;
    }

    /* Get Ingress Policer Error Counter value */
    return prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp, regAddr,
                                                  0, 8, cntrValuePtr, NULL);
}

/**
* @internal cpssDxChPolicerPortGroupErrorCounterGet function
* @endinternal
*
* @brief   Gets the value of the Policer ECC Error Counter.
*         The Error Counter is a free-running non-sticky 8-bit read-only
*         counter.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; Falcon.
*
* @param[in] devNum                   - physical device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
*
* @param[out] cntrValuePtr             - pointer to the Policer ECC Error counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortGroupErrorCounterGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_U32                              *cntrValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupErrorCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, cntrValuePtr));

    rc = internal_cpssDxChPolicerPortGroupErrorCounterGet(devNum, portGroupsBmp, stage, cntrValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, cntrValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal sip5_20PolicerPortGroupManagementCountersOperation function
* @endinternal
*
* @brief   Gets/Reset the value of specified Management Counters.
*         the management counters need to be read by 'indirect action' (unlike previous devices)
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
* @param[in] triggerCmd               - Triggered command that the Policer needs to perform.
*                                      one of :
*                                      PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_READ_E
*                                      PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_RESET_E
*                                      PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_READ_AND_RESET_E
*
* @param[out] regAddrPtr               - (pointer to) the register address that we need to read
*                                      the values from.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, mngCntrType,
*                                       Management Counters Set number or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS sip5_20PolicerPortGroupManagementCountersOperation
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCntrSet,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    mngCntrType,
    IN PRV_CPSS_DXCH3_POLICER_ACCESS_CTRL_CMD_ENT  triggerCmd,
    OUT GT_U32                                  *regAddrPtr

)
{
    GT_STATUS   rc;
    GT_U32      portGroupId;    /* port group id */
    GT_U32  entryIndex = (mngCntrSet/*0..2*/ * 4) +
                          mngCntrType /*0..3*/;   /*0..11*/

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        /* Trigger the read & write action */
        rc = prvCpssDxCh3PolicerInternalTableAccess(devNum,
                              portGroupId,
                              stage,
                              entryIndex,
                              triggerCmd);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    if(triggerCmd == PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_RESET_E)
    {
        /* no more needed */
        return GT_OK;
    }

    if(regAddrPtr)
    {
        *regAddrPtr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerTableAccessData[0];
    }

    return GT_OK;
}


/**
* @internal internal_cpssDxChPolicerPortGroupManagementCountersSet function
* @endinternal
*
* @brief   Sets the value of specified Management Counters.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
* @param[in] mngCntrPtr               - pointer to the Management Counters Entry must be set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType,
*                                       Management Counters Set number or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupManagementCountersSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT      mngCntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT     mngCntrType,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC    *mngCntrPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      hwData[3];  /* data will be written to HW */
    GT_U32      wordEntryIndex; /* word base entry index in the Management Counters Table */
    GT_U32      numOfWords; /* number of words to write */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(mngCntrPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Verify Management Counters Set number and calculate Entry index */
    switch(mngCntrSet)
    {
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E:
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E:
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Verify Management Counters Type and calculate Entry index */
    switch(mngCntrType)
    {
        case CPSS_DXCH3_POLICER_MNG_CNTR_GREEN_E:
        case CPSS_DXCH3_POLICER_MNG_CNTR_YELLOW_E:
        case CPSS_DXCH3_POLICER_MNG_CNTR_RED_E:
        case CPSS_DXCH3_POLICER_MNG_CNTR_DROP_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if(mngCntrPtr->duMngCntr.l[0] ||
           mngCntrPtr->duMngCntr.l[1] ||
           mngCntrPtr->packetMngCntr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "The management counters can not be set to any value diff from 'ZERO'");
        }

        return sip5_20PolicerPortGroupManagementCountersOperation(devNum,
            portGroupsBmp, stage, mngCntrSet, mngCntrType,
            PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_RESET_E,
                NULL);
    }


    /* Nullify the hwData */
    cpssOsMemSet(hwData, 0, sizeof(hwData));

    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */

        /* calculate word index */
        wordEntryIndex = (mngCntrSet * 16) + (mngCntrType * 4);

        /* number of words to write */
        numOfWords = 3;

        /* Create HW format of Management Counters Entry */
        hwData[0] = mngCntrPtr->duMngCntr.l[0];
        hwData[1] = (mngCntrPtr->duMngCntr.l[1] & 0x3FF) |
                    ((mngCntrPtr->packetMngCntr & 0x3FFFFF) << 10);
        hwData[2] = (mngCntrPtr->packetMngCntr >> 22);

    }
    else
    {
        /* DxCh3 */

        /* calculate word index */
        wordEntryIndex = (mngCntrSet * 8) + (mngCntrType * 2);

        /* number of words to write */
        numOfWords = 2;

        /* Create HW Management Counters Entry */
        hwData[0] = mngCntrPtr->duMngCntr.l[0];
        hwData[1] = mngCntrPtr->packetMngCntr;

    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                    policerManagementCntrsTbl;
    }
    else
    {
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].policerManagementCntrsTbl;
    }

    regAddr += (0x4 * wordEntryIndex);
    /* Set Management Counters Entry */
    return prvCpssPortGroupsBmpCounterArraySet(devNum, portGroupsBmp,
                                               regAddr, numOfWords,
                                               &hwData[0]);
}

/**
* @internal cpssDxChPolicerPortGroupManagementCountersSet function
* @endinternal
*
* @brief   Sets the value of specified Management Counters.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
* @param[in] mngCntrPtr               - pointer to the Management Counters Entry must be set.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage or mngCntrType,
*                                       Management Counters Set number or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*
*/
GT_STATUS cpssDxChPolicerPortGroupManagementCountersSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT      mngCntrSet,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT     mngCntrType,
    IN CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC    *mngCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupManagementCountersSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, mngCntrSet, mngCntrType, mngCntrPtr));

    rc = internal_cpssDxChPolicerPortGroupManagementCountersSet(devNum, portGroupsBmp, stage, mngCntrSet, mngCntrType, mngCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, mngCntrSet, mngCntrType, mngCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortGroupManagementCountersGet function
* @endinternal
*
* @brief   Gets the value of specified Management Counters.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
*
* @param[out] mngCntrPtr               - pointer to the requested Management Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, mngCntrType,
*                                       Management Counters Set number or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupManagementCountersGet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCntrSet,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    mngCntrType,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC   *mngCntrPtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register hw value */
    GT_U32      wordEntryIndex; /* word base entry index in the Management Counters Table */
    GT_STATUS   retStatus;  /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(mngCntrPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Verify Management Counters Set number and calculate Entry index */
    switch(mngCntrSet)
    {
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E:
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E:
        case CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Verify Management Counters Type and calculate Entry index */
    switch(mngCntrType)
    {
        case CPSS_DXCH3_POLICER_MNG_CNTR_GREEN_E:
        case CPSS_DXCH3_POLICER_MNG_CNTR_YELLOW_E:
        case CPSS_DXCH3_POLICER_MNG_CNTR_RED_E:
        case CPSS_DXCH3_POLICER_MNG_CNTR_DROP_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* trigger read operation by using the 'indirect triggering' */
        /* and get the address from where to do the 'read' */
        retStatus = sip5_20PolicerPortGroupManagementCountersOperation(devNum,
            portGroupsBmp, stage, mngCntrSet, mngCntrType,
            PRV_CPSS_SIP_5_20_POLICER_MANAGEMENT_COUNTERS_READ_E,
            &regAddr);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }
    }
    else
    {
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat and above */

            /* calculate word index */
            wordEntryIndex = (mngCntrSet * 16) + (mngCntrType * 4);
        }
        else
        {
            /* DxCh3 */

            /* calculate word index */
            wordEntryIndex = (mngCntrSet * 8) + (mngCntrType * 2);
        }

        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                        policerManagementCntrsTbl;
        }
        else
        {
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].policerManagementCntrsTbl;
        }

        regAddr += (0x4 * wordEntryIndex);
    }

    /* Read a Data Unit counter part(Entry LSB) of Management Counters Entry */
    retStatus =
        prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp, regAddr,
                                               0,32, NULL, &mngCntrPtr->duMngCntr);
    if (retStatus != GT_OK)
    {
        return retStatus;
    }

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* next word in policerTableAccessData[] */
        regAddr += 0x4;
    }
    else
    /* Read LSB of Management Counters Entry from the PLR Shadow reg */
    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerShadow[0];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerShadowReg;
    }


    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* Read Policer Shadow0 register, where are located the     */
       /* MSB of DU counter[10 bit] and the LSB of Packet Counter [22 bit] */
        retStatus = prvCpssPortGroupsBmpMultiCounterSummary(devNum,
                                    portGroupsBmp, regAddr,
                                    0,10,
                                    &regValue,/* 10 MSbits of DU counter */
                                    10,22,
                                    &mngCntrPtr->packetMngCntr,/* 22 LSbits of Packt cntr */
                                    0,0,NULL);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        mngCntrPtr->duMngCntr.l[1] += regValue;

        /* Calculate the address of the Policer second shadow register */
        regAddr += 0x4;

        /* Read Ingress Policer Shadow1 register, where   */
        /* are located the MSB of Packet Counter [10 bit] */
        retStatus = prvCpssPortGroupsBmpCounterSummary(devNum,
                                                           portGroupsBmp,
                                                           regAddr,0,10,
                                                           &regValue,
                                                           NULL);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        mngCntrPtr->packetMngCntr += (regValue << 22);
    }
    else
    {
        /* DxCh3 */

        /* Get Packet counter part(Entry MSB) of Management Counters Entry */
        retStatus = prvCpssPortGroupsBmpCounterSummary(devNum,
                                               portGroupsBmp, regAddr, 0,32,
                                               &(mngCntrPtr->packetMngCntr),
                                               NULL);
    }

    return retStatus;
}

/**
* @internal cpssDxChPolicerPortGroupManagementCountersGet function
* @endinternal
*
* @brief   Gets the value of specified Management Counters.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] mngCntrSet               - Management Counters Set (APPLICABLE RANGES: 0..2).
* @param[in] mngCntrType              - Management Counters Type (GREEN, YELLOW, RED, DROP).
*
* @param[out] mngCntrPtr               - pointer to the requested Management Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, mngCntrType,
*                                       Management Counters Set number or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note In data unit management counter only 32 bits are used for DxCh3 devices
*       and 42 bits are used for xCat and above devices.
*
*/
GT_STATUS cpssDxChPolicerPortGroupManagementCountersGet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT     mngCntrSet,
    IN  CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT    mngCntrType,
    OUT CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC   *mngCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupManagementCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, mngCntrSet, mngCntrType, mngCntrPtr));

    rc = internal_cpssDxChPolicerPortGroupManagementCountersGet(devNum, portGroupsBmp, stage, mngCntrSet, mngCntrType, mngCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, mngCntrSet, mngCntrType, mngCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortGroupMeteringEntrySet function
* @endinternal
*
* @brief   Sets Metering Policer Entry.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] entryPtr                 - pointer to the metering policer entry going to be set.
*
* @param[out] tbParamsPtr              - pointer to actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage, entryIndex,
*                                       entry parameters or portGroupsBmp.
* @retval GT_OUT_OF_RANGE          - on Billing Entry Index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupMeteringEntrySet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbParamsPtr
)
{
    GT_U32              hwData;         /* data to write to HW */
    GT_U32              regAddr;        /* register address */
    GT_U32              hwRate0;        /* HW rate 0 value */
    GT_U32              hwRate1;        /* HW rate 1 value */
    GT_U32              hwBurstSize0;   /* HW Max Burst Size 0 value */
    GT_U32              hwBurstSize1;   /* HW Max Burst Size 1 value */
    GT_U32              hwRateType0;    /* HW rate type 0 value */
    GT_U32              hwRateType1;    /* HW rate type 1 value */
    GT_U32              hwBucketSize0;  /* HW Bucket Size 0 value */
    GT_U32              hwBucketSize1;  /* HW Bucket Size 1 value */
    GT_U32              portGroupId;    /* port group id */
    GT_STATUS           retStatus;      /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    CPSS_NULL_PTR_CHECK_MAC(tbParamsPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Check index of Policer Metering Entry */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,entryIndex);


    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        /* loop on all active port groups in the bmp */

        /* To write the rest of the entry,
          call private xCat A1 and above API */
        retStatus = prvCpssDxChPolicerMeteringEntrySet(
            devNum, stage, entryIndex,
            entryPtr, NULL/*envelopeCfgPtr*/,tbParamsPtr);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }
    }
    else
    {
        /* DxCh3 */

        /* Calculate Token Bucket  */
        switch(entryPtr->meterMode)
        {
            case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
                retStatus = prvCpssDxCh3PolicerHwTbCalculate(
                               devNum,
                               stage,
                               entryPtr->tokenBucketParams.srTcmParams.cir,
                               entryPtr->tokenBucketParams.srTcmParams.cbs,
                               entryPtr->tokenBucketParams.srTcmParams.ebs,
                               &(tbParamsPtr->srTcmParams.cir), /* Rate0 = Rate1 */
                               &(tbParamsPtr->srTcmParams.cbs), /* MaxBurstSize0 */
                               &(tbParamsPtr->srTcmParams.ebs), /* MaxBurstSize1 */
                               &hwRateType0);      /* RateType0 = RateType1 */
                if (retStatus != GT_OK)
                {
                    return retStatus;
                }
                hwRateType1 = hwRateType0;          /* RateType0 = RateType1 */
                hwRate0 = tbParamsPtr->srTcmParams.cir;
                hwRate1 = hwRate0;                              /* Rate0 = Rate1 */
                hwBurstSize0 = tbParamsPtr->srTcmParams.cbs;    /* MaxBurstSize0 */
                hwBurstSize1 = tbParamsPtr->srTcmParams.ebs;    /* MaxBurstSize1 */
                break;

            case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
                retStatus = prvCpssDxCh3PolicerHwTbCalculate(
                               devNum,
                               stage,
                               entryPtr->tokenBucketParams.trTcmParams.cir,
                               entryPtr->tokenBucketParams.trTcmParams.cbs,
                               0,                   /* Not exists in the TrTCM */
                               &(tbParamsPtr->trTcmParams.cir), /* Rate0 */
                               &(tbParamsPtr->trTcmParams.cbs), /* MaxBurstSize0 */
                               NULL,                /* Not exists in the TrTCM */
                               &hwRateType0);       /* RateType0 */
                if (retStatus != GT_OK)
                {
                    return retStatus;
                }
                hwRate0 = tbParamsPtr->trTcmParams.cir;         /* Rate0 */
                hwBurstSize0 = tbParamsPtr->trTcmParams.cbs;    /* MaxBurstSize0 */

                retStatus = prvCpssDxCh3PolicerHwTbCalculate(
                               devNum,
                               stage,
                               entryPtr->tokenBucketParams.trTcmParams.pir,
                               entryPtr->tokenBucketParams.trTcmParams.pbs,
                               0,                   /* Not exists in the TrTCM */
                               &(tbParamsPtr->trTcmParams.pir), /* Rate1 */
                               &(tbParamsPtr->trTcmParams.pbs), /* MaxBurstSize1 */
                               NULL,                /* Not exists in the TrTCM */
                               &hwRateType1);       /* RateType1 */
                if (retStatus != GT_OK)
                {
                    return retStatus;
                }
                hwRate1 = tbParamsPtr->trTcmParams.pir;         /* Rate1 */
                hwBurstSize1 = tbParamsPtr->trTcmParams.pbs;    /* MaxBurstSize1 */

                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /*************************************************/
        /* Convert actual TB parameters to the SW format */
        /*************************************************/
        retStatus = prvCpssDxCh3PolicerSwTbCalculate(entryPtr->meterMode,
                                                hwRateType0, hwRateType1,
                                                tbParamsPtr, tbParamsPtr);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        /* get bucket size values in bytes as Maximal Burst size of token bucket */
        if(entryPtr->meterMode == CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E)
        {
            hwBucketSize0 = tbParamsPtr->srTcmParams.cbs;
            hwBucketSize1 = tbParamsPtr->srTcmParams.ebs;
        }
        else
        {
            hwBucketSize0 = tbParamsPtr->trTcmParams.cbs;
            hwBucketSize1 = tbParamsPtr->trTcmParams.pbs;
        }
        /**********/
        /* Word 1 */
        /**********/
        /* set Bucket size 0 - LSB 6 bits to bits 26:31 */
        regAddr = 0x4 +
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerTblAccessDataReg;
        /* Set Ingress Policer Table Access Data 1 Register */
        retStatus = prvCpssHwPpSetRegField(devNum, regAddr, 26, 6,
                                              (hwBucketSize0 & 0x3f));
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        /**********/
        /* Word 2 */
        /**********/
        /* set Bucket size 0 - MSB 26 bits to bits 0:25,
           set Bucket size 1 - LSB 6 bits to bits 26:31 */
        hwData = (hwBucketSize0 >> 6) | ((hwBucketSize1 & 0x3f) << 26);

        regAddr = 0x8 +
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerTblAccessDataReg;
        /* Set Ingress Policer Table Access Data 1 Register */
        retStatus = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }
        /**********/
        /* Word 3 */
        /**********/
        /* set Bucket size 1 - MSB 26 bits to bits 0:25 */
        hwData = (hwBucketSize1 >> 6);

        /* Set Policer command for Red Packet */
        switch(entryPtr->redPcktCmd)
        {
            case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
                /* bits 26:27 = 0 */
                break;
            case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E:
                hwData |= (1 << 26);
                break;
            case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E:
                hwData |= (2 << 26);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Get Policer commands for Yellow Packet */
        switch(entryPtr->yellowPcktCmd)
        {
            case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E:
                /* bits 28:29 = 0 */
                break;
            case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E:
                hwData |= (1 << 28);
                break;
            case CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E:
                hwData |= (2 << 28);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Set 2 LSB of RateType0 value */
        hwData |= ((hwRateType0 & 0x3) << 30);

        regAddr = 0xC +
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerTblAccessDataReg;
        /* Set Ingress Policer Table Access Data 3 Register */
        retStatus = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        /**********/
        /* Word 4 */
        /**********/

        /* Set 1 MSB of RateType0 value */
        hwData = (hwRateType0 >> 2) & 0x1;
        /* Set RateType1 value */
        hwData |= (hwRateType1 << 1);

        /* Set Color Mode */
        switch(entryPtr->meterColorMode)
        {
            case CPSS_POLICER_COLOR_BLIND_E:
                break;
            case CPSS_POLICER_COLOR_AWARE_E:
                hwData |= (1 << 4);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Set Meter Mode */
        switch(entryPtr->meterMode)
        {
            case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
                break;
            case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
                hwData |= (1 << 5);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Set Rate 0 (CIR) */
        hwData |= (hwRate0 << 6);
        /* Set Rate 1 (PIR) */
        hwData |= (hwRate1 << 16);
        /* Set 6 LSB of Maximum burst size for bucket 0 */
        hwData |= ((hwBurstSize0 & 0x3F) << 26);

        regAddr = 0x10 +
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerTblAccessDataReg;

        /* Set Ingress Policer Table Access Data 4 Register */
        retStatus = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        /**********/
        /* Word 5 */
        /**********/

        /* Set 10 MSB of Maximum burst size for bucket 0 */
        hwData = ((hwBurstSize0 >> 6) & 0x3FF);
        /* Set Maximum burst size for bucket 1 */
        hwData |= (hwBurstSize1 << 10);

        /* Set Modify DSCP operation */
        switch(entryPtr->modifyDscp)
        {
            case CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E:
                break;
            case CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E:
                hwData |= (1 << 26);
                break;
            case CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E:
                hwData |= (2 << 26);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Set Modify UP operation */
        switch(entryPtr->modifyUp)
        {
            case CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E:
                break;
            case CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E:
                hwData |= (1 << 28);
                break;
            case CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E:
                hwData |= (2 << 28);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* FE#2046 policer - QoS attributes incorrect modification for out-of-profile traffic */
        if (PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                         PRV_CPSS_DXCH3_OUT_OF_PROFILE_QOS_ATTR_E))
        {
            if (entryPtr->yellowPcktCmd !=
                    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E &&
                entryPtr->redPcktCmd !=
                    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E)
            {
             /* Set zero to ModifyDSCP and ModifyUP bits if policer commands for
                Non-Conforming Yellows/Reds are different from CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E */
                hwData &= ~(0xf << 26);
            }
        }

        /* Set Management Counter Set */
        switch(entryPtr->mngCounterSet)
        {
            case CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E:
                break;
            case CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E:
                hwData |= (1 << 30);
                break;
            case CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E:
                hwData |= (2 << 30);
                break;
            case CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E:
                hwData |= (3 << 30);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regAddr = 0x14 +
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerTblAccessDataReg;
        /* Set Ingress Policer Table Access Data 5 Register */
        retStatus = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_CHEETAH3_E)
        {
            /* Word 6, Offset Formula: 0xC000030 + n*4 + max(n-5,0)*4:
                       where n (0-6) represents Word. For word 6 used additional
                       +4 in a address  */
            regAddr = 0x1C +
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerTblAccessDataReg;

        }
        else
        {
            /* Set Ingress Policer Table Access Data 6 Register */
            regAddr = 0x18 +
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerTblAccessDataReg;
        }

        /* Check index of Policer Billing Entry */
        if(entryPtr->countingEntryIndex >= BIT_10)/*10 bits*/
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* Set pointer to related billing entry (Billing Entry Index)*/
        hwData = entryPtr->countingEntryIndex;

        /* Set Ingress Policer Table Access Data 6 Register */
        retStatus = prvCpssHwPpSetRegField(devNum, regAddr, 0, 10, hwData);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }
    }


    /*******************************************************/
    /* Update Policer Metering Entry in the Internal Table */
    /*******************************************************/
    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        retStatus =
            prvCpssDxCh3PolicerInternalTableAccess(devNum, portGroupId, stage,
                             entryIndex,
                             PRV_CPSS_DXCH3_POLICER_METERING_UPDATE_E);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChPolicerPortGroupMeteringEntrySet function
* @endinternal
*
* @brief   Sets Metering Policer Entry.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] entryPtr                 - pointer to the metering policer entry going to be set.
*
* @param[out] tbParamsPtr              - pointer to actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage, entryIndex,
*                                       entry parameters or portGroupsBmp.
* @retval GT_OUT_OF_RANGE          - on Billing Entry Index out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPortGroupMeteringEntrySet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr,
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  *tbParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupMeteringEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, entryIndex, entryPtr, tbParamsPtr));

    rc = internal_cpssDxChPolicerPortGroupMeteringEntrySet(devNum, portGroupsBmp, stage, entryIndex, entryPtr, tbParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, entryIndex, entryPtr, tbParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPolicerPortGroupMeteringEntryGet function
* @endinternal
*
* @brief   Gets Metering Policer Entry configuration.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @param[out] entryPtr                 - pointer to the requested metering policer entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, entryIndex or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupMeteringEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr
)
{
    GT_U32              regAddr;        /* register address */
    GT_U32              hwData[5];      /* data read from HW */
    GT_U32              hwRateType0;    /* HW rate type 0 value */
    GT_U32              hwRateType1;    /* HW rate type 1 value */
    GT_U32              hwRate0;        /* HW rate 0 value */
    GT_U32              hwRate1;        /* HW rate 1 value */
    GT_U32              hwBurstSize0;   /* HW Max Burst Size 0 value */
    GT_U32              hwBurstSize1;   /* HW Max Burst Size 1 value */
    GT_U32              portGroupId;    /* port group id */
    GT_STATUS           retStatus;      /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Check index of Policer Metering Entry */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,entryIndex);

    /* Get the first active port group */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp,
                                                        portGroupId);

    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */

        retStatus = prvCpssDxChPolicerMeteringEntryGet(
            devNum, portGroupId,
            stage, entryIndex,
            entryPtr, NULL /*envelopeCfgPtr*/);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }
    }
    else
    {
        /* DxCh3 */

        /*********************************************************/
        /* Read Word 3,4,5,6 of requested Policer Metering Entry */
        /*********************************************************/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            PLR[stage].policerMeteringCountingTbl + (entryIndex * METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS) + 12;

        retStatus = prvCpssHwPpPortGroupReadRam(devNum, portGroupId,
                                                   regAddr, 4, &hwData[0]);
        if (retStatus != GT_OK)
        {
            return retStatus;
        }

     /* Get Policer command for Red Packet */
        switch((hwData[0] >> 26) & 0x3)
        {
            case 0:
                entryPtr->redPcktCmd =
                                    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
                break;
            case 1:
                entryPtr->redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
                break;
            case 2:
                entryPtr->redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* Get Policer command for Yellow Packet */
        switch((hwData[0] >> 28) & 0x3)
        {
            case 0:
                entryPtr->yellowPcktCmd =
                                    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
                break;
            case 1:
                entryPtr->yellowPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
                break;
            case 2:
                entryPtr->yellowPcktCmd =
                                    CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* Get Color Mode */
        entryPtr->meterColorMode = ((hwData[1] >> 4) & 0x1) ?
                                                CPSS_POLICER_COLOR_AWARE_E :
                                                CPSS_POLICER_COLOR_BLIND_E;

        /* Get Meter Mode */
        entryPtr->meterMode = ((hwData[1] >> 5) & 0x1) ?
                                 CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E :
                                 CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E;


        /* Get Modify DSCP operation */
        switch((hwData[2] >> 26) & 0x3)
        {
            case 0:
                entryPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                break;
            case 1:
                entryPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
                break;
            case 2:
                entryPtr->modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* Get Modify UP operation */
        switch((hwData[2] >> 28) & 0x3)
        {
            case 0:
                entryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
                break;
            case 1:
                entryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
                break;
            case 2:
                entryPtr->modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* Get Management Counter Set */
        switch((hwData[2] >> 30) & 0x3)
        {
            case 0:
                entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E;
                break;
            case 1:
                entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E;
                break;
            case 2:
                entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E;
                break;
            case 3:
                entryPtr->mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* never should be reached*/
        }

        /* Get Billing Entry Index */
        entryPtr->countingEntryIndex = hwData[3] & 0x3FF;

        /* Get RateType0 value */
        hwRateType0 = ((hwData[0] >> 30) & 0x3) | ((hwData[1] & 0x1) << 2);
        /* Get ateType0 value */
        hwRateType1 = (hwData[1] >> 1) & 0x7;
        /* Get Rate 0 (CIR) */
        hwRate0 = (hwData[1] >> 6) & 0x3FF;
        /* Get Rate 1 (PIR) */
        hwRate1 = (hwData[1] >> 16) & 0x3FF;
        /* Get Maximum burst size for bucket 0 */
        hwBurstSize0 = ((hwData[1] >> 26) & 0x3F) | ((hwData[2] & 0x3FF) << 6);
        /* Get Maximum burst size for bucket 1 */
        hwBurstSize1 = (hwData[2] >> 10) & 0xFFFF;

        /****************************************************/
        /* Convert Token Bucket Parameters in the SW format */
        /****************************************************/
        switch(entryPtr->meterMode)
        {
            case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
                entryPtr->tokenBucketParams.srTcmParams.cir = hwRate0;
                entryPtr->tokenBucketParams.srTcmParams.cbs = hwBurstSize0;
                entryPtr->tokenBucketParams.srTcmParams.ebs = hwBurstSize1;

                retStatus = prvCpssDxCh3PolicerSwTbCalculate(
                    entryPtr->meterMode,
                    hwRateType0,
                    0,
                    &(entryPtr->tokenBucketParams),
                    &(entryPtr->tokenBucketParams));

                if (retStatus != GT_OK)
                {
                    return retStatus;
                }
                break;

            case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
                entryPtr->tokenBucketParams.trTcmParams.cir = hwRate0;
                entryPtr->tokenBucketParams.trTcmParams.pir = hwRate1;
                entryPtr->tokenBucketParams.trTcmParams.cbs = hwBurstSize0;
                entryPtr->tokenBucketParams.trTcmParams.pbs = hwBurstSize1;

                retStatus = prvCpssDxCh3PolicerSwTbCalculate(
                    entryPtr->meterMode,
                    hwRateType0,
                    hwRateType1,
                    &(entryPtr->tokenBucketParams),
                    &(entryPtr->tokenBucketParams));

                if (retStatus != GT_OK)
                {
                    return retStatus;
                }
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* never should be reached*/
        }
    }

    return retStatus;
}

/**
* @internal cpssDxChPolicerPortGroupMeteringEntryGet function
* @endinternal
*
* @brief   Gets Metering Policer Entry configuration.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Metering Entry.
*                                      Range: see datasheet for specific device.
*
* @param[out] entryPtr                 - pointer to the requested metering policer entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum, stage, entryIndex or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
GT_STATUS cpssDxChPolicerPortGroupMeteringEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT        stage,
    IN  GT_U32                                  entryIndex,
    OUT CPSS_DXCH3_POLICER_METERING_ENTRY_STC   *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupMeteringEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, entryIndex, entryPtr));

    rc = internal_cpssDxChPolicerPortGroupMeteringEntryGet(devNum, portGroupsBmp, stage, entryIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, entryIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortGroupBillingEntrySet function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out upon counter reset by
*                                       indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, entryIndex or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*       In order to set Billing entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxChIpfixEntrySet".
*
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    GT_U32      regAddr;            /* register address */
    GT_U32      hwData[7];          /* HW data */
    GT_U32      portGroupId;        /* port group id */
    GT_STATUS   retStatus;          /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(billingCntrPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Check index of Policer Metering Entry */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,entryIndex);

    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        retStatus = prvCpssDxChPolicerBillingEntrySet(devNum, portGroupsBmp,
                                                      stage, entryIndex,
                                                      billingCntrPtr);
    }
    else
    {
        /* DxCh3 */

        /* Using indirect access to the IPLR Table in case of counter's reset */
        if((0 == billingCntrPtr->greenCntr.l[0]) &&
           (0 == billingCntrPtr->yellowCntr.l[0]) &&
           (0 == billingCntrPtr->redCntr.l[0]))
        {
            /* loop on all active port groups in the bmp */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
            {
                /* Perform reset by indirect access to the Policer Table */
                retStatus = prvCpssDxCh3PolicerInternalTableAccess(devNum, portGroupId,
                                                                   stage, entryIndex,
                                         PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_RESET_E);
                if (retStatus != GT_OK)
                {
                    return retStatus;
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

            return GT_OK;
        }

        /* Nullify the hwData */
        cpssOsMemSet(hwData, 0, sizeof(hwData));

        /* Remap Application Entry index to the HW Entry index */
        entryIndex += PRV_CPSS_DXCH3_POLICER_ENTRY_NUM_CNS;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        PLR[stage].policerMeteringCountingTbl + (entryIndex * METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS);

        /************************/
        /* Create HW entry data */
        /************************/
        hwData[0] = billingCntrPtr->greenCntr.l[0];
        hwData[1] = billingCntrPtr->yellowCntr.l[0];
        hwData[2] = billingCntrPtr->redCntr.l[0];

        /* Set counters value */
        retStatus = prvCpssPortGroupsBmpCounterArraySet(devNum, portGroupsBmp,
                                                            regAddr, 7, &hwData[0]);
    }

    return retStatus;
}

/**
* @internal cpssDxChPolicerPortGroupBillingEntrySet function
* @endinternal
*
* @brief   Sets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out upon counter reset by
*                                       indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, entryIndex or
*                                       portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*       None-zeroed setting of Billing Counters while metring/counting is
*       enabled may cause data coherency problems.
*       In order to set Billing entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*       This API address the same memory area (counters place) as can be set
*       by "cpssDxChIpfixEntrySet".
*
*/
GT_STATUS cpssDxChPolicerPortGroupBillingEntrySet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupBillingEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, entryIndex, billingCntrPtr));

    rc = internal_cpssDxChPolicerPortGroupBillingEntrySet(devNum, portGroupsBmp, stage, entryIndex, billingCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, entryIndex, billingCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortGroupBillingEntryGet function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] reset                    -  flag:
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read counters operation only.
*
* @param[out] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, entryIndex or
*                                       portGroupsBmp.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  GT_BOOL                                 reset,
    OUT CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    GT_U32      regAddr;            /* register address */
    GT_U32      portGroupId;        /* port group id */
    GT_STATUS   retStatus;          /* generic return status code */
    GT_U32      ii;                 /* iterator */

    PRV_CPSS_DXCH3_POLICER_ACCESS_CTRL_CMD_ENT  accessAction;/* access action */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);

    CPSS_NULL_PTR_CHECK_MAC(billingCntrPtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* Check index of Policer Metering Entry */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,entryIndex);

    if(reset == GT_TRUE)
    {
        /* Define Policer Table Indirect Access triggered action */
        accessAction = PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_RESET_E;

        if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && PRV_CPSS_SIP_5_CHECK_MAC(devNum))
        {
            /* BC2 devices (SIP 5 and 5.10) need to use read-and-write operation
               to avoid reset of configuration fields. But at first need to read
               entry to get configuration fields */
            accessAction = PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E;

            /* loop on all active port groups in the bmp */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
            {
                /* Perform indirect access to the Policer Table */
                retStatus = prvCpssDxCh3PolicerInternalTableAccess(devNum,
                                                                   portGroupId,
                                                                   stage,
                                                                   entryIndex,
                                                                   accessAction);
                if (GT_OK != retStatus)
                {
                    return retStatus;
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

            /* clear counters bits 0:125, bits 139:126 are reserved.
               write 0 to 4 first registers (bits 0:127)for the operation. */
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerTableAccessData[0];
            for(ii = 0; ii < 4; ii++)
            {
                retStatus = prvCpssHwPpWriteRegister(devNum, regAddr, 0);
                if (GT_OK != retStatus)
                {
                    return retStatus;
                }

                regAddr += 4;
            }

            /* second operation should be read-and-write */
            accessAction = PRV_CPSS_DXCH3_POLICER_CNTR_READ_AND_WRITE_E;
        }
    }
    else
    {
        /* Define Policer Table Indirect Access triggered action */
        accessAction = PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E;
    }

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        /* Perform indirect access to the Policer Table */
        retStatus = prvCpssDxCh3PolicerInternalTableAccess(devNum,
                                                           portGroupId,
                                                           stage,
                                                           entryIndex,
                                                           accessAction);
        if (GT_OK != retStatus)
        {
            return retStatus;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* xCat and above */
        retStatus = prvCpssDxChPolicerBillingEntryGet(devNum,
                                                      portGroupsBmp,
                                                      stage,
                                                      billingCntrPtr);
    }
    else
    {
        /* DxCh3 and xCat A0 */

        /* check if the device type is xCat A0 */
        if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
        {
            /* xCat A0 */

            /* configure Billing counter mode */
            retStatus = prvCpssDxChPolicerBillingCountersModeGet(devNum,
                                            &billingCntrPtr->billingCntrMode);
            if (retStatus != GT_OK)
            {
                return retStatus;
            }
        }

        /******************************************************************/
        /* Get counter value from the corresponding Access Data Registers */
        /******************************************************************/

        /* Get Ingress Policer Table Access Data 0 Register Address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[stage].policerTblAccessDataReg;

        /* Get value of Green Billing counter */
        retStatus = prvCpssPortGroupsBmpCounterSummary(devNum,
                                        portGroupsBmp, regAddr,0,32,
                                        &(billingCntrPtr->greenCntr.l[0]),NULL);
        if (GT_OK != retStatus)
        {
            return retStatus;
        }

        /* Get value of Yellow Billing counter */
        regAddr += 4;
        retStatus = prvCpssPortGroupsBmpCounterSummary(devNum,
                                       portGroupsBmp, regAddr,0,32,
                                       &(billingCntrPtr->yellowCntr.l[0]),NULL);

        if (GT_OK == retStatus)
        {
            /* Get value of Red Billing counter */
            regAddr += 4;
            retStatus = prvCpssPortGroupsBmpCounterSummary(devNum,
                                         portGroupsBmp, regAddr,0,32,
                                         &(billingCntrPtr->redCntr.l[0]),NULL);
        }
    }

    return retStatus;
}

/**
* @internal cpssDxChPolicerPortGroupBillingEntryGet function
* @endinternal
*
* @brief   Gets Policer Billing Counters.
*         Billing is the process of counting the amount of traffic that belongs
*         to a flow. This process can be activated only by metering and is used
*         either for statistical analysis of a flow's traffic or for actual
*         billing.
*         The billing counters are free-running no-sticky counters.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number.
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
*                                      Stage type is significant for xCat and above devices
*                                      and ignored by DxCh3.
* @param[in] entryIndex               - index of Policer Billing Counters Entry.
*                                      Range: see datasheet for specific device.
* @param[in] reset                    -  flag:
*                                      GT_TRUE  - performing read and reset atomic operation.
*                                      GT_FALSE - performing read counters operation only.
*
* @param[out] billingCntrPtr           - pointer to the Billing Counters Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage, entryIndex or
*                                       portGroupsBmp.
* @retval GT_BAD_STATE             - on counter entry type mismatch.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*
* @note The packet is counted per outgoing DP or Conformance Level. When
*       metering is disabled, billing is always done according to packet's
*       Drop Precedence.
*       When metering is activated per source port, Billing counting cannot
*       be enabled.
*
*/
GT_STATUS cpssDxChPolicerPortGroupBillingEntryGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  GT_U32                                  entryIndex,
    IN  GT_BOOL                                 reset,
    OUT CPSS_DXCH3_POLICER_BILLING_ENTRY_STC    *billingCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupBillingEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, entryIndex, reset, billingCntrPtr));

    rc = internal_cpssDxChPolicerPortGroupBillingEntryGet(devNum, portGroupsBmp, stage, entryIndex, reset, billingCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, entryIndex, reset, billingCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortGroupPolicyCntrSet function
* @endinternal
*
* @brief   Sets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
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
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
* @param[in] cntrValue                - packets counter.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index (index range is limited by
*                                       max number of Policer Policy counters) or portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*       In order to set Policy counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupPolicyCntrSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              index,
    IN  GT_U32                              cntrValue
)
{
    GT_U32  countNumber;    /* the number of counter in the Entry */
    GT_U32  entryIndex;     /* entry index in the Internal Table */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* calculate the index inside Policer Counting Table */
    /* Policy Entry allignment is 8, only 6 indexes are in use. */
    entryIndex = (index >> 3);

    /* calculate the word offset inside the entry */
    countNumber = (index & 0x7);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        /* word offsets 4,5,6 and 7 are not valid */
        if (countNumber > 3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* word offsets 6 and 7 are not valid */
        if ((countNumber == 6) || (countNumber == 7))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* Set counters value */
    return prvCpssDxChPolicerCountingEntryWordSet(devNum, portGroupsBmp, stage,
                                                  entryIndex, countNumber,
                                                  cntrValue);
}

/**
* @internal cpssDxChPolicerPortGroupPolicyCntrSet function
* @endinternal
*
* @brief   Sets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
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
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
* @param[in] cntrValue                - packets counter.
*                                      For counter reset this parameter should be nullified.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or index (index range is limited by
*                                       max number of Policer Policy counters) or portGroupsBmp.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*       In order to set Policy counting entry under traffic perform the following
*       algorithm:
*       - disconnect entryes from traffic
*       - call cpssDxChPolicerCountingWriteBackCacheFlush
*       - set new values
*       - connect entries back to traffic
*
*/
GT_STATUS cpssDxChPolicerPortGroupPolicyCntrSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_U32                              index,
    IN  GT_U32                              cntrValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupPolicyCntrSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, index, cntrValue));

    rc = internal_cpssDxChPolicerPortGroupPolicyCntrSet(devNum, portGroupsBmp, stage, index, cntrValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, index, cntrValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortGroupPolicyCntrGet function
* @endinternal
*
* @brief   Gets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
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
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
*
* @param[out] cntrValuePtr             - Pointer to the packet counter.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, index or portGroupsBmp.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupPolicyCntrGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    OUT GT_U32                              *cntrValuePtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  countNumber;    /* the number of counter in the Entry */
    GT_U32  entryIndex;     /* entry index in the Internal Table */
    GT_STATUS rc;           /* return code                      */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(cntrValuePtr);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* calculate the index inside Policer Counting Table */
    entryIndex = (index >> 3);

    /* check allocated number of counters - Policy Entry allignment is 8,
       only 6 indexes are in use. */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum, stage, entryIndex);

    /* calculate the word offset inside the entry */
    countNumber = (index & 0x7);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E)
    {
        /* word offsets 4,5,6 and 7 are not valid */
        if (countNumber > 3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* word offsets 6 and 7 are not valid */
        if ((countNumber == 6) || (countNumber == 7))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* Check if Policer Counting memory errata is enabled */
    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                          PRV_CPSS_DXCH_LION_POLICER_COUNTING_MEM_ACCESS_WA_E))
    {
        /* Trigger Indirect Read from Counting memory */
        rc = prvCpssDxCh3PolicerInternalTableAccess(devNum,
                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                              stage,
                              entryIndex,
                              PRV_CPSS_DXCH3_POLICER_CNTR_READ_ONLY_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Get Ingress Policer Table Access Data 0 Register Address */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                        PLR[stage].policerTblAccessDataReg;
    }
    else
    {
        /* Direct access to counting memory */
        if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCountingTbl;
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                            PLR[stage].policerCountingTbl;
        }

        regAddr += (entryIndex * METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS);
    }

    /* calculate the word address inside the table */
    regAddr += (countNumber * 0x4);

    /* Get counters value */
    return prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp,
                                                  regAddr, 0, 32, cntrValuePtr,
                                                  NULL);
}

/**
* @internal cpssDxChPolicerPortGroupPolicyCntrGet function
* @endinternal
*
* @brief   Gets Policer Policy Counters located in the Policer Counting Entry upon
*         enabled Policy Counting mode.
*         When working in Policy Counting Mode each counter counts the number of
*         packets matching on a a Policy rules.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
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
* @param[in] index                    -  set by Policy Action Entry or Metering entry
*                                      and belongs to the range from 0 up to maximal number
*                                      of Policer Policy counters.
*                                      Maximal number is number of policer entries for
*                                      counting is number of counting entries
*                                      multiplied by 8.
*                                      Some indexes in the range are not valid. Not valid
* @param[in] index                    are:
*                                      - (index % 8) == 6
*                                      - (index % 8) == 7
*                                      e.g. not valid indexes 6, 7, 14, 15, 22, 23 ...
*
* @param[out] cntrValuePtr             - Pointer to the packet counter.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on disabled Policy Counting.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, index or portGroupsBmp.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_TIMEOUT               - on time out.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Policy counters are free-running no-sticky counters.
*
*/
GT_STATUS cpssDxChPolicerPortGroupPolicyCntrGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN  GT_U32                              index,
    OUT GT_U32                              *cntrValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupPolicyCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, stage, index, cntrValuePtr));

    rc = internal_cpssDxChPolicerPortGroupPolicyCntrGet(devNum, portGroupsBmp, stage, index, cntrValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, stage, index, cntrValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

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
)
{
    GT_U32      regAddr;                /* register address */
    GT_U32      portGroupId;            /* port group id    */
    GT_STATUS   retStatus;              /* return code      */

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).
                                                policerTableAccessCtrl;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        PLR[stage].policerTblAccessControlReg;
    }

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        retStatus = prvCpssPortGroupBusyWait(devNum,portGroupId,regAddr,0,GT_FALSE);
        if (GT_OK != retStatus)
        {
            return retStatus;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal internal_cpssDxChPolicerTrappedPacketsBillingEnableSet function
* @endinternal
*
* @brief   Enables the billing algorithm for Trapped packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - enable/disable billing for Trapped packets.
*                                      GT_TRUE  - enable billing for Trapped packets.
*                                      GT_FALSE - disable billing for Trapped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerTrappedPacketsBillingEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_BOOL                              enable
)
{
    GT_U32      regAddr;     /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl1;

    /* Set Policer Control1 register, <Enable_Billing_for_Trapped_Packets> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 5, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChPolicerTrappedPacketsBillingEnableSet function
* @endinternal
*
* @brief   Enables the billing algorithm for Trapped packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - enable/disable billing for Trapped packets.
*                                      GT_TRUE  - enable billing for Trapped packets.
*                                      GT_FALSE - disable billing for Trapped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerTrappedPacketsBillingEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_BOOL                              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerTrappedPacketsBillingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enable));

    rc = internal_cpssDxChPolicerTrappedPacketsBillingEnableSet(devNum, stage, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerTrappedPacketsBillingEnableGet function
* @endinternal
*
* @brief   Get state of the billing algorithm for Trapped packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - (pointer to) enable/disable billing for Trapped packets.
*                                      GT_TRUE  - enable billing for Trapped packets.
*                                      GT_FALSE - disable billing for Trapped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerTrappedPacketsBillingEnableGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      regValue;    /* register value */
    GT_STATUS   retStatus;   /* generic return status code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl1;

    /* Get Policer Control1 register, <Enable_Billing_for_Trapped_Packets> field */
    retStatus = prvCpssHwPpGetRegField(devNum, regAddr, 5, 1, &regValue);

    if (GT_OK != retStatus)
    {
        return retStatus;
    }

    /* Convert HW Value to the billing state */
    *enablePtr = (0 == regValue) ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssDxChPolicerTrappedPacketsBillingEnableGet function
* @endinternal
*
* @brief   Get state of the billing algorithm for Trapped packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - (pointer to) enable/disable billing for Trapped packets.
*                                      GT_TRUE  - enable billing for Trapped packets.
*                                      GT_FALSE - disable billing for Trapped packets.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or stage.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerTrappedPacketsBillingEnableGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerTrappedPacketsBillingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enablePtr));

    rc = internal_cpssDxChPolicerTrappedPacketsBillingEnableGet(devNum, stage, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerLossMeasurementCounterCaptureEnableSet function
* @endinternal
*
* @brief   Enables or disables Loss Measurement capture.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - enable/disable Loss Measurement capture.
*                                      GT_TRUE  - enable capture of Loss Measurement packets.
*                                      GT_FALSE - disable capture of Loss Measurement packets.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerLossMeasurementCounterCaptureEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_BOOL                              enable
)
{
    GT_U32 regAddr;                 /* Register address */
    GT_U32 value;                   /* Register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl1;

    value = BOOL2BIT_MAC(enable);

    /* Set Policer Control1 register, <enable_counter_capture_for_lm> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 7, 1, value);
}

/**
* @internal cpssDxChPolicerLossMeasurementCounterCaptureEnableSet function
* @endinternal
*
* @brief   Enables or disables Loss Measurement capture.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - enable/disable Loss Measurement capture.
*                                      GT_TRUE  - enable capture of Loss Measurement packets.
*                                      GT_FALSE - disable capture of Loss Measurement packets.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChPolicerLossMeasurementCounterCaptureEnableSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    IN GT_BOOL                              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerLossMeasurementCounterCaptureEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enable));

    rc = internal_cpssDxChPolicerLossMeasurementCounterCaptureEnableSet(devNum, stage, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerLossMeasurementCounterCaptureEnableGet function
* @endinternal
*
* @brief   Get Loss Measurement capture state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - (pointer to)enable/disable Loss Measurement capture.
*                                      GT_TRUE  - enable capture of Loss Measurement packets.
*                                      GT_FALSE - disable capture of Loss Measurement packets.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerLossMeasurementCounterCaptureEnableGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_U32      regAddr;                 /* Register address */
    GT_U32      value;                   /* Register value */
    GT_STATUS   rc;                      /* Return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl1;

    /* Get Policer Control1 register, <enable_counter_capture_for_lm> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 7, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPolicerLossMeasurementCounterCaptureEnableGet function
* @endinternal
*
* @brief   Get Loss Measurement capture state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - (pointer to)enable/disable Loss Measurement capture.
*                                      GT_TRUE  - enable capture of Loss Measurement packets.
*                                      GT_FALSE - disable capture of Loss Measurement packets.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChPolicerLossMeasurementCounterCaptureEnableGet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT     stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerLossMeasurementCounterCaptureEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enablePtr));

    rc = internal_cpssDxChPolicerLossMeasurementCounterCaptureEnableGet(devNum, stage, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerEAttributesMeteringModeSet function
* @endinternal
*
* @brief   Set ePort/eVLAN attributes metering mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - ePort/eVLAN Metering mode
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerEAttributesMeteringModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT  mode
)
{
    GT_U32      regAddr;                /* register address     */
    GT_U32      value;                  /* value to write to hw */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    switch(mode)
    {
        case CPSS_DXCH_POLICER_E_ATTR_METER_MODE_DISABLED_E:
            value = 0;
            break;
        case CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EPORT_ENABLED_E:
            value = 1;
            break;
        case CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl1;

    /* Set Policer Control1 register, <e_attributes_metering_mode> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 8, 2, value);
}

/**
* @internal cpssDxChPolicerEAttributesMeteringModeSet function
* @endinternal
*
* @brief   Set ePort/eVLAN attributes metering mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] mode                     - ePort/eVLAN Metering mode
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or mode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerEAttributesMeteringModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT  mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerEAttributesMeteringModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, mode));

    rc = internal_cpssDxChPolicerEAttributesMeteringModeSet(devNum, stage, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerEAttributesMeteringModeGet function
* @endinternal
*
* @brief   Get ePort/eVLAN attributes metering mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) ePort/eVLAN Metering mode
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
static GT_STATUS internal_cpssDxChPolicerEAttributesMeteringModeGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT *modePtr
)
{
    GT_U32      regAddr;                 /* register address */
    GT_U32      value;                   /* register value */
    GT_STATUS   rc;                      /* return code  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
          CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl1;

    /* Get Policer Control1 register, <e_attributes_metering_mode> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 8, 2, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Convert value to e-attribute meter mode */
    switch(value)
    {
        case 0:
            *modePtr = CPSS_DXCH_POLICER_E_ATTR_METER_MODE_DISABLED_E;
            break;
        case 1:
            *modePtr = CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EPORT_ENABLED_E;
            break;
        case 2:
            *modePtr = CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPolicerEAttributesMeteringModeGet function
* @endinternal
*
* @brief   Get ePort/eVLAN attributes metering mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] modePtr                  - (pointer to) ePort/eVLAN Metering mode
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on bad value in a entry.
*/
GT_STATUS cpssDxChPolicerEAttributesMeteringModeGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerEAttributesMeteringModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, modePtr));

    rc = internal_cpssDxChPolicerEAttributesMeteringModeGet(devNum, stage, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerFlowIdCountingCfgSet function
* @endinternal
*
* @brief   Set Flow Id based counting configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] billingIndexCfgPtr       - (pointer to) billing index configuration structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on out of range data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerFlowIdCountingCfgSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC  *billingIndexCfgPtr
)
{
    GT_STATUS   rc;                 /* Return code      */
    GT_U32 regAddr;                 /* Register address */
    GT_U32 hwMode;                  /* HW mode          */
    GT_U32 value;                   /* Register value   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E
        | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(billingIndexCfgPtr);

    switch (billingIndexCfgPtr->billingIndexMode)
    {
        case CPSS_DXCH_POLICER_BILLING_INDEX_MODE_STANDARD_E:
            hwMode = 0;
            break;
        case CPSS_DXCH_POLICER_BILLING_INDEX_MODE_FLOW_ID_E:
            hwMode = 1;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (billingIndexCfgPtr->billingMinFlowId >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (billingIndexCfgPtr->billingMaxFlowId >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (billingIndexCfgPtr->billingFlowIdIndexBase >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr =
        PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).globalBillingCntrIndexingModeConfig0;

    value = (hwMode | (billingIndexCfgPtr->billingFlowIdIndexBase << 16));

    rc = prvCpssHwPpWriteRegBitMask(devNum, regAddr, 0xFFFF0001, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr =
        PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).globalBillingCntrIndexingModeConfig1;

    value =
        ((billingIndexCfgPtr->billingMaxFlowId << 16)
        | billingIndexCfgPtr->billingMinFlowId);

    rc = prvCpssHwPpWriteRegister(devNum, regAddr, value);

    return rc;
}

/**
* @internal cpssDxChPolicerFlowIdCountingCfgSet function
* @endinternal
*
* @brief   Set Flow Id based counting configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] billingIndexCfgPtr       - (pointer to) billing index configuration structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on out of range data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChPolicerFlowIdCountingCfgSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    IN  CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC  *billingIndexCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerFlowIdCountingCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, billingIndexCfgPtr));

    rc = internal_cpssDxChPolicerFlowIdCountingCfgSet(devNum, stage, billingIndexCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, billingIndexCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerFlowIdCountingCfgGet function
* @endinternal
*
* @brief   Get Flow Id based counting configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] billingIndexCfgPtr       - (pointer to) billing index configuration structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerFlowIdCountingCfgGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC  *billingIndexCfgPtr
)
{
    GT_STATUS   rc;                 /* Return code      */
    GT_U32 regAddr;                 /* Register address */
    GT_U32 hwMode;                  /* HW mode          */
    GT_U32 value;                   /* Register value   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E
        | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(billingIndexCfgPtr);

    regAddr =
        PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).globalBillingCntrIndexingModeConfig0;

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwMode = (value & 1);

    billingIndexCfgPtr->billingIndexMode =
        (hwMode == 0)
            ? CPSS_DXCH_POLICER_BILLING_INDEX_MODE_STANDARD_E
            : CPSS_DXCH_POLICER_BILLING_INDEX_MODE_FLOW_ID_E;

    billingIndexCfgPtr->billingFlowIdIndexBase = ((value >> 16) & 0xFFFF);

    regAddr =
        PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).globalBillingCntrIndexingModeConfig1;

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    billingIndexCfgPtr->billingMaxFlowId = ((value >> 16) & 0xFFFF);
    billingIndexCfgPtr->billingMinFlowId = (value & 0xFFFF);

    return GT_OK;
}

/**
* @internal cpssDxChPolicerFlowIdCountingCfgGet function
* @endinternal
*
* @brief   Get Flow Id based counting configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] billingIndexCfgPtr       - (pointer to) billing index configuration structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChPolicerFlowIdCountingCfgGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage,
    OUT CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC  *billingIndexCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerFlowIdCountingCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, billingIndexCfgPtr));

    rc = internal_cpssDxChPolicerFlowIdCountingCfgGet(devNum, stage, billingIndexCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, billingIndexCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerHierarchicalTableEntrySet function
* @endinternal
*
* @brief   Set Hierarchical Table Entry configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Hierarchical Table Entry index.
* @param[in] entryPtr                 - (pointer to) Hierarchical Table Entry structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or index
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on out of range data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Applicable only to Ingress #0 Policer Stage.
*
*/
static GT_STATUS internal_cpssDxChPolicerHierarchicalTableEntrySet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          index,
    IN  CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC  *entryPtr
)
{
    GT_U32 hwData; /* HW data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E
        | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /* Check index of Policer Hierarchical Table Entry */
    /* will be done in table writing function         */


    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (entryPtr->policerPointer >= BIT_12)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        hwData = ((BOOL2BIT_MAC(entryPtr->countingEnable) << 13) |
                  (BOOL2BIT_MAC(entryPtr->meteringEnable) << 12) |
                  entryPtr->policerPointer);
    }
    else
    {
        if (entryPtr->policerPointer >= BIT_16)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        hwData = ((BOOL2BIT_MAC(entryPtr->countingEnable) << 17) |
                  (BOOL2BIT_MAC(entryPtr->meteringEnable) << 16) |
                  entryPtr->policerPointer);
    }

    return  prvCpssDxChWriteTableEntry(
        devNum,
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_HIERARCHICAL_E,
        index,
        &hwData);
}

/**
* @internal cpssDxChPolicerHierarchicalTableEntrySet function
* @endinternal
*
* @brief   Set Hierarchical Table Entry configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Hierarchical Table Entry index.
* @param[in] entryPtr                 - (pointer to) Hierarchical Table Entry structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or index
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_RANGE          - on out of range data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Applicable only to Ingress #0 Policer Stage.
*
*/
GT_STATUS cpssDxChPolicerHierarchicalTableEntrySet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          index,
    IN  CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC  *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerHierarchicalTableEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChPolicerHierarchicalTableEntrySet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerHierarchicalTableEntryGet function
* @endinternal
*
* @brief   Get Hierarchical Table Entry configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Hierarchical Table Entry index.
*
* @param[out] entryPtr                 - (pointer to) Hierarchical Table Entry structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or index
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Applicable only to Ingress #0 Policer Stage.
*
*/
static GT_STATUS internal_cpssDxChPolicerHierarchicalTableEntryGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          index,
    OUT CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC  *entryPtr
)
{
    GT_STATUS   rc;     /* Return code  */
    GT_U32      hwData; /* HW data      */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E
        | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /* Check index of Policer Hierarchical Table Entry */
    /* will be done in table reading function          */

    rc =  prvCpssDxChReadTableEntry(
        devNum,
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_HIERARCHICAL_E,
        index,
        &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        entryPtr->countingEnable = BIT2BOOL_MAC(((hwData >> 13) & 1));
        entryPtr->meteringEnable = BIT2BOOL_MAC(((hwData >> 12) & 1));
        entryPtr->policerPointer = (hwData & 0x0FFF);
    }
    else
    {
        entryPtr->countingEnable = BIT2BOOL_MAC(((hwData >> 17) & 1));
        entryPtr->meteringEnable = BIT2BOOL_MAC(((hwData >> 16) & 1));
        entryPtr->policerPointer = (hwData & 0xFFFF);
    }


    return GT_OK;
}

/**
* @internal cpssDxChPolicerHierarchicalTableEntryGet function
* @endinternal
*
* @brief   Get Hierarchical Table Entry configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Hierarchical Table Entry index.
*
* @param[out] entryPtr                 - (pointer to) Hierarchical Table Entry structure
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or stage or index
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Applicable only to Ingress #0 Policer Stage.
*
*/
GT_STATUS cpssDxChPolicerHierarchicalTableEntryGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          index,
    OUT CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC  *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerHierarchicalTableEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChPolicerHierarchicalTableEntryGet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerSecondStageIndexMaskSet function
* @endinternal
*
* @brief   Sets second stage policer index mask.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] plrIndexMask             - mask for policer index
*                                      (APPLICABLE RANGES: Lion 0..0xFFF, Lion2 0..0xFFFF).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on illegal plrIndexMask
*
* @note Note: Lion device applicable starting from revision B2
*       Lion2 device applicable starting from revision B1
*
*/
static GT_STATUS internal_cpssDxChPolicerSecondStageIndexMaskSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  plrIndexMask
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      regAddr;        /* register address */
    GT_U32      maxValue;       /* max mask value */
    GT_U32      regOffset;      /* register field offset */
    GT_U32      length;         /* register field length */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E );

    /* supported only in Lion starting revision B2 and Lion2 starting revision B1 */
    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E && (PRV_CPSS_PP_MAC(devNum)->revision <= 3)) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E && (PRV_CPSS_PP_MAC(devNum)->revision <= 1)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        maxValue = BIT_12;
        regOffset = 20;
        length = 12;
    }
    else
    {   /* Lion2 case */
        maxValue = BIT_16;
        regOffset = 16;
        length = 16;
    }

    if(plrIndexMask >= maxValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* IPLR1 register - 2nd stage plr index mask */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E].metalFix;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset, length, plrIndexMask);

    return rc;
}

/**
* @internal cpssDxChPolicerSecondStageIndexMaskSet function
* @endinternal
*
* @brief   Sets second stage policer index mask.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] plrIndexMask             - mask for policer index
*                                      (APPLICABLE RANGES: Lion 0..0xFFF, Lion2 0..0xFFFF).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on illegal plrIndexMask
*
* @note Note: Lion device applicable starting from revision B2
*       Lion2 device applicable starting from revision B1
*
*/
GT_STATUS cpssDxChPolicerSecondStageIndexMaskSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  plrIndexMask
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerSecondStageIndexMaskSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, plrIndexMask));

    rc = internal_cpssDxChPolicerSecondStageIndexMaskSet(devNum, plrIndexMask);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, plrIndexMask));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPolicerSecondStageIndexMaskGet function
* @endinternal
*
* @brief   Gets second stage policer index mask.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] plrIndexMaskPtr          - (pointer to) mask for policer index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Note: Lion device applicable starting from revision B2
*       Lion2 device applicable starting from revision B1
*
*/
static GT_STATUS internal_cpssDxChPolicerSecondStageIndexMaskGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *plrIndexMaskPtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      regAddr;        /* register address */
    GT_U32      regOffset;      /* register field offset */
    GT_U32      length;         /* register field length */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_XCAT2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(plrIndexMaskPtr);

    /* supported only in Lion starting revision B2 and Lion2 starting revision B1 */
    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E && (PRV_CPSS_PP_MAC(devNum)->revision <= 3)) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E && (PRV_CPSS_PP_MAC(devNum)->revision <= 1)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION_E)
    {
        regOffset = 20;
        length = 12;
    }
    else
    {   /* Lion2 case */
        regOffset = 16;
        length = 16;
    }

    /* IPLR1 register - 2nd stage plr index mask */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E].metalFix;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, regOffset, length, plrIndexMaskPtr);

    return rc;
}

/**
* @internal cpssDxChPolicerSecondStageIndexMaskGet function
* @endinternal
*
* @brief   Gets second stage policer index mask.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
*
* @param[out] plrIndexMaskPtr          - (pointer to) mask for policer index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Note: Lion device applicable starting from revision B2
*       Lion2 device applicable starting from revision B1
*
*/
GT_STATUS cpssDxChPolicerSecondStageIndexMaskGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *plrIndexMaskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerSecondStageIndexMaskGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, plrIndexMaskPtr));

    rc = internal_cpssDxChPolicerSecondStageIndexMaskGet(devNum, plrIndexMaskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, plrIndexMaskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortStormTypeInFlowModeEnableSet function
* @endinternal
*
* @brief   Enable port-based metering triggered in Flow mode to work in
*         (storm-pkt-type, port) based, otherwise (port) based is used.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - Enable\Disable Port metering according to storm type
*                                      when working in flow mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerPortStormTypeInFlowModeEnableSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_BOOL                             enable
)
{
    GT_U32    regAddr;  /* register address */
    GT_U32    regValue; /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControl2Reg;

    regValue = BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(devNum, regAddr, 5, 1, regValue);
}

/**
* @internal cpssDxChPolicerPortStormTypeInFlowModeEnableSet function
* @endinternal
*
* @brief   Enable port-based metering triggered in Flow mode to work in
*         (storm-pkt-type, port) based, otherwise (port) based is used.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] enable                   - Enable\Disable Port metering according to storm type
*                                      when working in flow mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerPortStormTypeInFlowModeEnableSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortStormTypeInFlowModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enable));

    rc = internal_cpssDxChPolicerPortStormTypeInFlowModeEnableSet(devNum, stage, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortStormTypeInFlowModeEnableGet function
* @endinternal
*
* @brief   Get if port-based metering triggered in Flow mode is enabled for
*         (storm-pkt-type, port) based (otherwise (port) based is used).
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - (pointer to) Enable status of Port metering according to storm type
*                                      when working in flow mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerPortStormTypeInFlowModeEnableGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;   /* return code */
    GT_U32    regAddr;  /* register address */
    GT_U32    regValue; /* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |CPSS_XCAT_E |
         CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                                PLR[stage].policerControl2Reg;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 5, 1, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}

/**
* @internal cpssDxChPolicerPortStormTypeInFlowModeEnableGet function
* @endinternal
*
* @brief   Get if port-based metering triggered in Flow mode is enabled for
*         (storm-pkt-type, port) based (otherwise (port) based is used).
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] enablePtr                - (pointer to) Enable status of Port metering according to storm type
*                                      when working in flow mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or stage
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerPortStormTypeInFlowModeEnableGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    OUT GT_BOOL                             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortStormTypeInFlowModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enablePtr));

    rc = internal_cpssDxChPolicerPortStormTypeInFlowModeEnableGet(devNum, stage, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPacketSizeModeForTimestampTagSet function
* @endinternal
*
* @brief   Determines whether the timestamp tag should be included in the byte count
*         computation or not.
*         The configuration is used for IPFIX, VLAN and Management counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] timestampTagMode         - packet size counting mode for timestamp tag.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or timestampTagMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerPacketSizeModeForTimestampTagSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT                stage,
    IN  CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT     timestampTagMode
)
{
    GT_U32      regAddr;                /* register address     */
    GT_U32      value;                  /* value to write to hw */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    switch(timestampTagMode)
    {
        case CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E:
            value = 0;
            break;
        case CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;

    /* Set Policer Control0 register, <Timestamp Tag Counting Mode> field */
    return prvCpssHwPpSetRegField(devNum, regAddr, 4, 1, value);
}

/**
* @internal cpssDxChPolicerPacketSizeModeForTimestampTagSet function
* @endinternal
*
* @brief   Determines whether the timestamp tag should be included in the byte count
*         computation or not.
*         The configuration is used for IPFIX, VLAN and Management counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] timestampTagMode         - packet size counting mode for timestamp tag.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage or timestampTagMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPacketSizeModeForTimestampTagSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT                stage,
    IN  CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT     timestampTagMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPacketSizeModeForTimestampTagSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, timestampTagMode));

    rc = internal_cpssDxChPolicerPacketSizeModeForTimestampTagSet(devNum, stage, timestampTagMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, timestampTagMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPacketSizeModeForTimestampTagGet function
* @endinternal
*
* @brief   Retrieves whether the timestamp tag should be included in the byte count
*         computation or not.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] timestampTagModePtr      - (pointer to) packet size counting mode for timestamp tag.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerPacketSizeModeForTimestampTagGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT                stage,
    OUT CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT    *timestampTagModePtr
)
{
    GT_U32      regAddr;                /* register address     */
    GT_U32      value;                  /* value to read from hw */
    GT_STATUS   rc;                     /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
          CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(timestampTagModePtr);

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCtrl0;

    /* Read Policer Control0 register, <Timestamp Tag Counting Mode> field */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 4, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *timestampTagModePtr = (value) ? CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E :
                                     CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E;

    return GT_OK;
}

/**
* @internal cpssDxChPolicerPacketSizeModeForTimestampTagGet function
* @endinternal
*
* @brief   Retrieves whether the timestamp tag should be included in the byte count
*         computation or not.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] timestampTagModePtr      - (pointer to) packet size counting mode for timestamp tag.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum, stage.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerPacketSizeModeForTimestampTagGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT                stage,
    OUT CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT    *timestampTagModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPacketSizeModeForTimestampTagGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, timestampTagModePtr));

    rc = internal_cpssDxChPolicerPacketSizeModeForTimestampTagGet(devNum, stage, timestampTagModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, timestampTagModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Enable: Flow Based Metering to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Metering to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_BOOL                       enable
)
{
    GT_U32      value;      /* value to write         */
    GT_U32      regAddr;    /* register address       */
    GT_STATUS   rc;         /* function return code   */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_XCAT2_E |
         CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

    value = BOOL2BIT_MAC(enable);

        /* write value to hardware register */
    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeGlobalConfigRegArray[2];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 16, 1, value);

    return rc;
}

/**
* @internal cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Enable: Flow Based Metering to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Metering to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_BOOL                       enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Enable: Flow Based Metering to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Metering to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                       *enablePtr
)
{
    GT_U32      value;      /* value to write         */
    GT_U32      regAddr;    /* register address       */
    GT_STATUS   rc;         /* function return code   */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_XCAT2_E |
         CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

    /* read value from hardware register */
    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeGlobalConfigRegArray[2];
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 16, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr  = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Metering to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Enable: Flow Based Metering to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Metering to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                       *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPolicerFlowBasedMeteringToFloodedOnlyEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Enable: Flow Based Billing to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Billing to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_BOOL                       enable
)
{
    GT_U32      value;      /* value to write         */
    GT_U32      regAddr;    /* register address       */
    GT_STATUS   rc;         /* function return code   */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_XCAT2_E |
         CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

    value = BOOL2BIT_MAC(enable);

        /* write value to hardware register */
    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeGlobalConfigRegArray[2];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 18, 1, value);

    return rc;
}

/**
* @internal cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet function
* @endinternal
*
* @brief   Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Enable: Flow Based Billing to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Billing to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet
(
    IN  GT_U8                         devNum,
    IN  GT_BOOL                       enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Enable: Flow Based Billing to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Billing to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                       *enablePtr
)
{
    GT_U32      value;      /* value to write         */
    GT_U32      regAddr;    /* register address       */
    GT_STATUS   rc;         /* function return code   */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E | CPSS_LION_E | CPSS_XCAT2_E |
         CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

    /* read value from hardware register */
    regAddr =
        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.bridgeGlobalConfigRegArray[2];
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 18, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr  = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet function
* @endinternal
*
* @brief   Gets Enable/Disable Flow Based Billing to Flooded Traffic Only.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion; xCat2; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - Enable: Flow Based Billing to Flooded Traffic Only.
*                                      GT_FALSE - Disable: Flow Based Billing to All Traffic.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet
(
    IN  GT_U8                         devNum,
    OUT GT_BOOL                       *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPolicerFlowBasedBillingToFloodedOnlyEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerMeteringEntryEnvelopeSet function
* @endinternal
*
* @brief   Sets Envelope of Metering Policer Entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startEntryIndex          - index of Start Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] envelopeSize             - amount of entries in the envelope
* @param[in] couplingFlag0            - in MEF 10.3, the CIR bucket of the bottom rank in the
*                                      MEF 10.3 envelope may either spill the overflow to
*                                      the EIR bucket of the highest rank or not, according
*                                      to coupling flag 0.
*                                      GT_FALSE = DONT SPILL: The extra tokens of the CIR bucket
*                                      are not spilled to the EIR bucket.
*                                      GT_TRUE = SPILL: The extra tokens of the CIR bucket
*                                      are spilled to the EIR bucket.
* @param[in] entryArr[]               - array of the metering policer entries going to be set.
*
* @param[out] tbParamsArr[]            - array of actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex or
*                                       entry parameters.
* @retval GT_OUT_OF_RANGE          - on some parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS internal_cpssDxChPolicerMeteringEntryEnvelopeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN  GT_U32                                      startEntryIndex,
    IN  GT_U32                                      envelopeSize,
    IN  GT_BOOL                                     couplingFlag0,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC       entryArr[],
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT      tbParamsArr[]
)
{
    GT_STATUS                                     rc;         /* return code */
    PRV_CPSS_DXCH_POLICER_ENVELOPE_MEMBER_CFG_STC prvEnvelopeCfgArr[
        PRV_CPSS_DXCH_POLICER_MAX_ENVELOPE_SIZE_CNS]; /* work array of bucket pair parameters*/
    GT_U32                                         i;         /* loop counter */
    GT_U32                                         bucketIdx; /* bucket index */
    CPSS_DXCH3_POLICER_METER_MODE_ENT              mode;      /* meter mode */
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC          entryCopy; /* metering entry copy */
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT         tbParams;  /* resulting TB parameters */
    GT_U32                                         portGroupId; /* port Group Id */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E |
        CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E |
        CPSS_XCAT2_E | CPSS_LION2_E);
    if (! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    /* Check index of Policer Metering Entry */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,startEntryIndex);
    if (envelopeSize > PRV_CPSS_DXCH_POLICER_MAX_ENVELOPE_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(entryArr);
    CPSS_NULL_PTR_CHECK_MAC(tbParamsArr);

    /* mode for first entry */
    mode = CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E;
    for (i = 0; (i < envelopeSize); i++)
    {
        if (entryArr[i].meterMode != mode)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* mode for all not first entries */
        mode = CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E;
    }

    /* shadow of several fields in HW bucket entry taqble */
    for (i = 0; (i < envelopeSize); i++)
    {
        /* regardless whether the buckets will be reordered             */
        /* couplingFlag and maxRateIndex copied from the relevant entry */
        /* couplingFlag0 and envelopeSize written to each entry         */
        prvEnvelopeCfgArr[i].envelopeSize  = envelopeSize;
        prvEnvelopeCfgArr[i].couplingFlag  = entryArr[envelopeSize - 1 - i].couplingFlag;
        prvEnvelopeCfgArr[i].couplingFlag0 = couplingFlag0;
        prvEnvelopeCfgArr[i].maxRateIndex  = entryArr[envelopeSize - 1 - i].maxRateIndex;
    }

    if (couplingFlag0 == GT_FALSE)
    {
        for (i = 0; (i < envelopeSize); i++)
        {
           prvEnvelopeCfgArr[i].bucket0Rank   = (envelopeSize - i);
           prvEnvelopeCfgArr[i].bucket0Color  = PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E;
           prvEnvelopeCfgArr[i].bucket1Rank   = (envelopeSize - i);
           prvEnvelopeCfgArr[i].bucket1Color  = PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_YELLOW_E;
        }
    }
    else
    {
        for (i = 0; (i < envelopeSize); i++)
        {
            /* reorder buckets */
            /* Commited bucket */
            bucketIdx = i;
            if ((bucketIdx & 1) == 0) /* even bucket */
            {
                prvEnvelopeCfgArr[bucketIdx / 2].bucket0Rank  = (envelopeSize - i);
                prvEnvelopeCfgArr[bucketIdx / 2].bucket0Color = PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E;
            }
            else /* odd bucket */
            {
                prvEnvelopeCfgArr[bucketIdx / 2].bucket1Rank  = (envelopeSize - i);
                prvEnvelopeCfgArr[bucketIdx / 2].bucket1Color = PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E;
            }
            /* Excess bucket */
            bucketIdx = envelopeSize + i;
            if ((bucketIdx & 1) == 0) /* even bucket */
            {
                prvEnvelopeCfgArr[bucketIdx / 2].bucket0Rank  = (envelopeSize - i);
                prvEnvelopeCfgArr[bucketIdx / 2].bucket0Color = PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_YELLOW_E;
            }
            else /* odd bucket */
            {
                prvEnvelopeCfgArr[bucketIdx / 2].bucket1Rank  = (envelopeSize - i);
                prvEnvelopeCfgArr[bucketIdx / 2].bucket1Color = PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_YELLOW_E;
            }
        }
    }

    for (i = 0; (i < envelopeSize); i++)
    {
        cpssOsMemCpy(&entryCopy, &(entryArr[i]), sizeof(entryCopy));
        /* reorder buckets */
        /* Commited bucket */
        bucketIdx = prvEnvelopeCfgArr[i].bucket0Rank - 1;
        if (prvEnvelopeCfgArr[i].bucket0Color ==
            PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E)
        {
            entryCopy.tokenBucketParams.envelope.cbs =
                entryArr[bucketIdx].tokenBucketParams.envelope.cbs;
            entryCopy.tokenBucketParams.envelope.cir =
                entryArr[bucketIdx].tokenBucketParams.envelope.cir;
        }
        else
        {
            entryCopy.tokenBucketParams.envelope.cbs =
                entryArr[bucketIdx].tokenBucketParams.envelope.ebs;
            entryCopy.tokenBucketParams.envelope.cir =
                entryArr[bucketIdx].tokenBucketParams.envelope.eir;
        }
        /* Excess bucket */
        bucketIdx = prvEnvelopeCfgArr[i].bucket1Rank - 1;
        if (prvEnvelopeCfgArr[i].bucket1Color ==
            PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E)
        {
            entryCopy.tokenBucketParams.envelope.ebs =
                entryArr[bucketIdx].tokenBucketParams.envelope.cbs;
            entryCopy.tokenBucketParams.envelope.eir =
                entryArr[bucketIdx].tokenBucketParams.envelope.cir;
        }
        else
        {
            entryCopy.tokenBucketParams.envelope.ebs =
                entryArr[bucketIdx].tokenBucketParams.envelope.ebs;
            entryCopy.tokenBucketParams.envelope.eir =
                entryArr[bucketIdx].tokenBucketParams.envelope.eir;
        }

        rc = prvCpssDxChPolicerMeteringEntrySet(
            devNum, stage, (startEntryIndex + i),
            &entryCopy, &(prvEnvelopeCfgArr[i]), &tbParams);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Write Policer Metering Entry indirect access control register */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,portGroupId)
        {
            rc = prvCpssDxCh3PolicerInternalTableAccess(
                devNum, portGroupId, stage, ((startEntryIndex + i)),
                PRV_CPSS_DXCH3_POLICER_METERING_UPDATE_E);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,portGroupId)

        /* Commited bucket */
        bucketIdx = prvEnvelopeCfgArr[i].bucket0Rank - 1;
        if (prvEnvelopeCfgArr[bucketIdx].bucket0Color ==
            PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E)
        {
            tbParamsArr[bucketIdx].envelope.cbs = tbParams.envelope.cbs;
            tbParamsArr[bucketIdx].envelope.cir = tbParams.envelope.cir;
        }
        else
        {
            tbParamsArr[bucketIdx].envelope.ebs = tbParams.envelope.cbs;
            tbParamsArr[bucketIdx].envelope.eir = tbParams.envelope.cir;
        }
        /* Excess bucket */
        bucketIdx = prvEnvelopeCfgArr[i].bucket1Rank - 1;
        if (prvEnvelopeCfgArr[bucketIdx].bucket1Color ==
            PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E)
        {
            tbParamsArr[bucketIdx].envelope.cbs = tbParams.envelope.ebs;
            tbParamsArr[bucketIdx].envelope.cir = tbParams.envelope.eir;
        }
        else
        {
            tbParamsArr[bucketIdx].envelope.ebs = tbParams.envelope.ebs;
            tbParamsArr[bucketIdx].envelope.eir = tbParams.envelope.eir;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPolicerMeteringEntryEnvelopeSet function
* @endinternal
*
* @brief   Sets Envelope of Metering Policer Entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startEntryIndex          - index of Start Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] envelopeSize             - amount of entries in the envelope
* @param[in] couplingFlag0            - in MEF 10.3, the CIR bucket of the bottom rank in the
*                                      MEF 10.3 envelope may either spill the overflow to
*                                      the EIR bucket of the highest rank or not, according
*                                      to coupling flag 0.
*                                      GT_FALSE = DONT SPILL: The extra tokens of the CIR bucket
*                                      are not spilled to the EIR bucket.
*                                      GT_TRUE = SPILL: The extra tokens of the CIR bucket
*                                      are spilled to the EIR bucket.
* @param[in] entryArr[]               - array of the metering policer entries going to be set.
*
* @param[out] tbParamsArr[]            - array of actual policer token bucket parameters.
*                                      The token bucket parameters are returned as output
*                                      values. This is due to the hardware rate resolution,
*                                      the exact rate or burst size requested may not be
*                                      honored. The returned value gives the user the
*                                      actual parameters configured in the hardware.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex or
*                                       entry parameters.
* @retval GT_OUT_OF_RANGE          - on some parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerMeteringEntryEnvelopeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN  GT_U32                                      startEntryIndex,
    IN  GT_U32                                      envelopeSize,
    IN  GT_BOOL                                     couplingFlag0,
    IN  CPSS_DXCH3_POLICER_METERING_ENTRY_STC       entryArr[],
    OUT CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT      tbParamsArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMeteringEntryEnvelopeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, stage, startEntryIndex, envelopeSize,
         couplingFlag0, entryArr, tbParamsArr));

    rc = internal_cpssDxChPolicerMeteringEntryEnvelopeSet(
        devNum, stage, startEntryIndex, envelopeSize,
         couplingFlag0, entryArr, tbParamsArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, stage, startEntryIndex, envelopeSize,
         couplingFlag0, entryArr, tbParamsArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerMeteringEntryEnvelopeGet function
* @endinternal
*
* @brief   Gets Envelope of Metering Policer Entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startEntryIndex          - index of Start Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] maxEnvelopeSize          - amount of entries in output arrays
*
* @param[out] envelopeSizePtr          - (pointer to) amount of entries in the envelope
* @param[out] couplingFlag0Ptr         - (pointer to) GT_TRUE - spill overflow of the low level
*                                      Commited bucket to the high level Excess bucket,
*                                      GT_FALSE - drop overflow of the low level Commited bucket
* @param[out] couplingFlag0Ptr         - (pointer to)   in MEF 10.3, the CIR bucket of the bottom
*                                      rank in the MEF 10.3 envelope may either spill the overflow
*                                      to the EIR bucket of the highest rank or not, according
*                                      to coupling flag 0.
*                                      GT_FALSE = DONT SPILL: The extra tokens of the CIR bucket
*                                      are not spilled to the EIR bucket.
*                                      GT_TRUE = SPILL: The extra tokens of the CIR bucket
*                                      are spilled to the EIR bucket.
* @param[out] entryArr[]               - array of the metering policer entries going to be get.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex or
*                                       entry parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
static GT_STATUS internal_cpssDxChPolicerMeteringEntryEnvelopeGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      startEntryIndex,
    IN   GT_U32                                      maxEnvelopeSize,
    OUT  GT_U32                                      *envelopeSizePtr,
    OUT  GT_BOOL                                     *couplingFlag0Ptr,
    OUT  CPSS_DXCH3_POLICER_METERING_ENTRY_STC       entryArr[]
)
{
    PRV_CPSS_DXCH_POLICER_ENVELOPE_MEMBER_CFG_STC prvEnvelopeCfgArr[
        PRV_CPSS_DXCH_POLICER_MAX_ENVELOPE_SIZE_CNS]; /* work array of bucket pair parameters */
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT        tbParamsArr[
        PRV_CPSS_DXCH_POLICER_MAX_ENVELOPE_SIZE_CNS]; /* resulting TB parameters   */

    struct
    {
        GT_U32  cIndex;               /* commited bucket index             */
        GT_U32  cBktNum;              /* commited bucket number 0 or 1     */
        GT_U32  eIndex;               /* extended bucket index             */
        GT_U32  eBktNum;              /* extended bucket number 0 or 1     */
    } reorder[PRV_CPSS_DXCH_POLICER_MAX_ENVELOPE_SIZE_CNS];

    GT_STATUS       rc;               /* return code        */
    GT_U32          i;                /* loop counter       */
    GT_U32          bucketIdx;        /* bucket index       */
    GT_U32          bucketRank;       /* bucket rank        */
    GT_U32          portGroupId;      /* port Group Id      */
    GT_U32          envelopeSize;     /* envelope Size      */
    GT_BOOL         couplingFlag0;    /* coupling Flag0     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    /* Check index of Policer Metering Entry */
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum,stage,startEntryIndex);
    CPSS_NULL_PTR_CHECK_MAC(envelopeSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(couplingFlag0Ptr);
    CPSS_NULL_PTR_CHECK_MAC(entryArr);

    /* Get the first active port group */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, portGroupId);

    /* initial value */
    envelopeSize = PRV_CPSS_DXCH_POLICER_MAX_ENVELOPE_SIZE_CNS;
    for (i = 0; (i < envelopeSize); i++)
    {
        rc = prvCpssDxChPolicerMeteringEntryGet(
            devNum, portGroupId, stage, (startEntryIndex + i),
            &(entryArr[i]), &(prvEnvelopeCfgArr[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
        if (i == 0)
        {
            if (entryArr[i].meterMode !=
                CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            /* envelopeSize from the first entry */
            envelopeSize = prvEnvelopeCfgArr[i].envelopeSize;
            if ((envelopeSize > PRV_CPSS_DXCH_POLICER_MAX_ENVELOPE_SIZE_CNS)
                || (envelopeSize < 2))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            if (envelopeSize > maxEnvelopeSize)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            if (entryArr[i].meterMode !=
                CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }
    *envelopeSizePtr = envelopeSize;
    for (i = 0; (i < envelopeSize); i++)
    {
        /* fields from bucket table in oposite order */
        /* copy when all entries areade read         */
        entryArr[i].couplingFlag = prvEnvelopeCfgArr[envelopeSize - 1 - i].couplingFlag;
        entryArr[i].maxRateIndex = prvEnvelopeCfgArr[envelopeSize - 1 - i].maxRateIndex;
    }

    /* get couplingFlag0 from Green Bucket with maximal rank */
    couplingFlag0 = GT_FALSE;
    bucketRank = 0;
    for (i = 0; (i < envelopeSize); i++)
    {
        if ((prvEnvelopeCfgArr[i].bucket0Color ==
            PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E)
            && (prvEnvelopeCfgArr[i].bucket0Rank >= bucketRank))
        {
            couplingFlag0 = prvEnvelopeCfgArr[i].couplingFlag0;
            bucketRank = prvEnvelopeCfgArr[i].bucket0Rank;
        }
        if ((prvEnvelopeCfgArr[i].bucket1Color ==
            PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E)
            && (prvEnvelopeCfgArr[i].bucket1Rank >= bucketRank))
        {
            couplingFlag0 = prvEnvelopeCfgArr[i].couplingFlag0;
            bucketRank = prvEnvelopeCfgArr[i].bucket1Rank;
        }
    }
    *couplingFlag0Ptr = couplingFlag0;

    /* init reodrer array */
    for (i = 0; (i < envelopeSize); i++)
    {
        reorder[i].cIndex = 0xFFFF;
        reorder[i].eIndex = 0xFFFF;
    }

    /* fill reodrer array */
    for (i = 0; (i < envelopeSize); i++)
    {
        bucketIdx = prvEnvelopeCfgArr[i].bucket0Rank - 1;
        if (bucketIdx >= envelopeSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        if (prvEnvelopeCfgArr[i].bucket0Color ==
            PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E)
        {
            if (reorder[bucketIdx].cIndex < envelopeSize)
            {
                /* already filled */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            reorder[bucketIdx].cIndex = i;
            reorder[bucketIdx].cBktNum = 0;
        }
        else
        {
            if (reorder[bucketIdx].eIndex < envelopeSize)
            {
                /* already filled */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            reorder[bucketIdx].eIndex = i;
            reorder[bucketIdx].eBktNum = 0;
        }
        bucketIdx = prvEnvelopeCfgArr[i].bucket1Rank - 1;
        if (bucketIdx >= envelopeSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        if (prvEnvelopeCfgArr[i].bucket1Color ==
            PRV_CPSS_DXCH_POLICER_BUCKET_COLOR_GREEN_E)
        {
            if (reorder[bucketIdx].cIndex < envelopeSize)
            {
                /* already filled */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            reorder[bucketIdx].cIndex = i;
            reorder[bucketIdx].cBktNum = 1;
        }
        else
        {
            if (reorder[bucketIdx].eIndex < envelopeSize)
            {
                /* already filled */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            reorder[bucketIdx].eIndex = i;
            reorder[bucketIdx].eBktNum = 1;
        }
    }

    /* reorder buckets */
    /* copy to work array */
    for (i = 0; (i < envelopeSize); i++)
    {
        cpssOsMemCpy(&(tbParamsArr[i]), &(entryArr[i].tokenBucketParams), sizeof(tbParamsArr[i]));
    }
    /* back copy from work array */
    for (i = 0; (i < envelopeSize); i++)
    {
        /* commited bucket */
        bucketIdx = reorder[i].cIndex;
        if (reorder[i].cBktNum == 0)
        {
            entryArr[i].tokenBucketParams.envelope.cbs =
                tbParamsArr[bucketIdx].envelope.cbs;
            entryArr[i].tokenBucketParams.envelope.cir =
                tbParamsArr[bucketIdx].envelope.cir;
        }
        else
        {
            entryArr[i].tokenBucketParams.envelope.cbs =
                tbParamsArr[bucketIdx].envelope.ebs;
            entryArr[i].tokenBucketParams.trTcmParams.cir =
                tbParamsArr[bucketIdx].envelope.eir;
        }
        /* extended bucket */
        bucketIdx = reorder[i].eIndex;
        if (reorder[i].eBktNum == 0)
        {
            entryArr[i].tokenBucketParams.envelope.ebs =
                tbParamsArr[bucketIdx].envelope.cbs;
            entryArr[i].tokenBucketParams.envelope.eir =
                tbParamsArr[bucketIdx].envelope.cir;
        }
        else
        {
            entryArr[i].tokenBucketParams.envelope.ebs =
                tbParamsArr[bucketIdx].envelope.ebs;
            entryArr[i].tokenBucketParams.envelope.eir =
                tbParamsArr[bucketIdx].envelope.eir;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPolicerMeteringEntryEnvelopeGet function
* @endinternal
*
* @brief   Gets Envelope of Metering Policer Entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] startEntryIndex          - index of Start Policer Metering Entry.
*                                      Range: see datasheet for specific device.
* @param[in] maxEnvelopeSize          - amount of entries in output arrays
*
* @param[out] envelopeSizePtr          - (pointer to) amount of entries in the envelope
* @param[out] couplingFlag0Ptr         - (pointer to)   in MEF 10.3, the CIR bucket of the bottom
*                                      rank in the MEF 10.3 envelope may either spill the overflow
*                                      to the EIR bucket of the highest rank or not, according
*                                      to coupling flag 0.
*                                      GT_FALSE = DONT SPILL: The extra tokens of the CIR bucket
*                                      are not spilled to the EIR bucket.
*                                      GT_TRUE = SPILL: The extra tokens of the CIR bucket
*                                      are spilled to the EIR bucket.
* @param[out] entryArr[]               - array of the metering policer entries going to be get.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_TIMEOUT               - on time out of Policer Tables
*                                       indirect access.
* @retval GT_BAD_PARAM             - wrong devNum, stage or entryIndex or
*                                       entry parameters.
* @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
*/
GT_STATUS cpssDxChPolicerMeteringEntryEnvelopeGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      startEntryIndex,
    IN   GT_U32                                      maxEnvelopeSize,
    OUT  GT_U32                                      *envelopeSizePtr,
    OUT  GT_BOOL                                     *couplingFlag0Ptr,
    OUT  CPSS_DXCH3_POLICER_METERING_ENTRY_STC       entryArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMeteringEntryEnvelopeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, stage, startEntryIndex, maxEnvelopeSize,
         envelopeSizePtr, couplingFlag0Ptr, entryArr));

    rc = internal_cpssDxChPolicerMeteringEntryEnvelopeGet(
        devNum, stage, startEntryIndex, maxEnvelopeSize,
         envelopeSizePtr, couplingFlag0Ptr, entryArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, stage, startEntryIndex, maxEnvelopeSize,
         envelopeSizePtr, couplingFlag0Ptr, entryArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerMeterTableFlowBasedIndexConfigSet function
* @endinternal
*
* @brief   Set Policy Pointer threshold and maximal envelope size for using the
*         Flow based algorithm. For Policy Pointer values greater or equal
*         than threshold the Priority based algorithm used.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] threshold                - upper  of Policy Pointer for Flow Based algorithm.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
* @param[in] maxSize                  - maximal envelope size and alignment.
*                                      the index of start entry of each envelope
*                                      accessed by Flow Based Algorithm should be
*                                      aligned to maxSize.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChPolicerMeterTableFlowBasedIndexConfigSet
(
    IN  GT_U8                                          devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT               stage,
    IN  GT_U32                                         threshold,
    IN  CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT        maxSize
)
{
    GT_U32      mask;       /* value to write         */
    GT_U32      regAddr;    /* register address       */
    GT_STATUS   rc;         /* function return code   */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    if (threshold >= BIT_17)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    switch (maxSize)
    {
        case CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_2_E:
            mask = 0x1FFFE;
            break;
        case CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_4_E:
            mask = 0x1FFFC;
            break;
        case CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_8_E:
            mask = 0x1FFF8;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* write value to hardware register */
    regAddr =
        PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).meteringAddressingModeCfg0;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 17, threshold);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr =
        PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).meteringAddressingModeCfg1;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 17, mask);

    return rc;
}

/**
* @internal cpssDxChPolicerMeterTableFlowBasedIndexConfigSet function
* @endinternal
*
* @brief   Set Policy Pointer threshold and maximal envelope size for using the
*         Flow based algorithm. For Policy Pointer values greater or equal
*         than threshold the Priority based algorithm used.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] threshold                - upper  of Policy Pointer for Flow Based algorithm.
*                                      (APPLICABLE RANGES: 0..0x1FFFF)
* @param[in] maxSize                  - maximal envelope size and alignment.
*                                      the index of start entry of each envelope
*                                      accessed by Flow Based Algorithm should be
*                                      aligned to maxSize.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerMeterTableFlowBasedIndexConfigSet
(
    IN  GT_U8                                          devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT               stage,
    IN  GT_U32                                         threshold,
    IN  CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT        maxSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMeterTableFlowBasedIndexConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, stage, threshold, maxSize));

    rc = internal_cpssDxChPolicerMeterTableFlowBasedIndexConfigSet(
        devNum, stage, threshold, maxSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, stage, threshold, maxSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerMeterTableFlowBasedIndexConfigGet function
* @endinternal
*
* @brief   Get Policy Pointer threshold and maximal envelope size for using the
*         Flow based algorithm. For Policy Pointer values greater or equal
*         than threshold the Priority based algorithm used.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] thresholdPtr             - (poiner to) upper threshold
*                                      of Policy Pointer for Flow Based algorithm.
* @param[out] maxSizePtr               - (poiner to) maximal envelope size and alignment.
*                                      the index of start entry of each envelope
*                                      accessed by Flow Based Algorithm should be
*                                      aligned to maxSize.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on wrong HW data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChPolicerMeterTableFlowBasedIndexConfigGet
(
    IN   GT_U8                                            devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT                 stage,
    OUT  GT_U32                                           *thresholdPtr,
    OUT  CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT          *maxSizePtr
)
{
    GT_U32      mask;       /* value to write         */
    GT_U32      regAddr;    /* register address       */
    GT_STATUS   rc;         /* function return code   */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    CPSS_NULL_PTR_CHECK_MAC(thresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(maxSizePtr);

    /* read value from hardware register */
    regAddr =
        PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).meteringAddressingModeCfg0;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 17, thresholdPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr =
        PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).meteringAddressingModeCfg1;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 17, &mask);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (mask & 0x1FFFF)
    {
        case 0x1FFFE:
            *maxSizePtr = CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_2_E;
            break;
        case 0x1FFFC:
            *maxSizePtr = CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_4_E;
            break;
        case 0x1FFF8:
            *maxSizePtr = CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_8_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPolicerMeterTableFlowBasedIndexConfigGet function
* @endinternal
*
* @brief   Get Policy Pointer threshold and maximal envelope size for using the
*         Flow based algorithm. For Policy Pointer values greater or equal
*         than threshold the Priority based algorithm used.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
*
* @param[out] thresholdPtr             - (poiner to) upper threshold
*                                      of Policy Pointer for Flow Based algorithm.
* @param[out] maxSizePtr               - (poiner to) maximal envelope size and alignment.
*                                      the index of start entry of each envelope
*                                      accessed by Flow Based Algorithm should be
*                                      aligned to maxSize.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_STATE             - on wrong HW data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerMeterTableFlowBasedIndexConfigGet
(
    IN   GT_U8                                            devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT                 stage,
    OUT  GT_U32                                           *thresholdPtr,
    OUT  CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT          *maxSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMeterTableFlowBasedIndexConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, stage, thresholdPtr, maxSizePtr));

    rc = internal_cpssDxChPolicerMeterTableFlowBasedIndexConfigGet(
        devNum, stage, thresholdPtr, maxSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, stage, thresholdPtr, maxSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerTokenBucketMaxRateSet function
* @endinternal
*
* @brief   Set the value of the Maximal Rate in the Maximal Rate table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - Bucket Max Rate table index.
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] bucketIndex              - Bucket index.
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] meterEntryRate           - CIR or EIR of meter entry that use this
*                                      maximal rate in Kbps (1Kbps = 1000 bits per second)
* @param[in] meterEntryRate           - CBS or EBS of meter entry that use this maximal rate in bytes.
* @param[in] maxRate                  - maximal Information Rate in Kbps (1Kbps = 1000 bits per second)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See MEF10.3 examples in FS to know relation between bucket0/1 and Commited/Exess
*
*/
static GT_STATUS internal_cpssDxChPolicerTokenBucketMaxRateSet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      entryIndex,
    IN   GT_U32                                      bucketIndex,
    IN   GT_U32                                      meterEntryRate,
    IN   GT_U32                                      meterEntryBurstSize,
    IN   GT_U32                                      maxRate
)
{
    GT_STATUS rc;          /* return code      */
    GT_U32  hwRate;        /* HW rate          */
    GT_U32  hwBurstSize;   /* HW Burst Size    */
    GT_U32  hwRateType;    /* Hw Rate Type     */
    GT_U32  regAddr;       /* register address */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    if (entryIndex >= 128)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (bucketIndex >= 2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* maxRate and meterEntryRate swapped */
    /* to get hwRate result by maxRate    */
    rc = prvCpssDxChPolicerHwTbEnvelopeCalculate(
        devNum, stage,
        maxRate /*swRate*/,
        meterEntryBurstSize /*swBurstSize0*/,
        meterEntryRate /*swMaxRate*/,
        &hwRate /*hwRatePtr*/,
        &hwBurstSize /*hwBurstSizePtr*/,
        &hwRateType /*rateTypePtr*/);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = (bucketIndex == 0)
        ? PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).mef10_3_bucket0MaxRateArr[entryIndex]
        : PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).mef10_3_bucket1MaxRateArr[entryIndex];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 17, hwRate);

    return rc;
}

/**
* @internal cpssDxChPolicerTokenBucketMaxRateSet function
* @endinternal
*
* @brief   Set the value of the Maximal Rate in the Maximal Rate table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - Bucket Max Rate table index.
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] bucketIndex              - Bucket index.
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] meterEntryRate           - CIR or EIR of meter entry that use this
*                                      maximal rate in Kbps (1Kbps = 1000 bits per second)
* @param[in] meterEntryRate           - CBS or EBS of meter entry that use this maximal rate in bytes.
* @param[in] maxRate                  - maximal Information Rate in Kbps (1Kbps = 1000 bits per second)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
* @note See MEF10.3 examples in FS to know relation between bucket0/1 and Commited/Exess
*
*/
GT_STATUS cpssDxChPolicerTokenBucketMaxRateSet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      entryIndex,
    IN   GT_U32                                      bucketIndex,
    IN   GT_U32                                      meterEntryRate,
    IN   GT_U32                                      meterEntryBurstSize,
    IN   GT_U32                                      maxRate
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerTokenBucketMaxRateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, stage, entryIndex, bucketIndex,
         meterEntryRate, meterEntryBurstSize, maxRate));

    rc = internal_cpssDxChPolicerTokenBucketMaxRateSet(
        devNum, stage, entryIndex, bucketIndex,
        meterEntryRate, meterEntryBurstSize, maxRate);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, stage, entryIndex, bucketIndex,
        meterEntryRate, meterEntryBurstSize, maxRate));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerTokenBucketMaxRateGet function
* @endinternal
*
* @brief   Get the value of the Maximal Rate in the Maximal Rate table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - Bucket Max Rate table index.
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] bucketIndex              - Bucket index.
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] meterEntryRate           - CIR or EIR of meter entry that use this
*                                      maximal rate in Kbps (1Kbps = 1000 bits per second)
* @param[in] meterEntryRate           - CBS or EBS of meter entry that use this maximal rate in bytes.
*
* @param[out] maxRatePtr               - (pointer to) maximal Information Rate in Kbps (1Kbps = 1000 bits per second)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChPolicerTokenBucketMaxRateGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      entryIndex,
    IN   GT_U32                                      bucketIndex,
    IN   GT_U32                                      meterEntryRate,
    IN   GT_U32                                      meterEntryBurstSize,
    OUT  GT_U32                                      *maxRatePtr
)
{
    GT_STATUS rc;          /* return code      */
    GT_U32  maxHwRate;     /* max HW rate      */
    GT_U32  regAddr;       /* register address */
    GT_U32  hwRate;        /* HW rate          */
    GT_U32  hwBurstSize;   /* HW Burst Size    */
    GT_U32  hwRateType;    /* Hw Rate Type     */

    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  tbHwParams;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  tbSwParams;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    if (entryIndex >= 128)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (bucketIndex >= 2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(maxRatePtr);

    /* meterEntryRate used also as maxRate         */
    /* to get probable hwRateType                  */
    /* We have no info for more exact calculations */
    rc = prvCpssDxChPolicerHwTbEnvelopeCalculate(
        devNum, stage,
        meterEntryRate /*swRate*/,
        meterEntryBurstSize /*swBurstSize0*/,
        meterEntryRate /*swMaxRate*/,
        &hwRate /*hwRatePtr*/,
        &hwBurstSize /*hwBurstSizePtr*/,
        &hwRateType /*rateTypePtr*/);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = (bucketIndex == 0)
        ? PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).mef10_3_bucket0MaxRateArr[entryIndex]
        : PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).mef10_3_bucket1MaxRateArr[entryIndex];
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 17, &maxHwRate);
    if (rc != GT_OK)
    {
        return rc;
    }

    tbHwParams.envelope.cir = hwRate;
    tbHwParams.envelope.cbs = hwBurstSize;
    tbHwParams.envelope.eir = hwRate;
    tbHwParams.envelope.ebs = hwBurstSize;
    tbHwParams.envelope.maxCir = maxHwRate;
    tbHwParams.envelope.maxEir = maxHwRate;
    rc = prvCpssDxCh3PolicerSwTbCalculate(
        CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E,
        hwRateType /*rateType0*/, hwRateType/*rateType1*/,
        &tbHwParams, &tbSwParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    *maxRatePtr = tbSwParams.envelope.maxCir;
    return GT_OK;
}

/**
* @internal cpssDxChPolicerTokenBucketMaxRateGet function
* @endinternal
*
* @brief   Get the value of the Maximal Rate in the Maximal Rate table entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] entryIndex               - Bucket Max Rate table index.
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] bucketIndex              - Bucket index.
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] meterEntryRate           - CIR or EIR of meter entry that use this
*                                      maximal rate in Kbps (1Kbps = 1000 bits per second)
* @param[in] meterEntryRate           - CBS or EBS of meter entry that use this maximal rate in bytes.
*
* @param[out] maxRatePtr               - (pointer to) maximal Information Rate in Kbps (1Kbps = 1000 bits per second)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerTokenBucketMaxRateGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      entryIndex,
    IN   GT_U32                                      bucketIndex,
    IN   GT_U32                                      meterEntryRate,
    IN   GT_U32                                      meterEntryBurstSize,
    OUT  GT_U32                                      *maxRatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerTokenBucketMaxRateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, stage, entryIndex, bucketIndex,
         meterEntryRate, meterEntryBurstSize, maxRatePtr));

    rc = internal_cpssDxChPolicerTokenBucketMaxRateGet(
        devNum, stage, entryIndex, bucketIndex,
        meterEntryRate, meterEntryBurstSize, maxRatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, stage, entryIndex, bucketIndex,
        meterEntryRate, meterEntryBurstSize, maxRatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerQosProfileToPriorityMapSet function
* @endinternal
*
* @brief   Set entry of Qos Profile Mapping to Priority table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] qosProfileIndex          - Qos Profile index.
*                                      (APPLICABLE RANGES: 0..1023)
* @param[in] priority                 - priority.
*                                      (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChPolicerQosProfileToPriorityMapSet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      qosProfileIndex,
    IN   GT_U32                                      priority
)
{
    GT_STATUS rc;               /* return code          */
    GT_U32  entryIndex;         /* entry index          */
    GT_U32  offset;             /* offset in entry      */
    GT_U32  regAddr;            /* register address     */
    CPSS_DXCH_TABLE_ENT   tableType;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    if (qosProfileIndex >= 1024)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (priority >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        tableType = CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E + stage;

        /* updating <Qos Profile to Priority Mapping> field */
        rc = prvCpssDxChWriteTableEntryField(devNum,tableType,qosProfileIndex,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0,3,priority);
        return rc;
    }

    entryIndex = (qosProfileIndex / 8);
    offset     = ((qosProfileIndex % 8) * 3);

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).qosProfile2PriorityMapArr[entryIndex];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, offset, 3, priority);

    return rc;
}

/**
* @internal cpssDxChPolicerQosProfileToPriorityMapSet function
* @endinternal
*
* @brief   Set entry of Qos Profile Mapping to Priority table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] qosProfileIndex          - Qos Profile index.
*                                      (APPLICABLE RANGES: 0..1023)
* @param[in] priority                 - priority.
*                                      (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerQosProfileToPriorityMapSet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      qosProfileIndex,
    IN   GT_U32                                      priority
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerQosProfileToPriorityMapSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, stage, qosProfileIndex, priority));

    rc = internal_cpssDxChPolicerQosProfileToPriorityMapSet(
        devNum, stage, qosProfileIndex, priority);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, stage, qosProfileIndex, priority));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerQosProfileToPriorityMapGet function
* @endinternal
*
* @brief   Get entry of Qos Profile Mapping to Priority table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] qosProfileIndex          - Qos Profile index.
*                                      (APPLICABLE RANGES: 0..1023)
*
* @param[out] priorityPtr              - (pointer to) priority.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChPolicerQosProfileToPriorityMapGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      qosProfileIndex,
    OUT  GT_U32                                      *priorityPtr
)
{
    GT_STATUS rc;               /* return code          */
    GT_U32  entryIndex;         /* entry index          */
    GT_U32  offset;             /* offset in entry      */
    GT_U32  regAddr;            /* register address     */
    CPSS_DXCH_TABLE_ENT   tableType;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    if (qosProfileIndex >= 1024)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(priorityPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        tableType = CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E + stage;

        /* updating <Qos Profile to Priority Mapping> field */
        rc = prvCpssDxChReadTableEntryField(devNum,tableType,qosProfileIndex,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            0,3,priorityPtr);
        return rc;
    }

    entryIndex = (qosProfileIndex / 8);
    offset     = ((qosProfileIndex % 8) * 3);

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum, stage).qosProfile2PriorityMapArr[entryIndex];
    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 3, priorityPtr);

    return rc;
}

/**
* @internal cpssDxChPolicerQosProfileToPriorityMapGet function
* @endinternal
*
* @brief   Get entry of Qos Profile Mapping to Priority table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] qosProfileIndex          - Qos Profile index.
*                                      (APPLICABLE RANGES: 0..1023)
*
* @param[out] priorityPtr              - (pointer to) priority.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerQosProfileToPriorityMapGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      qosProfileIndex,
    OUT  GT_U32                                      *priorityPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerQosProfileToPriorityMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, stage, qosProfileIndex, priorityPtr));

    rc = internal_cpssDxChPolicerQosProfileToPriorityMapGet(
        devNum, stage, qosProfileIndex, priorityPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, stage, qosProfileIndex, priorityPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerPortGroupBucketsCurrentStateGet function
* @endinternal
*
* @brief   Get current state of metering entry buckets.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Data retieved from the first poert group of bitmap.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] policerIndex             - index of metering entry
*
* @param[out] bucketsStatePtr          - (pointer to) Buckets State structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChPolicerPortGroupBucketsCurrentStateGet
(
    IN   GT_U8                                       devNum,
    IN   GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      policerIndex,
    OUT  CPSS_DXCH_POLICER_BUCKETS_STATE_STC        *bucketsStatePtr
)
{
    GT_STATUS                       rc;               /* generic return status code */
    GT_U32                          portGroupId;      /* port group id */
    GT_U32                          regAddr;          /* register address */
    GT_U32                          hwData[8];        /* HW data */
    GT_U32                          val;              /* temporary value */
    CPSS_DXCH_TABLE_ENT         tableType;        /* metering table type */
    PRV_CPSS_ENTRY_FORMAT_TABLE_STC *tableFormatPtr;  /* HW format of table*/

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
         CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);
    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);
    PRV_CPSS_DXCH_POLICERS_NUM_CHECK_MAC(devNum, stage, policerIndex);
    CPSS_NULL_PTR_CHECK_MAC(bucketsStatePtr);

    /* Get the first active port group */
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);
    cpssOsMemSet(bucketsStatePtr, 0, sizeof(CPSS_DXCH_POLICER_BUCKETS_STATE_STC));

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum) != GT_FALSE)
    {
        /* Bobcat2 and above */
        if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
        {
            tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
        }
        else if (CPSS_DXCH_POLICER_STAGE_INGRESS_1_E == stage )
        {
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                /* access to Metering Token Bucket table is done by IPLR0 */
                tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
                policerIndex += PLR0_SIZE_MAC(devNum);
            }
            else
            {
                tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E;
            }
        }
        else /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
        {
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                /* access to Metering Token Bucket table is done by IPLR0 */
                tableType = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
                policerIndex += PLR0_SIZE_MAC(devNum) + PLR1_SIZE_MAC(devNum);
            }
            else
            {
                tableType = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E;
            }
        }

        rc =  prvCpssDxChPortGroupReadTableEntry(
            devNum, portGroupId, tableType, policerIndex, &(hwData[0]));
        if (rc != GT_OK)
        {
            return rc;
        }
        tableFormatPtr =
            PRV_CPSS_DXCH_PP_MAC(devNum)->tableFormatInfo[
                PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_E].fieldsInfoPtr;
        U32_GET_FIELD_BY_ID_MAC(hwData,tableFormatPtr,
            SIP5_PLR_METERING_TABLE_FIELDS_LAST_UPDATE_TIME0_E, val);
        bucketsStatePtr->lastTimeUpdate0 = val;
        U32_GET_FIELD_BY_ID_MAC(hwData,tableFormatPtr,
            SIP5_PLR_METERING_TABLE_FIELDS_LAST_UPDATE_TIME1_E, val);
        bucketsStatePtr->lastTimeUpdate1 = val;
        U32_GET_FIELD_BY_ID_MAC(hwData,tableFormatPtr,
            SIP5_PLR_METERING_TABLE_FIELDS_WRAP_AROUND_INDICATOR0_E, val);
        bucketsStatePtr->wrapAround0 = BIT2BOOL_MAC(val);
        U32_GET_FIELD_BY_ID_MAC(hwData,tableFormatPtr,
            SIP5_PLR_METERING_TABLE_FIELDS_WRAP_AROUND_INDICATOR1_E, val);
        bucketsStatePtr->wrapAround1 = BIT2BOOL_MAC(val);
        U32_GET_FIELD_BY_ID_MAC(hwData,tableFormatPtr,
            SIP5_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE0_E, val);
        bucketsStatePtr->bucketSize0 = val;
        U32_GET_FIELD_BY_ID_MAC(hwData,tableFormatPtr,
            SIP5_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE1_E, val);
        bucketsStatePtr->bucketSize1 = val;
        if PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)
        {
            if (CPSS_DXCH_POLICER_STAGE_INGRESS_0_E == stage )
            {
                tableType = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E;
            }
            else if (CPSS_DXCH_POLICER_STAGE_INGRESS_1_E == stage )
            {
                tableType = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E;
            }
            else /* CPSS_DXCH_POLICER_STAGE_EGRESS_E */
            {
                tableType = CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E;
            }

            /*Caelum, Bobcat3*/
            rc =  prvCpssDxChPortGroupReadTableEntry(
                devNum, portGroupId, tableType, policerIndex, &(hwData[0]));
            if (rc != GT_OK)
            {
                return rc;
            }
            tableFormatPtr =
                PRV_CPSS_DXCH_PP_MAC(devNum)->tableFormatInfo[
                    PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_CONFORM_SIGN_E].fieldsInfoPtr;
            U32_GET_FIELD_BY_ID_MAC(hwData,tableFormatPtr,
                PRV_CPSS_DXCH_PLR_METERING_CONFORM_SIGN_TABLE_FIELDS_BUCKET0_SIGN_E, val);
            bucketsStatePtr->bucketSignPositive0 = BIT2BOOL_MAC(val);
            U32_GET_FIELD_BY_ID_MAC(hwData,tableFormatPtr,
                PRV_CPSS_DXCH_PLR_METERING_CONFORM_SIGN_TABLE_FIELDS_BUCKET1_SIGN_E, val);
            bucketsStatePtr->bucketSignPositive1 = BIT2BOOL_MAC(val);
        }
    }
    else
    {
        /* from DxCh3 to Lion2 */
        /*********************************************************/
        /* Read Word 0-3 of requested Policer Metering Entry */
        /*********************************************************/
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
            PLR[stage].policerMeteringCountingTbl + (policerIndex * METER_AND_COUNTING_ENTRY_NUM_BYTES_CNS);

        rc = prvCpssHwPpPortGroupReadRam(
            devNum, portGroupId, regAddr, 4, &(hwData[0]));
        if (rc != GT_OK)
        {
            return rc;
        }
        /*[27:0]  lastTimeUpdate0 */
        U32_GET_FIELD_IN_ENTRY_MAC(hwData,0,28,val);
        bucketsStatePtr->lastTimeUpdate0 = val;
        /*[55:28] lastTimeUpdate1 */
        U32_GET_FIELD_IN_ENTRY_MAC(hwData,28,28,val);
        bucketsStatePtr->lastTimeUpdate1 = val;
        /*[56:56] wrapAround0 */
        U32_GET_FIELD_IN_ENTRY_MAC(hwData,56,1,val);
        bucketsStatePtr->wrapAround0 = BIT2BOOL_MAC(val);
        /*[57:57] wrapAround1 */
        U32_GET_FIELD_IN_ENTRY_MAC(hwData,57,1,val);
        bucketsStatePtr->wrapAround1 = BIT2BOOL_MAC(val);
        /*[89:58] bucketSize0 */
        U32_GET_FIELD_IN_ENTRY_MAC(hwData,58,32,val);
        bucketsStatePtr->bucketSize0 = val;
        /*[121:90] bucketSize1 */
        U32_GET_FIELD_IN_ENTRY_MAC(hwData,90,32,val);
        bucketsStatePtr->bucketSize1 = val;
    }
    return GT_OK;
}

/**
* @internal cpssDxChPolicerPortGroupBucketsCurrentStateGet function
* @endinternal
*
* @brief   Get current state of metering entry buckets.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Data retieved from the first poert group of bitmap.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] policerIndex             - index of metering entry
*
* @param[out] bucketsStatePtr          - (pointer to) Buckets State structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerPortGroupBucketsCurrentStateGet
(
    IN   GT_U8                                       devNum,
    IN   GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      policerIndex,
    OUT  CPSS_DXCH_POLICER_BUCKETS_STATE_STC        *bucketsStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerPortGroupBucketsCurrentStateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, portGroupsBmp, stage, policerIndex, bucketsStatePtr));

    rc = internal_cpssDxChPolicerPortGroupBucketsCurrentStateGet(
        devNum, portGroupsBmp, stage, policerIndex, bucketsStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, portGroupsBmp, stage,  policerIndex, bucketsStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerBucketsCurrentStateGet function
* @endinternal
*
* @brief   Get current state of metering entry buckets.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] policerIndex             - index of metering entry
*
* @param[out] bucketsStatePtr          - (pointer to) Buckets State structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChPolicerBucketsCurrentStateGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      policerIndex,
    OUT  CPSS_DXCH_POLICER_BUCKETS_STATE_STC        *bucketsStatePtr
)
{
    return cpssDxChPolicerPortGroupBucketsCurrentStateGet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        stage, policerIndex, bucketsStatePtr);
}

/**
* @internal cpssDxChPolicerBucketsCurrentStateGet function
* @endinternal
*
* @brief   Get current state of metering entry buckets.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] policerIndex             - index of metering entry
*
* @param[out] bucketsStatePtr          - (pointer to) Buckets State structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChPolicerBucketsCurrentStateGet
(
    IN   GT_U8                                       devNum,
    IN   CPSS_DXCH_POLICER_STAGE_TYPE_ENT            stage,
    IN   GT_U32                                      policerIndex,
    OUT  CPSS_DXCH_POLICER_BUCKETS_STATE_STC        *bucketsStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerBucketsCurrentStateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, devNum, stage, policerIndex, bucketsStatePtr));

    rc = internal_cpssDxChPolicerBucketsCurrentStateGet(
        devNum, stage, policerIndex, bucketsStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, stage,  policerIndex, bucketsStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


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
)
{
    GT_STATUS   rc;
    GT_U32      ram0Size;       /* RAM#0 size, used for device with
                                   3 memories like xCat2, Bobcat2, Caelum  */
    GT_U32      ram1Size;       /* RAM#1 size */
    GT_U32      ram2Size;       /* RAM#2 size */
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT  mode;
    GT_U32      hwData3;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E |
                                            CPSS_XCAT2_E | CPSS_LION_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);


    rc = ramSize0Get(devNum,&ram0Size);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = ramSize1Get(devNum,&ram1Size);
    if(rc != GT_OK)
    {
        return rc;
    }
    ram2Size = 256;

    *numEntriesRam0Ptr = ram0Size;
    *numEntriesRam1Ptr = ram1Size;
    *numEntriesRam2Ptr = ram2Size;

    rc = cpssDxChPolicerMemorySizeModeGet(devNum,&mode,NULL,NULL,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    *iplr0StartRamPtr = 0;
    *iplr1StartRamPtr = 0;
    *iplr0NumRamsUsedPtr = 0;

    switch (mode)
    {
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E:
            hwData3 = MEM_2;/*0x4*/
            break;
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E:
            hwData3 = 0x0;
            break;
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E:
            hwData3 = MEM_2 | MEM_1 | MEM_0;/*0x7*/
            break;
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_PLR0_LOWER_E:
            hwData3 = MEM_1 | MEM_0;/*0x3*/
            break;
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_4_E:
             hwData3 = MEM_2 | MEM_1;/*0x6*/
            break;
        case CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_5_E:
             hwData3 = MEM_0;/*0x1*/
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(hwData3)/*represents the usage of PLR with the RAMs */
    {
        case 0:
            /* all memories by PLR0*/
            *iplr0NumRamsUsedPtr = 3;
            break;
        case MEM_0:
            /* Mem 0 by PLR1 , rest by PLR0*/
            *iplr0NumRamsUsedPtr = 2;
            *iplr0StartRamPtr = 1;
            break;
        case MEM_0 | MEM_1:
            /* Mem 0,1 by PLR1 , rest by PLR0*/
            *iplr0NumRamsUsedPtr = 1;
            *iplr0StartRamPtr = 2;
            break;
        case MEM_0 | MEM_1 | MEM_2:
            /* Mem 0,1,2 by PLR1 , none by PLR0*/
            *iplr0NumRamsUsedPtr = 0;
            break;
        case MEM_1 | MEM_2:
            /* Mem 1,2 by PLR1 , rest by PLR0*/
            *iplr0NumRamsUsedPtr = 1;
            *iplr1StartRamPtr = 1;
            break;
        case MEM_2:
            /* Mem 2 by PLR1 , rest by PLR0*/
            *iplr0NumRamsUsedPtr = 2;
            *iplr1StartRamPtr = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *iplr1NumRamsUsedPtr = 3 -  (*iplr0NumRamsUsedPtr);

    return GT_OK;
}

/**
* @internal internal_cpssDxChPolicerMeteringAnalyzerIndexSet function
* @endinternal
*
* @brief  This function sets analyzer index per color of the packet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] color                    - possible values are: green, yellow or red.
* @param[in] enable                   - enable/disable mirroring.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index or color.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerMeteringAnalyzerIndexSet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN CPSS_DP_LEVEL_ENT                   color,
    IN GT_BOOL                             enable,
    IN GT_U32                              index
)
{
    GT_STATUS rc;
    GT_U32    hwValue;
    GT_U32    startBit;
    GT_U32    regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    if(index >  CPSS_DXCH_MIRROR_ANALYZER_MAX_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH3_COS_DP_CONVERT_MAC(color, hwValue);
    startBit = hwValue * 3;

    if (enable == GT_TRUE)
    {
        hwValue = index + 1;
    }
    else
    {
        hwValue = 0;
    }

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).globalConfig.meteringAnalyzerIndexes;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, startBit, 3, hwValue);

    return rc;  
}

/**
* @internal cpssDxChPolicerMeteringAnalyzerIndexSet function
* @endinternal
*
* @brief  This function sets analyzer index per color of the packet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] color                    - possible values are: green, yellow or red.
* @param[in] enable                   - enable/disable mirroring.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index or color.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerMeteringAnalyzerIndexSet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN CPSS_DP_LEVEL_ENT                   color,
    IN GT_BOOL                             enable,
    IN GT_U32                              index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMeteringAnalyzerIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, color, enable, index));

    rc = internal_cpssDxChPolicerMeteringAnalyzerIndexSet(devNum, stage, color, enable, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, color, enable, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPolicerMeteringAnalyzerIndexGet function
* @endinternal
*
* @brief  This function gets analyzer index per color of the packet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] color                    - possible values are: green, yellow or red.
* @param[out] enablePtr               - (pointer to) enable/disable mirroring.
* @param[out] indexPtr                - (pointer to) analyzer interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or color.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPolicerMeteringAnalyzerIndexGet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN CPSS_DP_LEVEL_ENT                   color,
    OUT GT_BOOL                            *enablePtr,
    OUT GT_U32                             *indexPtr
)
{
    GT_STATUS rc;
    GT_U32    hwValue;
    GT_U32    startBit;
    GT_U32    regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_CH1_E | CPSS_CH1_DIAMOND_E | CPSS_CH2_E | CPSS_CH3_E |
        CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_LION_E | CPSS_XCAT2_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_ALDRIN_E | CPSS_AC3X_E);    

    PRV_CPSS_DXCH_PLR_STAGE_CHECK_MAC(devNum, stage);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);

    PRV_CPSS_DXCH3_COS_DP_CONVERT_MAC(color, hwValue);
    startBit = hwValue * 3;

    regAddr = PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).globalConfig.meteringAnalyzerIndexes;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 3, &hwValue);

    if (rc != GT_OK)
        return rc;

    if (!hwValue)
    {
        *enablePtr = GT_FALSE;
    }
    else
    {
        *enablePtr = GT_TRUE;
        *indexPtr = hwValue - 1;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPolicerMeteringAnalyzerIndexGet function
* @endinternal
*
* @brief  This function gets  analyzer index per color of the packet.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] stage                    - Policer Stage type: Ingress #0, Ingress #1 or Egress.
* @param[in] color                    - possible values are: green, yellow or red.
* @param[out] enablePtr               - (pointer to) enable/disable mirroring.
* @param[out] indexPtr                - (pointer to) analyzer interface index. (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or color.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPolicerMeteringAnalyzerIndexGet
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_POLICER_STAGE_TYPE_ENT    stage,
    IN CPSS_DP_LEVEL_ENT                   color,
    OUT GT_BOOL                            *enablePtr,
    OUT GT_U32                             *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPolicerMeteringAnalyzerIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, color, enablePtr, indexPtr));

    rc = internal_cpssDxChPolicerMeteringAnalyzerIndexGet(devNum, stage, color, enablePtr, indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, color, enablePtr, indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

