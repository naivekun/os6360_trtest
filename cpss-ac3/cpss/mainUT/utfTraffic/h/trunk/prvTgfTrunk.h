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
* @file prvTgfTrunk.h
*
* @brief trunk testing
*
* @version   15
********************************************************************************
*/
#ifndef __prvTgfTrunk
#define __prvTgfTrunk

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/trunk/cpssGenTrunkTypes.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <common/tgfPclGen.h>

/**
* @enum PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT
 *
 * @brief enumerator for expected load balance modes , the mode that we expect the
 * traffic that was sent to the trunk will egress from it's ports.
*/
typedef enum{

    /** @brief the traffic expected to
     *  egress only single port in the trunk (any port)
     */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E,

    /** @brief the traffic expected to
     *  egress from all ports in trunk , with EVEN values (as much as possible)
     *  by '%' operator
     */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EVEN_E,

    /** @brief the traffic not
     *  expected to egress the trunk ports !
     */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_EMPTY_E,

    /** @brief the traffic
     *  expected to egress ONLY from specific ports in trunk , with EVEN values
     *  (as much as possible) by '%' operator.
     *  the specific members are given by prvTgfTrunkLoadBalanceSpecificMembersSet(...)
     */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E,

    /** @brief the traffic
     *  expected to egress ONLY from limited number of ports in trunk , with EVEN values
     *  (as much as possible) by '%' operator.
     *  the specific number of limited members are given by prvTgfTrunkLoadBalanceLimitedNumSet(...)
     */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E,

    /** @brief the traffic
     *  expected to do load balance according to the proportional weight of each
     *  member in the trunk.
     *  to 'set' and 'get' the weighted members of the trunk use :
     *  prvTgfTrunkWithWeightedMembersSet , prvTgfTrunkWithWeightedMembersGet
     */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_WEIGHTED_MEMBERS_E,

    /** @brief the LBH on the
     *  trunk ports can not be predicted in general , but we need to check
     *  that the trunk got all the traffic that was destined to it.
     */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_CHECK_ONLY_TOTAL_E,

    /** @brief the traffic used the 'designated table' (for multidestination and
     *  for cascade trunk ) for selecting the trunk member .
     *  because the 'designated table' is 8 or 64 entries , the LBH may be
     *  'Not even'.
     *  for example when 7 members in the trunk , member in index 0 is also in index 7 (2 times)
     *  but all other members are only 1 time in the table so LBH will be :
     *  member 0 - 25%
     *  members 1..7 - 12.5%
     */
    PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_MULTI_DESTINATION_EVEN_E

} PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT;

/* indication that test that was written for 'trunk' will run with 'global eport that represents the trunk' */
extern GT_U32   tgfTrunkGlobalEPortRepresentTrunk;
/**
* @internal prvTgfTrunkWithWeightedMembersSet function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         This function sets the trunk with the Weighted specified members
*         overriding any previous setting.
*         the weights effect the number of times that each member will get representation
*         in the 'trunk table' (regular trunks) and in 'designated table'(regular trunks and cascade trunks) .
*         The trunk may be invalidated by numOfMembers = 0.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] numOfMembers             - number of members in the array.
*                                      value 0 - meaning that the trunk-id is 'invalidated' and
*                                      trunk-type will be changed to : CPSS_TRUNK_TYPE_FREE_E
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
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk ,
* @retval GT_BAD_PTR               - when numOfMembers != 0 and weightedMembersArray = NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfTrunkWithWeightedMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfMembers,
    IN CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
);

/**
* @internal prvTgfTrunkWithWeightedMembersGet function
* @endinternal
*
* @brief   Function Relevant mode : ALL modes
*         This function gets the Weighted members of trunk .
*         the weights reflect the number of times that each member is represented
*         in the 'trunk table' (regular trunks) and in 'designated table'(regular trunks and cascade trunks) .
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in,out] numOfMembersPtr          - (pointer to) max num of members to retrieve - this value refer to the number of
*                                      members that the array of weightedMembersArray[] can retrieve.
*                                      OUTPUTS:
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
* @param[in] weightedMembersArray[]   - (array of) members that are members of the cascade trunk.
*                                      each member hold relative weight (relative to Weight of all members)
* @param[in,out] numOfMembersPtr          - (pointer to) the actual num of members in the trunk
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
GT_STATUS prvTgfTrunkWithWeightedMembersGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    INOUT GT_U32                   *numOfMembersPtr,
    OUT CPSS_TRUNK_WEIGHTED_MEMBER_STC       weightedMembersArray[]
);



/**
* @internal prvTgfTrunkForHashTestConfigure function
* @endinternal
*
* @brief   This function configures trunk of two ports
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfTrunkForHashTestConfigure
(
    GT_VOID
);

/**
* @internal prvTgfTrunkForHashTestConfigurationReset function
* @endinternal
*
* @brief   This function resets configuration of trunk
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfTrunkForHashTestConfigurationReset
(
    GT_VOID
);

/**
* @internal prvTgfTrunkHashTestTrafficMplsModeEnableSet function
* @endinternal
*
* @brief   Test for prvTgfTrunkHashMplsModeEnableSet;
*         Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets from both trunk ports
* @param[in] L3L4MplsParsingEnable    -  flag that state if the <enable L3 L4 parsing over MPLS> is enabled/disabled
*                                      when enabled and IPvX over MPLS packet is doing trunk LBH according to L3/L4 of the
*                                      passenger (regardless to tunnel termination)
*                                       None
*/
GT_VOID prvTgfTrunkHashTestTrafficMplsModeEnableSet
(
    IN  GT_BOOL     L3L4MplsParsingEnable
);

/**
* @internal prvTgfTrunkHashTestTrafficMaskSet function
* @endinternal
*
* @brief   Test for prvTgfTrunkHashMaskSet;
*         Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets from both trunk ports
*/
GT_VOID prvTgfTrunkHashTestTrafficMaskSet
(
    GT_VOID
);

/**
* @internal prvTgfTrunkHashTestTrafficIpShiftSet function
* @endinternal
*
* @brief   Test for prvTgfTrunkHashIpShiftSet;
*         Generate traffic:
*         Send to device's port given packet:
*         Check amount of egressed packets from both trunk ports
*/
GT_VOID prvTgfTrunkHashTestTrafficIpShiftSet
(
    GT_VOID
);


/**
* @internal prvTgfTrunkDesignatedPorts function
* @endinternal
*
* @brief   Test for designated member in trunk.
*/
GT_VOID prvTgfTrunkDesignatedPorts
(
    GT_VOID
);

/**
* @internal prvTgfTrunkAndVlanTestConfigure function
* @endinternal
*
* @brief   This function configures trunk and vlan of two ports
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfTrunkAndVlanTestConfigure
(
    GT_VOID
);

/**
* @internal prvTgfTrunkToCpuTraffic function
* @endinternal
*
* @brief   Generate and test traffic
*/
GT_VOID prvTgfTrunkToCpuTraffic
(
    GT_VOID
);

/**
* @internal prvTgfTrunkAndVlanTestConfigurationReset function
* @endinternal
*
* @brief   This function resets configuration of trunk and vlan
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - trunk library was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - some values are out of range
* @retval GT_BAD_PARAM             - on illegal parameters
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
*/
GT_STATUS prvTgfTrunkAndVlanTestConfigurationReset
(
    GT_VOID
);

/**
* @internal prvTgfTrunkLoadBalanceCheck function
* @endinternal
*
* @brief   check that total summary of counters of ports that belong to trunk got value
*         of numPacketSent.
* @param[in] trunkId                  - trunk Id that his ports should received the traffic and on
*                                      them we expect total of numPacketSent packets
* @param[in] mode                     - the  of expected load balance .
*                                      all traffic expected to be received from single port or with
*                                      even distribution on the trunk ports
* @param[in] numPacketSent            - number of packet that we expect the trunk to receive
* @param[in] tolerance                - relevant to PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_SINGLE_PORT_E
*                                      state the number of packet that may get to other port in the trunk.
*                                      meaning that (numPacketSent - tolerance) should get to port A
*                                      and tolerance should get to port B , but other ports (in trunk)
*                                      should not get it.
* @param[in] trunkMemberSenderPtr     - when not NULL , indicates that the traffic to check
*                                      LBH that egress the trunk was originally INGRESSED from the trunk.
*                                      but since in the 'enhanced UT' the CPU send traffic to a port
*                                      due to loopback it returns to it, we need to ensure that the
*                                      member mentioned here should get the traffic since it is the
*                                      'original sender'
*
* @param[out] stormingDetectedPtr      - when not NULL , used to check if storming detected.
*                                      relevant only when trunkMemberSenderPtr != NULL
*                                       None
*/
void prvTgfTrunkLoadBalanceCheck
(
    IN GT_TRUNK_ID  trunkId,
    IN PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ENT  mode,
    IN GT_U32       numPacketSent,
    IN GT_U32       tolerance,
    IN CPSS_TRUNK_MEMBER_STC *trunkMemberSenderPtr,
    OUT GT_BOOL     *stormingDetectedPtr
);

/**
* @internal prvTgfTrunkLoadBalanceSpecificMembersSet function
* @endinternal
*
* @brief   set the specific members of a trunk that will get traffic when the LBH
*         expected mode is : PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_ONLY_SPECIFIC_MEMBERS_E.
* @param[in] membersIndexesBmp        - bmp of indexes in the arrays of :
*                                      prvTgfDevsArray[] , prvTgfPortsArray[]
*                                      the traffic should do LBH in those ports and not in other ports of
*                                      the trunk.
*                                      WARNING : this is NOT a good practice to set the system to get this behavior
*                                       None
*/
void prvTgfTrunkLoadBalanceSpecificMembersSet
(
    IN GT_U32   membersIndexesBmp
);

/**
* @internal prvTgfTrunkLoadBalanceLimitedNumSet function
* @endinternal
*
* @brief   set the limited number of members of a trunk that will get traffic when the LBH
*         expected mode is : PRV_TGF_TRUNK_EXPECTED_LOAD_BALANCE_MODE_LIMITED_MEMBERS_E.
* @param[in] numOfLimitedMembers      - limited number of members of a trunk
*                                      WARNING : this is NOT a good practice to set the system to get this behavior
*                                       None
*/
void prvTgfTrunkLoadBalanceLimitedNumSet
(
    IN GT_U32   numOfLimitedMembers
);




/**
* @internal prvTgfTrunkCrcHashTest function
* @endinternal
*
* @brief   check CRC hash feature.
*         applicable devices: Lion and above
* @param[in] L3L4ParsingOverMplsEnabled - check CRC hash L3L4 for traffic over MPLS.
*                                      or test the CRC hash for all traffic types.
*                                      GT_TRUE - tests CRC for L3L4 traffic over MPLS.
*                                      GT_FALSE - test CRC for all traffic types (not include L3L4 over MPLS)
*                                       None
*/
void prvTgfTrunkCrcHashTest
(
    IN GT_BOOL          L3L4ParsingOverMplsEnabled
);

/**
* @internal prvTgfTrunkCrcHashTestInit function
* @endinternal
*
* @brief   init for CRC hash feature test
*         applicable devices: Lion and above
* @param[in] L3L4ParsingOverMplsEnabled - check CRC hash L3L4 for traffic over MPLS.
*                                      or test the CRC hash for all traffic types.
*                                      GT_TRUE - tests CRC for L3L4 traffic over MPLS.
*                                      GT_FALSE - test CRC for all traffic types (not include L3L4 over MPLS)
*                                       None
*/
void prvTgfTrunkCrcHashTestInit
(
    IN GT_BOOL          L3L4ParsingOverMplsEnabled
);

/**
* @internal prvTgfTrunkCrcHashTestRestore function
* @endinternal
*
* @brief   restore for CRC hash feature test
*         applicable devices: Lion and above
*/
void prvTgfTrunkCrcHashTestRestore
(
    void
);


/**
* @internal prvTgfTrunkCrcHashMaskPriorityTest function
* @endinternal
*
* @brief   check CRC hash mask priority:
*         default of accessing the CRC mask hash table is :
*         'Traffic type' --> index 16..27
*         higher priority for the 'per port' --> index 0..15
*         highest priority to the TTI action --> index 1..15 (without index 0)
*         applicable devices: Lion and above
*/
void prvTgfTrunkCrcHashMaskPriorityTest
(
    void
);

/**
* @internal tgfTrunkCascadeTrunkTestInit function
* @endinternal
*
* @brief   init for cascade trunks test
*         applicable devices: ALL DxCh devices
* @param[in] useWeightedPorts         - use weighted ports
*                                       None
*/
void tgfTrunkCascadeTrunkTestInit
(
    IN  GT_BOOL     useWeightedPorts
);


/**
* @internal tgfTrunkCascadeTrunkTestRestore function
* @endinternal
*
* @brief   restore values after cascade trunks test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkCascadeTrunkTestRestore
(
    void
);


/**
* @internal tgfTrunkCascadeTrunkTest function
* @endinternal
*
* @brief   The cascade trunks test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkCascadeTrunkTest
(
    void
);

/**
* @internal tgfTrunkDesignatedTableModesTestInit function
* @endinternal
*
* @brief   init for designated table modes test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkDesignatedTableModesTestInit
(
    void
);


/**
* @internal tgfTrunkDesignatedTableModesTestRestore function
* @endinternal
*
* @brief   restore values after designated table modes test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkDesignatedTableModesTestRestore
(
    void
);


/**
* @internal tgfTrunkDesignatedTableModesTest function
* @endinternal
*
* @brief   The designated table modes test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkDesignatedTableModesTest
(
    void
);


/**
* @internal tgfTrunkMcLocalSwitchingTestInit function
* @endinternal
*
* @brief   init for trunk multi-destination local switching test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkMcLocalSwitchingTestInit
(
    void
);


/**
* @internal tgfTrunkMcLocalSwitchingTestRestore function
* @endinternal
*
* @brief   restore values after trunk multi-destination local switching test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkMcLocalSwitchingTestRestore
(
    void
);


/**
* @internal tgfTrunkMcLocalSwitchingTest function
* @endinternal
*
* @brief   The trunk multi-destination local switching test
*         applicable devices: ALL DxCh devices
*         The test:
*         a. set trunk with 2 ports
*         b. send flooding from it  --> check no flood back
*         c. 'disable filter' on the trunk
*         d. send flooding from it  --> check flood back to only one of the 2 ports of the trunk
*         e. add port to the trunk
*         f. send flooding from it  --> check flood back to only one of the 3 ports of the trunk
*         g. 'enable filter' on the trunk
*         h. send flooding from it  --> check no flood back
*         i. 'disable filter' on the trunk
*         j. send flooding from it  --> check flood back to only one of the 3 ports of the trunk
*         k. remove port from the trunk
*         l. send flooding from it  --> check flood back to only one of the 2 ports of the trunk, check that the removed port get flood.
*         m. 'enable filter' on the trunk
*/
void tgfTrunkMcLocalSwitchingTest
(
    void
);

/**
* @internal tgfTrunkBasicIpv4UcRoutingTestInit function
* @endinternal
*
* @brief   Init for IPv4 routing with trunk(s)
*         applicable devices: ALL
* @param[in] senderIsTrunk            - send is trunk ?
*                                      GT_FALSE - the sender is port (index 0)
*                                      GT_FALSE - the sender is trunk (trunk A)
* @param[in] nextHopIsTrunk           - next hop is trunk ?
*                                      GT_FALSE - the NH is port (index 0)
*                                      GT_FALSE - the NH is trunk (trunk B)
* @param[in] useSameTrunk             - trunk A and trunk B are the same trunk
*                                      relevant when   senderIsTrunk = GT_TRUE and nextHopIsTrunk = GT_TRUE
*                                       None
*/
void tgfTrunkBasicIpv4UcRoutingTestInit
(
    GT_BOOL     senderIsTrunk,
    GT_BOOL     nextHopIsTrunk,
    GT_BOOL     useSameTrunk
);

/**
* @internal tgfTrunkBasicIpv4UcRoutingTestRestore function
* @endinternal
*
* @brief   restore trunk configuration relate to IPv4 routing with trunk(s)
*         applicable devices: ALL
*/
void tgfTrunkBasicIpv4UcRoutingTestRestore
(
    void
);
/**
* @internal prvTgfTrunkSimpleHashMacSaDaTestInit function
* @endinternal
*
* @brief   Init for Simple hash for mac SA,DA LBH
*         applicable devices: All DX , All Puma
*/
void prvTgfTrunkSimpleHashMacSaDaTestInit
(
    void
);

/**
* @internal prvTgfTrunkSimpleHashMacSaDaTestRestore function
* @endinternal
*
* @brief   restore for Simple hash feature test
*         applicable devices: All DX , All Puma
*/
void prvTgfTrunkSimpleHashMacSaDaTestRestore
(
    void
);

/**
* @internal prvTgfTrunkSimpleHashMacSaDaTest function
* @endinternal
*
* @brief   check Simple hash for Mac SA DA.
*         2.1.1.1    Mac DA LBH
*         1.    Define trunk-Id 255 with 12 members.
*         2.    Send 1K incremental macs for 'learning' from one of the trunk ports (mac A , mac A+1 ...)
*         3.    from port that is not in trunk , send 1K packets with incremental DA (constant SA) that associated with the trunk (DA = incremental mac A , mac A+1 ...)
*         a.    do the same from 3 other ports that not in trunk
*         4.    check that each port of the trunk got ~1K/<num of members> packets
*         a.    check that total packets received at the trunk is 1K (no packet loss)
*         5.    check that ports that are not in the trunk got no packets (no flooding)
*         6.    If we left with 1 port in the trunk ' Stop test.
*         7.    Remove one port from the trunk.
*         8.    go to #3
*         2.1.1.2    Mac SA LBH
*         9.    Define trunk-Id 255 with 12 members.
*         10.    Send 1 mac for 'learning' from one of the trunk ports (mac A)
*         11.    from port that is not in trunk , send 1K packets with constant DA and incremental SA that associated with the trunk (DA = constant mac A)
*         a.    do the same from 3 other ports that not in trunk
*         12.    check that each port of the trunk got ~1K/<num of members> packets
*         a.    check that total packets received at the trunk is 1K (no packet loss)
*         13.    check that ports that are not in the trunk got no packets (no flooding)
*         14.    If we left with 1 port in the trunk ' Stop test.
*         15.    Remove one port from the trunk.
*         16.    go to #3
*         applicable devices: All DX , All Puma
*/
void prvTgfTrunkSimpleHashMacSaDaTest
(
    void
);

/**
* @internal tgfTrunkSortModeTest function
* @endinternal
*
* @brief   The sort mode test
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkSortModeTest
(
    void
);

/**
* @internal tgfTrunkWithRemoteMembersTest function
* @endinternal
*
* @brief   Purpose:
*         Define enh-UT / RDE test for LBH (load balance hash) on trunk members with ‘remote’ members.
*         The test require single device
*         Configuration 1:
*         FDB : Set 64 mac addresses related to trunk A
*         Trunk A "regular" with 8 members : 2 locals + 6 remote
*         Trunk B "cascade" with 2 ports
*         Trunk C "cascade" with 2 ports
*         Dev 2 "Remote" reached via trunk B (3 members in trunk A) --> device map table
*         Dev 3 "Remote" reached via trunk C (2 members in trunk A) --> device map table
*         Dev 4 "Remote" reached via port (1 member in trunk A) --> device map table
*         Configuration 2:
*         Same as configuration 1 + next:
*         Trunk B is "weighted cascade" the 2 ports weights are (2,6)
*         Configuration 3:
*         Same as configuration 1 or 2 + next:
*         Case 1 : Remove one of the ports from Cascade trunk A or B
*         Case 2 : Remove one of the members from Trunk A , a member of Dev 2 or Dev 3
*         Configuration 4:
*         After configuration 3 : restore the removed port
*         Traffic generated:
*         Send traffic from a port that is not relate to any trunk / remote device:
*         1. known UC – 64 mac DA
*         2. unknown UC – 64 mac DA (other then the known)
*         Check:
*         1. a. when 3 members of Dev 2 in trunk A : the ports of trunk B got 3/numMembersOf_trunkA of traffic
*         b. when 2 members of Dev 2 in trunk A : the ports of trunk B got 2/numMembersOf_trunkA of traffic
*         c. when ‘flood’ trunk B get FULL traffic
*         2. a. when 2 members of Dev 3 in trunk A : the ports of trunk C got 2/numMembersOf_trunkA of traffic
*         b. when 1 members of Dev 3 in trunk A : the ports of trunk C got 1/numMembersOf_trunkA of traffic
*         c. when ‘flood’ trunk C get FULL traffic
*         3. a. when 2 ports in trunk B : the ports share traffic according to weight (when no weight --> equal)
*         b. when 1 port in trunk B : the port get all traffic
*         4. Local Dev 1 Member 1 : get 1/numMembersOf_trunkA
*         5. Local Dev 1 Member 5 : get 1/numMembersOf_trunkA
*         6. a. Local Dev 1 Member 24 : get 1/numMembersOf_trunkA
*         b. when ‘flood’ Local Dev 1 Member 24 get FULL traffic
*         Test 1:
*         1. Configuration 1 ,traffic generate ,check
*         2. Configuration 3 ,traffic generate ,check
*         3. Configuration 4 ,traffic generate ,check
*         Test 2: (weighted cascade trunk)
*         Like test 1 , use configuration 2 instead of configuration 1
*         applicable devices: ALL DxCh devices
*/
void tgfTrunkWithRemoteMembersTest
(
    void
);


/**
* @internal tgfTrunkSaLearningTest function
* @endinternal
*
* @brief   Test mac SA learning from ports that are trunk members in local device,
*         and from from ports that are trunk members in remote device (via the DSA tag)
*/
void tgfTrunkSaLearningTest
(
    void
);

/**
* @internal tgfL2EcmpBasicCrc32HashTest function
* @endinternal
*
* @brief   Basic L2ECMP test with crc32
*/
void tgfL2EcmpBasicCrc32HashTest
(
    void
);

/**
* @internal tgfL2EcmpBasicCrc32HashSaltTest function
* @endinternal
*
* @brief   Basic L2ECMP test with crc32 and 'salt'
*         the tests uses SALT A and SALT B on the specific byte and check that
*         is produce different hash value (by select different secondary eport)
*/
void tgfL2EcmpBasicCrc32HashSaltTest
(
    void
);

/**
* @internal tgfL2EcmpSaLearningAndFilterTest function
* @endinternal
*
* @brief   Basic L2ECMP test for SA learning and source filtering.
*         Test L2ECMP for SA learning and source filtering.
*         1. define the L2ECMP as global EPorts in the Bridge(to ignore srcDev,trgDev
*         in SA learning and source filtering) ,
*         2. in FDB set the MAC SA with 'ownDev',eport=L2ECMP .
*         3. send traffic from cascade port ,
*         from 'reomte' srcDev and with 'srcEPort' of the L2ECMP.
*         4. check that no SA relearning due to change of source device.
*         4.a remove L2ECMP eport from 'global eports'
*         and check that there is re-learning.
*         (restore L2ECMP eport to 'global eports')
*         5. disable 'bride bypass' on cascade port
*         6. send traffic from cascade port ,
*         from 'reomte' srcDev and with 'srcEPort' of the L2ECMP , with DA
*         that associated with 'ownDev',eport=L2ECMP
*         7. check that packet is filtered although <'ownDev'> != <'reomte' srcDev>
*         7.a remove L2ECMP eport from 'global eports'
*         and check that packet not filtered.
*/
void tgfL2EcmpSaLearningAndFilterTest
(
    void
);

/**
* @internal tgfTrunkCrcHash_4K_members_known_UC_EPCL function
* @endinternal
*
* @brief   set 4K members in a trunk
*         send 'known UC' traffic to it , check by EPCL that bound to CNC counters
*         that the members got traffic according to 'predicted' egress ports.
*/
void tgfTrunkCrcHash_4K_members_known_UC_EPCL
(
    void
);


/**
* @internal tgfTrunk_80_members_flood_EPCL function
* @endinternal
*
* @brief   check that only first 64 members of the trunk get flooded traffic.
*         set 80 members in a trunk
*         send 4K flows of 'unknown' traffic to it , check by EPCL that bound to CNC counters
*         that only first 64 members of the trunk got traffic.
*         NOTE:
*         the test runs as '2 devices' : A and B
*         members in the trunk are ports from : A and B :
*         'first 64 ports : 25 ports from A , 39 ports from B
*         next 16 ports  : 8 ports from A , 8 ports from B
*         total 80 members
*         first run as device A :
*         send flood traffic , check that only those 25 ports got traffic and not
*         the other 8
*         second run as device B :
*         send flood traffic (the same traffic) , check that only those 39 ports got traffic and not
*         the other 8
*         NOTE: there is no use of DSA traffic , and the generating flood is network port
*         both on device A and on device B
*/
void tgfTrunk_80_members_flood_EPCL
(
    void
);



TGF_PACKET_STC * prvTgfTrunkPacketGet(IN PRV_TGF_PCL_PACKET_TYPE_ENT packetType);

/* return index of the member */
GT_U32 do_calculatedHash_expectedTrunkMemberIndex(
    IN  TGF_PACKET_STC    *packetInfoPtr,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT     trafficType,
    IN GT_BOOL  usedUdb,
    IN GT_U32   fieldByteIndex,
    IN GT_U32   localSrcPort,
    IN GT_U32   numTrunkMembers
);

/* force link up on all tested ports */
void forceLinkUpOnAllTestedPorts
(
    void
);

/* clear all the counter of the test */
void clearAllTestedCounters
(
    void
);

/**
* @internal prvTgfTrunkVlanTestInit function
* @endinternal
*
* @brief   Initialize local vlan default settings
*
* @param[in] vlanId                   - vlan id.
*                                      portMembers - bit map of ports belonging to the vlan.
*                                       None
*/
GT_VOID prvTgfTrunkVlanTestInit
(
    IN GT_U16                                  vlanId,
    CPSS_PORTS_BMP_STC                         portsMembers
);

/**
* @internal prvTgfTrunkVlanRestore function
* @endinternal
*
* @brief   Restore\Cancel prvTgfTrunkToCpuTraffic settings
*
* @param[in] vlanId                   - vlan id.
*                                       None
*/
GT_VOID prvTgfTrunkVlanRestore
(
    IN GT_U16   vlanId
);


/**
* @internal prvTgfTrunkTestPacketSend function
* @endinternal
*
* @brief   Function sends packets.
*
* @param[in] devNum                   - device number to send traffic from
* @param[in] portNum                  - port number to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
* @param[in] burstCount               - number of packets
* @param[in] numVfd                   - number of VFDs in vfdArray
* @param[in] vfdArray[]               - vfd Array (can be NULL when numVfd == 0)
*                                       None
*/
GT_VOID prvTgfTrunkTestPacketSend
(
    IN GT_U8           devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN TGF_PACKET_STC *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[]
);

/**
* @internal tgfTrunkIpclLoadBalancingHashConfig function
* @endinternal
*
* @brief   Configure/Deconfigure IPCL Load balancing hash test.
*
* @param[in] config                   - GT_TRUE  configure
*                                      GT_FALSE deconfigure
*                                       None
*/
void tgfTrunkIpclLoadBalancingHashConfig
(
    IN  GT_BOOL config
);

/**
* @internal tgfTrunkIpclLoadBalancingHashExecute function
* @endinternal
*
* @brief   Execute IPCL Load balancing hash test.
*/
void tgfTrunkIpclLoadBalancingHashExecute
(
    void
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTrunk */


