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
* @file prvCpssDxChHsu.h
*
* @brief CPSS DxCh HSU data.
*
* @version   6
********************************************************************************
*/

#ifndef __prvCpssDxChHsuh
#define __prvCpssDxChHsuh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/generic/systemRecovery/hsu/cpssGenHsu.h>
#include <cpss/generic/systemRecovery/hsu/private/prvCpssGenHsu.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>
#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>

#define PRV_CPSS_DXCH_HSU_GET_SIZE_IN_ONE_ITERATION_CNC 0xffffffff
#define PRV_CPSS_DXCH_HSU_ITERATOR_MAGIC_NUMBER_CNC     0xEFABCD90

/**
* @enum PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_ENT
 *
 * @brief it shows how to update stageArray structure.
*/
typedef enum{

    /** it used to update size and address only for rx/tx stages */
    PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_RXTX_E,

    /** it used to update size and address for all stages */
    PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_ALL_E

} PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_ENT;

/**
* @enum PRV_CPSS_DXCH_HSU_GLOBAL_DATA_STAGE_ENT
 *
 * @brief It represents global shadow iterator stages
 * values:
 * PRV_CPSS_DXCH_HSU_SYS_GLOBAL_STAGE_E              - global system stage
 * PRV_CPSS_DXCH_HSU_GEN_PP_CONFIG_STAGE_E             - device generic stage
 * PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE0_STAGE_E             - rx queue0 stage
 * PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE1_STAGE_E             - rx queue1 stage
 * PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE2_STAGE_E             - rx queue2 stage
 * PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE3_STAGE_E             - rx queue3 stage
 * PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE4_STAGE_E             - rx queue4 stage
 * PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE5_STAGE_E             - rx queue5 stage
 * PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE6_STAGE_E             - rx queue6 stage
 * PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE7_STAGE_E             - rx queue7 stage
 * PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE0_STAGE_E             - tx queue0 stage
 * PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE1_STAGE_E             - tx queue1 stage
 * PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE2_STAGE_E             - tx queue2 stage
 * PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE3_STAGE_E             - tx queue3 stage
 * PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE4_STAGE_E             - tx queue4 stage
 * PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE5_STAGE_E             - tx queue5 stage
 * PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE6_STAGE_E             - tx queue6 stage
 * PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE7_STAGE_E             - tx queue7 stage
 * PRV_CPSS_DXCH_HSU_GEN_TX_GENERATOR_COOKIE_STAGE_E        - tx generator cookie stage
 * PRV_CPSS_DXCH_HSU_GEN_TX_GENERATOR_FREE_LINKED_LIST_STAGE_E   - tx generator free linked list stage
 * PRV_CPSS_DXCH_HSU_GEN_TRUNK_ARRAY_STAGE_E            - trunk array stage
 * PRV_CPSS_DXCH_HSU_GEN_TRUNK_ALL_MEMBERS_ARRAY_STAGE_E      - all trunk members array stage
 * PRV_CPSS_DXCH_HSU_GEN_TRUNK_ALL_MEMBERS_IS_LOCAL_ARRAY_STAGE_E - trunk local indication array stage
 * PRV_CPSS_DXCH_HSU_GEN_PORT_INFO_ARRAY_STAGE_E          - port info array stage
 * PRV_CPSS_DXCH_HSU_BRIDGE_STAGE_E                - brige info stage
 * PRV_CPSS_DXCH_HSU_NETIF_STAGE_E                 - network info stage
 * PRV_CPSS_DXCH_HSU_POLICER_STAGE_E                - policer info stage
 * PRV_CPSS_DXCH_HSU_PORT_STAGE_E                 - port info stage
 * PRV_CPSS_DXCH_HSU_PORT_GROUPS_STAGE_E              - port groups info stage
 * PRV_CPSS_DXCH_HSU_SERDES_BMP_STAGE_E              - bitmap of SERDES lanes specify stages of lanes appliable
 * (bit 0-> lane 0, bit 1 -> lane 1, etc.)
 * for example if (bit 3 == 1) it means lane 3
 * was initialized and have data that should be
 * exported and imported.
 * if (bit3==0) then the lane is NULL and no data
 * to be saved in HSU operation
 * PRV_CPSS_DXCH_HSU_SERDES_LANE0_STAGE_E             - serdes lane 0 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE1_STAGE_E             - serdes lane 1 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE2_STAGE_E             - serdes lane 2 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE3_STAGE_E             - serdes lane 3 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE4_STAGE_E             - serdes lane 4 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE5_STAGE_E             - serdes lane 5 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE6_STAGE_E             - serdes lane 6 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE7_STAGE_E             - serdes lane 7 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE8_STAGE_E             - serdes lane 8 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE9_STAGE_E             - serdes lane 9 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE10_STAGE_E             - serdes lane 10 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE11_STAGE_E             - serdes lane 11 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE12_STAGE_E             - serdes lane 12 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE13_STAGE_E             - serdes lane 13 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE14_STAGE_E             - serdes lane 14 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE15_STAGE_E             - serdes lane 15 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE16_STAGE_E             - serdes lane 16 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE17_STAGE_E             - serdes lane 17 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE18_STAGE_E             - serdes lane 18 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE19_STAGE_E             - serdes lane 19 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE20_STAGE_E             - serdes lane 20 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE21_STAGE_E             - serdes lane 21 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE22_STAGE_E             - serdes lane 22 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE23_STAGE_E             - serdes lane 23 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE24_STAGE_E             - serdes lane 24 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE25_STAGE_E             - serdes lane 25 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE26_STAGE_E             - serdes lane 26 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE27_STAGE_E             - serdes lane 27 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE28_STAGE_E             - serdes lane 28 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE29_STAGE_E             - serdes lane 29 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE30_STAGE_E             - serdes lane 30 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE31_STAGE_E             - serdes lane 31 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE32_STAGE_E             - serdes lane 32 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE33_STAGE_E             - serdes lane 33 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE34_STAGE_E             - serdes lane 34 stage
 * PRV_CPSS_DXCH_HSU_SERDES_LANE35_STAGE_E             - serdes lane 35 stage
 * PRV_CPSS_DXCH_HSU_GLOBAL_LAST_STAGE_E              - last stage
*/
typedef enum{

    PRV_CPSS_DXCH_HSU_SYS_GLOBAL_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_PP_CONFIG_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE0_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE1_STAGE_E ,

    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE2_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE3_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE4_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE5_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE6_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_RX_QUEUE7_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE0_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE1_STAGE_E ,

    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE2_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE3_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE4_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE5_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE6_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TX_QUEUE7_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TX_GENERATOR_COOKIE_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TX_GENERATOR_FREE_LINKED_LIST_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TRUNK_ARRAY_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_TRUNK_ALL_MEMBERS_ARRAY_STAGE_E ,

    PRV_CPSS_DXCH_HSU_GEN_TRUNK_ALL_MEMBERS_IS_LOCAL_ARRAY_STAGE_E,

    PRV_CPSS_DXCH_HSU_GEN_PORT_INFO_ARRAY_STAGE_E,

    PRV_CPSS_DXCH_HSU_BRIDGE_STAGE_E,

    PRV_CPSS_DXCH_HSU_NETIF_STAGE_E,

    PRV_CPSS_DXCH_HSU_POLICER_STAGE_E,

    PRV_CPSS_DXCH_HSU_PORT_STAGE_E,

    PRV_CPSS_DXCH_HSU_PORT_GROUPS_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_BMP_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE0_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE1_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE2_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE3_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE4_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE5_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE6_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE7_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE8_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE9_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE10_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE11_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE12_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE13_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE14_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE15_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE16_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE17_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE18_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE19_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE20_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE21_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE22_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE23_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE24_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE25_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE26_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE27_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE28_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE29_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE30_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE31_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE32_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE33_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE34_STAGE_E,

    PRV_CPSS_DXCH_HSU_SERDES_LANE35_STAGE_E,

    PRV_CPSS_DXCH_HSU_GLOBAL_LAST_STAGE_E

} PRV_CPSS_DXCH_HSU_GLOBAL_DATA_STAGE_ENT;



/**
* @enum PRV_CPSS_DXCH_HSU_DATA_MAIN_PHASE_TYPE_ENT
 *
 * @brief Types of cpss data for export/import/size_calculations actions during
 * HSU process. It could be cpss internal data structures, shadow tables,
 * global variables, data per feature/mechanism.
*/
typedef enum{

    /** LPM DB HSU phase */
    PRV_CPSS_DXCH_HSU_DATA_MAIN_PHASE_LPM_DB_E,

    /** Global HSU phase */
    PRV_CPSS_DXCH_HSU_DATA_MAIN_PHASE_GLOBAL_E

} PRV_CPSS_DXCH_HSU_DATA_MAIN_PHASE_TYPE_ENT;


/**
* @struct PRV_CPSS_DXCH_HSU_MAIN_FRAME_ITERATOR_STC
 *
 * @brief This struct that holds HSU global shadow iterator
*/
typedef struct{

    /** amount of HSU data that was processed so far */
    GT_U32 hsuAllAccumulatedSize;

    /** all hsu data size that should be treated */
    GT_U32 hsuAllDataSize;

    /** hsu current phase . */
    PRV_CPSS_DXCH_HSU_DATA_MAIN_PHASE_TYPE_ENT hsuCurrentPhase;

} PRV_CPSS_DXCH_HSU_MAIN_FRAME_ITERATOR_STC;


/**
* @struct PRV_CPSS_DXCH_HSU_GLOBAL_ITERATOR_STC
 *
 * @brief This struct that holds HSU global shadow iterator
*/
typedef struct
{
    PRV_CPSS_HSU_GEN_ITERATOR_STC             genIterator;
    PRV_CPSS_DXCH_HSU_GLOBAL_DATA_STAGE_ENT   currStage;
    GT_U8                                     devNum;
    GT_U32                                    hsuBlockGlobalSize;
    GT_U32                                    accumulatedGlobalSize;
    GT_BOOL                                   systemDataProcessed;
    GT_BOOL                                   genStageComplete;
    PRV_CPSS_SW_RX_DESC_STC                   *rxOrigDescArray[NUM_OF_RX_QUEUES][3];
    PRV_CPSS_SW_TX_DESC_STC                   *txOrigDescArray[NUM_OF_TX_QUEUES][3];
    PRV_CPSS_TRUNK_ENTRY_INFO_STC             *trunksArrayPtr;
    CPSS_TRUNK_MEMBER_STC                     *allMembersArrayPtr;
    GT_BOOL                                   *allMembersIsLocalArrayPtr;
    PRV_CPSS_PORT_INFO_ARRAY_STC              *phyPortInfoArrayPtr;
    PRV_CPSS_FAMILY_TRUNK_BIND_FUNC_STC       *virtualFunctionsPtr;
    PRV_CPSS_PHY_INFO_STC                     *phyInfoPtr;
    CPSS_MACDRV_OBJ_STC                       **portMacObjPtr;
    PRV_CPSS_INTERRUPT_CTRL_STC               *allDescListPtr;
    struct{
        GT_U32      *cookie;
        GT_U32      *freeLinkedList;
        GT_U32      firstFree;
        GT_U32      generatorsTotalDesc;
    }txGeneratorPacketIdDb;
}PRV_CPSS_DXCH_HSU_GLOBAL_ITERATOR_STC;


/**
* @struct PRV_CPSS_DXCH_HSU_LPM_DB_ITERATOR_STC
 *
 * @brief This struct that holds LPM DB shadow iterator
*/
typedef struct{

    /** hsu LPM DBs data that was processed so far */
    GT_U32 accumulatedLpmDbSize;

    /** @brief hsu LPM DBs data size that should be treated
     *  isLpmDbProcessed    - (pointer to) LPM DBs processing status array .
     */
    GT_U32 lpmDbSize;

    GT_BOOL *isLpmDbProcessed;

} PRV_CPSS_DXCH_HSU_LPM_DB_ITERATOR_STC;

/**
* @struct PRV_CPSS_DXCH_HSU_STAGE_STC
 *
 * @brief The iteration stage information.
*/
typedef struct{

    /** stage size in bytes */
    GT_U32 stageSize;

    /** stage address */
    GT_U8* stageAddress;

} PRV_CPSS_DXCH_HSU_STAGE_STC;

/**
* @internal prvCpssDxChHsuLpmDbDataBlockSizeGet function
* @endinternal
*
* @brief   This function gets the memory size needed to export the LPM DB HSU
*         data block.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] sizePtr                  - the LPM DB size calculated in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuLpmDbDataBlockSizeGet
(
    OUT   GT_U32    *sizePtr
);

/**
* @internal prvCpssDxChHsuGlobalDataBlockSizeGet function
* @endinternal
*
* @brief   This function gets the memory size needed to export the Global HSU
*         data block.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] sizePtr                  - the Global HSU data block size calculated in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuGlobalDataBlockSizeGet
(
    OUT   GT_U32    *sizePtr
);

/**
* @internal prvCpssDxChHsuGlobalDataBlockExport function
* @endinternal
*
* @brief   This function exports Global HSU data block to survived restart
*         memory area supplied by application.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] iteratorPtr              - pointer to iterator, to start - set iterator to 0.
* @param[in,out] hsuBlockMemSizePtr       - pointer hsu block data size supposed to be exported
*                                      in current iteration. The minimal value is 1k bytes.
* @param[in] hsuBlockMemPtr           - pointer to HSU survived restart memory area
* @param[in,out] iteratorPtr              - the iterator - points to the point from which
*                                      process will be continued in future iteration.
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size exported in current iteration.
*
* @param[out] exportCompletePtr        - GT_TRUE -  HSU data export is completed.
*                                      GT_FALSE - HSU data export is not completed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong hsuBlockMemSize, wrong iterator.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuGlobalDataBlockExport
(
    INOUT  GT_UINTPTR                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *exportCompletePtr
);

/**
* @internal prvCpssDxChHsuGlobalDataBlockImport function
* @endinternal
*
* @brief   This function imports Global HSU data block from survived restart
*         memory area supplied by application.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] iteratorPtr              - pointer to iterator, to start - set iterator to 0.
* @param[in,out] hsuBlockMemSizePtr       - pointer hsu block data size supposed to be exported
*                                      in current iteration. The minimal value is 1k bytes.
* @param[in] hsuBlockMemPtr           - pointer to HSU survived restart memory area
* @param[in,out] iteratorPtr              - the iterator - points to the point from which
*                                      process will be continued in future iteration.
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size imported in current iteration.
*
* @param[out] importCompletePtr        - GT_TRUE -  HSU data import is completed.
*                                      GT_FALSE - HSU data import is not completed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong hsuBlockMemSize, wrong iterator.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuGlobalDataBlockImport
(
    INOUT  GT_UINTPTR                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *importCompletePtr
);

/**
* @internal prvCpssDxChHsuLpmDbDataBlockExport function
* @endinternal
*
* @brief   This function exports LPM DB HSU data block to survived restart
*         memory area supplied by application.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] iteratorPtr              - pointer to iterator, to start - set iterator to 0.
* @param[in,out] hsuBlockMemSizePtr       - pointer hsu block data size supposed to be exported
*                                      in current iteration. The minimal value is 1k bytes.
* @param[in] hsuBlockMemPtr           - pointer to HSU survived restart memory area
* @param[in,out] iteratorPtr              - the iterator - points to the point from which
*                                      process will be continued in future iteration.
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size exported in current iteration.
*
* @param[out] exportCompletePtr        - GT_TRUE -  HSU data export is completed.
*                                      GT_FALSE - HSU data export is not completed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong hsuBlockMemSize.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuLpmDbDataBlockExport
(
    INOUT  GT_UINTPTR                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *exportCompletePtr
);

/**
* @internal prvCpssDxChHsuLpmDbDataBlockImport function
* @endinternal
*
* @brief   This function imports LPM DB HSU data block from survived restart
*         memory area supplied by application.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] iteratorPtr              - pointer to iterator, to start - set iterator to 0.
* @param[in,out] hsuBlockMemSizePtr       - pointer hsu block data size supposed to be exported
*                                      in current iteration. The minimal value is 1k bytes.
* @param[in] hsuBlockMemPtr           - pointer to HSU survived restart memory area
* @param[in,out] iteratorPtr              - the iterator - points to the point from which
*                                      process will be continued in future iteration.
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size exported in current iteration.
*
* @param[out] importCompletePtr        - GT_TRUE -  HSU data import is completed.
*                                      GT_FALSE - HSU data import is not completed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong hsuBlockMemSize.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuLpmDbDataBlockImport
(
    INOUT  GT_UINTPTR                    *iteratorPtr,
    INOUT  GT_U32                        *hsuBlockMemSizePtr,
    IN     GT_U8                         *hsuBlockMemPtr,
    OUT    GT_BOOL                       *importCompletePtr
);

/**
* @internal prvCpssDxChHsuRestoreRxTxSwDescriptorsChain function
* @endinternal
*
* @brief   This function restores RxTx Sw descriptors chain.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuRestoreRxTxSwDescriptorsChain
(
    IN     GT_U8                         devNum
);

/**
* @internal prvCpssDxChHsuRxTxImportPreparation function
* @endinternal
*
* @brief   This function prepares for import stage.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on out of CPU memory
* @retval GT_NO_RESOURCE           - on NO_RESOURCE under cpssBmPoolCreate
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuRxTxImportPreparation
(
    IN   GT_U8                                   devNum
);

/**
* @internal prvCpssDxChHsuSerdesImportPreparation function
* @endinternal
*
* @brief   This function prepares for serdes import stage.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on out of CPU memory
* @retval GT_NO_RESOURCE           - on NO_RESOURCE under cpssBmPoolCreate
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuSerdesImportPreparation
(
    IN   GT_U8                                   devNum
);

/**
* @internal prvCpssDxChHsuExportImportDataHandling function
* @endinternal
*
* @brief   This function handle import/export data.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] actionType               - PRV_CPSS_DXCH_HSU_EXPORT_E - export action
*                                      PRV_CPSS_DXCH_HSU_IMPORT_E - import action
* @param[in,out] currentIterPtr           - points to the current iteration.
* @param[in,out] hsuBlockMemSizePtr       - pointer hsu block data size supposed to be exported
*                                      in current iteration.
* @param[in,out] hsuBlockMemPtrPtr        - pointer to HSU survived restart memory area
* @param[in,out] currentIterPtr           - points to the current iteration
* @param[in,out] hsuBlockMemSizePtr       - pointer to hsu block data size exported in current iteration.
* @param[in,out] hsuBlockMemPtrPtr        - pointer to HSU survived restart memory area
*
* @param[out] accumSizePtr             - points to accumulated size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong hsuBlockMemSize, wrong iterator.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuExportImportDataHandling
(
    IN     PRV_CPSS_HSU_ACTION_TYPE_ENT            actionType,
    INOUT  PRV_CPSS_DXCH_HSU_GLOBAL_ITERATOR_STC   *currentIterPtr,
    INOUT  GT_U32                                  *hsuBlockMemSizePtr,
    INOUT  GT_U8                                   **hsuBlockMemPtrPtr,
    OUT    GT_U32                                  *accumSizePtr
);

/**
* @internal prvCpssDxChHsuConvertStageInQueueIndex function
* @endinternal
*
* @brief   This function converts stage enum to tx queue index.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] stageIndex               - import/export tx stage
*
* @param[out] queueIndexPtr            - pointer to tx queue index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad hsu data type
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuConvertStageInQueueIndex
(
    IN   PRV_CPSS_DXCH_HSU_GLOBAL_DATA_STAGE_ENT stageIndex,
    OUT  GT_U32                                  *queueIndexPtr
);

/**
* @internal prvCpssDxChSystemRecoveryDisableModeHandle function
* @endinternal
*
* @brief   Handling au and fu queues,Rx SDMA to provide smooth reprogramming in new immage.
*         It is applicable to system recovery mode when application is not interested
*         in getting Rx/Au/Fu messages during system recovery process.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChSystemRecoveryDisableModeHandle
(
    GT_VOID
);

/**
* @internal prvCpssDxChHsuInitStageArray function
* @endinternal
*
* @brief   This function inits stage size array for given HSU data block.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] dataType                 - stageArray data type
*
* @param[out] stageArrayPtr[]          - pointer to stage array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad hsu data type
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note none.
*
*/
GT_STATUS prvCpssDxChHsuInitStageArray
(
    IN   GT_U8                                       devNum,
    IN   PRV_CPSS_DXCH_HSU_STAGE_ARRAY_DATA_TYPE_ENT dataType,
    OUT  PRV_CPSS_DXCH_HSU_STAGE_STC                 stageArrayPtr[]
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHsuh */

