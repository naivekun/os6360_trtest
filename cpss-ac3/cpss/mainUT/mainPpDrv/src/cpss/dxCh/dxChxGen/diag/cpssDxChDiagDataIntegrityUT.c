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
* @file cpssDxChDiagDataIntegrityUT.c
*
* @brief Unit tests for cpssDxChDiagDataIntegrity
* CPSS DXCH Diagnostic API
*
* @version   5
********************************************************************************
*/
/* includes */

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrityTables.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#define LION2_MAX_LOCATION_NUM_PER_MEMORY_CNS       512

#define IS_BOBK_WITHOUT_TM_MAC(dev) \
    (IS_BOBK_DEV_MAC(dev) &&        \
    !PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.featureInfo.TmSupported)

/* returns any DFX ECC protected memory relevant for specified device.*/
#define PRV_LEGAL_DFX_ECC_PROTECTED_MEMORY(devNum)                       \
    ( PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ?                              \
          CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_TB_TBL_E : \
          CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E)

/* bobcat2 memories types with external (non-DFX) protection which are not
 * supported by DataIntegrity APIs.
 * not implemented because of any reasons */

/* Lion2 memories not supporting error injection
   BMA: special memory vendor (MISL_SR512) - no error injection via DFX
   TXQ Q Counters: special memory vendor (MISL_SR512) - no error injection via DFX
   TXQ Q Index FIFO: useless(spare) memory - no access with traffic and/or MG
   TXQ: false interrupts that cause storming */
#define LION2_MEMORIES_NO_ERROR_INJECTION_CASES_MAC                              \
         CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E:        \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_0_COUNTERS_E: \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_1_COUNTERS_E: \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_2_COUNTERS_E: \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_3_COUNTERS_E: \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_4_COUNTERS_E: \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_5_COUNTERS_E: \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_6_COUNTERS_E: \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_7_COUNTERS_E: \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_0_E:     \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_1_E:     \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_2_E:     \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_3_E:     \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_4_E:     \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_5_E:     \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_6_E:     \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_7_E:     \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:           \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E:             \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E:                   \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E:                   \
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E



/* bobcat3 logical tables consisting of HW tables which have not the same
 * protection type(ECC/Parity) */
#define PRV_BOBCAT3_LOGICAL_TABLES_AMBIGUOUS_PROTECTION_CASES_MAC \
    CPSS_DXCH_LOGICAL_TABLE_PHYSICAL_PORT_E:                               \
    case CPSS_DXCH_LOGICAL_TABLE_TRUNK_MEMBERS_E:                          \
    case CPSS_DXCH_LOGICAL_TABLE_PCL_UDB_SELECT_E

#define TCAM_RULEIDX_SUBSAMPLING_CNS (1024)
#define TCAM_RULEIDX_GET(_hwaddr) (U32_GET_FIELD_MAC(_hwaddr, 13, 4) *\
 CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS +\
 U32_GET_FIELD_MAC(_hwaddr, 5, 8) * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS +\
 U32_GET_FIELD_MAC(_hwaddr, 1, 4))

#ifndef ASIC_SIMULATION
typedef GT_STATUS DXCH_DATA_INTEGRITY_EVENT_CB_FUNC(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr
);

extern DXCH_DATA_INTEGRITY_EVENT_CB_FUNC *dxChDataIntegrityEventIncrementFunc;

static GT_U32 parityErrorPortgroupBmp;
static GT_U8  parityErrorDevice;
static GT_U32 parityErrorRuleIdx;
static GT_U32 parityErrorProduced;
static GT_U32 parityErrorLastIndexFromEvent = 0xFFFFFFFF;
#endif

/*----------------------------------------------------------------------------*/

/* maximal size of rule in words - 80 Byte / 4 */
#define UTF_TCAM_MAX_RULE_SIZE_CNS 20



typedef enum
{
    PRV_UTF_DXCH_DATA_INTEGRITY_OP_ERROR_INJECTION_E,
    PRV_UTF_DXCH_DATA_INTEGRITY_OP_ERROR_COUNT_ENABLE_E,
    PRV_UTF_DXCH_DATA_INTEGRITY_OP_EVENT_MASK_E
}
PRV_UTF_DXCH_DATA_INTEGRITY_OP_ENT;

#ifdef CPSS_LOG_ENABLE
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[];
extern GT_U32                              prvCpssLogEnum_size_CPSS_DXCH_TABLE_ENT;
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT[];
extern GT_U32                              prvCpssLogEnum_size_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT;
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_LOGICAL_TABLE_ENT[];
extern GT_U32                              prvCpssLogEnum_size_CPSS_DXCH_LOGICAL_TABLE_ENT;
#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer) \
    PRV_CPSS_LOG_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)

#else

#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)
#endif

extern GT_U8 strNameBuffer[120];

/**
* @internal prvUtfCheckMemSkip function
* @endinternal
*
* @brief   Check if specified operation shouldn't be somewhy applied to
*         specified memory
* @param[in] dev                      - device number
* @param[in] memType                  - memory type
* @param[in] op                       - operation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - the mapping HW table->Logical Table is not found
* @retval GT_NOT_FOUND             - if memType is not found in DB
*/
static GT_BOOL prvUtfCheckMemSkip
(
    IN GT_U8 dev,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType,
    IN PRV_UTF_DXCH_DATA_INTEGRITY_OP_ENT op
)
{
    GT_STATUS doSkip = 0;
    switch (op)
    {
        case PRV_UTF_DXCH_DATA_INTEGRITY_OP_ERROR_COUNT_ENABLE_E:
            switch (memType)
            {
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X74_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X66_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXDATA_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXHDR_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_TXDATA_E:
                    /* skipped in bobcat3. See Jira CPSS-6627 */
                    doSkip = PRV_CPSS_SIP_5_20_CHECK_MAC(dev);
                    break;

                default:
                    break;
            }
            break;

        case PRV_UTF_DXCH_DATA_INTEGRITY_OP_EVENT_MASK_E:
            switch (memType)
            {
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E:
                    /* false interrupt in lion2 > could not be enabled */
                    doSkip = PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev);
                    break;

                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
                    /* lion2, sip5, xcat3 */
                    /* false interrupt > could not be enabled. For sip5 see erratum
                       PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E
                    */
                    doSkip = 1;
                    break;

                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
                    /* the BMA interrupt mask register is out of bobcat3
                       interrupts tree */
                    doSkip =  PRV_CPSS_SIP_5_20_CHECK_MAC(dev);
                    break;


                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X74_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X66_E:
                    /* skipped in bobcat3. See Jira CPSS-6627 */
                    doSkip = PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(dev);
                    break;

                default:
                    break;
            }
            break;

        case PRV_UTF_DXCH_DATA_INTEGRITY_OP_ERROR_INJECTION_E:
            switch (memType)
            {
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E:
                    /* lion2, sip5: because of HW behavior it is impossible to check it */
                    doSkip = 1;
                    break;

                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXDATA_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXHDR_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_TXDATA_E:
                    /* sip5. skip this memories. They are updated constantly by device.
                       So Error Injection bit can be reset before we read it */
                    doSkip = PRV_CPSS_SIP_5_CHECK_MAC(dev);
                    break;

                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L0_DATA_BANK_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L0_TAG_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L1_ATTR_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L1_DATA_BANK_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L1_DPARITY_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L1_STATE_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L1_TAG_WAY_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ICACHE_L0_DATA_BANK_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ICACHE_L0_TAG_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ICACHE_L1_DATA_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ICACHE_L1_TAG_WAY_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IF_ID_IS_BPM_BANK_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2_ECC_RAM_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2_TDV_RAM_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MMU_TLB_DATA_WAY_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MMU_TLB_TAG_WAY_E:
                    /* BobK only memories */
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_D_RAM512X96BE11_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_D_RAM64X74_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_D_RAM64X68BE33_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_D_RAM32X68BE16_E:
                    /* bobcat2: skip error injection to CPU memory to avoid crashes on MSYS
                       in aldrin, ac3x, bobcat3 this memories is absent */
                    doSkip = (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(dev) ||
                              IS_BOBK_DEV_MAC(dev));

                    break;

                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2_PLRU_RAM_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2_DATA_RAM_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L0_DPARITY_E:
                    /* bobcat2: skip error injection to CPU memory to avoid crashes on MSYS
                       bobk:    memories are updated constantly by device.
                                So Error Injection bit can be reset before we read it.
                      In aldrin, ac3x, bobcat3 this memory is absent */
                    doSkip = (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(dev) ||
                              IS_BOBK_DEV_MAC(dev));
                    break;

                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_DESKEW_MEM_E:
                    /* in bobcat3 injection doesn't work somewhy.
                       in bobcat2, aldrin, ac3x this memory is absent */
                    doSkip = PRV_CPSS_SIP_5_20_CHECK_MAC(dev);
                    break;

                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X74_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_RUNIT_NAND_RAM16X66_E:
                    /* skipped in bobcat3. See Jira CPSS-6627 */
                    doSkip = PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(dev);
                    break;

                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_ETH_TXFIFO_HEADER_LL_FREE_BUFS_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_ETH_TXFIFO_PAYLOAD_LL_FREE_BUFS_E:
                    /* bobk. Skip this memories. They are updated constantly by device.
                       So Error Injection bit can be reset before we read it.
                    */
                    doSkip = IS_BOBK_WITHOUT_TM_MAC(dev);
                    break;

                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_TBMEM_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_TBNEGMEM_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_TBNEG2MEM_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_WFSMEM_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTBMEM_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PTBMEM_E:
                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMFCU_INGRESS_TIMERS_TABLE_E:
                    /* Traffic Manager is working. Skip this memories.
                       They are updated constantly by device.
                       So Error Injection bit can be reset before we read it.
                    */
                    doSkip = prvUtfIsTrafficManagerUsed();
                    break;

                default:
                    break;
            }

            break;
        default:
            break;
    }
    return  (doSkip? GT_TRUE: GT_FALSE);
}


/**
* @internal prvUtfCheckMemForErrorStatusIssue function
* @endinternal
*
* @brief   Bits 5,6 of "DFX Memory Control register" ("Fixable/Non Fixable
*          Error Status Lock En") of some memories
*          doesn't react on writing somewhy. The function return true
*          if specified memory is one of them.
* @param[in] dev                      - device number
* @param[in] memType                  - memory type
* @param[in] op                       - operation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - the mapping HW table->Logical Table is not found
* @retval GT_NOT_FOUND             - if memType is not found in DB
*/
static GT_BOOL prvUtfCheckMemForErrorStatusIssue
(
    IN GT_U8 dev,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType
)
{
    GT_U32 isIssue = 0;
    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMIG_L1_OVERHEAD_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMEG_AGING_PROFILES_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMEG_TMPORT2DMA_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMFCU_ETH_DMA_2TM_PORT_MAPPING_TABLE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMFCU_ILK_DMA_2TM_PORT_MAPPING_TABLE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMFCU_TCPORT2CNODE_MAPPING_TABLE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMFCU_INGRESS_TIMERS_TABLE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMFCU_PORT_TIMERS_CONF_TABLE_E:
            /* he following memories are managed by TM Clock.
             * Probably an access to these memories doesn't work as expected
             * if the clock is disabled */
            isIssue = IS_BOBK_DEV_MAC(dev);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_TXDMA_RD_BURST_FIFO_E:
            isIssue = (IS_BOBK_DEV_MAC(dev) || PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev));
            break;

        default:
            isIssue = 0;
            break;
    }

    return isIssue ? GT_TRUE : GT_FALSE;
}

/**
* @internal prvUtfInitLocationsFromMemType function
* @endinternal
*
* @brief   Fill RAM/HW Table/Logical Table locations using incoming memory type
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] memEntryPtr              - (pointer to) RAM-location.
* @param[out] hwEntryPtr               - (pointer to) HW table-location
* @param[out] ltEntryPtr               - (pointer to) Logical Table-location
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - the mapping HW table->Logical Table is not found
* @retval GT_NOT_FOUND             - if memType is not found in DB
*/
static GT_STATUS prvUtfInitLocationsFromMemType
(
    IN GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType,
    OUT CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC       *memEntryPtr,
    OUT CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC       *hwEntryPtr,
    OUT CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC       *ltEntryPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_LOCATION_FULL_INFO_STC locationFull;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT protectType;
    GT_U32  arrSize = 1;

    cpssOsMemSet(&locationFull, 0, sizeof(locationFull));

    locationFull.ramEntryInfo.memType = memType;
    locationFull.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* fill DFX pipe, client, memory according to memType. */

    rc = prvCpssDxChDiagDataIntegrityMemoryIndexesGet(
        devNum, locationFull.ramEntryInfo.memType, GT_FALSE, 0 /* ignored */, &arrSize,
        &locationFull.ramEntryInfo.memLocation,
        &protectType);

    if (rc != GT_OK && rc != GT_BAD_SIZE)
    {
         /* GT_BAD_SIZE is not error. We intentionally specified arrSize==1
            to get only first RAM of specified type from DB */

        return rc; /* can be GT_NOT_FOUND */
    }

    if ( PRV_CPSS_SIP_5_CHECK_MAC(devNum) && (!PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum)))
    {
        rc = prvCpssDxChDiagDataIntegrityFillHwLogicalFromRam(devNum, &locationFull);
        if (rc != GT_OK)
        {
            return rc;
        }

        if ( (locationFull.hwEntryInfo.hwTableType != CPSS_DXCH_TABLE_LAST_E)
             &&
             locationFull.logicalEntryInfo.numOfLogicalTables == 0)
        {
            /* there are no mapping "HW table -> Logical Table" */
            PRV_UTF_LOG2_MAC("device %d, HW table %d is not mapped to logical table\n",
                             devNum,
                             locationFull.hwEntryInfo.hwTableType);
            return GT_FAIL;
        }
    }
    else
    {
        /* Fill HW/Logical tables locations with emtry values.
           This cause calling code use only RAM location for iteration */
        locationFull.hwEntryInfo.hwTableType             = CPSS_DXCH_TABLE_LAST_E;
        locationFull.logicalEntryInfo.numOfLogicalTables = 0;
    }

    cpssOsMemSet(memEntryPtr, 0, sizeof(*memEntryPtr));
    cpssOsMemSet(hwEntryPtr, 0, sizeof(*hwEntryPtr));
    cpssOsMemSet(ltEntryPtr, 0, sizeof(*ltEntryPtr));

    memEntryPtr->type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
    memEntryPtr->info.ramEntryInfo    = locationFull.ramEntryInfo;

    hwEntryPtr->type  = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
    hwEntryPtr->info.hwEntryInfo      = locationFull.hwEntryInfo;

    ltEntryPtr->type  = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    ltEntryPtr->info.logicalEntryInfo = locationFull.logicalEntryInfo;

    return GT_OK;
}


/**
* @internal prvUtfDiagDataIntegrityMemProtectionKinds function
* @endinternal
*
* @brief   Scan device Data Integrity DB for first item of specified memory type
*         and return protection types of the item.
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] dfxProtectionPtr         - DFX protection type
* @param[out] extProtectionPtr         - external (non-DFX) protection type
*
* @retval GT_OK                    - if memory is found in DB
* @retval GT_NOT_FOUND             - if memory is not found in DB
*/
static GT_STATUS  prvUtfDiagDataIntegrityMemProtectionKinds
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT *dfxProtectionPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT *extProtectionPtr
)
{
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC  *memDbArr;
    GT_U32                                        memDbSize;
    GT_U32                                        i;


    prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &memDbArr, &memDbSize);

    CPSS_NULL_PTR_CHECK_MAC(memDbArr);

    for (i=0; i<memDbSize; i++)
    {
        if (memDbArr[i].memType == (GT_U32)memType)
        {
            if (dfxProtectionPtr)
                *dfxProtectionPtr = memDbArr[i].protectionType;
            if (extProtectionPtr)
                *extProtectionPtr = memDbArr[i].externalProtectionType;
            return GT_OK;
        }
    }
    return GT_NOT_FOUND;
}

/**
* @internal prvUtfDiagDataIntegrityIsMemValid function
* @endinternal
*
* @brief   Check if the memory type is relevent for device.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*/
static GT_BOOL prvUtfDiagDataIntegrityIsMemValid
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType
)
{
    return (GT_OK == prvUtfDiagDataIntegrityMemProtectionKinds(devNum, memType, NULL, NULL));
}

/**
* @internal prvUtfDiagDataIntegrityMemIsDfxProtected function
* @endinternal
*
* @brief   Check if the memory type is DFX protected
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @retval GT_TRUE                  - memory is DFX protected
* @retval GT_FALSE                 - memory either not protected by DFX or is not relevant to device
*/
static GT_BOOL prvUtfDiagDataIntegrityMemIsDfxProtected
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType
)
{
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT dfxProtection;
    GT_STATUS rc;
    rc = prvUtfDiagDataIntegrityMemProtectionKinds(devNum, memType, &dfxProtection, NULL);

    if (rc == GT_OK && dfxProtection != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }

}


/**
* @internal prvUtfDiagDataIntegrityIsEventMaskedAsExpected function
* @endinternal
*
* @brief   Check if cpssDxChDiagDataIntegrityEventMaskGet returned values are
*         appropriate to the previously called cpssDxChDiagDataIntegrityEventMaskSet
*
* @retval GT_TRUE                  - if read values corresponds to written values
* @retval GT_FALSE                 - otherwise
*/
static GT_BOOL prvUtfDiagDataIntegrityIsEventMaskedAsExpected
(
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT         typeSet,
    IN CPSS_EVENT_MASK_SET_ENT                               opSet,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT         typeGet,
    IN CPSS_EVENT_MASK_SET_ENT                               opGet
)
{

    GT_U32 isOk;

    if (typeGet == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E ||
        typeGet == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E)
    {
        isOk = (opSet == opGet);
    }
    else
    {
        /* ECC protected, not SINGLE_AND_MULTIPLE */

        /* a result "op:unmasked: type:<ECC multiple>" is satisfactory
           for a Set-request "op:mask, type:<ECC single>" */
        isOk =
            ((opSet == CPSS_EVENT_MASK_E) && (opSet != opGet) && (typeSet != typeGet))
            ||
            ((opSet == CPSS_EVENT_UNMASK_E) && (opSet == opGet) && (typeSet == typeGet));
    }
    return BIT2BOOL_MAC(isOk);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityEventMaskSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityEventMaskSet)
{
/*
    ITERATE_DEVICES(Lion2, bobcat2, aldrin, xcat3x, bobcat3)
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
        errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E - parameter
                     relevant only for ECC protected memories.
        operation = CPSS_EVENT_UNMASK_E
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT       errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT       errorTypeGet;
    CPSS_EVENT_MASK_SET_ENT                             operation;
    CPSS_EVENT_MASK_SET_ENT                             operationGet;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    GT_BOOL                                             isLion2;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT          devLastMem;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT          memType;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                memEntry; /* RAM location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                hwEntry;  /* hw table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                ltEntry;  /* logical table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*               locationsArr[4];
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*               locationPtr;
    GT_U32                                              locationIter;
    GT_U32                                              opIter;
    GT_BOOL                                             isOkExpected;
    GT_BOOL                                             doIgnoreErrorType; /* if the API ignores errorType */

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E
        | UTF_FALCON_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        isLion2    = PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev) ? GT_TRUE : GT_FALSE;
        devLastMem = (isLion2 == GT_TRUE) ?
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E :
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E;

        for (memType = 0; memType < devLastMem; memType++)
        {
            if (GT_TRUE == prvUtfCheckMemSkip(
                    dev, memType, PRV_UTF_DXCH_DATA_INTEGRITY_OP_EVENT_MASK_E))
            {
                continue;
            }

            /* LION2: cpssDxChDiagDataIntegrityEventMaskSet ignores errorType for
               some ECC external (not-DFX) protected memories.
               I.e. it will not be failed if errorType == parity */
            doIgnoreErrorType = GT_FALSE;
            if (PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev)) {
                switch (memType) {
                    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_DESCRIPTOR_FIFO_E:
                    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG2_DESC_RETURN_TWO_FIFO_E:
                    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG4_DESC_RETURN_TWO_FIFO_E:
                    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E:
                    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E:
                        doIgnoreErrorType = GT_TRUE;
                        break;
                    default:
                        break;
                }
            }
            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];

                if ((locationPtr->type == CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE &&
                     locationPtr->info.hwEntryInfo.hwTableType == CPSS_DXCH_TABLE_LAST_E)
                    ||
                    (locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE &&
                     locationPtr->info.logicalEntryInfo.numOfLogicalTables == 0))
                {
                    /* there are no table */
                    continue;
                }

                st = cpssDxChDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                if (st == GT_NOT_FOUND) /* this memType is not applicable for device */
                {
                    continue;
                }
                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                    locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE)
                {
                    switch (locationPtr->info.logicalEntryInfo.logicaTableInfo[0].logicalTableType)
                    {
                        case PRV_BOBCAT3_LOGICAL_TABLES_AMBIGUOUS_PROTECTION_CASES_MAC:
                            /* in bobcat 3 this logical tables contains HW tables
                               with different (ECC/Parity) protection */
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_VALUE, st, dev,
                                                        memType, locationIter);
                            continue;

                            break;
                        default:
                            break;
                    }
                }
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, locationIter);

                if (protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
                {
                    doIgnoreErrorType = GT_TRUE;
                }

                for (errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                     errorType <= CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                     errorType++)
                {
                    if (doIgnoreErrorType == GT_TRUE)
                    {
                        /* run cpssDxChDiagDataIntegrityEventMaskSet once in cases when it
                           ignores errorType (it do this for parity protected memories and
                           for some ECC non-DFX protected memories in Lion2).
                           It is convenient to do this when errorType
                           appropriates to  protectionType: it makes errorTypeGet compatible
                           with errorType without any additional checks. */

                        if (((protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E) &&
                             (errorType != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E))
                            ||
                            ((protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E) &&
                             (errorType != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E)))
                        {
                            continue;
                        }
                    }

                    isOkExpected = doIgnoreErrorType;
                    if (protectionType ==
                             CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E
                             && errorType != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E)
                    {
                        isOkExpected = GT_TRUE;
                    }
                    for (opIter = 0; opIter < 3; opIter++)
                    {
                        /* 1) To ensure mask/unmask really works previous event
                               state should be unmasked/masked.
                           2) Last operation should be "mask" to avoid  unmasking
                              of events (masked by default) after UT finished
                           So operations will be {mask, unmask, mask} */
                        operation = (opIter % 2) ?  CPSS_EVENT_UNMASK_E : CPSS_EVENT_MASK_E;
                        st = cpssDxChDiagDataIntegrityEventMaskSet(dev, locationPtr,
                                                                   errorType, operation);
                        if (GT_FALSE == isOkExpected)
                        {
                            UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                            errorType, operation);
                        }
                        else
                        {
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                        errorType, operation);
                        }

                        if(st == GT_OK)
                        {
                            st = cpssDxChDiagDataIntegrityEventMaskGet(dev, locationPtr,
                                                                       &errorTypeGet, &operationGet);
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                        errorTypeGet, operationGet);
                            if(st == GT_OK)
                            {
                                UTF_VERIFY_EQUAL7_STRING_MAC(
                                    GT_TRUE,
                                    prvUtfDiagDataIntegrityIsEventMaskedAsExpected(
                                        errorType, operation,
                                        errorTypeGet, operationGet),
                                    "bad masking: dev %d, mem %d, iter %d,"
                                    " errorType %d(%d), operation %d(%d)",
                                    dev, memType, locationIter,
                                    errorType, errorTypeGet, operation, operationGet);
                            }
                        }
                    }
                }
            }
        }

        memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
        operation = CPSS_EVENT_MASK_E;
        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssDxChDiagDataIntegrityEventMaskSet
                                          (dev, &memEntry, errorType, operation),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);

        /* restore valid ECC protected memType. */
        memType = PRV_LEGAL_DFX_ECC_PROTECTED_MEMORY(dev);
        memEntry.info.ramEntryInfo.memType = memType;

        /*
              1.2. Call with wrong errorType (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityEventMaskSet
                            (dev, &memEntry, errorType, operation),
                            errorType);

        /*
              1.2. Call with wrong operation (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityEventMaskSet
                            (dev, &memEntry, errorType, operation),
                            operation);
    }

    memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E
        | UTF_FALCON_E);


    /* restore valid values of memType, errorType, operation */
    memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E;
    memEntry.info.ramEntryInfo.memType = memType;
    errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
    operation = CPSS_EVENT_UNMASK_E;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityEventMaskSet(dev, &memEntry, errorType, operation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDataIntegrityEventMaskSet(dev, &memEntry, errorType, operation);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityEventMaskGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityEventMaskGet)
{
/*
    ITERATE_DEVICES(Lion2, bobcat2, aldrin, xcat3x, bobcat3)
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
        errorType = CPSS_DXCH_DIAG_DATA_INTEGRITY_SINGLE_AND_MULTIPLE_ECC_ERROR_TYPE_E - parameter
                     relevant only for ECC protected memories.
        operation = CPSS_EVENT_UNMASK_E
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.
    1.3 Call with NULL ptr
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT       errorType;
    CPSS_EVENT_MASK_SET_ENT                             operation;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    GT_BOOL                                             isLion2;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT          devLastMem;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT          memType;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                memEntry; /* RAM location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                hwEntry;  /* hw table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                ltEntry;  /* logical table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*               locationsArr[4];
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*               locationPtr;
    GT_U32                                              locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        isLion2    = PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev) ? GT_TRUE : GT_FALSE;
        devLastMem = (isLion2 == GT_TRUE) ?
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E :
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E;

        for (memType = 0; memType < devLastMem; memType++)
        {
            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];

                if ((locationPtr->type == CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE &&
                     locationPtr->info.hwEntryInfo.hwTableType == CPSS_DXCH_TABLE_LAST_E)
                    ||
                    (locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE &&
                     locationPtr->info.logicalEntryInfo.numOfLogicalTables == 0))
                {
                    /* there are no table */
                    continue;
                }

                st = cpssDxChDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                if (st == GT_NOT_FOUND) /* this memType is not applicable for device */
                {
                    continue;
                }

                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                    locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE)
                {
                    switch (locationPtr->info.logicalEntryInfo.logicaTableInfo[0].logicalTableType)
                    {
                        case PRV_BOBCAT3_LOGICAL_TABLES_AMBIGUOUS_PROTECTION_CASES_MAC:
                            /* in bobcat 3 this logical tables contains HW tables
                               with different (ECC/Parity) protection */
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_VALUE, st, dev,
                                                        memType, locationIter);
                            continue;

                            break;
                        default:
                            break;
                    }
                }
                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter, errorType, operation);

                st = cpssDxChDiagDataIntegrityEventMaskGet(dev, locationPtr, &errorType, &operation);
                if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    errorType, operation);
                }
                else
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorType, operation);
                }
            }
        }

        memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssDxChDiagDataIntegrityEventMaskGet
                                          (dev, &memEntry, &errorType, &operation),
                                          memEntry.info.ramEntryInfo.memType,
                                          GT_NOT_FOUND);

        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */
        memType = PRV_LEGAL_DFX_ECC_PROTECTED_MEMORY(dev);
        memEntry.info.ramEntryInfo.memType = memType;

        st = cpssDxChDiagDataIntegrityEventMaskGet(dev, &memEntry, NULL, &operation);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssDxChDiagDataIntegrityEventMaskGet(dev, &memEntry, &errorType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

    }

    memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

    /* restore valid memType */
    memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E;
    memEntry.info.ramEntryInfo.memType = memType;


    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityEventMaskGet(dev, &memEntry, &errorType, &operation);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDataIntegrityEventMaskGet(dev, &memEntry, &errorType, &operation);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityErrorInjectionConfigSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode,
    IN  GT_BOOL                                         injectEnable
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityErrorInjectionConfigSet)
{
/*
    ITERATE_DEVICES(Lion2, bobcat2, Aldrin, AC3X)
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectModeGet;
    GT_BOOL                                         injectEnable = GT_TRUE;
    GT_BOOL                                         injectEnableGet;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    GT_BOOL                                         isLion2;
    GT_BOOL                                         isInjectionSupported;
    GT_U32                                          i;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      devLastMem;
    GT_BOOL                                         tbScanDisable = GT_FALSE;
    GT_U32                                          regAddr;
    GT_U32                                          savedRegData;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            memEntry; /* RAM location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwEntry;  /* hw table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            ltEntry;  /* logical table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*           locationsArr[4];
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *locationPtr;
    GT_U32                                          locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        isLion2    = PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev) ? GT_TRUE : GT_FALSE;
        devLastMem = (isLion2 == GT_TRUE) ?
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E :
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E;

        for (memType = 0; memType < devLastMem; memType++)
        {
            if (prvUtfCheckMemSkip(dev, memType,
                                   PRV_UTF_DXCH_DATA_INTEGRITY_OP_ERROR_INJECTION_E ))
            {
                continue;
            }

            PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(
                memType, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer);

            if (isLion2 == GT_TRUE)
            {
                /* determine if injection supported */
                switch (memType)
                {
                    case LION2_MEMORIES_NO_ERROR_INJECTION_CASES_MAC:
                        isInjectionSupported = GT_FALSE;
                        break;
                    default:
                        isInjectionSupported = GT_TRUE;
                        break;
                }
            }
            else
            {
                isInjectionSupported = GT_TRUE;

                if (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E
                    ||
                    memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E)
                {
                    /* this memories are updated constantly by device. Disable shaper
                       tocken bucket update to guarantee that Error Injection bit
                       will not be reset by a memory update before we read it */
                    tbScanDisable = isInjectionSupported;
                }

            }

            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* save register value and disable shaper token buket update */
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* the device not supports those SIP5 TXQ features */
            }
            else
            if (tbScanDisable)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(dev).shaper.tokenBucketUpdateRate;
                st = prvCpssHwPpReadRegister(dev, regAddr, &savedRegData);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                if (st != GT_OK)
                {
                    /* to avoid restoring register's data later.  */
                    tbScanDisable = GT_FALSE;
                }

                /* set TBScanEn to zero */
                st = prvCpssHwPpSetRegField(dev, regAddr, 29, 1, 0);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];

                if ((locationPtr->type == CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE &&
                     locationPtr->info.hwEntryInfo.hwTableType == CPSS_DXCH_TABLE_LAST_E)
                    ||
                    (locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE &&
                     locationPtr->info.logicalEntryInfo.numOfLogicalTables == 0))
                {
                    /* there are no table */
                    continue;
                }
                st = cpssDxChDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                if (st == GT_NOT_FOUND) /* this memType is not applicable for device */
                {
                    continue;
                }

                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                    locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE)
                {
                    switch (locationPtr->info.logicalEntryInfo.logicaTableInfo[0].logicalTableType)
                    {
                        case PRV_BOBCAT3_LOGICAL_TABLES_AMBIGUOUS_PROTECTION_CASES_MAC:
                            /* in bobcat 3 this logical tables contains HW tables
                               with different (ECC/Parity) protection */
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_VALUE, st, dev,
                                                        memType, locationIter);
                            continue;

                            break;
                        default:
                            break;
                    }
                }
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);


                /* 1.1 a) Call with
                          injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E
                          injectEnable = GT_TRUE
                       b) Call with
                          injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E
                          injectEnable = GT_TRUE
                       c) Call with
                          injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E
                          injectEnable = GT_FALSE

                        Expect (for all cases): GT_OK for all memTypes that support injection,
                                                otherwise GT_NOT_SUPPORTED */
                for (i=0; i<3; i++)
                {
                    switch (i)
                    {
                        case 0: /* 1.1.a */
                            injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E;
                            injectEnable = GT_TRUE;
                            break;
                        case 1: /* 1.1.b */
                            injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
                            injectEnable = GT_TRUE;
                            break;
                        default: /* 1.1.c */
                            injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
                            injectEnable = GT_FALSE;
                            break;
                    }

                    st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
                        dev, locationPtr, injectMode, injectEnable);
                    if (GT_FALSE == isInjectionSupported)
                    {
                        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, memType, locationIter,
                                                    injectMode, injectEnable);
                    }
                    else if (isLion2 == GT_FALSE)
                    {
                        /* SIP5 supports all RAMs */
                        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    injectMode, injectEnable);
                    }
                    else if (protectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                    {
                        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    injectMode, injectEnable);
                    }
                    else
                    {
                        UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                        injectMode, injectEnable);
                    }

                    if(st == GT_OK)
                    {
                        st = cpssDxChDiagDataIntegrityErrorInjectionConfigGet(
                            dev, locationPtr, &injectModeGet, &injectEnableGet);
                        UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    injectModeGet, injectEnableGet);
                        if(st == GT_OK)
                        {
                            if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
                            {
                                UTF_VERIFY_EQUAL6_STRING_MAC(injectMode, injectModeGet,
                                                             "get another injectModeGet than was set: %d %d %s %d %d %d",
                                                             dev, memType, strNameBuffer, locationIter, injectModeGet, injectEnableGet);
                            }
                            UTF_VERIFY_EQUAL6_STRING_MAC(injectEnable, injectEnableGet,
                                                         "get another injectEnableGet than was set: %d %d %s %d %d %d",
                                                         dev, memType, strNameBuffer, locationIter, injectModeGet, injectEnableGet);
                        }
                    }
                }
            }
            /* restore modified register value */
            if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
            {
                /* the device not supports those SIP5 TXQ features */
            }
            else
            if (tbScanDisable)
            {
                regAddr = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(dev).shaper.tokenBucketUpdateRate;
                st = prvCpssHwPpWriteRegister(dev, regAddr, savedRegData);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                tbScanDisable = GT_FALSE;
            }
        }

        memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssDxChDiagDataIntegrityErrorInjectionConfigSet
                                          (dev, &memEntry, injectMode, injectEnable),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);

        /*
              1.2. Call with wrong injectMode (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityErrorInjectionConfigSet
                            (dev, &memEntry, injectMode, injectEnable),
                            injectMode);
    }

    /* restore valid values */
    memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E;
    memEntry.info.ramEntryInfo.memType = memType;
    injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
    injectEnable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(dev, &memEntry, injectMode, injectEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(dev, &memEntry, injectMode, injectEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityErrorInjectionConfigGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  *injectModePtr,
    OUT GT_BOOL                                         *injectEnablePtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityErrorInjectionConfigGet)
{
/*
    ITERATE_DEVICES(Lion2, bobcat2, Aldrin, AC3X)
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode;
    GT_BOOL                                         injectEnable;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    GT_BOOL                                         isLion2;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      devLastMem;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            memEntry; /* RAM location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwEntry;  /* hw table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            ltEntry;  /* logical table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*           locationsArr[4];
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *locationPtr;
    GT_U32                                          locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        isLion2    = PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev) ? GT_TRUE : GT_FALSE;
        devLastMem = (isLion2 == GT_TRUE) ?
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E :
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E;

        for (memType = 0; memType < devLastMem; memType++)
        {
            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];

                if ((locationPtr->type == CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE &&
                     locationPtr->info.hwEntryInfo.hwTableType == CPSS_DXCH_TABLE_LAST_E)
                    ||
                    (locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE &&
                     locationPtr->info.logicalEntryInfo.numOfLogicalTables == 0))
                {
                    /* there are no table */
                    continue;
                }

                st = cpssDxChDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                if (st == GT_NOT_FOUND) /* this memType is not applicable for device */
                {
                    continue;
                }

                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                    locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE)
                {
                    switch (locationPtr->info.logicalEntryInfo.logicaTableInfo[0].logicalTableType)
                    {
                        case PRV_BOBCAT3_LOGICAL_TABLES_AMBIGUOUS_PROTECTION_CASES_MAC:
                            /* in bobcat 3 this logical tables contains HW tables
                               with different (ECC/Parity) protection */
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_VALUE, st, dev,
                                                        memType, locationIter);
                            continue;

                            break;
                        default:
                            break;
                    }
                }
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, locationIter);


                injectMode = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E;
                injectEnable = GT_TRUE;

                st = cpssDxChDiagDataIntegrityErrorInjectionConfigGet(
                    dev, locationPtr, &injectMode, &injectEnable);
                if (GT_TRUE == isLion2)
                {
                    switch (memType)
                    {
                        case LION2_MEMORIES_NO_ERROR_INJECTION_CASES_MAC:
                            UTF_VERIFY_EQUAL5_PARAM_MAC(GT_NOT_SUPPORTED, st, dev,
                                                        memType, locationIter,
                                                        injectMode, injectEnable);
                            break;
                        default:
                            if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                            {
                                UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev,
                                                                memType, locationIter,
                                                                injectMode, injectEnable);
                            }
                            else
                            {
                                UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev,
                                                            memType, locationIter,
                                                            injectMode, injectEnable);
                            }
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev,
                                                memType, locationIter,
                                                injectMode, injectEnable);
                }
            }
        }

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssDxChDiagDataIntegrityErrorInjectionConfigGet
                                          (dev, &memEntry, &injectMode, &injectEnable),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);

        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */
        memType = PRV_LEGAL_DFX_ECC_PROTECTED_MEMORY(dev);
        memEntry.info.ramEntryInfo.memType = memType;

        st = cpssDxChDiagDataIntegrityErrorInjectionConfigGet(dev, &memEntry, NULL, &injectEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssDxChDiagDataIntegrityErrorInjectionConfigGet(dev, &memEntry, &injectMode, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

    }

    /* restore valid values */
    memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E;
    memEntry.info.ramEntryInfo.memType = memType;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityErrorInjectionConfigGet(dev, &memEntry, &injectMode, &injectEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDataIntegrityErrorInjectionConfigGet(dev, &memEntry, &injectMode, &injectEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityErrorCountEnableSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  GT_BOOL                                         countEnable
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityErrorCountEnableSet)
{
/*
    ITERATE_DEVICES(Lion2, bobcat2, aldrin, xcat3x, bobcat3)
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorTypeGet;
    GT_BOOL                                         countEnable = GT_TRUE;
    GT_BOOL                                         countEnableGet;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    GT_BOOL                                         isLion2;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      devLastMem;
    GT_BOOL                                         memFound;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType, memTypeLast;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            memEntry;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwEntry;  /* hw table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            ltEntry;  /* logical table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*           locationsArr[4];
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *locationPtr;
    GT_U32                                          locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        isLion2    = PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev) ? GT_TRUE : GT_FALSE;
        devLastMem = (isLion2 == GT_TRUE) ?
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E :
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E;

        memFound = GT_FALSE;
        memTypeLast = devLastMem;
        for (memType = 0; memType < devLastMem; memType++)
        {
            /* skip non-dfx protected memories */
            if (GT_FALSE == prvUtfDiagDataIntegrityMemIsDfxProtected(dev, memType))
            {
                continue;
            }


            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);
            if (GT_TRUE == prvUtfCheckMemSkip(
                    dev, memType, PRV_UTF_DXCH_DATA_INTEGRITY_OP_ERROR_COUNT_ENABLE_E))
            {
                continue;
            }

            PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(
                memType, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer);

            memFound = GT_TRUE;
            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];

                if ((locationPtr->type == CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE &&
                     locationPtr->info.hwEntryInfo.hwTableType == CPSS_DXCH_TABLE_LAST_E)
                    ||
                    (locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE &&
                     locationPtr->info.logicalEntryInfo.numOfLogicalTables == 0))
                {
                    /* there are no table */
                    continue;
                }

                st = cpssDxChDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                if (st == GT_NOT_FOUND) /* this memType is not applicable for device */
                {
                    continue;
                }

                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                    locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE)
                {
                    switch (locationPtr->info.logicalEntryInfo.logicaTableInfo[0].logicalTableType)
                    {
                        case PRV_BOBCAT3_LOGICAL_TABLES_AMBIGUOUS_PROTECTION_CASES_MAC:
                            /* in bobcat 3 this logical tables contains HW tables
                               with different (ECC/Parity) protection */
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_VALUE, st, dev,
                                                        memType, locationIter);
                            continue;

                            break;
                        default:
                            break;
                    }
                }
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, locationIter);


                memTypeLast = memType;
                /*
                    1.1 Call with
                        errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E
                        countEnable = GT_TRUE

                        Expect: GT_OK for all memTypes that support enable counters, otherwise GT_BAD_PARAM/GT_NOT_SUPPORTED
                */

                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                countEnable = GT_TRUE;

                st = cpssDxChDiagDataIntegrityErrorCountEnableSet(dev, locationPtr, errorType, countEnable);
                if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorType, countEnable);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    errorType, countEnable);
                }

                if(st == GT_OK)
                {
                    st = cpssDxChDiagDataIntegrityErrorCountEnableGet(dev, locationPtr, &errorTypeGet, &countEnableGet);
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorTypeGet, countEnableGet);
                    if(st == GT_OK)
                    {
                        UTF_VERIFY_EQUAL6_STRING_MAC(errorType, errorTypeGet,
                                                     "get another errorType than was set: %d %d %s %d %d %d",
                                                     dev, memType, strNameBuffer, locationIter, errorTypeGet, countEnableGet);
                        UTF_VERIFY_EQUAL6_STRING_MAC(countEnable, countEnableGet,
                                                     "get another countEnable than was set: %d %d %s %d %d %d",
                                                     dev, memType, strNameBuffer, locationIter, errorTypeGet, countEnableGet);
                    }
                }

                /*
                    1.1 Call with
                        errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E

                        Expect: GT_OK for all memTypes that support enable counters, otherwise GT_BAD_PARAM/GT_NOT_SUPPORTED
                */
                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;

                st = cpssDxChDiagDataIntegrityErrorCountEnableSet(dev, locationPtr, errorType, countEnable);

                if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorType, countEnable);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    errorType, countEnable);
                }

                if(st == GT_OK)
                {
                    st = cpssDxChDiagDataIntegrityErrorCountEnableGet(dev, locationPtr, &errorTypeGet, &countEnableGet);
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorTypeGet, countEnableGet);
                    if(st == GT_OK )
                    {
                        /*set errorTypeGet equal to errorType to avoid test fail. */
                        if (GT_FALSE != prvUtfCheckMemForErrorStatusIssue(dev, memType))
                        {
                            errorTypeGet = errorType;
                        }

                        UTF_VERIFY_EQUAL6_STRING_MAC(
                            errorType, errorTypeGet,
                            "get another errorType than was set: %d %d %s %d %d %d",
                            dev, memType, strNameBuffer, locationIter, errorTypeGet, countEnableGet);
                        UTF_VERIFY_EQUAL6_STRING_MAC(
                            countEnable, countEnableGet,
                            "get another countEnable than was set: %d %d %s %d %d %d",
                            dev, memType, strNameBuffer, locationIter, errorTypeGet, countEnableGet);
                    }
                }


                /*
                    1.1 Call with
                        errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E

                        Expect: GT_OK for all memTypes that support enable counters, otherwise GT_BAD_PARAM/GT_NOT_SUPPORTED
                */
                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;

                st = cpssDxChDiagDataIntegrityErrorCountEnableSet(dev, locationPtr, errorType, countEnable);

                if(protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorType, countEnable);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    errorType, countEnable);
                }

                if(st == GT_OK)
                {
                    st = cpssDxChDiagDataIntegrityErrorCountEnableGet(dev, locationPtr, &errorTypeGet, &countEnableGet);
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorTypeGet, countEnableGet);
                    if(st == GT_OK)
                    {
                        /*set errorTypeGet equal to errorType to avoid test fail. */
                        if (GT_FALSE != prvUtfCheckMemForErrorStatusIssue(dev, memType))
                        {
                            errorTypeGet = errorType;
                        }

                        UTF_VERIFY_EQUAL6_STRING_MAC(
                            errorType, errorTypeGet,
                            "get another errorType than was set: %d %d %s %d %d %d",
                            dev, memType, strNameBuffer, locationIter, errorTypeGet, countEnableGet);
                        UTF_VERIFY_EQUAL6_STRING_MAC(
                            countEnable, countEnableGet,
                            "get another countEnable than was set: %d %d %s %d %d",
                            dev, memType, strNameBuffer, locationIter, errorTypeGet, countEnableGet);
                    }
                }
            }
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, memFound,
                                     "no one valid memory is found in device %d ",
                                     dev);

        /* use last valid */
        memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
        memEntry.info.ramEntryInfo.memType = memTypeLast;

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssDxChDiagDataIntegrityErrorCountEnableSet
                                          (dev, &memEntry, errorType, countEnable),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);

        /*
              1.2. Call with wrong errorType (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityErrorCountEnableSet
                            (dev, &memEntry, errorType, countEnable),
                            errorType);
    }
    /* restore valid values */
    memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E;
    memEntry.info.ramEntryInfo.memType = memType;
    errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E;
    countEnable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityErrorCountEnableSet(dev, &memEntry, errorType, countEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDataIntegrityErrorCountEnableSet(dev, &memEntry, errorType, countEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityErrorCountEnableGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT GT_BOOL                                         *countEnablePtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityErrorCountEnableGet)
{
/*
    ITERATE_DEVICES(Lion2, bobcat2, aldrin, xcat3x, bobcat3)
    1.1.Call with
        All memory types and with HW/Logical tables relevant to memType
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType;
    GT_BOOL                                         countEnable;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    GT_BOOL                                         isLion2;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      devLastMem;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            memEntry;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwEntry;  /* hw table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            ltEntry;  /* logical table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*           locationsArr[4];
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*           locationPtr;
    GT_U32                                          locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        isLion2    = PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev) ? GT_TRUE : GT_FALSE;
        devLastMem = (isLion2 == GT_TRUE) ?
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E :
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E;

        for (memType = 0; memType < devLastMem; memType++)
        {
            if ((memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E) ||
                (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E))
            {
                continue;
            }
            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];
                if ((locationPtr->type == CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE &&
                     locationPtr->info.hwEntryInfo.hwTableType == CPSS_DXCH_TABLE_LAST_E)
                    ||
                    (locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE &&
                     locationPtr->info.logicalEntryInfo.numOfLogicalTables == 0))
                {
                    /* there are no table */
                    continue;
                }


                st = cpssDxChDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);
                if (st == GT_NOT_FOUND) /* this memType is not applicable for device */
                {
                    continue;
                }

                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                    locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE)
                {
                    switch (locationPtr->info.logicalEntryInfo.logicaTableInfo[0].logicalTableType)
                    {
                        case PRV_BOBCAT3_LOGICAL_TABLES_AMBIGUOUS_PROTECTION_CASES_MAC:
                            /* in bobcat 3 this logical tables contains HW tables
                               with different (ECC/Parity) protection */
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_VALUE, st, dev,
                                                        memType, locationIter);
                            continue;

                            break;
                        default:
                            break;
                    }
                }
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, locationIter);

                /*
                    1.1 Call with

                        Expect: GT_OK for all memTypes that support enable counters, otherwise GT_BAD_PARAM/GT_NOT_SUPPORTED
                */

                st = cpssDxChDiagDataIntegrityErrorCountEnableGet(dev, locationPtr, &errorType, &countEnable);
                if(protectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    UTF_VERIFY_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                errorType, countEnable);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL5_PARAM_MAC(GT_OK, st, dev, memType, locationIter,
                                                    errorType, countEnable);
                }
            }
        }

        memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */

        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssDxChDiagDataIntegrityErrorCountEnableGet
                                          (dev, &memEntry, &errorType, &countEnable),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);


        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */

        memType = PRV_LEGAL_DFX_ECC_PROTECTED_MEMORY(dev);
        memEntry.info.ramEntryInfo.memType = memType;
        st = cpssDxChDiagDataIntegrityErrorCountEnableGet(dev, &memEntry, NULL, &countEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssDxChDiagDataIntegrityErrorCountEnableGet(dev, &memEntry, &errorType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

    }

    /* restore valid values */
    memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E;
    memEntry.info.ramEntryInfo.memType = memType;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityErrorCountEnableGet(dev, &memEntry, &errorType, &countEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDataIntegrityErrorCountEnableGet(dev, &memEntry, &errorType, &countEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC        *memEntryPtr,
    IN  GT_BOOL                                     enable
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet)
{
/*
    ITERATE_DEVICES(Lion2, Bobcat2, Caelum, Bobcat3, Aldrin, AC3X)
    1.1.Call with
        All memory types
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_BOOL                                         enable = GT_TRUE;
    GT_BOOL                                         enableGet;
    GT_BOOL                                         isLion2;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      devLastMem;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT          memType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        isLion2    = PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev) ? GT_TRUE : GT_FALSE;
        devLastMem = (isLion2 == GT_TRUE) ?
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E :
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E;

        for (memType = 0; memType < devLastMem; memType++)
        {
            /*
                1.1 Call with
                    enable = GT_TRUE

                    Expect: GT_OK for all TCAM memTypes, otherwise GT_BAD_PARAM
            */
            enable = GT_TRUE;

            st = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(dev, memType, enable);
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                /* for SIP5 devices cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet
                 * ignores memType and just enables TCAM daemon */
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, enable);
            }
            else if(PRV_CPSS_DXCH_PP_MAC(dev)->diagInfo.tcamParityCalcEnable == GT_FALSE)
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_STATE, st, dev, memType, enable);
            }
            else if((memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E) ||
                    (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, memType, enable);
            }

            if(st == GT_OK)
            {
                st = cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet(dev, memType, &enableGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, enableGet);
                if(st == GT_OK)
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(enable, enableGet, "get another enableGet than was set: %d %d %d ", dev, memType, enableGet);
                }
            }
        }

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        if((PRV_CPSS_DXCH_PP_MAC(dev)->diagInfo.tcamParityCalcEnable == GT_TRUE) &&
           (! PRV_CPSS_SIP_5_CHECK_MAC(dev)) )
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet
                                (dev, memType, enable),
                                memType);
        }
    }

    /* restore valid values */
    memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E;

    enable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(dev, memType, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(dev, memType, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC        *memEntryPtr,
    OUT GT_BOOL                                     *enablePtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet)
{
/*
    ITERATE_DEVICES(Lion2, Bobcat2, Caelum, Bobcat3, Aldrin, AC3X)
    1.1.Call with
        All memory types
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    GT_BOOL                                         enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for (memType = 0; memType < CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E; memType++)
        {
            /*
                1.1 Call with
                    enable = GT_TRUE

                    Expect: GT_OK for all TCAM memTypes, otherwise GT_BAD_PARAM
            */

            st = cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet(dev, memType, &enable);
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, enable);
            }
            else if ((memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E) ||
                     (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E))
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, enable);
            }
            else
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "get unexpected return value %d, %d", dev, memType);
            }
        }

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        if(! PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet
                                (dev, memType, &enable),
                                memType);
        }

        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */
        memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E;

        st = cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet(dev, memType, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);
    }

    /* restore valid values */
    memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet(dev, memType, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet(dev, memType, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityProtectionTypeGet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                    *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityProtectionTypeGet)
{
/*
    ITERATE_DEVICES(Lion2, bobcat2, Aldrin, AC3X)
    1.1.Call with
        All memory types
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT          memType;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      devLastMem;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                memEntry; /* RAM location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                hwEntry;  /* hw table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                ltEntry;  /* logical table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*               locationsArr[4];
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*               locationPtr;
    GT_U32                                              locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        devLastMem = PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev) ?
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E :
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E;

        for (memType = 0; memType < devLastMem; memType++)
        {
            /* check if memType exists in device DataIntegrity DB */
            if (GT_FALSE == prvUtfDiagDataIntegrityIsMemValid(dev, memType))
            {
                continue;
            }

            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {
                locationPtr = locationsArr[locationIter];

                if ((locationPtr->type == CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE &&
                     locationPtr->info.hwEntryInfo.hwTableType == CPSS_DXCH_TABLE_LAST_E)
                    ||
                    (locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE &&
                     locationPtr->info.logicalEntryInfo.numOfLogicalTables == 0))
                {
                    /* there are no table */
                    continue;
                }

                st = cpssDxChDiagDataIntegrityProtectionTypeGet(dev, locationPtr, &protectionType);

                if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev) &&
                    locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE)
                {
                    switch (locationPtr->info.logicalEntryInfo.logicaTableInfo[0].logicalTableType)
                    {
                        case PRV_BOBCAT3_LOGICAL_TABLES_AMBIGUOUS_PROTECTION_CASES_MAC:
                            /* in bobcat 3 this logical tables contains HW tables
                               with different (ECC/Parity) protection */
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_VALUE, st, dev,
                                                        memType, locationIter);
                            continue;

                            break;
                        default:
                            break;
                    }
                }

                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, memType, protectionType);
            }
        }

        memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_NOT_FOUND.
        */
        UTF_ENUMS_CHECK_WITH_RET_CODE_MAC(cpssDxChDiagDataIntegrityProtectionTypeGet
                                          (dev, &memEntry, &protectionType),
                                          memEntry.info.ramEntryInfo.memType,
                                         GT_NOT_FOUND);

        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */
        memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E;
        memEntry.info.ramEntryInfo.memType = memType;

        st = cpssDxChDiagDataIntegrityProtectionTypeGet(dev, &memEntry, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);


    }

    /* restore valid values */
    memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E;
    memEntry.info.ramEntryInfo.memType = memType;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
        UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E |
        UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityProtectionTypeGet(dev, &memEntry, &protectionType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDataIntegrityProtectionTypeGet(dev, &memEntry, &protectionType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDataIntegrityErrorInfoGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    *locationPtr,
    OUT GT_U32                                          *errorCounterPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSyndromePtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityErrorInfoGet)
{
/*
    ITERATE_DEVICES(Lion2, bobcat2, aldrin, xcat3x, bobcat3)
    1.1.Call with
        All memory types
    Expected: GT_OK.
    1.2. Call with wrong memType (wrong enum).
    Expected: GT_BAD_PARAM.

*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    GT_U32                                          errorCounter;
    GT_U32                                          failedRow;
    GT_U32                                          failedSegment;
    GT_U32                                          failedSyndrome;
    GT_BOOL                                         isLion2;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      devLastMem;

    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC mppmLocation = {1, 1, 0};
    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC *mppmLocationPtr = NULL;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            memEntry;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwEntry;  /* hw table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            ltEntry;  /* logical table location */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*           locationsArr[4];
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*           locationPtr;
    GT_U32                                          locationIter;

    locationsArr[0] = &memEntry;
    locationsArr[1] = &hwEntry;
    locationsArr[2] = &ltEntry;
    locationsArr[3] = (CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC*)NULL; /* last entry should be NULL */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E);

    GM_NOT_SUPPORT_THIS_TEST_MAC

    cpssOsMemSet(&memEntry, 0, sizeof(memEntry));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        isLion2    = PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(dev) ? GT_TRUE : GT_FALSE;
        devLastMem = (isLion2 == GT_TRUE) ?
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E :
            CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LAST_E;

        for (memType = 0; memType < devLastMem; memType++)
        {
            if ( (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E ||
                  memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E)
                 && PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
            {
                /* registers addresses are not initialized. See Jira 6623 */
                continue;
            }

            st = prvUtfInitLocationsFromMemType(dev, memType, &memEntry, &hwEntry, &ltEntry);
            if (GT_NOT_FOUND == st)
            {
                /* RAM is not found in DataIntegrity DB */
                continue;
            }
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* AUTODOC: call the API for all location types: RAM, HW table, Logical table */
            for (locationIter=0; locationsArr[locationIter] != NULL; locationIter++)
            {

                locationPtr = locationsArr[locationIter];

                if ((locationPtr->type == CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE &&
                     locationPtr->info.hwEntryInfo.hwTableType == CPSS_DXCH_TABLE_LAST_E)
                    ||
                    (locationPtr->type == CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE &&
                     locationPtr->info.logicalEntryInfo.numOfLogicalTables == 0))
                {
                    /* there are no table */
                    continue;
                }

                mppmLocationPtr = NULL;
                if((memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E ||
                    memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E)
                   && (isLion2 == GT_TRUE))
                {
                    mppmLocationPtr = &mppmLocation;
                }


                if( isLion2 == GT_TRUE &&
                    memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E)
                {

                    memEntry.info.ramEntryInfo.memLocation.dfxMemoryId = 51;
                }

                st = cpssDxChDiagDataIntegrityErrorInfoGet(dev, locationPtr, mppmLocationPtr,
                                                           &errorCounter, &failedRow,
                                                           &failedSegment, &failedSyndrome);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);
            }
        }

        memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

        /*
              1.2. Call with wrong memType (wrong enum).
              Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDataIntegrityErrorInfoGet
                            (dev, &memEntry, NULL,  &errorCounter, &failedRow,
                             &failedSegment, &failedSyndrome),
                            memEntry.info.ramEntryInfo.memType);

        /*
              1.3. Call with NULL ptr.
              Expected: GT_BAD_PTR.
        */
        memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E;
        memEntry.info.ramEntryInfo.memType = memType;

        st = cpssDxChDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, NULL,
                                                   &failedRow, &failedSegment, &failedSyndrome);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssDxChDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, &errorCounter,
                                                   NULL, &failedSegment, &failedSyndrome);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssDxChDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, &errorCounter,
                                                   &failedRow, NULL, &failedSyndrome);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);

        st = cpssDxChDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, &errorCounter,
                                                   &failedRow, &failedSegment, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);
    }

    /* restore valid values */
    memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;

    memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E;
    memEntry.info.ramEntryInfo.memType = memType;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, &errorCounter,
                                                   &failedRow, &failedSegment, &failedSyndrome);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDataIntegrityErrorInfoGet(dev, &memEntry, NULL, &errorCounter,
                                               &failedRow, &failedSegment, &failedSyndrome);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

#ifndef ASIC_SIMULATION
#define WAIT_TIME (200) /* ms */
/**
* @internal tcamParityErrorHandler function
* @endinternal
*
* @brief   Callback called in case of data integrity error. Asserts static variable
*         parityErrorProduced in case if parity error for selected device and
*         selected TCAM rule index produced.
* @param[in] dev                      - the device.
* @param[in] eventPtr                 - filled structure which describes data integrity error event
*/
static GT_STATUS tcamParityErrorHandler
(
    IN GT_U8                                     dev,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr

)
{
    GT_BOOL checkIndex = GT_TRUE;
    if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        /* BC2 sometimes returns wrong index number */
        checkIndex = GT_FALSE;
    }

#if 1 /* Print debug info */
    PRV_UTF_LOG0_MAC("======DATA INTEGRITY ERROR EVENT======\n");
    PRV_UTF_LOG2_MAC("Device:      %d - %d\n", dev, parityErrorDevice);
    PRV_UTF_LOG2_MAC("PortGroupBmp: %d - %d\n",
        eventPtr->location.portGroupsBmp, parityErrorPortgroupBmp);
    PRV_UTF_LOG2_MAC("RuleIdx:     0x%X - 0x%X\n",
        eventPtr->location.tcamMemLocation.ruleIndex,
        parityErrorRuleIdx);
    PRV_UTF_LOG1_MAC("Event type:  %d\n", eventPtr->eventsType);
#endif
    if((dev == parityErrorDevice) &&
       (eventPtr->location.portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS ||
        eventPtr->location.portGroupsBmp == parityErrorPortgroupBmp) &&
       (eventPtr->location.isTcamInfoValid == GT_TRUE) &&
       /* check index for some devices */
       ((checkIndex == GT_FALSE) || (eventPtr->location.tcamMemLocation.ruleIndex == parityErrorRuleIdx))
      )
    {
        /* AUTODOC: Set error event produced flag */
        parityErrorProduced = 1;
    }

    if (eventPtr->location.isTcamInfoValid == GT_TRUE)
    {
        parityErrorLastIndexFromEvent = eventPtr->location.tcamMemLocation.ruleIndex;
    }

    return GT_OK;
}

static GT_U32 prvTgfDiagTcamSingleReadDebug = 0;
GT_U32 prvTgfDiagTcamSingleReadDebugSet(GT_U32 newVal)
{
    GT_U32 oldVal = prvTgfDiagTcamSingleReadDebug;
    prvTgfDiagTcamSingleReadDebug = newVal;
    return oldVal;
}

/**
* @internal diagDataIntegrityInjectErrorToRule function
* @endinternal
*
* @brief   Injects error to a single rule with specific index
*
* @param[in] dev                      - the device.
* @param[in] portGroupsBmp            - portgroup bitmap to which rule will be applied
* @param[in] ruleIdx                  - index of the rule
* @param[in] useSwDaemon              - use SW daemon or HW one
* @param[in] tcamSize                 - TCAM size
*/
static GT_VOID diagDataIntegrityInjectErrorToRule
(
    IN GT_U8    dev,
    IN GT_U32   portGroupsBmp,
    IN GT_U32   ruleIdx,
    IN GT_BOOL  useSwDaemon,
    IN GT_U32   tcamSize
)
{
    GT_U32          patternArr[3]  = {0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF};
    GT_U32          maskArr[3]     = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    GT_STATUS       rc;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC memoryLocation;

    memoryLocation.portGroupsBmp = portGroupsBmp;
    memoryLocation.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
    memoryLocation.info.ramEntryInfo.memType =
        CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E;

    parityErrorRuleIdx = ruleIdx;
    parityErrorProduced = 0;
    parityErrorLastIndexFromEvent = 0xFFFFFFFF;

    /* AUTODOC: Enable error injection for TCAM */
    rc = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
        dev,
        &memoryLocation,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
        GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "cpssDxChDiagDataIntegrityErrorInjectionConfigSet: "
        "dev=[%d],  injectMode=[%d], injectEn=[%d]",
        dev, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_TRUE);

    /* AUTDOC: Set own callback */
    dxChDataIntegrityEventIncrementFunc = tcamParityErrorHandler;

    /* AUTODOC: Write a rule */
    rc = cpssDxChTcamPortGroupRuleWrite(dev, portGroupsBmp, ruleIdx, GT_TRUE,
        CPSS_DXCH_TCAM_RULE_SIZE_10_B_E, patternArr, maskArr);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc,
        "cpssDxChTcamPortGroupRuleWrite: "
        "dev=[%d], portGroup=[0x%X], index=[%d], valid=[%d], size=[%d]",
        dev, portGroupsBmp, ruleIdx, GT_TRUE, CPSS_DXCH_TCAM_RULE_SIZE_10_B_E);

    /* TCAM parity daemon works only on the first floor in some devices.
       Use SW daemon instead.*/
    if(useSwDaemon)
    {
        CPSS_DXCH_TCAM_RULE_SIZE_ENT            ruleSize;
        GT_U32                                  tcamEntryPatternArr[3];
        GT_U32                                  tcamEntryMaskArr[3];
        GT_BOOL                                 validPtr;
        CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC    location;
        GT_U32                                  nextEntry;
        GT_BOOL                                 wrapAround;

        if (prvTgfDiagTcamSingleReadDebug)
        {
            /* AUTODOC: Produce data integrity error event */
            rc = prvCpssDxChTcamPortGroupRuleReadEntry(
                dev,
                portGroupsBmp,
                ruleIdx,
                &validPtr,
                &ruleSize,
                tcamEntryPatternArr,
                tcamEntryMaskArr
            );
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                "prvCpssDxChTcamPortGroupRuleReadEntry: "
                "dev=[%d], portGroupBmp=[%d], ruleIdx=[0x%X]",
                dev, portGroupsBmp, ruleIdx
            );
        }
        else
        {
            /* Use TCAM Logical table for SW scan */
            location.type = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
            location.portGroupsBmp = portGroupsBmp;
            location.info.logicalEntryInfo.numOfLogicalTables = 1;
            location.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType =
                CPSS_DXCH_LOGICAL_TABLE_TCAM_E;
            location.info.logicalEntryInfo.logicaTableInfo[0].
                logicalTableEntryIndex = 0;
            location.info.logicalEntryInfo.logicaTableInfo[0].numEntries = tcamSize;

            rc = cpssDxChDiagDataIntegrityTableScan(
                dev,
                &location,
                tcamSize,
                &nextEntry,
                &wrapAround
            );
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssDxChDiagDataIntegrityTableScan");
        }
    }

    /* AUTODOC: Wait for failures */
    cpssOsTimerWkAfter(WAIT_TIME);

    /* AUTDOC: Disable callback */
    dxChDataIntegrityEventIncrementFunc = NULL;

    /* AUTODOC: Invalidate rule */
    rc = cpssDxChTcamPortGroupRuleValidStatusSet(dev, portGroupsBmp, ruleIdx,
        GT_FALSE);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
        "cpssDxChTcamPortGroupRuleWrite: "
        "dev=[%d], portGroup=[0x%X], index=[%d], valid=[%d]",
         dev, portGroupsBmp, ruleIdx, GT_FALSE);

    /* AUTODOC: Disable injection. Just in case if last write failed */
    cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
        dev,
        &memoryLocation,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
        GT_FALSE
    );
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "cpssDxChDiagDataIntegrityErrorInjectionConfigSet: "
        "dev=[%d], injectMode=[%d], injectEn=[%d]",
        dev,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E,
        GT_FALSE
    );

    /* AUTODOC: Check if parity error produced. If not, the test fails */
    UTF_VERIFY_EQUAL2_STRING_MAC(parityErrorProduced, 1,
        "Parity error is not produced for rule 0x%X, index from event 0x%X\n", ruleIdx, parityErrorLastIndexFromEvent);

    /* AUTODOC: Wait some time after. To process unprocessed events if exist */
    cpssOsTimerWkAfter(WAIT_TIME);
}

/*
 * AUTODOC:
 * Parity daemon events test:
 * 1. Enable parity daemon
 * 2. Enable error injection
 * 3. Write TCAM entry
 * 4. Get interrupt of parity daemon
 * 5. Check if the interrupt valid
 * 6. Restore
 */
UTF_TEST_CASE_MAC(cpssDxChDiagDataIntegrityTcamErrorInjectionTest)
{
    GT_STATUS rc;
    GT_U8     dev;
    GT_U32    tcamSize         = 0;
    DXCH_DATA_INTEGRITY_EVENT_CB_FUNC *savedDataIntegrityHandler;

    /* It runs only on the Bobcat2 now */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E |
                                           UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_LION_E | UTF_LION2_E |
                                           UTF_XCAT2_E);

    /* AUTODOC: Save data integrity error callback. */
    savedDataIntegrityHandler = dxChDataIntegrityEventIncrementFunc;

    /* AUTODOC: Clear callback. Callback will be set right before rule write */
    dxChDataIntegrityEventIncrementFunc = NULL;

    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GT_U32                                         portGroupId = 0;
        GT_PORT_GROUPS_BMP                             portGroupsBmp = 1;
        GT_U32                                         ii;
        GT_BOOL                                        useSwDaemon = GT_FALSE;

        /* Configure */
        parityErrorDevice = dev;

        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
        {
            /* BC2 has erratum FE-5035373. TCAM daemon is not functional.
               It checks only first 3K entries. Use SW daemon for such devices. */
            useSwDaemon = GT_TRUE;
        }

        /* Get TCAM entry count */
        rc = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_PCL_TCAM_E,
        &tcamSize);
        UTF_VERIFY_EQUAL2_STRING_MAC(rc, GT_OK, "cpssDxChCfgTableNumEntriesGet:"
            " dev=[%d], table=[%d]", dev, CPSS_DXCH_CFG_TABLE_PCL_TCAM_E);

        if (useSwDaemon)
        {
            /* AUTODOC: Disable parity daemon. */
            rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(
                dev,
                CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E,
                GT_FALSE);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                "cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet: "
                "dev=[%d], memtype=[%d], enable=[%d]",
                dev, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E,
                GT_FALSE);

            /* provide time to stop daemon */
            cpssOsTimerWkAfter(500);
        }
        else
        {
            /* AUTODOC: Enable parity daemon. */
            rc = cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(
                dev,
                CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E,
                GT_TRUE);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                "cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet: "
                "dev=[%d], memtype=[%d], enable=[%d]",
                dev, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E,
                GT_TRUE);
        }

        /* Iterate through portgroups */
        PRV_TGF_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* Create bitmap for current port group */
            portGroupsBmp =
                utfTcamPortGroupsBmpForCurrentPortGroupId(dev, portGroupId);
            if(portGroupsBmp == 0)
            {
                continue;
            }

            parityErrorPortgroupBmp = portGroupsBmp;

            /* AUTODOC: Write rules */
            for(ii = 0; ii < tcamSize; ii++)
            {
               /* Subsample TCAM entries */
                if(((ii % TCAM_RULEIDX_SUBSAMPLING_CNS) != 0) &&
                    (ii != (tcamSize - 1)))
                {
                    continue;
                }
                diagDataIntegrityInjectErrorToRule(dev, portGroupsBmp, ii, useSwDaemon, tcamSize);
            }
        }
        PRV_TGF_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

    }
    /* AUTODOC: Restore dxChDataIntegrityEventIncrementFunc */
    dxChDataIntegrityEventIncrementFunc = savedDataIntegrityHandler;
}
#endif /* ASIC_SIMULATION */

/*-----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChDiagDataIntegrity suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChDiagDataIntegrity)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityEventMaskSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityEventMaskGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityErrorInjectionConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityErrorInjectionConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityErrorCountEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityErrorCountEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityProtectionTypeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityErrorInfoGet)
#ifndef ASIC_SIMULATION
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDataIntegrityTcamErrorInjectionTest)
#endif
UTF_SUIT_END_TESTS_MAC(cpssDxChDiagDataIntegrity)
