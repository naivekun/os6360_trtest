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
* @file prvCpssDxChDiagDataIntegrityMainMappingDb.h
*
* @brief Internal header with DFX Data Integrity module main mapping batabase.
*
* @version   6
********************************************************************************
*/
#ifndef __prvCpssDxChDiagDataIntegrityMainMappingDbh
#define __prvCpssDxChDiagDataIntegrityMainMappingDbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrityTables.h>
#include <cpss/common/diag/private/prvCpssCommonDiag.h>

#define LAST_VALID_TABLE_CNS   0xFFFFFFFF

/* Lion2 max port group number */
#define LION2_MAX_PORT_GROUP_NUM_CNS                    8
/* Lion2 max number of DFX pipes */
#define LION2_MAX_NUMBER_OF_PIPES_CNS                   8
/* Maximal DFX cause index  */
#define MAX_DFX_INT_CAUSE_NUM_CNS                       27
/* indicator of unused DB entry */
#define DATA_INTEGRITY_ENTRY_NOT_USED_CNS               0xCAFECAFE
/* Lion2 Hooper max port group number */
#define HOOPER_MAX_PORT_GROUP_NUM_CNS                   4

/* tables that supported only by device with TM */
extern CPSS_DXCH_TABLE_ENT prvCpssDxChHwTrafficManagerTablesArr[];

/**
* @enum PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT
*
* @brief This enum defines Data Integrity special tables treatment
*/
typedef enum{

    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE__FIRST__E = CPSS_DXCH_TABLE_LAST_E,

    /** @brief relate to 3 RAMs of IPLR0/1 shared memory.
     *  need to use table type : CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E or
     *  CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E
     */
    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_IPLR_METERING_E,

    /** @brief relate to 3 RAMs of IPLR0/1 shared memory.
     *  need to use table type : CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E or
     *  CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E
     *  PRV_DXCH_REG1_UNIT_PLR_MAC(devNum,stage).policerCountingTbl
     */
    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_IPLR_COUNTING_E,

    /** @brief relate to ermrk qos map.
     *  need to use table type : CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_DSCP_MAP_E or
     *  CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_TC_DP_MAP_E
     */
    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ERMRK_QOS_MAP_E,

    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP_E,

    /** @brief the LPM build of 20 memories. this one is index 0. (first)
     *  ... LPM memories 1..18
     */
    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E,

    /** in sip5 the LPM build of 20 memories. this one is index 19. (last) */
    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_19_E =

    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 19,

    /** in sip5.20 there are 40 'tables'. This one is last. */
    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_39_E =

    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E + 39,

    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE__LAST__E

} PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT;

/*
 *       PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_GOP_PR_MIB_COUNTERS_E -
 *              relate to GOP PR MIB counters
 *              cpss code not use 'tables engine' but 'base addr' :
 *              PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.PR.PRMIB.MIBCountersBase
*/
/* the memory for 'GOP_PR_MIB_COUNTERS' currently not implemented */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_GOP_PR_MIB_COUNTERS_E CPSS_DXCH_TABLE_LAST_E



extern PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC lion2DataIntegrityDbArray[];
extern GT_U32 lion2DataIntegrityDbArrayEntryNum;

extern PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC hooperDataIntegrityDbArray[];
extern GT_U32 hooperDataIntegrityDbArrayEntryNum;

extern PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC bobcat2DataIntegrityDbArray[];
extern GT_U32 bobcat2DataIntegrityDbArrayEntryNum;

extern PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC bobcat2B0DataIntegrityDbArray[];
extern GT_U32 bobcat2B0DataIntegrityDbArrayEntryNum;

extern PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC caelumDataIntegrityDbArray[];
extern GT_U32 caelumDataIntegrityDbArrayEntryNum;

extern PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC aldrinDataIntegrityDbArray[];
extern GT_U32 aldrinDataIntegrityDbArrayEntryNum;

extern PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC bobcat3DataIntegrityDbArray[];
extern GT_U32 bobcat3DataIntegrityDbArrayEntryNum;

extern PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC aldrin2DataIntegrityDbArray[];
extern GT_U32 aldrin2DataIntegrityDbArrayEntryNum;


extern PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC lion2DataIntegrityFirstStageMappingArray[LION2_MAX_PORT_GROUP_NUM_CNS][MAX_DFX_INT_CAUSE_NUM_CNS];
extern PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC hooperDataIntegrityFirstStageMappingArray[HOOPER_MAX_PORT_GROUP_NUM_CNS][MAX_DFX_INT_CAUSE_NUM_CNS];

extern PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC bobcat2DfxIntToPipeClientMapArr[32];
extern PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC bobcat2Dfx1IntToPipeClientMapArr[32];

extern PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC bobkDfxIntToPipeClientMapArr[32];
extern PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC bobkDfx1IntToPipeClientMapArr[32];

extern PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC aldrinDfxIntToPipeClientMapArr[32];
extern PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC aldrinDfx1IntToPipeClientMapArr[32];

/* from sip 5.15 (bobk) the IPLR 0,1 share the same "Metering Configuration",
 * "Metering Token Bucket" tables (but each has it's own entiries range)
 * I.e.
 * CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E
 * CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E
 * are used and
 * CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E
 * CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E
 * are not used (table size is 0).
 * from sip 5.20 (bobcat3) "Metering Configuration" and "Metering Token Bucken"
 * tables are share between all 3 units IPLR0, IPLR1, EPLR */
#define PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL /* empty */


/* from sip 5.20 (bc3)  the IPLR 0,1 and EPLR are sharing counting with 'base address' between them */
/* legacy devices shared IPLR 0,1 counting on 2 or 3 memories 'partition' */
#define PLR_COUNTING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL /* empty */


/**
* @internal prvCpssDxChDiagDataIntegrityMemoryIndexesGet function
* @endinternal
*
* @brief   Function gets all memory location coordinates for given memory type.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - memory type
* @param[in] isPerPortGroup           - filtering flag
*                                      GT_TRUE - only per given portgroup id
*                                      GT_FALSE - all
* @param[in] portGroupId              - portgroup id
*                                      relevant only if isPerPortGroup = GT_TRUE
* @param[in,out] arraySizePtr        - in : size of input array
*                                      out: actual size of array
*
* @param[out] memLocationArr           - array of memory locations
* @param[out] protectionTypePtr        - (pointer to) protection memory type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_SIZE              - if size of input array is not enough to store
*                                       coordinates of all found items
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS prvCpssDxChDiagDataIntegrityMemoryIndexesGet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT              memType,
    IN  GT_BOOL                                                 isPerPortGroup,
    IN  GT_U32                                                  portGroupId,
    INOUT GT_U32                                                *arraySizePtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC            *memLocationArr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
);

/**
* @internal prvCpssDxChDiagDataIntegrityDbPointerSet function
* @endinternal
*
* @brief   Function initializes current pointer to DB and size.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion.
*
* @param[in] devNum                   - device number
*
* @param[out] dbArrayPtrPtr            - (pointer to pointer to) current DB array
*                                      NULL if DB doesn't exists
* @param[out] dbArrayEntryNumPtr       - (pointer to) current DB array size
*                                      0 if DB doesn't exists
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_VOID prvCpssDxChDiagDataIntegrityDbPointerSet
(
    IN  GT_U8                                           devNum,
    OUT PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    **dbArrayPtrPtr,
    OUT GT_U32                                          *dbArrayEntryNumPtr
);

/**
* @internal prvCpssDxChDiagDataIntegrityDfxErrorConvert function
* @endinternal
*
* @brief   Function converts from 'Specific RAM + failedRow(index)' to 'HW_table + index'
*         HW index means a value passed as IN-parameter "entryIndex" in API
*         prvCpssDxChReadTableEntry/prvCpssDxChWriteTableEntry.
*         I.e. it can differ from actual hardware geometry. See using of
*         VERTICAL_INDICATION_CNS, FRACTION_INDICATION_CNS, FRACTION_HALF_TABLE_INDICATION_CNS
*         for details
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Caelum.
*
* @param[in] devNum                   - PP device number
* @param[in] memLocationPtr           - (pointer to) DFX memory info
* @param[in] failedRow                - the failed row in the DFX memory.
*
* @param[out] hwErrorInfoPtr           - (pointer to) the converted 'HW_table + index'
* @param[out] portGroupIdPtr           - (pointer to) port group id
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - the DFX location was not found in DB.
* @retval GT_EMPTY                 - the DFX location is not mapped to CPSS HW table
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagDataIntegrityDfxErrorConvert
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocationPtr,
    IN  GT_U32                                          failedRow,
    OUT GT_U32                                          *portGroupIdPtr,
    OUT CPSS_DXCH_HW_INDEX_INFO_STC                     *hwErrorInfoPtr
);

/**
* @internal prvCpssDxChDiagDataIntegrityHwTableCheck function
* @endinternal
*
* @brief   Function checks table avalability for test based on DFX injection error.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Caelum.
*
* @param[in] devNum                   - PP device number
* @param[in] tableType                - table type
*
* @param[out] memTypePtr               - (pointer to) memory type
* @param[out] numOfEntriesPtr          - (pointer to) number of table entries
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on not supported tableType
* @retval GT_NOT_FOUND             - on not found tableType in RAM DB
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on other error
*/
GT_STATUS prvCpssDxChDiagDataIntegrityHwTableCheck
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      *memTypePtr,
    OUT GT_U32                                          *numOfEntriesPtr
);

/**
* @internal prvCpssDxChDiagDataIntegrityTableHwToLogicalGet function
* @endinternal
*
* @brief   Function returns logical tables for specific hw table.
*         Function can be called multiple times (with different startPtr)
*         to get all of logical table. The function returns GT_NOT_FOUND if given
*         hw tables was not found in logical tables DB or there are no logical
*         tables any more.
* @param[in] devNum                   - device number
* @param[in] hwTable                  - the hw table name
* @param[in,out] startPtr             - (pointer to) a marker where from to start search.
*                                      0 on NULL means "search from beginning".
*                                      An out-value is used as input for next call
*                                      to continue the search.
* @param[out] logicalTablePtr          - (pointer to) the logical table name
*
* @retval GT_OK                    - logical table is found
* @retval GT_NOT_FOUND             - logical table is not found
*
* @note an example how to get all logical tables:
*       i = 0;
*       while (prvCpssDxChDiagDataIntegrityTableHwToLogicalGet(hwTable, &i, &ltable) == GT_OK)
*       (
*       printf("logical table : %s", ltable);
*       )
*
*/
GT_STATUS prvCpssDxChDiagDataIntegrityTableHwToLogicalGet
(
    IN    GT_U8                          devNum,
    IN    CPSS_DXCH_TABLE_ENT            hwTable,
    INOUT GT_U32                         *startPtr,
    OUT   CPSS_DXCH_LOGICAL_TABLE_ENT    *logicalTablePtr
);

/**
* @internal prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet function
* @endinternal
*
* @brief   Function returns the list of HW tables that relate to the logical table
*
* @param[in] devNum                   - PP device number
* @param[in] logicalTable             - the logical table
*
* @param[out] hwTableslistPtr          - (pointer to) the list of the HW tables
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if table not supported
*/
GT_STATUS prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_LOGICAL_TABLE_ENT         logicalTable,
    OUT const CPSS_DXCH_TABLE_ENT           **hwTableslistPtr
);

/**
* @internal prvCpssDxChDiagDataIntegrityLogicalTablePortGroupsBmpCheck function
* @endinternal
*
* @brief   the function checks if the portGroupsBmp is supported for the logical table
*         utility function called from the mainUT .. for tests.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] logicalTable             - the 'logical table'
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagDataIntegrityLogicalTablePortGroupsBmpCheck
(
    IN GT_U8 devNum ,
    IN GT_U32 portGroupsBmp,
    IN CPSS_DXCH_LOGICAL_TABLE_ENT logicalTable
);

/**
* @internal prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable function
* @endinternal
*
* @brief   Function checks if the device supports the HW table entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] hwTableType              - type of HW table
* @param[in] entryIndex               - index of entry
*
* @retval GT_TRUE                  - the device   supports the table and entry index is valid
* @retval GT_FALSE                 - the device NOT supports the table or entry index is not valid
*/
GT_BOOL  prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(
    IN GT_U8                devNum ,
    IN CPSS_DXCH_TABLE_ENT  hwTableType,
    IN GT_U32               entryIndex
);

/**
* @internal prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTableShadow function
* @endinternal
*
* @brief   Function returns the 'shadow type' that the CPSS may hold for the HW table
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - PP device number
*                                      hwTable     - the HW table
*                                       the 'shadow type'
*/
CPSS_DXCH_SHADOW_TYPE_ENT  prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(
    IN GT_U8                devNum ,
    IN CPSS_DXCH_TABLE_ENT  hwTableType
);


/**
* @internal prvCpssDxChDiagDataIntegrityTableHwMaxIndexGet function
* @endinternal
*
* @brief   Function returns the number of entries,lines that HW table hold
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - PP device number
* @param[in] hwTable                  - the IPLR table
*
* @param[out] maxNumEntriesPtr         - (pointer to) the number of entries supported by the table
* @param[out] maxNumLinesPtr           - (pointer to) the number of lines   supported by the table
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if table not covered by this function
*/
GT_STATUS prvCpssDxChDiagDataIntegrityTableHwMaxIndexGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TABLE_ENT                 hwTable,
    OUT GT_U32                              *maxNumEntriesPtr,
    OUT GT_U32                              *maxNumLinesPtr
);

/**
* @internal prvCpssDxChTableEngineToHwRatioGet function
* @endinternal
*
* @brief   return the ratio between the index that the 'table engine' gets from the
*         'cpss API' to the 'HW index' in the table.
*         NOTE: this to support 'multi entries' in line or fraction of entry in line.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] ratioPtr                 - (pointer to) number of entries for in single line or
*                                      number of lines  for in single entry.
* @param[out] isMultiplePtr            - (pointer to) indication that ratio is 'multiple' or 'fraction'.
*                                      GT_TRUE  - ratio is 'multiple' (number of entries for in single line)
*                                      GT_FALSE - ratio is 'fraction' (number of lines  for in single entry)
*                                       GT_OK
*/
GT_STATUS prvCpssDxChTableEngineToHwRatioGet
(
    IN GT_U8                   devNum,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    OUT GT_U32                 *ratioPtr,
    OUT GT_BOOL                 *isMultiplePtr
);

/**
* @internal prvCpssDxChPortGroupReadTableEntry_fromShadow function
* @endinternal
*
* @brief   Read a whole entry from the 'SHADOW' table. - for specific portGroupId
*         operation involves copy of values from the shadow to the 'OUT buffer'
*         (pointer of entryValuePtr)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] entryValuePtr            - (pointer to) the data read from the entry in the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortGroupReadTableEntry_fromShadow
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  entryIndex,
    OUT GT_U32                 *entryValuePtr
);

/**
* @internal prvCpssDxChDiagDataIntegrityShadowAndHwSynch function
* @endinternal
*
* @brief   Synchronize all cpss shadow tables with values from HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagDataIntegrityShadowAndHwSynch
(
    IN GT_U8                devNum
);
/**
* @internal prvCpssDxChDiagDataIntegrityShadowAndHwSynchTable function
* @endinternal
*
* @brief   Synchronize specific table cpss shadow with values from HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] hwTable                  - the HW table to Synchronize
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_SUPPORTED         - the table not supports the shadow
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagDataIntegrityShadowAndHwSynchTable
(
    IN GT_U8                devNum,
    IN CPSS_DXCH_TABLE_ENT  hwTable
);

/**
* @internal prvCpssDxChDiagDataIntegrityIsMultipleNames function
* @endinternal
*
* @brief   is the HW table duplicated in HW.
*
* @param[in] hwTable                  - the HW table to check
*
* @param[out] mainTablePtr             - pointer to the 'main table' (ignored if NULL)
*
* @retval GT_TRUE                  - table has other main table name
*/
GT_BOOL prvCpssDxChDiagDataIntegrityIsMultipleNames(
    IN  CPSS_DXCH_TABLE_ENT hwTable,
    OUT CPSS_DXCH_TABLE_ENT *mainTablePtr
);

/**
* @internal prvCpssDxChDiagDataIntegrityShadowAndHwSynchTm function
* @endinternal
*
* @brief   Synchronize all TM related cpss shadow tables with values from HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagDataIntegrityShadowAndHwSynchTm
(
    IN GT_U8                devNum
);

/**
* @internal prvCpssDxChDiagDataIntegrityLogicalToHwTableMap function
* @endinternal
*
* @brief   Convert logical table entries range to an appropriate HW table entries
*         range. If logical range is too big the HW range relevant to sub-range
*         applicable for the HW table will be returned instead.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - The PP device number
* @param[in] logicalTable             - logical table
* @param[in] logicalStart             - logical item index starting the range
* @param[in] logicalRange             - range size
* @param[in] hwTable                  - HW table
*
* @param[out] hwStartPtr               - (pointer to) HW entry index appropriate to logicalStart.
*                                      Can be NULL.
* @param[out] hwRangePtr               - (pointer to) HW indexes range size.
*                                      Can be NULL.
* @param[out] hwLogicalFirstPtr        - (pointer to) the logical index appropriate
*                                      to the HW table first entry.
*                                      Can be NULL.
* @param[out] hwLogicalMaxPtr          - (pointer to) index of last+1 logical entry
*                                      appropriate to the full HW table size.
*                                      Can be NULL.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on impossible logicalRange
* @retval GT_BAD_PARAM             - on unknown logical table or HW table.
* @retval GT_BAD_VALUE             - if HW table belongs to logical table not covered by the
*                                       function
*
* @note Use logicalStart=0, logicalRange = 0xFFFFFFFF to get full HW range
*       relevant to the logical table
*
*/
GT_STATUS prvCpssDxChDiagDataIntegrityLogicalToHwTableMap
(
    IN  GT_U32                       devNum,
    IN  CPSS_DXCH_LOGICAL_TABLE_ENT  logicalTable,
    IN  GT_U32                       logicalStart,
    IN  GT_U32                       logicalRange,
    IN  CPSS_DXCH_TABLE_ENT          hwTable,
    OUT GT_U32                       *hwStartPtr,
    OUT GT_U32                       *hwRangePtr,
    OUT GT_U32                       *hwLogicalFirstPtr,
    OUT GT_U32                       *hwLogicalMaxPtr
);


/**
* @internal prvCpssDxChDiagDataIntegrityFillHwLogicalFromRam function
* @endinternal
*
* @brief   Using memory location (it is assumed filled) fill appropriately
*         HW and logical locations of CPSS_DXCH_LOCATION_FULL_INFO_STC struct.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Caelum.
*
* @param[in] devNum                   - PP device number
* @param[in,out] locationPtr          - (pointer to) location. HW- and logical-
*                                       related parts will be modified.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssDxChDiagDataIntegrityFillHwLogicalFromRam
(
    IN    GT_U8 devNum,
    INOUT CPSS_DXCH_LOCATION_FULL_INFO_STC *locationPtr
);


/**
* @internal prvCpssDxChDiagDataIntegrityDfxServerClientsIntMaskSet function
* @endinternal
*
* @brief   Mask/unmask bits indicating Data Integrity ECC/Parity errors in the
*         DFX registers "Server Interrupt Summary Mask Register",
*         "Client Interrupt Summary Mask Register".
*         It makes possible manage DFX memories events appearance inside this
*         registers via signle register - "Memory Interrupt Mask Resigter"
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - if pipe number is out of range.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_IMPLEMENTED       - if feature is not implemented for the device yet
*/
GT_STATUS prvCpssDxChDiagDataIntegrityDfxServerClientsIntMaskSet
(
    IN GT_U8                   devNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
);


/**
* @internal prvCpssDxChDfxClientsBmpGet function
* @endinternal
*
* @brief   return bitmap of DFX clients relevant for the specified DFX pipe.
*         If bit #N is raised client #N exists in the device.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - pipe index
*
* @param[out] clientsBmpPtr            - (pinter to) clients bitmap
*
* @retval GT_OK                    - on success
* @retval GT_NOT_IMPLEMENTED       - if device is not supported by the function yet
*/
GT_STATUS prvCpssDxChDfxClientsBmpGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   pipeId,
    OUT GT_U32   *clientsBmpPtr
);


/**
* @internal prvCpssDxChDiagDataIntegrityHwTableGet function
* @endinternal
*
* @brief   Function gets 'HW_table + offset' from DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Caelum.
*
* @param[in] devNum                   - PP device number
* @param[in,out] hwTableTypePtr       - in : HW table type.
*                                       out: HW table type as it is in DB.
*
* @param[out] offsetPtr                - (pointer to) HW table offset. Can be NULL.
* @param[out] maxEntriesPtr            - (pointer to) HW table actual number of entries.
*                                      Can be NULL.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagDataIntegrityHwTableGet
(
    IN  GT_U8                                           devNum,
    INOUT GT_U32                                      * hwTableTypePtr,
    OUT GT_U32                                        * offsetPtr,
    OUT GT_U32                                        * maxEntriesPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChDiagDataIntegrityMainMappingDbh */
