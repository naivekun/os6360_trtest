/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoLion2TrunkWa.c
*
* @brief Includes configurations of trunks for cut through WA.
*
* @version   6
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoLion2TrunkWa.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>

#define MAX_DEV_IDS_CNS  32

/* Number of relay ports per one uplink port */
#define APP_DEMO_LION2_TRUNK_WA_RELAY_PER_UPLINK_PORTS_NUM_CNS   4
/* Maximal trunk numbers used in WA */
#define APP_DEMO_LION2_TRUNK_WA_MAX_NUM_CNS                      7

/**
* @struct RELAY_TRUNK_WA_PORTS_MEMBERS_STC
 *
 * @brief The structure holds info for trunks WA relay trunks.
*/
typedef struct{

    /** current trunk ID for relay ports */
    GT_TRUNK_ID trunkId;

    /** uplink trunk ID */
    GT_TRUNK_ID upLinkTrunkId;

    /** default trunk ports members */
    CPSS_PORTS_BMP_STC origPortsMember;

    /** default trunk ID for relay ports when uplink trunk is not in use */
    GT_TRUNK_ID origTrunkId;

    /** source ID */
    GT_U32 srcId;

    /** default trunk ID for relay ports when uplink trunk is in use */
    GT_TRUNK_ID activeTrunkId;

} RELAY_TRUNK_WA_PORTS_MEMBERS_STC;

/**
* @struct UPLINK_TRUNK_INFO_STC
 *
 * @brief The structure holds info for uplink trunks.
*/
typedef struct{

    /** trunk ID */
    GT_TRUNK_ID trunkId;

    /** uplink trunk type */
    CPSS_TRUNK_TYPE_ENT trunkType;

} UPLINK_TRUNK_INFO_STC;

/**
* @struct APP_DEMO_LION2_TRUNK_WA_CONFIG_STC
 *
 * @brief The structure holds info for trunk configuration WA.
*/
typedef struct{

    /** @brief first reserved trunk ID used in the WA
     *  firstReservedSourceId - first reserved source ID used in the WA
     */
    GT_TRUNK_ID firstReservedTrunkId;

    GT_U32 uplinkPortsMembersArr[APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS];

    GT_U32 relayPortsMembersArr[APP_DEMO_LION2_TRUNK_WA_RELAY_PORTS_NUM_CNS];

    RELAY_TRUNK_WA_PORTS_MEMBERS_STC relayTrunkMembersDb[APP_DEMO_LION2_TRUNK_WA_MAX_NUM_CNS];

    UPLINK_TRUNK_INFO_STC uplinkPortsTrunk[APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS];

    /** virtual trunk ID */
    GT_TRUNK_ID virtualDefaultTrunkId;

} APP_DEMO_LION2_TRUNK_WA_CONFIG_STC;

static APP_DEMO_LION2_TRUNK_WA_CONFIG_STC trunkWaConfig[MAX_DEV_IDS_CNS];

/* Dummy port for virtual cascade trunk */
#define PRV_VIRTUAL_CSCD_TRUNK_DUMMY_PORT   69

/**
* @internal appDemoLion2DeviceMapTableInit function
* @endinternal
*
* @brief   Configure device map table for remote device.
*
* @param[in] devNum                   - device number
* @param[in] hwDevNum                 - HW device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoLion2DeviceMapTableInit
(
    IN GT_U8    devNum,
    IN GT_HW_DEV_NUM    hwDevNum
)
{
    CPSS_CSCD_LINK_TYPE_STC cascadeLink;            /* cascade link info */
    GT_PHYSICAL_PORT_NUM    portNum;                /* port number */
    GT_STATUS               rc;                     /* return code */
    CPSS_PORTS_BMP_STC      portsMembersBmp;        /* bitmap ports members  */

    rc = cpssDxChCscdDbRemoteHwDevNumModeSet(hwDevNum, CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembersBmp);

    /* Set virtual port member. All relay ports are in the second hemisphere,
       so dummy port belongs also to the second hemisphere */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembersBmp, PRV_VIRTUAL_CSCD_TRUNK_DUMMY_PORT);

    /* Create virtual cascade trunk on dummy port */
    cpssDxChTrunkCascadeTrunkPortsSet(devNum,
                                      trunkWaConfig[devNum].virtualDefaultTrunkId,
                                      &portsMembersBmp);

    /* Configure device map table */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;
    cascadeLink.linkNum = trunkWaConfig[devNum].virtualDefaultTrunkId;

    /* Loop over all ports on remote device */
    for(portNum = 0; portNum < 128; portNum++)
    {
        /* Set the cascade map table for all ports on remote device */
        rc = cpssDxChCscdDevMapTableSet(devNum, hwDevNum,
                                        0, portNum,
                                        &cascadeLink,
                                        CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E,
                                        GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}
/**
* @internal appDemoLion2CheckWaTrunkMember function
* @endinternal
*
* @brief   Check that trunk member is uplink port.
*
* @param[in] devNum                   - device number.
* @param[in] memberPtr                - (pointer to) trunk member.
*
* @retval GT_TRUE                  - trunk member valid.
* @retval GT_FALSE                 - trunk member not valid
*/
static GT_BOOL appDemoLion2CheckWaTrunkMember
(
    IN GT_U8    devNum,
    IN CPSS_TRUNK_MEMBER_STC *memberPtr
)
{
    GT_HW_DEV_NUM           hwDevNum; /* HW device number */
    GT_U32 idx;

    hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    if(hwDevNum != memberPtr->hwDevice)
    {
        /* HW device not own device */
        return GT_FALSE;
    }

    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        if (trunkWaConfig[devNum].uplinkPortsMembersArr[idx] == memberPtr->port)
        {
            /* Port found in uplink ports array */
            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

/**
* @internal appDemoLion2TrunkWaInit function
* @endinternal
*
* @brief   Initialization of trunk WA.
*         Set first trunk ID and first source ID for WA usage.
* @param[in] devNum                   - device number
* @param[in] firstReservedTrunkId     - first reserved trunk ID
* @param[in] firstReservedSourceId    - first reserved Source ID
* @param[in] uplinkPortsMembersArr[APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS] - uplink ports array
* @param[in] relayPortsMembersArr[APP_DEMO_LION2_TRUNK_WA_RELAY_PORTS_NUM_CNS] - relay ports array
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Application should reserve 13 sequential trunk IDs and 12 sequential source IDs
*
*/
GT_STATUS appDemoLion2TrunkWaInit
(
    IN GT_U8 devNum,
    IN GT_TRUNK_ID firstReservedTrunkId,
    IN GT_U32 firstReservedSourceId,
    IN GT_U32 uplinkPortsMembersArr[APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS],
    IN GT_U32 relayPortsMembersArr[APP_DEMO_LION2_TRUNK_WA_RELAY_PORTS_NUM_CNS]
)
{
    GT_U32 idx, i;                      /* iterator index */
    GT_TRUNK_ID trunkId, activeTrunkId; /* current and active trunk ID for relay ports */
    CPSS_PORTS_BMP_STC  relayTrunkBmp;  /* bmp of ports members in the cascade trunk */
    GT_STATUS rc;                       /* return code */

    if (devNum >= MAX_DEV_IDS_CNS)
    {
        return GT_FAIL;
    }

    osMemSet(trunkWaConfig, 0, sizeof(trunkWaConfig)); 

    trunkWaConfig[devNum].firstReservedTrunkId = firstReservedTrunkId;

    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        trunkWaConfig[devNum].uplinkPortsMembersArr[idx] = uplinkPortsMembersArr[idx];
        trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkType = CPSS_TRUNK_TYPE_REGULAR_E;
        /* Init all uplink port trunk ID by it's relay trunk ID.
        Single uplink port doesn't have relay trunk, but default assignement is needed for database management */
        trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkId =
            (GT_TRUNK_ID)(firstReservedTrunkId + idx);
    }

    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_RELAY_PORTS_NUM_CNS; idx++)
    {
        trunkWaConfig[devNum].relayPortsMembersArr[idx] = relayPortsMembersArr[idx];

        i = idx / APP_DEMO_LION2_TRUNK_WA_RELAY_PER_UPLINK_PORTS_NUM_CNS;

        /* Set default ports members for relay  trunk */
        CPSS_PORTS_BMP_PORT_SET_MAC(&trunkWaConfig[devNum].
                                    relayTrunkMembersDb[i].origPortsMember,
                                    relayPortsMembersArr[idx]);
    }

    /* Initialize trunk members database by WA trunk ID range */
    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_MAX_NUM_CNS; idx++)
    {
        trunkId = (GT_TRUNK_ID)(firstReservedTrunkId + idx);
        activeTrunkId = trunkId + APP_DEMO_LION2_TRUNK_WA_MAX_NUM_CNS;

        trunkWaConfig[devNum].
            relayTrunkMembersDb[idx].trunkId =
        trunkWaConfig[devNum].
            relayTrunkMembersDb[idx].origTrunkId =
        trunkWaConfig[devNum].
            relayTrunkMembersDb[idx].upLinkTrunkId = trunkId;
        trunkWaConfig[devNum].
            relayTrunkMembersDb[idx].srcId = firstReservedSourceId + idx;
        trunkWaConfig[devNum].
            relayTrunkMembersDb[idx].activeTrunkId = activeTrunkId;

        /* clear the local ports bmp for the new cascade trunk and source ID group */
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&relayTrunkBmp);

        /* remove all relay ports from relay trunk */
        rc = cpssDxChTrunkCascadeTrunkPortsSet(devNum, activeTrunkId,
                                           &relayTrunkBmp);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* configure relay trunks source ID groups */
        rc = cpssDxChBrgSrcIdGroupEntrySet(devNum, activeTrunkId, GT_TRUE,
                                           &relayTrunkBmp);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* Set virtual trunk ID */
    trunkWaConfig[devNum].virtualDefaultTrunkId =
        trunkWaConfig[devNum].relayTrunkMembersDb[APP_DEMO_LION2_TRUNK_WA_MAX_NUM_CNS - 1].origTrunkId;

    return GT_OK;
}

/**
* @internal prvTrunkDbUplinkPortRelayTrunksGet function
* @endinternal
*
* @brief   Get relay cascade trunks for uplink port depending on ADD/DELETE action.
*
* @param[in] devNum                   - the device to update
* @param[in] uplinkTrunkId            - uplink trunk ID
* @param[in] portNum                  - uplink port
* @param[in] action                   - operation that curtrently done on trunk members
*
* @param[out] relayTrunkPtr            - (pointer to) relay trunk ID after uplink port is added/deleted
* @param[out] relayTrunkMembersPtr     - (pointer to) ports members for relay trunk ID after uplink port is added/deleted
* @param[out] origRelayTrunkPtr        - (pointer to) original relay trunk ID for uplink port. Always uplnk_trunk_arr[port] == relay_trunk_arr[port4]
* @param[out] origRelayTrunkMembersPtr - (pointer to) ports members for original relay trunk ID
*
* @note Example for ADD new uplink port:
*       Input:
*       UPLINK port 84 is added to UPLINK trunk A ports: 68, 73 ->
*       RELAY trunk ID X ports: [64, 65, 66, 67] [72, 74, 75, 80]
*       Output:
*       UPLINK trunk A ports: 68, 73, 84 ->
*       RELAY trunk ID X ports: [64, 65, 66, 67] [72, 74, 75, 80] [81, 82, 83, 88]
*       RELAY original trunk ID X + 2 - ports [81, 82, 83, 88]
*       Example for REMOVE uplink port:
*       Input:
*       UPLINK port 68 is removed from UPLINK trunk A ports: 68, 73, 84 ->
*       RELAY trunk ID X ports: [64, 65, 66, 67] [72, 74, 75, 80] [81, 82, 83, 88]
*       Output:
*       UPLINK trunk A ports: 73, 84 ->
*       RELAY trunk ID X + 1 ports: [72, 74, 75, 80] [81, 82, 83, 88]
*       RELAY original trunk ID X - ports [64, 65, 66, 67]
*
*/
static GT_VOID prvTrunkDbUplinkPortRelayTrunksGet
(
    IN GT_U8                        devNum,
    IN GT_TRUNK_ID                  uplinkTrunkId,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN PRV_CPSS_TRUNK_ACTION_ENT    action,
    OUT GT_TRUNK_ID               * relayTrunkPtr,
    OUT CPSS_PORTS_BMP_STC        * relayTrunkMembersPtr,
    OUT GT_TRUNK_ID               * origRelayTrunkPtr,
    OUT CPSS_PORTS_BMP_STC        * origRelayTrunkMembersPtr
)
{
    CPSS_PORTS_BMP_STC relayTrunkMembers;
    CPSS_PORTS_BMP_STC origRelayTrunkMembers;
    GT_U32 idx;
    GT_TRUNK_ID activeTrunkId = 0;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&relayTrunkMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&origRelayTrunkMembers);

    /* Set initial uplink trunk IDs */
    *relayTrunkPtr = *origRelayTrunkPtr = 0;

    /* Find trunk ID for uplink port */
    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        if (trunkWaConfig[devNum].uplinkPortsMembersArr[idx] == portNum)
        {
            /* Set original relay trunk ID */
            *origRelayTrunkPtr =
                trunkWaConfig[devNum].relayTrunkMembersDb[idx].origTrunkId;
            /* Set relay ports for uplink port */
            origRelayTrunkMembers =
                trunkWaConfig[devNum].relayTrunkMembersDb[idx].origPortsMember;

            /* Set active relay trunk */
            *relayTrunkPtr = activeTrunkId =
                trunkWaConfig[devNum].relayTrunkMembersDb[idx].activeTrunkId;

            break;
        }
    }

    /* Find uplink and relay trunk ID in all relay trunks */
    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_MAX_NUM_CNS; idx++)
    {
        if (trunkWaConfig[devNum].
            relayTrunkMembersDb[idx].upLinkTrunkId == uplinkTrunkId)
        {
            /* Set relay ports for uplink port */
            CPSS_PORTS_BMP_BITWISE_OR_MAC(&relayTrunkMembers, &relayTrunkMembers,
                &trunkWaConfig[devNum].relayTrunkMembersDb[idx].origPortsMember);

            if (*relayTrunkPtr !=
                trunkWaConfig[devNum].relayTrunkMembersDb[idx].trunkId)
            {
                *relayTrunkPtr =
                    trunkWaConfig[devNum].relayTrunkMembersDb[idx].trunkId;
            }
        }
    }

    if (action == PRV_CPSS_TRUNK_ACTION_ADD_E ||
        action == PRV_CPSS_TRUNK_ACTION_ENABLE_E)
    {
        CPSS_PORTS_BMP_BITWISE_OR_MAC(&relayTrunkMembers, &relayTrunkMembers,
                                      &origRelayTrunkMembers);
    }
    else
    {
        CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&relayTrunkMembers, &relayTrunkMembers,
                                      &origRelayTrunkMembers);
    }

    /* Relay ports members after update */
    *relayTrunkMembersPtr = relayTrunkMembers;
    /* Original relay ports members */
    *origRelayTrunkMembersPtr = origRelayTrunkMembers;
}

/**
* @internal prvRelayOriginTrunkMembersDbEntryGet function
* @endinternal
*
* @brief   Get entry of origin port trunk members from data base
*
* @param[in] devNum                   - the device to update.
* @param[in] trunkId                  - the trunk to act on.
*
* @param[out] portsMembersPtr          - (pointer to) port trunk members entry.
*/
static GT_VOID prvRelayOriginTrunkMembersDbEntryGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT CPSS_PORTS_BMP_STC   *  portsMembersPtr
)
{
    GT_U32 idx;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(portsMembersPtr);

    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_MAX_NUM_CNS; idx++)
    {
        if (trunkWaConfig[devNum].relayTrunkMembersDb[idx].origTrunkId == trunkId)
        {
            CPSS_PORTS_BMP_BITWISE_OR_MAC(portsMembersPtr, portsMembersPtr,
                                          &trunkWaConfig[devNum].
                                          relayTrunkMembersDb[idx].origPortsMember);

            break;
        }
    }
}

/**
* @internal prvTrunkMembersDbSet function
* @endinternal
*
* @brief   Set trunks data base info
*
* @param[in] devNum                   - the device to update.
* @param[in] action                   -  on trunk
* @param[in] uplinkTrunkId            - uplink trunk ID.
* @param[in] relayTrunkId             - relay trunk ID.
* @param[in] relayOrigTrunk           - relay original trunk ID.
* @param[in] uplinkPortIdx            - uplink port index.
*                                      portsMembersSetPtr - (pointer to) port trunk members to be set.
*/
static GT_VOID prvTrunkMembersDbSet
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_TRUNK_ACTION_ENT action,
    IN GT_TRUNK_ID              uplinkTrunkId,
    IN GT_TRUNK_ID              relayTrunkId,
    IN GT_TRUNK_ID              relayOrigTrunk,
    IN GT_U32                   uplinkPortIdx,
    IN CPSS_PORTS_BMP_STC     * portsMembersPtr
)
{
    if(action == PRV_CPSS_TRUNK_ACTION_ADD_E ||
       action == PRV_CPSS_TRUNK_ACTION_ENABLE_E)
    {
        /* Assign new trunk ID for new uplink port */
        trunkWaConfig[devNum].
            uplinkPortsTrunk[uplinkPortIdx].trunkId = uplinkTrunkId;
        trunkWaConfig[devNum].
            relayTrunkMembersDb[uplinkPortIdx].upLinkTrunkId = uplinkTrunkId;
        trunkWaConfig[devNum].
            relayTrunkMembersDb[uplinkPortIdx].trunkId = relayTrunkId;
    }
    else
    {
        /* Reset trunk ID for removed uplink port */
        trunkWaConfig[devNum].
            uplinkPortsTrunk[uplinkPortIdx].trunkId = relayOrigTrunk;
        trunkWaConfig[devNum].
            relayTrunkMembersDb[uplinkPortIdx].upLinkTrunkId = relayOrigTrunk;
        trunkWaConfig[devNum].
            relayTrunkMembersDb[uplinkPortIdx].trunkId = relayOrigTrunk;
    }
}

/**
* @internal prvTrunkUplinkTrunkTypeDbSet function
* @endinternal
*
* @brief   Set trunk type for uplink trunk
*
* @param[in] devNum                   - the device to update.
* @param[in] trunkId                  - the trunk to act on.
* @param[in] trunkType                - trunk type
*/
static GT_VOID prvTrunkUplinkTrunkTypeDbSet
(
    IN GT_U8                devNum,
    IN GT_TRUNK_ID          trunkId,
    IN CPSS_TRUNK_TYPE_ENT  trunkType
)
{
    GT_U32 idx;

    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        if (trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkId == trunkId)
        {
            trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkType = trunkType;
            break;
        }
    }
}

/**
* @internal prvTrunkUplinkTrunkTypeDbGet function
* @endinternal
*
* @brief   Get trunk type for uplink trunk
*
* @param[in] devNum                   - the device to update.
* @param[in] trunkId                  - the trunk to act on.
*/
static CPSS_TRUNK_TYPE_ENT prvTrunkUplinkTrunkTypeDbGet
(
    IN GT_U8                  devNum,
    IN GT_TRUNK_ID            trunkId
)
{
    GT_U32 idx;

    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        if (trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkId == trunkId)
        {
            return trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkType;
        }
    }

    return CPSS_TRUNK_TYPE_REGULAR_E;
}

/**
* @internal prvTrunkAllPortsDbGet function
* @endinternal
*
* @brief   Get uplink and relay trunk members from data base
*
* @param[in] devNum                   - the device to update.
* @param[in] trunkId                  - the trunk to act on.
* @param[in] uplinkPortsPtr           - (pointer to)uplink ports members.
* @param[in] relayPortsPtr            - (pointer to)relay ports members.
*/
static GT_VOID prvTrunkAllPortsDbGet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    OUT CPSS_PORTS_BMP_STC    * uplinkPortsPtr,
    OUT CPSS_PORTS_BMP_STC    * relayPortsPtr
)
{
    GT_U32 idx;
    GT_PHYSICAL_PORT_NUM port;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(uplinkPortsPtr);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(relayPortsPtr);

    /* Iterate over all uplink ports  */
    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        if (trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkId == trunkId)
        {
            port = trunkWaConfig[devNum].uplinkPortsMembersArr[idx];
            CPSS_PORTS_BMP_PORT_SET_MAC(uplinkPortsPtr, port);
        }
    }

    /* Iterate over all relay trunks */
    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_MAX_NUM_CNS; idx++)
    {
        if (trunkWaConfig[devNum].relayTrunkMembersDb[idx].upLinkTrunkId == trunkId)
        {
            CPSS_PORTS_BMP_BITWISE_OR_MAC(relayPortsPtr, relayPortsPtr,
               &trunkWaConfig[devNum].relayTrunkMembersDb[idx].origPortsMember);
        }
    }
}

/**
* @internal appDemoLion2TrunkUpdatedPortsGet function
* @endinternal
*
* @brief   Get added and removed ports for specified trunk
*
* @param[in] devNum                   - device number.
* @param[in] trunkId                  - trunk ID.
* @param[in] portsMembersPtr          - (pointer to) trunk member ports
*
* @param[out] addedToTrunkPortsBmpPtr  - (pointer to) added ports
* @param[out] removedFromTrunkPortsBmpPtr - (pointer to) removed ports
*/
GT_VOID appDemoLion2TrunkUpdatedPortsGet
(
    IN GT_U8        devNum,
    IN GT_TRUNK_ID  trunkId,
    IN CPSS_PORTS_BMP_STC * portsMembersPtr,
    OUT CPSS_PORTS_BMP_STC * addedToTrunkPortsBmpPtr,
    OUT CPSS_PORTS_BMP_STC * removedFromTrunkPortsBmpPtr
)
{
    CPSS_PORTS_BMP_STC fullPortsBmp; /* full ports bmp of this device */
    CPSS_PORTS_BMP_STC currentNonTrunkPortsBmp; /* current non trunk ports bmp */
    CPSS_PORTS_BMP_STC nonTrunkPorts;/* ports that not member in the trunk */
    CPSS_PORTS_BMP_STC uplinkMembersBmp;       /* bitmap of uplink ports members */
    CPSS_PORTS_BMP_STC relayMembersBmp;        /* bitmap of relay ports members */

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(addedToTrunkPortsBmpPtr);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(removedFromTrunkPortsBmpPtr);

    /* Get uplink and relay ports members from database */
    prvTrunkAllPortsDbGet(devNum, trunkId, &uplinkMembersBmp, &relayMembersBmp);

    /* Get bmp of all ports of the device */
    prvCpssDrvPortsFullMaskGet(PRV_CPSS_PP_MAC(devNum)->numOfVirtPorts,
                               &fullPortsBmp);

    if(portsMembersPtr)
    {
        /* Calculate non-trunk ports */
        CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&nonTrunkPorts,
                                           &fullPortsBmp, portsMembersPtr);
    }
    else
    {
        nonTrunkPorts = fullPortsBmp;
    }

    /* Calculate current non trunk uplink ports */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&currentNonTrunkPortsBmp,
                                       &fullPortsBmp, &uplinkMembersBmp);
    /* ports added to the trunk are ports that in HW (currentNonTrunkPortsBmp)
       are 'Not in trunk' but need to be 'In trunk' (nonTrunkPorts) */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(addedToTrunkPortsBmpPtr,
                                       &currentNonTrunkPortsBmp, &nonTrunkPorts);
    /* ports removed from the trunk are ports that need to be 'Not In trunk' (nonTrunkPorts)
       But in HW (currentNonTrunkPortsBmp) are 'in trunk'*/
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(removedFromTrunkPortsBmpPtr,
                                       &nonTrunkPorts, &currentNonTrunkPortsBmp);
}

/**
* @internal prvAppDemoLion2DeviceTableUpdate function
* @endinternal
*
* @brief   Update device map table for all ports in uplink trunk that belongs to
*         relay trunk.
* @param[in] devNum                   - the device to update
* @param[in] action                   -  applied on trunk ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvAppDemoLion2DeviceTableUpdate
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_TRUNK_ACTION_ENT action
)
{
    GT_STATUS rc = GT_OK;
    GT_PHYSICAL_PORT_NUM    portNum;            /* port number */
    GT_HW_DEV_NUM           hwDevNum;           /* HW device number */
    CPSS_CSCD_LINK_TYPE_STC cascadeLink;        /* cascade link info */
    GT_U32 idx, i;
    GT_TRUNK_ID             uplinkTrunk;        /* uplink port trunk ID */

    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        portNum = trunkWaConfig[devNum].uplinkPortsMembersArr[idx];

        cascadeLink.linkNum =
                trunkWaConfig[devNum].relayTrunkMembersDb[idx].trunkId;

        uplinkTrunk = trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkId;

        for(i = 0; i < APP_DEMO_LION2_TRUNK_WA_MAX_NUM_CNS; i++)
        {
            if (trunkWaConfig[devNum].
                relayTrunkMembersDb[i].upLinkTrunkId == uplinkTrunk)
            {
                cascadeLink.linkNum =
                        trunkWaConfig[devNum].relayTrunkMembersDb[i].trunkId;
                break;
            }
        }

        /* Configure device map table */
        hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_TRUNK_E;

        /* Set the cascade map table for uplink ports */
        rc = cpssDxChCscdDevMapTableSet(devNum, hwDevNum, 0, portNum,
                        &cascadeLink,
                        CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E,
                        GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvAppDemoLion2TrunkSrcIdUpdate function
* @endinternal
*
* @brief   Configure source IDs for cascade trunk and relay ports members
*         in mixed Cut-Through 40G and 10G configuration.
* @param[in] devNum                   - the device to update.
* @param[in] uplinkTrunkId            - uplink trunk ID.
* @param[in] trunkType                - trunk type.
* @param[in] relayTrunkId             - relay trunk ID
* @param[in] relayOrigTrunkId         - relay original trunk ID
* @param[in] action                   -   type currently done by the trunk lib.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvAppDemoLion2TrunkSrcIdUpdate
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              uplinkTrunkId,
    IN CPSS_TRUNK_TYPE_ENT      trunkType,
    IN GT_TRUNK_ID              relayTrunkId,
    IN GT_TRUNK_ID              relayOrigTrunkId,
    IN PRV_CPSS_TRUNK_ACTION_ENT action
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM portNum;  /* port number */
    GT_U32 relaySrcId;             /* source ID for new relay trunk */
    GT_U32 relayOrigSrcId;         /* source ID for origin relay trunk */
    GT_U32 relayOrigTrunkIdx;      /* trunk index for original relay trunk */
    GT_U32 idx;
    CPSS_PORTS_BMP_STC  origPortsMember;

    if (devNum >= MAX_DEV_IDS_CNS)
    {
        return GT_FAIL;
    }

    relaySrcId = relayTrunkId;

    relayOrigTrunkIdx =
        relayOrigTrunkId - trunkWaConfig[devNum].firstReservedTrunkId;
    /* Get source ID for previous relay trunk */
    relayOrigSrcId =
        trunkWaConfig[devNum].relayTrunkMembersDb[relayOrigTrunkIdx].srcId;

    /* For all relay ports with the same uplink trunk */
    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_RELAY_PORTS_NUM_CNS; idx++)
    {
        if(trunkWaConfig[devNum].
           relayTrunkMembersDb[idx / 4].upLinkTrunkId != uplinkTrunkId)
        {
            continue;
        }

        portNum = trunkWaConfig[devNum].relayPortsMembersArr[idx];

        /* Set new sorce ID for relay port */
        rc = cpssDxChBrgSrcIdPortDefaultSrcIdSet(devNum, portNum, relaySrcId);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Remove relay port from new source ID group */
        rc = cpssDxChBrgSrcIdGroupPortDelete(devNum, relaySrcId, portNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        if((idx % 4))
        {
            continue;
        }
        /* Update source group for uplink port */
        portNum = trunkWaConfig[devNum].uplinkPortsMembersArr[idx / 4];

        if(action == PRV_CPSS_TRUNK_ACTION_ADD_E ||
           action == PRV_CPSS_TRUNK_ACTION_ENABLE_E)
        {
            /* Remove uplink port from origin source ID group */
            rc = cpssDxChBrgSrcIdGroupPortDelete(devNum, relayOrigSrcId, portNum);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(trunkType == CPSS_TRUNK_TYPE_CASCADE_E)
            {
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&origPortsMember);

                /* Remove relay trunks source ID groups from origin */
                rc = cpssDxChBrgSrcIdGroupEntrySet(devNum,
                                                   relayOrigSrcId,
                                                   GT_TRUE, &origPortsMember);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

        }

        /* Add uplink port to new source ID group */
        rc = cpssDxChBrgSrcIdGroupPortAdd(devNum, relaySrcId, portNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(action == PRV_CPSS_TRUNK_ACTION_ADD_E ||
       action == PRV_CPSS_TRUNK_ACTION_ENABLE_E)
    {
        /* No more process on add action */
        return GT_OK;
    }

    /* Set sorce ID for relay ports per deleted uplink port */
    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_RELAY_PORTS_NUM_CNS; idx++)
    {
        if(trunkWaConfig[devNum].
           relayTrunkMembersDb[idx / 4].trunkId != relayOrigTrunkId)
        {
            continue;
        }

        portNum = trunkWaConfig[devNum].relayPortsMembersArr[idx];

        /* Set origin sorce ID for relay port */
        rc = cpssDxChBrgSrcIdPortDefaultSrcIdSet(devNum, portNum, relayOrigSrcId);
        if(rc != GT_OK)
        {
            return rc;
        }

        if((idx % 4))
        {
            continue;
        }

        if(trunkType != CPSS_TRUNK_TYPE_CASCADE_E)
        {
            origPortsMember =
                trunkWaConfig[devNum].relayTrunkMembersDb[idx / 4].origPortsMember;

            /* Add relay trunks source ID groups to origin */
            rc = cpssDxChBrgSrcIdGroupEntrySet(devNum,
                                               relayOrigSrcId,
                                               GT_TRUE, &origPortsMember);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* Update source group for uplink port */
        portNum = trunkWaConfig[devNum].uplinkPortsMembersArr[idx / 4];

        /* Remove uplink port from current source ID group */
        rc = cpssDxChBrgSrcIdGroupPortDelete(devNum, relaySrcId, portNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Add uplink port to origin source ID group */
        rc = cpssDxChBrgSrcIdGroupPortAdd(devNum, relayOrigSrcId, portNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvAppDemoLion2NonTrunkConfigUpdate function
* @endinternal
*
* @brief   Configure non-trunk member table for cascade trunk
*         in mixed Cut-Through 40G and 10G configuration.
* @param[in] devNum                   - the device to update.
* @param[in] action                   -   type currently done by the trunk lib.
* @param[in] trunkType                - trunk type.
* @param[in] uplinkTrunkId            - uplink trunk ID.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_NOT_FOUND             - on entry not found
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on illegal state
*/
static GT_STATUS prvAppDemoLion2NonTrunkConfigUpdate
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_TRUNK_ACTION_ENT action,
    IN CPSS_TRUNK_TYPE_ENT      trunkType,
    IN GT_TRUNK_ID              uplinkTrunkId
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC      portsMembersBmp;        /* bitmap of non trunk ports members  */
    CPSS_PORTS_BMP_STC      uplinkMembersBmp;       /* bitmap of uplink ports members */
    CPSS_PORTS_BMP_STC      relayMembersBmp;        /* bitmap of relay ports members */

    if(trunkType != CPSS_TRUNK_TYPE_CASCADE_E)
    {
        return GT_OK;
    }

    /* Get bmp of all ports of the device */
    prvCpssDrvPortsFullMaskGet(PRV_CPSS_PP_MAC(devNum)->numOfVirtPorts,
                               &portsMembersBmp);
    /* Get uplink and relay ports members */
    prvTrunkAllPortsDbGet(devNum, uplinkTrunkId, &uplinkMembersBmp, &relayMembersBmp);

    /* Add uplink ports members for virtual cascade trunk */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&portsMembersBmp, &portsMembersBmp,
                                       &uplinkMembersBmp);

    /* Add relay ports members for virtual cascade trunk */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&portsMembersBmp, &portsMembersBmp,
                                       &relayMembersBmp);

    /* Add ports to the trunk's non-trunk entry */
    rc = cpssDxChTrunkNonTrunkPortsEntrySet(devNum,
                                       trunkWaConfig[devNum].virtualDefaultTrunkId,
                                       &portsMembersBmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsTimerWkAfter(10);

    return rc;
}

/**
* @internal prvAppDemoLion2TrunkConfigUpdate function
* @endinternal
*
* @brief   Configure uplink and relay trunks, source IDs and device map table
*         for mixed Cut-Through 40G and 10G configuration.
* @param[in] devNum                   - the device to update.
* @param[in] trunkType                - trunk type.
* @param[in] uplinkTrunkId            - uplink trunk ID.
* @param[in] upLinkPortsMembersPtr    - (pointer to)uplink ports members bitmap to be added/removed.
* @param[in] action                   -   type currently done by the trunk lib.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_NOT_FOUND             - on entry not found
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on illegal state
*
* @note This function should detect uplink ports manipulations
*       and execute WA related code if need:
*       - updated relay trunks
*       - update virtual trunk in case of uplink cascade trunks
*       - update device map table entries
*       - update source IDs
*
*/
static GT_STATUS prvAppDemoLion2TrunkConfigUpdate
(
    IN GT_U8                    devNum,
    IN CPSS_TRUNK_TYPE_ENT      trunkType,
    IN GT_TRUNK_ID              uplinkTrunkId,
    IN CPSS_PORTS_BMP_STC     * upLinkPortsMembersPtr,
    IN PRV_CPSS_TRUNK_ACTION_ENT action
)
{
    GT_U32                  idx;
    GT_PHYSICAL_PORT_NUM    uplinkPortNum; /* uplink port number */
    GT_STATUS               rc;
    CPSS_PORTS_BMP_STC      relayPortsMembersDb;  /*  bitmap of relay port members in casaded trunk */
    CPSS_PORTS_BMP_STC      relayOrigPortsMembers; /* ports members in relay cascade trunk to be add/delete */
    GT_TRUNK_ID             relayTrunkId; /* cascade trunk ID */
    GT_TRUNK_ID             relayOrigTrunkId; /* cascade trunk ID to update */

    /* Iterate over all uplink ports */
    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        /* Get uplink port */
        uplinkPortNum = trunkWaConfig[devNum].uplinkPortsMembersArr[idx];

        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(upLinkPortsMembersPtr, uplinkPortNum) == 0)
        {
            continue;
        }

        /* Get relay trunks for uplink trunk port */
        prvTrunkDbUplinkPortRelayTrunksGet(devNum, uplinkTrunkId,
                                   uplinkPortNum, action,
                                   &relayTrunkId, &relayPortsMembersDb,
                                   &relayOrigTrunkId, &relayOrigPortsMembers);

        /* Update trunks database info */
        prvTrunkMembersDbSet(devNum, action, uplinkTrunkId, relayTrunkId,
                             relayOrigTrunkId, idx, &relayPortsMembersDb);

        if(action == PRV_CPSS_TRUNK_ACTION_REMOVE_E ||
           action == PRV_CPSS_TRUNK_ACTION_DISABLE_E)
        {
            /* Update non-trunk table configuration for uplink trunk */
            rc = prvAppDemoLion2NonTrunkConfigUpdate(devNum, action,
                                                     trunkType, uplinkTrunkId);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Update ports in cascade trunk */
            rc = cpssDxChTrunkCascadeTrunkPortsSet(devNum, relayTrunkId,
                                                   &relayPortsMembersDb);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Update ports of origin relay trunk */
            rc = cpssDxChTrunkCascadeTrunkPortsSet(devNum, relayOrigTrunkId,
                                                   &relayOrigPortsMembers);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Update source ID group */
            rc = prvAppDemoLion2TrunkSrcIdUpdate(devNum, uplinkTrunkId,
                                                 trunkType,
                                                 relayTrunkId, relayOrigTrunkId,
                                                 action);
            if (rc != GT_OK)
            {
                return rc;
            }

        }
        else
        {
            /* Update source ID group */
            rc = prvAppDemoLion2TrunkSrcIdUpdate(devNum, uplinkTrunkId,
                                                 trunkType,
                                                 relayTrunkId, relayOrigTrunkId,
                                                 action);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Remove ports from origin trunk */
            rc = cpssDxChTrunkCascadeTrunkPortsSet(devNum, relayOrigTrunkId, NULL);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* Update ports in cascade trunk */
            rc = cpssDxChTrunkCascadeTrunkPortsSet(devNum, relayTrunkId,
                                                   &relayPortsMembersDb);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* When port added to trunk, need to update the 'non-trunk' table
               after updating the designated table */
            rc = prvAppDemoLion2NonTrunkConfigUpdate(devNum, action,
                                                     trunkType, uplinkTrunkId);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* Update uplink trunk type in database */
    prvTrunkUplinkTrunkTypeDbSet(devNum, uplinkTrunkId, trunkType);

    /* Update device map table according to uplink ports */
    rc = prvAppDemoLion2DeviceTableUpdate(devNum, action);

    return rc;
}

/**
* @internal prvAppDemoLion2TrunkConfigWa function
* @endinternal
*
* @brief   Configure uplink and relay trunks, source IDs and device map table
*         for mixed Cut-Through 40G and 10G configuration.
* @param[in] devNum                   - the device to update.
* @param[in] trunkType                - trunk type.
* @param[in] trunkId                  - uplink trunk ID.
* @param[in] upLinkPortsMembersPtr    - (pointer to) uplink ports members bitmap to be added/removed
* @param[in] action                   -   type currently done by the trunk lib
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_NOT_FOUND             - on entry not found
* @retval GT_HW_ERROR              - on hardware error
*
* @note This function should detect uplink ports manipulations
*       and execute WA related code if need:
*       - updated relay trunks
*       - update virtual trunk
*       - update device map table entries
*       - update source IDs
*
*/
static GT_STATUS prvAppDemoLion2TrunkConfigWa
(
    IN GT_U8                    devNum,
    IN CPSS_TRUNK_TYPE_ENT      trunkType,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC     * upLinkPortsMembersPtr,
    IN PRV_CPSS_TRUNK_ACTION_ENT action
)
{
    GT_STATUS rc;

    if (CPSS_PORTS_BMP_IS_ZERO_MAC(upLinkPortsMembersPtr))
    {
        /* No uplink ports to update */
        return GT_OK;
    }

    /* Update all uplink trunk related common configuration */
    rc = prvAppDemoLion2TrunkConfigUpdate(devNum, trunkType, trunkId,
                                          upLinkPortsMembersPtr, action);

    return rc;
}

/**
* @internal appDemoLion2TrunkCascadeTrunkPortsSet function
* @endinternal
*
* @brief   The function checks does input parameters belong to uplink ports and
*         configures relay port trunks and source ID accordingly.
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
*/
GT_STATUS appDemoLion2TrunkCascadeTrunkPortsSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC addedToTrunkPortsBmp;    /* ports added to the trunk */
    CPSS_PORTS_BMP_STC removedFromTrunkPortsBmp; /* ports removed from the trunk */
    GT_U32 portNum, idx;

    for (portNum = 0; portNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portNum++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsMembersPtr, portNum) == 0)
        {
            continue;
        }
        /* Find trunk ID for uplink port */
        for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
        {
            if (trunkWaConfig[devNum].uplinkPortsMembersArr[idx] == portNum)
            {
                /* Port is uplink */
                break;
            }
        }

        if(idx == APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS)
        {
            /* Port is not uplink */
            return GT_OK;
        }
    }

    appDemoLion2TrunkUpdatedPortsGet(devNum, trunkId, portsMembersPtr,
                                     &addedToTrunkPortsBmp, &removedFromTrunkPortsBmp);


    if(0 == CPSS_PORTS_BMP_IS_ZERO_MAC(&addedToTrunkPortsBmp))
    {
        rc = prvAppDemoLion2TrunkConfigWa(devNum, CPSS_TRUNK_TYPE_CASCADE_E,
                                          trunkId, &addedToTrunkPortsBmp,
                                          PRV_CPSS_TRUNK_ACTION_ADD_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(0 == CPSS_PORTS_BMP_IS_ZERO_MAC(&removedFromTrunkPortsBmp))
    {
        rc = prvAppDemoLion2TrunkConfigWa(devNum, CPSS_TRUNK_TYPE_CASCADE_E,
                                          trunkId, &removedFromTrunkPortsBmp,
                                          PRV_CPSS_TRUNK_ACTION_REMOVE_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoLion2TrunkMembersSet function
* @endinternal
*
* @brief   The function checks does input parameters belong to uplink ports and
*         configures relay port trunks and source ID accordingly.
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] enabledMembersArray[]    - (array of) members to set in this trunk as enabled
*                                      members .
*                                      (this parameter ignored if numOfEnabledMembers = 0)
* @param[in] numOfEnabledMembers      - number of enabled members in the array.
* @param[in] disabledMembersArray[]   - (array of) members to set in this trunk as disabled
*                                      members .
*                                      (this parameter ignored if numOfDisabledMembers = 0)
* @param[in] numOfDisabledMembers     - number of disabled members in the array.
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
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoLion2TrunkMembersSet
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
)
{
    GT_STATUS rc;
    GT_BOOL retVal;
    CPSS_PORTS_BMP_STC addedToTrunkPortsBmp;    /* ports added to the trunk */
    CPSS_PORTS_BMP_STC removedFromTrunkPortsBmp; /* ports removed from the trunk */
    GT_U32 idx;
    CPSS_TRUNK_TYPE_ENT trunkType;
    CPSS_PORTS_BMP_STC uplinkMembersBmp;       /* bitmap of uplink ports members */
    CPSS_PORTS_BMP_STC relayMembersBmp;        /* bitmap of relay ports members */

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&addedToTrunkPortsBmp);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&removedFromTrunkPortsBmp);

    for(idx = 0; idx < numOfEnabledMembers; idx++)
    {
        retVal = appDemoLion2CheckWaTrunkMember(devNum, &enabledMembersArray[idx]);
        if(retVal == GT_FALSE)
        {
            return GT_OK;
        }
    }

    for(idx = 0; idx < numOfDisabledMembers; idx++)
    {
        retVal = appDemoLion2CheckWaTrunkMember(devNum, &disabledMembersArray[idx]);
        if(retVal == GT_FALSE)
        {
            return GT_OK;
        }
    }

    /* Get uplink and relay ports members */
    prvTrunkAllPortsDbGet(devNum, trunkId, &uplinkMembersBmp, &relayMembersBmp);

    trunkType = prvTrunkUplinkTrunkTypeDbGet(devNum, trunkId);

    for(idx = 0; idx < numOfEnabledMembers; idx++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&addedToTrunkPortsBmp,
                                    enabledMembersArray[idx].port);
    }

    /* New trunk ports different from old one */
    if(0 == CPSS_PORTS_BMP_ARE_EQUAL_MAC(&addedToTrunkPortsBmp, &uplinkMembersBmp))
    {
        /* Remove all ports from trunk */
        rc = prvAppDemoLion2TrunkConfigWa(devNum, trunkType, trunkId,
                                          &uplinkMembersBmp,
                                          PRV_CPSS_TRUNK_ACTION_REMOVE_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(0 == CPSS_PORTS_BMP_IS_ZERO_MAC(&addedToTrunkPortsBmp))
    {
        rc = prvAppDemoLion2TrunkConfigWa(devNum, trunkType, trunkId,
                                          &addedToTrunkPortsBmp,
                                          PRV_CPSS_TRUNK_ACTION_ADD_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    for(idx = 0; idx < numOfDisabledMembers; idx++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&removedFromTrunkPortsBmp,
                                    disabledMembersArray[idx].port);
    }

    if(0 == CPSS_PORTS_BMP_IS_ZERO_MAC(&removedFromTrunkPortsBmp))
    {
        rc = prvAppDemoLion2TrunkConfigWa(devNum, trunkType, trunkId,
                                          &removedFromTrunkPortsBmp,
                                          PRV_CPSS_TRUNK_ACTION_REMOVE_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoLion2TrunkMemberAdd function
* @endinternal
*
* @brief   The function checks does input parameters belong to uplink ports and
*         configures relay port trunks and source ID accordingly.
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
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
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
GT_STATUS appDemoLion2TrunkMemberAdd
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC addedToTrunkPortsBmp;    /* ports added to the trunk */
    GT_BOOL retVal;
    CPSS_TRUNK_TYPE_ENT trunkType;

    retVal = appDemoLion2CheckWaTrunkMember(devNum, memberPtr);
    if(retVal == GT_FALSE)
    {
        return GT_OK;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&addedToTrunkPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&addedToTrunkPortsBmp, memberPtr->port);

    trunkType = prvTrunkUplinkTrunkTypeDbGet(devNum, trunkId);

    rc = prvAppDemoLion2TrunkConfigWa(devNum, trunkType, trunkId,
                                      &addedToTrunkPortsBmp,
                                      PRV_CPSS_TRUNK_ACTION_ADD_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoLion2TrunkMemberDisable function
* @endinternal
*
* @brief   The function checks does input parameters belong to uplink ports and
*         configures relay port trunks and source ID accordingly.
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
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoLion2TrunkMemberDisable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC removedFromTrunkPortsBmp; /* ports removed from the trunk */
    GT_BOOL retVal;
    CPSS_TRUNK_TYPE_ENT trunkType;


    retVal = appDemoLion2CheckWaTrunkMember(devNum, memberPtr);
    if(retVal == GT_FALSE)
    {
        return GT_OK;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&removedFromTrunkPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&removedFromTrunkPortsBmp, memberPtr->port);

    trunkType = prvTrunkUplinkTrunkTypeDbGet(devNum, trunkId);

    rc = prvAppDemoLion2TrunkConfigWa(devNum, trunkType, trunkId,
                                      &removedFromTrunkPortsBmp,
                                      PRV_CPSS_TRUNK_ACTION_REMOVE_E);
    return rc;
}

/**
* @internal appDemoLion2TrunkMemberEnable function
* @endinternal
*
* @brief   The function checks does input parameters belong to uplink ports and
*         configures relay port trunks and source ID accordingly.
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
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoLion2TrunkMemberEnable
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC addedToTrunkPortsBmp;    /* ports added to the trunk */
    GT_BOOL retVal;
    CPSS_TRUNK_TYPE_ENT trunkType;

    retVal = appDemoLion2CheckWaTrunkMember(devNum, memberPtr);
    if(retVal == GT_FALSE)
    {
        return GT_OK;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&addedToTrunkPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&addedToTrunkPortsBmp, memberPtr->port);

    trunkType = prvTrunkUplinkTrunkTypeDbGet(devNum, trunkId);

    rc = prvAppDemoLion2TrunkConfigWa(devNum, trunkType, trunkId,
                                      &addedToTrunkPortsBmp,
                                      PRV_CPSS_TRUNK_ACTION_ADD_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoLion2TrunkMemberRemove function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad member parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoLion2TrunkMemberRemove
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
)
{
    GT_STATUS rc;
    CPSS_PORTS_BMP_STC removedFromTrunkPortsBmp; /* ports removed from the trunk */
    GT_BOOL retVal;
    CPSS_TRUNK_TYPE_ENT trunkType;

    retVal = appDemoLion2CheckWaTrunkMember(devNum, memberPtr);
    if(retVal == GT_FALSE)
    {
        return GT_OK;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&removedFromTrunkPortsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&removedFromTrunkPortsBmp, memberPtr->port);

    trunkType = prvTrunkUplinkTrunkTypeDbGet(devNum, trunkId);

    rc = prvAppDemoLion2TrunkConfigWa(devNum, trunkType, trunkId,
                                      &removedFromTrunkPortsBmp,
                                      PRV_CPSS_TRUNK_ACTION_REMOVE_E);
    return rc;
}

GT_STATUS appDemoLion2TrunkConfigDump
(
    IN GT_U8 devNum
)
{
    CPSS_PORTS_BMP_STC  uplinkMembersBmp;       /* bitmap of uplink ports members */
    CPSS_PORTS_BMP_STC  relayMembersBmp;        /* bitmap of relay ports members */
    CPSS_PORTS_BMP_STC  membersBmp;             /* bitmap of ports members in the trunk */
    CPSS_PORTS_BMP_STC  uplinkVirtualMembersBmp;/* bitmap of virtual uplink ports members */
    CPSS_PORTS_BMP_STC  relayVirtualMembersBmp; /* bitmap of virtual relay ports members */
    GT_TRUNK_ID         trunkId;                /* trunk ID */
    GT_U32              srcId;                  /* source ID */
    GT_BOOL             cpu;
    GT_U32              idx;
    GT_STATUS           rc;
    CPSS_CSCD_LINK_TYPE_STC cascadeLink;        /* cascade link info */
    CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT cscdTrunkHashEn;
    GT_BOOL             egressAttributesLocallyEn;
    GT_HW_DEV_NUM       hwDevNum;
    GT_PHYSICAL_PORT_NUM portNum;               /* port number */
    GT_BOOL             isLinkUp;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT speed;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&uplinkVirtualMembersBmp);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&relayVirtualMembersBmp);

    trunkId = 0;

    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        prvRelayOriginTrunkMembersDbEntryGet(devNum,
                     trunkWaConfig[devNum].relayTrunkMembersDb[idx].origTrunkId,
                     &relayMembersBmp);

        cpssOsPrintf("Relay origin trunk %d words 0-3\n",
                     trunkWaConfig[devNum].relayTrunkMembersDb[idx].origTrunkId);
        cpssOsPrintf(" %08X %08X %08X %08X\n",
                     relayMembersBmp.ports[0],
                     relayMembersBmp.ports[1],
                     relayMembersBmp.ports[2],
                     relayMembersBmp.ports[3]);
    }
    cpssOsPrintf(" ==================================\n");

    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        if(trunkId != trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkId &&
                    (trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkId !=
                     trunkWaConfig[devNum].relayTrunkMembersDb[idx].trunkId))
        {
            trunkId = trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkId;

            prvTrunkAllPortsDbGet(devNum, trunkId, &uplinkMembersBmp, &relayMembersBmp);

            /* Print uplink trunks */
            cpssOsPrintf("Uplink trunk %d type %d words 0-3\n",
                         trunkId,
                         trunkWaConfig[devNum].uplinkPortsTrunk[idx].trunkType);
            cpssOsPrintf(" %08X %08X %08X %08X\n",
                         uplinkMembersBmp.ports[0],
                         uplinkMembersBmp.ports[1],
                         uplinkMembersBmp.ports[2],
                         uplinkMembersBmp.ports[3]);

            cpssOsPrintf("Relay trunk %d words 0-3\n",
                         trunkWaConfig[devNum].relayTrunkMembersDb[idx].trunkId);
            cpssOsPrintf(" %08X %08X %08X %08X\n",
                         relayMembersBmp.ports[0],
                         relayMembersBmp.ports[1],
                         relayMembersBmp.ports[2],
                         relayMembersBmp.ports[3]);

            if(trunkWaConfig[devNum].
               uplinkPortsTrunk[idx].trunkType == CPSS_TRUNK_TYPE_CASCADE_E)
            {
                uplinkVirtualMembersBmp.ports[0] =
                    relayMembersBmp.ports[0] | uplinkMembersBmp.ports[0];
                uplinkVirtualMembersBmp.ports[1] =
                    relayMembersBmp.ports[1] | uplinkMembersBmp.ports[1];
                uplinkVirtualMembersBmp.ports[2] =
                    relayMembersBmp.ports[2] | uplinkMembersBmp.ports[2];
                uplinkVirtualMembersBmp.ports[3] =
                    relayMembersBmp.ports[3] | uplinkMembersBmp.ports[3];
            }
        }
    }
    cpssOsPrintf(" ==================================\n");

    cpssOsPrintf("Virtual trunk ID %d\n",
                 trunkWaConfig[devNum].virtualDefaultTrunkId);
    cpssOsPrintf(" %08X %08X %08X %08X\n",
                 uplinkVirtualMembersBmp.ports[0],
                 uplinkVirtualMembersBmp.ports[1],
                 uplinkVirtualMembersBmp.ports[2],
                 uplinkVirtualMembersBmp.ports[3]);
    cpssOsPrintf(" ==================================\n");

    /* Print source ID table for relay trunks */
    cpssOsPrintf(" Source ID table : \n");

    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_MAX_NUM_CNS; idx++)
    {
        srcId = trunkWaConfig[devNum].relayTrunkMembersDb[idx].srcId;
        rc = cpssDxChBrgSrcIdGroupEntryGet(devNum, srcId, &cpu, &membersBmp);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Read entry [%d] error %d\n", idx, rc);
        }

        cpssOsPrintf(" %2d %d %08X %08X %08X %08X\n", srcId, cpu,
                     membersBmp.ports[0], membersBmp.ports[1],
                     membersBmp.ports[2], membersBmp.ports[3]);

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&membersBmp);

        srcId = trunkWaConfig[devNum].relayTrunkMembersDb[idx].activeTrunkId;

        rc = cpssDxChBrgSrcIdGroupEntryGet(devNum, srcId, &cpu, &membersBmp);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Read entry [%d] error %d\n", idx, rc);
        }

        if (CPSS_PORTS_BMP_IS_ZERO_MAC(&membersBmp) == 0)
        {
            cpssOsPrintf(" %2d %d %08X %08X %08X %08X\n", srcId, cpu,
                         membersBmp.ports[0], membersBmp.ports[1],
                         membersBmp.ports[2], membersBmp.ports[3]);
        }
    }
    cpssOsPrintf(" ==================================\n");

    /* Print device map table */
    cpssOsPrintf(" Device %d map table \n", devNum);

    /* Configure device map table */
    hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    /* Loop over all ports on remote device */
    for(portNum = 0; portNum < 128; portNum++)
    {
        /* Set the cascade map table for all ports on remote device */
        rc = cpssDxChCscdDevMapTableGet(devNum, hwDevNum,
                                        0, portNum,
                                        &cascadeLink,
                                        &cscdTrunkHashEn,
                                        &egressAttributesLocallyEn);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(cascadeLink.linkNum != 0)
        {
            cpssOsPrintf("Target Port %3d Trunk ID %3d\n", portNum, cascadeLink.linkNum);
        }
    }
    cpssOsPrintf(" ==================================\n");

    /* loop over all ports */
    for(portNum = 0; portNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portNum++)
    {
        /* skip not existed ports */
        if(! PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum, portNum))
            continue;

        /* skip CPU port */
        if(portNum == CPSS_CPU_PORT_NUM_CNS)
            continue;

        rc = cpssDxChPortSpeedGet(devNum, portNum, &speed);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPortLinkStatusGet(devNum, portNum, &isLinkUp);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChBrgSrcIdPortDefaultSrcIdGet(devNum, portNum, &srcId);
        if(rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("Port %3d Speed %2d Interface %2d Link %2d Source ID %d\n",
                     portNum, speed, ifMode, isLinkUp, srcId);
    }

    cpssOsPrintf(" ==================================\n");

    return GT_OK;
}

GT_STATUS appDemoLion2SrcIdTrunkNonTrunkCheck
(
    IN GT_U8   devNum,
    IN PRV_CPSS_TRUNK_ACTION_ENT action
)
{
    GT_U32 idx;
    GT_STATUS           rc;
    CPSS_PORTS_BMP_STC  nonTrunkMembersBmp;     /* non-trunk bitmap ports members for virtual trunk */
    CPSS_PORTS_BMP_STC  designatedMembersBmp;    /* designated bitmap ports members for current relay trunk */
    CPSS_PORTS_BMP_STC  srcIdMembersBmp;        /* source ID bitmap ports members for current relay trunk */
    CPSS_PORTS_BMP_STC  finalDesignatedMembersBmp;    /* final designated bitmap ports members for current relay trunk */
    GT_U32              srcId;                  /* source ID */
    GT_BOOL             cpu;
    GT_U32              ii;
    GT_U32              numOfSetPorts;

    if (devNum >= MAX_DEV_IDS_CNS)
    {
        return GT_FAIL;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&nonTrunkMembersBmp);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&designatedMembersBmp);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&srcIdMembersBmp);

    /* Get trunk's non-trunk entry */
    rc = cpssDxChTrunkNonTrunkPortsEntryGet(devNum, 16, &nonTrunkMembersBmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Check designated ports */
    for(idx = 0; idx < 64; idx++)
    {
        rc = cpssDxChTrunkDesignatedPortsEntryGet(devNum, idx, &designatedMembersBmp);
        if(rc != GT_OK)
        {
            return GT_OK;
        }

        CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&designatedMembersBmp,
                                           &designatedMembersBmp,
                                           &nonTrunkMembersBmp);
        numOfSetPorts = 0;
        for(ii = 0; ii < CPSS_MAX_PORTS_NUM_CNS; ii++)
        {
            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&designatedMembersBmp, ii))
            {
                continue;
            }

            numOfSetPorts++;
        }
        if(numOfSetPorts > 2)
        {
            cpssOsPrintf("Designated entry %2d %08X %08X %08X %08X\n", idx,
                         designatedMembersBmp.ports[0],
                         designatedMembersBmp.ports[1],
                         designatedMembersBmp.ports[2],
                         designatedMembersBmp.ports[3]);

            return GT_FAIL;
        }
    }

    /* Check relay ports source ID */
    for(idx = 0; idx < APP_DEMO_LION2_TRUNK_WA_UPLINK_PORTS_NUM_CNS; idx++)
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&finalDesignatedMembersBmp);

        srcId = trunkWaConfig[devNum].relayTrunkMembersDb[idx].activeTrunkId;

        rc = cpssDxChBrgSrcIdGroupEntryGet(devNum, srcId, &cpu, &srcIdMembersBmp);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (CPSS_PORTS_BMP_IS_ZERO_MAC(&srcIdMembersBmp) == 0)
        {
            CPSS_PORTS_BMP_BITWISE_AND_MAC(&finalDesignatedMembersBmp,
                                           &designatedMembersBmp,
                                           &srcIdMembersBmp);
            numOfSetPorts = 0;
            for(ii = 0; ii < CPSS_MAX_PORTS_NUM_CNS; ii++)
            {
                if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&finalDesignatedMembersBmp, ii))
                {
                    continue;
                }

                numOfSetPorts++;
            }
            if(numOfSetPorts > 1)
            {
                cpssOsPrintf("Relay source ID %2d %d %08X %08X %08X %08X\n", srcId, cpu,
                             finalDesignatedMembersBmp.ports[0],
                             finalDesignatedMembersBmp.ports[1],
                             finalDesignatedMembersBmp.ports[2],
                             finalDesignatedMembersBmp.ports[3]);

                return GT_FAIL;
            }
        }
    }

    /* Check network port source ID */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&finalDesignatedMembersBmp);

    srcId = 1;
    rc = cpssDxChBrgSrcIdGroupEntryGet(devNum, srcId, &cpu, &srcIdMembersBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_PORTS_BMP_BITWISE_AND_MAC(&finalDesignatedMembersBmp,
                                   &designatedMembersBmp, &srcIdMembersBmp);
    numOfSetPorts = 0;
    for(ii = 0; ii < CPSS_MAX_PORTS_NUM_CNS; ii++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&finalDesignatedMembersBmp, ii))
        {
            continue;
        }

        numOfSetPorts++;
    }
    if(numOfSetPorts > 1)
    {
        cpssOsPrintf("Network port source ID %2d %d %08X %08X %08X %08X\n", srcId, cpu,
                     finalDesignatedMembersBmp.ports[0],
                     finalDesignatedMembersBmp.ports[1],
                     finalDesignatedMembersBmp.ports[2],
                     finalDesignatedMembersBmp.ports[3]);

        return GT_FAIL;
    }

    return GT_OK;
}



