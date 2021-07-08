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
* @file tgfMplsGen.h
*
* @brief Generic API for MPLS
*
* @version   1
********************************************************************************
*/
#ifndef CHX_FAMILY
    /* we not want those includes !! */
    #define __tgfMplsGenh
#endif /*CHX_FAMILY*/

#ifndef __tgfMplsGenh
#define __tgfMplsGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfIpGen.h>
#include <common/tgfInlifGen.h>
#include <common/tgfCommon.h>

#ifdef EXMXPM_FAMILY
    #include <cpss/exMxPm/exMxPmGen/ip/cpssExMxPmIpStat.h>
    #include <cpss/exMxPm/exMxPmGen/ip/cpssExMxPmIpTypes.h>
    #include <cpss/exMxPm/exMxPmGen/inlif/cpssExMxPmInlif.h>
    #include <cpss/exMxPm/exMxPmGen/mpls/cpssExMxPmMpls.h>
#endif /* EXMXPM_FAMILY */


/**
* @enum PRV_TGF_MPLS_EXCEPTION_TYPE_ENT
 *
 * @brief Enumeration of MPLS Exception Types.
*/
typedef enum{

    /** @brief packets with Incoming TTL 0,
     *  and TTL and Options check is not bypassed
     *  (NHLFE <Bypass TTL and Options Check>=0)
     */
    PRV_TGF_MPLS_INCOMING_TTL_EXCP_E,

    /** @brief packets with TTL that is
     *  about to be exceeded, and TTL and Options check is not
     *  bypassed (NHLFE <Bypass TTL and Options Check>=0)
     */
    PRV_TGF_MPLS_OUTGOING_TTL_EXCP_E,

    /** @brief packets length is greater than
     *  the MTU profile pointed to by the NHLFE.
     */
    PRV_TGF_MPLS_MTU_EXCP_E

} PRV_TGF_MPLS_EXCEPTION_TYPE_ENT;

/**
* @enum PRV_TGF_MPLS_SPECIAL_ROUTER_TRIGGER_TYPE_ENT
 *
 * @brief Types of soft dropped or trapped traffic that can be
 * enabled/disabled to trigger MPLS router engine.
*/
typedef enum{

    /** @brief Packets that were
     *  assigned a command of TRAP prior to the MPLS engine
     */
    PRV_TGF_MPLS_SPECIAL_ROUTER_TRIGGER_TRAP_E      = 0,

    /** @brief Packets that were
     *  assigned a command of SOFT DROP prior to the MPLS engine
     */
    PRV_TGF_MPLS_SPECIAL_ROUTER_TRIGGER_SOFT_DROP_E = 1

} PRV_TGF_MPLS_SPECIAL_ROUTER_TRIGGER_TYPE_ENT;


/**
* @enum PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_ENT
 *
 * @brief Enumeration of MPLS Next Hop Access Method
 * (controls the method of calculation the NHLFE Pointer)
*/
typedef enum{

    /** @brief The ILM Table consists
     *  of NHLFE entry blocks used for Equal Cost Multi Path.
     */
    PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_ECMP_E    = 0,

    /** @brief The ILM Table consists
     *  of NHLFE entry blocks used for QoS routing.
     */
    PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_QOS_E     = 1,

    /** @brief The ILM Table consists
     *  of individual NHLFE entries.
     *  The MPLS label is used as a direct pointer to the NHLFE.
     */
    PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_REGULAR_E = 2

} PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_ENT;


/**
* @struct PRV_TGF_MPLS_INTERFACE_ENTRY_STC
 *
 * @brief A structure to hold MPLS Interface Table entry fields
*/
typedef struct{

    /** @brief validates the entry data.
     *  FALSE:Entry Not valid, TRUE:Entry is valid
     *  valid range (0...7) when 0 indicates a single entry.
     */
    GT_BOOL valid;

    /** the minimum MPLS label supported for the interface */
    GT_U32 minLabel;

    /** the maximum MPLS label supported for the interface */
    GT_U32 maxLabel;

    /** @brief A base index in the ILM(Incoming Label Map)
     *  Table for the entries that are bound to the interface.
     *  NOTE: this is an offset entry Index relative to
     *  <NHLFE Table Base Address>.
     *  The entries for the minLabel-maxLabel range in the
     *  NHLF table will be written from this base on.
     *  0 means the first entry in the ILM Table relative
     *  to the <MPLS NHLFE Table Base Address>.
     */
    GT_U32 baseIndex;

    /** @brief The number of entries in the ILM(Incoming Label Map)
     *  that are associated with an ECMP or QoS forwarding.
     */
    GT_U32 ecmpQosSize;

    /** Method of calculating the final NHLFE pointer. */
    PRV_TGF_MPLS_NEXT_HOP_ROUTE_METHOD_ENT nextHopRouteMethod;

} PRV_TGF_MPLS_INTERFACE_ENTRY_STC;


/**
* @enum PRV_TGF_MPLS_CMD_ENT
 *
 * @brief Enumeration of MPLS command assigned to the packet
*/
typedef enum{

    /** Do nothing */
    PRV_TGF_MPLS_NOP_CMD_E,

    /** Swap the most outer label with another Label */
    PRV_TGF_MPLS_SWAP_CMD_E,

    /** Add a new label on top of the MPLS labels stack */
    PRV_TGF_MPLS_PUSH_CMD_E,

    /** @brief (Penultimate Hop Pop): Pop the label from the
     *  MPLS label stack. Forwarding decision is based on the popped label
     */
    PRV_TGF_MPLS_PHP_CMD_E

} PRV_TGF_MPLS_CMD_ENT;

/**
* @enum PRV_TGF_MPLS_COUNTER_SET_INDEX_ENT
 *
 * @brief Enumeration of Management counters sets updated when packets
 * hits this Route Entry.
*/
typedef enum{

    /** Management Counters Set 0 */
    PRV_TGF_MPLS_COUNTER_SET_INDEX_0_E = 0,

    /** Management Counters Set 1 */
    PRV_TGF_MPLS_COUNTER_SET_INDEX_1_E = 1,

    /** Management Counters Set 2 */
    PRV_TGF_MPLS_COUNTER_SET_INDEX_2_E = 2,

    /** Management Counters Set 3 */
    PRV_TGF_MPLS_COUNTER_SET_INDEX_3_E = 3,

    /** @brief No management counter set is
     *  bound to this route entry. Traffic matching this route entry is not counted.
     */
    PRV_TGF_MPLS_COUNTER_SET_DISABLE_E = 7

} PRV_TGF_MPLS_COUNTER_SET_INDEX_ENT;


/**
* @enum PRV_TGF_MPLS_TTL_MODE_ENT
 *
 * @brief Enumeration of TTL Manipulation mode.
*/
typedef enum{

    /** @brief TTL of routed packets associated
     *  with this Next Hop Entry is not decremented when the packet is
     *  transmitted from this device.
     */
    PRV_TGF_MPLS_TTL_NOT_DECREMENTED_E = 0,

    /** @brief TTL of routed packets associated
     *  with this Next Hop Entry is decremented by 1 when the packet is
     *  transmitted from this device.
     */
    PRV_TGF_MPLS_TTL_DECREMENTED_BY_ONE_E,

    /** @brief TTL of routed packets
     *  associated with this Next Hop Entry is decremented by <TTL> when
     *  the packet is transmitted from this device.
     */
    PRV_TGF_MPLS_TTL_DECREMENTED_BY_ENTRY_E,

    /** @brief TTL of routed packets associated with
     *  this Next Hop Entry is assigned by this entry to a new TTL
     *  value, when the packet is transmitted from this device.
     */
    PRV_TGF_MPLS_TTL_COPY_ENTRY_E

} PRV_TGF_MPLS_TTL_MODE_ENT;

/**
* @struct PRV_TGF_MPLS_NHLFE_COMMON_STC
 *
 * @brief Describes the logical content of the NHLF Table entries
*/
typedef struct{

    /** The label to use when MPLS Command is Swap or Push */
    GT_U32 mplsLabel;

    /** MPLS command assigned to the packet. */
    PRV_TGF_MPLS_CMD_ENT mplsCmd;

    /** output logical interfce configuration parameters. */
    PRV_TGF_OUTLIF_INFO_STC outlifConfig;

    /** @brief The next hop VLAN
     *  label lookup
     */
    GT_U16 nextHopVlanId;

    /** @brief This bit is set to GT_TRUE when the
     *  label lookup matches this entry
     */
    GT_BOOL ageRefresh;

    /** index of MTU profile. Range (0..15) */
    GT_U32 mtuProfileIndex;

    /** @brief Management Counter set update when packets
     *  hits this Route Entry.
     */
    PRV_TGF_MPLS_COUNTER_SET_INDEX_ENT counterSetIndex;

    GT_BOOL mirrorToIngressAnalyzer;

    /** @brief The TTL value that is used when the <TTL Mode>
     *  is either: Decrement TTL by NHLFE<TTL> or
     *  Copy NHLFE<TTL>
     *  Range(0...255)
     *  ttMode         - TTL manipulation mode
     */
    GT_U32 ttl;

    PRV_TGF_MPLS_TTL_MODE_ENT ttlMode;

    /** @brief Enables/disables bypassing TTL
     *  exception and Options check.
     */
    GT_BOOL bypassTtlExceptionCheckEnable;

    /** @brief Used for indexing in CPU code table; Relevant
     *  only if <Command>= TRAP or ROUTE_AND_MIRROR
     *  Range(0...3)
     */
    GT_U32 cpuCodeIndex;

    /** @brief The NHLFE assigns a command based on the
     *  entry <Packet Command>.
     *  Valid options :
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     *  CPSS_PACKET_CMD_DROP_HARD_E
     *  CPSS_PACKET_CMD_DROP_SOFT_E
     *  CPSS_PACKET_CMD_ROUTE_E
     *  CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E
     */
    CPSS_PACKET_CMD_ENT packetCmd;

} PRV_TGF_MPLS_NHLFE_COMMON_STC;

/**
* @struct PRV_TGF_MPLS_NHLFE_STC
 *
 * @brief Describes the logical content of the NHLF Table entries.
*/
typedef struct{

    /** NHLFE common content */
    PRV_TGF_MPLS_NHLFE_COMMON_STC nhlfeCommonParams;

    /** Enable/Disable modifing per QoS parametes */
    PRV_TGF_QOS_PARAM_MODIFY_STC qosParamsModify;

    /** QoS Parameters */
    PRV_TGF_QOS_PARAM_STC qosParams;

} PRV_TGF_MPLS_NHLFE_STC;

/**
* @enum PRV_TGF_MPLS_FRR_COMMAND_ENT
 *
 * @brief Enumeration of Fast-Reroute Command modes.
*/
typedef enum{

    /** packet's OutLIF isnt mapped from FRR entry */
    PRV_TGF_MPLS_FRR_COMMAND_DONT_MAP_E = 0,

    /** packet's OutLIF is assigned from FRR entry */
    PRV_TGF_MPLS_FRR_COMMAND_MAP_E      = 1

} PRV_TGF_MPLS_FRR_COMMAND_ENT;

/*
 * Typedef: struct PRV_TGF_MPLS_FAST_REROUTE_STC
 *
 * Description:
 *          Logical content of the Fast Re-Route table entry.
 *
 * Fields:
 *  outLif          - The output interface this FRR entry sends to.
 *                       Valid types of outLif.interfaceInfo.type:
 *                           CPSS_INTERFACE_PORT_E
 *                           CPSS_INTERFACE_TRUNK_E
 *                           CPSS_INTERFACE_VIDX_E
 *                           CPSS_INTERFACE_FABRIC_VIDX_E
 *  mapCommand      - whether to map outlif from FRR entry or to retain packet's outlif
 */

typedef struct PRV_TGF_MPLS_FAST_REROUTE_STCT
{
    PRV_TGF_OUTLIF_INFO_STC         outLif;
    PRV_TGF_MPLS_FRR_COMMAND_ENT    mapCommand;

}PRV_TGF_MPLS_FAST_REROUTE_STC;


/**
* @enum PRV_TGF_MPLS_LABEL_SPACE_MODE_ENT
 *
 * @brief Enumeration of MPLS Label space mode.
*/
typedef enum{

    /** @brief Label space is common for
     *  all interfaces.
     */
    PRV_TGF_MPLS_PER_PLATFORM_E,

    /** @brief Label space is defined per
     *  interface according to the assigned VRF.
     */
    PRV_TGF_MPLS_PER_INTERFACE_E

} PRV_TGF_MPLS_LABEL_SPACE_MODE_ENT;


/**
* @internal prvTgfMplsLsrEnableSet function
* @endinternal
*
* @brief   Enable MPLS LSR(Label Switch Router) switching
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable feature
*                                      GT_FALSE - MPLS LSR switching is bypassed
*                                      GT_TRUE  - MPLS LSR switching is enabled
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsLsrEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal prvTgfMplsLsrEnableGet function
* @endinternal
*
* @brief   Get MPLS LSR(Label Switch Router) switching state
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) MPLS Engine state
*                                      GT_FALSE - MPLS LSR switching is bypassed
*                                      GT_TRUE  - MPLS LSR switching is enabled
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsLsrEnableGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvTgfInlifMplsRouteEnableSet function
* @endinternal
*
* @brief   Set the MPLS switching on the interface (Enable\disable)
*
* @param[in] devNum                   - device number
* @param[in] inlifType                - inlif type port/vlan/external
* @param[in] inlifIndex               - inlif index in the table
*                                      (APPLICABLE RANGES: 0..63) for inLif type port or
*                                      (APPLICABLE RANGES: 0..4095) for inLif type vlan or
*                                      (APPLICABLE RANGES: Puma2 4096..65535; Puma3 4096..16383)
*                                      for inLif type external
* @param[in] enable                   - GT_TRUE = Enable.
*                                      GT_FALSE = Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_FAIL                  - otherwise
* @retval GT_UNFIXABLE_ECC_ERROR   - the CPSS detected ECC error that can't
*                                       be fixed when read from the memory that
*                                       protected by ECC generated.
*                                       if entry can't be fixed due to 2 data errors
*                                       NOTE: relevant only to memory controller that
*                                       work with ECC protection , and the CPSS need
*                                       to emulate ECC protection.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfInlifMplsRouteEnableSet
(
    IN  GT_U8                       devNum,
    IN  PRV_TGF_INLIF_TYPE_ENT      inlifType,
    IN  GT_U32                      inlifIndex,
    IN  GT_BOOL                     enable
);

/**
* @internal prvTgfMplsLabelSpaceModeSet function
* @endinternal
*
* @brief   Defines whether the label space is common or per Interface
*
* @param[in] devNum                   - device number
* @param[in] mode                     - MPLS Label Space mode
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsLabelSpaceModeSet
(
    IN GT_U8                              devNum,
    IN PRV_TGF_MPLS_LABEL_SPACE_MODE_ENT  mode
);

/**
* @internal prvTgfMplsLabelSpaceModeGet function
* @endinternal
*
* @brief   Get label space mode (common or per Interface)
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - (pointer to) MPLS Label Space mode
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - the registers relate to the feature are not synchronized
*                                       (NOTE : values returned in OUT parameters are valid)
*/
GT_STATUS prvTgfMplsLabelSpaceModeGet
(
    IN  GT_U8                               devNum,
    OUT PRV_TGF_MPLS_LABEL_SPACE_MODE_ENT   *modePtr
);

/**
* @internal prvTgfMplsNhlfeSpecialLabelsBaseIndexSet function
* @endinternal
*
* @brief   Set MPLS Special Labels Table base index
*         Used for MPLS labels in the range 0-15.
* @param[in] devNum                   - device number
* @param[in] baseIndex                - base index of the MPLS Special Labels in the NHLFE table.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - bad paramter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is recommended to set baseIndex to 0, meaning that these labels are
*       located at the beggining of the NHLFE Table.
*
*/

GT_STATUS prvTgfMplsNhlfeSpecialLabelsBaseIndexSet
(
    IN GT_U8   devNum,
    IN GT_U32  baseIndex
);

/**
* @internal prvTgfMplsNhlfeSpecialLabelsBaseIndexGet function
* @endinternal
*
* @brief   Get MPLS Special Labels Table base index
*         Used for MPLS labels in the range 0-15.
* @param[in] devNum                   - device number
*
* @param[out] baseIndexPtr             - (pointer to) base index of the MPLS Special Labels
*                                      in the NHLFE table.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If baseIndex is 0 it means that these labels are located
*       at the beggining of the NHLFE Table
*
*/
GT_STATUS prvTgfMplsNhlfeSpecialLabelsBaseIndexGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *baseIndexPtr
);

/**
* @internal prvTgfMplsInterfaceEntrySet function
* @endinternal
*
* @brief   Set the MPLS Interface Table Entry
*
* @param[in] devNum                   - device number
* @param[in] index                    - MPLS Interface Table  calculated based on the VRF-ID
* @param[in] mplsInterfaceEntryStcPtr - (pointer to)The MPLS Interface entry to set
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When the device is configured to work in Per-Platform Label Space Mode,
*       index 0 is used.
*
*/
GT_STATUS prvTgfMplsInterfaceEntrySet
(
    IN GT_U8                             devNum,
    IN GT_U32                            index,
    IN PRV_TGF_MPLS_INTERFACE_ENTRY_STC  *mplsInterfaceEntryStcPtr
);

/**
* @internal prvTgfMplsInterfaceEntryGet function
* @endinternal
*
* @brief   Get the MPLS Interface Table Entry
*
* @param[in] devNum                   - device number
* @param[in] index                    - MPLS Interface Table  calculated based on the VRF-ID
*
* @param[out] mplsInterfaceEntryStcPtr - (pointer to) The MPLS Interface entry
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
*
* @note When the device is configured to work in Per-Platform Label Space Mode,
*       index 0 is used.
*
*/
GT_STATUS prvTgfMplsInterfaceEntryGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            index,
    OUT PRV_TGF_MPLS_INTERFACE_ENTRY_STC  *mplsInterfaceEntryStcPtr
);

/**
* @internal prvTgfMplsNhlfEntriesWrite function
* @endinternal
*
* @brief   Write an array of NHLFE entries to hw.
*
* @param[in] devNum                   - device number
* @param[in] nhlfEntryBaseIndex       - The base Index in the NHLFE table
*                                      The entries will be written from this base on.
* @param[in] numOfNhlfEntries         - The number NHLF entries to write.
* @param[in] nhlfEntriesArrayPtr      - (pointer to) The NHLF entries array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note No partial write.
*       Errata FE-8050: MPLS Router always uses a Uniform model on the TTL
*       Description:
*       The device supports MPLS switching.
*       When the incoming packet is MPLS and the following conditions are met:
*       1. Incoming MPLS packet contains one label or more.
*       2. MPLS routing is enabled in the InLIF.
*       3. No Match in the TTI.
*       4. NHLFE<MPLS Cmd> = POP/SWAP/PUSH.
*       5. NHLFE<TTL Mode> = Decrement/Not Decrement/Copy NHLFE<TTL>.
*       6. TTL > 0.
*       The expected TTL of the outer MPLS header is as follows:
*       - POP: TTL is copied from the original outer MPLS header or the Inner MPLS header TTL is used.
*       - SWAP: TTL is copied from the original outer MPLS header or the NHLFE<TTL> is used.
*       - PUSH: TTL is copied from the original inner MPLS header or the NHLFE<TTL> is used.
*       However, due to this erratum, the TTL of the new outer MPLS header contains the TTL of the
*       original outer MPLS header (Uniform model) instead of having the ability to control which
*       TTL is used (Pipe model/Uniform model).
*       When the packet does not pass through the TTI or there is no match to the TTI key, the router
*       always copies/uses the original outer MPLS header’s TTL.
*       Workaround: None
*       Errata FE-8250: Wrong access to internal nhe table
*       Description:
*       When accessing the nhe internal table the address decode is wrong:
*       1. Address decode is:
*       mg[4:0] - resolution in line
*       mg[18:5] - line number
*       2. The Table offset is: 0x00540000 which means that in the address
*       offset bit18 is turned on. This bit is also used for the line number
*       decode. This creates a situation where address 0x00540000 actually accesses line:
*       0x2000 instead of line number: 0x0
*       Workaround:
*       1. lines 0x0-0x1FFF    addresses should be: 0x00580000-0x005BFFFF.
*       2. lines 0x2000-0x3FFF addresses should be: 0x00540000-0x0057FFFF.
*       - This erratum allow to look at this table as 2 separate tables with 2 different offsets.
*       Errata FE-8490: Wrong packet command resolution in IP/MPLS router engine
*       Description:
*       Each ingress pipeline engine can assign a new packet command to the
*       packet according to the description in the command resolution matrix.
*       The command resolution matrix defines that the final command should be
*       trapped if the Previous Engine assigned the Packet Command to Soft Drop,
*       and the New Engine assigned the Packet Command to Mirror.
*       However, in the IP/MPLS Router Engine, the final command in this case
*       will be soft dropped and not trapped.
*       Functional Impact:
*       IP/MPLS Router Engine will not change the packet command according to
*       the Ingress Command Resolution Matrix for all the packets that enter the
*       IP/MPLS Router Engine with packet commands that equal Soft Drop, and if
*       the IP/MPLS Router Engine decided to change the packet command to Mirror.
*       Workaround:
*       None
*
*/
GT_STATUS prvTgfMplsNhlfEntriesWrite
(

    IN  GT_U8                               devNum,
    IN  GT_U32                              nhlfEntryBaseIndex,
    IN  GT_U32                              numOfNhlfEntries,
    IN  PRV_TGF_MPLS_NHLFE_STC              *nhlfEntriesArrayPtr
);

/**
* @internal prvTgfMplsNhlfEntriesRead function
* @endinternal
*
* @brief   Read an array of NHLF entries from hw.
*
* @param[in] devNum                   - device number
* @param[in] nhlfEntryBaseIndex       - The base Index in the NHLFE table
*                                      The entries will be read from this base on.
* @param[in] numOfNhlfEntries         - The number NHLF entries to read.
*
* @param[out] nhlfEntriesArrayPtr      - (pointer to) The NHLF entries array
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_UNFIXABLE_ECC_ERROR   - the CPSS detected ECC error that can't
*                                       be fixed when read from the memory that
*                                       protected by ECC generated.
*                                       if entry can't be fixed due to 2 data errors
*                                       NOTE: relevant only to memory controller that
*                                       work with ECC protection , and the CPSS need
*                                       to emulate ECC protection.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsNhlfEntriesRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          nhlfEntryBaseIndex,
    IN  GT_U32                          numOfNhlfEntries,
    OUT PRV_TGF_MPLS_NHLFE_STC          *nhlfEntriesArrayPtr
);

/**
* @internal prvTgfMplsCountersIpSet function
* @endinternal
*
* @brief   Set IP couters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvTgfMplsCountersIpSet
(
    IN GT_U32                          portNum,
    IN GT_U32                         counterIndex
);

/**
* @internal prvTgfMplsInvalidEntryCmdSet function
* @endinternal
*
* @brief   Set the command assigned to MPLS packets if any of the following is true:
*         - The MPLS Interface Table entry is not valid
*         - The incoming label is out of label space boundaries
* @param[in] devNum                   - device number
* @param[in] cmd                      - Valid commands:
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E - Packets are trapped to the CPU
*                                      with a CPU code of INVALID_ENTRY_ERROR
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - bad paramter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsInvalidEntryCmdSet
(
    IN GT_U8                               devNum,
    IN CPSS_PACKET_CMD_ENT                 cmd
);

/**
* @internal prvTgfMplsInvalidEntryCmdGet function
* @endinternal
*
* @brief   Get the command assigned to MPLS packets if any of the following is true:
*         - The MPLS Interface Table entry is not valid
*         - The incoming label is out of label space boundaries
* @param[in] devNum                   - device number
*
* @param[out] cmdPtr                   - (pointer to) Valid commands:
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E - Packets are trapped to the CPU
*                                      with a CPU code of INVALID_ENTRY_ERROR
*                                      CPSS_PACKET_CMD_DROP_HARD_E
*                                      CPSS_PACKET_CMD_DROP_SOFT_E
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on HW error.
* @retval GT_BAD_PARAM             - bad paramter
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfMplsInvalidEntryCmdGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_PACKET_CMD_ENT                 *cmdPtr
);

#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __tgfMplsGenh */

