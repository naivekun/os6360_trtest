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
* @file prvTgfIpv4Ipv6UcMcRoutingAddDelAndFillLpm.h
*
* @brief IPV4 & IPV6 UC Routing when filling the Lpm and using many Add and
* Delete operations.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv4Ipv6UcMcRoutingAddDelAndFillLpmh
#define __prvTgfIpv4Ipv6UcMcRoutingAddDelAndFillLpmh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmIpv4Ipv6UcPrefixAddSomePrefix function
* @endinternal
*
* @brief   Add some ipv4 and ipv6 Prefixes
*/
GT_STATUS prvTgfIpLpmIpv4Ipv6UcPrefixAddSomePrefix(GT_VOID);

/**
* @internal prvTgfIpLpmIpv6UcPrefix64 function
* @endinternal
*
* @brief   Add ipv6 UC entries with prefixLen 64
*
* @param[in] totalNumIpUc             - number of ipv6 UC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the change.
*                                      next octet changed will be firstOctetIndexToChange+1
*                                      and so on
* @param[in] delete                   - is this an add or  operation
* @param[in] FirstByteIndex           - A value different then 0 will be given to the prefix
*                                      in location of  FirstByteIndex
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcPrefix64
(
    GT_U32  totalNumIpUc,
    GT_U32  firstOctetIndexToChange,
    GT_BOOL delete,
    GT_U32  FirstByteIndex,
    GT_BOOL order
);

/**
* @internal prvTgfIpLpmIpv6UcPrefix128 function
* @endinternal
*
* @brief   Add and delete ipv6 UC entries with prefixLen 128
*
* @param[in] first_totalNumIpUc       - number of ipv6 UC entries to add
* @param[in] second_totalNumIpUc      - number of ipv6 UC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the change.
*                                      next octet changed will be firstOctetIndexToChange+1
*                                      and so on
* @param[in] delete                   - is this an add or  operation
* @param[in] FirstByteIndex           - A value different then 0 will be given to the prefix
*                                      in location of  FirstByteIndex
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcPrefix128
(
    GT_U32  first_totalNumIpUc,
    GT_U32  second_totalNumIpUc,
    GT_U32  firstOctetIndexToChange,
    GT_BOOL delete,
    GT_U32  FirstByteIndex,
    GT_BOOL order
);

/**
* @internal prvTgfIpLpmIpv6McPrefixSrcChange function
* @endinternal
*
* @brief   Test how many IPv6 MC could be added into a LPM DB,
*         when source is changed and group stay with same value
* @param[in] totalNumIpMc             - number of ipv6 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] secondOctetIndexToChange - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6McPrefixSrcChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
);

/**
* @internal prvTgfIpLpmIpv6McPrefixGroupChange function
* @endinternal
*
* @brief   Test how many IPv6 MC could be added into a LPM DB,
*         when group is changed and source stay with same value
* @param[in] totalNumIpMc             - number of ipv6 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] firstOctetIndexToChange  - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6McPrefixGroupChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
);



/**
* @internal prvTgfIpLpmIpv6UcPrefix128AddDelMix function
* @endinternal
*
* @brief   Test how many IPv6 UC could be added into a LPM DB,
*         when doing scenario of Add and Delete prefixes in a mixed manner
* @param[in] totalNumIpUc             - number of ipv6 Uc entries to add
* @param[in] ipFirstByteValue         - value for the first octet
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] mixNum                   - the index from were we want to mix the add and delete operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcPrefix128AddDelMix
(
    GT_U32  totalNumIpUc,
    GT_U8   ipFirstByteValue,
    GT_U32  firstOctetIndexToChange,
    GT_U32  mixNum,
    GT_BOOL order
);

/**
* @internal prvTgfIpLpmIpv4McPrefixSrcChange function
* @endinternal
*
* @brief   Test how many IPv4 MC could be added into a LPM DB,
*         when source is changed and group stay with same value
* @param[in] totalNumIpMc             - number of ipv4 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] secondOctetIndexToChange - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4McPrefixSrcChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
);

/**
* @internal prvTgfIpLpmIpv4McPrefixGroupChange function
* @endinternal
*
* @brief   Test how many IPv4 MC could be added into a LPM DB,
*         when group is changed and source stay with same value
* @param[in] totalNumIpMc             - number of ipv4 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] firstOctetIndexToChange  - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4McPrefixGroupChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
);


/**
* @internal prvTgfIpLpmIpv6UcOrderAndDisorder function
* @endinternal
*
* @brief   Add some ipv6 Prefixes in order and disorder manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv6 UC entries to add
* @param[in] randomEnable             - GT_TRUE: increment the octets in a disordered manner
*                                      GT_FALSE:increment the octets in a sequentially manner
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcOrderAndDisorder
(
    GT_U32  totalNumIpUc,
    GT_BOOL randomEnable,
    GT_BOOL defragEnable
);


/**
* @internal prvTgfIpLpmIpv4UcOrderAndDisorder function
* @endinternal
*
* @brief   Add some ipv4 Prefixes in order and disorder manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
* @param[in] randomEnable             - GT_TRUE: increment the octets in a disordered manner
*                                      GT_FALSE:increment the octets in a sequentially manner
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcOrderAndDisorder
(
    GT_U32  totalNumIpUc,
    GT_BOOL randomEnable,
    GT_BOOL defragEnable
);

/**
* @internal prvTgfIpLpmIpv4UcDisorder function
* @endinternal
*
* @brief   Add some ipv4 Prefixes in disorder manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
*                                      randomEnable      - GT_TRUE: increment the octets in a disordered manner
*                                      GT_FALSE:increment the octets in a sequentially manner
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcDisorder
(
    GT_U32  totalNumIpUc,
    GT_BOOL defragEnable
);

/**
* @internal prvTgfIpLpmIpv6UcAddSpecificPrefix function
* @endinternal
*
* @brief   Add specific ipv6 Uc entry with prefix length = 64
*
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcAddSpecificPrefix
(
    GT_BOOL defragEnable
);

/**
* @internal prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop function
* @endinternal
*
* @brief   Add some Predefined and sequentially ipv4 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
* @param[in] ipv4Addr                 - ip address to start the loop from
* @param[in] ipPrefixLen              - prefix length
* @param[in] nextHopInfoPtr           - (pointer to) the next hop information
* @param[in] startLoop                - beginning of loop index
* @param[in] endLoop                  - end of loop index
* @param[in] numOfExpectedPrefixes    - number of prefixes expected to be added/deleted
* @param[in] firstOctetToChange       - first octet we want to change in the ip address
* @param[in] secondOctetToChange      - second octet we want to change in the ip address
* @param[in] addFlag                  - GT_TRUE: add prefixes
*                                      GT_FALSE: delete prefixes
* @param[in] defragEnable             - GT_TRUE: use defrag mechanism when adding the prefix
*                                      GT_FALSE: do not use defrag mechanism when adding the prefix
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop
(
    GT_IPADDR                               ipv4Addr,
    GT_U32                                  ipPrefixLen,
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr,
    GT_U32                                  startLoop,
    GT_U32                                  endLoop,
    GT_U32                                  numOfExpectedPrefixes,
    GT_U32                                  firstOctetToChange,
    GT_U32                                  secondOctetToChange,
    GT_BOOL                                 addFlag,
    GT_BOOL                                 defragEnable
);

/**
* @internal prvTgfIpLpmIpv4UcCheckMergeBanks function
* @endinternal
*
* @brief   Add some Predefined and sequentially ipv4 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
*         test the Merge bank mechanism
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcCheckMergeBanks
(
    GT_U32  totalNumIpUc
);

/**
* @internal prvTgfIpLpmIpv4UcOrderAndPredefined function
* @endinternal
*
* @brief   Add some Predefined and sequentially ipv4 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
* @param[in] flushFlag                - GT_TRUE: flush all entries before the test
*                                      GT_FALSE: do not flush all entries before the test
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcOrderAndPredefined
(
    GT_U32  totalNumIpUc,
    GT_BOOL flushFlag
);

/**
* @internal prvTgfIpLpmIpUcOrderOctet0To6 function
* @endinternal
*
* @brief  Add some sequentially ipv6 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add

*/
GT_VOID prvTgfIpLpmIpUcOrderOctet0To6
(
    GT_U32                      totalNumIpUc,
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack,
    CPSS_UNICAST_MULTICAST_ENT  prefixType,
    GT_U32                      prefixLength,
    GT_U32                      srcPrefixLength
);

#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __prvTgfIpv4Ipv6UcMcRoutingAddDelAndFillLpmh */


