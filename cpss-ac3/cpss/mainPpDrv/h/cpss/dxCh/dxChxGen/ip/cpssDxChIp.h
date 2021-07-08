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
* @file cpssDxChIp.h
*
* @brief The CPSS DXCH Ip HW structures APIs
*
* @version   22
********************************************************************************
*/
#ifndef __cpssDxChIph
#define __cpssDxChIph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>


/**
* @internal cpssDxChIpv4PrefixSet function
* @endinternal
*
* @brief   Sets an ipv4 UC or MC prefix to the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to set
* @param[in] routerTtiTcamColumn      - the TCAM column to set
* @param[in] prefixPtr                - prefix to set
* @param[in] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail.
* @retval GT_BAD_PARAM             - on devNum not active
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. IP prefix entries reside in router TCAM. The router TCAM is organized
*       in rows, where each row contains 4 column entries for Ch3 and 5 column
*       entries for Ch2. Each line can hold:
*       - 1 TTI (tunnel termination interface) entry
*       - 1 ipv6 address
*       - 4 (ch3)/ 5 (ch2) ipv4 addresses
*       Note that if a line is used for ipv6 address or TTI entry then
*       it can't be used for ipv4 addresses and if an entry in a line is used
*       for ipv4 address, then the other line entries must hold ipv4 addresses
*       as well.
*       2. The match for prefixes is done using a first match , thus if two prefixes
*       can match a packet (they have the same prefix , but diffrent prefix
*       length) and we want LPM to work on them we have to make sure to put
*       the longest prefix before the short one.
*       Search order is by columns, first column, then second and so on.
*       All and all default UC prefix 0.0.0.0/mask 0 must reside at the last
*       possible ipv4 prefix, and default MC prefix 224.0.0.0/ mask 240.0.0.0
*       must reside at the index before it.
*       3. In Ch2 MC source addresses can reside only on the second and third
*       columns (on other PP, there is no such restriction).
*       4. In Ch2 there is no support for multiple virtual routers. Therefore in
*       Ch2 vrId is ignored.
*
*/
GT_STATUS cpssDxChIpv4PrefixSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  GT_U32                      routerTtiTcamColumn,
    IN  CPSS_DXCH_IPV4_PREFIX_STC   *prefixPtr,
    IN  CPSS_DXCH_IPV4_PREFIX_STC   *maskPtr
);


/**
* @internal cpssDxChIpv4PrefixGet function
* @endinternal
*
* @brief   Gets an ipv4 UC or MC prefix from the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to get from
* @param[in] routerTtiTcamColumn      - the TCAM column to get from
*
* @param[out] validPtr                 - whether the entry is valid (if the entry isn't valid
*                                      all the following fields will not get values)
* @param[out] prefixPtr                - prefix to set
* @param[out] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv4PrefixSet
*
*/
GT_STATUS cpssDxChIpv4PrefixGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  GT_U32                      routerTtiTcamColumn,
    OUT GT_BOOL                     *validPtr,
    OUT CPSS_DXCH_IPV4_PREFIX_STC   *prefixPtr,
    OUT CPSS_DXCH_IPV4_PREFIX_STC   *maskPtr
);


/**
* @internal cpssDxChIpv4PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv4 UC or MC prefix in the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to invalidate
* @param[in] routerTtiTcamColumn      - the TCAM column to invalidate
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssDxChIpv4PrefixInvalidate
(
    IN  GT_U8       devNum,
    IN  GT_U32      routerTtiTcamRow,
    IN  GT_U32      routerTtiTcamColumn
);

/**
* @internal cpssDxChIpv6PrefixSet function
* @endinternal
*
* @brief   Sets an ipv6 UC or MC prefix to the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to set
* @param[in] prefixPtr                - prefix to set
* @param[in] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv4PrefixSet
*
*/
GT_STATUS cpssDxChIpv6PrefixSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    IN  CPSS_DXCH_IPV6_PREFIX_STC   *prefixPtr,
    IN  CPSS_DXCH_IPV6_PREFIX_STC   *maskPtr
);

/**
* @internal cpssDxChIpv6PrefixGet function
* @endinternal
*
* @brief   Gets an ipv6 UC or MC prefix from the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to get from
*
* @param[out] validPtr                 - whether the entry is valid (if the entry isn't valid
*                                      all the following fields will not get values)
* @param[out] prefixPtr                - prefix to set
* @param[out] maskPtr                  - mask of the prefix
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpv6PrefixSet
*
*/
GT_STATUS cpssDxChIpv6PrefixGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerTtiTcamRow,
    OUT GT_BOOL                     *validPtr,
    OUT CPSS_DXCH_IPV6_PREFIX_STC   *prefixPtr,
    OUT CPSS_DXCH_IPV6_PREFIX_STC   *maskPtr
);

/**
* @internal cpssDxChIpv6PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv6 UC or MC prefix in the Router Tcam.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] routerTtiTcamRow         - the TCAM row to invalidate
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that invalidating ipv6 address (that takes whole TCAM line) will
*       result invalidating all columns in that line.
*
*/
GT_STATUS cpssDxChIpv6PrefixInvalidate
(
    IN  GT_U8           devNum,
    IN  GT_U32          routerTtiTcamRow
);


/**
* @internal cpssDxChIpLttWrite function
* @endinternal
*
* @brief   Writes a LookUp Translation Table Entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number.
* @param[in] lttTtiRow                - the entry's row index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttTtiColumn             - the entry's column index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttEntryPtr              - the lookup translation table entry
*
* @retval GT_OK                    - if succeeded
* @retval GT_BAD_PARAM             - on devNum not active or
*                                       invalid ipv6MCGroupScopeLevel value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Lookup translation table is shared with the Tunnel Termination (TT)
*       Table.
*       If the relevant index in the Router Tcam is in line used for tunnel
*       termination key then the indexes in the LTT/TT table will function as
*       Tunnel Termination Action, and you shouldn't write LTT entries in these
*       indexes. It works Vice-verse.
*       For Ipv6 Entries the relevant index is the line prefix (same index sent
*       as parameter to the ipv6 prefix set API). The other LTT/TT entries
*       related to that TCAM line are left unused.
*       FEr#2018 - if the PCL action uses redirection to Router Lookup Translation Table (LTT)
*       (policy based routing) or if the TTI action uses redirection to LTT
*       (TTI base routing), then the LTT index written in the PCL/TTI action is
*       restricted only to column 0 of the LTT row.
*       NOTE: - Since LTT entries can be used also for Router TCAM routing, this API is not
*       affected directly by this errata. However when the LTT should be pointed from
*       PCL action or LTT action, it should be set at column 0.
*       - The API affected by this errata are PCL rule set, PCL action update,
*       TTI rule set and TTI action update.
*
*/
GT_STATUS cpssDxChIpLttWrite
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          lttTtiRow,
    IN  GT_U32                          lttTtiColumn,
    IN  CPSS_DXCH_IP_LTT_ENTRY_STC      *lttEntryPtr
);


/**
* @internal cpssDxChIpLttRead function
* @endinternal
*
* @brief   Reads a LookUp Translation Table Entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number.
* @param[in] lttTtiRow                - the entry's row index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
* @param[in] lttTtiColumn             - the entry's column index in LTT table (equivalent to
*                                      the router tcam entry it is attached to)
*
* @param[out] lttEntryPtr              - the lookup translation table entry
*
* @retval GT_OK                    - if succeeded
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpLttWrite
*
*/
GT_STATUS cpssDxChIpLttRead
(
    IN  GT_U8                              devNum,
    IN  GT_U32                             lttTtiRow,
    IN  GT_U32                             lttTtiColumn,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC         *lttEntryPtr
);


/**
* @internal cpssDxChIpUcRouteEntriesWrite function
* @endinternal
*
* @brief   Writes an array of uc route entries to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
*
* @param[in] devNum                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to write the array
* @param[in] routeEntriesArray        - the uc route entries array
* @param[in] numOfRouteEntries        - the number route entries in the array (= the
*                                      number of route entries to write)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In case uRPF ECMP/QOS is globally enabled, then for ECMP/QOS block
*       with x route entries, additional route entry should be included after
*       the block with the uRPF route entry information.
*
*/
GT_STATUS cpssDxChIpUcRouteEntriesWrite
(
    IN GT_U8                           devNum,
    IN GT_U32                          baseRouteEntryIndex,
    IN CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN GT_U32                          numOfRouteEntries
);


/**
* @internal cpssDxChIpUcRouteEntriesRead function
* @endinternal
*
* @brief   Reads an array of uc route entries from the hw.
* 
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
* 
* @param[in] devNum                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to start reading
* @param[in,out] routeEntriesArray        - the uc route entries array, for each element
*                                      set the uc route entry type to determine how
*                                      entry data will be interpreted
* @param[in] numOfRouteEntries        - the number route entries in the array (= the
*                                      number of route entries to read)
* @param[in,out] routeEntriesArray        - the uc route entries array read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note See cpssDxChIpUcRouteEntriesWrite
*
*/
GT_STATUS cpssDxChIpUcRouteEntriesRead
(
    IN    GT_U8                             devNum,
    IN    GT_U32                            baseRouteEntryIndex,
    INOUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC   *routeEntriesArray,
    IN    GT_U32                            numOfRouteEntries
);

/**
* @internal cpssDxChIpMcRouteEntriesWrite function
* @endinternal
*
* @brief   Writes a MC route entry to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
* 
* @param[in] devNum                   - the device number
* @param[in] routeEntryIndex          - the Index in the Route entries table
* @param[in] routeEntryPtr            - the MC route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMcRouteEntriesWrite
(
    IN GT_U8                           devNum,
    IN GT_U32                          routeEntryIndex,
    IN CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
);

/**
* @internal cpssDxChIpMcRouteEntriesRead function
* @endinternal
*
* @brief   Reads a MC route entry from the hw.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
* 
* @param[in] devNum                   - the device number
* @param[in] routeEntryIndex          - the route entry index.
*
* @param[out] routeEntryPtr            - the MC route entry read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMcRouteEntriesRead
(
    IN GT_U8                            devNum,
    IN GT_U32                           routeEntryIndex,
    OUT CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *routeEntryPtr
);

/**
* @internal cpssDxChIpRouterNextHopTableAgeBitsEntryWrite function
* @endinternal
*
* @brief   set router next hop table age bits entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
* 
* @param[in] devNum                   - the device number
* @param[in] routerNextHopTableAgeBitsEntry - the router next hop table age bits
*                                      entry index. each entry is 32 Age
*                                      bits.
* @param[in] routerNextHopTableAgeBitsEntry - a 32 Age Bit map for route entries
*                                      <32routeEntriesIndexesArrayPtr>..
*                                      <32routeEntriesIndexesArrayPtr+31>
*                                      bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterNextHopTableAgeBitsEntryWrite
(
    IN  GT_U8   devNum,
    IN  GT_U32  routerNextHopTableAgeBitsEntryIndex,
    IN  GT_U32  routerNextHopTableAgeBitsEntry
);

/**
* @internal cpssDxChIpRouterNextHopTableAgeBitsEntryRead function
* @endinternal
*
* @brief   read router next hop table age bits entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
* 
* @param[in] devNum                   - the device number
* @param[in] routerNextHopTableAgeBitsEntryIndex - the router next hop table age bits
*                                      entry index. each entry is 32 Age
*                                      bits.
*
* @param[out] routerNextHopTableAgeBitsEntryPtr - a 32 Age Bit map for route entries
*                                      <32routeEntriesIndexesArrayPtr>..
*                                      <32routeEntriesIndexesArrayPtr+31>
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpRouterNextHopTableAgeBitsEntryRead
(
    IN  GT_U8   devNum,
    IN  GT_U32  routerNextHopTableAgeBitsEntryIndex,
    OUT GT_U32  *routerNextHopTableAgeBitsEntryPtr
);

/**
* @internal cpssDxChIpMLLPairWrite function
* @endinternal
*
* @brief   Write a Mc Link List (MLL) pair entry to hw. 
*  
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairWriteForm         - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - the Mc lisk list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active or invalid mllPairWriteForm.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note MLL entries are two words long.
*       In Lion and above devices the whole entry is written when last word is
*       set.
*       In PP prior to lion The atomic hw write operation is done on one word.
*       Writing order of MLL parameters could be significant as some parameters
*       in word[1],[3] depend on parameters in word[0],[2].
*       Erroneous handling of mllRPFFailCommand/ nextHopTunnelPointer may
*       result with bad traffic. nextPointer may result with PP infinite loop.
*       cpssDxChIpMLLPairWrite handles the nextPointer parameter in the
*       following way:
*       if (last == 1) first set word[0] or word[2].
*       if (last == 0) first set word[3].
*
*/
GT_STATUS cpssDxChIpMLLPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN CPSS_DXCH_IP_MLL_PAIR_STC                    *mllPairEntryPtr
);

/**
* @internal cpssDxChIpMLLPairRead function
* @endinternal
*
* @brief   Read a Mc Link List (MLL) pair entry from the hw.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairReadForm          - the way to read the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
*
* @param[out] mllPairEntryPtr          - the Mc lisk list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid parameter.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMLLPairRead
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairReadForm,
    OUT CPSS_DXCH_IP_MLL_PAIR_STC                   *mllPairEntryPtr
);


/**
* @internal cpssDxChIpMLLLastBitWrite function
* @endinternal
*
* @brief   Write a Mc Link List (MLL) Last bit to the hw. 
*  
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] lastBit                  - the Last bit
* @param[in] mllEntryPart             - to which part of the mll Pair to write the Last bit
*                                      for.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active or invalid mllEntryPart.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpMLLLastBitWrite
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           mllPairEntryIndex,
    IN GT_BOOL                                          lastBit,
    IN CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT    mllEntryPart
);

/**
* @internal cpssDxChIpRouterArpAddrWrite function
* @endinternal
*
* @brief   write a ARP MAC address to the router ARP / Tunnel start / NAT table (for NAT capable devices)
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
*
* @param[in] devNum                   - the device number
* @param[in] routerArpIndex           - The Arp Address index (to be inserted later
*                                      in the UC Route entry Arp nextHopARPPointer
*                                      field)
* @param[in] arpMacAddrPtr            - the ARP MAC address to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note pay attention that the router ARP table is shared with a tunnel start
*       table and NAT table (for NAT capable devices),
*       each tunnel start / NAT (for NAT capable devices) entry takes 4 ARP Address.
*       that is Tunnel start <n> / NAT (for NAT capable devices) <n>
*       takes ARP addresses <4n>,<4n+1>,<4n+2>,<4n+3>
*
*/
GT_STATUS cpssDxChIpRouterArpAddrWrite
(
    IN GT_U8                         devNum,
    IN GT_U32                        routerArpIndex,
    IN GT_ETHERADDR                  *arpMacAddrPtr
);


/**
* @internal cpssDxChIpRouterArpAddrRead function
* @endinternal
*
* @brief   read a ARP MAC address from the router ARP / Tunnel start / NAT table (for NAT capable devices)
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.  
*
* @param[in] devNum                   - the device number
* @param[in] routerArpIndex           - The Arp Address index (to be inserted later in the
*                                      UC Route entry Arp nextHopARPPointer field)
*
* @param[out] arpMacAddrPtr            - the ARP MAC address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_OUT_OF_RANGE          - on routerArpIndex bigger then 4095 or
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note see cpssDxChIpRouterArpAddrWrite
*
*/
GT_STATUS cpssDxChIpRouterArpAddrRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       routerArpIndex,
    OUT GT_ETHERADDR                 *arpMacAddrPtr
);

/**
* @internal cpssDxChIpEcmpEntryWrite function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*                                      (APPLICABLE RANGES: 0..12287)
* @param[in] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChIpEcmpEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       ecmpEntryIndex,
    IN CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntryPtr
);

/**
* @internal cpssDxChIpEcmpEntryRead function
* @endinternal
*
* @brief   Read an ECMP entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*                                      (APPLICABLE RANGES: 0..12287)
*
* @param[out] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChIpEcmpEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ecmpEntryIndex,
    OUT CPSS_DXCH_IP_ECMP_ENTRY_STC  *ecmpEntryPtr
);

/**
* @internal cpssDxChIpEcmpHashNumBitsSet function
* @endinternal
*
* @brief   Set the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism. Those parameters are used to select the ECMP member
*         within the ECMP block according to one of the following formulas:
*         - when randomEnable bit in the ECMP entry is GT_FALSE:
*         ECMP member offset =
*         ((size of ECMP block) (the value received between [startBit] and
*         [startBit + numOfBits - 1])) >> numOfBits
*         - when randomEnable bit in the ECMP entry is GT_TRUE:
*         ECMP member offset =
*         ((size of ECMP block) (random seed value)) >> numOfBits
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] startBit                 - the index of the first bit that is needed by the L3
*                                      ECMP hash mechanism (APPLICABLE RANGES: 0..31)
* @param[in] numOfBits                - the number of bits that is needed by the L3 ECMP hash
*                                      mechanism (APPLICABLE RANGES: 1..16)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong startBit or numOfBits
*
* @note startBit is relevant only when randomEnable field in the entry is set to
*       GT_FALSE.
*       startBit + numOfBits must not exceed 32.
*       startBit and numOfBits values may also be changed by
*       cpssDxChTrunkHashNumBitsSet
*
*/
GT_STATUS cpssDxChIpEcmpHashNumBitsSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       startBit,
    IN GT_U32                       numOfBits
);

/**
* @internal cpssDxChIpEcmpHashNumBitsGet function
* @endinternal
*
* @brief   Get the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism. Those parameters are used to select the ECMP member
*         within the ECMP block according to one of the following formulas:
*         - when randomEnable bit in the ECMP entry is GT_FALSE:
*         ECMP member offset =
*         ((size of ECMP block) (the value received between [startBit] and
*         [startBit + numOfBits - 1])) >> numOfBits
*         - when randomEnable bit in the ECMP entry is GT_TRUE:
*         ECMP member offset =
*         ((size of ECMP block) (random seed value)) >> numOfBits
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] startBitPtr              - (pointer to) the index of the first bit that is needed
*                                      by the L3 ECMP hash mechanism
* @param[out] numOfBitsPtr             - (pointer to) the number of bits that is needed by the
*                                      L3 ECMP hash mechanism
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note startBit is relevant only when randomEnable field in the entry is set to
*       GT_FALSE.
*
*/
GT_STATUS cpssDxChIpEcmpHashNumBitsGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *startBitPtr,
    OUT GT_U32                      *numOfBitsPtr
);

/**
* @internal cpssDxChIpEcmpHashSeedValueSet function
* @endinternal
*
* @brief   Set the "seed value" parameter used by the L3 ECMP hash mechanism
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] seed                     - the " value" parameter used by the L3 ECMP hash
*                                      (APPLICABLE RANGES: 1..0xffffffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on wrong seed
*
* @note The "seed value" parameter is relevant only for ECMP entries that are
*       configured with randomEnable == GT_TRUE.
*       0 is illegal seed value.
*
*/
GT_STATUS cpssDxChIpEcmpHashSeedValueSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       seed
);

/**
* @internal cpssDxChIpEcmpHashSeedValueGet function
* @endinternal
*
* @brief   Get the "seed value" parameter used by the L3 ECMP hash mechanism
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] seedPtr                  - (pointer to) the "seed value" parameter used by the L3
*                                      ECMP hash
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The "seed value" parameter is relevant only for ECMP entries that are
*       configured with randomEnable == GT_TRUE.
*
*/
GT_STATUS cpssDxChIpEcmpHashSeedValueGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *seedPtr
);


/**
* @internal cpssDxChIpEcmpIndirectNextHopEntrySet function
* @endinternal
*
* @brief   This function sets ECMP/QOS indirect table entry
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3x; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -  the device number
* @param[in] indirectIndex            - index of ECMP/QOS indirect table
* @param[in] nextHopIndex             - index of the next hop at Next Hop Table
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on out of range parameter.
* @retval GT_FAIL                  -    otherwise.
*
* @note none.
*
*/

GT_STATUS cpssDxChIpEcmpIndirectNextHopEntrySet
(
    IN GT_U8                        devNum,
    IN GT_U32                       indirectIndex,
    IN GT_U32                       nextHopIndex
);
/**
* @internal cpssDxChIpEcmpIndirectNextHopEntryGet function
* @endinternal
*
* @brief   This function gets ECMP/QOS indirect table entry
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3x; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   -  the device number
* @param[in] indirectIndex            - index of ECMP/QOS indirect table
* @param[in] nextHopIndexPtr          - Pointer the the next hop index at Next Hop Table
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - on out of range parameter.
* @retval GT_FAIL                  -    otherwise.
*
* @note none.
*
*/

GT_STATUS cpssDxChIpEcmpIndirectNextHopEntryGet
(
    IN        GT_U8                               devNum,
    IN        GT_U32                              indirectIndex,
    OUT       GT_U32                              * nextHopIndexPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIph */


