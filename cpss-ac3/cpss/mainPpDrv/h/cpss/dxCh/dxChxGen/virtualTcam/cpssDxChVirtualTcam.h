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
* @file cpssDxChVirtualTcam.h
*
* @brief The CPSS DXCH High Level Virtual TCAM Manager
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChVirtualTcam_h
#define __cpssDxChVirtualTcam_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>

/* maximal amount of Virtual TCAM Managers - Id range 0..31 */
#define CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS 31

/* maximal amount of vTCAMs - Id range 0..1023 */
#define CPSS_DXCH_VIRTUAL_TCAM_VTCAM_MAX_ID_CNS 1023

/* used to point the last rule place in Virtual TCAM */
#define CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS 0xFFFFFFFF

/* used to point the place in Virtual TCAM precedes the first rule */
#define CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS 0xFFFFFFFE

/* used to start iteration of rules in Virtual TCAM */
#define CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS 0xFFFFFFFF

/*
 * Typedef: CPSS_DXCH_VIRTUAL_TCAM_RULE_ID
 *
 * Description: Rule ID of virtual TCAM
 *
 *  used as the type for the rule ID
 *
 */
typedef GT_U32 CPSS_DXCH_VIRTUAL_TCAM_RULE_ID;

/**
* @enum CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT
 *
 * @brief Rule types.
*/
typedef enum{

    /** Rule type is PCL */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E,

    /** Rule type is TTI */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E

} CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT;

/**
* @enum CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT
 *
 * @brief Rule create types.
*/
typedef enum{

    /** @brief Rule placed by given logical index.
     *  If logical index of rule1 more than logical index of rule2
     *  than rule1 will be checked to match after rule2
     */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E,

    /** @brief Rule created by unique rule Id and attributes(priority).
     *  If priority of rule1 less than priority of rule2
     *  than rule1 will be checked to match before rule2
     *  If two rules created with the same priority
     *  the rule created later be checked to match later.
     */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E

} CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT;

/**
* @enum CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT
 *
 * @brief TCAM rule size.
*/
typedef enum{

    /** @brief TCAM rule size to be used 10 Bytes
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E = 0,

    /** @brief TCAM rule size to be used 20 Bytes
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E = 1,

    /** TCAM rule size to be used 30 Bytes */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E = 2,

    /** @brief TCAM rule size to be used 40 Bytes
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E = 3,

    /** @brief TCAM rule size to be used 50 Bytes
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon)
     */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E = 4,

    /** TCAM rule size to be used 60 Bytes */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E = 5,

    /** TCAM rule size to be used 80 Bytes */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E = 7,

    /** Standard rule size for xCat3 devices. */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E,

    /** Extended rule size for xCat3 devices. */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_EXT_E = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,

    /** Ultra rule size for xCat3 devices. */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ULTRA_E = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E

} CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT;

/**
* @enum CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT
 *
 * @brief Rule TCAM Availability.
*/
typedef enum{

    /** Free TCAM Memory already available */
    CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_AVAILABLE_E,

    /** Free TCAM Memory not available */
    CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_NOT_AVAILABLE_E

} CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT;

/**
* @enum CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT
 *
 * @brief Position or rule in range of the same priority rules.
*/
typedef enum{

    /** First in the range */
    CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E,

    /** Last in the range */
    CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E

} CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT;

/**
* @struct CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC
 *
 * @brief This structure describe attributes of the rule.
*/
typedef struct{

    /** @brief used only for priority driven vTcams.
     *  Comments:
     */
    GT_U32 priority;

} CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC;

/**
* @struct CPSS_DXCH_VIRTUAL_TCAM_INFO_STC
 *
 * @brief This structure to create virtual TCAM (partition).
*/
typedef struct{

    /** TCAM Client Group. */
    GT_U32 clientGroup;

    /** @brief TCAM paralell sublookup number;
     *  (APPLICABLE RANGES: 0..3);
     */
    GT_U32 hitNumber;

    /** size of rules used in virtual TCAM. */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT ruleSize;

    /** @brief GT_TRUE
     *  GT_FALSE - resized only when calling "Resize" API.
     *  Relevant only to priority driven vTCAMs,
     *  For locical-index driven vTCAMs GT_TRUE causes return GT_BAD_PARAM,.
     *  Automatic resize option never reduces vTCAM size less
     *  than guaranteedNumOfRules rules.
     */
    GT_BOOL autoResize;

    /** guaranteed amount of rules. */
    GT_U32 guaranteedNumOfRules;

    /** @brief rules addition method.
     *  Comments:
     */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT ruleAdditionMethod;

} CPSS_DXCH_VIRTUAL_TCAM_INFO_STC;

/**
* @struct CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC
 *
 * @brief This structure to check TCAM memory availability.
*/
typedef struct{

    /** @brief for existing vTcam being overriden or resized
     *  for new vTcam - GT_FALSE.
     */
    GT_BOOL isExist;

    /** virtual TCAM Id, relevant only when isExist == GT_TRUE */
    GT_U32 vTcamId;

    /** @brief virtual TCAM Info (see structure).
     *  Comments:
     */
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC vTcamInfo;

} CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC;

/**
* @struct CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC
 *
 * @brief This structure of rule action content
*/
typedef struct
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT       ruleType;
    union
    {
        struct
        {
            CPSS_PCL_DIRECTION_ENT             direction;
        } pcl;
    } action;
} CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC;

/**
* @struct CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC
 *
 * @brief This structure of rule action content
*/
typedef struct
{
    union
    {
        struct
        {
            CPSS_DXCH_PCL_ACTION_STC           *actionPtr;
        } pcl;
        struct
        {
            CPSS_DXCH_TTI_ACTION_UNT           *actionPtr;
        } tti;
    } action;
} CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC;

/**
* @struct CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC
 *
 * @brief This structure of rule content
*/
typedef struct
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT       ruleType;
    union
    {
        struct
        {
            CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
        } pcl;
        struct
        {
            CPSS_DXCH_TTI_RULE_TYPE_ENT        ruleFormat;
        } tti;
    } rule;
} CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC;

/**
* @struct CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC
 *
 * @brief This structure of rule data
*/
typedef struct
{
    GT_BOOL                                    valid;
    union
    {
        struct
        {
            CPSS_DXCH_PCL_RULE_FORMAT_UNT      *maskPtr;
            CPSS_DXCH_PCL_RULE_FORMAT_UNT      *patternPtr;
            CPSS_DXCH_PCL_ACTION_STC           *actionPtr;
        } pcl;
        struct
        {
            CPSS_DXCH_TTI_RULE_UNT             *maskPtr;
            CPSS_DXCH_TTI_RULE_UNT             *patternPtr;
            CPSS_DXCH_TTI_ACTION_UNT           *actionPtr;
        } tti;
    } rule;
} CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC;

/**
* @struct CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC
 *
 * @brief This structure to get virtual TCAM current usage.
*/
typedef struct{

    /** amount of used rules in virtual TCAM. */
    GT_U32 rulesUsed;

    /** @brief amount of free rules in virtual TCAM.
     *  Comments:
     */
    GT_U32 rulesFree;

} CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC;

/**
* @internal cpssDxChVirtualTcamManagerCreate function
* @endinternal
*
* @brief   Create Virtual TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_ALREADY_EXIST         - if already exist TCAM Manager with given Id
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory allocation fail
*/
GT_STATUS cpssDxChVirtualTcamManagerCreate
(
    IN  GT_U32                              vTcamMngId
);

/**
* @internal cpssDxChVirtualTcamManagerDelete function
* @endinternal
*
* @brief   Delete Virtual TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on devices still attached to TCAM manager
* @retval GT_NOT_FOUND             - on TCAM manager not found
*/
GT_STATUS cpssDxChVirtualTcamManagerDelete
(
    IN  GT_U32                              vTcamMngId
);

/**
* @internal cpssDxChVirtualTcamManagerDevListAdd function
* @endinternal
*
* @brief   This function adds devices to an existing Virtual TCAM manager.
*         TCAM of one of old devices (if exist) copied to TCAMs the newly added devices.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] devListArr[]             - the array of device ids to add to the Virtual TCAM Manager.
*                                      (APPLICABLE RANGES: 0..255)
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
*
* @note NONE
*
*/
GT_STATUS cpssDxChVirtualTcamManagerDevListAdd
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
);

/**
* @internal cpssDxChVirtualTcamManagerDevListRemove function
* @endinternal
*
* @brief   This function removes devices from an existing Virtual TCAM manager.
*         TCAMs of removed devices not updated.
*         Removing last device caused cleaning up all Tcam Manager configurations.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] devListArr[]             - the array of device ids to remove from the
*                                      Virtual TCAM Manager.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_NOT_INITIALIZED       - if there are no devices in DB
*
* @note NONE
*
*/
GT_STATUS cpssDxChVirtualTcamManagerDevListRemove
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
);


/**
* @internal cpssDxChVirtualTcamDefrag function
* @endinternal
*
* @brief   Defragment TCAM manager memory.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamDefrag
(
    IN  GT_U32                              vTcamMngId
);

/**
* @internal cpssDxChVirtualTcamMemoryAvailabilityCheck function
* @endinternal
*
* @brief   Check availability of TCAM manager memory for list of new and existing Virtual TCAMs.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamAmount              - amount of Virtual TCAMs in the array.
* @param[in] vTcamCheckInfoArr[]      - array of infos for Virtual TCAMs.
*
* @param[out] tcamAvailabilityPtr      - (pointer to)TCAM Availability (see enum)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if pointer or array-pointer is NULL.
* @retval GT_NOT_FOUND             - TCAM manager with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamMemoryAvailabilityCheck
(
    IN   GT_U32                                 vTcamMngId,
    IN   GT_U32                                 vTcamAmount,
    IN   CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC  vTcamCheckInfoArr[],
    OUT  CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT *tcamAvailabilityPtr
);

/**
* @internal cpssDxChVirtualTcamCreate function
* @endinternal
*
* @brief   Create Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] vTcamInfoPtr             - (pointer to) Virtual TCAM info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_ALREADY_EXIST         - vTcam with given Id exists
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamCreate
(
    IN  GT_U32                           vTcamMngId,
    IN  GT_U32                           vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC   *vTcamInfoPtr
);

/**
* @internal cpssDxChVirtualTcamRemove function
* @endinternal
*
* @brief   Remove Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - vTcam contains valid rules
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRemove
(
    IN  GT_U32                             vTcamMngId,
    IN  GT_U32                             vTcamId
);

/**
* @internal cpssDxChVirtualTcamResize function
* @endinternal
*
* @brief   Resize Virtual TCAM. Guaranteed amount of rules updated.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] rulePlace                - place to insert/remove space after it.
*                                      for logical-index driven vTCAMs - logical-index,
*                                      for priority driven vTCAMs - priority value.
*                                      ----------------------------------------
*                                      For logical-index driven vTCAMs:
*                                      a) toInsert == GT_TRUE
*                                      Function adds free space with size <sizeInRules> rules
*                                      after logical-index == rulePlace.
*                                      Logical indexes of existing rules higher than inserted space
*                                      (i.e. rules with old-logical-index >= rulePlace)
*                                      are increased by <sizeInRules>.
*                                      b) toInsert == GT_FALSE
*                                      Function invalidates existing rules with logical indexes
*                                      between <rulePlace> and <rulePlace + sizeInRules - 1>.
*                                      This space became free. Function decreases logical indexes of
*                                      existing rules higher than freed space
*                                      (i.e. rules with old-logical-index >= rulePlace) by <sizeInRules>.
*                                      ----------------------------------------
*                                      For priority driven vTCAMs:
*                                      a) toInsert == GT_TRUE
*                                      Function adds free space with size <sizeInRules> rules
*                                      after all rules with priority == rulePlace.
*                                      b) toInsert == GT_FALSE
*                                      Function removes and frees space of <sizeInRules> rules
*                                      after all rules with priority == rulePlace.
*                                      No rules are removed, but rules can be moved in TCAM
*                                      to gather free spaces to one chunk .
*                                      Rules with given and lower priorities can be compressed
*                                      using free spaces between them.
*                                      ----------------------------------------
*                                      CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS - used
*                                      for logical-index driven vTCAMs only
*                                      to append free space (i.e toInsert == GT_TRUE)
*                                      CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS - used
*                                      for logical-index driven vTCAMs only
*                                      to insert free space before the 0-th rule (i.e toInsert == GT_TRUE)
* @param[in] toInsert                 - GT_TRUE - insert rule-places, GT_FALSE - remove rule-places
*                                      logical index of rules up to inserted space increased,
*                                      logical index of rules up to removed space - decreased,
*                                      rules inside removed space - removed.
* @param[in] sizeInRules              - amount of rule-space to insert/remove
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note Example1 (logical-index driven vTCAM):
*       Old vTcam size = 200, rulePlace = 50, toInsert = GT_TRUE, sizeInRules = 100
*       New vTam size is 300.
*       Rules 0-49 logical index unchanged.
*       Rules 50-199 logical index increased by 100 to be 150-299.
*       Place for rules 50-149 added, contains no valid rules.
*       Example2 (logical-index driven vTCAM)
*       Old vTcam size = 300, rulePlace = 50, toInsert = GT_FALSE, sizeInRules = 100
*       New vTam size is 200.
*       Rules 0-49 logical index unchanged.
*       Rules 150-299 logical index decreased by 100 to be 50-199.
*       Place of rules 50-149 removed with all rules that contained.
*
*/
GT_STATUS cpssDxChVirtualTcamResize
(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  GT_U32                                rulePlace,
    IN  GT_BOOL                               toInsert,
    IN  GT_U32                                sizeInRules
);

/**
* @internal cpssDxChVirtualTcamInfoGet function
* @endinternal
*
* @brief   Get Virtual TCAM info.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*
* @param[out] vTcamInfoPtr             - (pointer to) Virtual TCAM Info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamInfoGet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    OUT  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC    *vTcamInfoPtr
);

/**
* @internal cpssDxChVirtualTcamUsageGet function
* @endinternal
*
* @brief   Get Virtual TCAM usage.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*
* @param[out] vTcamUsagePtr            - (pointer to) Virtual TCAM Usage structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamUsageGet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    OUT  CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC   *vTcamUsagePtr
);

/**
* @internal cpssDxChVirtualTcamNextRuleIdGet function
* @endinternal
*
* @brief   Function gets next existing rule ID.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*                                      CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS used to start iteration of rules
*
* @param[out] nextRuleIdPtr            -      (pointer to)id of next rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_EMPTY                 - on 'START_RULE_ID' indication and no rules exists.
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NO_MORE               - all rule already passed by iteration
*
* @note The next rule is once written and not removed rule.
*       It can be valid or invalid.
*       It is the next by lookup order.
*
*/
GT_STATUS cpssDxChVirtualTcamNextRuleIdGet
(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID        ruleId,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_ID        *nextRuleIdPtr
);

/**
* @internal cpssDxChVirtualTcamRuleWrite function
* @endinternal
*
* @brief   Write Rule to Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*                                      if rule with given rule Id already exists it overridden only
*                                      when the same priotity specified, otherwize GT_BAD_PARAM returned.
* @param[in] ruleAttributesPtr        - (pointer to)rule attributes (for priority driven vTCAM - priority)
* @param[in] ruleTypePtr              -     (pointer to)type of contents of rule
* @param[in] ruleDataPtr              -     (pointer to)data of contents of rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note The function may expand the the vTCAM (when needed) by additional rules
*       to globally configurable granularity. This granularity is configured by
*       cpssDxChVirtualTcamAutoResizeGranularitySet function and rounded to whole
*       TCAM row size (for SIP6 - 60 bytes). The default granularity is 12 rules.
*/
GT_STATUS cpssDxChVirtualTcamRuleWrite
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  *ruleAttributesPtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
);

/**
* @internal cpssDxChVirtualTcamRuleRead function
* @endinternal
*
* @brief   Read Rule from Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] ruleTypePtr              -     (pointer to)type of contents of rule
*
* @param[out] ruleDataPtr              -      (pointer to)data of contents of rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRuleRead
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
);

/**
* @internal cpssDxChVirtualTcamRuleActionUpdate function
* @endinternal
*
* @brief   Update Rule Action in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] actionTypePtr            - (pointer to)type of contents of action
* @param[in] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRuleActionUpdate
(
    IN  GT_U32                                        vTcamMngId,
    IN  GT_U32                                        vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
);

/**
* @internal cpssDxChVirtualTcamRuleActionGet function
* @endinternal
*
* @brief   Get Rule Action in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] actionTypePtr            - (pointer to)type of contents of action
*
* @param[out] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRuleActionGet
(
    IN  GT_U32                                        vTcamMngId,
    IN  GT_U32                                        vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
);

/**
* @internal cpssDxChVirtualTcamRuleValidStatusSet function
* @endinternal
*
* @brief   Set Rule Valid Status in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] valid                    - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRuleValidStatusSet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId,
    IN   GT_BOOL                            valid
);

/**
* @internal cpssDxChVirtualTcamRuleValidStatusGet function
* @endinternal
*
* @brief   Get Rule Valid Status in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @param[out] validPtr                 - (pointer to)GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRuleValidStatusGet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId,
    OUT  GT_BOOL                            *validPtr
);

/**
* @internal cpssDxChVirtualTcamRuleDelete function
* @endinternal
*
* @brief   Delete Rule From internal DB and Invalidate it in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRuleDelete
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId
);

/**
* @internal cpssDxChVirtualTcamRuleMove function
* @endinternal
*
* @brief   Move Rule in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] srcRuleId                - source rule id: for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] dstRuleId                - destination rule id: for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRuleMove
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     srcRuleId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     dstRuleId
);

/**
* @internal cpssDxChVirtualTcamRulePriorityGet function
* @endinternal
*
* @brief   Get Rule Priority (for priority driven Virtual TCAMs only).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   -  that used to refer existing rule.
*
* @param[out] rulePriorityPtr          - (pointer to)rule priority
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters and for logical index driven Virtual TCAMs.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamRulePriorityGet
(
    IN   GT_U32                                      vTcamMngId,
    IN   GT_U32                                      vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    OUT  GT_U32                                      *rulePriorityPtr
);

/**
* @internal cpssDxChVirtualTcamRulePriorityUpdate function
* @endinternal
*
* @brief   Update rule priority (for priority driven vTCAM).
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - rule id: that used to refer existing rule
* @param[in] priority                 - the new  of the rule
* @param[in] position                 -  in range of rule of the
*                                      specified priority (see enum)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FULL                  - when the TCAM (not only the vTcam) is FULL
*                                       and moving the rule will require the entry to
*                                       be absent from the HW for some time.
*                                       (the function can not guarantee loss less hits
*                                       on that rule)
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRulePriorityUpdate
(
    IN   GT_U32                                                    vTcamMngId,
    IN   GT_U32                                                    vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                            ruleId,
    IN   GT_U32                                                    priority,
    IN   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT   position
);

/**
* @internal cpssDxChVirtualTcamDbRuleIdToHwIndexConvert function
* @endinternal
*
* @brief   debug function - convert rule Id to logical index and to hardware index.
*         purpose is to allow application to know where the CPSS inserted the rule
*         in to the hardware.
*         this is 'DB' (DataBase) operation that not access the hardware.
*         NOTE: the function will return failure if the ruleId was not inserted.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   -  to find.
*
* @param[out] logicalIndexPtr          - (pointer to) the logical index.  (can be NULL)
* @param[out] hwIndexPtr               - (pointer to) the hardware index. (can be NULL)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - if both logicalIndexPtr and hwIndexPtr are NULL
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChVirtualTcamDbRuleIdToHwIndexConvert
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    OUT  GT_U32                                     *logicalIndexPtr,
    OUT  GT_U32                                     *hwIndexPtr
);

/**
* @internal cpssDxChVirtualTcamAutoResizeEnable function
* @endinternal
*
* @brief   Set the auto resize enable in Priority mode Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] enable                   - for logical-index driven vTCAM - NOT APPLICABLE
*                                      for priority driven vTCAM without auto-resize - NOT APPLICABLE
*                                      for priority driven vTCAM with auto-resize -
*                                      FALSE - Auto-resize forced OFF
*                                      TRUE - Auto-resize restored to ON (default)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamAutoResizeEnable
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   GT_BOOL                            enable
);

/**
* @internal cpssDxChVirtualTcamAutoResizeGranularitySet function
* @endinternal
*
* @brief   Set the auto resize granularity in Priority mode Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] granularity              - Granularity (in rules) to be applied at auto-resize events
*                                      on vTCAMs belonging to this vTCAM manager.
*                                      (Rounded up to a whole TCAM row)
*                                      (APPLICABLE RANGES: 1..240)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note To be called only while vTCAM manager has no devices attached
*
*/
GT_STATUS cpssDxChVirtualTcamAutoResizeGranularitySet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             granularity
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChVirtualTcam_h */


