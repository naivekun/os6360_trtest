/*******************************************************************************
 *              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file tgfBridgeGen.h
*
* @brief Generic API for Bridge
*
*
* @version   1
********************************************************************************
*/
#ifndef __tgfBridgeGen
#define __tgfBridgeGen

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/generic/bridge/cpssGenBrgGen.h>
#include <extUtils/tgf/tgfCommonList.h>
#include <extUtils/tgf/tgfCommon.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#endif  /* CHX_FAMILY */

/* indication that the FDB shadow used */
extern GT_BOOL prvTgfFdbShadowUsed;

/**
* @enum PRV_TGF_FDB_ENTRY_TYPE_ENT
 *
 * @brief This enum defines the type of the FDB Entry
*/
typedef enum{

    /** The entry is MAC Address */
    PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E,

    /** The entry is IPv4 Multicast */
    PRV_TGF_FDB_ENTRY_TYPE_IPV4_MCAST_E,

    /** The entry is IPv6 Multicast */
    PRV_TGF_FDB_ENTRY_TYPE_IPV6_MCAST_E,

    /** The entry is VPLS entry */
    PRV_TGF_FDB_ENTRY_TYPE_VPLS_E,

    /** The entry is IPv4 UC entry */
    PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E,

    /** The entry is IPv6 UC address entry */
    PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E,

    /** The entry is IPv6 UC data entry */
    PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E,

    /** @brief The entry is MAC Address
     *  Double Tag FDB Lookup Key Mode is used
     */
    PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E,

    /** @brief The entry is IPv4 Multicast
     *  Double Tag FDB Lookup Key Mode is used
     */
    PRV_TGF_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E,

    /** @brief The entry is IPv6 Multicast
     *  Double Tag FDB Lookup Key Mode is used
     */
    PRV_TGF_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E

} PRV_TGF_FDB_ENTRY_TYPE_ENT;

/**
* @enum PRV_TGF_PACKET_CMD_ENT
 *
 * @brief This enum defines the packet command.
*/
typedef enum{

    /** forward packet */
    PRV_TGF_PACKET_CMD_FORWARD_E,

    /** mirror packet to CPU */
    PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E,

    /** trap packet to CPU */
    PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E,

    /** hard drop packet */
    PRV_TGF_PACKET_CMD_DROP_HARD_E,

    /** soft drop packet */
    PRV_TGF_PACKET_CMD_DROP_SOFT_E,

    /** IP Forward the packets */
    PRV_TGF_PACKET_CMD_ROUTE_E,

    /** Packet is routed and mirrored */
    PRV_TGF_PACKET_CMD_ROUTE_AND_MIRROR_E,

    /** Bridge and Mirror to CPU. */
    PRV_TGF_PACKET_CMD_BRIDGE_AND_MIRROR_E,

    /** Bridge only */
    PRV_TGF_PACKET_CMD_BRIDGE_E,

    /** intervention to CPU */
    PRV_TGF_PACKET_CMD_INTERV_E,

    /** control (unconditionally trap to CPU) */
    PRV_TGF_PACKET_CMD_CNTL_E,

    /** Do nothing. (disable) */
    PRV_TGF_PACKET_CMD_NONE_E

} PRV_TGF_PACKET_CMD_ENT;


/**
* @struct PRV_TGF_MAC_ENTRY_KEY_STC
 *
 * @brief Key parameters of the MAC Address Entry
*/
typedef struct
{
    PRV_TGF_FDB_ENTRY_TYPE_ENT         entryType;
    GT_U32                             vid1;
    union  {
       struct {
           GT_ETHERADDR     macAddr;
           GT_U16           vlanId;
       } macVlan;

       struct {
           GT_U8            sip[4];
           GT_U8            dip[4];
           GT_U16           vlanId;
       } ipMcast;

       struct{
           GT_U8            dip[4];
           GT_U32           vrfId;
       }ipv4Unicast;

       struct{
           GT_U8            dip[16];
           GT_U32           vrfId;
       }ipv6Unicast;

    } key;

} PRV_TGF_MAC_ENTRY_KEY_STC;

/**
* @struct PRV_TGF_BRG_MAC_ENTRY_STC
 *
 * @brief FDB Entry
*/
typedef struct{

    /** key data */
    PRV_TGF_MAC_ENTRY_KEY_STC key;

    /** interface info */
    CPSS_INTERFACE_INFO_STC dstInterface;

    /** @brief Age flag that is used for the two
     *  GT_FALSE - The entry will be aged out in the next pass.
     *  GT_TRUE - The entry will be aged-out in two age-passes.
     *  Notes: used only for DxCh devices.
     *  IP UC entries are not aged out, but only marked as such
     */
    GT_BOOL age;

    /** outlif info: port/trunk/vidx/dit */
    PRV_TGF_OUTLIF_INFO_STC dstOutlif;

    /** static/dynamic entry */
    GT_BOOL isStatic;

    /** action taken when a packet's DA matches this entry */
    PRV_TGF_PACKET_CMD_ENT daCommand;

    /** action taken when a packet's SA matches this entry */
    PRV_TGF_PACKET_CMD_ENT saCommand;

    /** send packet to the IPv4 or MPLS */
    GT_BOOL daRoute;

    /** configurable analyzer port. */
    GT_BOOL mirrorToRxAnalyzerPortEn;

    /** user defined field */
    GT_U32 userDefined;

    /** destination Qos attribute */
    GT_U32 daQosIndex;

    /** source Qos attribute */
    GT_U32 saQosIndex;

    /** security level assigned to the MAC DA */
    GT_U32 daSecurityLevel;

    /** security level assigned to the MAC SA */
    GT_U32 saSecurityLevel;

    /** specific CPU code */
    GT_BOOL appSpecificCpuCode;

    /** Pseudo */
    GT_U32 pwId;

    /** @brief relevant when auto
     *  vid1      - Tag1 VID value. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     */
    GT_BOOL spUnknown;

    /** Source ID */
    GT_U32 sourceId;

    CPSS_FDB_UC_ROUTING_INFO_STC fdbRoutingInfo;

} PRV_TGF_BRG_MAC_ENTRY_STC;


/**
* @struct PRV_TGF_BRG_FDB_AUQ_MESSAGE_PART1_STC
 *
 * @brief An item of linked list used to store FDB AU messages
*/
typedef struct{

    /** @brief a basic linked list struct. Should be always a first
     *  struct's field to make it possible a using of
     *  this structure as descendant of PRV_TGF_LIST_ITEM_STC
     */
    PRV_TGF_LIST_ITEM_STC item;

    /** a device number, generated the AU message */
    GT_U8 devNum;

    /** an FDB AU message */
    CPSS_MAC_UPDATE_MSG_EXT_STC message;

} PRV_TGF_BRG_FDB_AUQ_MESSAGE_PART1_STC;

extern PRV_TGF_LIST_HEAD_STC brgFdbAuqMessagesPart1;

#ifdef CHX_FAMILY

extern GT_U32 prvTgfBrgFdbDoNaShadow_part2;

/**
* @struct PRV_TGF_BRG_FDB_AUQ_MESSAGE_PART2_STC
 *
 * @brief An item of linked list used to store FDB entries
*/
typedef struct{

    /** @brief a basic linked list struct. Should be always a first struct's
     *  field to make it possible a using of this structure as
     *  descendant of PRV_TGF_LIST_ITEM_STC
     */
    PRV_TGF_LIST_ITEM_STC item;

    /** a device number wielding FDB table */
    GT_U8 devNum;

    /** @brief is required when a node should be deleted logically but can't
     *  be deleted phisically yet. For example, you need to read some
     *  field of this node after calling the function which should
     *  "think" this node is deleted
     */
    GT_BOOL deleted;

    GT_U32 index;

    /** @brief a mac entry. In case of FDB IP UC Entries added by index this
     *  field will hold the macEntryIndex where the entry was added in
     *  the HW. If the entry is added by message this field value will
     *  be 0xFFFFFFFF
     */
    PRV_TGF_BRG_MAC_ENTRY_STC message;

} PRV_TGF_BRG_FDB_AUQ_MESSAGE_PART2_STC;

extern PRV_TGF_LIST_HEAD_STC brgFdbAuqMessagesPart2;
#endif /*CHX_FAMILY*/


/*************************** functions declaration ****************************/
/**
* @internal prvTgfBrgFdbInit function
* @endinternal
*
* @brief   Init the TGF FDB lib. needed for GM FDB shadow.
*         (but with flag allowAnyDevice = true , we can use it for any device)
* @param[in] devNum                   - the device number
*                                      allowAnyDevice - allow any device to use the shadow.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS   prvTgfBrgFdbInit(IN GT_U8 devNum , IN GT_BOOL allowAnyDevice);

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericToDxChMacEntryKey function
* @endinternal
*
* @brief   Convert generic MAC entry key into device specific MAC entry key
*
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry key parameters
*
* @param[out] dxChMacEntryKeyPtr       - (pointer to) DxCh MAC entry key parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChMacEntryKey
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *macEntryKeyPtr,
    OUT CPSS_MAC_ENTRY_EXT_KEY_STC   *dxChMacEntryKeyPtr
);


/**
* @internal prvTgfConvertCpssToGenericMacKey function
* @endinternal
*
* @brief   Convert DX MAC entry key into device generic MAC entry key
*
* @param[in] dxChMacEntryKeyPtr       - (pointer to) DxCh MAC entry key parameters
*
* @param[out] macEntryKeyPtr           - (pointer to) MAC entry key parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertCpssToGenericMacKey
(
    IN CPSS_MAC_ENTRY_EXT_KEY_STC       *dxChMacEntryKeyPtr,
    OUT  PRV_TGF_MAC_ENTRY_KEY_STC      *macEntryKeyPtr
);

/**
* @internal prvTgfConvertGenericToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devNum                   - device number
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToDxChMacEntry
(
    IN  GT_U8                         devNum,
    IN  PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr,
    OUT CPSS_MAC_ENTRY_EXT_STC       *dxChMacEntryPtr
);

/**
* @internal prvTgfConvertDxChToGenericMacEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertDxChToGenericMacEntry
(
    IN  CPSS_MAC_ENTRY_EXT_STC       *dxChMacEntryPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
);


/**
* @internal prvTgfBrgFdbMacEntryDelete_unregister function
* @endinternal
*
* @brief   Remove the MAC entry from the linked list brgFdbAuqMessagesPart2
*
* @param[in] devNum                   - a device number
* @param[in] macEntryKeyPtr           - a MAC entry struct pointer
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvTgfBrgFdbMacEntryDelete_unregister
(
    IN GT_U8                          devNum,
    IN PRV_TGF_MAC_ENTRY_KEY_STC     *macEntryKeyPtr
);

/**
* @internal prvTgfBrgFdbMacEntrySet_register function
* @endinternal
*
* @brief   Add the MAC entry to the linked list brgFdbAuqMessagesPart2
*
* @param[in] devNum                   - a device number
* @param[in] index                    - an FDB entry index
*                                      macEntryKeyPtr - a MAC entry struct pointer
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvTgfBrgFdbMacEntrySet_register
(
    IN GT_U8                         devNum,
    IN GT_U32                        index,
    IN PRV_TGF_BRG_MAC_ENTRY_STC     *macEntryPtr
);

/**
* @internal prvTgfBrgFdbMacEntryWrite_register function
* @endinternal
*
* @brief   Add/Remove the MAC entry to/from the linked list brgFdbAuqMessagesPart2
*         depending on the 'skip' parameter.
* @param[in] devNum                   - a device number
* @param[in] index                    - an FDB entry index
* @param[in] skip                     - whether to delete (GT_TRUE) or add (GT_FASE) a mac
*                                      entry to the linked list
*                                      macEntryKeyPtr - a MAC entry struct pointer
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvTgfBrgFdbMacEntryWrite_register
(
    IN GT_U8                         devNum,
    IN GT_U32                        index,
    IN GT_BOOL                       skip,
    IN PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
);

/**
* @internal prvTgfBrgFdbFlushByShadowPart1 function
* @endinternal
*
* @brief   Go through the linked list brgFdbAuqMessagesPart1 look for NA
*         messages and delete appropriate FDB entries
* @param[in] devNum                   - a device number
* @param[in] fdbDelMode               - FDB deletion mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS prvTgfBrgFdbFlushByShadowPart1
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_BRG_FDB_DEL_MODE_ENT fdbDelMode
);

/**
* @internal prvTgfBrgFdbFlushByShadowPart2 function
* @endinternal
*
* @brief   Go through the linked list brgFdbAuqMessagesPart2
*         and delete FDB entries that apporopriate to the list's items
* @param[in] devNum                   - a device number
* @param[in] fdbDelMode               - FDB deletion mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS prvTgfBrgFdbFlushByShadowPart2
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_BRG_FDB_DEL_MODE_ENT fdbDelMode
);

/**
* @internal prvTgfBrgFdbMacEntryFromShadowGetNext_shadowPart1 function
* @endinternal
*
* @brief   get next entry from the FDB shadow 'part 1'.
*
* @param[in] devNum                   - a device number
*                                      fdbDelMode - FDB deletion mode
* @param[in,out] cookiePtr                - (pointer to) cookiePtr. the 'cookie' as IN
*                                      (identify the previous entry returned)
*                                      when (cookiePtrPtr)==NULL means to start from first element
* @param[in,out] cookiePtr                - (pointer to) cookiePtr. the 'cookie' as OUT
*                                      (identify the current entry returned)
*
* @param[out] OUT_macEntryPtr          - (pointer to) the fdb entry found in the shadow.
*
* @retval GT_OK                    - on success,
* @retval GT_NO_MORE               - no more entries in this FDB shadow.
*/
GT_STATUS prvTgfBrgFdbMacEntryFromShadowGetNext_shadowPart1
(
    IN GT_U8                         devNum,
    IN CPSS_DXCH_BRG_FDB_DEL_MODE_ENT mode,
    INOUT GT_U32                   *cookiePtr,
    OUT CPSS_MAC_ENTRY_EXT_STC    **OUT_macEntryPtr
);

/**
* @internal prvTgfBrgFdbMacEntryFromShadowGetNext_shadowPart2 function
* @endinternal
*
* @brief   get next entry from the FDB shadow 'part 2'.
*
* @param[in] devNum                   - a device number
*                                      fdbDelMode - FDB deletion mode
* @param[in,out] cookiePtr                - (pointer to) cookiePtr. the 'cookie' as IN
*                                      (identify the previous entry returned)
*                                      when (cookiePtrPtr)==NULL means to start from first element
* @param[in,out] cookiePtr                - (pointer to) cookiePtr. the 'cookie' as OUT
*                                      (identify the current entry returned)
*
* @param[out] OUT_macEntryPtr          - (pointer to) the fdb entry found in the shadow.
*                                      NOTE: the format is 'TGF'
*
* @retval GT_OK                    - on success,
* @retval GT_NO_MORE               - no more entries in this FDB shadow.
*/
GT_STATUS prvTgfBrgFdbMacEntryFromShadowGetNext_shadowPart2
(
    IN GT_U8                         devNum,
    IN CPSS_DXCH_BRG_FDB_DEL_MODE_ENT mode,
    INOUT GT_U32                      *cookiePtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    **OUT_macEntryPtr
);

#endif /*CHX_FAMILY*/


#ifdef EXMXPM_FAMILY
/**
* @internal prvTgfConvertGenericToExMxPmMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] exMxPmMacEntryPtr        - (pointer to) ExMxPm MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertGenericToExMxPmMacEntry
(
    IN  PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr,
    OUT CPSS_EXMXPM_FDB_ENTRY_STC    *exMxPmMacEntryPtr
);

/**
* @internal prvTgfConvertExMxPmToGenericMacEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] exMxPmMacEntryPtr        - (pointer to) ExMxPm MAC entry parameters
*
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertExMxPmToGenericMacEntry
(
    IN  CPSS_EXMXPM_FDB_ENTRY_STC    *exMxPmMacEntryPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
);


#endif /* EXMXPM_FAMILY */


/**
* @internal prvTgfBrgFdbMacEntryInvalidatePerDev function
* @endinternal
*
* @brief   Invalidate an entry in Hardware MAC address table in specified index
*         of specified device.
*         the invalidation done by resetting to first word of the entry
* @param[in] devNum                   - a device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index                    - hw mac entry index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS prvTgfBrgFdbMacEntryInvalidatePerDev
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_U32               index
);

/**
* @internal prvTgfBrgFdbHashCrcMultiResultsCalc function
* @endinternal
*
* @brief   This function calculates the CRC multiple hash results
*         (indexes into the FDB table).
*         NOTE: the function do not access the HW , and do only SW calculations.
* @param[in] devNum                   - device number
* @param[in] macEntryKeyPtr           - (pointer to) entry key
* @param[in] multiHashStartBankIndex  - the first bank for 'multi hash' result.
*                                      (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks               - number of banks for 'multi hash' result.
*                                      this value indicates the number of elements that will be
*                                      retrieved by crcMultiHashArr[]
*                                      restriction of (numOfBanks + multiHashStartBankIndex) <= 16 .
*                                      (APPLICABLE RANGES: 1..16)
*
* @param[out] crcMultiHashArr[]        - (array of) 'multi hash' CRC results. index in this array is 'bank Id'
*                                      (index 0 will hold value relate to bank multiHashStartBankIndex).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgFdbHashCrcMultiResultsCalc
(
    IN  GT_U8                       devNum,
    IN  PRV_TGF_MAC_ENTRY_KEY_STC   *macEntryKeyPtr,
    IN  GT_U32                      multiHashStartBankIndex,
    IN  GT_U32                      numOfBanks,
    OUT GT_U32                      crcMultiHashArr[]
);

/**
* @internal prvTgfBrgFdbMacEntryDeletePerDev function
* @endinternal
*
* @brief   Delete an old entry in Hardware MAC address table of specified device
*
* @param[in] devNum                   - a device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] macEntryKeyPtr           - (pointer to) key parameters of the mac entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfBrgFdbMacEntryDeletePerDev
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN PRV_TGF_MAC_ENTRY_KEY_STC     *macEntryKeyPtr
);

/**
* @internal prvTgfConvertCpssToGenericMacEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] cpssMacEntryPtr          - (pointer to) cpss MAC entry parameters
*
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfConvertCpssToGenericMacEntry
(
    IN  CPSS_MAC_ENTRY_EXT_STC       *cpssMacEntryPtr,
    OUT PRV_TGF_BRG_MAC_ENTRY_STC    *macEntryPtr
);

/**
* @internal prvTgfBrgFdbAuqCbRegister function
* @endinternal
*
* @brief   register a CB (callback) function for a specific AUQ message type.
*         the registered function (cbFuncPtr) will be called for every AUQ message
*         that match the messageType.
* @param[in] messageType              - AUQ message type
* @param[in] bindOrUnbind             - bind/unbind   :
*                                      GT_TRUE - bind
*                                      GT_FALSE - unbind
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfBrgFdbAuqCbRegister
(
    IN CPSS_UPD_MSG_TYPE_ENT       messageType,
    IN GT_BOOL                     bindOrUnbind
);

/**
* @internal tgfCpssDxChBrgFdbFromCpuAuMsgStatusPerPortGroupsGet function
* @endinternal
*
* @brief   Wait an FDB Address Update (AU) message processing is finished and get
*         it status for every specified port group.
* @param[in] devNum                   - a device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] mustSucceed              - expected status of the message processing
*
* @retval GT_OK                    - if all portGroups finished message processing
*                                       and expected status is the same as expected
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on timeout expired
* @retval GT_BAD_PARAM             - on bad devNum or portGroupsBmp
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS tgfCpssDxChBrgFdbFromCpuAuMsgStatusPerPortGroupsGet
(
    IN GT_U8              devNum,
    IN GT_PORT_GROUPS_BMP portGroupsBmp,
    IN GT_BOOL            mustSucceed
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfBridgeGen */

