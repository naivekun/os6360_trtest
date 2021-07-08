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
* @file cpssDxChBrgFdb.h
*
* @brief FDB tables facility CPSS DxCh implementation.
*
* @version   44
********************************************************************************
*/
#ifndef __cpssDxChBrgFdbh
#define __cpssDxChBrgFdbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>

/**
* @enum CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT
 *
 * @brief FDB Lookup Key mode
*/
typedef enum{

    /** use Single Tag Key */
    CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E,

    /** use Double Tag Key */
    CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E

} CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT;

/**
* @enum CPSS_DXCH_FDB_QUEUE_TYPE_ENT
 *
 * @brief Enumeration of FDB message queues (AU/FU)
*/
typedef enum{

    /** type for AUQ (address update queue) */
    CPSS_DXCH_FDB_QUEUE_TYPE_AU_E,

    /** type for FUQ (FDB upload queue) */
    CPSS_DXCH_FDB_QUEUE_TYPE_FU_E

} CPSS_DXCH_FDB_QUEUE_TYPE_ENT;



/**
* @enum CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT
 *
 * @brief Enumeration for trunk aging mode
*/
typedef enum{

    /** @brief Entries associated with Trunks
     *  are aged out according to the <ActionMode> configuration.
     */
    CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_REGULAR_E,

    /** @brief Entries associated with Trunks are agedwithoutremoval regardless
     *  of the dev# they are associated with and regardless of the configuration
     *  in the ActionMode configuration
     */
    CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_FORCE_AGE_WITHOUT_REMOVAL_E

} CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT;

/**
* @enum CPSS_DXCH_BRG_FDB_DEL_MODE_ENT
 *
 * @brief Enum for FDB Deletetion Modes
*/
typedef enum{

    /** Only dynamic addresses matching the deleting action are deleted */
    CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E  = GT_FALSE,

    /** Dynamic and static addresses matching the deleting action are deleted */
    CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E = GT_TRUE,

    /** @brief Only static addresses matching the deleting action are deleted.
     *  APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon.
     */
    CPSS_DXCH_BRG_FDB_DEL_MODE_STATIC_ONLY_E

} CPSS_DXCH_BRG_FDB_DEL_MODE_ENT;

/**
* @enum CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT
 *
 * @brief An enum to state FDB mac entry muxing modes.
 * how the 12 bits that are muxed in the FDB mac entry.
 * APPLICABLE DEVICES: Falcon.
 * Values:
 * CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E -
 * 12 bits of 'TAG_1_VID'
 * CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_SRC_ID_E -
 * 12 bits of 'SRC_ID'
 * CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E -
 * 10 bits of UDB (bits 1..10) .NOTE: bit 0 is never muxed and used for 'multi-core FDB aging'
 * 1 bit of 'SRC_ID' (bit 0)
 * 1 bit of 'DA_ACCESS_LEVEL'
 * CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_UDB_4_AND_4_SRC_ID_3_DA_ACCESS_LEVEL_E -
 * 4 bits of UDB (bits 1..4) .NOTE: bit 0 is never muxed and used for 'multi-core FDB aging'
 * and 4 bits of UDB (bits 7..10)
 * 3 bits of 'SRC_ID' (bit 0..2)
 * 1 bit of 'DA_ACCESS_LEVEL'
 * CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E - not to be used
*/
typedef enum{

    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E,

    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_SRC_ID_E,

    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E,

    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_UDB_4_AND_4_SRC_ID_3_DA_ACCESS_LEVEL_E,

    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E

} CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT;

/**
* @enum CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT
 *
 * @brief An enum to state IPMC (IPv4/Ipv6 MC) entry muxing modes.
 * how the 7 bits that are muxed in the FDB IPMC (IPv4/Ipv6 MC) entry.
 * APPLICABLE DEVICES: Falcon.
 * Values:
 * CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_SRC_ID_7_E -
 * 7 bits of 'SRC_ID'
 * CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E -
 * 4 bits of UDB (bits 1..4)  .NOTE: bit 0 is never muxed and used for 'multi-core FDB aging'
 * 3 bit of 'SRC_ID' (bits 0..2)
 * CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_UDB_7_E -
 * 7 bits of UDB (bits 1..7) .NOTE: bit 0 is never muxed and used for 'multi-core FDB aging'
 * CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E - not to be used
*/
typedef enum{

    CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_SRC_ID_7_E,

    CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E,

    CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_UDB_7_E,

    CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE__MUST_BE_LAST__E

} CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT;

/**
* @enum CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT
 *
 * @brief An enum to state SA lookup mode in global level.
 * APPLICABLE DEVICES: Falcon.
*/
typedef enum{

    /** @brief Skip FDB SA lookup in next device if autolearning is disabled in this device**/
    CPSS_DXCH_FDB_SA_AUTO_LEARNING_MODE_E,

    /** @brief Skip FDB SA lookup in next device if there is SA lookup match in this device
     * (Moved address is considered as mismatch)**/
    CPSS_DXCH_FDB_SA_MATCHING_MODE_E,
} CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT;


/**
* @internal cpssDxChBrgFdbNaToCpuPerPortSet function
* @endinternal
*
* @brief   Enable/disable forwarding a new mac address message to CPU --per port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
* @param[in] enable                   - If GT_TRUE, forward NA message to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note function also called from cascade management
*
*/
GT_STATUS cpssDxChBrgFdbNaToCpuPerPortSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgFdbNaToCpuPerPortGet function
* @endinternal
*
* @brief   Get Enable/disable forwarding a new mac address message to CPU --
*         per port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] enablePtr                - If GT_TRUE, NA message is forwarded to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note function also called from cascade management
*
*/
GT_STATUS cpssDxChBrgFdbNaToCpuPerPortGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);


/**
* @internal cpssDxChBrgFdbPortLearnStatusSet function
* @endinternal
*
* @brief   Enable/disable learning of new source MAC addresses for packets received
*         on specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
* @param[in] status                   - GT_TRUE for enable  or GT_FALSE otherwise
* @param[in] cmd                      - how to forward packets with unknown/changed SA,
*                                      if status is GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on unknown command
* @retval GT_BAD_STATE             - the setting not allowed in current state of
*                                       configuration.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       1. Unified FDBs mode and Unified-Linked FDBs mode :
*       Automatic learning is not recommended in this mode, due to the fact
*       that FDBs are supposed to be synced.
*       2. Linked FDBs mode - no limitations
*
*/
GT_STATUS cpssDxChBrgFdbPortLearnStatusSet
(
    IN GT_U8                    devNum,
    IN GT_PORT_NUM              portNum,
    IN GT_BOOL                  status,
    IN CPSS_PORT_LOCK_CMD_ENT   cmd
);

/**
* @internal cpssDxChBrgFdbPortLearnStatusGet function
* @endinternal
*
* @brief   Get state of new source MAC addresses learning on packets received
*         on specified port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] statusPtr                - (pointer to) GT_TRUE for enable  or GT_FALSE otherwise
* @param[out] cmdPtr                   - (pointer to) how to forward packets with unknown/changed SA,
*                                      when (statusPtr) is GT_FALSE
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbPortLearnStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_NUM             portNum,
    OUT GT_BOOL                 *statusPtr,
    OUT CPSS_PORT_LOCK_CMD_ENT  *cmdPtr
);

/**
* @internal cpssDxChBrgFdbNaStormPreventSet function
* @endinternal
*
* @brief   Enable/Disable New Address messages Storm Prevention.
*         Controlled learning relies on receiving NA messages for new source MAC
*         address. To prevent forwarding multiple NA messages to the CPU for the
*         same source MAC address, when NA Storm Prevention (SP) is enabled, the
*         device auto-learns this address with a special Storm Prevention flag set in
*         the entry. Subsequent packets from this source address do not generate
*         further NA messages to the CPU. Packets destined to this MAC address however
*         are treated as unknown packets. Upon receiving the single NA
*         message, the CPU can then overwrite the SP entry with a normal FDB
*         forwarding entry.
*         Only relevant in controlled address learning mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE -  NA Storm Prevention (SP),
*                                      GT_FALSE - disable NA Storm Prevention
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/portNum/auMsgType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       Enabling this mechanism in any of the ports, creates a situation where the
*       different portGroups populate different MACs at the same indexes,
*       causing FDBs to become unsynchronized, this leads to the situation that once
*       sending MAC update to different portGroups from CPU, it may succeed on some
*       of the portGroups, while failing on the others (due to SP entries already
*       occupying indexes)... So the only way to bypass these problems, assuming SP
*       is a must, is to add new MACs by exact index - this mode of operation
*       overwrites any SP entry currently there.
*       Application that enables the SP in any of the ports should not use the NA
*       sending mechanism (cpssDxChBrgFdbMacEntrySet / cpssDxChBrgFdbPortGroupMacEntrySet APIs)
*       to add new MAC addresses, especially to multiple portGroups -
*       as this may fail on some of the portGroups.
*       Removing or updating existing (non-SP) entries may still use the NA messages.
*
*/
GT_STATUS cpssDxChBrgFdbNaStormPreventSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgFdbNaStormPreventGet function
* @endinternal
*
* @brief   Get status of New Address messages Storm Prevention.
*         Controlled learning relies on receiving NA messages for new source MAC
*         address. To prevent forwarding multiple NA messages to the CPU for the
*         same source MAC address, when NA Storm Prevention (SP) is enabled, the
*         device auto-learns this address with a special Storm Prevention flag set in
*         the entry. Subsequent packets from this source address do not generate
*         further NA messages to the CPU. Packets destined to this MAC address however
*         are treated as unknown packets. Upon receiving the single NA
*         message, the CPU can then overwrite the SP entry with a normal FDB
*         forwarding entry.
*         Only relevant in controlled address learning mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - pointer to the status of repeated NA CPU messages
*                                      GT_TRUE - NA Storm Prevention (SP) is enabled,
*                                      GT_FALSE - NA Storm Prevention is disabled
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/portNum/auMsgType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbNaStormPreventGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgFdbPortVid1LearningEnableSet function
* @endinternal
*
* @brief   Enable/Disable copying Tag1 VID from packet to FDB entry
*         at automatic learning.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE -  copying Tag1 VID from packet to FDB entry,
*                                      GT_FALSE - disable, the Tag1Vid in FDB will be 0
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbPortVid1LearningEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgFdbPortVid1LearningEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status of copying Tag1 VID from packet to FDB entry
*         at automatic learning.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE - enable copying Tag1 VID from packet to FDB entry,
*                                      GT_FALSE - disable, the Tag1Vid in FDB will be 0
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbPortVid1LearningEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
);

/**
* @internal cpssDxChBrgFdbDeviceTableSet function
* @endinternal
*
* @brief   This function sets the device table of the PP.
*         the PP use this configuration in the FDB aging daemon .
*         once the aging daemon encounter an entry with non-exists devNum
*         associated with it , the daemon will DELETE the entry without sending
*         the CPU any notification.
*         a non-exists device determined by it's bit in the "device table"
*         So for proper work of PP the application must set the relevant bits of
*         all devices in the system prior to inserting FDB entries associated with
*         them
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] devTableBmp              - bmp of devices to set (APPLICABLE RANGES: 0..31).
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbDeviceTableSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  devTableBmp
);

/**
* @internal cpssDxChBrgFdbDeviceTableGet function
* @endinternal
*
* @brief   This function gets the device table of the PP.
*         the PP use this configuration in the FDB aging daemon .
*         once the aging daemon encounter an entry with non-exists devNum
*         associated with it , the daemon will DELETE the entry without sending
*         the CPU any notification.
*         a non-exists device determined by it's bit in the "device table"
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] devTableBmpPtr           - pointer to bmp of devices to set (APPLICABLE RANGES: 0..31).
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbDeviceTableGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *devTableBmpPtr
);

/**
* @internal cpssDxChBrgFdbMacEntrySet function
* @endinternal
*
* @brief   Create new or update existing entry in Hardware MAC address table through
*         Address Update message.(AU message to the PP is non direct access to MAC
*         address table).
*         The function use New Address message (NA) format.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that AU message was processed by PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] macEntryPtr              - pointer mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - bad device number.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the AU message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*       If the entry type is IPv6_MC, then the Application should set the four
*       bytes for SIP and DIP that selected by cpssDxChBrgMcIpv6BytesSelectSet.
*
*/
GT_STATUS cpssDxChBrgFdbMacEntrySet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
);

/**
* @internal cpssDxChBrgFdbQaSend function
* @endinternal
*
* @brief   The function Send Query Address (QA) message to the hardware MAC address
*         table.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that QA message was processed by PP.
*         The PP sends Query Response message after QA processing.
*         An application can get QR message by general AU message get API.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] macEntryKeyPtr           - pointer to mac entry key
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/vlan
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the QA message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*
*/
GT_STATUS cpssDxChBrgFdbQaSend
(
    IN  GT_U8                       devNum,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
);


/**
* @internal cpssDxChBrgFdbMacEntryDelete function
* @endinternal
*
* @brief   Delete an old entry in Hardware MAC address table through Address Update
*         message.(AU message to the PP is non direct access to MAC address table).
*         The function use New Address message (NA) format with skip bit set to 1.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that AU message was processed by PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry
*                                      according to the entry type:
*                                      MAC Address entry -> MAC Address + vlan ID.
*                                      IPv4/IPv6 entry -> srcIP + dstIP + vald ID.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the AU message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*
*/
GT_STATUS cpssDxChBrgFdbMacEntryDelete
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
);



/**
* @internal cpssDxChBrgFdbMacEntryWrite function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in specified index.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - hw mac entry index
* @param[in] skip                     - entry  control
*                                      GT_TRUE - used to "skip" the entry ,
*                                      the HW will treat this entry as "not used"
*                                      GT_FALSE - used for valid/used entries.
* @param[in] macEntryPtr              - pointer to MAC entry parameters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,saCommand,daCommand
* @retval GT_OUT_OF_RANGE          - vidx/trunkId/portNum-devNum with values bigger then HW
*                                       support
*                                       index out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       index,
    IN GT_BOOL                      skip,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr

);

/**
* @internal cpssDxChBrgFdbMacEntryStatusGet function
* @endinternal
*
* @brief   Get the Valid and Skip Values of a FDB entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - hw mac entry index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index is out of range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacEntryStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr
);


/**
* @internal cpssDxChBrgFdbMacEntryRead function
* @endinternal
*
* @brief   Reads the new entry in Hardware MAC address table from specified index.
*         This action do direct read access to RAM .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - hw mac entry index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
* @param[out] agedPtr                  - (pointer to) is entry aged
* @param[out] associatedHwDevNumPtr    = (pointer to) is HW device number associated with the
*                                      entry (even for vidx/trunk entries the field is used by
*                                      PP for aging/flush/transplant purpose).
*                                      Relevant only in case of Mac Address entry.
* @param[out] entryPtr                 - (pointer to) extended Mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index out of range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacEntryRead
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr,
    OUT GT_BOOL                 *agedPtr,
    OUT GT_HW_DEV_NUM           *associatedHwDevNumPtr,
    OUT CPSS_MAC_ENTRY_EXT_STC  *entryPtr
);

/**
* @internal cpssDxChBrgFdbMacEntryInvalidate function
* @endinternal
*
* @brief   Invalidate an entry in Hardware MAC address table in specified index.
*         the invalidation done by resetting to first word of the entry
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - hw mac entry index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - index out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacEntryInvalidate
(
    IN GT_U8         devNum,
    IN GT_U32        index
);


/**
* @internal cpssDxChBrgFdbMaxLookupLenSet function
* @endinternal
*
* @brief   Set the the number of entries to be looked up in the MAC table lookup
*         (the hash chain length), for all devices in unit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] lookupLen                - The maximal length of MAC table lookup, this must be
*                                      value divided by 4 with no left over.
*                                      (APPLICABLE RANGES: 4, 8, 12..32).
*                                      in 'multi hash' mode the valid value is : 16.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - If the given lookupLen is too large, or not divided
*                                       by 4 with no left over.
* @retval GT_OUT_OF_RANGE          - lookupLen > 32 or lookupLen < 4
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
 GT_STATUS cpssDxChBrgFdbMaxLookupLenSet
(
    IN GT_U8    devNum,
    IN GT_U32   lookupLen
);

/**
* @internal cpssDxChBrgFdbMaxLookupLenGet function
* @endinternal
*
* @brief   Get the the number of entries to be looked up in the MAC table lookup
*         (the hash chain length), for all devices in unit.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] lookupLenPtr             - The maximal length of MAC table lookup, this must be
*                                      value divided by 4 with no left over.
*                                      (APPLICABLE RANGES: 4, 8, 12..32).
*                                      in 'multi hash' mode the valid value is : 16.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - If the given lookupLen is too large, or not divided
*                                       by 4 with no left over.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMaxLookupLenGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *lookupLenPtr
);

/**
* @internal cpssDxChBrgFdbMacVlanLookupModeSet function
* @endinternal
*
* @brief   Sets the VLAN Lookup mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - lookup mode:
*                                      CPSS_IVL_E - Both MAC and Vlan are used for MAC table lookup
*                                      and learning.
*                                      CPSS_SVL_E - Only MAC is used for MAC table lookup and learning.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacVlanLookupModeSet
(
    IN GT_U8            devNum,
    IN CPSS_MAC_VL_ENT  mode
);

/**
* @internal cpssDxChBrgFdbMacVlanLookupModeGet function
* @endinternal
*
* @brief   Get VLAN Lookup mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to lookup mode:
*                                      CPSS_IVL_E - Both MAC and Vlan are used for MAC table lookup
*                                      and learning.
*                                      CPSS_SVL_E - Only MAC is used for MAC table lookup and learning.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacVlanLookupModeGet
(
    IN  GT_U8            devNum,
    OUT CPSS_MAC_VL_ENT  *modePtr
);

/**
* @internal cpssDxChBrgFdbAuMsgRateLimitSet function
* @endinternal
*
* @brief   Set Address Update CPU messages rate limit
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] msgRate                  - maximal rate of AU messages per second
* @param[in] enable                   - enable/disable message limit
*                                      GT_TRUE - message rate is limited with msgRate
*                                      GT_FALSE - there is no limitations, WA to CPU
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_OUT_OF_RANGE          - out of range.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note granularity - 200 messages,
*       rate limit - 0 - 51000 messages,
*       when DxCh Clock 200 MHz(DxCh2 220 MHz Clock) ,
*       granularity and rate changed linearly depending on clock.
*       for multi-port groups device :
*       The rate-limiting is done individually in each one of the port Groups.
*       This means that potentially configuring this value would result a rate
*       which is up to n times bigger then expected (n is number of port groups)
*
*/
GT_STATUS cpssDxChBrgFdbAuMsgRateLimitSet
(
    IN GT_U8                       devNum,
    IN GT_U32                      msgRate,
    IN GT_BOOL                     enable
);

/**
* @internal cpssDxChBrgFdbAuMsgRateLimitGet function
* @endinternal
*
* @brief   Get Address Update CPU messages rate limit
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] msgRatePtr               - (pointer to) maximal rate of AU messages per second
* @param[out] enablePtr                - (pointer to) GT_TRUE - message rate is limited with
* @param[out] msgRatePtr
*                                      GT_FALSE - there is no limitations, WA to CPU
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on error.
* @retval GT_OUT_OF_RANGE          - out of range.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note granularity - 200 messages,
*       rate limit - 0 - 51000 messages,
*       when DxCh Clock 200 MHz(DxCh2 220 MHz Clock) ,
*       granularity and rate changed linearly depending on clock.
*
*/
GT_STATUS cpssDxChBrgFdbAuMsgRateLimitGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *msgRatePtr,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChBrgFdbNaMsgOnChainTooLongSet function
* @endinternal
*
* @brief   Enable/Disable sending NA messages to the CPU indicating that the device
*         cannot learn a new SA . It has reached its max hop (bucket is full).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  sending the message to the CPU
*                                      GT_FALSE for disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbNaMsgOnChainTooLongSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgFdbNaMsgOnChainTooLongGet function
* @endinternal
*
* @brief   Get the status (enabled/disabled) of sending NA messages to the CPU
*         indicating that the device cannot learn a new SA. It has reached its
*         max hop (bucket is full).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - pointer to status of sending the NA message to the CPU
*                                      GT_TRUE - enabled
*                                      GT_FALSE - disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbNaMsgOnChainTooLongGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChBrgFdbSpAaMsgToCpuSet function
* @endinternal
*
* @brief   Enabled/Disabled sending Aged Address (AA) messages to the CPU for the
*         Storm Prevention (SP) entries when those entries reach an aged out status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  sending the SP AA message to the CPU
*                                      GT_FALSE for disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       Whenever applicable it's highly recommended disabling these messages,
*       as enabling this would stress the AUQ with excessive messages
*
*/
GT_STATUS cpssDxChBrgFdbSpAaMsgToCpuSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgFdbSpAaMsgToCpuGet function
* @endinternal
*
* @brief   Get the status (enabled/disabled) of sending Aged Address (AA) messages
*         to the CPU for the Storm Prevention (SP) entries when those entries reach
*         an aged out status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - pointer to status of sending the SP AA message to the CPU
*                                      GT_TRUE - enabled
*                                      GT_FALSE - disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbSpAaMsgToCpuGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChBrgFdbAAandTAToCpuSet function
* @endinternal
*
* @brief   Enable/Disable the PP to/from sending an AA and TA address
*         update messages to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  or disable the message
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or auMsgType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       Whenever applicable it's highly recommended disabling these messages,
*       as enabling this would stress the AUQ with excessive messages
*
*/
GT_STATUS cpssDxChBrgFdbAAandTAToCpuSet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      enable
);

/**
* @internal cpssDxChBrgFdbAAandTAToCpuGet function
* @endinternal
*
* @brief   Get state of sending an AA and TA address update messages to the CPU
*         as configured to PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - the messages to CPU enabled
*                                      GT_FALSE - the messages to CPU disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or auMsgType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbAAandTAToCpuGet
(
    IN GT_U8                        devNum,
    OUT GT_BOOL                     *enablePtr
);

/**
* @internal cpssDxChBrgFdbHashModeSet function
* @endinternal
*
* @brief   Sets the FDB hash function mode.
*         The CRC based hash function provides the best hash index distribution
*         for random addresses and vlans.
*         The XOR based hash function provides optimal hash index distribution
*         for controlled testing scenarios, where sequential addresses and vlans
*         are often used.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - hash function based mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbHashModeSet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_HASH_FUNC_MODE_ENT  mode
);

/**
* @internal cpssDxChBrgFdbHashModeGet function
* @endinternal
*
* @brief   Gets the FDB hash function mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - hash function based mode:
*                                      CPSS_FDB_HASH_FUNC_XOR_E - XOR based hash function mode
*                                      CPSS_FDB_HASH_FUNC_CRC_E - CRC based hash function mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbHashModeGet
(
    IN  GT_U8                         devNum,
    OUT CPSS_MAC_HASH_FUNC_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChBrgFdbAgingTimeoutSet function
* @endinternal
*
* @brief   Sets the timeout period in seconds for aging out dynamically learned
*         forwarding information. The standard recommends 300 sec.
*         NOTE :
*         For Bobcat2 the device support aging time with granularity of 1 second
*         in range 1.. max. where max depend on FDB size and 'core clock'.
*         (the max value is very high and at least several thousands seconds !)
*         For DxCh3 the device support aging time with granularity of 16 seconds
*         (16,32..1008) . So value of 226 will be round down to 224 , and value of
*         255 will be round up to 256.
*         For others the device support aging time with
*         granularity of 10 seconds (10,20..630) .
*         So value of 234 will be round down to 230 ,
*         and value of 255 will be round up to 260.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] timeout                  - aging time in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For the 270MHz range from 16..1008 (but due to rounding we support values 8..636)
*       HW supports steps 16
*       For the 220MHz range from 9..573 (but due to rounding we support values 5..577)
*       HW supports steps 9 (9.09)
*       For the 200MHz range from 10..630 (but due to rounding we support values 5..634)
*       HW supports steps 10
*       For the 144 MHz range from 14..875 (but due to rounding we support values 7..882)
*       HW supports steps 13.88
*       For the 167 MHz range from 14..886 (but due to rounding we support values 7..893)
*       HW supports steps 14 (14.07)
*       For the 360MHz range from 10..630 (but due to rounding we support values 5..634)
*       HW supports steps 10
*
*/
GT_STATUS cpssDxChBrgFdbAgingTimeoutSet
(
    IN GT_U8  devNum,
    IN GT_U32 timeout
);

/**
* @internal cpssDxChBrgFdbAgingTimeoutGet function
* @endinternal
*
* @brief   Gets the timeout period in seconds for aging out dynamically learned
*         forwarding information. The standard recommends 300 sec.
*         NOTE :
*         For Bobcat2 the device support aging time with granularity of 1 second
*         in range 1.. max. where max depend on FDB size and 'core clock'.
*         (the max value is very high and at least several thousands seconds !)
*         For DxCh3 the device support aging time with granularity of 16 seconds
*         (16,32..1008).
*         For others the device support aging time with
*         granularity of 10 seconds (10,20..630) .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] timeoutPtr               - pointer to aging time in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For core clock of 270MHz The value ranges from 16 seconds to 1008
*       seconds in steps of 16 seconds.
*       For core clock of 220MHz The value ranges from 9 seconds to 573
*       seconds in steps of 9 seconds.
*       For core clock of 200 MHZ The value ranges from 10 seconds to 630
*       seconds in steps of 10 seconds.
*       For core clock of 144MHZ the aging timeout ranges from 14 (13.8) to 875
*       seconds in steps of 14 seconds.
*
*/
GT_STATUS cpssDxChBrgFdbAgingTimeoutGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *timeoutPtr
);

/**
* @internal cpssDxChBrgFdbSecureAgingSet function
* @endinternal
*
* @brief   Enable or disable secure aging. This mode is relevant to automatic or
*         triggered aging with removal of aged out FDB entries. This mode is
*         applicable together with secure auto learning mode see
*         cpssDxChBrgFdbSecureAutoLearnSet. In the secure aging mode the device
*         sets aged out unicast FDB entry with <Multiple> = 1 and VIDX = 0xfff but
*         not invalidates entry. This causes packets destined to this FDB entry MAC
*         address to be flooded to the VLAN. But such packets are known ones.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  secure aging
*                                      GT_FALSE - disable secure aging
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_STATE             - the setting not allowed in current state of
*                                       configuration.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       1. Unified FDBs mode and Unified-Linked FDBs mode :
*       Since Automatic learning is not recommended in this mode nor secured automatic
*       learning .
*       2. Linked FDBs mode - no limitations
*
*/
GT_STATUS cpssDxChBrgFdbSecureAgingSet
(
    IN GT_U8          devNum,
    IN GT_BOOL        enable
);

/**
* @internal cpssDxChBrgFdbSecureAgingGet function
* @endinternal
*
* @brief   Get the status (enabled/disabled) of secure aging.
*         This mode is relevant to automatic or triggered aging with removal of
*         aged out FDB entries. This mode is applicable together with secure auto
*         learning mode see cpssDxChBrgFdbSecureAutoLearnGet. In the secure aging
*         mode the device sets aged out unicast FDB entry with <Multiple> = 1 and
*         VIDX = 0xfff but not invalidates entry. This causes packets destined to
*         this FDB entry MAC address to be flooded to the VLAN.
*         But such packets are known ones.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE - enable secure aging
*                                      GT_FALSE - disable secure aging
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbSecureAgingGet
(
    IN  GT_U8          devNum,
    OUT GT_BOOL        *enablePtr
);

/**
* @internal cpssDxChBrgFdbSecureAutoLearnSet function
* @endinternal
*
* @brief   Configure secure auto learning mode.
*         If Secure Automatic Learning is enabled, and a packet enabled for
*         automatic learning, then:
*         - If the source MAC address is found in the FDB, and the associated
*         location of the address has changed, then the FDB entry is updated
*         with the new location of the address.
*         - If the FDB entry is modified with the new location of the address,
*         a New Address Update message is sent to the CPU,
*         if enabled to do so on the port and the packet VLAN.
*         - If the source MAC address is NOT found in the FDB, then
*         Secure Automatic Learning Unknown Source command (Trap to CPU,
*         Soft Drop or Hard Drop) is assigned to the packet according to mode
*         input parameter and packet treated as security breach event.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - Enable/Disable secure automatic learning.
*                                      The command assigned to packet with new SA not found in FDB
*                                      when secure automatic learning enable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbSecureAutoLearnSet
(
    IN GT_U8                                devNum,
    IN CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT  mode
);

/**
* @internal cpssDxChBrgFdbSecureAutoLearnGet function
* @endinternal
*
* @brief   Get secure auto learning mode.
*         If Secure Automatic Learning is enabled, and a packet enabled for
*         automatic learning, then:
*         - If the source MAC address is found in the FDB, and the associated
*         location of the address has changed, then the FDB entry is updated
*         with the new location of the address.
*         - If the FDB entry is modified with the new location of the address,
*         a New Address Update message is sent to the CPU,
*         if enabled to do so on the port and the packet VLAN.
*         - If the source MAC address is NOT found in the FDB, then
*         Secure Automatic Learning Unknown Source command (Trap to CPU,
*         Soft Drop or Hard Drop) is assigned to the packet according to mode
*         input parameter and packet treated as security breach event.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - Enabled/Disabled secure automatic learning.
*                                      The command assigned to packet with new SA not found in FDB
*                                      when secure automatic learning enabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbSecureAutoLearnGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT  *modePtr
);

/**
* @internal cpssDxChBrgFdbStaticTransEnable function
* @endinternal
*
* @brief   This routine determines whether the transplanting operate on static
*         entries.
*         When the PP will do the transplanting , it will/won't transplant
*         FDB static entries (entries that are not subject to aging).
*         When the PP will do the "port flush" (delete FDB entries associated
*         with to specific port), it will/won't Flush the FDB static entries
*         (entries that are not subject to aging).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE transplanting is enabled on static entries
*                                      GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbStaticTransEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgFdbStaticTransEnableGet function
* @endinternal
*
* @brief   Get whether the transplanting enabled to operate on static entries.
*         when the PP do the transplanting , it do/don't transplant
*         FDB static entries (entries that are not subject to aging).
*         When the PP will do the "port flush" (delete FDB entries associated
*         with to specific port), it do/don't Flush the FDB static entries
*         (entries that are not subject to aging).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE transplanting is enabled on static entries,
*                                      GT_FALSE otherwise.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbStaticTransEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChBrgFdbStaticDelEnable function
* @endinternal
*
* @brief   This routine determines whether flush delete operates on static entries.
*         When the PP will do the Flush , it will/won't Flush
*         FDB static entries (entries that are not subject to aging).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - FDB deletion mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbStaticDelEnable
(
    IN GT_U8                           devNum,
    IN CPSS_DXCH_BRG_FDB_DEL_MODE_ENT  mode
);

/**
* @internal cpssDxChBrgFdbStaticDelEnableGet function
* @endinternal
*
* @brief   Get whether flush delete operates on static entries.
*         When the PP do the Flush , it do/don't Flush
*         FDB static entries (entries that are not subject to aging).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - FDB deletion mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbStaticDelEnableGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_BRG_FDB_DEL_MODE_ENT  *modePtr
);

/**
* @internal cpssDxChBrgFdbActionsEnableSet function
* @endinternal
*
* @brief   Enables/Disables FDB actions.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  Actions are enabled
*                                      GT_FALSE -  Actions are disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbActionsEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgFdbActionsEnableGet function
* @endinternal
*
* @brief   Get the status of FDB actions.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE - actions are enabled
*                                      GT_FALSE - actions are disabled
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbActionsEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChBrgFdbTrigActionStatusGet function
* @endinternal
*
* @brief   Get the action status of the FDB action registers.
*         When the status is changing from GT_TRUE to GT_FALSE, then the device
*         performs the action according to the setting of action registers.
*         When the status returns to be GT_TRUE then the action is completed.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] actFinishedPtr           - GT_TRUE  - triggered action completed
*                                      GT_FALSE - triggered action is not completed yet
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       'action finished' only when action finished on all port groups.
*
*/
GT_STATUS cpssDxChBrgFdbTrigActionStatusGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *actFinishedPtr
);

/**
* @internal cpssDxChBrgFdbMacTriggerModeSet function
* @endinternal
*
* @brief   Sets Mac address table Triggered\Automatic action mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - action mode:
*                                      CPSS_ACT_AUTO_E - Action is done Automatically.
*                                      CPSS_ACT_TRIG_E - Action is done via a trigger from CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacTriggerModeSet
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ACTION_MODE_ENT     mode
);

/**
* @internal cpssDxChBrgFdbMacTriggerModeGet function
* @endinternal
*
* @brief   Gets Mac address table Triggered\Automatic action mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - pointer to action mode:
*                                      CPSS_ACT_AUTO_E - Action is done Automatically.
*                                      CPSS_ACT_TRIG_E - Action is done via a trigger from CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacTriggerModeGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_MAC_ACTION_MODE_ENT     *modePtr
);

/**
* @internal cpssDxChBrgFdbStaticOfNonExistDevRemove function
* @endinternal
*
* @brief   enable the ability to delete static mac entries when the aging daemon in
*         PP encounter with entry registered on non-exist device in the
*         "device table" the PP removes it ---> this flag regard the static
*         entries
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] deleteStatic             -  GT_TRUE - Action will delete static entries as well as
*                                      dynamic
*                                      GT_FALSE - only dynamic entries will be deleted.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It's application responsibility to check the status of Aging Trigger by
*       busy wait (use cpssDxChBrgFdbTrigActionStatusGet API), in order to be
*       sure that that there is currently no action done.
*
*/
GT_STATUS cpssDxChBrgFdbStaticOfNonExistDevRemove
(
    IN GT_U8                       devNum,
    IN GT_BOOL                     deleteStatic
);

/**
* @internal cpssDxChBrgFdbStaticOfNonExistDevRemoveGet function
* @endinternal
*
* @brief   Get whether enabled/disabled the ability to delete static mac entries
*         when the aging daemon in PP encounter with entry registered on non-exist
*         device in the "device table".
*         entries
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] deleteStaticPtr          -  GT_TRUE - Action deletes entries if there associated
*                                      device number don't exist (static etries
*                                      as well)
*                                      GT_FALSE - Action don't delete entries if there
*                                      associated device number don't exist
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbStaticOfNonExistDevRemoveGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *deleteStaticPtr
);

/**
* @internal cpssDxChBrgFdbDropAuEnableSet function
* @endinternal
*
* @brief   Enable/Disable dropping the Address Update messages when the queue is
*         full .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - Drop the address update messages
*                                      GT_FALSE - Do not drop the address update messages
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbDropAuEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgFdbDropAuEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable of dropping the Address Update messages.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE - The address update messages are dropped
*                                      GT_FALSE - The address update messages are not dropped
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbDropAuEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable function
* @endinternal
*
* @brief   Enable/Disable aging out of all entries associated with trunk,
*         regardless of the device to which they are associated.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ageOutAllDevOnTrunkEnable - GT_TRUE : All entries associated with a
*                                      trunk, regardless of the device they
*                                      are associated with are aged out.
*                                      GT_FALSE: Only entries associated with a
*                                      trunk on the local device are aged out.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  ageOutAllDevOnTrunkEnable
);

/**
* @internal cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet function
* @endinternal
*
* @brief   Get if aging out of all entries associated with trunk, regardless of
*         the device to which they are associated is enabled/disabled.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] ageOutAllDevOnTrunkEnablePtr - GT_TRUE : All entries associated with a
*                                      trunk, regardless of the device they
*                                      are associated with are aged out.
*                                      GT_FALSE: Only entries associated with a
*                                      trunk on the local device are aged out.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *ageOutAllDevOnTrunkEnablePtr
);

/**
* @internal cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable function
* @endinternal
*
* @brief   Enable/Disable aging out of all entries associated with port,
*         regardless of the device to which they are associated.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] ageOutAllDevOnNonTrunkEnable - GT_TRUE : All entries associated with a
*                                      port, regardless of the device they
*                                      are associated with are aged out.
*                                      GT_FALSE: Only entries associated with a
*                                      port on the local device are aged out.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable
(
    IN GT_U8    devNum,
    IN GT_BOOL  ageOutAllDevOnNonTrunkEnable
);

/**
* @internal cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet function
* @endinternal
*
* @brief   Get if aging out of all entries associated with port, regardless of
*         the device to which they are associated is enabled/disabled.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] ageOutAllDevOnNonTrunkEnablePtr - GT_TRUE : All entries associated with a
*                                      port, regardless of the device they
*                                      are associated with are aged out.
*                                      GT_FALSE: Only entries associated with a
*                                      port on the local device are aged out.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *ageOutAllDevOnNonTrunkEnablePtr
);

/**
* @internal cpssDxChBrgFdbActionTransplantDataSet function
* @endinternal
*
* @brief   Prepares the entry for transplanting (old and new interface parameters).
*         VLAN and VLAN mask for transplanting is set by
*         cpssDxChBrgFdbActionActiveVlanSet. Execution of transplanting is done
*         by cpssDxChBrgFdbTrigActionStart.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] oldInterfacePtr          - pointer to old Interface parameters
*                                      (it may be only port or trunk).
* @param[in] newInterfacePtr          - pointer to new Interface parameters
*                                      (it may be only port or trunk).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is no check done of current status of action's register
*
*/
GT_STATUS cpssDxChBrgFdbActionTransplantDataSet
(
    IN GT_U8                    devNum,
    IN CPSS_INTERFACE_INFO_STC  *oldInterfacePtr,
    IN CPSS_INTERFACE_INFO_STC  *newInterfacePtr
);

/**
* @internal cpssDxChBrgFdbActionTransplantDataGet function
* @endinternal
*
* @brief   Get transplant data: old interface parameters and the new ones.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] oldInterfacePtr          - pointer to old Interface parameters.
*                                      (it may be only port or trunk).
* @param[out] newInterfacePtr          - pointer to new Interface parameters.
*                                      (it may be only port or trunk).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbActionTransplantDataGet
(
    IN  GT_U8                    devNum,
    OUT CPSS_INTERFACE_INFO_STC  *oldInterfacePtr,
    OUT CPSS_INTERFACE_INFO_STC  *newInterfacePtr
);

/**
* @internal cpssDxChBrgFdbFromCpuAuMsgStatusGet function
* @endinternal
*
* @brief   Get status of FDB Address Update (AU) message processing in the PP.
*         The function returns AU message processing completion and success status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in,out] entryOffsetPtr           - (pointer to) to receive the offset from the hash calculated index
*                                      NULL value means that caller not need to retrieve this value.
*
* @param[out] completedPtr             - pointer to AU message processing completion.
*                                      GT_TRUE - AU message was processed by PP.
*                                      GT_FALSE - AU message processing is not completed yet by PP.
* @param[out] succeededPtr             - pointer to a success of AU operation.
*                                      GT_TRUE  - the AU action succeeded.
*                                      GT_FALSE - the AU action has failed.
* @param[in,out] entryOffsetPtr           - (pointer to) the offset from the hash calculated index
*                                      of the entry that action performed on.
*                                      (similar to parameter in an AU message CPSS_MAC_UPDATE_MSG_EXT_STC::entryOffset)
*                                      The parameter ignored when NULL pointer.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*                                      the offset is relevant only when:
*                                      1. 'completed = GT_TRUE and succeeded = GT_TRUE'
*                                      2. The function that triggered the action is one of :
*                                      a. cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbPortGroupMacEntryDelete
*                                      the offset relate to the delete entry.
*                                      b. cpssDxChBrgFdbMacEntrySet, cpssDxChBrgFdbPortGroupMacEntrySet functions.
*                                      the offset relate to the inserted/updated entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DXCH devices the succeed status doesn't relevant for query
*       address messages (message type = CPSS_QA_E) - FEr#93.
*       The succeed status is relevant only when completed.
*       An AU message sent by CPU can fail in the following cases:
*       1. The message type = CPSS_NA_E and the hash chain has reached it's
*       maximum length.
*       2. The message type = CPSS_QA_E or CPSS_AA_E and the FDB entry doesn't
*       exist.
*
*/
GT_STATUS cpssDxChBrgFdbFromCpuAuMsgStatusGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *completedPtr,
    OUT GT_BOOL  *succeededPtr,
    INOUT GT_U32 *entryOffsetPtr
);


/**
* @internal cpssDxChBrgFdbActionActiveVlanSet function
* @endinternal
*
* @brief   Set action active vlan and vlan mask.
*         All actions will be taken on entries belonging to a certain VLAN
*         or a subset of VLANs.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - Vlan Id
* @param[in] vlanMask                 - vlan mask filter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Use mask 0xFFF and VLAN ID to set mode on a single VLAN
*       Use mask 0 and VLAN 0 to set mode for all VLANs
*
*/
GT_STATUS cpssDxChBrgFdbActionActiveVlanSet
(
    IN GT_U8    devNum,
    IN GT_U16   vlanId,
    IN GT_U16   vlanMask
);

/**
* @internal cpssDxChBrgFdbActionActiveVlanGet function
* @endinternal
*
* @brief   Get action active vlan and vlan mask.
*         All actions are taken on entries belonging to a certain VLAN
*         or a subset of VLANs.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] vlanIdPtr                - pointer to Vlan Id
* @param[out] vlanMaskPtr              - pointer to vlan mask filter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Use mask 0xFFF and VLAN ID to set mode on a single VLAN
*       Use mask 0 and VLAN 0 to set mode for all VLANs
*
*/
GT_STATUS cpssDxChBrgFdbActionActiveVlanGet
(
    IN  GT_U8    devNum,
    OUT GT_U16   *vlanIdPtr,
    OUT GT_U16   *vlanMaskPtr
);

/**
* @internal cpssDxChBrgFdbActionActiveDevSet function
* @endinternal
*
* @brief   Set the Active device number and active device number mask , that
*         control the FDB actions of : Trigger aging , transplant , flush ,
*         flush port.
*         A FDB entry will be treated by the FDB action only if the entry's
*         "associated devNum" masked by the "active device number mask" equal to
*         "Active device number"
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] actDev                   - Action active HW device number (APPLICABLE RANGES: 0..31)
* @param[in] actDevMask               - Action active mask (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*                                       in 'Dual deviceId system' , if the Pattern/mask
*                                       of the LSBit of the actDev,actDevMask are 'exact match = 1'
*                                       because all devices in this system with 'even numbers'
* @retval GT_OUT_OF_RANGE          - actDev > 31 or actDevMask > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is no check done of current status of action's register
*       For 'Dual deviceId system':
*       the LSBit on the actDev,acdDevMask are ignored and actually taken from
*       MSBit of actTrunkPort and actTrunkPortMask from cpssDxChBrgFdbActionActiveInterfaceSet
*
*/
GT_STATUS cpssDxChBrgFdbActionActiveDevSet
(
    IN GT_U8    devNum,
    IN GT_U32   actDev,
    IN GT_U32   actDevMask
);

/**
* @internal cpssDxChBrgFdbActionActiveDevGet function
* @endinternal
*
* @brief   Get the Active device number and active device number mask , that
*         control the FDB actions of : Trigger aging , transplant , flush ,
*         flush port.
*         A FDB entry will be treated by the FDB action only if the entry's
*         "associated devNum" masked by the "active device number mask" equal to
*         "Active device number"
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] actDevPtr                - pointer to action active HW device number (APPLICABLE RANGES: 0..31)
* @param[out] actDevMaskPtr            - pointer to action active mask (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - actDev > 31 or actDevMask > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbActionActiveDevGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *actDevPtr,
    OUT GT_U32   *actDevMaskPtr
);

/**
* @internal cpssDxChBrgFdbActionActiveInterfaceSet function
* @endinternal
*
* @brief   Set the Active trunkId/portNumber active trunk/port mask and
*         trunkID mask that control the FDB actions of : Triggered aging with
*         removal, Triggered aging without removal and Triggered address deleting.
*         A FDB entry will be treated by the FDB action only if the following
*         cases are true:
*         1. The interface is trunk and "active trunk mask" is set and
*         "associated trunkId" masked by the "active trunk/port mask" equal to
*         "Active trunkId".
*         2. The interface is port and "associated portNumber" masked by the
*         "active trunk/port mask" equal to "Active portNumber".
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] actIsTrunk               - determines if the interface is port or trunk
* @param[in] actIsTrunkMask           - action active trunk mask.
* @param[in] actTrunkPort             - action active interface (port/trunk)
* @param[in] actTrunkPortMask         - action active port/trunk mask.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To enable Trunk-ID filter set actIsTrunk and actIsTrunkMask to 0x1,
*       set actTrunkPort to trunkID and actTrunkPortMask to 0x7F. To disable
*       Trunk-ID filter set all those parameters to 0x0.
*       To enable Port/device filter set actIsTrunk to 0x0, actTrunkPort to
*       portNum, actTrunkPortMask to 0x7F, actDev to device number and
*       actDevMask to 0x1F (all ones) by cpssDxChBrgFdbActionActiveDevSet.
*       To disable port/device filter set all those parameters to 0x0.
*       There is no check done of current status of action's register
*       For 'Dual deviceId system':
*       the MSBit of the actTrunkPort and actTrunkPortMask are actually used as
*       LSBit of actDev and actDevMask of cpssDxChBrgFdbActionActiveDevSet
*       - configuration limitation :
*       not support next configuration :
*       ((actIsTrunkMask == 0 ) && //care don't if trunk/port
*       (actTrunkPortMask[MSBit] == 1)) // exact match of MSBit of trunk/port
*       reason for the limitation : trunk can come from 2 hemispheres
*       more detailed behavior:
*       A. If <isTrunk> is exact match = ‘port’
*       Then Cpss do :
*       < hwDevNum >[0] .Pattern = <port/trunk>[6].Pattern
*       < hwDevNum >[0] .Mask = <port/trunk>[6].Mask
*       <port/trunk>[6]. Pattern = 0
*       <port/trunk>[6]. Mask = no change
*       B. Else If <isTrunk> is exact match = ‘trunk’
*       Then Cpss do :
*       < hwDevNum >[0] .Mask = ‘don’t care’
*       C. Else // <isTrunk> is ‘don’t care’
*       1) if <port/trunk>[6] is ‘don’t care’
*       Then Cpss do :
*       < hwDevNum >[0] .Mask = ‘don’t care’
*       2) else //exact match – 0 or 1
*       return bad state //because trunk can come from 2 hemispheres
*
*/
GT_STATUS cpssDxChBrgFdbActionActiveInterfaceSet
(
    IN GT_U8   devNum,
    IN GT_U32  actIsTrunk,
    IN GT_U32  actIsTrunkMask,
    IN GT_U32  actTrunkPort,
    IN GT_U32  actTrunkPortMask
);

/**
* @internal cpssDxChBrgFdbActionActiveInterfaceGet function
* @endinternal
*
* @brief   Get the Active trunkId/portNumber active trunk/port mask and
*         trunkID mask that control the FDB actions of: Triggered aging with
*         removal, Triggered aging without removal and Triggered address deleting.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] actIsTrunkPtr            - determines if the interface is port or trunk
* @param[out] actIsTrunkMaskPtr        - action active trunk mask.
* @param[out] actTrunkPortPtr          - action active interface (port/trunk)
* @param[out] actTrunkPortMaskPtr      - action active port/trunk mask.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbActionActiveInterfaceGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *actIsTrunkPtr,
    OUT GT_U32  *actIsTrunkMaskPtr,
    OUT GT_U32  *actTrunkPortPtr,
    OUT GT_U32  *actTrunkPortMaskPtr
);

/**
* @internal cpssDxChBrgFdbIpmcAddrDelEnableSet function
* @endinternal
*
* @brief   Function enable/disable the address aging/deleting IPMC FDB entries.
*         when disabled : the IPMC FDB are not subject to aging/delete operations
*         by the aging daemon.
*         when enabled : the IPMC FDB are subject to aging/delete operations
*         by the aging daemon.
*         aging is still subject to only 'non-static' entries.
*         NOTE: no other filter (vlan/dev...) is relevant for those entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable the address aging/deleting IPMC FDB entries.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbIpmcAddrDelEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgFdbIpmcAddrDelEnableGet function
* @endinternal
*
* @brief   Function gets enabled/disabled of the address aging/deleting IPMC FDB entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable/disable the address aging/deleting IPMC FDB entries.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbIpmcAddrDelEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChBrgFdbUploadEnableSet function
* @endinternal
*
* @brief   Enable/Disable reading FDB entries via AU messages to the CPU.
*         The API only configures mode of triggered action.
*         To execute upload use the cpssDxChBrgFdbTrigActionStart.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  FDB Upload
*                                      GT_FALSE - disable FDB upload
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is no check done of current status of action's register
*
*/
GT_STATUS cpssDxChBrgFdbUploadEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChBrgFdbUploadEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable of uploading FDB entries via AU messages to the CPU.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE - FDB Upload is enabled
*                                      GT_FALSE - FDB upload is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbUploadEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChBrgFdbTrigActionStart function
* @endinternal
*
* @brief   Enable actions, sets action type, action mode to CPSS_ACT_TRIG_E and
*         starts triggered action by setting Aging Trigger.
*         This API may be used to start one of triggered actions: Aging, Deleting,
*         Transplanting and FDB Upload.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - action mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous FDB triggered action is not completed yet
*                                       or CNC block upload not finished (or not all of it's
*                                       results retrieved from the common used FU and CNC
*                                       upload queue)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Deleting and Transplanting actions can be done only by CPU triggered
*       FDB action that activated by the function.
*       See below sequence of CPSS API calls for Deleting and
*       Transplanting actions.
*       Before making Delete or Transplant sequence the application need to
*       disable Actions to avoid wrong automatic aging.
*       It is Application responsibility to get and store all parameters
*       that are changed during those actions.
*       1. AA message to CPU status by cpssDxChBrgFdbAAandTAToCpuGet.
*       2. Action Active configuration by
*       cpssDxChBrgFdbActionActiveInterfaceGet,
*       cpssDxChBrgFdbActionActiveDevGet,
*       cpssDxChBrgFdbActionActiveVlanGet.
*       3. Action mode and trigger mode by cpssDxChBrgFdbActionModeGet
*       cpssDxChBrgFdbActionTriggerModeGet.
*       The AA and TA messages may be disabled before the FDB action
*       4. Disable AA and TA messages to CPU by cpssDxChBrgFdbAAandTAToCpuSet.
*       5. Set Active configuration by: cpssDxChBrgFdbActionActiveInterfaceSet,
*       cpssDxChBrgFdbActionActiveVlanSet and cpssDxChBrgFdbActionActiveDevSet.
*       6. Start triggered action by cpssDxChBrgFdbTrigActionStart
*       7. Wait that triggered action is completed by:
*       - Busy-wait poling of status - cpssDxChBrgFdbTrigActionStatusGet
*       - Wait of event CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E.
*       This wait may be done in context of dedicated task to restore
*       Active configuration and AA messages configuration.
*       for multi-port groups device :
*       the trigger is done on ALL port groups regardless to operation mode
*       (Trigger aging/transplant/delete/upload).
*       see also description of function cpssDxChBrgFdbActionModeSet about
*       'multi-port groups device'.
*
*/
GT_STATUS cpssDxChBrgFdbTrigActionStart
(
    IN GT_U8                        devNum,
    IN CPSS_FDB_ACTION_MODE_ENT     mode
);

/**
* @internal cpssDxChBrgFdbActionModeSet function
* @endinternal
*
* @brief   Sets FDB action mode without setting Action Trigger
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - FDB action mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device or mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - the setting not allowed in current state of
*                                       configuration.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       set configuration to all portGroups with same value.
*       1. Age with delete (may be Triggered or Auto)
*       a. Unified FDBs mode and Unified-Linked FDB mode :
*       This type of operation is not supported , due to the control
*       learning being used, the risk of AA messages being dropped & the
*       need to avoid dropping MAC entries from portGroups which are not
*       the source of these entries (meaning they would never be refreshed)
*       API returns GT_BAD_STATE if trying to use this action mode.
*       b. Linked FDBs mode :
*       Application should use this type of operation only in Automatic mode
*       (not in Controlled mode) + Static FDB entries .
*       In this mode automatic learnt addresses would be silently removed
*       by PP while application addresses would be removed only by specific
*       application API call.
*       2. Age without delete (may be Triggered or Auto)
*       The only type of Aging that can be used with Unified FDBs mode and
*       Unified-Linked FDB mode (in Linked FDBs mode it should be used when
*       using controlled mode), AA would be sent by PP to Application that
*       would remove entry from relevant portGroups.
*       3. Transplant (only Triggered)
*       a. Unified-Linked FDB mode and Linked FDB mode :
*       This action mode is not supported - API returns GT_BAD_STATE
*       because triggering this would possibly result with entries which
*       have their new 'source' information properly set - but these entries
*       would not be relocated to the new source portGroups, meaning that
*       packet received from new station location (which was supposed to
*       be transplanted), may still create NA message.
*       Computing new source portGroup is not always possible, especially
*       when station is relocated to trunk or it was previously residing
*       on multiple-portGroups.
*       Application is required to perform Flush operation instead.
*       This would clear previously learnt MAC
*       4. Delete (only Triggered)
*       supported in all FDB modes
*
*/
GT_STATUS cpssDxChBrgFdbActionModeSet
(
    IN GT_U8                        devNum,
    IN CPSS_FDB_ACTION_MODE_ENT     mode
);

/**
* @internal cpssDxChBrgFdbActionModeGet function
* @endinternal
*
* @brief   Gets FDB action mode.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - FDB action mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbActionModeGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_FDB_ACTION_MODE_ENT     *modePtr
);

/**
* @internal cpssDxChBrgFdbMacTriggerToggle function
* @endinternal
*
* @brief   Toggle Aging Trigger and cause the device to scan its MAC address table.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - the previous action not done
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       the trigger is done on ALL port groups regardless to operation mode
*       (Trigger aging/transplant/delete/upload).
*
*/
GT_STATUS cpssDxChBrgFdbMacTriggerToggle
(
    IN GT_U8  devNum
);

/**
* @internal cpssDxChBrgFdbTrunkAgingModeSet function
* @endinternal
*
* @brief   Sets bridge FDB Aging Mode for trunk entries.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] trunkAgingMode           - FDB aging mode for trunk entries.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or aging mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbTrunkAgingModeSet
(
    IN GT_U8                         devNum,
    IN CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT  trunkAgingMode
);

/**
* @internal cpssDxChBrgFdbTrunkAgingModeGet function
* @endinternal
*
* @brief   Get bridge FDB Aging Mode for trunk entries.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
*
* @param[out] trunkAgingModePtr        - (pointer to) FDB aging mode for trunk entries.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or aging mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbTrunkAgingModeGet
(
    IN GT_U8                        devNum,
    OUT CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT *trunkAgingModePtr
);

/**
* @internal cpssDxChBrgFdbInit function
* @endinternal
*
* @brief   Init FDB system facility for a device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - on not supported FDB table size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbInit
(
    IN  GT_U8   devNum
);

/**
* @internal cpssDxChBrgFdbAuMsgBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of FDB Address Update (AU) messages,
*         the max number of elements defined by the caller
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number from which AU are taken
* @param[in,out] numOfAuPtr               - (pointer to)max number of AU messages to get
* @param[in,out] numOfAuPtr               - (pointer to)actual number of AU messages that were received
*
* @param[out] auMessagesPtr            - array that holds received AU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       AU messages
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       1. Unified FDB mode:
*       portGroup indication is required , for:
*       a. Aging calculations of Trunks
*       b. Aging calculations of regular entries when DA refresh is enabled.
*       In these 2 cases entries are considered as aged-out only if AA is
*       receives from all 4 portGroups.
*       2. In Unified-Linked FDB mode and Linked FDB mode:
*       portGroup indication is required for these reasons:
*       a. Aging calculations of Trunk and regular entries which have
*       been written to multiple portGroups.
*       b. New addresses (NA) source portGroup indication is required
*       so application can add new entry to the exact portGroup
*       which saw this station (especially on trunk entries).
*       c. Indication on which portGroup has removed this address when
*       AA of delete is fetched (relevant when same entry was written
*       to multiple portGroups).
*       d. Indication on which portGroup currently has this address
*       when QR (query reply) is being fetched (relevant when same
*       entry was written to multiple portGroups).
*
*/
GT_STATUS cpssDxChBrgFdbAuMsgBlockGet
(
    IN     GT_U8                       devNum,
    INOUT  GT_U32                      *numOfAuPtr,
    OUT    CPSS_MAC_UPDATE_MSG_EXT_STC *auMessagesPtr
);

/**
* @internal cpssDxChBrgFdbFuMsgBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of FDB Address Update (AU) messages,
*         the max number of elements defined by the caller.
*         The PP may place FU messages in common FDB Address Update (AU) messages
*         queue or in separate queue only for FU messages. The function
*         cpssDxChHwPpPhase2Init configures queue for FU messages by the
*         fuqUseSeparate parameter. If common AU queue is used then function
*         returns all AU messages in the queue including FU ones.
*         If separate FU queue is used then function returns only FU messages.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in,out] numOfFuPtr               - (pointer to)max number of FU messages to get
* @param[in,out] numOfFuPtr               - (pointer to)actual number of FU messages that were received
*
* @param[out] fuMessagesPtr            - array that holds received FU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfFuPtr entries that was given as INPUT
*                                      parameter.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       AU messages
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - not all results of the CNC block upload
*                                       retrieved from the common used FU and CNC
*                                       upload queue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       since FU is triggered on all port groups , 'duplicated' messages may
*       appear on entries that reside on several/all port groups.
*
*/
GT_STATUS cpssDxChBrgFdbFuMsgBlockGet
(
    IN     GT_U8                       devNum,
    INOUT  GT_U32                      *numOfFuPtr,
    OUT    CPSS_MAC_UPDATE_MSG_EXT_STC *fuMessagesPtr
);

/**
* @internal cpssDxChBrgFdbAuqFuqMessagesNumberGet function
* @endinternal
*
* @brief   The function scan the AU/FU queues and returns the number of
*         AU/FU messages in the queue.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which AU are counted
* @param[in] queueType                - AUQ or FUQ. FUQ valid for DxCh2 and above.
*
* @param[out] numOfAuPtr               - (pointer to) number of AU messages in the specified queue.
*                                      for multi-port groups device :
*                                      number of AU messages returned is the summary from AUQs
*                                      of all port groups.
* @param[out] endOfQueueReachedPtr     - (pointer to)
*                                      for NON multi-port groups device :
*                                      GT_TRUE:
*                                      The queue reached to the end.
*                                      GT_FALSE:
*                                      The queue NOT reached to the end.
*                                      for multi-port groups device :
*                                      GT_TRUE:
*                                      At least one queue of the port groups has reached
*                                      its end.(this would alert the application to
*                                      start fetching the waiting messages, as at least
*                                      one of the AUQs can't accept additional messages).
*                                      GT_FALSE:
*                                      No queue reached to the end.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, queueType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on not initialized queue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In order to have the accurate number of entries application should
*       protect Mutual exclusion between HW access to the AUQ/FUQ
*       SW access to the AUQ/FUQ and calling to this API.
*       i.e. application should stop the PP from sending AU messages to CPU.
*       and should not call the api's
*       cpssDxChBrgFdbFuMsgBlockGet, cpssDxChBrgFdbFuMsgBlockGet
*
*/
GT_STATUS cpssDxChBrgFdbAuqFuqMessagesNumberGet
(
    IN  GT_U8                         devNum,
    IN  CPSS_DXCH_FDB_QUEUE_TYPE_ENT  queueType,
    OUT GT_U32                       *numOfAuPtr,
    OUT GT_BOOL                      *endOfQueueReachedPtr
);

/**
* @internal cpssDxChBrgFdbAgeBitDaRefreshEnableSet function
* @endinternal
*
* @brief   Enables/disables destination address-based aging. When this bit is
*         set, the aging process is done both on the source and the destination
*         address (i.e. the age bit will be refresh when MAC DA hit occurs, as
*         well as MAC SA hit).
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  refreshing
*                                      GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       COMMENTS
* @retval for multi                -port groups device:
* @retval Once enabled, MAC address is eligibly to be aged -out if no packets were
*                                       received from it or sent to it from any other location during aging period.
*                                       In order to properly compute this application is required to keep a
*                                       'score board' for every regular entry (same as needed for Trunk entries),
*                                       each bit representing age bit from each underlying portGroup which this
* @retval entry is written to. Entry is aged -out if all relevant portGroups have
* @retval sent AA, indicating entry was aged -out (once DA refresh is disabled -
*                                       only the source portGroup indication interests us for aging calculations).
*                                       Note:
*                                       that it's highly recommended not to enable this feature, as this
*                                       feature enables transmissions of many AA messages to the CPU
*/
GT_STATUS cpssDxChBrgFdbAgeBitDaRefreshEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
);

/**
* @internal cpssDxChBrgFdbAgeBitDaRefreshEnableGet function
* @endinternal
*
* @brief   Gets status (enabled/disabled) of destination address-based aging bit.
*         When this bit is set, the aging process is done both on the source and
*         the destination address (i.e. the age bit will be refresh when MAC DA
*         hit occurs, as well as MAC SA hit).
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) status DA refreshing of aged bit.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       COMMENTS
*/
GT_STATUS cpssDxChBrgFdbAgeBitDaRefreshEnableGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChBrgFdbRoutedLearningEnableSet function
* @endinternal
*
* @brief   Enable or disable learning on routed packets.
*         There may be cases when the ingress device has modified
*         that packet’s MAC SA to the router’s MAC SA.
*         Disable learning on routed packets prevents the FDB from
*         being filled with unnecessary routers’ Source Addresses.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  or disable Learning from Routed packets
*                                      GT_TRUE  - enable Learning from Routed packets
*                                      GT_FALSE - disable Learning from Routed packets
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutedLearningEnableSet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      enable
);

/**
* @internal cpssDxChBrgFdbRoutedLearningEnableGet function
* @endinternal
*
* @brief   Get Enable or Disable state of learning on routed packets.
*         There may be cases when the ingress device has modified
*         that packet’s MAC SA to the router’s MAC SA.
*         Disable learning on routed packets prevents the FDB from
*         being filled with unnecessary routers’ Source Addresses.
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable or disable Learning from Routed packets
*                                      GT_TRUE  - enable Learning from Routed packets
*                                      GT_FALSE - disable Learning from Routed packets
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbRoutedLearningEnableGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *enablePtr
);

/**
* @internal cpssDxChBrgFdbTriggerAuQueueWa function
* @endinternal
*
* @brief   Trigger the Address Update (AU) queue workaround for case when learning
*         from CPU is stopped due to full Address Update (AU) fifo queue
*         (FEr#3119)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_STATE             - the secondary AUQ not reached the end of
*                                       queue , meaning no need to set the secondary
*                                       base address again.
*                                       this protect the SW from losing unread messages
*                                       in the secondary queue (otherwise the PP may
*                                       override them with new messages).
*                                       caller may use function
*                                       cpssDxChBrgFdbSecondaryAuMsgBlockGet(...)
*                                       to get messages from the secondary queue.
* @retval GT_NOT_SUPPORTED         - the device not need / not support the WA
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbTriggerAuQueueWa
(
    IN  GT_U8 devNum
);

/**
* @internal cpssDxChBrgFdbSecondaryAuMsgBlockGet function
* @endinternal
*
* @brief   The function returns a block (array) of FDB Address Update (AU) messages,
*         the max number of elements defined by the caller --> from the secondary AUQ.
*         The function cpssDxChHwPpPhase2Init configures queue for secondary AU
*         messages. If common FU queue is used then function returns all AU
*         messages in the queue including FU ones.
*         If separate AU queue is used then function returns only AU messages.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number from which AU are taken
* @param[in,out] numOfAuPtr               - (pointer to)max number of AU messages to get
* @param[in,out] numOfAuPtr               - (pointer to)actual number of AU messages that were received
*
* @param[out] auMessagesPtr            - array that holds received AU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       AU messages
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbSecondaryAuMsgBlockGet
(
    IN      GT_U8                                    devNum,
    INOUT   GT_U32                                   *numOfAuPtr,
    OUT     CPSS_MAC_UPDATE_MSG_EXT_STC              *auMessagesPtr
);


/**
* @internal cpssDxChBrgFdbQueueFullGet function
* @endinternal
*
* @brief   The function returns the state that the AUQ/FUQ - is full/not full
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - the device number
* @param[in] queueType                - queue type AU/FU queue
*                                      NOTE: when using shared queue for AU and FU messages, this
*                                      parameter is ignored (and the AUQ is queried)
*                                      meaning that this parameter always ignored for DxCh1
*                                      devices.
*
* @param[out] isFullPtr                - (pointer to) is the relevant queue full
*                                      GT_TRUE  - the queue is full
*                                      GT_FALSE - the queue is not full
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum , queueType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note for multi-port groups device :
*       'queue is full' when at least one of the port groups has queue full.
*
*/
GT_STATUS cpssDxChBrgFdbQueueFullGet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_FDB_QUEUE_TYPE_ENT  queueType,
    OUT GT_BOOL *isFullPtr
);

/**
* @internal cpssDxChBrgFdbQueueRewindStatusGet function
* @endinternal
*
* @brief   function check if the specific AUQ was 'rewind' since the last time
*         this function was called for that AUQ
*         this information allow the application to know when to finish processing
*         of the WA relate to cpssDxChBrgFdbTriggerAuQueueWa(...)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] rewindPtr                - (pointer to) was the AUQ rewind since last call
*                                      GT_TRUE - AUQ was rewind since last call
*                                      GT_FALSE - AUQ wasn't rewind since last call
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbQueueRewindStatusGet
(
    IN  GT_U8                          devNum,
    OUT GT_BOOL                        *rewindPtr
);

/**
* @internal cpssDxChBrgFdbMessagesQueueManagerInfoGet function
* @endinternal
*
* @brief   debug tool - print info that the CPSS hold about the state of the :
*         primary AUQ,FUQ,secondary AU queue
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMessagesQueueManagerInfoGet
(
    IN  GT_U8   devNum
);

/**
* @internal cpssDxChBrgFdbMacEntryAgeBitSet function
* @endinternal
*
* @brief   Set age bit in specific FDB entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] index                    - hw mac entry index
* @param[in] age                      - Age flag that is used for the two-step Aging process.
*                                      GT_FALSE - The entry will be aged out in the next pass.
*                                      GT_TRUE - The entry will be aged-out in two age-passes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - index out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacEntryAgeBitSet
(
    IN GT_U8        devNum,
    IN GT_U32       index,
    IN GT_BOOL      age
);

/**
* @internal cpssDxChBrgFdbNaMsgVid1EnableSet function
* @endinternal
*
* @brief   Enable/Disable Tag1 VLAN Id assignment in vid1 field of the NA AU
*         message CPSS_MAC_UPDATE_MSG_EXT_STC.
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: vid1 field of the NA AU message is taken from
*                                      Tag1 VLAN.
*                                      GT_FALSE: vid1 field of the NA AU message is taken from
*                                      Tag0 VLAN.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbNaMsgVid1EnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgFdbNaMsgVid1EnableGet function
* @endinternal
*
* @brief   Get the status of Tag1 VLAN Id assignment in vid1 field of the NA AU
*         message CPSS_MAC_UPDATE_MSG_EXT_STC (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE: vid1 field of the NA AU message is taken from
*                                      Tag1 VLAN.
*                                      GT_FALSE: vid1 field of the NA AU message is taken from
*                                      Tag0 VLAN.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbNaMsgVid1EnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);




/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******* start of functions with portGroupsBmp parameter **********************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/**
* General NOTEs about portGroupsBmp In FDB APIs:
*   1. In Falcon :
*     the FDB is shared between 2 port groups.
*     the caller need to use 'representative' port groups , for example:
*     value 0x00000014 hold bits 2,4 represent FDBs of port groups 2,3 and 4,5
*     value 0x00000041 hold bits 0,6 represent FDBs of port groups 0,1 and 6,7
*     Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.(represent all port groups)
*
**/

/**
* @internal cpssDxChBrgFdbPortGroupMacEntrySet function
* @endinternal
*
* @brief   Create new or update existing entry in Hardware MAC address table through
*         Address Update message.(AU message to the PP is non direct access to MAC
*         address table).
*         The function use New Address message (NA) format.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that AU message was processed by PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] macEntryPtr              - pointer mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - bad device number or portGroupsBmp.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the AU message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*       If the entry type is IPv6_MC, then the Application should set the four
*       bytes for SIP and DIP that selected by cpssDxChBrgMcIpv6BytesSelectSet.
*       for multi-port groups device :
*       1. Application should not use this API to add new MAC addresses once
*       SP feature is enabled in any of the ports, due to the fact it may
*       fail on one or more of the portGroups - in this case need
*       to write by index.
*       2. Updating existing MAC addresses is not affected and
*       can be preformed with no problem
*       3. application should be aware that when configuring MAC addresses only
*       to portGroups which none of them is the real source portGroups,
*       these entries would be "unseen" from AU perspective
*       (HW is filtering any AA, TA, QR etc... messages these entries invokes).
*       And although these entries are "Unseen" there are still used for forwarding
*
*/
GT_STATUS cpssDxChBrgFdbPortGroupMacEntrySet
(
    IN GT_U8                        devNum,
    IN GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
);

/**
* @internal cpssDxChBrgFdbPortGroupQaSend function
* @endinternal
*
* @brief   The function Send Query Address (QA) message to the hardware MAC address
*         table.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that QA message was processed by PP.
*         The PP sends Query Response message after QA processing.
*         An application can get QR message by general AU message get API.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] macEntryKeyPtr           - pointer to mac entry key
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/vlan or portGroupsBmp
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the QA message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*
*/
GT_STATUS cpssDxChBrgFdbPortGroupQaSend
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_MAC_ENTRY_EXT_KEY_STC  *macEntryKeyPtr
);

/**
* @internal cpssDxChBrgFdbPortGroupMacEntryDelete function
* @endinternal
*
* @brief   Delete an old entry in Hardware MAC address table through Address Update
*         message.(AU message to the PP is non direct access to MAC address table).
*         The function use New Address message (NA) format with skip bit set to 1.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that AU message was processed by PP.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] macEntryKeyPtr           - pointer to key parameters of the mac entry
*                                      according to the entry type:
*                                      MAC Address entry -> MAC Address + vlan ID.
*                                      IPv4/IPv6 entry -> srcIP + dstIP + vald ID.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number or portGroupsBmp.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Application should synchronize call of cpssDxChBrgFdbMacEntrySet,
*       cpssDxChBrgFdbQaSend , cpssDxChBrgFdbMacEntryDelete , cpssDxChBrgFdbHashRequestSend
*       , cpssDxChBrgFdbPortGroupMacEntrySet, cpssDxChBrgFdbPortGroupQaSend ,
*       cpssDxChBrgFdbPortGroupMacEntryDelete functions.
*       Invocations of these functions should be mutual exclusive because they
*       use same HW resources.
*       Application can check that the AU message processing has completed by
*       cpssDxChBrgFdbFromCpuAuMsgStatusGet or cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet.
*
*/
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryDelete
(
    IN GT_U8                        devNum,
    IN GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
);

/**
* @internal cpssDxChBrgFdbPortGroupMacEntryWrite function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in specified index.
*         for multi-port groups device :
*         1. Unified FDBs mode :
*         (APPLICABLE DEVICES Lion; Lion2; Falcon)
*         API should return GT_BAD_PARAM if value other then CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*         was passed when working in this mode, due to the fact that all
*         FDBs are supposed to be synced.
*         Application should use this API to add new MAC addresses once SP
*         feature is enabled in any of the ports , due to the fact
*         cpssDxChBrgFdbMacEntrySet/cpssDxChBrgFdbPortGroupMacEntrySet API
*         may fail on one or more of the portGroups.
*            a. see general note about portGroupsBmp in FDB APIs for Falcon
*         2. Unified-Linked FDB mode / Linked FDBs mode :
*         (APPLICABLE DEVICES Lion; Lion2)
*         Setting either CPSS_PORT_GROUP_UNAWARE_MODE_CNS or any subset of
*         portGroup can be supported in this mode, still application should
*         note that in Linked FDBs when using multiple portGroups, setting
*         same index would potentially result affecting up to 4 different
*         MAC addresses (up to 2 different MAC addresses in Unified-Linked
*         FDB mode).
*         3.
*         (APPLICABLE DEVICES Lion; Lion2)
*         application should be aware that when configuring MAC addresses only
*         to portGroups which none of them is the real source portGroups,
*         these entries would be "unseen" from AU perspective
*         (HW is filtering any AA, TA, QR etc... messages these entries invokes).
*         And although these entries are "Unseen" there are still used for forwarding
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] index                    - hw mac entry index
* @param[in] skip                     - entry  control
*                                      GT_TRUE - used to "skip" the entry ,
*                                      the HW will treat this entry as "not used"
*                                      GT_FALSE - used for valid/used entries.
* @param[in] macEntryPtr              - pointer to MAC entry parameters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,portGroupsBmp,saCommand,daCommand
* @retval GT_OUT_OF_RANGE          - vidx/trunkId/portNum-devNum with values bigger then HW
*                                       support
*                                       index out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_PORT_GROUPS_BMP           portGroupsBmp,
    IN GT_U32                       index,
    IN GT_BOOL                      skip,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
);

/**
* @internal cpssDxChBrgFdbPortGroupMacEntryStatusGet function
* @endinternal
*
* @brief   Get the Valid and Skip Values of a FDB entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] index                    - hw mac entry index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index is out of range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr
);

/**
* @internal cpssDxChBrgFdbPortGroupMacEntryRead function
* @endinternal
*
* @brief   Reads the new entry in Hardware MAC address table from specified index.
*         This action do direct read access to RAM .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] index                    - hw mac entry index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
* @param[out] agedPtr                  - (pointer to) is entry aged
* @param[out] associatedHwDevNumPtr    = (pointer to) is HW device number associated with the
*                                      entry (even for vidx/trunk entries the field is used by
*                                      PP for aging/flush/transplant purpose).
*                                      Relevant only in case of Mac Address entry.
* @param[out] entryPtr                 - (pointer to) extended Mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index out of range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryRead
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  index,
    OUT GT_BOOL                 *validPtr,
    OUT GT_BOOL                 *skipPtr,
    OUT GT_BOOL                 *agedPtr,
    OUT GT_HW_DEV_NUM           *associatedHwDevNumPtr,
    OUT CPSS_MAC_ENTRY_EXT_STC  *entryPtr
);

/**
* @internal cpssDxChBrgFdbPortGroupMacEntryInvalidate function
* @endinternal
*
* @brief   Invalidate an entry in Hardware MAC address table in specified index.
*         the invalidation done by resetting to first word of the entry
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] index                    - hw mac entry index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - index out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryInvalidate
(
    IN GT_U8         devNum,
    IN GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN GT_U32        index
);

/**
* @internal cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet function
* @endinternal
*
* @brief   Get status of FDB Address Update (AU) message processing in the PP.
*         The function returns AU message processing completion and success status.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
*
* @param[out] completedPortGroupsBmpPtr - (pointer to) bitmap of port groups on which
*                                      AU message processing completion.
*                                      for NON multi-port groups device :
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS means that
*                                      operation completed , otherwise not completed
*                                      for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      when bit is set (1) - AU message was processed by PP (in the
*                                      corresponding port group).
*                                      when bit is not set (0) - AU message processed is not
*                                      completed yet by PP. (in the corresponding
*                                      port group)
*                                       a. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[out] succeededPortGroupsBmpPtr - (pointer to) bitmap of port groups on which
*                                      AU operation succeeded.
*                                      for NON multi-port groups device :
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS means that
*                                      operation succeeded , otherwise failed
*                                      for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      when bit is set (1) - AU action succeeded (in the
*                                      corresponding port group).
*                                      when bit is not set (0) - AU action has failed. (in the
*                                      corresponding port group)
*                                       a. see general note about portGroupsBmp in FDB APIs for Falcon
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad devNum or portGroupsBmp
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For DXCH devices the succeed status doesn't relevant for query
*       address messages (message type = CPSS_QA_E) - FEr#93.
*       The succeed status is relevant only when completed.
*       An AU message sent by CPU can fail in the following cases:
*       1. The message type = CPSS_NA_E and the hash chain has reached it's
*       maximum length.
*       2. The message type = CPSS_QA_E or CPSS_AA_E and the FDB entry doesn't
*       exist.
*
*/
GT_STATUS cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_PORT_GROUPS_BMP  *completedPortGroupsBmpPtr,
    OUT GT_PORT_GROUPS_BMP  *succeededPortGroupsBmpPtr
);

/**
* @internal cpssDxChBrgFdbPortGroupMacEntryAgeBitSet function
* @endinternal
*
* @brief   Set age bit in specific FDB entry.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] index                    - hw mac entry index
* @param[in] age                      - Age flag that is used for the two-step Aging process.
*                                      GT_FALSE - The entry will be aged out in the next pass.
*                                      GT_TRUE - The entry will be aged-out in two age-passes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - index out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbPortGroupMacEntryAgeBitSet
(
    IN GT_U8        devNum,
    IN GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN GT_U32       index,
    IN GT_BOOL      age
);

/**
* @internal cpssDxChBrgFdbPortGroupTrigActionStatusGet function
* @endinternal
*
* @brief   Get the action status of the FDB action registers.
*         When the status is changing from GT_TRUE to GT_FALSE, then the device
*         performs the action according to the setting of action registers.
*         When the status returns to be GT_TRUE then the action is completed.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
*
* @param[out] actFinishedPortGroupsBmpPtr - (pointer to) bitmap of port groups on which
*                                      'trigger action' processing completion.
*                                      for NON multi-port groups device :
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS means that
*                                      operation completed , otherwise not completed
*                                      for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      when bit is set (1) - 'trigger action' was completed by PP (in the
*                                      corresponding port group).
*                                      when bit is not set (0) - 'trigger action' is not
*                                      completed yet by PP. (in the corresponding
*                                      port group)
*                                       a. see general note about portGroupsBmp in FDB APIs for Falcon
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad devNum or portGroupsBmp
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbPortGroupTrigActionStatusGet
(
    IN  GT_U8    devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    OUT GT_PORT_GROUPS_BMP  *actFinishedPortGroupsBmpPtr
);

/**
* @internal cpssDxChBrgFdbPortGroupQueueFullGet function
* @endinternal
*
* @brief   The function returns the state that the AUQ/FUQ - is full/not full
*
* @note   APPLICABLE DEVICES:      DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in FDB APIs for Falcon
* @param[in] queueType                - queue type AU/FU queue
*                                      NOTE: when using shared queue for AU and FU messages, this
*                                      parameter is ignored (and the AUQ is queried)
*                                      meaning that this parameter always ignored for DxCh1
*                                      devices.
*
* @param[out] isFullPortGroupsBmpPtr   - (pointer to) bitmap of port groups on which
*                                      'the queue is full'.
*                                      for NON multi-port groups device :
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS means that
*                                      'queue is full' , otherwise 'queue is NOT full'
*                                      for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Falcon)
*                                      when bit is set (1) - 'queue is full' (in the
*                                      corresponding port group).
*                                      when bit is not set (0) - 'queue is NOT full'. (in the corresponding
*                                      port group)
*                                       a. see general note about portGroupsBmp in FDB APIs for Falcon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad devNum or portGroupsBmp or queueType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbPortGroupQueueFullGet
(
    IN GT_U8    devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN CPSS_DXCH_FDB_QUEUE_TYPE_ENT  queueType,
    OUT GT_PORT_GROUPS_BMP  *isFullPortGroupsBmpPtr
);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******* end of functions with portGroupsBmp parameter ************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/**
* @internal cpssDxChBrgFdb16BitFidHashEnableSet function
* @endinternal
*
* @brief   global enable/disable configuration for FID 16 bits.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: hash is calculated with FID[11:0],
*                                      GT_TRUE : hash is calculated with FID[15:0]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NOTE: For systems with both new and old devices must configure
*       FID = VID in the eVLAN table.
*
*/
GT_STATUS cpssDxChBrgFdb16BitFidHashEnableSet
(
    IN GT_U8            devNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgFdb16BitFidHashEnableGet function
* @endinternal
*
* @brief   Get global configuration for FID 16 bits.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: hash is calculated with FID[11:0],
*                                      GT_TRUE : hash is calculated with FID[15:0]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdb16BitFidHashEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgFdbMaxLengthSrcIdEnableSet function
* @endinternal
*
* @brief   global enable/disable configuration for Src ID 12 bits.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: The SrcID field in FDB table is 9b.
*                                      SrcID[11:9] are used for extending the
*                                      user defined bits
*                                      GT_TRUE : The SrcID filed in FDB is 12b
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMaxLengthSrcIdEnableSet
(
    IN GT_U8            devNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgFdbMaxLengthSrcIdEnableGet function
* @endinternal
*
* @brief   Get global configuration for Src ID 12 bits.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: The SrcID field in FDB table is 9b.
*                                      SrcID[11:9] are used for extending the
*                                      user defined bits
*                                      GT_TRUE : The SrcID filed in FDB is 12b
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMaxLengthSrcIdEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgFdbVid1AssignmentEnableSet function
* @endinternal
*
* @brief   Enable/Disable using of VID1 in FDB entry.
*         The Source ID bits [8:6], SA Security Level and DA Security Level are read as 0
*         from the FDB entry when VID1 enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: <Tag1 VID> is not written in the FDB and is not read from the FDB.
*                                      <SrcID>[8:6] can be used for src-id filtering and <SA Security Level>
*                                      and <DA Security Level> are written/read from the FDB.
*                                      GT_TRUE:  Tag1 VID> is written in the FDB and read from the FDB
*                                      as described in Mac Based VLAN FS section.
*                                      <SrcID>[8:6], <SA Security Level> and <DA Security Level>
*                                      are read as 0 from the FDB entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbVid1AssignmentEnableSet
(
    IN GT_U8            devNum,
    IN GT_BOOL          enable
);

/**
* @internal cpssDxChBrgFdbVid1AssignmentEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable status of using of VID1 in FDB entry.
*         The Source ID bits [8:6], SA Security Level and DA Security Level are read as 0
*         from the FDB entry when VID1 enabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: <Tag1 VID> is not written in the FDB and is not read from the FDB.
*                                      <SrcID>[8:6] can be used for src-id filtering and <SA Security Level>
*                                      and <DA Security Level> are written/read from the FDB.
*                                      GT_TRUE:  Tag1 VID> is written in the FDB and read from the FDB
*                                      as described in Mac Based VLAN FS section.
*                                      <SrcID>[8:6], <SA Security Level> and <DA Security Level>
*                                      are read as 0 from the FDB entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbVid1AssignmentEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChBrgFdbSaLookupAnalyzerIndexSet function
* @endinternal
*
* @brief   Set analyzer index used when the FDB SA lookup has
*         <SA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - set mirror to analyzer index field
*                                      GT_FALSE - set ZERO to the analyzer index field
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NOTE: If a previous mechanism assigns a different analyzer index,
*       the higher index wins.
*
*/
GT_STATUS cpssDxChBrgFdbSaLookupAnalyzerIndexSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable,
    IN GT_U32  index
);

/**
* @internal cpssDxChBrgFdbSaLookupAnalyzerIndexGet function
* @endinternal
*
* @brief   Get analyzer index used when the FDB SA lookup has
*         <SA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - analyzer index field is NOT ZERO
*                                      GT_FALSE - analyzer index field is set to ZERO
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbSaLookupAnalyzerIndexGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
);

/**
* @internal cpssDxChBrgFdbDaLookupAnalyzerIndexSet function
* @endinternal
*
* @brief   Set analyzer index used when the FDB DA lookup has
*         <DA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - set mirror to analyzer index field
*                                      GT_FALSE - set ZERO to the analyzer index field
* @param[in] index                    - Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NOTE: If a previous mechanism assigns a different analyzer index,
*       the higher index wins.
*
*/
GT_STATUS cpssDxChBrgFdbDaLookupAnalyzerIndexSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable,
    IN GT_U32  index
);

/**
* @internal cpssDxChBrgFdbDaLookupAnalyzerIndexGet function
* @endinternal
*
* @brief   Get analyzer index used when the FDB DA lookup has
*         <DA Lookup Ingress Mirror to Analyzer Enable> set.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE  - analyzer index field is NOT ZERO
*                                      GT_FALSE - analyzer index field is set to ZERO
* @param[out] indexPtr                 - (pointer to) Mirror to Analyzer Index (APPLICABLE RANGES: 0..6)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbDaLookupAnalyzerIndexGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr,
    OUT GT_U32  *indexPtr
);

/**
* @internal cpssDxChBrgFdbActionActiveUserDefinedSet function
* @endinternal
*
* @brief   Set the Active userDefined and active userDefined mask , that
*         control the FDB actions of : Trigger aging , transplant , flush ,
*         flush port.
*         A FDB entry will be treated by the FDB action only if the entry's
*         "userDefined" masked by the "userDefinedmask" equal to
*         "Active userDefined"
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] actUerDefined            - action active UerDefined
* @param[in] actUerDefinedMask        - action active UerDefined Mask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - actUerDefined or actUerDefinedMask out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is no check done of current status of action's register
*
*/
GT_STATUS cpssDxChBrgFdbActionActiveUserDefinedSet
(
    IN GT_U8    devNum,
    IN GT_U32   actUerDefined,
    IN GT_U32   actUerDefinedMask
);

/**
* @internal cpssDxChBrgFdbActionActiveUserDefinedGet function
* @endinternal
*
* @brief   Get the Active userDefined and active userDefined mask , that
*         control the FDB actions of : Trigger aging , transplant , flush ,
*         flush port.
*         A FDB entry will be treated by the FDB action only if the entry's
*         "userDefined" masked by the "userDefinedmask" equal to
*         "Active userDefined"
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] actUerDefinedPtr         - pointer to action active UerDefined
* @param[out] actUerDefinedMaskPtr     - pointer to action active UerDefined Mask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbActionActiveUserDefinedGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *actUerDefinedPtr,
    OUT GT_U32   *actUerDefinedMaskPtr
);

/**
* @internal cpssDxChBrgFdbLearnPrioritySet function
* @endinternal
*
* @brief   Set Learn priority per port, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] learnPriority            - Learn priority can be Low or High
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for trunk configuration: cpssDxChTrunkLearnPrioritySet
*
*/
GT_STATUS cpssDxChBrgFdbLearnPrioritySet
(
    IN GT_U8                                devNum,
    IN GT_PORT_NUM                          portNum,
    IN CPSS_DXCH_FDB_LEARN_PRIORITY_ENT     learnPriority
);

/**
* @internal cpssDxChBrgFdbLearnPriorityGet function
* @endinternal
*
* @brief   Get Learn priority per port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] learnPriorityPtr         - (pointer to)Learn priority can be Low or High
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for trunk configuration: cpssDxChTrunkLearnPriorityGet
*
*/
GT_STATUS cpssDxChBrgFdbLearnPriorityGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    OUT CPSS_DXCH_FDB_LEARN_PRIORITY_ENT    *learnPriorityPtr
);

/**
* @internal cpssDxChBrgFdbUserGroupSet function
* @endinternal
*
* @brief   Set User Group per port, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] userGroup                - user group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for trunk configuration: cpssDxChTrunkUserGroupSet
*
*/
GT_STATUS cpssDxChBrgFdbUserGroupSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      portNum,
    IN GT_U32           userGroup
);

/**
* @internal cpssDxChBrgFdbUserGroupGet function
* @endinternal
*
* @brief   Get User Group per port, used for Mac SA moved logic
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  port number
*
* @param[out] userGroupPtr             - (pointer to) user group
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note There is a parallel API for trunk configuration: cpssDxChTrunkUserGroupGet
*
*/
GT_STATUS cpssDxChBrgFdbUserGroupGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *userGroupPtr
);



/**
* @internal cpssDxChBrgFdbBankCounterValueGet function
* @endinternal
*
* @brief   Get the value of counter of the specific FDB table bank.
*         relevant to CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] bankIndex                - the index of the bank
*                                      (APPLICABLE RANGES: 0..15)
*
* @param[out] valuePtr                 - (pointer to)the value of the counter.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbBankCounterValueGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          bankIndex,
    OUT GT_U32          *valuePtr
);

/**
* @internal cpssDxChBrgFdbBankCounterUpdate function
* @endinternal
*
* @brief   Update (increment/decrement) the counter of the specific bank
*         The function will fail (GT_BAD_STATE) if the PP is busy with previous
*         update.
*         the application can determine if PP is ready by calling
*         cpssDxChBrgFdbBankCounterUpdateStatusGet.
*         relevant to CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E
*         NOTE: this function is needed to be called after calling one of the next:
*         'write by index' - cpssDxChBrgFdbMacEntryWrite ,
*         cpssDxChBrgFdbPortGroupMacEntryWrite
*         'invalidate by index' - cpssDxChBrgFdbMacEntryInvalidate ,
*         cpssDxChBrgFdbPortGroupMacEntryInvalidate
*         The application logic should be:
*         if last action was 'write by index' then :
*         if the previous entry (in the index) was valid --
*         do no call this function.
*         if the previous entry (in the index) was not valid --
*         do 'increment'.
*         if last action was 'invalidate by index' then :
*         if the previous entry (in the index) was valid --
*         do 'decrement'.
*         if the previous entry (in the index) was not valid --
*         do no call this function.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] bankIndex                - the index of the bank
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] incOrDec                 - increment or decrement the counter by one.
*                                      GT_TRUE - increment the counter
*                                      GT_FALSE - decrement the counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - bankIndex > 15
* @retval GT_BAD_STATE             - the PP is not ready to get a new update from CPU.
*                                       (PP still busy with previous update)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbBankCounterUpdate
(
    IN GT_U8                        devNum,
    IN GT_U32                       bankIndex,
    IN GT_BOOL                      incOrDec
);

/**
* @internal cpssDxChBrgFdbBankCounterUpdateStatusGet function
* @endinternal
*
* @brief   Get indication if PP finished processing last update of counter of a
*         specific bank. (cpssDxChBrgFdbBankCounterUpdate)
*         relevant to CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] isFinishedPtr            - (pointer to) indication that the PP finished processing
*                                      the last counter update.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbBankCounterUpdateStatusGet
(
    IN GT_U8                        devNum,
    OUT GT_BOOL                     *isFinishedPtr
);


/**
* @internal cpssDxChBrgFdbMacEntryMuxingModeSet function
* @endinternal
*
* @brief   Set FDB mac entry muxing modes.
*         how the 12 bits that are muxed in the FDB mac entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] muxingMode               - one of the muxing modes according to
*                                      CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or muxingMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacEntryMuxingModeSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT muxingMode
);

/**
* @internal cpssDxChBrgFdbMacEntryMuxingModeGet function
* @endinternal
*
* @brief   Get FDB mac entry muxing modes.
*         how the 12 bits that are muxed in the FDB mac entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
*
* @param[out] muxingModePtr            - (pointer to) one of the muxing modes according to
*                                      CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or muxingMode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbMacEntryMuxingModeGet
(
    IN GT_U8            devNum,
    OUT CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT *muxingModePtr
);


/**
* @internal cpssDxChBrgFdbIpmcEntryMuxingModeSet function
* @endinternal
*
* @brief   Set FDB IPMC (Ipv4MC/Ipv6MC) entry muxing modes.
*         how the 12 bits that are muxed in the IPMC (Ipv4MC/Ipv6MC) mac entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] muxingMode               - one of the muxing modes according to
*                                      CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or muxingMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbIpmcEntryMuxingModeSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT muxingMode
);

/**
* @internal cpssDxChBrgFdbIpmcEntryMuxingModeGet function
* @endinternal
*
* @brief   Get FDB IPMC (Ipv4MC/Ipv6MC) entry muxing modes.
*         how the 12 bits that are muxed in the IPMC (Ipv4MC/Ipv6MC) mac entry.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
*
* @param[out] muxingModePtr            - (pointer to) one of the muxing modes according to
*                                      CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or muxingMode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unexpected value reading from the hw
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbIpmcEntryMuxingModeGet
(
    IN GT_U8            devNum,
    OUT CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT *muxingModePtr
);

/**
* @internal cpssDxChBrgFdbSaLookupSkipModeSet function
* @endinternal
*
* @brief  Sets the SA lookup skip mode for the bridge.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] saLookupMode          - SA lookup mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or saLookupMode
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbSaLookupSkipModeSet
(
    IN GT_U8            devNum,
    IN CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT saLookupMode
);

/**
* @internal cpssDxChBrgFdbSaLookupSkipModeGet function
* @endinternal
*
* @brief  Gets the SA lookup skip mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @param[out] saLookupMode         - (pointer to)SA lookup mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on unexpected value reading from the hw
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbSaLookupSkipModeGet
(
    IN GT_U8            devNum,
    OUT CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT *saLookupModePtr
);

/**
* @internal cpssDxChBrgFdbSaDaCmdDropModeSet function
* @endinternal
*
* @brief  Sets SA and DA Drop command mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @param[in] saDropCmdMode         - Source Address Drop Command mode, valid values are
*                                    CPSS_MAC_TABLE_DROP_E      - hard drop
*                                    CPSS_MAC_TABLE_SOFT_DROP_E - soft drop
* @param[in] daDropCmdMode         - Destination Address Drop Command mode, valid values are
*                                    CPSS_MAC_TABLE_DROP_E      - hard drop
*                                    CPSS_MAC_TABLE_SOFT_DROP_E - soft drop
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter values
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbSaDaCmdDropModeSet
(
    IN  GT_U8                  devNum,
    IN  CPSS_MAC_TABLE_CMD_ENT saDropCmdMode,
    IN  CPSS_MAC_TABLE_CMD_ENT daDropCmdMode
);

/**
* @internal cpssDxChBrgFdbSaDaCmdDropModeGet function
* @endinternal
*
* @brief  Gets SA and DA Drop command mode.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum                - device number
*
* @param[out] saDropCmdMode         - (pointer to)Source Address Drop Command mode, valid values are
*                                     CPSS_MAC_TABLE_DROP_E      - hard drop
*                                     CPSS_MAC_TABLE_SOFT_DROP_E - soft drop
* @param[out] daDropCmdMode         - (pointer to)Destination Address Drop Command mode, valid values are
*                                     CPSS_MAC_TABLE_DROP_E      - hard drop
*                                     CPSS_MAC_TABLE_SOFT_DROP_E - soft drop
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbSaDaCmdDropModeGet
(
    IN  GT_U8                  devNum,
    OUT CPSS_MAC_TABLE_CMD_ENT *saDropCmdModePtr,
    OUT CPSS_MAC_TABLE_CMD_ENT *daDropCmdModePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgFdbh */

