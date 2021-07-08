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
* @file prvCpssTrunkTypes.h
*
* @brief API definitions for 802.3ad Link Aggregation (Trunk) facility
* private - CPSS - generic
*
* @version   20
********************************************************************************
*/

#ifndef __prvCpssTrunkTypesh
#define __prvCpssTrunkTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/trunk/cpssGenTrunkTypes.h>

/* the generic max number of members in a trunk (currently ExMxPm devices has max) */
#define PRV_CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS   12

/* the generic 8 max number of members in a trunk -- ExMx / DxSal/DxCh devices */
#define PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS   8

/* the generic 12 max number of members in a trunk -- ExMxPm devices*/
#define PRV_CPSS_TRUNK_12_MAX_NUM_OF_MEMBERS_CNS   12

#define PRV_CPSS_TRUNK_ACTION_MEMBER_MAC(devNum,trunkId,trunkPort,trunkDev)                        \
    {                                                                                              \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.memberInAction.device = (trunkDev);\
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.memberInAction.port = (trunkPort); \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkIdInAction = (trunkId);       \
    }

#define PRV_CPSS_TRUNK_ACTION_ADD_MAC(devNum,trunkId,trunkPort,trunkDev)            \
    {                                                                               \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkAction =       \
            PRV_CPSS_TRUNK_ACTION_ADD_E;                                            \
        PRV_CPSS_TRUNK_ACTION_MEMBER_MAC((devNum),(trunkId),(trunkPort),(trunkDev));\
    }

#define PRV_CPSS_TRUNK_ACTION_REMOVE_MAC(devNum,trunkId,trunkPort,trunkDev)         \
    {                                                                               \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkAction =       \
            PRV_CPSS_TRUNK_ACTION_REMOVE_E;                                         \
        PRV_CPSS_TRUNK_ACTION_MEMBER_MAC((devNum),(trunkId),(trunkPort),(trunkDev));\
    }

#define PRV_CPSS_TRUNK_ACTION_ENABLE_MAC(devNum,trunkId,trunkPort,trunkDev)         \
    {                                                                               \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkAction =       \
            PRV_CPSS_TRUNK_ACTION_ENABLE_E;                                         \
        PRV_CPSS_TRUNK_ACTION_MEMBER_MAC((devNum),(trunkId),(trunkPort),(trunkDev));\
    }


#define PRV_CPSS_TRUNK_ACTION_DISABLE_MAC(devNum,trunkId,trunkPort,trunkDev)        \
    {                                                                               \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkAction =       \
            PRV_CPSS_TRUNK_ACTION_DISABLE_E;                                        \
        PRV_CPSS_TRUNK_ACTION_MEMBER_MAC((devNum),(trunkId),(trunkPort),(trunkDev));\
    }

#define PRV_CPSS_TRUNK_ACTION_ADD_NON_TRUNK_TABLE_MAC(devNum,trunkId,trunkPort,trunkDev)\
    {                                                                               \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkAction =       \
            PRV_CPSS_TRUNK_ACTION_ADD_NON_TRUNK_TABLE_E;                            \
        PRV_CPSS_TRUNK_ACTION_MEMBER_MAC((devNum),(trunkId),(trunkPort),(trunkDev));\
    }

#define PRV_CPSS_TRUNK_ACTION_REMOVE_NON_TRUNK_TABLE_MAC(devNum,trunkId,trunkPort,trunkDev)\
    {                                                                               \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkAction =       \
            PRV_CPSS_TRUNK_ACTION_REMOVE_NON_TRUNK_TABLE_E;                         \
        PRV_CPSS_TRUNK_ACTION_MEMBER_MAC((devNum),(trunkId),(trunkPort),(trunkDev));\
    }

#define PRV_CPSS_TRUNK_ACTION_NONE_MAC(devNum)                                \
    {                                                                         \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkAction = \
            PRV_CPSS_TRUNK_ACTION_NONE_E;                                     \
        PRV_CPSS_TRUNK_ACTION_MEMBER_MAC((devNum),0xFFFF,0xFF,0xFF);          \
    }

#define PRV_CPSS_TRUNK_ACTION_INITIALIZATION_MAC(devNum)                      \
    {                                                                         \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkAction = \
            PRV_CPSS_TRUNK_ACTION_INITIALIZATION_E;                           \
        PRV_CPSS_TRUNK_ACTION_MEMBER_MAC((devNum),0xFFFF,0xFF,0xFF);          \
    }

#define PRV_CPSS_TRUNK_ACTION_DESTROY_MAC(devNum,trunkId)\
    {                                                                         \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkAction = \
            PRV_CPSS_TRUNK_ACTION_DESTROY_TRUNK_E;                           \
        PRV_CPSS_TRUNK_ACTION_MEMBER_MAC((devNum),(trunkId),0xFF,0xFF);      \
    }

#define PRV_CPSS_TRUNK_ACTION_RE_CALC_DESIGNATED_TABLE_MAC(devNum)            \
    {                                                                         \
        corePpDevs[(devNum)]->genTrunkDbPtr->trunkCurrentAction.trunkAction = \
            PRV_CPSS_TRUNK_ACTION_RE_CALC_DESIGNATED_TABLE_E;                 \
        PRV_CPSS_TRUNK_ACTION_MEMBER_MAC((devNum),0xFFFF,0xFF,0xFF);          \
    }

/* macro to get number of designated entries per table */
#define PRV_CPSS_TRUNK_DESIGNATED_TABLE_SIZE_MAC(devNum) \
    ((PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->designatedTrunkTableSplit == GT_FALSE) ? \
      PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numOfDesignatedTrunkEntriesHw :          \
      PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numOfDesignatedTrunkEntriesHw/2)


/**
* @enum PRV_CPSS_TRUNK_ACTION_ENT
 *
 * @brief enumerator that hold values for the type of action currently done by the
 * trunk lib
*/
typedef enum{

    /** no current action */
    PRV_CPSS_TRUNK_ACTION_NONE_E,

    /** the member is added to the trunk */
    PRV_CPSS_TRUNK_ACTION_ADD_E,

    /** add member as disabled */
    PRV_CPSS_TRUNK_ACTION_ADD_AS_DISABLED_E,

    /** the member is removed from the trunk */
    PRV_CPSS_TRUNK_ACTION_REMOVE_E,

    /** the member is enabled in the trunk */
    PRV_CPSS_TRUNK_ACTION_ENABLE_E,

    /** the member is disabled in the trunk */
    PRV_CPSS_TRUNK_ACTION_DISABLE_E,

    /** @brief the member is added only
     *  to the nontrunk table
     */
    PRV_CPSS_TRUNK_ACTION_NON_TRUNK_TABLE_ADD_E,

    /** @brief the member is removed
     *  only from the nontrunk table
     */
    PRV_CPSS_TRUNK_ACTION_NON_TRUNK_TABLE_REMOVE_E,

    /** the trunk lib initializing */
    PRV_CPSS_TRUNK_ACTION_INITIALIZATION_E,

    /** @brief re-calc the designated
     *  trunk table
     */
    PRV_CPSS_TRUNK_ACTION_RE_CALC_DESIGNATED_TABLE_E,

    /** @brief set entire trunk entry ,
     *  from empty trunk
     */
    PRV_CPSS_TRUNK_ACTION_SET_TRUNK_E,

    /** @brief set entire trunk
     *  entry , with the same configuration
     *  as already exists in HW
     */
    PRV_CPSS_TRUNK_ACTION_SET_KEEP_THE_CURRENT_TRUNK_E,

    /** set trunk with no members */
    PRV_CPSS_TRUNK_ACTION_CLEAR_TRUNK_E,

    /** @brief update the non-trunk
     *  table to enable/disable multidestination packets to be sent
     *  back to their source trunk on the local device
     */
    PRV_CPSS_TRUNK_ACTION_MULTI_DEST_FLOOD_BACK_SET_E,

    /** last value in the enum */
    PRV_CPSS_TRUNK_ACTION_LAST_E

} PRV_CPSS_TRUNK_ACTION_ENT;


/**
* @enum PRV_CPSS_TRUNK_LOAD_BALANCE_TYPE_ENT
 *
 * @brief enumerator that hold values for the type of how to fill the HW with
 * ports that actually effect the load balancing that the PP will generate
*/
typedef enum{

    /** @brief the load balance without any
     *  emulations/WA
     */
    PRV_CPSS_TRUNK_LOAD_BALANCE_NATIVE_E,

    /** @brief the load balance WA
     *  that needed in the TwistC,D,Samba
     */
    PRV_CPSS_TRUNK_LOAD_BALANCE_WA_TWIST_C_D_SAMBA_E,

    /** @brief the load balance to emulate
     *  the load balance that the Sx (SOHO) devices use
     *  the Dx107 needs it to work with the Opals.
     */
    PRV_CPSS_TRUNK_LOAD_BALANCE_SX_EMULATION_E,

    /** @brief the PP need to always
     *  work with 8 members in the trunk , although
     *  there are less needed SW members.
     */
    PRV_CPSS_TRUNK_LOAD_BALANCE_FORCE_8_MEMBERS_E,

    /** @brief A mode to allows flexibility for
     *  each Regular trunk to state it's max number of members (before starting to add members).
     *  (this mode not effect 'cascade trunk' members)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  Regular trunk may hold : max of 4K members. (each trunk set it's own limit)
     *  Cascade trunk may hold : max of 64 members.
     *  1. Choosing this mode allows the application to state per trunk the
     *  max number of members that will be used (each trunk can be with
     *  different max number of members)
     *  The application must call next API : cpssDxChTrunkFlexInfoSet(...)
     *  before calling the APIs that manage trunk members :
     *  cpssDxChTrunkTableEntryGet(...)
     *  cpssDxChTrunkMembersSet(...)
     *  cpssDxChTrunkMemberAdd(...)
     *  cpssDxChTrunkMemberRemove(...)
     *  cpssDxChTrunkMemberDisable(...)
     *  cpssDxChTrunkMemberEnable(...)
     *  2. Choosing this mode also impact next 2 APIs:
     *  a. cpssDxChBrgL2EcmpTableSet(...) :
     *  The CPSS will allows application full access to all 'L2 ECMP' table entries
     *  (in mode of CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E :
     *  the application is limited to 1/2 the table (lower half))
     *  b. cpssDxChBrgL2EcmpLttTableSet(...) :
     *  The CPSS will allows application full manage:
     *  to the 'ePort ECMP lookup translation' table
     *  (in mode of CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E :
     *  the application is limited to point to the lower half of the 'L2 ECMP' table)
     *  NOTE: for both APIs the CPSS will not check for 'collisions' with 'trunk settings'
     *  3. Choosing this mode also impact 'designated member table' :
     *  As this table is limited to 64 members (64 for UC , and separate 64 for MC),
     *  only the first 64 members of the 'flex' trunk will be represented
     *  in the 'designated member table'.
     *  So only those first 64 ports can 'flood' in vlan (single one per 'flood').
     *  Expectations from Application:
     *  1. Not to 'modify' entries in 'L2 ECMP' table those were 'given' to the
     *  CPSS for trunk management.
     *  2. Not to point from 'ePort ECMP lookup translation' to 'L2 ECMP'
     *  table to those were 'given' to the CPSS for trunk management.
     */
    PRV_CPSS_TRUNK_LOAD_BALANCE_FLEX_E

} PRV_CPSS_TRUNK_LOAD_BALANCE_TYPE_ENT;

/*******************************************************************************
* PRV_CPSS_TRUNK_DB_FLAGS_INIT_FUN
*
* DESCRIPTION:
*       get info about the flags of the device about trunk
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum - device num
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (* PRV_CPSS_TRUNK_DB_FLAGS_INIT_FUN)
(
    IN  GT_U8                           devNum
);

/*******************************************************************************
* PRV_CPSS_TRUNK_PORT_TRUNK_ID_SET_FUNC
*
* DESCRIPTION:
*       Set the trunkId field in the port's control register in the device
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       portId  - the physical port number.
*       memberOfTrunk - is the port associated with the trunk
*                 GT_FALSE - the port is set as "not member" in the trunk
*                 GT_TRUE  - the port is set with the trunkId
*
*       trunkId - the trunk to which the port associate with
*                 This field indicates the trunk group number (ID) to which the
*                 port is a member.
*                 1 through 31 = The port is a member of the trunk
*                 this value relevant only when memberOfTrunk = GT_TRUE
*
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK   - successful completion
*       GT_FAIL - an error occurred.
*       GT_HW_ERROR - on hardware error
*       GT_BAD_PARAM - bad device number , or
*                      bad port number , or
*                      bad trunkId number
*
* COMMENTS:
*
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_TRUNK_PORT_TRUNK_ID_SET_FUNC)
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portId,
    IN GT_BOOL                  memberOfTrunk,
    IN GT_TRUNK_ID              trunkId
);

/*******************************************************************************
* PRV_CPSS_TRUNK_MEMBERS_SET_FUNC
*
* DESCRIPTION:
*       Set the trunk table entry , and set the number of members in it.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       trunkId - trunk id
*       numMembers - num of enabled members in the trunk
*                    Note : value 0 is not recommended.
*       membersArray - array of enabled members of the trunk
*
* OUTPUTS:
*       none.
* RETURNS:
*       GT_OK       - successful completion
*       GT_FAIL     - an error occurred.
*       GT_HW_ERROR - on hardware error
*       GT_OUT_OF_RANGE - numMembers exceed the number of maximum number
*                         of members in trunk (total of 0 - 8 members allowed)
*       GT_BAD_PARAM - bad device number , or
*                      bad trunkId number , or
*                      bad members parameters :
*                          (device & 0x80) != 0  means that the HW can't support
*                                              this value , since HW has 7 bit
*                                              for device number
*                          (port & 0xC0) != 0  means that the HW can't support
*                                              this value , since HW has 6 bit
*                                              for port number
*       GT_BAD_PTR - one of the parameters in NULL pointer
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_TRUNK_MEMBERS_SET_FUNC)
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    IN  GT_U32                  numMembers,
    IN  CPSS_TRUNK_MEMBER_STC   membersArray[]
);


/*******************************************************************************
* Function: PRV_CPSS_TRUNK_NON_MEMBERS_BMP_SET_FUNC
*
* DESCRIPTION:
*       Set the trunk's non-trunk ports specific bitmap entry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       trunkId - trunk id - in this API trunkId can be ZERO !
*       nonTrunkPortsPtr - (pointer to) non trunk port bitmap of the trunk.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK       - successful completion
*       GT_FAIL     - an error occurred.
*       GT_HW_ERROR - on hardware error
*       GT_BAD_PARAM - bad device number , or
*                      bad trunkId number
*       GT_BAD_PTR - one of the parameters in NULL pointer
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_TRUNK_NON_MEMBERS_BMP_SET_FUNC)
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    IN  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
);

/*******************************************************************************
* Function: PRV_CPSS_TRUNK_NON_MEMBERS_BMP_GET_FUNC
*
* DESCRIPTION:
*       Get the trunk's non-trunk ports specific bitmap entry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum  - the device number
*       trunkId - trunk id - in this API trunkId can be ZERO !
*       nonTrunkPortsPtr - (pointer to) non trunk port bitmap of the trunk.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK       - successful completion
*       GT_FAIL     - an error occurred.
*       GT_HW_ERROR - on hardware error
*       GT_BAD_PARAM - bad device number , or
*                      bad trunkId number
*       GT_BAD_PTR - one of the parameters in NULL pointer
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_TRUNK_NON_MEMBERS_BMP_GET_FUNC)
(
    IN  GT_U8               devNum,
    IN  GT_TRUNK_ID         trunkId,
    IN  CPSS_PORTS_BMP_STC  *nonTrunkPortsPtr
);


/*******************************************************************************
* PRV_CPSS_TRUNK_DESIGNATED_MEMBERS_BMP_SET_FUNC
*
* DESCRIPTION:
*       Set the designated trunk table specific entry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - the device number
*       entryIndex      - the index in the designated ports bitmap table
*       designatedPortsPtr - (pointer to) designated ports bitmap
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK       - successful completion
*       GT_FAIL     - an error occurred.
*       GT_HW_ERROR - on hardware error
*       GT_BAD_PARAM - bad device number
*       GT_BAD_PTR - one of the parameters in NULL pointer
*       GT_OUT_OF_RANGE - entryIndex exceed the number of HW table.
*                         the index must be in range (0 - 15)
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_TRUNK_DESIGNATED_MEMBERS_BMP_SET_FUNC)
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    IN  CPSS_PORTS_BMP_STC  *designatedPortsPtr
);

/*******************************************************************************
* PRV_CPSS_TRUNK_DESIGNATED_MEMBERS_BMP_GET_FUNC
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Get the designated trunk table specific entry.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - the device number
*       entryIndex      - the index in the designated ports bitmap table
*
* OUTPUTS:
*       designatedPortsPtr - (pointer to) designated ports bitmap
*
* RETURNS:
*       GT_OK                    - successful completion
*       GT_FAIL                  - an error occurred.
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - bad device number
*       GT_BAD_PTR               - one of the parameters in NULL pointer
*       GT_OUT_OF_RANGE          - entryIndex exceed the number of HW table.
*                         the index must be in range (0 - 7)
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_TRUNK_DESIGNATED_MEMBERS_BMP_GET_FUNC)
(
    IN  GT_U8               devNum,
    IN  GT_U32              entryIndex,
    OUT  CPSS_PORTS_BMP_STC  *designatedPortsPtr
);

/**
* @struct PRV_CPSS_FAMILY_TRUNK_BIND_FUNC_STC
 *
 * @brief A structure to hold common Trunk functions for PP Family needed
 * in CPSS
 * INFO "PER DEVICE FAMILY"
*/
typedef struct{

    /** @brief function that set the specific flags relate to
     *  this device - see info in structure :
     *  PRV_CPSS_TRUNK_DB_INFO_STC
     */
    PRV_CPSS_TRUNK_DB_FLAGS_INIT_FUN dbFlagsInitFunc;

    /** @brief function that set the trunkId in the port control
     *  register -- used in the ingress pipe to assign the
     *  packet as came from port/trunk.
     */
    PRV_CPSS_TRUNK_PORT_TRUNK_ID_SET_FUNC portTrunkIdSetFunc;

    /** @brief function that set the trunk members and the number of
     *  ports in trunk -- used in the ingress pipe to find
     *  the designated port of trunk that the FDB/NH set as
     *  the outgoing interface.
     */
    PRV_CPSS_TRUNK_MEMBERS_SET_FUNC membersSetFunc;

    /** @brief function that set the ports that considered as
     *  "non-trunk" local ports bitmap from the specific
     *  trunk -- used when multi-destination traffic
     *  (flooding) from a trunk will not return to any of
     *  the trunk ports
     */
    PRV_CPSS_TRUNK_NON_MEMBERS_BMP_SET_FUNC nonMembersBmpSetFunc;

    /** @brief function that get the ports that considered as
     *  "non-trunk" local ports bitmap from the specific
     *  trunk
     */
    PRV_CPSS_TRUNK_NON_MEMBERS_BMP_GET_FUNC nonMembersBmpGetFunc;

    /** @brief function that set the designated local
     *  port bitmap for a specific index -- used to assure
     *  that multi-destination traffic (flood) will not
     *  egress from more that one port of trunk (one port
     *  of each trunk in the vlan/vidx).
     */
    PRV_CPSS_TRUNK_DESIGNATED_MEMBERS_BMP_SET_FUNC designatedMembersBmpSetFunc;

    /** @brief function that get the designated local
     *  port bitmap for a specific index -- used to assure
     *  that multi-destination traffic (flood) will not
     *  egress from more that one port of trunk (one port
     *  of each trunk in the vlan/vidx).
     */
    PRV_CPSS_TRUNK_DESIGNATED_MEMBERS_BMP_GET_FUNC designatedMembersBmpGetFunc;

} PRV_CPSS_FAMILY_TRUNK_BIND_FUNC_STC;

/**
* @struct PRV_CPSS_TRUNK_ENTRY_INFO_STC
 *
 * @brief trunk info per trunk -- used for "global trunks"
 * the trunk ports are set in the next order :
 * starting forward from index 0 ---> enable port 1
 * 1 ---> enable port 2
 * ...
 * x ---> enable port x+1
 * total enabled ports are : enabledCount
 * starting forward from index x+1 ---> disabled port 1
 * x+2 ---> disabled port 2
 * ...
 * total disabled ports are : disabledCount
*/
typedef struct{
    CPSS_TRUNK_MEMBER_STC   *membersPtr;
    GT_BOOL                 *isLocalMembersPtr;

    GT_U32  enabledCount;
    GT_U32  disabledCount;

    GT_BOOL                 designatedMemberExists;
    CPSS_TRUNK_MEMBER_STC   designatedMember;
    GT_BOOL                 designatedMemberIsLocal;

    CPSS_TRUNK_TYPE_ENT     type;

    GT_BOOL                 allowMultiDestFloodBack;

    struct{
        GT_U32      myTrunkStartIndexInMembersTable;
        GT_U32      myTrunkMaxNumOfMembers;
    }flexInfo;

}PRV_CPSS_TRUNK_ENTRY_INFO_STC;

/**
* @struct PRV_CPSS_TRUNK_DB_INFO_STC
 *
 * @brief Structure represent the DB of the CPSS for trunks in that device
 * INFO "PER DEVICE"
*/
typedef struct{

    PRV_CPSS_FAMILY_TRUNK_BIND_FUNC_STC *virtualFunctionsPtr;

    /** is this device initialized the trunk */
    GT_BOOL initDone;

    /** @brief the type of load balance behavior to have used for WA
     *  and emulations for load balancing
     */
    PRV_CPSS_TRUNK_LOAD_BALANCE_TYPE_ENT loadBalanceType;

    /** @brief null port number of the device , used as "discard" port
     *  of the trunk .
     */
    GT_PHYSICAL_PORT_NUM nullPort;

    /** mask for validity check on trunk members */
    CPSS_TRUNK_MEMBER_STC validityMask;

    /** indication that validityMask used also for 'per port' APIs. */
    GT_BOOL isPhysicalPortByMask;

    /** @brief do trunk designated table 3 ports
     *  work around
     */
    GT_BOOL doDesignatedTableWorkAround;

    /** number of trunks supported by HW */
    GT_U32 numTrunksSupportedHw;

    /** number of non */
    GT_U32 numNonTrunkEntriesHw;

    /** number of designated trunk entries in HW */
    GT_U32 numOfDesignatedTrunkEntriesHw;

    /** @brief GT_TRUE: table is split between MC & cascade trunk UC
     *  GT_FALSE: table is used for both MC & cascade trunk UC
     */
    GT_BOOL designatedTrunkTableSplit;

    /** number of trunk in localTrunksArray */
    GT_U32 numberOfTrunks;

    /** @brief max number of members in trunk .
     *  the ExMx,DxCh,DxSal :support 8 members.
     *  the ExMxPm device : support 12 members
     *  sip5 devices in flex mode   : support 4K members
     *  sip5 devices in non flex mode : support 'fixed' value
     */
    GT_U32 maxNumMembersInTrunk;

    /** @brief indication that the number of members is
     *  per trunk or per device.
     *  GT_TRUE - each trunk may have different 'max' number of members
     *  in this mode the (numMembersInTrunknumberOfTrunks) is NOT valid !
     *  need to use maxNumMembersInDevice
     *  GT_FALSE - all trunks hold the same 'max' number of members
     *  need to use maxNumMembersInDevice anyway !!!
     */
    GT_BOOL maxNumMembersInTrunk_isPerTrunk;

    /** @brief hold the max number of trunk members (from all needed trunks)
     *  that the device supports.
     */
    GT_U32 maxNumMembersInDevice;

    /** @brief enable/disable 'sorting' of trunk members in the
     *  'trunk members table' and in the 'designated trunk table'
     *  This mode not effect 'cascade trunks' (that not need synchronization between devices)
     *  'sorting enabled' : when the application will add/remove/set members in a trunk
     *  cpss will make sure to set the trunk members into the
     *  device in an order that is not affected by the 'history'
     *  of 'add/remove' members to/from the trunk.
     *  'sorting disabled' : (legacy mode / default mode) when the application
     *  will add/remove/set members in a trunk cpss will set
     *  the trunk members into the device in an order that is
     *  effected by the 'history' of 'add/remove' members
     *  to/from the trunk.
     */
    GT_BOOL sortTrunkMembers;

    /** @brief is the shadow of the 3 tables allocated in CPU memory
     *  valid
     */
    GT_BOOL shadowValid;

    GT_TRUNK_ID portTrunkIdArray[CPSS_MAX_PORTS_NUM_CNS];

    PRV_CPSS_TRUNK_ENTRY_INFO_STC *trunksArray;

    CPSS_TRUNK_MEMBER_STC *allMembersArray;

    GT_BOOL *allMembersIsLocalArray;

    /** bitmap of ports in mode : CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_FORBID_ALL_E. */
    CPSS_PORTS_BMP_STC portsMcForbid;

    /** bitmap of ports in mode : CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_FORCE_ALL_E. */
    CPSS_PORTS_BMP_STC portsMcForce;

    /** @brief bitmap of ports that the application manage the trunkId
     *  Comments:
     */
    CPSS_PORTS_BMP_STC portsTrunkIdByApplication;

} PRV_CPSS_TRUNK_DB_INFO_STC;

/**
* @struct PRV_CPSS_FAMILY_TRUNK_INFO_STC
 *
 * @brief A structure to hold common Trunk PP Family needed in CPSS
 * INFO "PER DEVICE FAMILY"
*/
typedef struct{

    /** @brief functions of trunks (that were bound) common to all PP of a
     *  specific family
     */
    PRV_CPSS_FAMILY_TRUNK_BIND_FUNC_STC boundFunc;

} PRV_CPSS_FAMILY_TRUNK_INFO_STC;

/**
* @internal prvCpssGenericTrunkInit function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         CPSS generic Trunk initialization of PP Tables/registers and
*         SW shadow data structures, all ports are set as non-trunk ports.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] maxNumberOfTrunks        - maximum number of trunk groups.
*                                      when this number is 0 , there will be no shadow
*                                      used.
*                                      Note:
*                                      that means that API's that used shadow will FAIL.
* @param[in] lbType                   - How the CPSS SW will fill the HW with trunk members
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - the numberOfTrunks is more then what HW support
*                                       the Ex1x5, Ex1x6 devices support 31 trunks.
*                                       the Ex6x5, Ex6x8 , Ex6x0 devices support 7 trunks.
* @retval GT_BAD_STATE             - if library already initialized with different number of
*                                       trunks than requested
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkInit
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxNumberOfTrunks,
    IN  PRV_CPSS_TRUNK_LOAD_BALANCE_TYPE_ENT    lbType
);

/**
* @internal prvCpssGenericTrunkDesignatedMemberSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function Configures per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         Setting value replace previously assigned designated member.
*         NOTE that:
*         under normal operation this should not be used with cascade Trunks,
*         due to the fact that in this case it affects all types of traffic -
*         not just Multi-destination as in regular Trunks.
*         Diagram 1 : Regular operation - Traffic distribution on all enabled
*         members (when no specific designated member defined)
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         index \ member %  M1 %  M2  %  M3 % M4 %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 0    %  1  %  0  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 1    %  0  %  1  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 2    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 3    %  0  %  0  %  0  % 1  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 4    %  1  %  0  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 5    %  0  %  1  %  0  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 6    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 7    %  0  %  0  %  0  % 1  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Diagram 2: Traffic distribution once specific designated member defined
*         (M3 in this case - which is currently enabled member in trunk)
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         index \ member %  M1 %  M2  %  M3 % M4 %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 0    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 1    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 2    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 3    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 4    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 5    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 6    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*         Index 7    %  0  %  0  %  1  % 0  %
*         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - the trunk id.
* @param[in] enable                   - enable/disable designated trunk member.
*                                      GT_TRUE -
*                                      1. Clears all current Trunk member's designated bits
*                                      2. If input designated member, is currently an
* @param[in] enable                   at Trunk (in local device) enable its
*                                      bits on all indexes
*                                      3. Store designated member at the DB (new DB parameter
*                                      should be created for that)
*                                      GT_FALSE -
*                                      1. Redistribute current Trunk members bits (on all enabled members)
*                                      2. Clear designated member at  the DB
* @param[in] memberPtr                - (pointer to)the designated member we set to the trunk.
*                                      relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0x80) != 0 means that the HW can't support
*                                       this value , since HW has 7 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDesignatedMemberSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_BOOL                  enable,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal prvCpssGenericTrunkDbDesignatedMemberGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function get Configuration per-trunk the designated member -
*         value should be stored (to DB) even designated member is not currently
*         a member of Trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] trunkId                  - the trunk id.
*
* @param[out] enablePtr                - (pointer to) enable/disable designated trunk member.
*                                      GT_TRUE -
*                                      1. Clears all current Trunk member's designated bits
*                                      2. If input designated member, is currently an
*                                      enabled-member at Trunk (in local device) enable its
*                                      bits on all indexes
*                                      3. Store designated member at the DB (new DB parameter
*                                      should be created for that)
*                                      GT_FALSE -
*                                      1. Redistribute current Trunk members bits (on all enabled members)
*                                      2. Clear designated member at  the DB
* @param[out] memberPtr                - (pointer to) the designated member of the trunk.
*                                      relevant only when enable = GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDbDesignatedMemberGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT GT_BOOL                 *enablePtr,
    OUT CPSS_TRUNK_MEMBER_STC   *memberPtr
);

/**
* @internal prvCpssGenericTrunkMembersSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function set the trunk with the specified enable and disabled
*         members.
*         this setting override the previous setting of the trunk members.
*         the user can "invalidate/unset" trunk entry by setting :
*         numOfEnabledMembers = 0 and numOfDisabledMembers = 0
*         This function support next "set entry" options :
*         1. "reset" the entry
*         function will remove the previous settings
*         2. set entry after the entry was empty
*         function will set new settings
*         3. set entry with the same members that it is already hold
*         function will rewrite the HW entries as it was
*         4. set entry with different setting then previous setting
*         a. function will remove the previous settings
*         b. function will set new settings
*         Notes about designated trunk table:
*         If (no designated defined)
*         distribute MC/Cascade trunk traffic among the enabled members
*         else
*         1. Set all member ports bits with 0
*         2. If designated member is one of the enabled members, set its relevant
*         bits on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] enabledMembersArray[]    - (array of) members to set in this trunk as enabled
*                                      members .
*                                      (this parameter ignored if numOfEnabledMembers = 0)
* @param[in] numOfEnabledMembers      - number of enabled members in the array.
* @param[in] disabledMembersArray[]   - (array of) members to set in this trunk as enabled
*                                      members .
*                                      (this parameter ignored if numOfDisabledMembers = 0)
* @param[in] numOfDisabledMembers     - number of enabled members in the array.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when the sum of number of enabled members + number of
*                                       disabled members exceed the number of maximum number
* @retval of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0x80) != 0 means that the HW can't support
*                                       this value , since HW has 7 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvCpssGenericTrunkMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
);

/**
* @internal prvCpssGenericTrunkCascadeTrunkPortsSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the 'cascade' trunk with the specified 'Local ports'
*         overriding any previous setting.
*         The cascade trunk may be invalidated/unset by portsMembersPtr = NULL.
*         Local ports are ports of only configured device.
*         This functions sets the trunk-type according to :
*         portsMembersPtr = NULL --> CPSS_TRUNK_TYPE_FREE_E
*         otherwise --> CPSS_TRUNK_TYPE_CASCADE_E
*         Cascade trunk is:
*         - members are ports of only configured device pointed by devNum
*         - trunk members table is empty (see cpssDxChTrunkTableEntrySet)
*         Therefore it cannot be used as target by ingress engines like FDB,
*         Router, TTI, Ingress PCL and so on.
*         - members ports trunk ID are set (see cpssDxChTrunkPortTrunkIdSet).
*         Therefore packets ingresses in member ports are associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] portsMembersPtr          - (pointer to) local ports bitmap to be members of the
*                                      cascade trunk.
*                                      NULL - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      not-NULL - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are ports in the bitmap that not supported by
*                                       the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or number
*                                       of ports (in the bitmap) larger then the number of
*                                       entries in the 'Designated trunk table'
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using cpssDxChTrunkMembersSet(...)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function does not set ports are 'Cascade ports' (and also not
*       check that ports are 'cascade').
*       2. This function sets only next tables :
*       a. the designated trunk table:
*       distribute MC/Cascade trunk traffic among the members
*       b. the 'Non-trunk' table entry.
*       c. 'per port' trunkId
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       5. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
*       cpssDxChTrunkMembersSet ,
*       cpssDxChTrunkMemberAdd , cpssDxChTrunkMemberRemove,
*       cpssDxChTrunkMemberEnable , cpssDxChTrunkMemberDisable
*       cpssDxChTrunkDbEnabledMembersGet , cpssDxChTrunkDbDisabledMembersGet
*       cpssDxChTrunkDesignatedMemberSet , cpssDxChTrunkDbDesignatedMemberGet
*       Comparing the 2 function :
*       cpssDxChTrunkCascadeTrunkPortsSet  |  cpssDxChTrunkMembersSet
*       ----------------------------------------------------------------------------
*       1. purpose 'Cascade trunk'        | 1. purpose 'Network trunk' , and
*       |  also 'Cascade trunk' with up to
*       |  8 members
*       ----------------------------------------------------------------------------
*       2. supported number of members depends  | 2. supports up to 8 members
*       on number of entries in the      |  (also in Lion).
*       'Designated trunk table'       |
*       -- Lion supports 64 entries (so up to |
*       64 ports in the 'Cascade trunk'). |
*       -- all other devices supports 8    |
*       entries (so up to 8 ports in the  |
*       'Cascade trunk').         |
*       ----------------------------------------------------------------------------
*       3. manipulate next trunk tables :    | 3. manipulate all trunk tables :
*       'Per port' trunk-id ,         | 'Per port' trunk-id , 'Trunk members',
*       'Non-trunk' , 'Designated trunk'   | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*
*/
GT_STATUS prvCpssGenericTrunkCascadeTrunkPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr
);

/**
* @internal prvCpssGenericTrunkCascadeTrunkPortsGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Gets the 'Local ports' of the 'cascade' trunk .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
*
* @param[out] portsMembersPtr          - (pointer to) local ports bitmap of the cascade trunk.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkCascadeTrunkPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT CPSS_PORTS_BMP_STC       *portsMembersPtr
);

/**
* @internal prvCpssGenericTrunkCascadeTrunkWithWeightedPortsSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the 'cascade' trunk with the Weighted specified 'Local ports'
*         overriding any previous setting.
*         the weights effect the number of times that each member will get representation
*         in the 'designated table' .
*         The cascade trunk may be invalidated by numOfMembers = 0.
*         Local ports are ports of only configured device.
*         Cascade trunk is:
*         - members are ports of only configured device pointed by devNum
*         - trunk members table is empty (see prvCpssGenericTrunkTableEntrySet)
*         Therefore it cannot be used as target by ingress engines like FDB,
*         Router, TTI, Ingress PCL and so on.
*         - members ports trunk ID are set (see prvCpssGenericTrunkPortTrunkIdSet).
*         Therefore packets ingresses in member ports are associated with trunk
*         - all members are enabled only and cannot be disabled.
*         - may be used for cascade traffic and pointed by the 'Device map table'
*         as the local target to reach to the 'Remote device'.
*         (For 'Device map table' refer to cpssDxChCscdDevMapTableSet(...))
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  ExMxPm; ExMx.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] numOfMembers             - number of members in the array.
*                                      value 0 - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
*                                      value != 0 - meaning that the trunk-type will be : CPSS_TRUNK_TYPE_CASCADE_E
* @param[in] weightedMembersArray[]   - (array of) members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - there are members that not supported by the device.
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number , or bad total
*                                       weights (see restrictions below)
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
*                                       or this trunk hold members defined using prvCpssGenericTrunkMembersSet(...)
* @retval GT_BAD_PTR               - when numOfMembers != 0 and weightedMembersArray = NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function does not set ports as 'Cascade ports' (and also not
*       check that ports are 'cascade').
*       2. This function sets only next tables :
*       a. the designated trunk table:
*       distribute MC/Cascade trunk traffic among the members according to their weight
*       b. the 'Non-trunk' table entry.
*       c. 'per port' trunkId
*       3. because this function not set the 'Trunk members' table entry , the
*       application should not point to this trunk from any ingress unit , like:
*       FDB , PCL action redirect , NH , TTI action redirect , PVE ...
*       (it should be pointed ONLY from the device map table)
*       4. this API supports only trunks with types : CPSS_TRUNK_TYPE_FREE_E or
*       CPSS_TRUNK_TYPE_CASCADE_E.
*       5. next APIs are not supported from trunk with type : CPSS_TRUNK_TYPE_CASCADE_E
*       prvCpssGenericTrunkMembersSet ,
*       prvCpssGenericTrunkMemberAdd , prvCpssGenericTrunkMemberRemove,
*       prvCpssGenericTrunkMemberEnable , prvCpssGenericTrunkMemberDisable
*       prvCpssGenericTrunkDbEnabledMembersGet , prvCpssGenericTrunkDbDisabledMembersGet
*       prvCpssGenericTrunkDesignatedMemberSet , prvCpssGenericTrunkDbDesignatedMemberGet
*       6. the total weights of all the trunk members is restricted to :
*       a. must be equal to power of 2 (1,2,4,8,16,32,64...)
*       b. must not be larger then actual designated table size of the device.
*       (Lion ,Lion2 : 64 , other devices : 8)
*       c. not relevant when single member exists
*       Comparing the 2 function :
*       prvCpssGenericTrunkCascadeTrunkWithWeightedPortsSet |  prvCpssGenericTrunkMembersSet
*       ----------------------------------------------------------------------------
*       1. purpose 'Cascade trunk'        | 1. purpose 'Network trunk' , and
*       |  also 'Cascade trunk' with up to
*       |  8 members
*       ----------------------------------------------------------------------------
*       2. supported number of members depends  | 2. supports up to 8 members
*       on number of entries in the      |  (also in Lion).
*       'Designated trunk table'       |
*       -- Lion supports 64 entries (so up to |
*       64 ports in the 'Cascade trunk'). |
*       -- all other devices supports 8    |
*       entries (so up to 8 ports in the  |
*       'Cascade trunk').         |
*       ----------------------------------------------------------------------------
*       3. manipulate next trunk tables :    | 3. manipulate all trunk tables :
*       'Per port' trunk-id          | 'Per port' trunk-id , 'Trunk members',
*       'Non-trunk' , 'Designated trunk'   | 'Non-trunk' , 'Designated trunk' tables.
*       ----------------------------------------------------------------------------
*       4. ingress unit must not point to this  | 4. no restriction on ingress/egress
*       trunk (because 'Trunk members' entry |  units.
*       hold no ports)            |
*
*/
GT_STATUS prvCpssGenericTrunkCascadeTrunkWithWeightedPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfMembers,
    IN CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
);



/**
* @internal prvCpssGenericTrunkCascadeTrunkWithWeightedPortsGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         This function gets the Weighted 'Local ports' of 'cascade' trunk .
*         the weights reflect the number of times that each member is represented
*         in the 'designated table'
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  ExMxPm; ExMx.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in,out] numOfMembersPtr          - (pointer to) max num of members to retrieve - this value refer to the number of
*                                      members that the array of weightedMembersArray[] can retrieve.
*                                      OUTPUTS:
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*                                      (up to : Lion,Lion2 : 64 , others : 8)
* @param[in] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
*                                      (up to : Lion,Lion2 : 64 , others : 8)
*
* @param[out] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
*                                      OUTPUTS:
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_CASCADE_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkCascadeTrunkWithWeightedPortsGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                   *numOfMembersPtr,
    OUT CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
);


/**
* @internal prvCpssGenericTrunkMemberAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function add member to the trunk in the device.
*         If member is already in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members,
*         now taking into account also the added member
*         else
*         1. If added member is not the designated member - set its relevant bits to 0
*         2. If added member is the designated member & it's enabled,
*         set its relevant bits on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to add member to the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to add to the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0x80) != 0 means that the HW can't support
*                                       this value , since HW has 7 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
* @retval GT_FULL                  - trunk already contains maximum supported members
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal prvCpssGenericTrunkMemberRemove function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function remove member from a trunk in the device.
*         If member not exists in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the removed member
*         else
*         1. If removed member is not the designated member - nothing to do
*         2. If removed member is the designated member set its relevant bits
*         on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to remove member from the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to remove from the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0x80) != 0 means that the HW can't support
*                                       this value , since HW has 7 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkMemberRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal prvCpssGenericTrunkMemberDisable function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function disable (enabled)existing member of trunk in the device.
*         If member is already disabled in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the disabled member
*         else
*         1. If disabled member is not the designated member - set its relevant bits to 0
*         2. If disabled member is the designated member set its relevant bits
*         on all indexes to 0.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to disable member in the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to disable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0x80) != 0 means that the HW can't support
*                                       this value , since HW has 7 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkMemberDisable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal prvCpssGenericTrunkMemberEnable function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function enable (disabled)existing member of trunk in the device.
*         If member is already enabled in this trunk , function do nothing and
*         return GT_OK.
*         Notes about designated trunk table:
*         If (no designated defined)
*         re-distribute MC/Cascade trunk traffic among the enabled members -
*         now taking into account also the enabled member
*         else
*         1. If enabled member is not the designated member - set its relevant bits to 0
*         2. If enabled member is the designated member set its relevant bits
*         on all indexes to 1.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which to enable member in the trunk
* @param[in] trunkId                  - the trunk id.
* @param[in] memberPtr                - (pointer to)the member to enable in the trunk.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0x80) != 0 means that the HW can't support
*                                       this value , since HW has 7 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkMemberEnable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
* @internal prvCpssGenericTrunkNonTrunkPortsAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         add the ports to the trunk's non-trunk entry .
*         NOTE : the ports are add to the "non trunk" table only and not effect
*         other trunk relate tables/registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsBmpPtr      - (pointer to)bitmap of ports to add to
*                                      "non-trunk members"
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkNonTrunkPortsAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
);

/**
* @internal prvCpssGenericTrunkNonTrunkPortsRemove function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Removes the ports from the trunk's non-trunk entry .
*         NOTE : the ports are removed from the "non trunk" table only and not
*         effect other trunk relate tables/registers.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id - in this API  can be ZERO !
* @param[in] nonTrunkPortsBmpPtr      - (pointer to)bitmap of ports to remove from
*                                      "non-trunk members"
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkNonTrunkPortsRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *nonTrunkPortsBmpPtr
);


/**
* @internal prvCpssGenericTrunkDbEnabledMembersGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         return the enabled members of the trunk
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in,out] numOfEnabledMembersPtr   - (pointer to) max num of enabled members to
*                                      retrieve - this value refer to the number of
*                                      members that the array of enabledMembersArray[]
*                                      can retrieve.
* @param[in,out] numOfEnabledMembersPtr   - (pointer to) the actual num of enabled members
*                                      in the trunk (up to max number supported by the PP)
*
* @param[out] enabledMembersArray[]    - (array of) enabled members of the trunk
*                                      array was allocated by the caller
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDbEnabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfEnabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   enabledMembersArray[]
);

/**
* @internal prvCpssGenericTrunkDbDisabledMembersGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         return the disabled members of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in,out] numOfDisabledMembersPtr  - (pointer to) max num of disabled members to
*                                      retrieve - this value refer to the number of
*                                      members that the array of enabledMembersArray[]
*                                      can retrieve.
* @param[in,out] numOfDisabledMembersPtr  -(pointer to) the actual num of disabled members
*                                      in the trunk (up to max number supported by the PP)
*
* @param[out] disabledMembersArray[]   - (array of) disabled members of the trunk
*                                      array was allocated by the caller
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDbDisabledMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                *numOfDisabledMembersPtr,
    OUT CPSS_TRUNK_MEMBER_STC   disabledMembersArray[]
);

/**
* @internal prvCpssGenericTrunkDbIsMemberOfTrunk function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Checks if a member (device,port) is a trunk member.
*         if it is trunk member the function retrieve the trunkId of the trunk.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] memberPtr                - (pointer to) the member to check if is trunk member
*
* @param[out] trunkIdPtr               - (pointer to) trunk id of the port .
*                                      this pointer allocated by the caller.
*                                      this can be NULL pointer if the caller not require the
*                                      trunkId(only wanted to know that the member belongs to a
*                                      trunk)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number
* @retval GT_NOT_FOUND             - the pair (devNum,port) not a trunk member
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDbIsMemberOfTrunk
(
    IN  GT_U8                   devNum,
    IN  CPSS_TRUNK_MEMBER_STC   *memberPtr,
    OUT GT_TRUNK_ID             *trunkIdPtr
);

/**
* @internal prvCpssGenericTrunkDbTrunkTypeGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Get the trunk type.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] trunkId                  - the trunk id.
*
* @param[out] typePtr                  - (pointer to) the trunk type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDbTrunkTypeGet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId,
    OUT CPSS_TRUNK_TYPE_ENT     *typePtr
);

/**
* @internal prvCpssGenericTrunkMcLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Enable/Disable sending multi-destination packets back to its source
*         trunk on the local device.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
* @param[in] enable                   - Boolean value:
*                                      GT_TRUE  - multi-destination packets may be sent back to
*                                      their source trunk on the local device.
*                                      GT_FALSE - multi-destination packets are not sent back to
*                                      their source trunk on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. the behavior of multi-destination traffic ingress from trunk is
*       not-affected by setting of cpssDxChBrgVlanLocalSwitchingEnableSet
*       and not-affected by setting of cpssDxChBrgPortEgressMcastLocalEnable
*       2. the functionality manipulates the 'non-trunk' table entry of the trunkId
*
*/
GT_STATUS prvCpssGenericTrunkMcLocalSwitchingEnableSet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    IN GT_BOOL          enable
);

/**
* @internal prvCpssGenericTrunkDbMcLocalSwitchingEnableGet function
* @endinternal
*
* @brief   Get the current status of Enable/Disable sending multi-destination packets
*         back to its source trunk on the local device.
*         function uses the DB (no HW operations)
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; ExMxPm; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number .
* @param[in] trunkId                  - the trunk id.
*
* @param[out] enablePtr                - (pointer to) Boolean value:
*                                      GT_TRUE  - multi-destination packets may be sent back to
*                                      their source trunk on the local device.
*                                      GT_FALSE - multi-destination packets are not sent back to
*                                      their source trunk on the local device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId number
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDbMcLocalSwitchingEnableGet
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId,
    OUT GT_BOOL          *enablePtr
);

/**
* @internal prvCpssGenericTrunkPortMcEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the mode how multicast destination traffic egress the
*         selected port while it is a trunk member.
*         see details in description of CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_E
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] mode                     - the 'designated mc table'  of the port
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device or port number or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If disabled multicast destination packets would not egress through the
*       configured port.
*       This configuration has no influence on unicast destination traffic or
*       when the port is not a trunk member.
*
*/
GT_STATUS prvCpssGenericTrunkPortMcEnableSet
(
    IN GT_U8                  devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT mode
);

/**
* @internal prvCpssGenericTrunkDbPortMcEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function gets the mode how multicast destination traffic egress the
*         selected port while it is a trunk member.
*         see details in description of CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] modePtr                  - (pointer to) the 'designated mc table' mode of the port
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device or port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function output is retrieved from CPSS related trunk DB.
*       This configuration has no influence on unicast destination traffic or
*       when the port is not a trunk member.
*
*/
GT_STATUS prvCpssGenericTrunkDbPortMcEnableGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT  *modePtr
);

/**
* @internal prvCpssGenericTrunkDbPortTrunkIdModeSet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Function allow application to be responsible for the 'TrunkId of the port'.
*         in High level mode APIs the 'TrunkId of the port' is set internally be
*         the CPSS and may override the current value of the 'TrunkId of the port'.
*         This API allows application to notify the CPSS to not manage the 'TrunkId
*         of the port'.
*         function cpssDxChTrunkPortTrunkIdSet(...) allow application to manage
*         the trunkId of the port
*         NOTE: this is 'DB' operation (database) , without HW access.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
* @param[in] manageMode               - the management mode.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or manageMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDbPortTrunkIdModeSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT manageMode
);

/**
* @internal prvCpssGenericTrunkDbPortTrunkIdModeGet function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Function get the 'management mode' of the port's trunkId.
*         NOTE: this is 'DB' operation (database) , without HW access.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical port number.
*
* @param[out] manageModePtr            - (pointer to) the management mode.
*
* @retval GT_OK                    - successful completion
* @retval GT_FAIL                  - an error occurred.
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad port number , or manageMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDbPortTrunkIdModeGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT *manageModePtr
);


/**
* @internal prvCpssGenericTrunkDbIsValid function
* @endinternal
*
* @brief   Function Relevant mode : High level mode
*         Function check if the trunkId is valid.
*         NOTE: this is 'DB' operation (database) , without HW access.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*                                      portNum  - physical port number.
*
* @retval GT_OK                    - the trunk is valid.
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_BAD_PARAM             - bad device number , or bad trunkId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDbIsValid
(
    IN GT_U8            devNum,
    IN GT_TRUNK_ID      trunkId
);

/**
* @internal prvCpssGenericTrunkDestroy function
* @endinternal
*
* @brief   Function Relevant mode : All modes
*         CPSS DxCh Trunk Destroy all relevant DB info.
*         No HW modifications are done.
*         Purpose :
*         Debug tool to allow to call cpssDxChTrunkInit(...) again with any new valid parameters.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenericTrunkDestroy
(
    IN  GT_U8                                devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssTrunkTypesh */


