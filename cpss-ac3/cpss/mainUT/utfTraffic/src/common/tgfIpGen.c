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
* @file tgfIpGen.c
*
* @brief Generic API implementation for IP
*
* @version   96
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfIpGen.h>
#include <common/tgfLpmGen.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* default Virtual Router ID */
static GT_U32        prvTgfDefVrId = 0;

#ifdef EXMXPM_FAMILY
    /* default IPv6 MC group rule index */
    static GT_U32        prvTgfIpv6McGroupRuleIndex  = 8;
#endif /* EXMXPM_FAMILY */

/* default LPM DB and Virtual Router configuration */
static struct
{
    GT_BOOL                                     isSupportIpv4Uc;
    GT_BOOL                                     isSupportIpv6Uc;
    GT_BOOL                                     isSupportIpv4Mc;
    GT_BOOL                                     isSupportIpv6Mc;
    GT_BOOL                                     isSupportFcoe;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv6UcNextHopInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv4McRouteLttEntry;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv6McRouteLttEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defFcoeNextHopInfo;
    PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT             shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC  indexesRange;
    GT_BOOL                                     partitionEnable;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC  tcamLpmManagerCapcityCfg;
    GT_BOOL                                     isTcamManagerInternal;
} prvTgfDefLpmDBVrCfg;

/* Flag whether to make IP validation (default: GT_TRUE) */
static GT_BOOL prvTgfIpValidityCheckFlag = GT_TRUE;

extern GT_STATUS appDemoBc2IpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);

extern GT_STATUS appDemoFalconIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT      sharedTableMode,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);


/******************************************************************************\
 *                             Define section                                 *
\******************************************************************************/

/* total number of counters */
#define PRV_TGF_MAX_COUNTER_NUM_CNS    35

/* max counter field's name length */
#define PRV_TGF_MAX_FIELD_NAME_LEN_CNS 25

/* max number of LPM blocks on eArch architecture */
#define PRV_TGF_MAX_NUM_OF_LPM_BLOCKS_CNS    20

/* return number of LPM lines in ram according to number of prefixes supported.
   We use the rule of average 10Bytes for Ipv4 prefix and 20Bytes for Ipv6 prefix.
   each line holds 4 byte so one prefix hold 10 Bytes = 2.5 Lines. */
#define PRV_TGF_LPM_RAM_GET_NUM_OF_LINES_MAC(_numOfPrefixes)        \
    ((GT_U32)(_numOfPrefixes*2.5))

/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericToDxChIpUcRouteEntry function
* @endinternal
*
* @brief   Convert generic IP UC route entry into device specific IP UC route entry
*
* @param[in] ipUcRouteEntryPtr        - (pointer to) IP UC route entry
*
* @param[out] dxChIpUcRouteEntryPtr    - (pointer to) DxCh IP UC route entry
*
* @retval GT_OK                    -  on OK
* @retval GT_BAD_PARAM             -  Invalid device id
*/
static GT_STATUS prvTgfConvertGenericToDxChIpUcRouteEntry
(
    IN  PRV_TGF_IP_UC_ROUTE_ENTRY_STC            *ipUcRouteEntryPtr,
    OUT CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC   *dxChIpUcRouteEntryPtr
)
{
    GT_STATUS        rc = GT_OK; /* return code */

    /* convert IP UC route entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, cmd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, appSpecificCpuCodeEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, unicastPacketSipFilterEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ingressMirror);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, countSet);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, trapMirrorArpBcEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, sipAccessLevel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, dipAccessLevel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ICMPRedirectEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, scopeCheckingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, siteId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, mtuProfileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopVlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ttlHopLimitDecEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ttlHopLimDecOptionsExtChkByPass);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosProfileMarkingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosProfileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosPrecedence);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, modifyUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, modifyDscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, isTunnelStart);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, isNat);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopARPPointer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopTunnelPointer);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopNatPointer);

    dxChIpUcRouteEntryPtr->cpuCodeIdx = ipUcRouteEntryPtr->cpuCodeIndex;

    cpssOsMemCpy((GT_VOID*) &(dxChIpUcRouteEntryPtr->nextHopInterface),
                 (GT_VOID*) &(ipUcRouteEntryPtr->nextHopInterface),
                 sizeof(ipUcRouteEntryPtr->nextHopInterface));

    if( CPSS_INTERFACE_PORT_E == ipUcRouteEntryPtr->nextHopInterface.type )
    {
        /* need to remove casting and to fix code after lion2 development is done */
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvUtfHwDeviceNumberGet((GT_U8)ipUcRouteEntryPtr->nextHopInterface.devPort.hwDevNum,
                                     &dxChIpUcRouteEntryPtr->nextHopInterface.devPort.hwDevNum);
        if (rc != GT_OK)
        {
            /* ignore NOT OK.
              Some tests uses not existing devNum and it's OK for tests */
            rc = GT_OK;
        }
    }

    return rc;
}

/**
* @internal prvTgfConvertDxChToGenericIpUcRouteEntry function
* @endinternal
*
* @brief   Convert device specific IP UC route entry into generic IP UC route entry
*
* @param[in] dxChIpUcRouteEntryPtr    - (pointer to) DxCh IP UC route entry
*
* @param[out] ipUcRouteEntryPtr        - (pointer to) IP UC route entry
*                                       None
*/
static GT_VOID prvTgfConvertDxChToGenericIpUcRouteEntry
(
    IN  CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC   *dxChIpUcRouteEntryPtr,
    OUT PRV_TGF_IP_UC_ROUTE_ENTRY_STC            *ipUcRouteEntryPtr
)
{
    /* convert IP UC route entry from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, cmd);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, appSpecificCpuCodeEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, unicastPacketSipFilterEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ingressMirror);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, countSet);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, trapMirrorArpBcEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, sipAccessLevel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, dipAccessLevel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ICMPRedirectEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, scopeCheckingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, siteId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, mtuProfileIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopVlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ttlHopLimitDecEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, ttlHopLimDecOptionsExtChkByPass);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosProfileMarkingEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosProfileIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, qosPrecedence);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, modifyUp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, modifyDscp);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, isTunnelStart);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, isNat);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopARPPointer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopTunnelPointer);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopNatPointer);

    ipUcRouteEntryPtr->cpuCodeIndex = dxChIpUcRouteEntryPtr->cpuCodeIdx;

    cpssOsMemCpy((GT_VOID*) &(ipUcRouteEntryPtr->nextHopInterface),
                 (GT_VOID*) &(dxChIpUcRouteEntryPtr->nextHopInterface),
                 sizeof(dxChIpUcRouteEntryPtr->nextHopInterface));

    if( CPSS_INTERFACE_PORT_E == dxChIpUcRouteEntryPtr->nextHopInterface.type )
    {
        prvUtfSwFromHwDeviceNumberGet(dxChIpUcRouteEntryPtr->nextHopInterface.devPort.hwDevNum,
                                      &ipUcRouteEntryPtr->nextHopInterface.devPort.hwDevNum);
    }
}

/**
* @internal prvTgfConvertGenericToDxChIpMcRouteEntry function
* @endinternal
*
* @brief   Convert generic IP MC route entry into device specific IP UC route entry
*
* @param[in] protocol                 -  ip  type
* @param[in] ipMcRouteEntryPtr        - (pointer to) IP MC route entry
*
* @param[out] dxChIpMcRouteEntryPtr    - (pointer to) DxCh IP MC route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIpMcRouteEntry
(
    IN CPSS_IP_PROTOCOL_STACK_ENT                protocol,
    IN  PRV_TGF_IP_MC_ROUTE_ENTRY_STC            *ipMcRouteEntryPtr,
    OUT CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC          *dxChIpMcRouteEntryPtr
)
{
    /* convert IP MC route entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, cmd);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, appSpecificCpuCodeEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, ttlHopLimitDecEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, ttlHopLimDecOptionsExtChkByPass);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, ingressMirror);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, qosProfileMarkingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, qosProfileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, qosPrecedence);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, modifyUp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, modifyDscp);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, countSet);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, multicastRPFCheckEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, multicastRPFVlan);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, RPFFailCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, scopeCheckingEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, siteId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChIpMcRouteEntryPtr, ipMcRouteEntryPtr, mtuProfileIndex);
    if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        dxChIpMcRouteEntryPtr->internalMLLPointer = ipMcRouteEntryPtr->ipv4MllPtr;

    }
    else
    {
        dxChIpMcRouteEntryPtr->externalMLLPointer = ipMcRouteEntryPtr->ipv6ExternalMllPtr;
        dxChIpMcRouteEntryPtr->internalMLLPointer = ipMcRouteEntryPtr->ipv6InternalMllPtr;
    }
    dxChIpMcRouteEntryPtr->cpuCodeIdx = ipMcRouteEntryPtr->cpuCodeIndex;

    /* convert mcRPFFailCmdMode into device specific format */
    switch (ipMcRouteEntryPtr->mcRPFFailCmdMode)
    {
        case PRV_TGF_IP_MC_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E:
            dxChIpMcRouteEntryPtr->multicastRPFFailCommandMode = CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E;
            break;

        case PRV_TGF_IP_MC_MLL_RPF_FAIL_COMMAND_MODE_E:
            dxChIpMcRouteEntryPtr->multicastRPFFailCommandMode = CPSS_DXCH_IP_MULTICAST_MLL_RPF_FAIL_COMMAND_MODE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChRouteType function
* @endinternal
*
* @brief   Convert generic route type into device specific route type
*
* @param[in] routeType                - route type
*
* @param[out] dxChRouteTypePtr         - (pointer to) DxCh route type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChRouteType
(
    IN  PRV_TGF_IP_ROUTE_ENTRY_METHOD_ENT       routeType,
    OUT CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT    *dxChRouteTypePtr
)
{
    /* convert routeType into device specific format */
    switch (routeType)
    {
        case PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E:
            *dxChRouteTypePtr = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
            break;

        case PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E:
            *dxChRouteTypePtr = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E;
            break;

        case PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E:
            *dxChRouteTypePtr = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
            break;
        case PRV_TGF_IP_MULTIPATH_ROUTE_ENTRY_GROUP_E:
            *dxChRouteTypePtr = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericRouteType function
* @endinternal
*
* @brief   Convert device specific route type into generic route type
*
* @param[in] dxChRouteType            - DxCh route type
*
* @param[out] routeTypePtr             - (pointer to) route type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericRouteType
(
    IN  CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT     dxChRouteType,
    OUT PRV_TGF_IP_ROUTE_ENTRY_METHOD_ENT      *routeTypePtr
)
{
    /* convert routeType from device specific format */
    switch (dxChRouteType)
    {
        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E:
            *routeTypePtr = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
            break;

        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E:
            *routeTypePtr = PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E;
            break;

        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E:
            *routeTypePtr = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChUcNextHopInfo function
* @endinternal
*
* @brief   Convert generic uc next hop info into DX specific uc next hop info
*
* @param[in] routingMode              - routing mode
* @param[in] defUcNextHopInfoPtr      - (pointer to)  generic uc next hop info
*
* @param[out] dxChDefUcNextHopInfoPtr  - (pointer to) DX uc next hop info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChUcNextHopInfo
(
    IN  PRV_TGF_IP_ROUTING_MODE_ENT            routingMode,
    IN  PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *defUcNextHopInfoPtr,
    OUT CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *dxChDefUcNextHopInfoPtr
)
{
    GT_STATUS rc = GT_OK;
    switch (routingMode)
    {
    case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                       &defUcNextHopInfoPtr->pclIpUcAction,
                                       pktCmd);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                       &defUcNextHopInfoPtr->pclIpUcAction,
                                       actionStop);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                       &defUcNextHopInfoPtr->pclIpUcAction,
                                       bypassBridge);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                       &defUcNextHopInfoPtr->pclIpUcAction,
                                       bypassIngressPipe);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                       &defUcNextHopInfoPtr->pclIpUcAction,
                                       egressPolicy);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       ipclConfigIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       pcl0_1OverrideConfigIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                       pcl1OverrideConfigIndex);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       cpuCode);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       mirrorTcpRstAndFinPacketsToCpu);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       mirrorToRxAnalyzerPort);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                       ingressMirrorToAnalyzerIndex);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                       &defUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                       enableMatchCount);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                       &defUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                       matchCounterIndex);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                       modifyUp);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                       modifyDscp);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                       profileAssignIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                       profileIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                       profilePrecedence);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect,
                                       &defUcNextHopInfoPtr->pclIpUcAction.redirect,
                                       redirectCmd);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       routerLttIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       routerLttIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                       vrfId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.policer,
                                       &defUcNextHopInfoPtr->pclIpUcAction.policer,
                                       policerId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.policer,
                                       &defUcNextHopInfoPtr->pclIpUcAction.policer,
                                       policerEnable);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                       &defUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                       assignSourceId);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                       &defUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                       sourceIdValue);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       arpDaIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       bypassTTLCheck);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       decrementTTL);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       doIpUcRoute);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                       icmpRedirectCheck);

        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                       modifyVlan);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                       nestedVlan);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                       precedence);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                       &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                       vlanId);
        break;
    case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
        /* set route type */
        rc = prvTgfConvertGenericToDxChRouteType(defUcNextHopInfoPtr->ipLttEntry.routeType,
                                                 &dxChDefUcNextHopInfoPtr->ipLttEntry.routeType);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);

            return rc;
        }
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       numOfPaths);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       routeEntryBaseIndex);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       ucRPFCheckEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       sipSaCheckMismatchEnable);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       ipv6MCGroupScopeLevel);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                       &defUcNextHopInfoPtr->ipLttEntry,
                                       priority);

        break;
    default:
            return GT_BAD_PARAM;
    }

    return rc;
}


/**
* @internal prvTgfConvertGenericToDxChMcLttEntryInfo function
* @endinternal
*
* @brief   Convert generic mc LTT info into DX specific mc LTT info
*
* @param[in] defMcRouteLttEntryPtr    - (pointer to) generic mc LTT info
*
* @param[out] dxChDefMcRouteLttEntryPtr - (pointer to) DX mc LTT info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChMcLttEntryInfo
(
    IN  PRV_TGF_IP_LTT_ENTRY_STC             *defMcRouteLttEntryPtr,
    OUT CPSS_DXCH_IP_LTT_ENTRY_STC      *dxChDefMcRouteLttEntryPtr
)
{
    GT_STATUS rc = GT_OK;
    /* set route type */
    rc = prvTgfConvertGenericToDxChRouteType(defMcRouteLttEntryPtr->routeType,
                                             &dxChDefMcRouteLttEntryPtr->routeType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert ltt entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, numOfPaths);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, routeEntryBaseIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, ucRPFCheckEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, sipSaCheckMismatchEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, ipv6MCGroupScopeLevel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, priority);

    return rc;
}

/**
* @internal prvTgfConvertDxChToGenericMcLttEntryInfo function
* @endinternal
*
* @brief   Convert generic mc LTT info from DX specific mc LTT info
*
* @param[in] dxChDefMcRouteLttEntryPtr - (pointer to) DX mc LTT info
*
* @param[out] defMcRouteLttEntryPtr    - (pointer to) generic mc LTT info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericMcLttEntryInfo
(
    IN  CPSS_DXCH_IP_LTT_ENTRY_STC      *dxChDefMcRouteLttEntryPtr,
    OUT PRV_TGF_IP_LTT_ENTRY_STC        *defMcRouteLttEntryPtr
)
{
    GT_STATUS rc = GT_OK;

    /* set route type */
    rc = prvTgfConvertDxChToGenericRouteType(dxChDefMcRouteLttEntryPtr->routeType,
                                             &(defMcRouteLttEntryPtr->routeType));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericRouteType FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert ltt entry into device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, numOfPaths);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, routeEntryBaseIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, ucRPFCheckEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, sipSaCheckMismatchEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, ipv6MCGroupScopeLevel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChDefMcRouteLttEntryPtr, defMcRouteLttEntryPtr, priority);
    return rc;
}

/**
* @internal prvTgfConvertDxChToGenericUcNextHopInfo function
* @endinternal
*
* @brief   Convert generic UC next hop info from DX specific UC next hop info
*
* @param[in] routingMode              - routing mode
* @param[in] dxChDefUcNextHopInfoPtr  - (pointer to) DxCh UC next hop info
*
* @param[out] defUcNextHopInfoPtr      - (pointer to) generic uc next hop info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericUcNextHopInfo
(
    IN  PRV_TGF_IP_ROUTING_MODE_ENT            routingMode,
    IN  CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *dxChDefUcNextHopInfoPtr,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT   *defUcNextHopInfoPtr
)
{
    GT_STATUS rc = GT_OK;

    switch (routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                           &defUcNextHopInfoPtr->pclIpUcAction,
                                           pktCmd);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                           &defUcNextHopInfoPtr->pclIpUcAction,
                                           actionStop);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                           &defUcNextHopInfoPtr->pclIpUcAction,
                                           bypassBridge);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                           &defUcNextHopInfoPtr->pclIpUcAction,
                                           bypassIngressPipe);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction,
                                           &defUcNextHopInfoPtr->pclIpUcAction,
                                           egressPolicy);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           ipclConfigIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           pcl0_1OverrideConfigIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           &defUcNextHopInfoPtr->pclIpUcAction.lookupConfig,
                                           pcl1OverrideConfigIndex);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           cpuCode);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           mirrorTcpRstAndFinPacketsToCpu);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           mirrorToRxAnalyzerPort);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           &defUcNextHopInfoPtr->pclIpUcAction.mirror,
                                           ingressMirrorToAnalyzerIndex);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                           &defUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                           enableMatchCount);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                           &defUcNextHopInfoPtr->pclIpUcAction.matchCounter,
                                           matchCounterIndex);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                           modifyUp);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                           modifyDscp);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                           profileAssignIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                           profileIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.qos.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.qos,
                                           profilePrecedence);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect,
                                           &defUcNextHopInfoPtr->pclIpUcAction.redirect,
                                           redirectCmd);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           routerLttIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           routerLttIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           &defUcNextHopInfoPtr->pclIpUcAction.redirect.data,
                                           vrfId);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.policer,
                                           &defUcNextHopInfoPtr->pclIpUcAction.policer,
                                           policerId);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.policer,
                                           &defUcNextHopInfoPtr->pclIpUcAction.policer,
                                           policerEnable);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                           &defUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                           assignSourceId);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                           &defUcNextHopInfoPtr->pclIpUcAction.sourceId,
                                           sourceIdValue);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           arpDaIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           bypassTTLCheck);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           decrementTTL);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           doIpUcRoute);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           &defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute,
                                           icmpRedirectCheck);

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                           modifyVlan);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                           nestedVlan);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress,
                                           &defUcNextHopInfoPtr->pclIpUcAction.vlan,
                                           precedence);
            dxChDefUcNextHopInfoPtr->pclIpUcAction.vlan.ingress.vlanId =
                (GT_U16)defUcNextHopInfoPtr->pclIpUcAction.vlan.vlanId;
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
            /* set route type */
            rc = prvTgfConvertDxChToGenericRouteType(dxChDefUcNextHopInfoPtr->ipLttEntry.routeType,
                                                     &defUcNextHopInfoPtr->ipLttEntry.routeType);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericRouteType FAILED, rc = [%d]", rc);

                return rc;
            }

            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           numOfPaths);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           routeEntryBaseIndex);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           ucRPFCheckEnable);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           sipSaCheckMismatchEnable);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           ipv6MCGroupScopeLevel);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&dxChDefUcNextHopInfoPtr->ipLttEntry,
                                           &defUcNextHopInfoPtr->ipLttEntry,
                                           priority);

            break;

        default:
            return GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvTgfConvertGenericToDxChIpHeaderError function
* @endinternal
*
* @brief   Convert generic IP header error into device specific IP header error
*
* @param[in] ipErrorHeader            - IP error header
*
* @param[out] dxChIpErrorHeader        - (pointer to) DxCh IP error header
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIpHeaderError
(
    IN  PRV_TGF_IP_HEADER_ERROR_ENT         ipErrorHeader,
    OUT CPSS_DXCH_IP_HEADER_ERROR_ENT       *dxChIpErrorHeader
)
{
    /* convert error header into device specific format */
    switch (ipErrorHeader)
    {
        case PRV_TGF_IP_HEADER_ERROR_CHECKSUM_ENT:
            *dxChIpErrorHeader = CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT;
            break;

        case PRV_TGF_IP_HEADER_ERROR_VERSION_ENT:
            *dxChIpErrorHeader = CPSS_DXCH_IP_HEADER_ERROR_VERSION_ENT;
            break;

        case PRV_TGF_IP_HEADER_ERROR_LENGTH_ENT:
            *dxChIpErrorHeader = CPSS_DXCH_IP_HEADER_ERROR_LENGTH_ENT;
            break;

        case PRV_TGF_IP_HEADER_ERROR_SIP_DIP_ENT:
            *dxChIpErrorHeader = CPSS_DXCH_IP_HEADER_ERROR_SIP_DIP_ENT;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChUnicastRpfType function
* @endinternal
*
* @brief   Convert generic uRPF type into device specific uRPF type
*
* @param[in] uRpfType                 - unicast RPF type
*
* @param[out] dxChURpfTypePtr          - (pointer to) DxCh unicast RPF type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChUnicastRpfType
(
    IN  PRV_TGF_IP_URPF_MODE_ENT            uRpfType,
    OUT CPSS_DXCH_IP_URPF_MODE_ENT         *dxChURpfTypePtr
)
{
    /* convert routeType into device specific format */
    switch (uRpfType)
    {
    case PRV_TGF_IP_URPF_DISABLE_MODE_E:
        *dxChURpfTypePtr = CPSS_DXCH_IP_URPF_DISABLE_MODE_E;
        break;
    case PRV_TGF_IP_URPF_VLAN_MODE_E:
        *dxChURpfTypePtr = CPSS_DXCH_IP_URPF_VLAN_MODE_E;
        break;
    case PRV_TGF_IP_URPF_PORT_TRUNK_MODE_E:
        *dxChURpfTypePtr = CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E;
        break;
    case PRV_TGF_IP_URPF_LOOSE_MODE_E:
        *dxChURpfTypePtr = CPSS_DXCH_IP_URPF_LOOSE_MODE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericUnicastRpfType function
* @endinternal
*
* @brief   Convert device specific unicast RPF type into generic unicast RPF type
*
* @param[in] dxChURpfType             - DxCh unicast RPF type
*
* @param[out] uRpfTypePtr              - (pointer to) unicast RPF type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericUnicastRpfType
(
    IN  CPSS_DXCH_IP_URPF_MODE_ENT          dxChURpfType,
    OUT PRV_TGF_IP_URPF_MODE_ENT           *uRpfTypePtr
)
{
    /* convert routeType from device specific format */
    switch (dxChURpfType)
    {
    case CPSS_DXCH_IP_URPF_DISABLE_MODE_E:
        *uRpfTypePtr = PRV_TGF_IP_URPF_DISABLE_MODE_E;
        break;

    case CPSS_DXCH_IP_URPF_VLAN_MODE_E:
        *uRpfTypePtr = PRV_TGF_IP_URPF_VLAN_MODE_E;
        break;

    case CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E:
        *uRpfTypePtr = PRV_TGF_IP_URPF_PORT_TRUNK_MODE_E;
        break;

    case CPSS_DXCH_IP_URPF_LOOSE_MODE_E:
        *uRpfTypePtr = PRV_TGF_IP_URPF_LOOSE_MODE_E;
        break;

    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChUnicastRpfLooseModeType function
* @endinternal
*
* @brief   Convert generic uRPF loose mode type into device specific uRPF loose mode type
*
* @param[in] uRpfLooseModeType        - unicast RPF loose mode type
*
* @param[out] dxChURpfLooseModeTypePtr - (pointer to) DxCh unicast RPF loose mode type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChUnicastRpfLooseModeType
(
    IN  PRV_TGF_URPF_LOOSE_MODE_TYPE_ENT    uRpfLooseModeType,
    OUT CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT  *dxChURpfLooseModeTypePtr
)
{
    /* convert routeType into device specific format */
    switch (uRpfLooseModeType)
    {
    case PRV_TGF_URPF_LOOSE_MODE_TYPE_0_E:
        *dxChURpfLooseModeTypePtr = CPSS_DXCH_URPF_LOOSE_MODE_TYPE_0_E;
        break;
    case PRV_TGF_URPF_LOOSE_MODE_TYPE_1_E:
        *dxChURpfLooseModeTypePtr = CPSS_DXCH_URPF_LOOSE_MODE_TYPE_1_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

#endif /* CHX_FAMILY */


#ifdef EXMXPM_FAMILY
/**
* @internal prvTgfConvertGenericToExMxPmIpUcRouteEntry function
* @endinternal
*
* @brief   Convert generic IP UC route entry into device specific IP UC route entry
*
* @param[in] ipUcRouteEntryPtr        - (pointer to) IP UC route entry
*
* @param[out] exMxPmIpUcRouteEntryPtr  - (pointer to) ExMxPm IP UC route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmIpUcRouteEntry
(
    IN  PRV_TGF_IP_UC_ROUTE_ENTRY_STC            *ipUcRouteEntryPtr,
    OUT CPSS_EXMXPM_IP_UC_ROUTE_ENTRY_FORMAT_STC *exMxPmIpUcRouteEntryPtr
)
{
    /* convert IP UC route entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, appSpecificCpuCodeEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, mtuProfileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopVlanId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, sipSaCheckEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, mplsLabel);

    exMxPmIpUcRouteEntryPtr->command                 = ipUcRouteEntryPtr->cmd;
    exMxPmIpUcRouteEntryPtr->bypassTtlExceptionCheckEnable = ipUcRouteEntryPtr->ttlHopLimDecOptionsExtChkByPass;
    exMxPmIpUcRouteEntryPtr->cpuCodeIndex                  = ipUcRouteEntryPtr->cpuCodeIndex;
    exMxPmIpUcRouteEntryPtr->ageRefresh                    = ipUcRouteEntryPtr->ageRefresh;
    exMxPmIpUcRouteEntryPtr->ipv6ScopeCheckEnable          = ipUcRouteEntryPtr->scopeCheckingEnable;
    exMxPmIpUcRouteEntryPtr->ipv6DestinationSiteId         = ipUcRouteEntryPtr->siteId;
    exMxPmIpUcRouteEntryPtr->sipAccessLevel                = ipUcRouteEntryPtr->sipAccessLevel;
    exMxPmIpUcRouteEntryPtr->dipAccessLevel                = ipUcRouteEntryPtr->dipAccessLevel;

    exMxPmIpUcRouteEntryPtr->ucPacketSipFilterEnable = ipUcRouteEntryPtr->unicastPacketSipFilterEnable;
    exMxPmIpUcRouteEntryPtr->mirrorToIngressAnalyzer = ipUcRouteEntryPtr->ingressMirror;
    exMxPmIpUcRouteEntryPtr->trapMirrorArpBcToCpu    = ipUcRouteEntryPtr->trapMirrorArpBcEnable;
    exMxPmIpUcRouteEntryPtr->icmpRedirectEnable      = ipUcRouteEntryPtr->ICMPRedirectEnable;

    /* convert nextHop Interface into device specific format */
    if (ipUcRouteEntryPtr->isTunnelStart == GT_FALSE)
    {
        if (ipUcRouteEntryPtr->isOutlifDit == GT_FALSE)
        {
            exMxPmIpUcRouteEntryPtr->nextHopIf.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_LL_E;
            exMxPmIpUcRouteEntryPtr->nextHopIf.outlifPointer.arpPtr =
            ipUcRouteEntryPtr->nextHopARPPointer;
        }
        else
        {
            exMxPmIpUcRouteEntryPtr->nextHopIf.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_DIT_E;
            exMxPmIpUcRouteEntryPtr->nextHopIf.outlifPointer.ditPtr =
            ipUcRouteEntryPtr->nextHopDITPointer;
        }
    }
    else
    {
                exMxPmIpUcRouteEntryPtr->nextHopIf.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_TUNNEL_E;
                        exMxPmIpUcRouteEntryPtr->nextHopIf.outlifPointer.tunnelStartPtr.passengerPacketType =
                            CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_OTHER_E;
                exMxPmIpUcRouteEntryPtr->nextHopIf.outlifPointer.tunnelStartPtr.ptr =
            ipUcRouteEntryPtr->nextHopTunnelPointer;
    }

    /* set interface info */
    cpssOsMemCpy((GT_VOID*) &(exMxPmIpUcRouteEntryPtr->nextHopIf.interfaceInfo),
                 (GT_VOID*) &(ipUcRouteEntryPtr->nextHopInterface),
                 sizeof(CPSS_INTERFACE_INFO_STC));

    /* set qos modify params */
    exMxPmIpUcRouteEntryPtr->qosParamsModify.modifyTc = ipUcRouteEntryPtr->qosParamsModify.modifyTc;
    exMxPmIpUcRouteEntryPtr->qosParamsModify.modifyUp = ipUcRouteEntryPtr->qosParamsModify.modifyUp;
    exMxPmIpUcRouteEntryPtr->qosParamsModify.modifyDp = ipUcRouteEntryPtr->qosParamsModify.modifyDp;
    exMxPmIpUcRouteEntryPtr->qosParamsModify.modifyDscp = ipUcRouteEntryPtr->qosParamsModify.modifyDscp;
    exMxPmIpUcRouteEntryPtr->qosParamsModify.modifyExp = ipUcRouteEntryPtr->qosParamsModify.modifyExp;


    /* set qos params */
    exMxPmIpUcRouteEntryPtr->qosParams.tc = ipUcRouteEntryPtr->qosParams.tc;
    exMxPmIpUcRouteEntryPtr->qosParams.dp = ipUcRouteEntryPtr->qosParams.dp;
    exMxPmIpUcRouteEntryPtr->qosParams.up = ipUcRouteEntryPtr->qosParams.up;
    exMxPmIpUcRouteEntryPtr->qosParams.dscp = ipUcRouteEntryPtr->qosParams.dscp;
    exMxPmIpUcRouteEntryPtr->qosParams.exp = ipUcRouteEntryPtr->qosParams.exp;

    /* convert ucRpfOrIcmpRedirectIfMode into device specific format */
    switch (ipUcRouteEntryPtr->ucRpfOrIcmpRedirectIfMode)
    {
        case PRV_TGF_RPF_IF_MODE_DISABLED_E:
            exMxPmIpUcRouteEntryPtr->ucRpfOrIcmpRedirectIfMode = CPSS_EXMXPM_RPF_IF_MODE_DISABLED_E;
            break;

        case PRV_TGF_RPF_IF_MODE_PORT_E:
            exMxPmIpUcRouteEntryPtr->ucRpfOrIcmpRedirectIfMode = CPSS_EXMXPM_RPF_IF_MODE_PORT_E;
            break;

        case PRV_TGF_RPF_IF_MODE_L2_VLAN_E:
            exMxPmIpUcRouteEntryPtr->ucRpfOrIcmpRedirectIfMode = CPSS_EXMXPM_RPF_IF_MODE_L2_VLAN_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert cntrSetIndex into device specific format */
    switch (ipUcRouteEntryPtr->countSet)
    {
        case CPSS_IP_CNT_SET0_E:
            exMxPmIpUcRouteEntryPtr->cntrSetIndex = CPSS_EXMXPM_IP_COUNTER_SET_INDEX_0_E;
            break;

        case CPSS_IP_CNT_SET1_E:
            exMxPmIpUcRouteEntryPtr->cntrSetIndex = CPSS_EXMXPM_IP_COUNTER_SET_INDEX_1_E;
            break;

        case CPSS_IP_CNT_SET2_E:
            exMxPmIpUcRouteEntryPtr->cntrSetIndex = CPSS_EXMXPM_IP_COUNTER_SET_INDEX_2_E;
            break;

        case CPSS_IP_CNT_SET3_E:
            exMxPmIpUcRouteEntryPtr->cntrSetIndex = CPSS_EXMXPM_IP_COUNTER_SET_INDEX_3_E;
            break;

        case CPSS_IP_CNT_NO_SET_E:
            exMxPmIpUcRouteEntryPtr->cntrSetIndex = CPSS_EXMXPM_IP_COUNTER_DISABLE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert hopLimitMode into device specific format */
    switch (ipUcRouteEntryPtr->hopLimitMode)
    {
        case PRV_TGF_IP_HOP_LIMIT_NOT_DECREMENTED_E:
            exMxPmIpUcRouteEntryPtr->hopLimitMode = CPSS_EXMXPM_IP_HOP_LIMIT_NOT_DECREMENTED_E;
            break;

        case PRV_TGF_IP_HOP_LIMIT_DECREMENTED_BY_ONE_E:
            exMxPmIpUcRouteEntryPtr->hopLimitMode = CPSS_EXMXPM_IP_HOP_LIMIT_DECREMENTED_BY_ONE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert ttlMode into device specific format */
    switch (ipUcRouteEntryPtr->ttlMode)
    {
        case PRV_TGF_IP_TTL_NOT_DECREMENTED_E:
            exMxPmIpUcRouteEntryPtr->ttlMode = CPSS_EXMXPM_IP_TTL_NOT_DECREMENTED_E;
            break;

        case PRV_TGF_IP_TTL_DECREMENTED_BY_ONE_E:
            exMxPmIpUcRouteEntryPtr->ttlMode = CPSS_EXMXPM_IP_TTL_DECREMENTED_BY_ONE_E;
            break;

        case PRV_TGF_IP_TTL_DECREMENTED_BY_ENTRY_E:
            exMxPmIpUcRouteEntryPtr->ttlMode = CPSS_EXMXPM_IP_TTL_DECREMENTED_BY_ENTRY_E;
            break;

        case PRV_TGF_IP_TTL_COPY_ENTRY_E:
            exMxPmIpUcRouteEntryPtr->ttlMode = CPSS_EXMXPM_IP_TTL_COPY_ENTRY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mplsCommand into device specific format */
    switch (ipUcRouteEntryPtr->mplsCommand)
    {
        case PRV_TGF_MPLS_NOP_CMD_E:
            exMxPmIpUcRouteEntryPtr->mplsCommand = CPSS_EXMXPM_MPLS_NOP_CMD_E;
            break;

        case PRV_TGF_MPLS_SWAP_CMD_E:
            exMxPmIpUcRouteEntryPtr->mplsCommand = CPSS_EXMXPM_MPLS_SWAP_CMD_E;
            break;

        case PRV_TGF_MPLS_PUSH_CMD_E:
            exMxPmIpUcRouteEntryPtr->mplsCommand = CPSS_EXMXPM_MPLS_PUSH_CMD_E;
            break;

        case PRV_TGF_MPLS_PHP_CMD_E:
            exMxPmIpUcRouteEntryPtr->mplsCommand = CPSS_EXMXPM_MPLS_PHP_CMD_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericIpUcRouteEntry function
* @endinternal
*
* @brief   Convert device specific IP UC route entry into generic IP UC route entry
*
* @param[in] exMxPmIpUcRouteEntryPtr  - (pointer to) ExMxPm IP UC route entry
*
* @param[out] ipUcRouteEntryPtr        - (pointer to) IP UC route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericIpUcRouteEntry
(
    IN  CPSS_EXMXPM_IP_UC_ROUTE_ENTRY_FORMAT_STC *exMxPmIpUcRouteEntryPtr,
    OUT PRV_TGF_IP_UC_ROUTE_ENTRY_STC            *ipUcRouteEntryPtr
)
{
    /* convert IP UC route entry from device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, cpuCodeIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, appSpecificCpuCodeEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, sipAccessLevel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, dipAccessLevel);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, mtuProfileIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, nextHopVlanId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, ageRefresh);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, sipSaCheckEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, ttl);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(exMxPmIpUcRouteEntryPtr, ipUcRouteEntryPtr, mplsLabel);

    ipUcRouteEntryPtr->cmd                          = exMxPmIpUcRouteEntryPtr->command;
    ipUcRouteEntryPtr->ttlHopLimDecOptionsExtChkByPass = exMxPmIpUcRouteEntryPtr->bypassTtlExceptionCheckEnable;
    ipUcRouteEntryPtr->unicastPacketSipFilterEnable = exMxPmIpUcRouteEntryPtr->ucPacketSipFilterEnable;
    ipUcRouteEntryPtr->ingressMirror                = exMxPmIpUcRouteEntryPtr->mirrorToIngressAnalyzer;
    ipUcRouteEntryPtr->trapMirrorArpBcEnable        = exMxPmIpUcRouteEntryPtr->trapMirrorArpBcToCpu;
    ipUcRouteEntryPtr->ICMPRedirectEnable           = exMxPmIpUcRouteEntryPtr->icmpRedirectEnable;
    ipUcRouteEntryPtr->scopeCheckingEnable          = exMxPmIpUcRouteEntryPtr->ipv6ScopeCheckEnable;
    ipUcRouteEntryPtr->siteId                       = exMxPmIpUcRouteEntryPtr->ipv6DestinationSiteId;

    /* convert nextHop Interface from device specific format */
    switch (exMxPmIpUcRouteEntryPtr->nextHopIf.outlifType)
    {
        case CPSS_EXMXPM_OUTLIF_TYPE_LL_E:
            ipUcRouteEntryPtr->isOutlifDit =  GT_FALSE;
            ipUcRouteEntryPtr->nextHopARPPointer =
                exMxPmIpUcRouteEntryPtr->nextHopIf.outlifPointer.arpPtr;

            break;

        case CPSS_EXMXPM_OUTLIF_TYPE_DIT_E:
            ipUcRouteEntryPtr->isOutlifDit =  GT_TRUE;
            ipUcRouteEntryPtr->nextHopDITPointer =
                exMxPmIpUcRouteEntryPtr->nextHopIf.outlifPointer.ditPtr;

            break;

        case CPSS_EXMXPM_OUTLIF_TYPE_TUNNEL_E:
            {
                ipUcRouteEntryPtr->isTunnelStart = GT_TRUE;

                /* convert passengerPacketType into device specific format */
                switch (exMxPmIpUcRouteEntryPtr->nextHopIf.outlifPointer.tunnelStartPtr.passengerPacketType)
                {
                    case CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_OTHER_E:
                        break;

                    default:
                        return GT_BAD_STATE;
                }

                ipUcRouteEntryPtr->nextHopTunnelPointer =
                    exMxPmIpUcRouteEntryPtr->nextHopIf.outlifPointer.tunnelStartPtr.ptr;
            }
            break;

        default:
            return GT_BAD_STATE;
    }

    /* set interface info */
    cpssOsMemCpy((GT_VOID*) &(ipUcRouteEntryPtr->nextHopInterface),
                 (GT_VOID*) &(exMxPmIpUcRouteEntryPtr->nextHopIf.interfaceInfo),
                 sizeof(CPSS_INTERFACE_INFO_STC));

    /* set qos modify params */
    ipUcRouteEntryPtr->qosParamsModify.modifyTc = exMxPmIpUcRouteEntryPtr->qosParamsModify.modifyTc;
    ipUcRouteEntryPtr->qosParamsModify.modifyUp = exMxPmIpUcRouteEntryPtr->qosParamsModify.modifyUp;
    ipUcRouteEntryPtr->qosParamsModify.modifyDp = exMxPmIpUcRouteEntryPtr->qosParamsModify.modifyDp;
    ipUcRouteEntryPtr->qosParamsModify.modifyDscp = exMxPmIpUcRouteEntryPtr->qosParamsModify.modifyDscp;
    ipUcRouteEntryPtr->qosParamsModify.modifyExp = exMxPmIpUcRouteEntryPtr->qosParamsModify.modifyExp;

    /* set qos params */
    ipUcRouteEntryPtr->qosParams.tc = exMxPmIpUcRouteEntryPtr->qosParams.tc;
    ipUcRouteEntryPtr->qosParams.dp = exMxPmIpUcRouteEntryPtr->qosParams.dp;
    ipUcRouteEntryPtr->qosParams.up = exMxPmIpUcRouteEntryPtr->qosParams.up;
    ipUcRouteEntryPtr->qosParams.dscp = exMxPmIpUcRouteEntryPtr->qosParams.dscp;
    ipUcRouteEntryPtr->qosParams.exp = exMxPmIpUcRouteEntryPtr->qosParams.exp;

    /* convert ucRpfOrIcmpRedirectIfMode from device specific format */
    switch (exMxPmIpUcRouteEntryPtr->ucRpfOrIcmpRedirectIfMode)
    {
        case CPSS_EXMXPM_RPF_IF_MODE_DISABLED_E:
            ipUcRouteEntryPtr->ucRpfOrIcmpRedirectIfMode = PRV_TGF_RPF_IF_MODE_DISABLED_E;
            break;

        case CPSS_EXMXPM_RPF_IF_MODE_PORT_E:
            ipUcRouteEntryPtr->ucRpfOrIcmpRedirectIfMode = PRV_TGF_RPF_IF_MODE_PORT_E;
            break;

        case CPSS_EXMXPM_RPF_IF_MODE_L2_VLAN_E:
            ipUcRouteEntryPtr->ucRpfOrIcmpRedirectIfMode = PRV_TGF_RPF_IF_MODE_L2_VLAN_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert cntrSetIndex from device specific format */
    switch (exMxPmIpUcRouteEntryPtr->cntrSetIndex)
    {
        case CPSS_EXMXPM_IP_COUNTER_SET_INDEX_0_E:
            ipUcRouteEntryPtr->countSet = CPSS_IP_CNT_SET0_E;
            break;

        case CPSS_EXMXPM_IP_COUNTER_SET_INDEX_1_E:
            ipUcRouteEntryPtr->countSet = CPSS_IP_CNT_SET1_E;
            break;

        case CPSS_EXMXPM_IP_COUNTER_SET_INDEX_2_E:
            ipUcRouteEntryPtr->countSet = CPSS_IP_CNT_SET2_E;
            break;

        case CPSS_EXMXPM_IP_COUNTER_SET_INDEX_3_E:
            ipUcRouteEntryPtr->countSet = CPSS_IP_CNT_SET3_E;
            break;

        case CPSS_EXMXPM_IP_COUNTER_DISABLE_E:
            ipUcRouteEntryPtr->countSet = CPSS_IP_CNT_NO_SET_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert hopLimitMode from device specific format */
    switch (exMxPmIpUcRouteEntryPtr->hopLimitMode)
    {
        case CPSS_EXMXPM_IP_HOP_LIMIT_NOT_DECREMENTED_E:
            ipUcRouteEntryPtr->hopLimitMode = PRV_TGF_IP_HOP_LIMIT_NOT_DECREMENTED_E;
            break;

        case CPSS_EXMXPM_IP_HOP_LIMIT_DECREMENTED_BY_ONE_E:
            ipUcRouteEntryPtr->hopLimitMode = PRV_TGF_IP_HOP_LIMIT_DECREMENTED_BY_ONE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert ttlMode from device specific format */
    switch (exMxPmIpUcRouteEntryPtr->ttlMode)
    {
        case CPSS_EXMXPM_IP_TTL_NOT_DECREMENTED_E:
            ipUcRouteEntryPtr->ttlMode = PRV_TGF_IP_TTL_NOT_DECREMENTED_E;
            break;

        case CPSS_EXMXPM_IP_TTL_DECREMENTED_BY_ONE_E:
            ipUcRouteEntryPtr->ttlMode = PRV_TGF_IP_TTL_DECREMENTED_BY_ONE_E;
            break;

        case CPSS_EXMXPM_IP_TTL_DECREMENTED_BY_ENTRY_E:
            ipUcRouteEntryPtr->ttlMode = PRV_TGF_IP_TTL_DECREMENTED_BY_ENTRY_E;
            break;

        case CPSS_EXMXPM_IP_TTL_COPY_ENTRY_E:
            ipUcRouteEntryPtr->ttlMode = PRV_TGF_IP_TTL_COPY_ENTRY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mplsCommand from device specific format */
    switch (exMxPmIpUcRouteEntryPtr->mplsCommand)
    {
        case CPSS_EXMXPM_MPLS_NOP_CMD_E:
            ipUcRouteEntryPtr->mplsCommand = PRV_TGF_MPLS_NOP_CMD_E;
            break;

        case CPSS_EXMXPM_MPLS_SWAP_CMD_E:
            ipUcRouteEntryPtr->mplsCommand = PRV_TGF_MPLS_SWAP_CMD_E;
            break;

        case CPSS_EXMXPM_MPLS_PUSH_CMD_E:
            ipUcRouteEntryPtr->mplsCommand = PRV_TGF_MPLS_PUSH_CMD_E;
            break;

        case CPSS_EXMXPM_MPLS_PHP_CMD_E:
            ipUcRouteEntryPtr->mplsCommand = PRV_TGF_MPLS_PHP_CMD_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToExMxPmIpMcRouteEntry function
* @endinternal
*
* @brief   Convert generic IP MC route entry into device specific IP MC route entry
*
* @param[in] protocol                 -  ip  type
* @param[in] ipMcRouteEntryPtr        - (pointer to) IP MC route entry
*
* @param[out] exMxPmIpMcRouteEntryPtr  - (pointer to) ExMxPm IP MC route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmIpMcRouteEntry
(
    IN CPSS_IP_PROTOCOL_STACK_ENT                protocol,
    IN  PRV_TGF_IP_MC_ROUTE_ENTRY_STC            *ipMcRouteEntryPtr,
    OUT CPSS_EXMXPM_IP_MC_ROUTE_ENTRY_STC        *exMxPmIpMcRouteEntryPtr
)
{
    /* convert IP MC route entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpMcRouteEntryPtr, ipMcRouteEntryPtr, bypassTtlExceptionCheckEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpMcRouteEntryPtr, ipMcRouteEntryPtr, cpuCodeIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpMcRouteEntryPtr, ipMcRouteEntryPtr, ageRefresh);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpMcRouteEntryPtr, ipMcRouteEntryPtr, ipv6ScopeLevel);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpMcRouteEntryPtr, ipMcRouteEntryPtr, mtuProfileIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpMcRouteEntryPtr, ipMcRouteEntryPtr, appSpecificCpuCodeEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpMcRouteEntryPtr, ipMcRouteEntryPtr, ttl);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(exMxPmIpMcRouteEntryPtr, ipMcRouteEntryPtr, mplsLabel);

    exMxPmIpMcRouteEntryPtr->command                 = ipMcRouteEntryPtr->cmd;
    exMxPmIpMcRouteEntryPtr->ipv6ScopeCheckEnable    = ipMcRouteEntryPtr->scopeCheckingEnable;
    exMxPmIpMcRouteEntryPtr->ipv6DestinationSiteId   = ipMcRouteEntryPtr->siteId;
    exMxPmIpMcRouteEntryPtr->mirrorToIngressAnalyzer = ipMcRouteEntryPtr->ingressMirror;
    ipMcRouteEntryPtr->nextHopIf.outlifType = PRV_TGF_OUTLIF_TYPE_DIT_E; /* that is the only case */

    /* set RPF interface */
    cpssOsMemCpy((GT_VOID*) &(exMxPmIpMcRouteEntryPtr->mcRpfIf),
                 (GT_VOID*) &(ipMcRouteEntryPtr->mcRpfIf),
                 sizeof(ipMcRouteEntryPtr->mcRpfIf));

    /* convert nextHop Interface into device specific format */
    switch (ipMcRouteEntryPtr->nextHopIf.outlifType)
    {
        case PRV_TGF_OUTLIF_TYPE_LL_E:
            exMxPmIpMcRouteEntryPtr->nextHopIf.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_LL_E;
            exMxPmIpMcRouteEntryPtr->nextHopIf.outlifPointer.arpPtr =
                ipMcRouteEntryPtr->nextHopIf.outlifPointer.arpPtr;

            break;

        case PRV_TGF_OUTLIF_TYPE_DIT_E:
            exMxPmIpMcRouteEntryPtr->nextHopIf.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_DIT_E;
            if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
            {
                exMxPmIpMcRouteEntryPtr->nextHopIf.outlifPointer.ditPtr = ipMcRouteEntryPtr->ipv4MllPtr;
            }
            else
            {
                exMxPmIpMcRouteEntryPtr->nextHopIf.outlifPointer.ditPtr = ipMcRouteEntryPtr->ipv6ExternalMllPtr;
            }
            break;

        case PRV_TGF_OUTLIF_TYPE_TUNNEL_E:
            {
                exMxPmIpMcRouteEntryPtr->nextHopIf.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_TUNNEL_E;

                /* convert passengerPacketType into device specific format */
                switch (ipMcRouteEntryPtr->nextHopIf.outlifPointer.tunnelStartPtr.passengerPacketType)
                {
                    case PRV_TGF_TS_PASSENGER_PACKET_TYPE_ETHERNET_E:
                        exMxPmIpMcRouteEntryPtr->nextHopIf.outlifPointer.tunnelStartPtr.passengerPacketType =
                            CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_ETHERNET_E;
                        break;

                    case PRV_TGF_TS_PASSENGER_PACKET_TYPE_OTHER_E:
                        exMxPmIpMcRouteEntryPtr->nextHopIf.outlifPointer.tunnelStartPtr.passengerPacketType =
                            CPSS_EXMXPM_TS_PASSENGER_PACKET_TYPE_OTHER_E;
                        break;

                    default:
                        return GT_BAD_PARAM;
                }

                exMxPmIpMcRouteEntryPtr->nextHopIf.outlifPointer.tunnelStartPtr.ptr =
                    ipMcRouteEntryPtr->nextHopIf.outlifPointer.tunnelStartPtr.ptr;
            }
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* set interface info */
    cpssOsMemCpy((GT_VOID*) &(exMxPmIpMcRouteEntryPtr->nextHopIf.interfaceInfo),
                 (GT_VOID*) &(ipMcRouteEntryPtr->nextHopIf.interfaceInfo),
                 sizeof(ipMcRouteEntryPtr->nextHopIf.interfaceInfo));

    exMxPmIpMcRouteEntryPtr->nextHopIf.interfaceInfo.type = CPSS_INTERFACE_VIDX_E;
    exMxPmIpMcRouteEntryPtr->nextHopIf.interfaceInfo.vidx = 0xFFF;


    /* set qos modify params */
    exMxPmIpMcRouteEntryPtr->qosParamsModify.modifyTc = ipMcRouteEntryPtr->qosParamsModify.modifyTc;
    exMxPmIpMcRouteEntryPtr->qosParamsModify.modifyUp = ipMcRouteEntryPtr->qosParamsModify.modifyUp;
    exMxPmIpMcRouteEntryPtr->qosParamsModify.modifyDp = ipMcRouteEntryPtr->qosParamsModify.modifyDp;
    exMxPmIpMcRouteEntryPtr->qosParamsModify.modifyDscp = ipMcRouteEntryPtr->qosParamsModify.modifyDscp;
    exMxPmIpMcRouteEntryPtr->qosParamsModify.modifyExp = ipMcRouteEntryPtr->qosParamsModify.modifyExp;

    /* set qos params */
    exMxPmIpMcRouteEntryPtr->qosParams.tc = ipMcRouteEntryPtr->qosParams.tc;
    exMxPmIpMcRouteEntryPtr->qosParams.dp = ipMcRouteEntryPtr->qosParams.dp;
    exMxPmIpMcRouteEntryPtr->qosParams.up = ipMcRouteEntryPtr->qosParams.up;
    exMxPmIpMcRouteEntryPtr->qosParams.dscp = ipMcRouteEntryPtr->qosParams.dscp;
    exMxPmIpMcRouteEntryPtr->qosParams.exp = ipMcRouteEntryPtr->qosParams.exp;

    /* convert mcRpfIfMode into device specific format */
    switch (ipMcRouteEntryPtr->mcRpfIfMode)
    {
        case PRV_TGF_RPF_IF_MODE_DISABLED_E:
            exMxPmIpMcRouteEntryPtr->mcRpfIfMode = CPSS_EXMXPM_RPF_IF_MODE_DISABLED_E;
            break;

        case PRV_TGF_RPF_IF_MODE_PORT_E:
            exMxPmIpMcRouteEntryPtr->mcRpfIfMode = CPSS_EXMXPM_RPF_IF_MODE_PORT_E;
            break;

        case PRV_TGF_RPF_IF_MODE_L2_VLAN_E:
            exMxPmIpMcRouteEntryPtr->mcRpfIfMode = CPSS_EXMXPM_RPF_IF_MODE_L2_VLAN_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert RPFFailCommand into device specific format */
    switch (ipMcRouteEntryPtr->RPFFailCommand)
    {
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            exMxPmIpMcRouteEntryPtr->mcRpfFailCommand = CPSS_EXMXPM_MC_RPF_FAIL_COMMAND_TRAP_TO_CPU_E;
            break;

        case CPSS_PACKET_CMD_DROP_HARD_E:
            exMxPmIpMcRouteEntryPtr->mcRpfFailCommand = CPSS_EXMXPM_MC_RPF_FAIL_COMMAND_DROP_HARD_E;
            break;

        case CPSS_PACKET_CMD_DROP_SOFT_E:
            exMxPmIpMcRouteEntryPtr->mcRpfFailCommand = CPSS_EXMXPM_MC_RPF_FAIL_COMMAND_DROP_SOFT_E;
            break;

        case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            exMxPmIpMcRouteEntryPtr->mcRpfFailCommand = CPSS_EXMXPM_MC_RPF_FAIL_COMMAND_BRIDGE_AND_MIRROR_E;
            break;

        case CPSS_PACKET_CMD_BRIDGE_E:
            exMxPmIpMcRouteEntryPtr->mcRpfFailCommand = CPSS_EXMXPM_MC_RPF_FAIL_COMMAND_BRIDGE_E;
            break;

        case CPSS_PACKET_CMD_NONE_E:
            exMxPmIpMcRouteEntryPtr->mcRpfFailCommand = CPSS_EXMXPM_MC_RPF_FAIL_COMMAND_DOWNSTREAM_INTERFACE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert cntrSetIndex into device specific format */
    switch (ipMcRouteEntryPtr->countSet)
    {
        case CPSS_IP_CNT_SET0_E:
            exMxPmIpMcRouteEntryPtr->cntrSetIndex = CPSS_EXMXPM_IP_COUNTER_SET_INDEX_0_E;
            break;

        case CPSS_IP_CNT_SET1_E:
            exMxPmIpMcRouteEntryPtr->cntrSetIndex = CPSS_EXMXPM_IP_COUNTER_SET_INDEX_1_E;
            break;

        case CPSS_IP_CNT_SET2_E:
            exMxPmIpMcRouteEntryPtr->cntrSetIndex = CPSS_EXMXPM_IP_COUNTER_SET_INDEX_2_E;
            break;

        case CPSS_IP_CNT_SET3_E:
            exMxPmIpMcRouteEntryPtr->cntrSetIndex = CPSS_EXMXPM_IP_COUNTER_SET_INDEX_3_E;
            break;

        case CPSS_IP_CNT_NO_SET_E:
            exMxPmIpMcRouteEntryPtr->cntrSetIndex = CPSS_EXMXPM_IP_COUNTER_DISABLE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert hopLimitMode into device specific format */
    switch (ipMcRouteEntryPtr->hopLimitMode)
    {
        case PRV_TGF_IP_HOP_LIMIT_NOT_DECREMENTED_E:
            exMxPmIpMcRouteEntryPtr->hopLimitMode = CPSS_EXMXPM_IP_HOP_LIMIT_NOT_DECREMENTED_E;
            break;

        case PRV_TGF_IP_HOP_LIMIT_DECREMENTED_BY_ONE_E:
            exMxPmIpMcRouteEntryPtr->hopLimitMode = CPSS_EXMXPM_IP_HOP_LIMIT_DECREMENTED_BY_ONE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert ttlMode into device specific format */
    switch (ipMcRouteEntryPtr->ttlMode)
    {
        case PRV_TGF_IP_TTL_NOT_DECREMENTED_E:
            exMxPmIpMcRouteEntryPtr->ttlMode = CPSS_EXMXPM_IP_TTL_NOT_DECREMENTED_E;
            break;

        case PRV_TGF_IP_TTL_DECREMENTED_BY_ONE_E:
            exMxPmIpMcRouteEntryPtr->ttlMode = CPSS_EXMXPM_IP_TTL_DECREMENTED_BY_ONE_E;
            break;

        case PRV_TGF_IP_TTL_DECREMENTED_BY_ENTRY_E:
            exMxPmIpMcRouteEntryPtr->ttlMode = CPSS_EXMXPM_IP_TTL_DECREMENTED_BY_ENTRY_E;
            break;

        case PRV_TGF_IP_TTL_COPY_ENTRY_E:
            exMxPmIpMcRouteEntryPtr->ttlMode = CPSS_EXMXPM_IP_TTL_COPY_ENTRY_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mplsCommand into device specific format */
    switch (ipMcRouteEntryPtr->mplsCommand)
    {
        case PRV_TGF_MPLS_NOP_CMD_E:
            exMxPmIpMcRouteEntryPtr->mplsCommand = CPSS_EXMXPM_MPLS_NOP_CMD_E;
            break;

        case PRV_TGF_MPLS_SWAP_CMD_E:
            exMxPmIpMcRouteEntryPtr->mplsCommand = CPSS_EXMXPM_MPLS_SWAP_CMD_E;
            break;

        case PRV_TGF_MPLS_PUSH_CMD_E:
            exMxPmIpMcRouteEntryPtr->mplsCommand = CPSS_EXMXPM_MPLS_PUSH_CMD_E;
            break;

        case PRV_TGF_MPLS_PHP_CMD_E:
            exMxPmIpMcRouteEntryPtr->mplsCommand = CPSS_EXMXPM_MPLS_PHP_CMD_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToExMxPmRouteType function
* @endinternal
*
* @brief   Convert generic route type into device specific route type
*
* @param[in] routeType                - route type
*
* @param[out] exMxPmRouteTypePtr       - (pointer to) ExMxPm route type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmRouteType
(
    IN  PRV_TGF_IP_ROUTE_ENTRY_METHOD_ENT       routeType,
    OUT CPSS_EXMXPM_IP_ROUTE_ENTRY_METHOD_ENT  *exMxPmRouteTypePtr
)
{
    /* convert routeType into device specific format */
    switch (routeType)
    {
        case PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E:
            *exMxPmRouteTypePtr = CPSS_EXMXPM_IP_ECMP_ROUTE_ENTRY_GROUP_E;
            break;

        case PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E:
            *exMxPmRouteTypePtr = CPSS_EXMXPM_IP_COS_ROUTE_ENTRY_GROUP_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}



/**
* @internal prvTgfConvertGenericToExMxPmRoutePointer function
* @endinternal
*
* @brief   Convert generic route type into device specific route type
*
* @param[in] ipLttEntryPtr            - (pointer to) generic LTT entry
*
* @param[out] exMxPmRouteEntryPointerPtr - (pointer to) ExMxPm route entry pointer
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToExMxPmRoutePointer
(
    IN  PRV_TGF_IP_LTT_ENTRY_STC               *ipLttEntryPtr,
    OUT CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC  *exMxPmRouteEntryPointerPtr
)
{
    exMxPmRouteEntryPointerPtr->routeEntryBaseMemAddr = ipLttEntryPtr->routeEntryBaseIndex;
    exMxPmRouteEntryPointerPtr->blockSize             = ipLttEntryPtr->numOfPaths + 1;
    if (exMxPmRouteEntryPointerPtr->blockSize == 1)
        exMxPmRouteEntryPointerPtr->routeEntryMethod = CPSS_EXMXPM_IP_REGULAR_ROUTE_ENTRY_E;
    else
    {
        switch (ipLttEntryPtr->routeType)
    {
        case PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E:
            exMxPmRouteEntryPointerPtr->routeEntryMethod = CPSS_EXMXPM_IP_ECMP_ROUTE_ENTRY_GROUP_E;
            break;
        case PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E:
            exMxPmRouteEntryPointerPtr->routeEntryMethod = CPSS_EXMXPM_IP_COS_ROUTE_ENTRY_GROUP_E;
            break;
        default:
            return GT_BAD_PARAM;
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertExMxPmToGenericRoutePointer function
* @endinternal
*
* @brief   Convert device specific route type into generic route type
*
* @param[in] exMxPmRouteEntryPointerPtr - (pointer to) ExMxPm route entry pointer
*
* @param[out] ipLttEntryPtr            - (pointer to) generic LTT entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertExMxPmToGenericRoutePointer
(
    IN  CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC  *exMxPmRouteEntryPointerPtr,
    OUT PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr

)
{
    ipLttEntryPtr->routeEntryBaseIndex = exMxPmRouteEntryPointerPtr->routeEntryBaseMemAddr;
    if(exMxPmRouteEntryPointerPtr->blockSize==0)
        return GT_BAD_VALUE;
    ipLttEntryPtr->numOfPaths = exMxPmRouteEntryPointerPtr->blockSize-1;

    switch (exMxPmRouteEntryPointerPtr->routeEntryMethod)
    {
        case CPSS_EXMXPM_IP_REGULAR_ROUTE_ENTRY_E:
        case CPSS_EXMXPM_IP_ECMP_ROUTE_ENTRY_GROUP_E:
            ipLttEntryPtr->routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
            break;
        case CPSS_EXMXPM_IP_COS_ROUTE_ENTRY_GROUP_E:
            ipLttEntryPtr->routeType = PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}


#endif /* EXMXPM_FAMILY */


/******************************************************************************\
 *                       CPSS generic API section                             *
\******************************************************************************/

/**
* @internal prvTgfIpRoutingModeGet function
* @endinternal
*
* @brief   Return the current routing mode
*
* @param[out] routingModePtr           - the current routing mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpRoutingModeGet
(
    OUT PRV_TGF_IP_ROUTING_MODE_ENT *routingModePtr
)
{
#ifdef CHX_FAMILY
    if (prvUtfIsPbrModeUsed())
    {
        *routingModePtr = PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E;
    }
    else
        *routingModePtr = PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    *routingModePtr = PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E;
#endif /* EXMXPM_FAMILY */

    return GT_OK;
}

/**
* @internal prvTgfIpUcRouteEntriesWrite function
* @endinternal
*
* @brief   Writes an array of uc route entries to hw
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpUcRouteEntriesWrite
(
    IN GT_U32                         baseRouteEntryIndex,
    IN PRV_TGF_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN GT_U32                         numOfRouteEntries
)
{
    GT_U8       devNum    = 0;
    GT_STATUS   rc, rc1   = GT_OK;
    GT_U32      entryIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *dxChRouteEntriesArray;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_UC_ROUTE_ENTRY_STC *exMxPmRouteEntriesArray;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    /* allocate memory */
    dxChRouteEntriesArray = cpssOsMalloc(numOfRouteEntries * sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChRouteEntriesArray, 0,
                 numOfRouteEntries * sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC));

    /* convert IP UC route entry into device specific format */
    for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
    {
        dxChRouteEntriesArray[entryIter].type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;

        /* convert IP UC route entry */
        rc = prvTgfConvertGenericToDxChIpUcRouteEntry(&routeEntriesArray[entryIter],
                                                 &(dxChRouteEntriesArray[entryIter].entry.regularEntry));

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIpUcRouteEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(dxChRouteEntriesArray);

            return rc;
        }
    }

    /* prepare device iterator */
    rc = prvUtfNextNotApplicableDeviceReset(&devNum, UTF_NONE_FAMILY_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfNextNotApplicableDeviceReset FAILED, rc = [%d]", rc);

        /* free allocated memory */
        cpssOsFree(dxChRouteEntriesArray);

        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpUcRouteEntriesWrite(devNum, baseRouteEntryIndex, dxChRouteEntriesArray, numOfRouteEntries);
        if (GT_OK != rc)
        {
            /* give indication that test that use IP failed due to lack of support in the CPSS for {dev,port}/{trunk}*/
            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(devNum))
            {
                for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
                {
                    if(dxChRouteEntriesArray[entryIter].entry.regularEntry.nextHopInterface.type == CPSS_INTERFACE_PORT_E)
                    {
                        if(dxChRouteEntriesArray[entryIter].entry.regularEntry.nextHopInterface.devPort.portNum >= BIT_6 ||
                           dxChRouteEntriesArray[entryIter].entry.regularEntry.nextHopInterface.devPort.hwDevNum >= BIT_5)
                        {
                            PRV_UTF_LOG1_MAC(" [%s]    prvTgfIpUcRouteEntriesWrite : the E-Arch for port >= 64 / hwDevNum >= 32 not supported yet \n",utfTestNameGet());
                            break;
                        }
                    }
                    else if(dxChRouteEntriesArray[entryIter].entry.regularEntry.nextHopInterface.type == CPSS_INTERFACE_TRUNK_E)
                    {
                        if(dxChRouteEntriesArray[entryIter].entry.regularEntry.nextHopInterface.trunkId >= BIT_7)
                        {
                            PRV_UTF_LOG1_MAC(" [%s]   prvTgfIpUcRouteEntriesWrite : the E-Arch for trunkId >= 127 not supported yet \n",utfTestNameGet());
                            break;
                        }
                    }
                }
            }


            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpUcRouteEntriesWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    /* free allocated memory */
    cpssOsFree(dxChRouteEntriesArray);

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* allocate memory */
    exMxPmRouteEntriesArray = cpssOsMalloc(numOfRouteEntries * sizeof(CPSS_EXMXPM_IP_UC_ROUTE_ENTRY_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) exMxPmRouteEntriesArray, 0,
                 numOfRouteEntries * sizeof(CPSS_EXMXPM_IP_UC_ROUTE_ENTRY_STC));

    /* convert IP UC route entry into device specific format */
    for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
    {
        exMxPmRouteEntriesArray[entryIter].type = CPSS_EXMXPM_IP_UC_ROUTE_ENTRY_E;

        /* convert IP UC route entry */
        rc = prvTgfConvertGenericToExMxPmIpUcRouteEntry(&routeEntriesArray[entryIter],
                                                        &(exMxPmRouteEntriesArray[entryIter].entry.regularEntry));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmIpUcRouteEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(exMxPmRouteEntriesArray);

            return rc;
        }
    }

    /* prepare device iterator */
    rc = prvUtfNextNotApplicableDeviceReset(&devNum, UTF_NONE_FAMILY_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfNextNotApplicableDeviceReset FAILED, rc = [%d]", rc);

        /* free allocated memory */
        cpssOsFree(exMxPmRouteEntriesArray);

        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmIpUcRouteEntriesWrite(devNum, baseRouteEntryIndex, numOfRouteEntries, exMxPmRouteEntriesArray);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpUcRouteEntriesWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    /* free allocated memory */
    cpssOsFree(exMxPmRouteEntriesArray);

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpUcRouteEntriesRead function
* @endinternal
*
* @brief   Reads an array of uc route entries from the hw
*
* @param[in] devNum                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to start reading
* @param[in,out] routeEntriesArray        - the uc route entries array
* @param[in] numOfRouteEntries        - the number route entries in the array
* @param[in,out] routeEntriesArray        - the uc route entries array read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpUcRouteEntriesRead
(
    IN    GT_U8                          devNum,
    IN    GT_U32                         baseRouteEntryIndex,
    INOUT PRV_TGF_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    IN    GT_U32                         numOfRouteEntries
)
{
    GT_STATUS   rc        = GT_OK;
    GT_U32      entryIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *dxChRouteEntriesArray;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_UC_ROUTE_ENTRY_STC *exMxPmRouteEntriesArray;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    /* allocate memory */
    dxChRouteEntriesArray = cpssOsMalloc(numOfRouteEntries * sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChRouteEntriesArray, 0,
                 numOfRouteEntries * sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC));

    /* convert IP UC route entry into device specific format */
    for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
    {
        dxChRouteEntriesArray[entryIter].type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;

        /* convert IP UC route entry */
        rc = prvTgfConvertGenericToDxChIpUcRouteEntry(&routeEntriesArray[entryIter],
                                                 &(dxChRouteEntriesArray[entryIter].entry.regularEntry));

        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIpUcRouteEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(dxChRouteEntriesArray);

            return rc;
        }
    }

    /* call device specific API */
    rc = cpssDxChIpUcRouteEntriesRead(devNum, baseRouteEntryIndex, dxChRouteEntriesArray, numOfRouteEntries);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpUcRouteEntriesRead FAILED, rc = [%d]", rc);

        /* free allocated memory */
        cpssOsFree(dxChRouteEntriesArray);

        return rc;
    }

    /* convert IP UC route entry from device specific format */
    for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
    {
        /* convert IP UC route entry */
        prvTgfConvertDxChToGenericIpUcRouteEntry(&(dxChRouteEntriesArray[entryIter].entry.regularEntry),
                                                 &routeEntriesArray[entryIter]);
    }

    /* free allocated memory */
    cpssOsFree(dxChRouteEntriesArray);

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* allocate memory */
    exMxPmRouteEntriesArray = cpssOsMalloc(numOfRouteEntries * sizeof(CPSS_EXMXPM_IP_UC_ROUTE_ENTRY_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) exMxPmRouteEntriesArray, 0,
                 numOfRouteEntries * sizeof(CPSS_EXMXPM_IP_UC_ROUTE_ENTRY_STC));

    /* convert IP UC route entry from device specific format */
    for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
    {
        exMxPmRouteEntriesArray[entryIter].type = CPSS_EXMXPM_IP_UC_ROUTE_ENTRY_E;

        /* convert IP UC route entry */
        rc = prvTgfConvertGenericToExMxPmIpUcRouteEntry(&routeEntriesArray[entryIter],
                                                        &(exMxPmRouteEntriesArray[entryIter].entry.regularEntry));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmIpUcRouteEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(exMxPmRouteEntriesArray);

            return rc;
        }
    }

    /* call device specific API */
    rc = cpssExMxPmIpUcRouteEntriesRead(devNum, baseRouteEntryIndex, numOfRouteEntries, exMxPmRouteEntriesArray);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpUcRouteEntriesRead FAILED, rc = [%d]", rc);

        /* free allocated memory */
        cpssOsFree(exMxPmRouteEntriesArray);

        return rc;
    }

    /* convert IP UC route entry from device specific format */
    for (entryIter = 0; entryIter < numOfRouteEntries; entryIter++)
    {
        /* convert IP UC route entry */
        rc = prvTgfConvertExMxPmToGenericIpUcRouteEntry(&(exMxPmRouteEntriesArray[entryIter].entry.regularEntry),
                                                        &routeEntriesArray[entryIter]);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertExMxPmToGenericIpUcRouteEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(exMxPmRouteEntriesArray);

            return rc;
        }
    }

    /* free allocated memory */
    cpssOsFree(exMxPmRouteEntriesArray);

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpRouterArpAddrWrite function
* @endinternal
*
* @brief   Write a ARP MAC address to the router ARP / Tunnel start Table
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type
*/
GT_STATUS prvTgfIpRouterArpAddrWrite
(
    IN GT_U32                         routerArpIndex,
    IN GT_ETHERADDR                  *arpMacAddrPtr
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpRouterArpAddrWrite(devNum, routerArpIndex, arpMacAddrPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpRouterArpAddrWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmIpRouterArpAddrWrite(devNum, routerArpIndex, arpMacAddrPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpRouterArpAddrWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpRouterArpAddrRead function
* @endinternal
*
* @brief   Read a ARP MAC address from the router ARP / Tunnel start Table
*
* @param[in] devNum                   - the device number
* @param[in] routerArpIndex           - The Arp Address index
*
* @param[out] arpMacAddrPtr            - the ARP MAC address to read
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - request is not supported for this device type
*/
GT_STATUS prvTgfIpRouterArpAddrRead
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        routerArpIndex,
    OUT GT_ETHERADDR                 *arpMacAddrPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterArpAddrRead(devNum, routerArpIndex, arpMacAddrPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmIpRouterArpAddrRead(devNum, routerArpIndex, arpMacAddrPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixAdd function
* @endinternal
*
* @brief   Creates a new or override an existing Ipv4 prefix
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on prefix length is too big
* @retval GT_ERROR                 - on the vrId was not created yet
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                vrId,
    IN GT_IPADDR                             ipAddr,
    IN GT_U32                                prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    IN GT_BOOL                               override,
    IN GT_BOOL                               defragmentationEnable
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChNextHopInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    PRV_TGF_PCL_ACTION_STC                *pclIpUcActionPtr;
    PRV_TGF_IP_LTT_ENTRY_STC              *ipLttEntryPtr;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC exMxPmNextHopInfo;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    cpssOsMemSet(&dxChNextHopInfo,0,sizeof(dxChNextHopInfo));

    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]\n", rc);

        return rc;
    }

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

            pclIpUcActionPtr = &nextHopInfoPtr->pclIpUcAction;

            rc = prvTgfConvertGenericToDxChRuleAction(pclIpUcActionPtr, &dxChNextHopInfo.pclIpUcAction);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

                return rc;
            }

            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            ipLttEntryPtr = &nextHopInfoPtr->ipLttEntry;

            /* set route type */
            rc = prvTgfConvertGenericToDxChRouteType(ipLttEntryPtr->routeType, &(dxChNextHopInfo.ipLttEntry.routeType));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);

                return rc;
            }

            /* convert ltt entry into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, numOfPaths);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, routeEntryBaseIndex);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, ucRPFCheckEnable);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, sipSaCheckMismatchEnable);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, ipv6MCGroupScopeLevel);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, priority);

            break;

        default:

            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &dxChNextHopInfo, override, defragmentationEnable);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    rc = prvTgfConvertGenericToExMxPmRoutePointer(&nextHopInfoPtr->ipLttEntry,
                                                  &exMxPmNextHopInfo);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRoutePointer FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, ipAddr, prefixLen, &exMxPmNextHopInfo, override);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmActivityBitEnableGet function
* @endinternal
*
* @brief   Get status of the aging/refresh mechanism of trie leaf entries in the
*         LPM memory.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmActivityBitEnableGet
(
    OUT GT_BOOL     *activityBitPtr
)
{
    return cpssDxChIpLpmActivityBitEnableGet(prvTgfDevNum, activityBitPtr);
}

/**
* @internal prvTgfIpLpmActivityBitEnableSet function
* @endinternal
*
* @brief   Set status of the aging/refresh mechanism of trie leaf entries in the
*         LPM memory.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmActivityBitEnableSet
(
    IN  GT_BOOL     activityBit
)
{
    return cpssDxChIpLpmActivityBitEnableSet(prvTgfDevNum, activityBit);
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv4 UC prefix for a specific LPM DB
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixActivityStatusGet
(
    IN GT_U32       lpmDbId,
    IN GT_U32       vrId,
    IN GT_IPADDR    ipAddr,
    IN GT_U32       prefixLen,
    IN GT_BOOL      clearActivity,
    OUT GT_BOOL     *activityStatusPtr
)
{
    return cpssDxChIpLpmIpv4UcPrefixActivityStatusGet(
                lpmDbId,
                vrId,
                &ipAddr,
                prefixLen,
                clearActivity,
                activityStatusPtr);
}

/**
* @internal prvTgfIpLpmIpv4McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv4 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2.
*
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv4McEntryActivityStatusGet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   ipGroup,
    IN  GT_U32      ipGroupPrefixLen,
    IN  GT_IPADDR   ipSrc,
    IN  GT_U32      ipSrcPrefixLen,
    IN  GT_BOOL     clearActivity,
    OUT GT_BOOL     *activityStatusPtr
)
{
    return cpssDxChIpLpmIpv4McEntryActivityStatusGet(
                lpmDbId,
                vrId,
                &ipGroup,
                ipGroupPrefixLen,
                &ipSrc,
                ipSrcPrefixLen,
                clearActivity,
                activityStatusPtr);
}

/**
* @internal prvTgfIpLpmIpv6McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2; Bobcat2.
*
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv6McEntryActivityStatusGet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR ipGroup,
    IN  GT_U32      ipGroupPrefixLen,
    IN  GT_IPV6ADDR ipSrc,
    IN  GT_U32      ipSrcPrefixLen,
    IN  GT_BOOL     clearActivity,
    OUT GT_BOOL     *activityStatusPtr
)
{
    return cpssDxChIpLpmIpv6McEntryActivityStatusGet(
                lpmDbId,
                vrId,
                &ipGroup,
                ipGroupPrefixLen,
                &ipSrc,
                ipSrcPrefixLen,
                clearActivity,
                activityStatusPtr);
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixActivityStatusGet
(
    IN GT_U32       lpmDbId,
    IN GT_U32       vrId,
    IN GT_IPV6ADDR  ipAddr,
    IN GT_U32       prefixLen,
    IN GT_BOOL      clearActivity,
    OUT GT_BOOL     *activityStatusPtr
)
{
    return cpssDxChIpLpmIpv6UcPrefixActivityStatusGet(
                lpmDbId,
                vrId,
                &ipAddr,
                prefixLen,
                clearActivity,
                activityStatusPtr);
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixDel function
* @endinternal
*
* @brief   Deletes an existing Ipv4 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         vrId,
    IN GT_IPADDR                      ipAddr,
    IN GT_U32                         prefixLen
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;


#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc =  cpssExMxPmIpLpmIpv4UcPrefixDelete(lpmDBId, vrId, ipAddr, prefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast IPv4 Routing table and stays with the default prefix
*         only for a specific LPM DB.
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4UcPrefixesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4UcPrefixesFlush FAILED, rc = [%d]", rc);
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc =  cpssExMxPmIpLpmIpv4UcPrefixesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpLpmIpv4UcPrefixesFlush FAILED, rc = [%d]", rc);
        return rc;
    }
    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixAdd function
* @endinternal
*
* @brief   Creates a new or override an existing Ipv6 prefix
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - If prefix length is too big.
* @retval GT_ERROR                 - If the vrId was not created yet.
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory.
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate TCAM memory.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                vrId,
    IN GT_IPV6ADDR                           ipAddr,
    IN GT_U32                                prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    IN GT_BOOL                               override,
    IN GT_BOOL                               defragmentationEnable
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChNextHopInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    PRV_TGF_PCL_ACTION_STC                *pclIpUcActionPtr;
    PRV_TGF_IP_LTT_ENTRY_STC              *ipLttEntryPtr;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC exMxPmNextHopInfo;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    cpssOsMemSet(&dxChNextHopInfo,0,sizeof(dxChNextHopInfo));

    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]\n", rc);

        return rc;
    }

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

            pclIpUcActionPtr = &nextHopInfoPtr->pclIpUcAction;

            rc = prvTgfConvertGenericToDxChRuleAction(pclIpUcActionPtr, &dxChNextHopInfo.pclIpUcAction);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

                return rc;
            }
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            ipLttEntryPtr = &nextHopInfoPtr->ipLttEntry;

            /* convert routeType into device specific format */
            rc = prvTgfConvertGenericToDxChRouteType(ipLttEntryPtr->routeType, &(dxChNextHopInfo.ipLttEntry.routeType));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);

                return rc;
            }
            /* convert ltt entry into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, numOfPaths);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, routeEntryBaseIndex);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, ucRPFCheckEnable);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, sipSaCheckMismatchEnable);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, ipv6MCGroupScopeLevel);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo.ipLttEntry), ipLttEntryPtr, priority);


            break;

        default:

            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId,
                                        vrId,
                                        &ipAddr,
                                        prefixLen,
                                        &dxChNextHopInfo,
                                        override,
                                        defragmentationEnable);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(defragmentationEnable);

    rc = prvTgfConvertGenericToExMxPmRoutePointer(&nextHopInfoPtr->ipLttEntry,
                                                  &exMxPmNextHopInfo);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRoutePointer FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen, &exMxPmNextHopInfo, override);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixDel function
* @endinternal
*
* @brief   Deletes an existing Ipv6 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         vrId,
    IN GT_IPV6ADDR                    ipAddr,
    IN GT_U32                         prefixLen
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv6UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv6UcPrefixDelete(lpmDBId, vrId, &ipAddr, prefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixesFlush function
* @endinternal
*
* @brief   Flushes the unicast IPv6 Routing table and stays with the default prefix
*         only for a specific LPM DB.
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv6UcPrefixesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv6UcPrefixesFlush FAILED, rc = [%d]", rc);
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv6UcPrefixesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpLpmIpv6UcPrefixesFlush FAILED, rc = [%d]", rc);
        return rc;
    }
    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCountersIpSet function
* @endinternal
*
* @brief   Set route entry mode and reset IP couters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvTgfCountersIpSet
(
    IN GT_U32                         portNum,
    IN GT_U32                         counterIndex
)
{
    GT_STATUS   rc       = GT_OK;
    GT_U32      portIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetMode       = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC interfaceModeCfg = {0};
    CPSS_DXCH_IP_COUNTER_SET_STC               ipCounters       = {0};
#endif /* CHX_FAMILY */
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_COUNTER_BIND_MODE_ENT        bindMode = CPSS_EXMXPM_IP_COUNTER_BIND_TO_INTERFACE_E;
    CPSS_EXMXPM_IP_COUNTER_SET_CFG_STC          exMxPmInterfaceModeCfg = {0};
    CPSS_EXMXPM_IP_COUNTER_SET_STC              ipCounters = {0};
#endif /* EXMXPM_FAMILY */


    /* find port index */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (prvTgfPortsArray[portIter] == portNum)
        {
            break;
        }
    }

    /* check if port is valid */
    if (prvTgfPortsArray[portIter] != portNum)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Port [%d] is not valid", portNum);

        return GT_BAD_PARAM;
    }

#ifdef CHX_FAMILY
    /* set counter mode */
    cntSetMode = CPSS_DXCH_IP_CNT_SET_ROUTE_ENTRY_MODE_E;

    /* set ROUTE_ENTRY mode for IP counters */
    interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
    interfaceModeCfg.ipMode           = CPSS_IP_PROTOCOL_IPV4_E;
    interfaceModeCfg.vlanMode         = CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E;
    interfaceModeCfg.hwDevNum         = prvTgfDevsArray[portIter]; /* don't care */
    interfaceModeCfg.portTrunk.port   = portNum;
    interfaceModeCfg.portTrunk.trunk  = 0;
    interfaceModeCfg.vlanId           = 0;

    rc = cpssDxChIpCntSetModeSet(prvTgfDevsArray[portIter], (CPSS_IP_CNT_SET_ENT) counterIndex,
                                 cntSetMode, &interfaceModeCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpCntSetModeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* reset IP couters */
    rc = cpssDxChIpCntSet(prvTgfDevsArray[portIter], (CPSS_IP_CNT_SET_ENT) counterIndex, &ipCounters);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpCntSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* set counter bind mode */
    bindMode = CPSS_EXMXPM_IP_COUNTER_BIND_TO_ROUTE_ENTRY_E;

    /* set ROUTE_ENTRY mode for IP counters */
    exMxPmInterfaceModeCfg.portTrunkMode      = CPSS_EXMXPM_IP_DISREGARD_PORT_TRUNK_COUNTER_MODE_E;
    exMxPmInterfaceModeCfg.portTrunk.devPort.devNum = prvTgfDevsArray[portIter];
    exMxPmInterfaceModeCfg.portTrunk.devPort.port   = portNum;
    exMxPmInterfaceModeCfg.portTrunk.trunk    = 0;
    exMxPmInterfaceModeCfg.packetType         = CPSS_EXMXPM_IP_PACKET_TYPE_IPV4_COUNTER_MODE_E;
    exMxPmInterfaceModeCfg.vlanMode           = CPSS_EXMXPM_IP_DISREGARD_VLAN_COUNTER_MODE_E;
    exMxPmInterfaceModeCfg.vlanId             = 0;

    rc = cpssExMxPmIpCounterConfigSet(prvTgfDevsArray[portIter], counterIndex, bindMode, &exMxPmInterfaceModeCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpCounterConfigSet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* reset IP couters */
    rc = cpssExMxPmIpCounterSet(prvTgfDevsArray[portIter], counterIndex, &ipCounters);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpCounterSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfCountersIpVlanModeSet function
* @endinternal
*
* @brief   Set route entry VLAN interface mode and reset IP couters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvTgfCountersIpVlanModeSet
(
    IN GT_U32                         vlanId,
    IN GT_U32                         counterIndex
)
{
    GT_STATUS   rc       = GT_OK;
    GT_U32      portIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetMode       = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC interfaceModeCfg = {0};
    CPSS_DXCH_IP_COUNTER_SET_STC               ipCounters       = {0};

    /* set ROUTE_ENTRY mode for IP counters */
    interfaceModeCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
    interfaceModeCfg.ipMode           = CPSS_IP_PROTOCOL_IPV4_E;
    interfaceModeCfg.vlanMode         = CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E;
    interfaceModeCfg.hwDevNum         = prvTgfDevsArray[portIter]; /* don't care */
    interfaceModeCfg.portTrunk.port   = 0;  /* don't care */
    interfaceModeCfg.portTrunk.trunk  = 0;  /* don't care */
    interfaceModeCfg.vlanId           = (GT_U16)vlanId;

    rc = cpssDxChIpCntSetModeSet(prvTgfDevsArray[portIter], (CPSS_IP_CNT_SET_ENT) counterIndex,
                                 cntSetMode, &interfaceModeCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpCntSetModeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* reset IP couters */
    rc = cpssDxChIpCntSet(prvTgfDevsArray[portIter], (CPSS_IP_CNT_SET_ENT) counterIndex, &ipCounters);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpCntSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfCountersIpGet function
* @endinternal
*
* @brief   This function gets the values of the various counters in the IP
*         Router Management Counter-Set.
* @param[in] devNum                   - device number
* @param[in] counterIndex             - counter index
* @param[in] enablePrint              - Enable/Disable output log
*
* @param[out] ipCountersPtr            - (pointer to) IP counters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfCountersIpGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        counterIndex,
    IN  GT_BOOL                       enablePrint,
    OUT PRV_TGF_IP_COUNTER_SET_STC   *ipCountersPtr
)
{
    GT_STATUS   rc       = GT_OK;
    GT_BOOL     isZero   = GT_TRUE;
    GT_U32      cntIter  = 0;
    GT_U32      cntNext  = 0;
    GT_U32      cntCount = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_COUNTER_SET_STC dxChIpCounters     = {0};
#endif /* CHX_FAMILY */
#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_COUNTER_SET_STC exMxPmIpCounters = {0};
#endif /* EXMXPM_FAMILY */
    GT_U8       ipCountersName[][PRV_TGF_MAX_FIELD_NAME_LEN_CNS] =
        {"inUcPkts", "inMcPkts", "inUcNonRoutedExcpPkts", "inUcNonRoutedNonExcpPkts",
         "inMcNonRoutedExcpPkts", "inMcNonRoutedNonExcpPkts", "inUcTrappedMirrorPkts",
         "inMcTrappedMirrorPkts", "mcRfpFailPkts", "outUcRoutedPkts"};


    /* get counter count */
    cntCount = sizeof(ipCountersName) / sizeof(ipCountersName[0]);

#ifdef CHX_FAMILY
    /* get counters */
    rc = cpssDxChIpCntGet(devNum, (CPSS_IP_CNT_SET_ENT) counterIndex, &dxChIpCounters);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpCntGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert counters from cpss */
    ipCountersPtr->inUcPkts = dxChIpCounters.inUcPkts;
    ipCountersPtr->inMcPkts = dxChIpCounters.inMcPkts;
    ipCountersPtr->inUcNonRoutedExcpPkts = dxChIpCounters.inUcNonRoutedExcpPkts;
    ipCountersPtr->inUcNonRoutedNonExcpPkts = dxChIpCounters.inUcNonRoutedNonExcpPkts;
    ipCountersPtr->inMcNonRoutedExcpPkts = dxChIpCounters.inMcNonRoutedExcpPkts;
    ipCountersPtr->inMcNonRoutedNonExcpPkts = dxChIpCounters.inMcNonRoutedNonExcpPkts;
    ipCountersPtr->inUcTrappedMirrorPkts = dxChIpCounters.inUcTrappedMirrorPkts;
    ipCountersPtr->inMcTrappedMirrorPkts = dxChIpCounters.inMcTrappedMirrorPkts;
    ipCountersPtr->mcRfpFailPkts = dxChIpCounters.mcRfpFailPkts;
    ipCountersPtr->outUcRoutedPkts = dxChIpCounters.outUcRoutedPkts;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* get counters */
    rc = cpssExMxPmIpCounterGet(devNum, counterIndex, &exMxPmIpCounters);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpCounterGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert counters from cpss */
    ipCountersPtr->inUcPkts = exMxPmIpCounters.inUcPkts;
    ipCountersPtr->inMcPkts = exMxPmIpCounters.inMcPkts;
    ipCountersPtr->inUcNonRoutedExcpPkts = exMxPmIpCounters.inUcNonRoutedExceptionPkts;
    ipCountersPtr->inUcNonRoutedNonExcpPkts = exMxPmIpCounters.inUcNonRoutedNonExceptionPkts;
    ipCountersPtr->inMcNonRoutedExcpPkts = exMxPmIpCounters.inMcNonRoutedExceptionPkts;
    ipCountersPtr->inMcNonRoutedNonExcpPkts = exMxPmIpCounters.inMcNonRoutedNonExceptionPkts;
    ipCountersPtr->inUcTrappedMirrorPkts = exMxPmIpCounters.inUcTrappedMirrorPkts;
    ipCountersPtr->inMcTrappedMirrorPkts = exMxPmIpCounters.inMcTrappedMirrorPkts;
    ipCountersPtr->mcRfpFailPkts = exMxPmIpCounters.inMcRfpFailPkts;
    ipCountersPtr->outUcRoutedPkts = exMxPmIpCounters.outUcPkts;
#endif /* EXMXPM_FAMILY */

    if (GT_TRUE == enablePrint)
    {
        /* print all not zero IP counters values */
        isZero = GT_TRUE;
        for (cntIter = 0; cntIter < cntCount; cntIter++)
        {
            /* next ip counter from CPSS_DXCH_IP_COUNTER_SET_STC */
            cntNext = *(cntIter + (GT_U32*) ipCountersPtr);

            if (cntNext != 0)
            {
                isZero = GT_FALSE;
                PRV_UTF_LOG2_MAC("  %s = %d\n", &ipCountersName[cntIter], cntNext);
            }
        }

        if (GT_TRUE == isZero)
        {
            PRV_UTF_LOG0_MAC("  All IP Counters are Zero\n");
        }

        PRV_UTF_LOG0_MAC("\n");
    }

    return rc;
}

/**
* @internal prvTgfIpPortRoutingEnable function
* @endinternal
*
* @brief   Enable multicast/unicast IPv4/v6 routing on a port
*
* @param[in] portIndex                - index of port to enable
* @param[in] ucMcEnable               - routing type to enable Unicast/Multicast
* @param[in] protocol                 - what type of traffic to enable ipv4 or ipv6 or both
* @param[in] enable                   -  IP routing for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpPortRoutingEnable
(
    IN GT_U8                          portIndex,
    IN CPSS_IP_UNICAST_MULTICAST_ENT  ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    IN GT_BOOL                        enable
)
{
    PRV_TGF_PORT_INDEX_CHECK_MAC(portIndex);

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpPortRoutingEnable(prvTgfDevsArray[portIndex], prvTgfPortsArray[portIndex],
                                       ucMcEnable, protocol, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    switch (ucMcEnable)
    {
        case CPSS_IP_UNICAST_E:
            /* call device specific API */
            return cpssExMxPmInlifIpUcRouteEnableSet(prvTgfDevsArray[portIndex],
                                                     CPSS_EXMXPM_INLIF_TYPE_PORT_E,
                                                     prvTgfPortsArray[portIndex],
                                                     protocol, enable);
        case CPSS_IP_MULTICAST_E:
            /* call device specific API */
            return cpssExMxPmInlifIpMcRouteEnableSet(prvTgfDevsArray[portIndex],
                                                     CPSS_EXMXPM_INLIF_TYPE_PORT_E,
                                                     prvTgfPortsArray[portIndex],
                                                     protocol, enable);
        default:
            return GT_BAD_PARAM;
    }

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfIpPortRoutingEnableGet function
* @endinternal
*
* @brief   Get status of multicast/unicast IPv4/v6 routing on a port.
*
* @param[out] enablePtr                - (pointer to)enable IP routing for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong protocolStack
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note CPSS_IP_PROTOCOL_IPV4V6_E is not supported in this get API.
*       Can not get both values for ipv4 and ipv6 in the same get.
*
*/
GT_STATUS prvTgfIpPortRoutingEnableGet
(
    IN GT_U8                          portIndex,
    IN CPSS_IP_UNICAST_MULTICAST_ENT  ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    OUT GT_BOOL                       *enablePtr
)
{
    PRV_TGF_PORT_INDEX_CHECK_MAC(portIndex);

#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpPortRoutingEnableGet(prvTgfDevsArray[portIndex], prvTgfPortsArray[portIndex],
                                       ucMcEnable, protocol, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(portIndex);
    TGF_PARAM_NOT_USED(ucMcEnable);
    TGF_PARAM_NOT_USED(protocol);
    TGF_PARAM_NOT_USED(enablePtr);

    /* call device specific API */
    return GT_NOT_SUPPORTED;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfIpVlanRoutingEnable function
* @endinternal
*
* @brief   Enable/Disable IPv4/Ipv6 multicast/unicast Routing on Vlan
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfIpVlanRoutingEnable
(
    IN GT_U16                         vlanId,
    IN CPSS_IP_UNICAST_MULTICAST_ENT  ucMcEnable,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    IN GT_BOOL                        enable
)
{
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


#ifdef CHX_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        switch (ucMcEnable)
        {
            case CPSS_IP_UNICAST_E:
                /* call device specific API */
                rc = cpssDxChBrgVlanIpUcRouteEnable(devNum, vlanId, protocol, enable);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanIpUcRouteEnable FAILED, rc = [%d]", rc);

                    rc1 = rc;
                }

                break;

            case CPSS_IP_MULTICAST_E:
                /* call device specific API */
                rc = cpssDxChBrgVlanIpMcRouteEnable(devNum, vlanId, protocol, enable);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgVlanIpMcRouteEnable FAILED, rc = [%d]", rc);

                    rc1 = rc;
                }

                break;

            default:
                rc1 = GT_BAD_PARAM;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        switch (ucMcEnable)
        {
            case CPSS_IP_UNICAST_E:
                /* call device specific API */
                rc = cpssExMxPmInlifIpUcRouteEnableSet(devNum, CPSS_EXMXPM_INLIF_TYPE_VLAN_E, vlanId, protocol, enable);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifIpUcRouteEnableSet FAILED, rc = [%d]", rc);

                    rc1 = rc;
                }

                break;

            case CPSS_IP_MULTICAST_E:
                /* call device specific API */
                rc = cpssExMxPmInlifIpMcRouteEnableSet(devNum, CPSS_EXMXPM_INLIF_TYPE_VLAN_E, vlanId, protocol, enable);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifIpMcRouteEnableSet FAILED, rc = [%d]", rc);

                    rc1 = rc;
                }

                break;

            default:
                rc1 = GT_BAD_PARAM;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfInlifEntrySet function
* @endinternal
*
* @brief   Set Inlif Entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfInlifEntrySet
(
    IN PRV_TGF_INLIF_TYPE_ENT         inlifType,
    IN GT_U32                         inlifIndex,
    IN PRV_TGF_INLIF_ENTRY_STC       *inlifEntryPtr

)
{
#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(inlifType);
    TGF_PARAM_NOT_USED(inlifIndex);
    TGF_PARAM_NOT_USED(inlifEntryPtr);

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8                           devNum           = 0;
    GT_STATUS                       rc, rc1          = GT_OK;
    CPSS_EXMXPM_INLIF_TYPE_ENT      exMxPmInlifType  = CPSS_EXMXPM_INLIF_TYPE_PORT_E;
    CPSS_EXMXPM_INLIF_ENTRY_STC     exMxPmInlifEntry = {0};


    /* convert Inlif type into device specific format */
    switch (inlifType)
    {
        case PRV_TGF_INLIF_TYPE_PORT_E:
            exMxPmInlifType  = CPSS_EXMXPM_INLIF_TYPE_PORT_E;
            break;

        case PRV_TGF_INLIF_TYPE_VLAN_E:
            exMxPmInlifType  = CPSS_EXMXPM_INLIF_TYPE_VLAN_E;
            break;

        case PRV_TGF_INLIF_TYPE_EXTERNAL_E:
            exMxPmInlifType  = CPSS_EXMXPM_INLIF_TYPE_EXTERNAL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert Inlif entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, bridgeEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, autoLearnEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, naMessageToCpuEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, naStormPreventionEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unkSaUcCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unkDaUcCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unkSaNotSecurBreachEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, untaggedMruIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unregNonIpMcCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unregIpMcCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unregIpv4BcCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unregNonIpv4BcCommand);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv4UcRouteEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv4McRouteEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6UcRouteEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6McRouteEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, mplsRouteEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, vrfId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv4IcmpRedirectEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6IcmpRedirectEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, bridgeRouterInterfaceEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipSecurityProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv4IgmpToCpuEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6IcmpToCpuEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, udpBcRelayEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, arpBcToCpuEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, arpBcToMeEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ripv1MirrorEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv4LinkLocalMcCommandEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6LinkLocalMcCommandEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6NeighborSolicitationEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, mirrorToAnalyzerEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, mirrorToCpuEnable);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmInlifEntrySet(devNum, exMxPmInlifType, inlifIndex, &exMxPmInlifEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfInlifEntryGet function
* @endinternal
*
* @brief   Get Inlif Entry
*
* @param[in] inlifType                - inlif type
* @param[in] inlifIndex               - inlif index
*
* @param[out] inlifEntryPtr            - (pointer to) the Inlif Fields
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfInlifEntryGet
(
    IN PRV_TGF_INLIF_TYPE_ENT         inlifType,
    IN GT_U32                         inlifIndex,
    OUT PRV_TGF_INLIF_ENTRY_STC       *inlifEntryPtr
)
{
#ifdef CHX_FAMILY
    TGF_PARAM_NOT_USED(inlifType);
    TGF_PARAM_NOT_USED(inlifIndex);
    TGF_PARAM_NOT_USED(inlifEntryPtr);

    return GT_OK;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8                           devNum           = 0;
    GT_STATUS                       rc, rc1          = GT_OK;
    CPSS_EXMXPM_INLIF_TYPE_ENT      exMxPmInlifType  = CPSS_EXMXPM_INLIF_TYPE_PORT_E;
    CPSS_EXMXPM_INLIF_ENTRY_STC     exMxPmInlifEntry = {0};

    /* convert Inlif type into device specific format */
    switch (inlifType)
    {
        case PRV_TGF_INLIF_TYPE_PORT_E:
            exMxPmInlifType  = CPSS_EXMXPM_INLIF_TYPE_PORT_E;
            break;

        case PRV_TGF_INLIF_TYPE_VLAN_E:
            exMxPmInlifType  = CPSS_EXMXPM_INLIF_TYPE_VLAN_E;
            break;

        case PRV_TGF_INLIF_TYPE_EXTERNAL_E:
            exMxPmInlifType  = CPSS_EXMXPM_INLIF_TYPE_EXTERNAL_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

     /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmInlifEntryGet(devNum, exMxPmInlifType, inlifIndex, &exMxPmInlifEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmInlifEntrySet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    /* convert Inlif entry into device specific format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, bridgeEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, autoLearnEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, naMessageToCpuEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, naStormPreventionEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unkSaUcCommand);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unkDaUcCommand);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unkSaNotSecurBreachEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, untaggedMruIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unregNonIpMcCommand);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unregIpMcCommand);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unregIpv4BcCommand);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, unregNonIpv4BcCommand);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv4UcRouteEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv4McRouteEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6UcRouteEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6McRouteEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, mplsRouteEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, vrfId);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv4IcmpRedirectEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6IcmpRedirectEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, bridgeRouterInterfaceEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipSecurityProfile);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv4IgmpToCpuEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6IcmpToCpuEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, udpBcRelayEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, arpBcToCpuEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, arpBcToMeEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ripv1MirrorEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv4LinkLocalMcCommandEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6LinkLocalMcCommandEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, ipv6NeighborSolicitationEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, mirrorToAnalyzerEnable);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(&exMxPmInlifEntry, inlifEntryPtr, mirrorToCpuEnable);

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmVirtualRouterAdd function
* @endinternal
*
* @brief   This function adds a virtual router in system for specific LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterAdd
(
    IN GT_U32                                lpmDbId,
    IN GT_U32                                vrId,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv4UcNextHopInfoPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv6UcNextHopInfoPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv4McRouteLttEntryPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv6McRouteLttEntryPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defFcoeNextHopInfoPtr
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo = {0};
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_LPM_VR_CONFIG_STC    exMxPmVrConfig = {0};
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    if (NULL != defIpv4UcNextHopInfoPtr)
    {
        rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                     defIpv4UcNextHopInfoPtr,
                                                     &vrConfigInfo.defIpv4UcNextHopInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
        vrConfigInfo.supportIpv4Uc = GT_TRUE;
    }
    if (NULL != defIpv6UcNextHopInfoPtr)
    {
        rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                     defIpv6UcNextHopInfoPtr,
                                                     &vrConfigInfo.defIpv6UcNextHopInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
        vrConfigInfo.supportIpv6Uc = GT_TRUE;
    }

    if (NULL != defIpv4McRouteLttEntryPtr)
    {
       rc = prvTgfConvertGenericToDxChMcLttEntryInfo(defIpv4McRouteLttEntryPtr,
                                                 &vrConfigInfo.defIpv4McRouteLttEntry);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
       vrConfigInfo.supportIpv4Mc = GT_TRUE;
    }
    if (NULL != defIpv6McRouteLttEntryPtr)
    {
       rc = prvTgfConvertGenericToDxChMcLttEntryInfo(defIpv6McRouteLttEntryPtr,
                                                     &vrConfigInfo.defIpv6McRouteLttEntry);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
       vrConfigInfo.supportIpv6Mc = GT_TRUE;
    }
    if (NULL != defFcoeNextHopInfoPtr)
    {
       rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                    defFcoeNextHopInfoPtr,
                                                    &vrConfigInfo.defaultFcoeForwardingNextHopInfo);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
       vrConfigInfo.supportFcoe = GT_TRUE;
    }

    /* call device specific API */
    return cpssDxChIpLpmVirtualRouterAdd(lpmDbId, vrId,&vrConfigInfo);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* clear entry */
    cpssOsMemSet((GT_VOID*) &exMxPmVrConfig, 0, sizeof(exMxPmVrConfig));
    if (NULL != defIpv4UcNextHopInfoPtr)
    {
        exMxPmVrConfig.supportUcIpv4 = GT_TRUE;
        /* set IPv4 UC entry */
        exMxPmVrConfig.defaultUcIpv4RouteEntry.routeEntryBaseMemAddr =
            defIpv4UcNextHopInfoPtr->ipLttEntry.routeEntryBaseIndex;
        exMxPmVrConfig.defaultUcIpv4RouteEntry.blockSize =
            defIpv4UcNextHopInfoPtr->ipLttEntry.numOfPaths;

        /* set route type */
        rc = prvTgfConvertGenericToExMxPmRouteType(defIpv4UcNextHopInfoPtr->ipLttEntry.routeType,
                                                   &(exMxPmVrConfig.defaultUcIpv4RouteEntry.routeEntryMethod));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRouteType FAILED, rc = [%d]", rc);
            return rc;
        }
        if((exMxPmVrConfig.defaultUcIpv4RouteEntry.blockSize==1)&&
           (exMxPmVrConfig.defaultUcIpv4RouteEntry.routeEntryMethod==CPSS_EXMXPM_IP_ECMP_ROUTE_ENTRY_GROUP_E))
        {
            exMxPmVrConfig.defaultUcIpv4RouteEntry.routeEntryMethod=CPSS_EXMXPM_IP_REGULAR_ROUTE_ENTRY_E;
        }
    }
    if (NULL != defIpv6UcNextHopInfoPtr)
    {
        exMxPmVrConfig.supportUcIpv6 = GT_TRUE;

        /* set IPv6 UC entry */
        exMxPmVrConfig.defaultUcIpv6RouteEntry.routeEntryBaseMemAddr =
            defIpv6UcNextHopInfoPtr->ipLttEntry.routeEntryBaseIndex;
        exMxPmVrConfig.defaultUcIpv6RouteEntry.blockSize =
            defIpv6UcNextHopInfoPtr->ipLttEntry.numOfPaths;

        /* set route type */
        rc = prvTgfConvertGenericToExMxPmRouteType(defIpv6UcNextHopInfoPtr->ipLttEntry.routeType,
                                                   &(exMxPmVrConfig.defaultUcIpv6RouteEntry.routeEntryMethod));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRouteType FAILED, rc = [%d]", rc);

            return rc;
        }
        if((exMxPmVrConfig.defaultUcIpv6RouteEntry.blockSize==1)&&
           (exMxPmVrConfig.defaultUcIpv6RouteEntry.routeEntryMethod==CPSS_EXMXPM_IP_ECMP_ROUTE_ENTRY_GROUP_E))
        {
            exMxPmVrConfig.defaultUcIpv6RouteEntry.routeEntryMethod=CPSS_EXMXPM_IP_REGULAR_ROUTE_ENTRY_E;
        }
    }

    if (NULL !=defIpv4McRouteLttEntryPtr)
    {
        exMxPmVrConfig.supportMcIpv4 = GT_TRUE;

        /* set IPv4 MC entry */
        exMxPmVrConfig.defaultMcIpv4RouteEntry.routeEntryBaseMemAddr =
            defIpv4McRouteLttEntryPtr->routeEntryBaseIndex;
        exMxPmVrConfig.defaultMcIpv4RouteEntry.blockSize =
            defIpv4McRouteLttEntryPtr->numOfPaths;

        /* set route type */
        rc = prvTgfConvertGenericToExMxPmRouteType(defIpv4McRouteLttEntryPtr->routeType,
                                                   &(exMxPmVrConfig.defaultMcIpv4RouteEntry.routeEntryMethod));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRouteType FAILED, rc = [%d]", rc);
            return rc;
        }
        if((exMxPmVrConfig.defaultMcIpv4RouteEntry.blockSize==1)&&
           (exMxPmVrConfig.defaultMcIpv4RouteEntry.routeEntryMethod==CPSS_EXMXPM_IP_ECMP_ROUTE_ENTRY_GROUP_E))
        {
            exMxPmVrConfig.defaultMcIpv4RouteEntry.routeEntryMethod=CPSS_EXMXPM_IP_REGULAR_ROUTE_ENTRY_E;
        }
    }

    if (NULL != defIpv6McRouteLttEntryPtr)
    {
        exMxPmVrConfig.supportMcIpv6 = GT_TRUE;

        /* set IPv6 MC entry */
        exMxPmVrConfig.defaultMcIpv6RouteEntry.routeEntryBaseMemAddr =
            defIpv6McRouteLttEntryPtr->routeEntryBaseIndex;
        exMxPmVrConfig.defaultMcIpv6RouteEntry.blockSize =
            defIpv6McRouteLttEntryPtr->numOfPaths;

        /* set route type */
        rc = prvTgfConvertGenericToExMxPmRouteType(defIpv6McRouteLttEntryPtr->routeType,
                                                   &(exMxPmVrConfig.defaultMcIpv6RouteEntry.routeEntryMethod));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRouteType FAILED, rc = [%d]", rc);
            return rc;
        }
        if((exMxPmVrConfig.defaultMcIpv6RouteEntry.blockSize==1)&&
           (exMxPmVrConfig.defaultMcIpv6RouteEntry.routeEntryMethod==CPSS_EXMXPM_IP_ECMP_ROUTE_ENTRY_GROUP_E))
        {
            exMxPmVrConfig.defaultMcIpv6RouteEntry.routeEntryMethod=CPSS_EXMXPM_IP_REGULAR_ROUTE_ENTRY_E;
        }
    }

    /* call device specific API */
    return cpssExMxPmIpLpmVirtualRouterAdd(lpmDbId, vrId, &exMxPmVrConfig);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmVirtualRouterAddDefault function
* @endinternal
*
* @brief   This function adds the default virtual router in the default LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterAddDefault
(
    GT_VOID
)
{
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT defIpv6UcNextHopInfo;
    PRV_TGF_IP_LTT_ENTRY_STC             defIpv4McRouteLttEntry;
    PRV_TGF_IP_LTT_ENTRY_STC             defIpv6McRouteLttEntry;

    cpssOsMemSet(&defIpv4UcNextHopInfo,0,sizeof(defIpv4UcNextHopInfo));
    cpssOsMemSet(&defIpv6UcNextHopInfo,0,sizeof(defIpv6UcNextHopInfo));
    cpssOsMemSet(&defIpv4McRouteLttEntry,0,sizeof(defIpv4McRouteLttEntry));
    cpssOsMemSet(&defIpv6McRouteLttEntry,0,sizeof(defIpv6McRouteLttEntry));

    defIpv4UcNextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    defIpv4UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    defIpv4UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv4UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    defIpv6UcNextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    defIpv6UcNextHopInfo.ipLttEntry.numOfPaths = 0;
    defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    defIpv6UcNextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    defIpv6UcNextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv6UcNextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    defIpv4McRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    defIpv4McRouteLttEntry.numOfPaths = 0;
    defIpv4McRouteLttEntry.routeEntryBaseIndex = 1;
    defIpv4McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    defIpv4McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv4McRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    defIpv6McRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    defIpv6McRouteLttEntry.numOfPaths = 0;
    defIpv6McRouteLttEntry.routeEntryBaseIndex = 1;
    defIpv6McRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    defIpv6McRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defIpv6McRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;

    return prvTgfIpLpmVirtualRouterAdd(0, 0,
                                       &defIpv4UcNextHopInfo,
                                       &defIpv6UcNextHopInfo,
                                       &defIpv4McRouteLttEntry,
                                       &defIpv6McRouteLttEntry,
                                       NULL);
}

/**
* @internal prvTgfIpLpmVirtualRouterSharedAdd function
* @endinternal
*
* @brief   This function adds shared virtual router in system for specific LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterSharedAdd
(
    IN GT_U32                                lpmDbId,
    IN GT_U32                                vrId,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv4UcNextHopInfoPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv6UcNextHopInfoPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv4McRouteLttEntryPtr,
    IN PRV_TGF_IP_LTT_ENTRY_STC             *defIpv6McRouteLttEntryPtr,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defFcoeNextHopInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vrConfigInfo = {0};
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
#endif /* CHX_FAMILY */


#ifdef CHX_FAMILY

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    if (NULL != defIpv4UcNextHopInfoPtr)
    {
        rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                     defIpv4UcNextHopInfoPtr,
                                                     &vrConfigInfo.defIpv4UcNextHopInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
        vrConfigInfo.supportIpv4Uc = GT_TRUE;
    }
    if (NULL != defIpv6UcNextHopInfoPtr)
    {
        rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                     defIpv6UcNextHopInfoPtr,
                                                     &vrConfigInfo.defIpv6UcNextHopInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
        vrConfigInfo.supportIpv6Uc = GT_TRUE;
    }

    if (NULL != defIpv4McRouteLttEntryPtr)
    {
       rc = prvTgfConvertGenericToDxChMcLttEntryInfo(defIpv4McRouteLttEntryPtr,
                                                 &vrConfigInfo.defIpv4McRouteLttEntry);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
       vrConfigInfo.supportIpv4Mc = GT_TRUE;
    }
    if (NULL != defIpv6McRouteLttEntryPtr)
    {
       rc = prvTgfConvertGenericToDxChMcLttEntryInfo(defIpv6McRouteLttEntryPtr,
                                                     &vrConfigInfo.defIpv6McRouteLttEntry);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
       vrConfigInfo.supportIpv6Mc = GT_TRUE;
    }
    if (NULL != defFcoeNextHopInfoPtr)
    {
        rc = prvTgfConvertGenericToDxChUcNextHopInfo(routingMode,
                                                     defFcoeNextHopInfoPtr,
                                                     &vrConfigInfo.defaultFcoeForwardingNextHopInfo);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
        vrConfigInfo.supportFcoe = GT_TRUE;
    }

    /* call device specific API */
    return cpssDxChIpLpmVirtualRouterSharedAdd(lpmDbId, vrId, &vrConfigInfo);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(defIpv4UcNextHopInfoPtr);
    TGF_PARAM_NOT_USED(defIpv6UcNextHopInfoPtr);
    TGF_PARAM_NOT_USED(defIpv4McRouteLttEntryPtr);
    TGF_PARAM_NOT_USED(defIpv6McRouteLttEntryPtr);
    TGF_PARAM_NOT_USED(defFcoeNextHopInfoPtr);
    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmVirtualRouterDel function
* @endinternal
*
* @brief   This function removes a virtual router in system for a specific LPM DB
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
*/
GT_STATUS prvTgfIpLpmVirtualRouterDel
(
    IN GT_U32                         lpmDbId,
    IN GT_U32                         vrId
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmIpLpmVirtualRouterDelete(lpmDbId, vrId);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmVirtualRouterDelDefault function
* @endinternal
*
* @brief   This function deletes the default virtual router in the default LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
*/
GT_STATUS prvTgfIpLpmVirtualRouterDelDefault
(
    GT_VOID
)
{
    return prvTgfIpLpmVirtualRouterDel(0, 0);
}

/**
* @internal prvTgfIpLpmVirtualRouterGet function
* @endinternal
*
* @brief   This function gets the virtual router in system for specific LPM DB
*
* @param[out] isSupportIpv4Uc          - (pointer to) whether VR support UC Ipv4
* @param[out] defIpv4UcNextHopInfoPtr  - (pointer to) ipv4 uc next hop info
* @param[out] isSupportIpv6Uc          - (pointer to) whether VR support UC Ipv6
* @param[out] defIpv6UcNextHopInfoPtr  - (pointer to) ipv6 uc next hop info
* @param[out] isSupportIpv4Mc          - (pointer to) whether VR support MC Ipv4
* @param[out] defIpv4McRouteLttEntryPtr - (pointer to) ipv4 mc LTT entry info
* @param[out] isSupportIpv6Mc          - (pointer to) whether VR support MC Ipv6
* @param[out] defIpv6McRouteLttEntryPtr - (pointer to) ipv6 mc LTT entry info
*
* @retval GT_OK                    - on success on success
* @retval GT_NOT_FOUND             - on the LPM DB id is not found
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - on the existing VR is not empty.
*/
GT_STATUS prvTgfIpLpmVirtualRouterGet
(
    IN  GT_U32                                lpmDbId,
    IN  GT_U32                                vrId,
    OUT GT_BOOL                              *isSupportIpv4Uc,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv4UcNextHopInfoPtr,
    OUT GT_BOOL                              *isSupportIpv6Uc,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *defIpv6UcNextHopInfoPtr,
    OUT GT_BOOL                              *isSupportIpv4Mc,
    OUT PRV_TGF_IP_LTT_ENTRY_STC             *defIpv4McRouteLttEntryPtr,
    OUT GT_BOOL                              *isSupportIpv6Mc,
    OUT PRV_TGF_IP_LTT_ENTRY_STC             *defIpv6McRouteLttEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                       rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC  vrConfigInfo = {0};
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;


    CPSS_NULL_PTR_CHECK_MAC(defIpv4UcNextHopInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(defIpv6UcNextHopInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(defIpv4McRouteLttEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(defIpv6McRouteLttEntryPtr);

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    /* call device specific API */
    rc = cpssDxChIpLpmVirtualRouterGet(lpmDbId, vrId, &vrConfigInfo);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmVirtualRouterGet FAILED, rc = [%d]", rc);
        return rc;
    }

    *isSupportIpv4Uc = vrConfigInfo.supportIpv4Uc;
    *isSupportIpv6Uc = vrConfigInfo.supportIpv6Uc;
    *isSupportIpv4Mc = vrConfigInfo.supportIpv4Mc;
    *isSupportIpv6Mc = vrConfigInfo.supportIpv6Mc;

    /* convert generic UC next hop info from DX specific UC next hop info */
    if (GT_TRUE == vrConfigInfo.supportIpv4Uc)
    {
        rc = prvTgfConvertDxChToGenericUcNextHopInfo(routingMode,
                                                     &vrConfigInfo.defIpv4UcNextHopInfo,
                                                     defIpv4UcNextHopInfoPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
    }

    /* convert generic UC next hop info from DX specific UC next hop info */
    if (GT_TRUE == vrConfigInfo.supportIpv6Uc)
    {
        rc = prvTgfConvertDxChToGenericUcNextHopInfo(routingMode,
                                                     &vrConfigInfo.defIpv6UcNextHopInfo,
                                                     defIpv6UcNextHopInfoPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericUcNextHopInfo FAILED, rc = [%d]", rc);
            return rc;
        }
    }

    /* convert generic mc LTT info from DX specific mc LTT info */
    if (GT_TRUE == vrConfigInfo.supportIpv4Mc)
    {
        rc = prvTgfConvertDxChToGenericMcLttEntryInfo(&vrConfigInfo.defIpv4McRouteLttEntry,
                                                      defIpv4McRouteLttEntryPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericMcLttEntryInfo FAILED, rc = [%d]", rc);

            return rc;
        }
    }

    /* convert generic mc LTT info from DX specific mc LTT info */
    if (GT_TRUE == vrConfigInfo.supportIpv6Mc)
    {
       rc = prvTgfConvertDxChToGenericMcLttEntryInfo(&vrConfigInfo.defIpv6McRouteLttEntry,
                                                     defIpv6McRouteLttEntryPtr);
       if (GT_OK != rc)
       {
           PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericMcLttEntryInfo FAILED, rc = [%d]", rc);

           return rc;
       }
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(isSupportIpv4Uc);
    TGF_PARAM_NOT_USED(defIpv4UcNextHopInfoPtr);
    TGF_PARAM_NOT_USED(isSupportIpv6Uc);
    TGF_PARAM_NOT_USED(defIpv6UcNextHopInfoPtr);
    TGF_PARAM_NOT_USED(isSupportIpv4Mc);
    TGF_PARAM_NOT_USED(defIpv4McRouteLttEntryPtr);
    TGF_PARAM_NOT_USED(isSupportIpv6Mc);
    TGF_PARAM_NOT_USED(defIpv6McRouteLttEntryPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLttWrite function
* @endinternal
*
* @brief   Writes a LookUp Translation Table Entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpLttWrite
(
    IN GT_U32                         lttTtiRow,
    IN GT_U32                         lttTtiColumn,
    IN PRV_TGF_IP_LTT_ENTRY_STC      *lttEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_U8                       devNum  = 0;
    GT_STATUS                   rc, rc1 = GT_OK;
    CPSS_DXCH_IP_LTT_ENTRY_STC  dxChLttEntry;

    cpssOsMemSet(&dxChLttEntry,0,sizeof(dxChLttEntry));


    /* set LTT entry */
    dxChLttEntry.numOfPaths = lttEntryPtr->numOfPaths;
    dxChLttEntry.routeEntryBaseIndex = lttEntryPtr->routeEntryBaseIndex;
    dxChLttEntry.ucRPFCheckEnable = lttEntryPtr->ucRPFCheckEnable;
    dxChLttEntry.sipSaCheckMismatchEnable = lttEntryPtr->sipSaCheckMismatchEnable;
    dxChLttEntry.ipv6MCGroupScopeLevel = lttEntryPtr->ipv6MCGroupScopeLevel;

    /* set route type */
    rc = prvTgfConvertGenericToDxChRouteType(lttEntryPtr->routeType, &(dxChLttEntry.routeType));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRouteType FAILED, rc = [%d]", rc);

        return rc;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpLttWrite(devNum, lttTtiRow, lttTtiColumn, &dxChLttEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLttWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(lttTtiRow);
    TGF_PARAM_NOT_USED(lttTtiColumn);
    TGF_PARAM_NOT_USED(lttEntryPtr);

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpv4PrefixSet function
* @endinternal
*
* @brief   Sets an ipv4 UC or MC prefix to the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv4PrefixSet
(
    IN GT_U32                         routerTtiTcamRow,
    IN GT_U32                         routerTtiTcamColumn,
    IN PRV_TGF_IPV4_PREFIX_STC       *prefixPtr,
    IN PRV_TGF_IPV4_PREFIX_STC       *maskPtr
)
{
#ifdef CHX_FAMILY
    GT_U8                       devNum  = 0;
    GT_STATUS                   rc, rc1 = GT_OK;
    CPSS_DXCH_IPV4_PREFIX_STC   dxChPrefix;
    CPSS_DXCH_IPV4_PREFIX_STC   dxChMask;

    cpssOsMemSet(&dxChPrefix,0,sizeof(dxChPrefix));
    cpssOsMemSet(&dxChMask,0,sizeof(dxChMask));


    /* convert Ipv4 prefix entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, vrId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, ipAddr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, isMcSource);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, mcGroupIndexRow);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, mcGroupIndexColumn);

    /* convert Ipv4 mask entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, vrId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, ipAddr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, isMcSource);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, mcGroupIndexRow);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, mcGroupIndexColumn);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpv4PrefixSet(devNum, routerTtiTcamRow, routerTtiTcamColumn,
                                   &dxChPrefix, &dxChMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpv4PrefixSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(routerTtiTcamRow);
    TGF_PARAM_NOT_USED(routerTtiTcamColumn);
    TGF_PARAM_NOT_USED(prefixPtr);
    TGF_PARAM_NOT_USED(maskPtr);

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpv6PrefixSet function
* @endinternal
*
* @brief   Set an ipv6 UC or MC prefix to the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv6PrefixSet
(
    IN GT_U32                  routerTtiTcamRow,
    IN PRV_TGF_IPV6_PREFIX_STC *prefixPtr,
    IN PRV_TGF_IPV6_PREFIX_STC *maskPtr
)
{
#ifdef CHX_FAMILY
    GT_U8                     devNum  = 0;
    GT_STATUS                 rc, rc1 = GT_OK;
    CPSS_DXCH_IPV6_PREFIX_STC dxChPrefix;
    CPSS_DXCH_IPV6_PREFIX_STC dxChMask;

    cpssOsMemSet(&dxChPrefix,0,sizeof(dxChPrefix));
    cpssOsMemSet(&dxChMask,0,sizeof(dxChMask));

    /* convert Ipv6 prefix entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, vrId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, ipAddr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, isMcSource);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChPrefix, prefixPtr, mcGroupIndexRow);

    /* convert Ipv6 mask entry into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, vrId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, ipAddr);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, isMcSource);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChMask, maskPtr, mcGroupIndexRow);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpv6PrefixSet(devNum, routerTtiTcamRow,
                                   &dxChPrefix, &dxChMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpv6PrefixSet FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(routerTtiTcamRow);
    TGF_PARAM_NOT_USED(prefixPtr);
    TGF_PARAM_NOT_USED(maskPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpv4PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv4 UC or MC prefix in the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv4PrefixInvalidate
(
    IN GT_U32                         routerTtiTcamRow,
    IN GT_U32                         routerTtiTcamColumn
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpv4PrefixInvalidate(devNum, routerTtiTcamRow, routerTtiTcamColumn);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpv4PrefixInvalidate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(routerTtiTcamRow);
    TGF_PARAM_NOT_USED(routerTtiTcamColumn);

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpv6PrefixInvalidate function
* @endinternal
*
* @brief   Invalidates an ipv6 UC or MC prefix in the Router Tcam
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active
*/
GT_STATUS prvTgfIpv6PrefixInvalidate
(
    IN GT_U32 routerTtiTcamRow
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpv6PrefixInvalidate(devNum, routerTtiTcamRow);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpv6PrefixInvalidate FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    TGF_PARAM_NOT_USED(routerTtiTcamRow);

    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpMcRouteEntriesWrite function
* @endinternal
*
* @brief   Write an array of MC route entries to hw
*
* @param[in] ucMcBaseIndex            - base Index in the Route entries table
* @param[in] numOfEntries             - number of route entries to write
* @param[in] protocol                 - ip  type
* @param[in] entriesArrayPtr          - MC route entry array to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvTgfIpMcRouteEntriesWrite
(
    IN GT_U32                         ucMcBaseIndex,
    IN GT_U32                         numOfEntries,
    IN CPSS_IP_PROTOCOL_STACK_ENT     protocol,
    IN PRV_TGF_IP_MC_ROUTE_ENTRY_STC *entriesArrayPtr
)
{
    GT_U8       devNum    = 0;
    GT_STATUS   rc, rc1   = GT_OK;
    GT_U32      entryIter = 0;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC *dxChRouteEntriesArray;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_MC_ROUTE_ENTRY_STC *exMxPmRouteEntriesArray;
#endif /* EXMXPM_FAMILY */

    switch (protocol) {
    case CPSS_IP_PROTOCOL_IPV4_E:
    case CPSS_IP_PROTOCOL_IPV6_E:
        break;
    default:
        return GT_BAD_PARAM;
    }
    for (entryIter = 0; entryIter < numOfEntries; entryIter++)
    {
        if (protocol == CPSS_IP_PROTOCOL_IPV6_E)
        {
            if (entriesArrayPtr[entryIter].ipv6InternalMllPtr != entriesArrayPtr[entryIter].ipv6ExternalMllPtr + 1 )
            {
              rc = GT_BAD_PARAM;
              PRV_UTF_LOG1_MAC("[TGF]: ipv6InternalMllPtr should be equal to ipv6ExternalMllPtr + 1 to support EXMXPM dit internal/external functionality , rc = [%d]", rc);
            }
        }
    }

#ifdef CHX_FAMILY
    /* allocate memory */
    dxChRouteEntriesArray = cpssOsMalloc(numOfEntries * sizeof(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChRouteEntriesArray, 0,
                 numOfEntries * sizeof(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC));

    /* convert IP MC route entry into device specific format */
    for (entryIter = 0; entryIter < numOfEntries; entryIter++)
    {
        /* convert IP MC route entry */
        rc = prvTgfConvertGenericToDxChIpMcRouteEntry(protocol,
                                                      &entriesArrayPtr[entryIter],
                                                      &(dxChRouteEntriesArray[entryIter]));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChIpMcRouteEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(dxChRouteEntriesArray);

            return rc;
        }
    }

    /* prepare device iterator */
    rc = prvUtfNextNotApplicableDeviceReset(&devNum, UTF_NONE_FAMILY_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfNextNotApplicableDeviceReset FAILED, rc = [%d]", rc);

        /* free allocated memory */
        cpssOsFree(dxChRouteEntriesArray);

        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for (entryIter = 0; entryIter < numOfEntries; entryIter++)
        {
            /* call device specific API */
            rc = cpssDxChIpMcRouteEntriesWrite(devNum, ucMcBaseIndex + entryIter, &(dxChRouteEntriesArray[entryIter]));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpMcRouteEntriesWrite FAILED, rc = [%d]", rc);

                rc1 = rc;
            }
        }
    }

    /* free allocated memory */
    cpssOsFree(dxChRouteEntriesArray);

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* allocate memory */
    exMxPmRouteEntriesArray = cpssOsMalloc(numOfEntries * sizeof(CPSS_EXMXPM_IP_MC_ROUTE_ENTRY_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) exMxPmRouteEntriesArray, 0,
                 numOfEntries * sizeof(CPSS_EXMXPM_IP_MC_ROUTE_ENTRY_STC));

    /* convert IP MC route entry into device specific format */
    for (entryIter = 0; entryIter < numOfEntries; entryIter++)
    {
        /* convert IP MC route entry */
        rc = prvTgfConvertGenericToExMxPmIpMcRouteEntry(protocol,
                                                        &entriesArrayPtr[entryIter],
                                                        &(exMxPmRouteEntriesArray[entryIter]));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmIpMcRouteEntry FAILED, rc = [%d]", rc);

            /* free allocated memory */
            cpssOsFree(exMxPmRouteEntriesArray);

            return rc;
        }
    }

    /* prepare device iterator */
    rc = prvUtfNextNotApplicableDeviceReset(&devNum, UTF_NONE_FAMILY_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvUtfNextNotApplicableDeviceReset FAILED, rc = [%d]", rc);

        /* free allocated memory */
        cpssOsFree(exMxPmRouteEntriesArray);

        return rc;
    }

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmIpMcRouteEntriesWrite(devNum, ucMcBaseIndex, numOfEntries, exMxPmRouteEntriesArray);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpMcRouteEntriesWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    /* free allocated memory */
    cpssOsFree(exMxPmRouteEntriesArray);

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfIpMllPairWrite function
* @endinternal
*
* @brief   Write a Mc Link List (MLL) pair entry to hw
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active or invalid mllPairWriteForm.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpMllPairWrite
(
    IN GT_U32                                   mllPairEntryIndex,
    IN PRV_TGF_PAIR_READ_WRITE_FORM_ENT         mllPairWriteForm,
    IN PRV_TGF_IP_MLL_PAIR_STC                  *mllPairEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT   dxChMllPairWriteForm;
    CPSS_DXCH_IP_MLL_PAIR_STC            dxChMllPairEntry;

    cpssOsMemSet(&dxChMllPairEntry,0,sizeof(dxChMllPairEntry));


    /* convert mllPairWriteForm into device specific format */
    switch (mllPairWriteForm)
    {
        case PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E:
            dxChMllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E;
            break;

        case PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
            dxChMllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E;
            break;

        case PRV_TGF_PAIR_READ_WRITE_WHOLE_E:
            dxChMllPairWriteForm = CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert mllPairEntryPtr->firstMllNode into device specific format */
    dxChMllPairEntry.firstMllNode.mllRPFFailCommand = mllPairEntryPtr->firstMllNode.mllRPFFailCommand;
    dxChMllPairEntry.firstMllNode.isTunnelStart = mllPairEntryPtr->firstMllNode.isTunnelStart;
    dxChMllPairEntry.firstMllNode.nextHopVlanId = mllPairEntryPtr->firstMllNode.nextHopVlanId;
    dxChMllPairEntry.firstMllNode.nextHopTunnelPointer = mllPairEntryPtr->firstMllNode.nextHopTunnelPointer;

    cpssOsMemCpy((GT_VOID*) &(dxChMllPairEntry.firstMllNode.nextHopInterface),
                 (GT_VOID*) &(mllPairEntryPtr->firstMllNode.nextHopInterface),
                  sizeof(mllPairEntryPtr->firstMllNode.nextHopInterface));

    /* convert firstMllNode.tunnelStartPassengerType into device specific format */
    switch (mllPairEntryPtr->firstMllNode.tunnelStartPassengerType)
    {
        case PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E:
            dxChMllPairEntry.firstMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
            break;

        case PRV_TGF_TUNNEL_PASSENGER_OTHER_E:
            dxChMllPairEntry.firstMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    dxChMllPairEntry.firstMllNode.ttlHopLimitThreshold = mllPairEntryPtr->firstMllNode.ttlHopLimitThreshold;
    dxChMllPairEntry.firstMllNode.excludeSrcVlan = mllPairEntryPtr->firstMllNode.excludeSrcVlan;
    dxChMllPairEntry.firstMllNode.last = mllPairEntryPtr->firstMllNode.last;

    /* convert mllPairEntryPtr->secondMllNode into device specific format */
    dxChMllPairEntry.secondMllNode.mllRPFFailCommand = mllPairEntryPtr->secondMllNode.mllRPFFailCommand;
    dxChMllPairEntry.secondMllNode.isTunnelStart = mllPairEntryPtr->secondMllNode.isTunnelStart;
    dxChMllPairEntry.secondMllNode.nextHopVlanId = mllPairEntryPtr->secondMllNode.nextHopVlanId;
    dxChMllPairEntry.secondMllNode.nextHopTunnelPointer = mllPairEntryPtr->secondMllNode.nextHopTunnelPointer;

    cpssOsMemCpy((GT_VOID*) &(dxChMllPairEntry.secondMllNode.nextHopInterface),
                     (GT_VOID*) &(mllPairEntryPtr->secondMllNode.nextHopInterface),
                      sizeof(mllPairEntryPtr->secondMllNode.nextHopInterface));

    /* convert secondMllNode.tunnelStartPassengerType into device specific format */
    switch (mllPairEntryPtr->secondMllNode.tunnelStartPassengerType)
    {
        case PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E:
            dxChMllPairEntry.secondMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
            break;

        case PRV_TGF_TUNNEL_PASSENGER_OTHER_E:
            dxChMllPairEntry.secondMllNode.tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    dxChMllPairEntry.secondMllNode.ttlHopLimitThreshold = mllPairEntryPtr->secondMllNode.ttlHopLimitThreshold;
    dxChMllPairEntry.secondMllNode.excludeSrcVlan = mllPairEntryPtr->secondMllNode.excludeSrcVlan;
    dxChMllPairEntry.secondMllNode.last = mllPairEntryPtr->secondMllNode.last;

    dxChMllPairEntry.nextPointer = mllPairEntryPtr->nextPointer;

    if(mllPairWriteForm == PRV_TGF_PAIR_READ_WRITE_WHOLE_E ||
        mllPairWriteForm == PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E)
    {
        if( CPSS_INTERFACE_PORT_E == mllPairEntryPtr->firstMllNode.nextHopInterface.type )
        {
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH
            rc = prvUtfHwDeviceNumberGet((GT_U8)mllPairEntryPtr->firstMllNode.nextHopInterface.devPort.hwDevNum,
                                         &dxChMllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

                return rc;
            }
        }
    }

    if(mllPairWriteForm == PRV_TGF_PAIR_READ_WRITE_WHOLE_E ||
        mllPairWriteForm ==
        PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E)
    {
        if( CPSS_INTERFACE_PORT_E == mllPairEntryPtr->secondMllNode.nextHopInterface.type )
        {
            /* need to remove casting and to fix code after lion2 development is done */
            CPSS_TBD_BOOKMARK_EARCH

            rc = prvUtfHwDeviceNumberGet((GT_U8)mllPairEntryPtr->secondMllNode.nextHopInterface.devPort.hwDevNum,
                                         &dxChMllPairEntry.secondMllNode.nextHopInterface.devPort.hwDevNum);
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvUtfHwDeviceNumberGet FAILED, rc = [%d]", rc);

                return rc;
            }
        }
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpMLLPairWrite(devNum, mllPairEntryIndex, dxChMllPairWriteForm, &dxChMllPairEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpMLLPairWrite FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_EXMXPM_DIT_IP_MC_PAIR_READ_WRITE_MODE_ENT  exMxPmDitPairWriteForm;
    CPSS_EXMXPM_DIT_IP_MC_PAIR_STC                  exMxPmDitPairEntry;
    /* convert mllPairWriteForm into device specific format */
    switch (mllPairWriteForm)
    {
        case PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E:
            exMxPmDitPairWriteForm = CPSS_EXMXPM_DIT_IP_MC_PAIR_READ_WRITE_FIRST_DIT_ONLY_E;
            break;

        case PRV_TGF_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
            exMxPmDitPairWriteForm = CPSS_EXMXPM_DIT_IP_MC_PAIR_READ_WRITE_SECOND_DIT_NEXT_POINTER_ONLY_E;
            break;

        case PRV_TGF_PAIR_READ_WRITE_WHOLE_E:
            exMxPmDitPairWriteForm = CPSS_EXMXPM_DIT_IP_MC_PAIR_READ_WRITE_WHOLE_E;
            break;

        default:
            return GT_BAD_PARAM;
    }
    /* convert mllPairEntryPtr->firstMllNode into device specific format */
    exMxPmDitPairEntry.firstDitNode.ditRpfFailCommand = mllPairEntryPtr->firstMllNode.mllRPFFailCommand;
    exMxPmDitPairEntry.firstDitNode.isTunnelStart = mllPairEntryPtr->firstMllNode.isTunnelStart;
    exMxPmDitPairEntry.firstDitNode.nextHopVlanId = mllPairEntryPtr->firstMllNode.nextHopVlanId;
    exMxPmDitPairEntry.firstDitNode.nextHopTunnelPointer = mllPairEntryPtr->firstMllNode.nextHopTunnelPointer;
    cpssOsMemCpy((GT_VOID*) &(exMxPmDitPairEntry.firstDitNode.nextHopIf),
                 (GT_VOID*) &(mllPairEntryPtr->firstMllNode.nextHopInterface),
                  sizeof(mllPairEntryPtr->firstMllNode.nextHopInterface));

    exMxPmDitPairEntry.firstDitNode.ttlHopLimitThreshold = mllPairEntryPtr->firstMllNode.ttlHopLimitThreshold;
    exMxPmDitPairEntry.firstDitNode.excludeSourceVlanIf = mllPairEntryPtr->firstMllNode.excludeSrcVlan;
    exMxPmDitPairEntry.firstDitNode.isLast = mllPairEntryPtr->firstMllNode.last;

    /* convert mllPairEntryPtr->secondMllNode into device specific format */
    exMxPmDitPairEntry.secondDitNode.ditRpfFailCommand = mllPairEntryPtr->secondMllNode.mllRPFFailCommand;
    exMxPmDitPairEntry.secondDitNode.isTunnelStart = mllPairEntryPtr->secondMllNode.isTunnelStart;
    exMxPmDitPairEntry.secondDitNode.nextHopVlanId = mllPairEntryPtr->secondMllNode.nextHopVlanId;
    exMxPmDitPairEntry.secondDitNode.nextHopTunnelPointer = mllPairEntryPtr->secondMllNode.nextHopTunnelPointer;
    cpssOsMemCpy((GT_VOID*) &(exMxPmDitPairEntry.secondDitNode.nextHopIf),
                 (GT_VOID*) &(mllPairEntryPtr->secondMllNode.nextHopInterface),
                  sizeof(mllPairEntryPtr->secondMllNode.nextHopInterface));

    exMxPmDitPairEntry.secondDitNode.ttlHopLimitThreshold = mllPairEntryPtr->secondMllNode.ttlHopLimitThreshold;
    exMxPmDitPairEntry.secondDitNode.excludeSourceVlanIf = mllPairEntryPtr->secondMllNode.excludeSrcVlan;
    exMxPmDitPairEntry.secondDitNode.isLast = mllPairEntryPtr->secondMllNode.last;

    exMxPmDitPairEntry.ditNextPointer = mllPairEntryPtr->nextPointer;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

        /* call device specific API */
        rc = cpssExMxPmDitIpMcPairWrite(devNum, mllPairEntryIndex, exMxPmDitPairWriteForm, &exMxPmDitPairEntry);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpMLLPairWrite FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmDBDevListAdd function
* @endinternal
*
* @brief   This function adds devices to an existing LPM DB
*
* @param[in] lpmDBId                  - the LPM DB id
* @param[in] devList[]                - the array of device ids to add to the LPM DB
* @param[in] numOfDevs                - the number of device ids in the array
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfIpLpmDBDevListAdd
(
    IN GT_U32                         lpmDBId,
    IN GT_U8                          devList[],
    IN GT_U32                         numOfDevs
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpLpmDBDevListAdd(lpmDBId, devList, numOfDevs);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmIpLpmDbDevListAdd(lpmDBId, numOfDevs, devList);
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmDBDevListAddDefault function
* @endinternal
*
* @brief   This function adds a default device to an existing LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvTgfIpLpmDBDevListAddDefault
(
    GT_VOID
)
{
    return prvTgfIpLpmDBDevListAdd(0,&prvTgfDevNum,1);
}

/**
* @internal prvTgfIpLpmIpv4McEntryAdd function
* @endinternal
*
* @brief   Add IP MC route for a particular/all source and group address
*
* @param[in] lpmDBId                  - LPM DB id
* @param[in] vrId                     - virtual private network identifier
* @param[in] ipGroup                  - IP MC group address
* @param[in] ipGroupPrefixLen         - number of bits that are actual valid in ipGroup
* @param[in] ipSrc                    - root address for source base multi tree protocol
* @param[in] ipSrcPrefixLen           - number of bits that are actual valid in ipSrc
* @param[in] mcRouteLttEntryPtr       - LTT entry pointing to the MC route entry
* @param[in] override                 - whether to  an mc Route pointer
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - if one of prefixes' lengths is too big
* @retval GT_ERROR                 - if the virtual router does not exist
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer
*/
GT_STATUS prvTgfIpLpmIpv4McEntryAdd
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPADDR                    ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPADDR                    ipSrc,
    IN GT_U32                       ipSrcPrefixLen,
    IN PRV_TGF_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    IN GT_BOOL                      override,
    IN GT_BOOL                      defragmentationEnable
)
{

    GT_STATUS                   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LTT_ENTRY_STC  dxChMcRouteLttEntry = {0};


    /* set LTT entry */
    rc = prvTgfConvertGenericToDxChMcLttEntryInfo(mcRouteLttEntryPtr, &dxChMcRouteLttEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChMcLttEntryInfo FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                     &ipSrc, ipSrcPrefixLen, &dxChMcRouteLttEntry,
                                     override, defragmentationEnable);
    if (GT_OK != rc)
    {
        return rc;
    }
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;


#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC mcRoutePointer;

    TGF_PARAM_NOT_USED(defragmentationEnable);
    if (ipGroupPrefixLen != 32)
    {
        return GT_BAD_PARAM;
    }
    rc = prvTgfConvertGenericToExMxPmRoutePointer(mcRouteLttEntryPtr,
                                                  &mcRoutePointer);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRoutePointer FAILED, rc = [%d]", rc);

        return rc;
    }
    /* call device specific API */
    rc =  cpssExMxPmIpLpmIpv4McEntryAdd(lpmDBId, vrId, ipGroup, ipSrc, ipSrcPrefixLen, &mcRoutePointer,
                                     override);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv4McEntryDel function
* @endinternal
*
* @brief   To delete a particular mc route entry for a specific LPM DB.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmIpv4McEntryDel
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPADDR                    ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPADDR                    ipSrc,
    IN GT_U32                       ipSrcPrefixLen
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                     &ipSrc, ipSrcPrefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(ipGroupPrefixLen);

    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv4McEntryDelete(lpmDBId, vrId, ipGroup,
                                            ipSrc, ipSrcPrefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixSearch function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the
*         information associated with it.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - if found, this is the route entry info.
*                                      accosiated with this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixSearch
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPADDR                            ipAddr,
    IN  GT_U32                               prefixLen,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                              rc = GT_OK;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChnextHopInfo;

    cpssOsMemSet(&dxChnextHopInfo,0,sizeof(dxChnextHopInfo));

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    cpssOsMemSet(&dxChnextHopInfo, 0, sizeof(dxChnextHopInfo));

    /* call device specific API */
    rc = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrId,
                                        &ipAddr, prefixLen,
                                        &dxChnextHopInfo,
                                        tcamRowIndexPtr, tcamColumnIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert Route entry */
    return prvTgfConvertDxChToGenericUcNextHopInfo(routingMode, &dxChnextHopInfo, nextHopInfoPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS                              rc = GT_OK;
    CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC exMxPmNextHopInfo;

    TGF_PARAM_NOT_USED(tcamRowIndexPtr);
    TGF_PARAM_NOT_USED(tcamColumnIndexPtr);

    cpssOsMemSet(&exMxPmNextHopInfo, 0, sizeof(exMxPmNextHopInfo));

    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv4UcPrefixGet(lpmDBId, vrId,
                                        ipAddr, prefixLen,
                                        &exMxPmNextHopInfo);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert Route entry */
    return prvTgfConvertExMxPmToGenericRoutePointer(&exMxPmNextHopInfo, &nextHopInfoPtr->ipLttEntry);

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixGet function
* @endinternal
*
* @brief   This function gets a given ip address, find LPM match in the trie and
*         returns the prefix length and pointer to the next hop information bound
*         to the longest prefix match.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - The prefix length of the found entry.
* @param[out] nextHopInfoPtr           - if found, this is the route entry info.
*                                      accosiated with this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this UC prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this UC prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixGet
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPADDR                            ipAddr,
    OUT GT_U32                               *prefixLenPtr,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                              rc = GT_OK;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChnextHopInfo;

    cpssOsMemSet(&dxChnextHopInfo,0,sizeof(dxChnextHopInfo));

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    cpssOsMemSet(&dxChnextHopInfo, 0, sizeof(dxChnextHopInfo));

    /* call device specific API */
    rc = cpssDxChIpLpmIpv4UcPrefixGet(lpmDBId, vrId,
                                      &ipAddr, prefixLenPtr,
                                      &dxChnextHopInfo,
                                      tcamRowIndexPtr, tcamColumnIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert Route entry */
    return prvTgfConvertDxChToGenericUcNextHopInfo(routingMode, &dxChnextHopInfo, nextHopInfoPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* avoid warnings */
    TGF_PARAM_NOT_USED(prefixLenPtr);
    TGF_PARAM_NOT_USED(nextHopInfoPtr);
    TGF_PARAM_NOT_USED(tcamRowIndexPtr);
    TGF_PARAM_NOT_USED(tcamColumnIndexPtr);

    return GT_OK;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv4McEntrySearch function
* @endinternal
*
* @brief   This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry, and ipSrc,ipGroup TCAM indexes
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - The ip Group prefix len.
* @param[in] ipSrc                    - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - the LTT entry pointer pointing to the MC route
*                                      entry associated with this MC route.
* @param[out] tcamGroupRowIndexPtr     - pointer to TCAM group row  index.
* @param[out] tcamGroupColumnIndexPtr  - pointer to TCAM group column  index.
* @param[out] tcamSrcRowIndexPtr       - pointer to TCAM source row  index.
* @param[out] tcamSrcColumnIndexPtr    - pointer to TCAM source column  index.
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
*/
GT_STATUS prvTgfIpLpmIpv4McEntrySearch
(
    IN  GT_U32                      lpmDBId,
    IN  GT_U32                      vrId,
    IN  GT_IPADDR                   ipGroup,
    IN  GT_U32                      ipGroupPrefixLen,
    IN  GT_IPADDR                   ipSrc,
    IN  GT_U32                      ipSrcPrefixLen,
    OUT PRV_TGF_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    OUT GT_U32                      *tcamGroupRowIndexPtr,
    OUT GT_U32                      *tcamGroupColumnIndexPtr,
    OUT GT_U32                      *tcamSrcRowIndexPtr,
    OUT GT_U32                      *tcamSrcColumnIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                   rc = GT_OK;
    CPSS_DXCH_IP_LTT_ENTRY_STC  dxChMcRouteLttEntry = {0};

    /* call device specific API */
    rc = cpssDxChIpLpmIpv4McEntrySearch(lpmDBId, vrId,
                                        &ipGroup, ipGroupPrefixLen,
                                        &ipSrc, ipSrcPrefixLen,
                                        &dxChMcRouteLttEntry,
                                        tcamGroupRowIndexPtr, tcamGroupColumnIndexPtr,
                                        tcamSrcRowIndexPtr, tcamSrcColumnIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert LTT entry */
    return prvTgfConvertDxChToGenericMcLttEntryInfo(&dxChMcRouteLttEntry, mcRouteLttEntryPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS                               rc = GT_OK;
    CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC  exMxPmMcRouteLttEntry = {0};

    TGF_PARAM_NOT_USED(ipGroupPrefixLen);
    TGF_PARAM_NOT_USED(tcamGroupRowIndexPtr);
    TGF_PARAM_NOT_USED(tcamGroupColumnIndexPtr);
    TGF_PARAM_NOT_USED(tcamSrcRowIndexPtr);
    TGF_PARAM_NOT_USED(tcamSrcColumnIndexPtr);

    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv4McEntryGet(lpmDBId, vrId,
                                        ipGroup, ipSrc, ipSrcPrefixLen,
                                        &exMxPmMcRouteLttEntry);

    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert LTT entry */
    return prvTgfConvertExMxPmToGenericRoutePointer(&exMxPmMcRouteLttEntry, mcRouteLttEntryPtr);

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixSearch function
* @endinternal
*
* @brief   This function searches for a given ip-uc address, and returns the next
*         hop pointer associated with it and TCAM prefix index.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - If  found, the route entry info accosiated with
*                                      this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixSearch
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPV6ADDR                          ipAddr,
    IN  GT_U32                               prefixLen,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                              rc = GT_OK;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChnextHopInfo;

    cpssOsMemSet(&dxChnextHopInfo,0,sizeof(dxChnextHopInfo));

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    cpssOsMemSet(&dxChnextHopInfo, 0, sizeof(dxChnextHopInfo));

    /* call device specific API */
    rc = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrId,
                                        &ipAddr, prefixLen,
                                        &dxChnextHopInfo,
                                        tcamRowIndexPtr, tcamColumnIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert Route entry */
    return prvTgfConvertDxChToGenericUcNextHopInfo(routingMode, &dxChnextHopInfo, nextHopInfoPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS                              rc = GT_OK;
    CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC exMxPmNextHopInfo;

    TGF_PARAM_NOT_USED(tcamRowIndexPtr);
    TGF_PARAM_NOT_USED(tcamColumnIndexPtr);

    cpssOsMemSet(&exMxPmNextHopInfo, 0, sizeof(exMxPmNextHopInfo));

    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv6UcPrefixGet(lpmDBId, vrId,
                                        &ipAddr, prefixLen,
                                        &exMxPmNextHopInfo);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert Route entry */
    return prvTgfConvertExMxPmToGenericRoutePointer(&exMxPmNextHopInfo, &nextHopInfoPtr->ipLttEntry);

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixGet function
* @endinternal
*
* @brief   This function gets a given ip address, find LPM match in the trie and
*         returns the prefix length and pointer to the next hop information bound
*         to the longest prefix match.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - The prefix length of the found entry.
* @param[out] nextHopInfoPtr           - If  found, the route entry info accosiated with
*                                      this UC prefix.
* @param[out] tcamRowIndexPtr          - if found, TCAM row index of this uc prefix.
* @param[out] tcamColumnIndexPtr       - if found, TCAM column index of this uc prefix.
*
* @retval GT_OK                    - if the required entry was found.
* @retval GT_OUT_OF_RANGE          - if prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixGet
(
    IN  GT_U32                               lpmDBId,
    IN  GT_U32                               vrId,
    IN  GT_IPV6ADDR                          ipAddr,
    OUT GT_U32                               *prefixLenPtr,
    OUT PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoPtr,
    OUT GT_U32                               *tcamRowIndexPtr,
    OUT GT_U32                               *tcamColumnIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                              rc = GT_OK;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT dxChnextHopInfo;

    cpssOsMemSet(&dxChnextHopInfo,0,sizeof(dxChnextHopInfo));

    /* get routing mode */
    rc =  prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]", rc);
        return rc;
    }

    cpssOsMemSet(&dxChnextHopInfo, 0, sizeof(dxChnextHopInfo));

    /* call device specific API */
    rc = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrId,
                                      &ipAddr, prefixLenPtr,
                                      &dxChnextHopInfo,
                                      tcamRowIndexPtr, tcamColumnIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert Route entry */
    return prvTgfConvertDxChToGenericUcNextHopInfo(routingMode, &dxChnextHopInfo, nextHopInfoPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    /* avoid warnings */
    TGF_PARAM_NOT_USED(prefixLenPtr);
    TGF_PARAM_NOT_USED(nextHopInfoPtr);
    TGF_PARAM_NOT_USED(tcamRowIndexPtr);
    TGF_PARAM_NOT_USED(tcamColumnIndexPtr);

    return GT_OK;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv6McEntrySearch function
* @endinternal
*
* @brief   This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry, and ipSrc,ipGroup TCAM indexes
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrc                    - The ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteLttEntryPtr       - the LTT entry pointer pointing to the MC route
*                                      entry associated with this MC route.
* @param[out] tcamGroupRowIndexPtr     - pointer to TCAM group row  index.
* @param[out] tcamSrcRowIndexPtr       - pointer to TCAM source row  index.
*
* @retval GT_OK                    - if found.
* @retval GT_OUT_OF_RANGE          - if one of prefix length is too big.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table.
*/
GT_STATUS prvTgfIpLpmIpv6McEntrySearch
(
    IN  GT_U32                   lpmDBId,
    IN  GT_U32                   vrId,
    IN  GT_IPV6ADDR              ipGroup,
    IN  GT_U32                   ipGroupPrefixLen,
    IN  GT_IPV6ADDR              ipSrc,
    IN  GT_U32                   ipSrcPrefixLen,
    OUT PRV_TGF_IP_LTT_ENTRY_STC *mcRouteLttEntryPtr,
    OUT GT_U32                   *tcamGroupRowIndexPtr,
    OUT GT_U32                   *tcamSrcRowIndexPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                   rc = GT_OK;
    CPSS_DXCH_IP_LTT_ENTRY_STC  dxChMcRouteLttEntry = {0};

    /* call device specific API */
    rc = cpssDxChIpLpmIpv6McEntrySearch(lpmDBId, vrId,
                                        &ipGroup, ipGroupPrefixLen,
                                        &ipSrc, ipSrcPrefixLen,
                                        &dxChMcRouteLttEntry,
                                        tcamGroupRowIndexPtr,
                                        tcamSrcRowIndexPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert LTT entry */
    return prvTgfConvertDxChToGenericMcLttEntryInfo(&dxChMcRouteLttEntry, mcRouteLttEntryPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_STATUS                   rc = GT_OK;
    CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC exMxPmMcRouteLttEntry = {0};

    TGF_PARAM_NOT_USED(tcamSrcRowIndexPtr);

    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv6McEntryGet(lpmDBId, vrId,
                                        ipGroup,
                                        ipSrc, ipSrcPrefixLen,
                                        tcamGroupRowIndexPtr,
                                        &exMxPmMcRouteLttEntry);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* convert LTT entry */
    return prvTgfConvertExMxPmToGenericRoutePointer(&exMxPmMcRouteLttEntry, mcRouteLttEntryPtr);

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv6McEntryAdd function
* @endinternal
*
* @brief   To add the multicast routing information for IP datagrams from a particular
*         source and addressed to a particular IP multicast group address for a
*         specific LPM DB.
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteLttEntryPtr       - the LTT entry pointing to the MC route entry
*                                      associated with this MC route.
* @param[in] override                 - weather to  the mcRoutePointerPtr for the
*                                      given prefix
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big.
* @retval GT_ERROR                 - if the virtual router does not exist.
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory.
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_NOT_IMPLEMENTED       - if this request is not implemented .
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
*
* @note to override the default mc route use ipGroup = ipGroupPrefixLen = 0.
*
*/
GT_STATUS prvTgfIpLpmIpv6McEntryAdd
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPV6ADDR                  ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                  ipSrc,
    IN GT_U32                       ipSrcPrefixLen,
    IN PRV_TGF_IP_LTT_ENTRY_STC    *mcRouteLttEntryPtr,
    IN GT_BOOL                      override,
    IN GT_BOOL                      defragmentationEnable
)
{
    GT_STATUS                   rc = GT_OK;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LTT_ENTRY_STC  dxChMcRouteLttEntry;

    cpssOsMemSet(&dxChMcRouteLttEntry,0,sizeof(dxChMcRouteLttEntry));

    /* set LTT entry */
    dxChMcRouteLttEntry.numOfPaths = mcRouteLttEntryPtr->numOfPaths;
    dxChMcRouteLttEntry.routeEntryBaseIndex = mcRouteLttEntryPtr->routeEntryBaseIndex;
    dxChMcRouteLttEntry.ucRPFCheckEnable = mcRouteLttEntryPtr->ucRPFCheckEnable;
    dxChMcRouteLttEntry.sipSaCheckMismatchEnable = mcRouteLttEntryPtr->sipSaCheckMismatchEnable;
    dxChMcRouteLttEntry.ipv6MCGroupScopeLevel = mcRouteLttEntryPtr->ipv6MCGroupScopeLevel;

    /* set route type */
    rc = prvTgfConvertGenericToDxChRouteType(mcRouteLttEntryPtr->routeType,
                                             &(dxChMcRouteLttEntry.routeType));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);

        return rc;
    }
     /* call device specific API */
    rc =  cpssDxChIpLpmIpv6McEntryAdd(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                     &ipSrc, ipSrcPrefixLen, &dxChMcRouteLttEntry,
                                     override, defragmentationEnable);
    if (GT_OK != rc)
    {
        return rc;
    }
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_MULTICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC mcRoutePointer;

    TGF_PARAM_NOT_USED(defragmentationEnable);
    if (ipGroupPrefixLen != 128)
    {
        return GT_BAD_PARAM;
    }
    rc = prvTgfConvertGenericToExMxPmRoutePointer(mcRouteLttEntryPtr,
                                                  &mcRoutePointer);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRoutePointer FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc =  cpssExMxPmIpLpmIpv6McEntryAdd(lpmDBId, vrId, ipGroup,
                                        prvTgfIpv6McGroupRuleIndex,ipSrc, ipSrcPrefixLen,
                                        &mcRoutePointer,override);
    if (GT_OK != rc)
    {
        return rc;
    }
    /* IPV6 MC validation check is not implemented yet */
    /* rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_MULTICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    */
    return rc;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv4McEntriesFlush function
* @endinternal
*
* @brief   Flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB
* @param[in] lpmDBId                  - LPM DB id
* @param[in] vrId                     - virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv4McEntriesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4McEntriesFlush FAILED, rc = [%d]", rc);
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv4McEntriesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpLpmIpv4McEntriesFlush FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV4_E,CPSS_MULTICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}



/**
* @internal prvTgfIpLpmIpv6McEntryDel function
* @endinternal
*
* @brief   To delete a particular mc route entry for a specific LPM DB.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If one of prefixes' lengths is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvTgfIpLpmIpv6McEntryDel
(
    IN GT_U32                       lpmDBId,
    IN GT_U32                       vrId,
    IN GT_IPV6ADDR                  ipGroup,
    IN GT_U32                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                  ipSrc,
    IN GT_U32                       ipSrcPrefixLen
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv6McEntryDel(lpmDBId, vrId, &ipGroup, ipGroupPrefixLen,
                                     &ipSrc, ipSrcPrefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_MULTICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(ipGroupPrefixLen);

    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv6McEntryDel(lpmDBId, vrId, ipGroup,
                                       ipSrc, ipSrcPrefixLen);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_MULTICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}





/**
* @internal prvTgfIpLpmIpv6McEntriesFlush function
* @endinternal
*
* @brief   Flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB
* @param[in] lpmDBId                  - LPM DB id
* @param[in] vrId                     - virtual router identifier
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmIpv6McEntriesFlush
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CHX_FAMILY
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv6McEntriesFlush FAILED, rc = [%d]", rc);
        return rc;
    }
    rc = prvTgfIpValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_MULTICAST_E);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
    }
    return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv6McEntriesFlush(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpLpmIpv6McEntriesFlush FAILED, rc = [%d]", rc);

        return rc;
    }
    /* IPV6 MC validation check is not implemented yet */
    /*
    rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId,CPSS_IP_PROTOCOL_IPV6_E,CPSS_MULTICAST_E,GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
    }
    */
    return rc;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpSetMllCntInterface function
* @endinternal
*
* @brief   Sets a mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpSetMllCntInterface
(
    IN GT_U32                                    mllCntSet,
    IN PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC *interfaceCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC  dxChInterfaceCfg;

    /* reset variable */
    cpssOsMemSet(&dxChInterfaceCfg, 0, sizeof(dxChInterfaceCfg));

    /* convert Port/Trunk mode into device specific format */
    switch (interfaceCfgPtr->portTrunkCntMode)
    {
        case PRV_TGF_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E:
            dxChInterfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
            break;

        case PRV_TGF_IP_PORT_CNT_MODE_E:
            dxChInterfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_PORT_CNT_MODE_E;
            dxChInterfaceCfg.portTrunk.port   = interfaceCfgPtr->portTrunk.port;
            break;

        case PRV_TGF_IP_TRUNK_CNT_MODE_E:
            dxChInterfaceCfg.portTrunkCntMode = CPSS_DXCH_IP_TRUNK_CNT_MODE_E;
            dxChInterfaceCfg.portTrunk.trunk  = interfaceCfgPtr->portTrunk.trunk;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert the counter Set vlan mode into device specific format */
    switch (interfaceCfgPtr->vlanMode)
    {
        case PRV_TGF_IP_DISREGARD_VLAN_CNT_MODE_E:
            dxChInterfaceCfg.vlanMode = CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E;
            break;

        case PRV_TGF_IP_USE_VLAN_CNT_MODE_E:
            dxChInterfaceCfg.vlanMode = CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E;
            dxChInterfaceCfg.vlanId   = interfaceCfgPtr->vlanId;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert other fields */
    dxChInterfaceCfg.ipMode = interfaceCfgPtr->ipMode;
    dxChInterfaceCfg.hwDevNum = interfaceCfgPtr->devNum;
    rc = prvUtfHwDeviceNumberGet(interfaceCfgPtr->devNum,&(dxChInterfaceCfg.hwDevNum));
    if (rc != GT_OK)
    {
        return rc;
    }


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpSetMllCntInterface(devNum, mllCntSet, &dxChInterfaceCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpSetMllCntInterface FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;
    CPSS_EXMXPM_DIT_COUNTER_SET_CFG_STC     exMxPmCounterCfg;

    /* reset variable */
    cpssOsMemSet(&exMxPmCounterCfg, 0, sizeof(CPSS_EXMXPM_DIT_COUNTER_SET_CFG_STC));

    /* convert Port/Trunk mode into device specific format */
    switch (interfaceCfgPtr->portTrunkCntMode)
    {
        case PRV_TGF_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E:
            exMxPmCounterCfg.portTrunkMode = CPSS_EXMXPM_DIT_DISREGARD_PORT_TRUNK_COUNTER_MODE_E;
            break;

        case PRV_TGF_IP_PORT_CNT_MODE_E:
            exMxPmCounterCfg.portTrunkMode = CPSS_EXMXPM_DIT_PORT_COUNTER_MODE_E;
            exMxPmCounterCfg.portTrunk.devPort.port = interfaceCfgPtr->portTrunk.port;
            break;

        case PRV_TGF_IP_TRUNK_CNT_MODE_E:
            exMxPmCounterCfg.portTrunkMode = CPSS_EXMXPM_DIT_TRUNK_COUNTER_MODE_E;
            exMxPmCounterCfg.portTrunk.trunk  = interfaceCfgPtr->portTrunk.trunk;
            break;

        default:
            return GT_BAD_PARAM;
    }
    /* convert devNum */
    exMxPmCounterCfg.portTrunk.devPort.devNum = interfaceCfgPtr->devNum;

    /* convert the counter Set vlan mode into device specific format */
    switch (interfaceCfgPtr->vlanMode)
    {
        case PRV_TGF_IP_DISREGARD_VLAN_CNT_MODE_E:
            exMxPmCounterCfg.vlanMode = CPSS_EXMXPM_DIT_DISREGARD_VLAN_COUNTER_MODE_E;
            break;

        case PRV_TGF_IP_USE_VLAN_CNT_MODE_E:
            exMxPmCounterCfg.vlanMode = CPSS_EXMXPM_DIT_USE_VLAN_COUNTER_MODE_E;
            exMxPmCounterCfg.vlanId   = interfaceCfgPtr->vlanId;
            break;

        default:
            return GT_BAD_PARAM;
    }
    /* convert interfaceCfgPtr->ipMode */
    switch (interfaceCfgPtr->ipMode)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            exMxPmCounterCfg.packetType = CPSS_EXMXPM_DIT_PACKET_TYPE_IPV4_COUNTER_MODE_E;
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            exMxPmCounterCfg.packetType = CPSS_EXMXPM_DIT_PACKET_TYPE_IPV6_COUNTER_MODE_E;
            break;
        case CPSS_IP_PROTOCOL_IPV4V6_E:
            exMxPmCounterCfg.packetType = CPSS_EXMXPM_DIT_PACKET_TYPE_ALL_COUNTER_MODE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssExMxPmDitCounterConfigSet(devNum, mllCntSet,CPSS_MULTICAST_E, &exMxPmCounterCfg);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpSetMllCntInterface FAILED, rc = [%d]", rc);

            rc1 = rc;
        }
    }

    return rc1;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpMllCntGet function
* @endinternal
*
* @brief   Get the mll counter.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
* @param[in] devNum                   - the device number.
* @param[in] mllCntSet                - the mll counter set out of the 2
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of routed IP Multicast packets Duplicated by the
*                                      MLL Engine and transmitted via this interface
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllCntGet(devNum, mllCntSet, mllOutMCPktsPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
   /* call device specific API */
    return cpssExMxPmDitCounterGet(devNum,mllCntSet,CPSS_MULTICAST_E,mllOutMCPktsPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpMllCntSet function
* @endinternal
*
* @brief   set an mll counter.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - the mll counter set out of the 2
* @param[in] mllOutMCPkts             - the counter value to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllCntSet
(
    IN GT_U8    devNum,
    IN GT_U32   mllCntSet,
    IN GT_U32   mllOutMCPkts
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllCntSet(devNum, mllCntSet, mllOutMCPkts);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mllCntSet);
    TGF_PARAM_NOT_USED(mllOutMCPkts);

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpMllSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the MLL priority queues.
*         A silent drop is a drop that is applied to a replica of the packet that
*         was previously replicated in the TTI.
* @param[in] devNum                   - the device number
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllSilentDropCntGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *silentDropPktsPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllSilentDropCntGet(devNum, silentDropPktsPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(silentDropPktsPtr);
    return GT_BAD_STATE;
#endif /* ! CHX_FAMILY */
}

/**
* @internal prvTgfIpMtuProfileSet function
* @endinternal
*
* @brief   Sets the next hop interface MTU profile limit value.
*
* @param[in] devNum                   - the device number
* @param[in] mtu                      - the mtu profile index.  CHX_FAMILY (0..7)
*                                      EXMXPM_FAMILY (0..15)
* @param[in] mtu                      - the maximum transmission unit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMtuProfileSet
(
    IN GT_U8  devNum,
    IN GT_U32 mtuProfileIndex,
    IN GT_U32 mtu
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMtuProfileSet(devNum, mtuProfileIndex, mtu);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return cpssExMxPmIpMtuProfileSet(devNum, mtuProfileIndex, mtu);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpMtuProfileGet function
* @endinternal
*
* @brief   Gets the next hop interface MTU profile limit value.
*
* @param[in] devNum                   - device number
* @param[in] mtuProfileIndex          - the mtu profile index.  CHX_FAMILY (0..7)
*                                      EXMXPM_FAMILY (0..15)
*
* @param[out] mtuPtr                   - points to the maximum transmission unit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMtuProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mtuProfileIndex,
    OUT GT_U32  *mtuPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMtuProfileGet(devNum, mtuProfileIndex, mtuPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return cpssExMxPmIpMtuProfileGet(devNum, mtuProfileIndex, mtuPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpv6AddrPrefixScopeSet function
* @endinternal
*
* @brief   Defines a prefix of a scope type.
*
* @param[in] devNum                   - the device number
* @param[in] prefix                   - an IPv6 address prefix
* @param[in] prefixLen                - length of the prefix (APPLICABLE RANGES: 0..16)
* @param[in] addressScope             - type of the address scope spanned by the prefix
* @param[in] prefixScopeIndex         - index of the new prefix scope entry (APPLICABLE RANGES: 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - case that the prefix length is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Configures an entry in the prefix look up table
*
*/
GT_STATUS prvTgfIpv6AddrPrefixScopeSet
(
    IN  GT_U8                           devNum,
    IN  GT_IPV6ADDR                     prefix,
    IN  GT_U32                          prefixLen,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT      addressScope,
    IN  GT_U32                          prefixScopeIndex
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpv6AddrPrefixScopeSet(devNum, &prefix, prefixLen, addressScope, prefixScopeIndex);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(prefix);
    TGF_PARAM_NOT_USED(prefixLen);
    TGF_PARAM_NOT_USED(addressScope);
    TGF_PARAM_NOT_USED(prefixScopeIndex);

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}

/**
* @internal prvTgfIpv6AddrPrefixScopeGet function
* @endinternal
*
* @brief   Get a prefix of a scope type.
*
* @param[in] devNum                   - the device number
* @param[in] prefixScopeIndex         - index of the new prefix scope entry (APPLICABLE RANGES: 0..3)
*
* @param[out] prefixPtr                - an IPv6 address prefix
* @param[out] prefixLenPtr             - length of the prefix
* @param[out] addressScopePtr          - type of the address scope spanned by the prefix
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - case that the prefix length is out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvTgfIpv6AddrPrefixScopeGet
(
    IN  GT_U8                           devNum,
    OUT GT_IPV6ADDR                     *prefixPtr,
    OUT GT_U32                          *prefixLenPtr,
    OUT CPSS_IPV6_PREFIX_SCOPE_ENT      *addressScopePtr,
    IN  GT_U32                          prefixScopeIndex
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpv6AddrPrefixScopeGet(devNum, prefixPtr, prefixLenPtr, addressScopePtr, prefixScopeIndex);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(prefixPtr);
    TGF_PARAM_NOT_USED(prefixLenPtr);
    TGF_PARAM_NOT_USED(addressScopePtr);
    TGF_PARAM_NOT_USED(prefixScopeIndex);

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}

/**
* @internal prvTgfIpExceptionCommandSet function
* @endinternal
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - Exception type
* @param[in] protocolStack            - whether to set for ipv4/v6 or both
* @param[in] packetType               - Packet type. Valid values:
*                                      CPSS_IP_UNICAST_E
*                                      CPSS_IP_MULTICAST_E
* @param[in] exceptionCmd             - Exception command
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpExceptionCommandSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_IP_EXCEPTION_TYPE_ENT        exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_IP_UNICAST_MULTICAST_ENT        packetType,
    IN PRV_TGF_IP_EXC_CMD_ENT               exceptionCmd
)
{

#ifdef CHX_FAMILY

    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT         exceptionTypeDx;
    CPSS_PACKET_CMD_ENT                     exceptionCmdDx;

    /* convert exceptionType into device specific format */
    switch(packetType)
    {
        case CPSS_IP_UNICAST_E:
        {
            switch (exceptionType)
            {
            case PRV_TGF_IP_EXCP_HDR_ERROR_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E;
                break;
            case PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E;
                break;
            case PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_DIP_DA_MISMATCH_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_E;
                break;
            case PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E:
                exceptionTypeDx=  CPSS_DXCH_IP_EXCP_UC_ALL_ZERO_SIP_E;
                break;
            case PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E;
                break;
            case PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E;
                break;
            case PRV_TGF_IP_EXCP_TTL_EXCEED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_TTL_EXCEED_E;
                break;
            case PRV_TGF_IP_EXCP_RPF_FAIL_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_RPF_FAIL_E;
                break;
            case PRV_TGF_IP_EXCP_SIP_SA_FAIL_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_SIP_SA_FAIL_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_NON_DF_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_DF_E;
                break;
            default:
                return GT_BAD_PARAM;
            }
             break;
        }
        case CPSS_IP_MULTICAST_E:
        {
            switch (exceptionType)
            {
            case PRV_TGF_IP_EXCP_HDR_ERROR_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E;
                break;
            case PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_ILLEGAL_ADDRESS_E;
                break;
            case PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_DIP_DA_MISMATCH_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_E;
                break;
            case PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E:
                exceptionTypeDx=  CPSS_DXCH_IP_EXCP_MC_ALL_ZERO_SIP_E;
                break;
            case PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_OPTION_HOP_BY_HOP_E;
                break;
            case PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_NON_HOP_BY_HOP_EXT_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_NON_DF_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_DF_E;
                break;
            default:
                return GT_BAD_PARAM;
            }
            break;
        }
        default:
            return GT_BAD_PARAM;
    }

    /* convert exceptionCmd into device specific format */
    switch(exceptionCmd)
    {
        case PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E:
            exceptionCmdDx= CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            break;
        case PRV_TGF_IP_EXC_CMD_DROP_HARD_E:
            exceptionCmdDx= CPSS_PACKET_CMD_DROP_HARD_E;
            break;
        case PRV_TGF_IP_EXC_CMD_DROP_SOFT_E:
            exceptionCmdDx= CPSS_PACKET_CMD_DROP_SOFT_E;
            break;
        case PRV_TGF_IP_EXC_CMD_ROUTE_E:
            exceptionCmdDx= CPSS_PACKET_CMD_ROUTE_E;
            break;
        case PRV_TGF_IP_EXC_CMD_ROUTE_AND_MIRROR_E:
            exceptionCmdDx= CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
            break;
        case PRV_TGF_IP_EXC_CMD_BRIDGE_AND_MIRROR_E:
            exceptionCmdDx= CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
            break;
        case PRV_TGF_IP_EXC_CMD_BRIDGE_E:
            exceptionCmdDx= CPSS_PACKET_CMD_BRIDGE_E;
            break;
        case PRV_TGF_IP_EXC_CMD_NONE_E:
            exceptionCmdDx= CPSS_PACKET_CMD_NONE_E;
            break;
        case PRV_TGF_IP_EXC_CMD_MIRROR_TO_CPU_E:
            exceptionCmdDx= CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
            break;
        case PRV_TGF_IP_EXC_CMD_FORWARD_E:
            exceptionCmdDx= CPSS_PACKET_CMD_FORWARD_E;
            break;
        case PRV_TGF_IP_EXC_CMD_LOOPBACK_E:
            exceptionCmdDx= CPSS_PACKET_CMD_LOOPBACK_E;
            break;
        case PRV_TGF_IP_EXC_CMD_DEFAULT_ROUTE_ENTRY_E:
            exceptionCmdDx= CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChIpExceptionCommandSet(devNum, exceptionTypeDx, protocolStack,exceptionCmdDx);
#endif /* CHX_FAMILY */


#ifdef EXMXPM_FAMILY

    CPSS_EXMXPM_IP_EXCEPTION_TYPE_ENT       exceptionTypeEx;
    CPSS_EXMXPM_IP_EXCEPTION_COMMAND_ENT    exceptionCmdEx;

    /* convert exceptionType into device specific format */
    switch (exceptionType)
    {
        case PRV_TGF_IP_EXCP_HDR_ERROR_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_HEADER_ERROR_E;
            break;
        case PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_ILLEGAL_ADDRESS_E;
            break;
        case PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_DIP_DA_MISMATCH_E;
            break;
        case PRV_TGF_IP_EXCP_MTU_EXCEEDED_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_MTU_EXCEEDED_E;
            break;
        case PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E:
            exceptionTypeEx=  CPSS_EXMXPM_IP_EXCEPTION_TYPE_ALL_ZERO_SIP_E;
            break;
        case PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_OPTION_HOP_BY_HOP_E;
            break;
        case PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_NON_HOP_BY_HOP_EXT_E;
            break;
        case PRV_TGF_IP_EXCP_SIP_SA_FAIL_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_SIP_SA_MISMATCH_E;
            break;
        case PRV_TGF_IP_EXCP_TTL0_EXCEED_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_TTL0_EXCEED_E;
            break;
        case PRV_TGF_IP_EXCP_TTL1_EXCEED_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_TTL1_EXCEED_E;
            break;
        case PRV_TGF_IP_EXCP_URPF_FAIL_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_URPF_FAIL_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* convert exceptionCmd into device specific format */
    switch(exceptionCmd)
    {
        case PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E:
            exceptionCmdEx= CPSS_EXMXPM_IP_EXCEPTION_COMMAND_TRAP_TO_CPU_E;
            break;
        case PRV_TGF_IP_EXC_CMD_DROP_HARD_E:
            exceptionCmdEx= CPSS_EXMXPM_IP_EXCEPTION_COMMAND_DROP_HARD_E;
            break;
        case PRV_TGF_IP_EXC_CMD_DROP_SOFT_E:
            exceptionCmdEx= CPSS_EXMXPM_IP_EXCEPTION_COMMAND_DROP_SOFT_E;
            break;
        case PRV_TGF_IP_EXC_CMD_ROUTE_E:
            exceptionCmdEx= CPSS_EXMXPM_IP_EXCEPTION_COMMAND_ROUTE_E;
            break;
        case PRV_TGF_IP_EXC_CMD_ROUTE_AND_MIRROR_E:
            exceptionCmdEx= CPSS_EXMXPM_IP_EXCEPTION_COMMAND_ROUTE_AND_MIRROR_E;
            break;
        case PRV_TGF_IP_EXC_CMD_BRIDGE_AND_MIRROR_E:
            exceptionCmdEx= CPSS_EXMXPM_IP_EXCEPTION_COMMAND_BRIDGE_AND_MIRROR_E;
            break;
        case PRV_TGF_IP_EXC_CMD_BRIDGE_E:
            exceptionCmdEx= CPSS_EXMXPM_IP_EXCEPTION_COMMAND_BRIDGE_E;
            break;
        case PRV_TGF_IP_EXC_CMD_NONE_E:
            exceptionCmdEx= CPSS_EXMXPM_IP_EXCEPTION_COMMAND_NONE;
            break;
        case PRV_TGF_IP_EXC_CMD_ROUTE_ALL_TRAP_DF_E:
            exceptionCmdEx= CPSS_EXMXPM_IP_EXCEPTION_COMMAND_ROUTE_ALL_TRAP_DF_E;
            break;
        case PRV_TGF_IP_EXC_CMD_DONT_ROUTE_DF_E:
            exceptionCmdEx= CPSS_EXMXPM_IP_EXCEPTION_COMMAND_DONT_ROUTE_DF_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssExMxPmIpExceptionCmdSet(devNum, exceptionTypeEx, protocolStack,packetType,exceptionCmdEx);

#endif /* EXMXPM_FAMILY */


#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpExceptionCommandGet function
* @endinternal
*
* @brief   Get a specific exception command.
*
* @param[in] devNum                   - device number
* @param[in] exceptionType            - Exception type
* @param[in] protocolStack            - IP protocol to set for
* @param[in] packetType               - Packet type. Valid values:
*                                      CPSS_IP_UNICAST_E
*                                      CPSS_IP_MULTICAST_E
*
* @param[out] exceptionCmdPtr          - points to exception command
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpExceptionCommandGet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_IP_EXCEPTION_TYPE_ENT        exceptionType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_IP_UNICAST_MULTICAST_ENT        packetType,
    OUT PRV_TGF_IP_EXC_CMD_ENT              *exceptionCmdPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT         exceptionTypeDx;
    CPSS_PACKET_CMD_ENT                     exceptionCmdDx;
    GT_STATUS                               rc;

    /* convert exceptionType into device specific format */
    switch(packetType)
    {
        case CPSS_IP_UNICAST_E:
        {
            switch (exceptionType)
            {
            case PRV_TGF_IP_EXCP_HDR_ERROR_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E;
                break;
            case PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E;
                break;
            case PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_DIP_DA_MISMATCH_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_E;
                break;
            case PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E:
                exceptionTypeDx=  CPSS_DXCH_IP_EXCP_UC_ALL_ZERO_SIP_E;
                break;
            case PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E;
                break;
            case PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E;
                break;
            case PRV_TGF_IP_EXCP_TTL_EXCEED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_TTL_EXCEED_E;
                break;
            case PRV_TGF_IP_EXCP_RPF_FAIL_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_RPF_FAIL_E;
                break;
            case PRV_TGF_IP_EXCP_SIP_SA_FAIL_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_SIP_SA_FAIL_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_NON_DF_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_DF_E;
                break;
            default:
                return GT_BAD_PARAM;
            }
             break;
        }
        case CPSS_IP_MULTICAST_E:
        {
            switch (exceptionType)
            {
            case PRV_TGF_IP_EXCP_HDR_ERROR_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E;
                break;
            case PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_ILLEGAL_ADDRESS_E;
                break;
            case PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_DIP_DA_MISMATCH_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_E;
                break;
            case PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E:
                exceptionTypeDx=  CPSS_DXCH_IP_EXCP_MC_ALL_ZERO_SIP_E;
                break;
            case PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_OPTION_HOP_BY_HOP_E;
                break;
            case PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_NON_HOP_BY_HOP_EXT_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_NON_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_NON_DF_E;
                break;
            case PRV_TGF_IP_EXCP_MTU_EXCEEDED_FOR_DF_E:
                exceptionTypeDx= CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_DF_E;
                break;
            default:
                return GT_BAD_PARAM;
            }
             break;
        }
            default:
                return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChIpExceptionCommandGet(devNum, exceptionTypeDx, protocolStack, &exceptionCmdDx);
    switch(exceptionCmdDx)
    {
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_DROP_HARD_E;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_DROP_SOFT_E;
            break;
        case CPSS_PACKET_CMD_ROUTE_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_ROUTE_E;
            break;
        case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_ROUTE_AND_MIRROR_E;
            break;
        case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_BRIDGE_AND_MIRROR_E;
            break;
        case CPSS_PACKET_CMD_BRIDGE_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_BRIDGE_E;
            break;
        case CPSS_PACKET_CMD_NONE_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_NONE_E;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_MIRROR_TO_CPU_E;
            break;
        case CPSS_PACKET_CMD_FORWARD_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_FORWARD_E;
            break;
        case CPSS_PACKET_CMD_LOOPBACK_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_LOOPBACK_E;
            break;
        case CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_DEFAULT_ROUTE_ENTRY_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY

    CPSS_EXMXPM_IP_EXCEPTION_TYPE_ENT       exceptionTypeEx;
    CPSS_EXMXPM_IP_EXCEPTION_COMMAND_ENT    exceptionCmdEx;
    GT_STATUS                               rc;

    /* convert exceptionType into device specific format */
    switch (exceptionType)
    {
        case PRV_TGF_IP_EXCP_HDR_ERROR_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_HEADER_ERROR_E;
            break;
        case PRV_TGF_IP_EXCP_ILLEGAL_ADDRESS_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_ILLEGAL_ADDRESS_E;
            break;
        case PRV_TGF_IP_EXCP_DIP_DA_MISMATCH_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_DIP_DA_MISMATCH_E;
            break;
        case PRV_TGF_IP_EXCP_MTU_EXCEEDED_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_MTU_EXCEEDED_E;
            break;
        case PRV_TGF_IP_EXCP_ALL_ZERO_SIP_E:
            exceptionTypeEx=  CPSS_EXMXPM_IP_EXCEPTION_TYPE_ALL_ZERO_SIP_E;
            break;
        case PRV_TGF_IP_EXCP_OPTION_HOP_BY_HOP_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_OPTION_HOP_BY_HOP_E;
            break;
        case PRV_TGF_IP_EXCP_NON_HOP_BY_HOP_EXT_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_NON_HOP_BY_HOP_EXT_E;
            break;
        case PRV_TGF_IP_EXCP_SIP_SA_FAIL_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_SIP_SA_MISMATCH_E;
            break;
        case PRV_TGF_IP_EXCP_TTL0_EXCEED_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_TTL0_EXCEED_E;
            break;
        case PRV_TGF_IP_EXCP_TTL1_EXCEED_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_TTL1_EXCEED_E;
            break;
        case PRV_TGF_IP_EXCP_URPF_FAIL_E:
            exceptionTypeEx= CPSS_EXMXPM_IP_EXCEPTION_TYPE_URPF_FAIL_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssExMxPmIpExceptionCmdGet(devNum, exceptionTypeEx, protocolStack,packetType,&exceptionCmdEx);
    switch(exceptionCmdEx)
    {
        case CPSS_EXMXPM_IP_EXCEPTION_COMMAND_TRAP_TO_CPU_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_TRAP_TO_CPU_E;
            break;
        case CPSS_EXMXPM_IP_EXCEPTION_COMMAND_DROP_HARD_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_DROP_HARD_E;
            break;
        case CPSS_EXMXPM_IP_EXCEPTION_COMMAND_DROP_SOFT_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_DROP_SOFT_E;
            break;
        case CPSS_EXMXPM_IP_EXCEPTION_COMMAND_ROUTE_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_ROUTE_E;
            break;
        case CPSS_EXMXPM_IP_EXCEPTION_COMMAND_ROUTE_AND_MIRROR_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_ROUTE_AND_MIRROR_E;
            break;
        case CPSS_EXMXPM_IP_EXCEPTION_COMMAND_BRIDGE_AND_MIRROR_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_BRIDGE_AND_MIRROR_E;
            break;
        case CPSS_EXMXPM_IP_EXCEPTION_COMMAND_BRIDGE_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_BRIDGE_E;
            break;
        case CPSS_EXMXPM_IP_EXCEPTION_COMMAND_NONE:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_NONE_E;
            break;
        case CPSS_EXMXPM_IP_EXCEPTION_COMMAND_ROUTE_ALL_TRAP_DF_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_ROUTE_ALL_TRAP_DF_E;
            break;
        case CPSS_EXMXPM_IP_EXCEPTION_COMMAND_DONT_ROUTE_DF_E:
            *exceptionCmdPtr= PRV_TGF_IP_EXC_CMD_DONT_ROUTE_DF_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    return rc;
#endif /* EXMXPM_FAMILY */


#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpv6McScopeCommandSet function
* @endinternal
*
* @brief   sets the ipv6 Multicast scope commands.
*
* @param[in] devNum                   - the device number
* @param[in] addressScopeSrc          - IPv6 address scope of source address
* @param[in] addressScopeDest         - IPv6 address scope of destination address
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
* @param[in] scopeCommand             - action to be done on a packet that match the above
*                                      scope configuration.
*                                      possible commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*                                      CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_BRIDGE_E
* @param[in] mllSelectionRule         - rule for choosing MLL for IPv6 Multicast propogation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpv6McScopeCommandSet
(
    IN GT_U8                            devNum,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN GT_BOOL                          borderCrossed,
    IN CPSS_PACKET_CMD_ENT              scopeCommand,
    IN CPSS_IPV6_MLL_SELECTION_RULE_ENT mllSelectionRule
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpv6McScopeCommandSet(devNum, addressScopeSrc, addressScopeDest, borderCrossed, scopeCommand, mllSelectionRule);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(addressScopeSrc);
    TGF_PARAM_NOT_USED(addressScopeDest);
    TGF_PARAM_NOT_USED(borderCrossed);
    TGF_PARAM_NOT_USED(scopeCommand);
    TGF_PARAM_NOT_USED(mllSelectionRule);

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpv6McScopeCommandGet function
* @endinternal
*
* @brief   gets the ipv6 Multicast scope commands.
*
* @param[in] devNum                   - the device number
* @param[in] addressScopeSrc          - IPv6 address scope of source address
* @param[in] addressScopeDest         - IPv6 address scope of destination address
* @param[in] borderCrossed            - GT_TRUE if source site ID is diffrent to destination
*                                      site ID
*
* @param[out] scopeCommandPtr          - action to be done on a packet that match the above
*                                      scope configuration.
*                                      possible commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*                                      CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_BRIDGE_E
* @param[out] mllSelectionRulePtr      - rule for choosing MLL for IPv6 Multicast propogation
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error.
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_NO_RESOURCE           - if failed to allocate CPU memory
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvTgfIpv6McScopeCommandGet
(
    IN  GT_U8                            devNum,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeSrc,
    IN  CPSS_IPV6_PREFIX_SCOPE_ENT       addressScopeDest,
    IN  GT_BOOL                          borderCrossed,
    OUT CPSS_PACKET_CMD_ENT              *scopeCommandPtr,
    OUT CPSS_IPV6_MLL_SELECTION_RULE_ENT *mllSelectionRulePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpv6McScopeCommandGet(devNum, addressScopeSrc, addressScopeDest, borderCrossed, scopeCommandPtr, mllSelectionRulePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(addressScopeSrc);
    TGF_PARAM_NOT_USED(addressScopeDest);
    TGF_PARAM_NOT_USED(borderCrossed);
    TGF_PARAM_NOT_USED(scopeCommandPtr);
    TGF_PARAM_NOT_USED(mllSelectionRulePtr);

    return GT_OK;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpRouterMacSaBaseSet function
* @endinternal
*
* @brief   Sets 40 MSBs of Router MAC SA Base address on specified device.
*
* @param[in] devNum                   - the device number
* @param[in] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaBaseSet
(
    IN  GT_U8           devNum,
    IN  GT_ETHERADDR    *macPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterMacSaBaseSet(devNum, macPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmIpRouterMacSaBaseSet(devNum, macPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpRouterMacSaBaseGet function
* @endinternal
*
* @brief   Gets 40 MSBs of Router MAC SA Base address on specified device.
*
* @param[in] devNum                   - the device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaBaseGet
(
    IN  GT_U8           devNum,
    OUT GT_ETHERADDR    *macPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterMacSaBaseGet(devNum, macPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmIpRouterMacSaBaseGet(devNum, macPtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpRouterMacSaModifyEnable function
* @endinternal
*
* @brief   Per Egress port bit Enable Routed packets MAC SA Modification
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical or CPU port number.
* @param[in] enable                   - GT_FALSE: MAC SA Modification of routed packets is disabled
*                                      GT_TRUE: MAC SA Modification of routed packets is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaModifyEnable
(
    IN  GT_U8                      devNum,
    IN  GT_U32                      portNum,
    IN  GT_BOOL                    enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterMacSaModifyEnable(
        devNum, portNum, enable);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enable);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpRouterMacSaModifyEnableGet function
* @endinternal
*
* @brief   Per Egress port bit Get Routed packets MAC SA Modification State
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - physical or CPU port number.
*
* @param[out] enablePtr                - GT_FALSE: MAC SA Modification of routed packets is disabled
*                                      GT_TRUE: MAC SA Modification of routed packets is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterMacSaModifyEnableGet
(
   IN  GT_U8                       devNum,
   IN  GT_PHYSICAL_PORT_NUM        portNum,
   OUT GT_BOOL                     *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterMacSaModifyEnableGet(devNum, portNum, enablePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(enablePtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpPortRouterMacSaLsbModeSet function
* @endinternal
*
* @brief   Sets the mode, per port, in which the device sets the packet's MAC SA
*         least significant bytes.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
* @param[in] saLsbMode                - The MAC SA least-significant bit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/saLsbMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device 5 most significant bytes are set by cpssDxChIpRouterMacSaBaseSet().
*       The least significant bytes are set by:
*       Port mode is set by cpssDxChIpRouterPortMacSaLsbSet().
*       Vlan mode is set by cpssDxChIpRouterVlanMacSaLsbSet().
*
*/
GT_STATUS prvTgfIpPortRouterMacSaLsbModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                       portNum,
    IN  CPSS_MAC_SA_LSB_MODE_ENT    saLsbMode
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpPortRouterMacSaLsbModeSet(
        devNum, portNum, saLsbMode);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmIpPortRouterMacSaLsbModeSet(
        devNum, portNum, saLsbMode);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpPortRouterMacSaLsbModeGet function
* @endinternal
*
* @brief   Gets the mode, per port, in which the device sets the packet's MAC SA
*         least significant bytes.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number
*
* @param[out] saLsbModePtr             - The MAC SA least-significant bit mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum/saLsbMode.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device 40 most significant bytes are get by cpssDxChIpRouterMacSaBaseGet().
*       The least significant bytes are get by:
*       Port mode is get by cpssDxChIpRouterPortMacSaLsbGet().
*       Vlan mode is get by cpssDxChIpRouterVlanMacSaLsbGet().
*
*/
GT_STATUS prvTgfIpPortRouterMacSaLsbModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                       portNum,
    OUT CPSS_MAC_SA_LSB_MODE_ENT    *saLsbModePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpPortRouterMacSaLsbModeGet(
        devNum, portNum, saLsbModePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmIpPortRouterMacSaLsbModeGet(
        devNum, portNum, saLsbModePtr);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}



/**
* @internal prvTgfIpRouterGlobalMacSaSet function
* @endinternal
*
* @brief   Sets full 48-bit Router MAC SA in Global MAC SA table.
*
* @param[in] devNum                   - the device number.
* @param[in] routerMacSaIndex         - The index into the global MAC SA table.
* @param[in] macSaAddrPtr             - The 48 bits MAC SA.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or routerMacSaIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpRouterGlobalMacSaSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      routerMacSaIndex,
    IN  GT_ETHERADDR                *macSaAddrPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterGlobalMacSaSet(
        devNum, routerMacSaIndex & 0xFF, macSaAddrPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return GT_BAD_PARAM;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfIpRouterGlobalMacSaGet function
* @endinternal
*
* @brief   Gets full 48-bit Router MAC SA from Global MAC SA table.
*
* @param[in] devNum                   - the device number.
* @param[in] routerMacSaIndex         - The index into the global MAC SA table.
*
* @param[out] macSaAddrPtr             - The 48 bits MAC SA.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - wrong devNum or routerMacSaIndex.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note This API is applicable when per-egress-physical-port MAC SA assignment
*       mode is configured to global :CPSS_SA_LSB_FULL_48_BIT_GLOBAL by API
*       cpssDxChIpPortRouterMacSaLsbModeSet. The routerMacSaIndex is configured
*       by cpssDxChIpRouterGlobalMacSaIndexSet.
*
*/
GT_STATUS prvTgfIpRouterGlobalMacSaGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       routerMacSaIndex,
    OUT GT_ETHERADDR *macSaAddrPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterGlobalMacSaGet(devNum, routerMacSaIndex, macSaAddrPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return GT_BAD_PARAM;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpRouterPortGlobalMacSaIndexSet function
* @endinternal
*
* @brief   Set router mac sa index refered to global MAC SA table.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] routerMacSaIndex         - global MAC SA table index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpRouterPortGlobalMacSaIndexSet
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               routerMacSaIndex
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterPortGlobalMacSaIndexSet(
        devNum, portNum, routerMacSaIndex & 0xFF);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    return GT_BAD_PARAM;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfIpRouterPortGlobalMacSaIndexGet function
* @endinternal
*
* @brief   Get router mac sa index refered to global MAC SA table.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] routerMacSaIndexPtr      - (pointer to) global MAC SA table index.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvTgfIpRouterPortGlobalMacSaIndexGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *routerMacSaIndexPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterPortGlobalMacSaIndexGet(devNum, portNum, routerMacSaIndexPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return GT_BAD_PARAM;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */

}



/**
* @internal prvTgfIpRouterPortMacSaLsbSet function
* @endinternal
*
* @brief   Sets the 8 LSB Router MAC SA for this EGGRESS PORT.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - the device number
* @param[in] portNum                  - Eggress Port number
* @param[in] saMac                    - The 8 bits SA mac value to be written to the SA bits of
*                                      routed packet if SA alteration mode is configured to
*                                      take LSB according to Eggress Port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterPortMacSaLsbSet
(
    IN GT_U8   devNum,
    IN GT_U32   portNum,
    IN GT_U8   saMac
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterPortMacSaLsbSet(
        devNum, portNum, saMac);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmIpRouterPortMacSaLsbSet(
        devNum, portNum, saMac);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpRouterVlanMacSaLsbSet function
* @endinternal
*
* @brief   Sets the LSBs of Router MAC SA for this VLAN.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] devNum                   - the device number
* @param[in] vlan                     - VLAN Id
* @param[in] saMac                    - The Least Significant bits SA mac value to be written to the SA bits of
*                                      routed packet if SA alteration mode is configured to
*                                      take LSB according to VLAN.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterVlanMacSaLsbSet
(
    IN GT_U8   devNum,
    IN GT_U16  vlan,
    IN GT_U32  saMac
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpRouterVlanMacSaLsbSet(
        devNum, vlan, saMac);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmIpRouterVlanMacSaLsbSet(
        devNum, vlan, (GT_U8)saMac);
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}


/**
* @internal prvTgfIpRouterNextHopTableAgeBitsEntryRead function
* @endinternal
*
* @brief   read router next hop table age bits entry.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above.
*
* @param[out] activityBitPtr           - (pointer to) Age bit value of requested Next-hop entry.
*                                      field. Range 0..1.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvTgfIpRouterNextHopTableAgeBitsEntryRead
(
    GT_U32     *activityBitPtr
)
{
#ifdef CHX_FAMILY
    GT_U32      rc = GT_OK;
    GT_U32      routerNextHopTableAgeBitsEntryIndex;
    GT_U32      routerNextHopTableAgeBitsEntryPtr;
    GT_U8       bitOffset;

    routerNextHopTableAgeBitsEntryIndex = prvTgfRouteEntryBaseIndex / 32;
    bitOffset = (GT_U8)(prvTgfRouteEntryBaseIndex % 32);

    /* call device specific API */
    rc = cpssDxChIpRouterNextHopTableAgeBitsEntryRead(prvTgfDevNum,
                                                      routerNextHopTableAgeBitsEntryIndex,
                                                      &routerNextHopTableAgeBitsEntryPtr);
    if(rc != GT_OK)
        return rc;

    *activityBitPtr = ((routerNextHopTableAgeBitsEntryPtr >> bitOffset) & 0x1);

    return rc;

#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    return cpssExMxPmIpRouteEntryAgeRefreshGet(prvTgfDevNum, prvTgfRouteEntryBaseIndex, activityBitPtr);

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmDBCapacityGet function
* @endinternal
*
* @brief   This function gets the current LPM DB allocation.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] lpmDBId                  - the LPM DB id.
*
* @param[out] indexesRangePtr          - the range of TCAM indexes availble for this
*                                      LPM DB (see explanation in
*                                      CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC).
*                                      this field is relevant when partitionEnable
*                                      in cpssDxChIpLpmDBCreate was GT_TRUE.
* @param[out] partitionEnablePtr       - GT_TRUE:  the TCAM is partitioned according
*                                      to the capacityCfgPtr, any unused TCAM entries
*                                      were allocated to IPv4 UC entries.
*                                      GT_FALSE: TCAM entries are allocated on demand
*                                      while entries are guaranteed as specified
*                                      in capacityCfgPtr.
* @param[out] tcamLpmManagerCapcityCfgPtr - the current capacity configuration. when
*                                      partitionEnable in cpssDxChIpLpmDBCreate
*                                      was set to GT_TRUE this means current
*                                      prefixes partition, when this was set to
*                                      GT_FALSE this means the current guaranteed
*                                      prefixes allocation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_RESOURCE           - failed to allocate resources needed to the
*                                       new configuration.
*/
GT_STATUS prvTgfIpLpmDBCapacityGet
(
    IN  GT_U32                                     lpmDBId,
    OUT GT_BOOL                                    *partitionEnablePtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC dxChIndexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC dxChTcamLpmManagerCapcityCfg;

    /* reset variables */
    cpssOsMemSet(&dxChIndexesRange,             0, sizeof(dxChIndexesRange));
    cpssOsMemSet(&dxChTcamLpmManagerCapcityCfg, 0, sizeof(dxChTcamLpmManagerCapcityCfg));

    /* call device specific API */
    rc = cpssDxChIpLpmDBCapacityGet(lpmDBId,
                                    partitionEnablePtr,
                                    &dxChIndexesRange,
                                    &dxChTcamLpmManagerCapcityCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDBCapacityGet FAILED, rc = [%d]", rc);
    }

    /* convert indexesRangePtr into device specific format */
    indexesRangePtr->firstIndex = dxChIndexesRange.firstIndex;
    indexesRangePtr->lastIndex  = dxChIndexesRange.lastIndex;

    /* convert the counter Set vlan mode into device specific format */
    tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes         = dxChTcamLpmManagerCapcityCfg.numOfIpv4Prefixes;
    tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes = dxChTcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes;
    tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes         = dxChTcamLpmManagerCapcityCfg.numOfIpv6Prefixes;

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lpmDBId);
    TGF_PARAM_NOT_USED(partitionEnablePtr);
    TGF_PARAM_NOT_USED(indexesRangePtr);
    TGF_PARAM_NOT_USED(tcamLpmManagerCapcityCfgPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmDBCapacityUpdate function
* @endinternal
*
* @brief   This function updates the initial LPM DB allocation.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above.
* @param[in] lpmDBId                  - the LPM DB id.
* @param[in] indexesRangePtr          - the range of TCAM indexes availble for this
*                                      LPM DB (see explanation in
*                                      CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC).
*                                      this field is relevant when partitionEnable
*                                      in cpssDxChIpLpmDBCreate was GT_TRUE.
* @param[in] tcamLpmManagerCapcityCfgPtr - the new capacity configuration. when
*                                      partitionEnable in cpssDxChIpLpmDBCreate
*                                      was set to GT_TRUE this means new prefixes
*                                      partition, when this was set to GT_FALSE
*                                      this means the new prefixes guaranteed
*                                      allocation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NO_RESOURCE           - failed to allocate resources needed to the
*                                       new configuration
*
* @note This API is relevant only when using TCAM Manager. This API is used for
*       updating only the capacity configuration of the LPM. for updating the
*       lines reservation for the TCAM Manger use cpssDxChTcamManagerRangeUpdate.
*
*/
GT_STATUS prvTgfIpLpmDBCapacityUpdate
(
    IN GT_U32                                     lpmDBId,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC dxChIndexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC dxChTcamLpmManagerCapcityCfg;

    /* reset variables */
    cpssOsMemSet(&dxChIndexesRange,             0, sizeof(dxChIndexesRange));
    cpssOsMemSet(&dxChTcamLpmManagerCapcityCfg, 0, sizeof(dxChTcamLpmManagerCapcityCfg));

    /* convert indexesRangePtr into device specific format */
    dxChIndexesRange.firstIndex = indexesRangePtr->firstIndex;
    dxChIndexesRange.lastIndex  = indexesRangePtr->lastIndex;

    /* convert the counter Set vlan mode into device specific format */
    dxChTcamLpmManagerCapcityCfg.numOfIpv4Prefixes         = tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes;
    dxChTcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes;
    dxChTcamLpmManagerCapcityCfg.numOfIpv6Prefixes         = tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes;

    /* call device specific API */
    rc = cpssDxChIpLpmDBCapacityUpdate(lpmDBId,
                                       &dxChIndexesRange,
                                       &dxChTcamLpmManagerCapcityCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDBCapacityUpdate FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lpmDBId);
    TGF_PARAM_NOT_USED(indexesRangePtr);
    TGF_PARAM_NOT_USED(tcamLpmManagerCapcityCfgPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmDBConfigGet function
* @endinternal
*
* @brief   This function retrieves configuration of the LPM DB.
*
* @param[in] lpmDBId                  - the LPM DB id.
*
* @param[out] shadowTypePtr            - (pointer to) shadow type
* @param[out] protocolStackPtr         - (pointer to) protocol stack this LPM DB support
* @param[out] indexesRangePtr          - (pointer to) range of TCAM indexes availble for this LPM DB
* @param[out] partitionEnablePtr       - GT_TRUE:  TCAM is partitioned to different prefix types
*                                      GT_FALSE: TCAM entries are allocated on demand
* @param[out] tcamLpmManagerCapcityCfgPtr - (pointer to) capacity configuration
* @param[out] isTcamManagerInternalPtr - GT_TRUE:  TCAM manager is used and created
*                                      internally by the LPM DB
*                                      GT_FALSE: otherwise
* @param[out] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_PTR               - NULL pointer.
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmDBConfigGet
(
    IN  GT_U32                                         lpmDBId,
    OUT PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT               *shadowTypePtr,
    OUT CPSS_IP_PROTOCOL_STACK_ENT                    *protocolStackPtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    *indexesRangePtr,
    OUT GT_BOOL                                       *partitionEnablePtr,
    OUT PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    *tcamLpmManagerCapcityCfgPtr,
    OUT GT_BOOL                                       *isTcamManagerInternalPtr,
    OUT GT_VOID                                       *tcamManagerHandlerPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                                    rc = GT_OK;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             dxChShadowType;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC dxChIndexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC dxChTcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;

    /* reset variables */
    cpssOsMemSet(&dxChIndexesRange,             0, sizeof(dxChIndexesRange));
    cpssOsMemSet(&dxChTcamLpmManagerCapcityCfg, 0, sizeof(dxChTcamLpmManagerCapcityCfg));
    cpssOsMemSet(&lpmMemoryConfig, 0, sizeof(lpmMemoryConfig));

    lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &dxChIndexesRange;
    lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &dxChTcamLpmManagerCapcityCfg;
    lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = tcamManagerHandlerPtr;
    /* call device specific API */
    rc = cpssDxChIpLpmDBConfigGet(lpmDBId, &dxChShadowType, protocolStackPtr,
                                  &lpmMemoryConfig);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDBConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert shadowType from device specific format */
    switch (dxChShadowType)
    {
        case CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E:
            *shadowTypePtr = PRV_TGF_IP_TCAM_ROUTER_BASED_SHADOW_E;
            break;

        case CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E:
            *shadowTypePtr = PRV_TGF_IP_TCAM_POLICY_BASED_ROUTING_SHADOW_E;
            break;

        case CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E:
            *shadowTypePtr = PRV_TGF_IP_RAM_SHADOW_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert indexesRangePtr from device specific format */
    indexesRangePtr->firstIndex = dxChIndexesRange.firstIndex;
    indexesRangePtr->lastIndex  = dxChIndexesRange.lastIndex;

    *partitionEnablePtr = lpmMemoryConfig.tcamDbCfg.partitionEnable;

    /* convert tcamLpmManagerCapcityCfgPtr from device specific format */
    tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes         = dxChTcamLpmManagerCapcityCfg.numOfIpv4Prefixes;
    tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes = dxChTcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes;
    tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes         = dxChTcamLpmManagerCapcityCfg.numOfIpv6Prefixes;

    *isTcamManagerInternalPtr = (tcamManagerHandlerPtr ? GT_FALSE : GT_TRUE);

    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lpmDBId);
    TGF_PARAM_NOT_USED(shadowTypePtr);
    TGF_PARAM_NOT_USED(protocolStackPtr);
    TGF_PARAM_NOT_USED(indexesRangePtr);
    TGF_PARAM_NOT_USED(partitionEnablePtr);
    TGF_PARAM_NOT_USED(tcamLpmManagerCapcityCfgPtr);
    TGF_PARAM_NOT_USED(isTcamManagerInternalPtr);
    TGF_PARAM_NOT_USED(tcamManagerHandlerPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmDBCreate function
* @endinternal
*
* @brief   This function creates an LPM DB for a shared LPM managment.
*
* @param[in] lpmDBId                  - LPM DB id.
* @param[in] shadowType               - type of shadow to maintain
* @param[in] protocolStack            - type of protocol stack this LPM DB support
* @param[in] indexesRangePtr          - (pointer to) range of TCAM indexes
* @param[in] partitionEnable          - GT_TRUE:  partition the TCAM range according to the
*                                      capacityCfgPtr, any unused TCAM entries will
*                                      be allocated to IPv4 UC entries
*                                      GT_FALSE: allocate TCAM entries on demand while
*                                      guarantee entries as specified in capacityCfgPtr
* @param[in] tcamLpmManagerCapcityCfgPtr - (pointer to) capacity configuration
* @param[in] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmDBCreate
(
    IN GT_U32                                      lpmDBId,
    IN PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT             shadowType,
    IN CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN GT_BOOL                                     partitionEnable,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr,
    IN GT_VOID                                    *tcamManagerHandlerPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             dxChShadowType;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC dxChIndexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC dxChTcamLpmManagerCapcityCfg;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;

    /* reset variables */
    cpssOsMemSet(&dxChIndexesRange,             0, sizeof(dxChIndexesRange));
    cpssOsMemSet(&dxChTcamLpmManagerCapcityCfg, 0, sizeof(dxChTcamLpmManagerCapcityCfg));
    cpssOsMemSet(&lpmMemoryConfig, 0, sizeof(lpmMemoryConfig));

    /* convert shadowType into device specific format */
    switch (shadowType)
    {
        case PRV_TGF_IP_TCAM_ROUTER_BASED_SHADOW_E:
            dxChShadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;

            break;

        case PRV_TGF_IP_TCAM_POLICY_BASED_ROUTING_SHADOW_E:
            dxChShadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E;
            break;

        case PRV_TGF_IP_RAM_SHADOW_E:
            dxChShadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* convert indexesRangePtr into device specific format */
    dxChIndexesRange.firstIndex = indexesRangePtr->firstIndex;
    dxChIndexesRange.lastIndex  = indexesRangePtr->lastIndex;
    lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &dxChIndexesRange;

    /* convert tcamLpmManagerCapcityCfgPtr into device specific format */
    dxChTcamLpmManagerCapcityCfg.numOfIpv4Prefixes         = tcamLpmManagerCapcityCfgPtr->numOfIpv4Prefixes;
    dxChTcamLpmManagerCapcityCfg.numOfIpv4McSourcePrefixes = tcamLpmManagerCapcityCfgPtr->numOfIpv4McSourcePrefixes;
    dxChTcamLpmManagerCapcityCfg.numOfIpv6Prefixes         = tcamLpmManagerCapcityCfgPtr->numOfIpv6Prefixes;
    lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &dxChTcamLpmManagerCapcityCfg;

    lpmMemoryConfig.tcamDbCfg.partitionEnable = partitionEnable;
    lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = tcamManagerHandlerPtr;

    /* call device specific API */
    return cpssDxChIpLpmDBCreate(lpmDBId, dxChShadowType, protocolStack,
                                 &lpmMemoryConfig);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lpmDBId);
    TGF_PARAM_NOT_USED(shadowType);
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(indexesRangePtr);
    TGF_PARAM_NOT_USED(partitionEnable);
    TGF_PARAM_NOT_USED(tcamLpmManagerCapcityCfgPtr);
    TGF_PARAM_NOT_USED(tcamManagerHandlerPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmRamDBCreate function
* @endinternal
*
* @brief   This function creates an RAM LPM DB for a shared LPM managment.
*
* @param[in] lpmDBId                  - LPM DB id.
* @param[in] protocolStack            - type of protocol stack this LPM DB support
* @param[in] ramDbCfgPtr              - (pointer to)the ram memory configuration for this LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmRamDBCreate
(
    IN GT_U32                                      lpmDBId,
    IN CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack,
    PRV_TGF_LPM_RAM_CONFIG_STC                     *ramDbCfgPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             dxChShadowType;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;
    GT_U32                                       i;

    /* reset variables */
    cpssOsMemSet(&lpmMemoryConfig, 0, sizeof(lpmMemoryConfig));

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        dxChShadowType = CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E;
        lpmMemoryConfig.ramDbCfg.maxNumOfPbrEntries = ramDbCfgPtr->maxNumOfPbrEntries;
        switch (ramDbCfgPtr->sharedMemCnfg)
        {
        case PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E:
            lpmMemoryConfig.ramDbCfg.sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E;
            break;
        case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E:
            lpmMemoryConfig.ramDbCfg.sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E;
            break;
        case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:
        case PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:
            lpmMemoryConfig.ramDbCfg.sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;
            break;
        default:
            return GT_BAD_PARAM;
        }
    }
    else
    {
        dxChShadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
    }
    if (!(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))
    {
        lpmMemoryConfig.ramDbCfg.numOfBlocks = ramDbCfgPtr->numOfBlocks;
        for (i=0;i<lpmMemoryConfig.ramDbCfg.numOfBlocks;i++)
        {
            lpmMemoryConfig.ramDbCfg.blocksSizeArray[i] = ramDbCfgPtr->blocksSizeArray[i];
        }
    }

    switch (ramDbCfgPtr->blocksAllocationMethod)
    {
    case PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E:
        lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;
        break;
    case PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E:
        lpmMemoryConfig.ramDbCfg.blocksAllocationMethod = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    switch (ramDbCfgPtr->lpmMemMode)
    {
    case PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E:
        lpmMemoryConfig.ramDbCfg.lpmMemMode= CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
        break;
    case PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:
        lpmMemoryConfig.ramDbCfg.lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChIpLpmDBCreate(lpmDBId, dxChShadowType, protocolStack,
                                 &lpmMemoryConfig);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lpmDBId);
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(ramDbCfgPtr);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculates the default RAM LPM DB configuration for LPM managment.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PTR               - Null pointer
*/
GT_STATUS prvTgfIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                        devNum,
    OUT PRV_TGF_LPM_RAM_CONFIG_STC   *ramDbCfgPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    GT_U32    i = 0;
    CPSS_DXCH_LPM_RAM_CONFIG_STC appDemoRamDbCfg;

    CPSS_NULL_PTR_CHECK_MAC(ramDbCfgPtr);
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        rc = appDemoFalconIpLpmRamDefaultConfigCalc(devNum,
                                                    PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->moduleCfg.ip.sharedTableMode,
                                                    PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->moduleCfg.ip.maxNumOfPbrEntries,
                                                    &appDemoRamDbCfg);
    }
    else
    {
        rc = appDemoBc2IpLpmRamDefaultConfigCalc(devNum,
                                                 PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->moduleCfg.ip.maxNumOfPbrEntries,
                                                 &appDemoRamDbCfg);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        ramDbCfgPtr->maxNumOfPbrEntries = appDemoRamDbCfg.maxNumOfPbrEntries;
        switch (appDemoRamDbCfg.sharedMemCnfg)
        {
        case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E:
            ramDbCfgPtr->sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E;
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E:
            ramDbCfgPtr->sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E;
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:
        case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:
            ramDbCfgPtr->sharedMemCnfg = PRV_TGF_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;
            break;
        default:
            return GT_BAD_PARAM;
        }
    }
    else
    {
        ramDbCfgPtr->numOfBlocks = appDemoRamDbCfg.numOfBlocks;
        for (i=0;i<ramDbCfgPtr->numOfBlocks;i++)
        {
            ramDbCfgPtr->blocksSizeArray[i] = appDemoRamDbCfg.blocksSizeArray[i];
        }
    }
    switch (appDemoRamDbCfg.blocksAllocationMethod)
    {
    case CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E:
        ramDbCfgPtr->blocksAllocationMethod = PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;
        break;
    case CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E:
        ramDbCfgPtr->blocksAllocationMethod = PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    switch (appDemoRamDbCfg.lpmMemMode)
    {
    case CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E:
        ramDbCfgPtr->lpmMemMode = PRV_TGF_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
        break;
    case CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:
        ramDbCfgPtr->lpmMemMode = PRV_TGF_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_NOT_SUPPORTED;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpLpmRamDBCreateDefault function
* @endinternal
*
* @brief   This function creates a default RAM LPM DB for a shared LPM managment.
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmRamDBCreateDefault
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    cpssOsMemSet(&ramDbCfg, 0, sizeof(ramDbCfg));

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    if(rc != GT_OK)
    {
        return rc;
    }

    return prvTgfIpLpmRamDBCreate(0,CPSS_IP_PROTOCOL_IPV4V6_E,&ramDbCfg);
}

/**
* @internal prvTgfIpLpmDBDelete function
* @endinternal
*
* @brief   This function deletes LPM DB.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_STATE             - the LPM DB is not empty
* @retval GT_FAIL                  - on error
*
* @note Deleting LPM DB is allowed only if no VR present in the LPM DB (since
*       prefixes reside within VR it means that no prefixes present as well).
*
*/
GT_STATUS prvTgfIpLpmDBDelete
(
    IN GT_U32           lpmDBId
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpLpmDBDelete(lpmDBId);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lpmDBId);

    return GT_NOT_IMPLEMENTED;
#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmDBDeleteDefault function
* @endinternal
*
* @brief   This function deletes default LPM DB.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_BAD_STATE             - the LPM DB is not empty
* @retval GT_FAIL                  - on error
*
* @note Deleting LPM DB is allowed only if no VR present in the LPM DB (since
*       prefixes reside within VR it means that no prefixes present as well).
*
*/
GT_STATUS prvTgfIpLpmDBDeleteDefault
(
    GT_VOID
)
{
    return prvTgfIpLpmDBDelete(0);
}

/**
* @internal prvTgfIpLpmDBExtTcamManagerAdd function
* @endinternal
*
* @brief   This function add external TCAM manager into default LPM DB
*
* @param[in] lpmDBId                  - LPM DB id
* @param[in] tcamManagerRangePtr      - (pointer to) allocated TCAM range
*
* @param[out] tcamManagerHandlerPtrPtr - (pointer to) pointer of the created TCAM manager
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
* @retval GT_OUT_OF_CPU_MEM        - if no CPU memory for memory allocation
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvTgfIpLpmDBExtTcamManagerAdd
(
    IN  GT_U32                                lpmDBId,
    IN  PRV_TGF_TCAM_MANAGER_RANGE_STC       *tcamManagerRangePtr,
    OUT GT_VOID                             **tcamManagerHandlerPtrPtr
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_TCAM_MANAGER_TCAM_TYPE_ENT  tcamType;
    GT_PTR                              defTcamManagerHandlerPtr;


    /* retrieves configuration of the LPM DB */
    rc = prvTgfIpLpmDBConfigGet(lpmDBId,
                                &(prvTgfDefLpmDBVrCfg.shadowType),
                                &(prvTgfDefLpmDBVrCfg.protocolStack),
                                &(prvTgfDefLpmDBVrCfg.indexesRange),
                                &(prvTgfDefLpmDBVrCfg.partitionEnable),
                                &(prvTgfDefLpmDBVrCfg.tcamLpmManagerCapcityCfg),
                                &(prvTgfDefLpmDBVrCfg.isTcamManagerInternal),
                                &defTcamManagerHandlerPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* retrieves configuration of the Virtual Router */
    rc = prvTgfIpLpmVirtualRouterGet(lpmDBId, prvTgfDefVrId,
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry));
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* delete Virtual Router */
    rc = prvTgfIpLpmVirtualRouterDel(lpmDBId, prvTgfDefVrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterDel FAILED, rc = [%d]", rc);

        return rc;
    }

    /* delete default LPM DB */
    rc = prvTgfIpLpmDBDelete(lpmDBId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBDelete FAILED, rc = [%d]", rc);

        return rc;
    }

    /* init check for memory leaks */
    osMemStartHeapAllocationCounter();

    /* set TCAM type */
    tcamType = (PRV_TGF_IP_TCAM_POLICY_BASED_ROUTING_SHADOW_E == prvTgfDefLpmDBVrCfg.shadowType) ?
                PRV_TGF_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E :
                PRV_TGF_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E;

    /* create TCAM maneger */
    rc = prvTgfTcamManagerCreate(tcamType, tcamManagerRangePtr, tcamManagerHandlerPtrPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfTcamManagerCreate FAILED, rc = [%d]", rc);

        return rc;
    }

    /* create new LPM DB with external TCAM manager */
    rc = prvTgfIpLpmDBCreate(lpmDBId,
                             prvTgfDefLpmDBVrCfg.shadowType,
                             prvTgfDefLpmDBVrCfg.protocolStack,
                             &(prvTgfDefLpmDBVrCfg.indexesRange),
                             prvTgfDefLpmDBVrCfg.partitionEnable,
                             &(prvTgfDefLpmDBVrCfg.tcamLpmManagerCapcityCfg),
                             *tcamManagerHandlerPtrPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBCreate FAILED, rc = [%d]", rc);

        return rc;
    }

    /* add device to LPM DB */
    rc = prvTgfIpLpmDBDevListAdd(lpmDBId, &prvTgfDevNum, 1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBDevListAdd FAILED, rc = [%d]", rc);

        return rc;
    }

    /* add Virtual Router */
    rc = prvTgfIpLpmVirtualRouterAdd(lpmDBId, prvTgfDefVrId,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv4Uc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv6Uc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv4Mc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv6Mc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportFcoe) ?
                                      &(prvTgfDefLpmDBVrCfg.defFcoeNextHopInfo) : NULL);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterAdd FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
}

/**
* @internal prvTgfIpLpmDBExtTcamManagerRemove function
* @endinternal
*
* @brief   This function remove external TCAM manager from default LPM DB
*
* @param[in] lpmDBId                  - LPM DB id
* @param[in] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - if the LPM DB id is already used
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - request is not supported if partitioning is disabled
*/
GT_STATUS prvTgfIpLpmDBExtTcamManagerRemove
(
    IN  GT_U32                                lpmDBId,
    IN  GT_VOID                              *tcamManagerHandlerPtr
)
{
    GT_STATUS       rc, rc1 = GT_OK;
    GT_U32          usedMem = 0;


    /* delete Virtual Router */
    rc = prvTgfIpLpmVirtualRouterDel(lpmDBId, prvTgfDefVrId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterDel FAILED, rc = [%d]", rc);

        return rc;
    }

    /* delete LPM DB */
    rc = prvTgfIpLpmDBDelete(lpmDBId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBDelete FAILED, rc = [%d]", rc);

        return rc;
    }

    /* delete TCAM Manager handler */
    rc = prvTgfTcamManagerDelete(tcamManagerHandlerPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfTcamManagerDelete FAILED, rc = [%d]", rc);

        return rc;
    }

    /* check for memory leaks */
    usedMem = osMemGetHeapAllocationCounter();

    rc1 = (0 == usedMem) ? GT_OK : GT_FAIL;
    if (GT_OK != rc1)
    {
        PRV_UTF_LOG1_MAC("[TGF]: Memory leak detected: %d bytes \n", usedMem);
    }

    /* restore default LPM DB */
    rc = prvTgfIpLpmDBCreate(lpmDBId,
                             prvTgfDefLpmDBVrCfg.shadowType,
                             prvTgfDefLpmDBVrCfg.protocolStack,
                             &(prvTgfDefLpmDBVrCfg.indexesRange),
                             prvTgfDefLpmDBVrCfg.partitionEnable,
                             &(prvTgfDefLpmDBVrCfg.tcamLpmManagerCapcityCfg),
                             NULL);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBCreate FAILED, rc = [%d]", rc);

        return rc;
    }

    /* add device to LPM DB */
    rc = prvTgfIpLpmDBDevListAdd(lpmDBId, &prvTgfDevNum, 1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterAdd FAILED, rc = [%d]", rc);

        return rc;
    }

    /* add Virtual Router */
    rc = prvTgfIpLpmVirtualRouterAdd(lpmDBId, prvTgfDefVrId,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv4Uc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv6Uc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv4Mc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportIpv6Mc) ?
                                      &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry) : NULL,
                                     (GT_TRUE == prvTgfDefLpmDBVrCfg.isSupportFcoe) ?
                                      &(prvTgfDefLpmDBVrCfg.defFcoeNextHopInfo) : NULL);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmVirtualRouterAdd FAILED, rc = [%d]", rc);

        return rc;
    }

    /* init check for memory leaks */
    osMemStartHeapAllocationCounter();

    return (GT_OK == rc) ? rc1 : rc;
}

/**
* @internal prvTgfIpLpmDBWithTcamCapacityUpdate function
* @endinternal
*
* @brief   This function updates LPM DB allocation and reserved TCAM lines.
*
* @param[in] tcamManagerHandlerPtr    - (pointer to) TCAM manager handler
* @param[in] lpmDBId                  - LPM DB id
* @param[in] indexesRangePtr          - (pointer to) range of TCAM indexes
* @param[in] tcamLpmManagerCapcityCfgPtr - (pointer to) new capacity configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - if the LPM DB id is not found
* @retval GT_NO_RESOURCE           - failed to allocate resources
*/
GT_STATUS prvTgfIpLpmDBWithTcamCapacityUpdate
(
    IN GT_VOID                                    *tcamManagerHandlerPtr,
    IN GT_U32                                      lpmDBId,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *indexesRangePtr,
    IN PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *tcamLpmManagerCapcityCfgPtr
)
{
    GT_STATUS                                   rc = GT_OK;
    GT_BOOL                                     partitionEnable = GT_FALSE;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC  indexesRange;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC  tcamLpmMngrCapcityCfg;
    PRV_TGF_TCAM_MANAGER_RANGE_STC              tcamMngrRange;


    /* clear entries */
    cpssOsMemSet(&indexesRange,          0, sizeof(indexesRange));
    cpssOsMemSet(&tcamLpmMngrCapcityCfg, 0, sizeof(tcamLpmMngrCapcityCfg));
    cpssOsMemSet(&tcamMngrRange, 0, sizeof(tcamMngrRange));

    if (indexesRangePtr->firstIndex > indexesRangePtr->lastIndex)
        return GT_BAD_PARAM;

    /* get LpmDB capacity */
    rc = prvTgfIpLpmDBCapacityGet(lpmDBId,
                                  &partitionEnable,
                                  &indexesRange,
                                  &tcamLpmMngrCapcityCfg);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBCapacityGet FAILED, rc = [%d]", rc);

        return rc;
    }

    if ((indexesRange.lastIndex - indexesRange.firstIndex) >
        (indexesRangePtr->lastIndex - indexesRangePtr->firstIndex)) /* shrink */
    {
        /* update LpmDB capacity */
        rc = prvTgfIpLpmDBCapacityUpdate(lpmDBId,
                                         indexesRangePtr,
                                         tcamLpmManagerCapcityCfgPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBCapacityUpdate FAILED, rc = [%d]", rc);

            return rc;
        }

        /* update TCAM ranges */
        tcamMngrRange.firstLine = indexesRangePtr->firstIndex;
        tcamMngrRange.lastLine  = indexesRangePtr->lastIndex;

        rc = prvTgfTcamManagerRangeUpdate(tcamManagerHandlerPtr,
                                          &tcamMngrRange,
                                          PRV_TGF_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfTcamManagerRangeUpdate FAILED, rc = [%d]", rc);

            return rc;
        }
    }
    else /* enlarge */
    {
        /* update TCAM ranges */
        tcamMngrRange.firstLine = indexesRangePtr->firstIndex;
        tcamMngrRange.lastLine  = indexesRangePtr->lastIndex;

        rc = prvTgfTcamManagerRangeUpdate(tcamManagerHandlerPtr,
                                          &tcamMngrRange,
                                          PRV_TGF_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfTcamManagerRangeUpdate FAILED, rc = [%d]", rc);

            return rc;
        }

        /* update LpmDB capacity */
        rc = prvTgfIpLpmDBCapacityUpdate(lpmDBId,
                                         indexesRangePtr,
                                         tcamLpmManagerCapcityCfgPtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDBCapacityUpdate FAILED, rc = [%d]", rc);

            return rc;
        }
    }

    return rc;
}


/**
* @internal prvTgfIpValidityCheck function
* @endinternal
*
* @brief   Check Patricia trie validity for specific LPM DB Id and virtual router Id
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
*                                      entryType   - uc/mc prefix type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - on bad state in patricia trie
*/
GT_STATUS prvTgfIpValidityCheck
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType

)
{
    if (GT_FALSE == prvTgfIpValidityCheckFlag)
    {
        /* do not perform IP validation */
        return GT_OK;
    }

#ifdef CHX_FAMILY
    /* call device specific API */
    if ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)) || (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))
    {
        return prvTgfIpLpmValidityCheck(lpmDBId, vrId, protocol, prefixType, GT_TRUE);
    }
    else
    {
        return cpssDxChIpLpmDbgPatTrieValidityCheck(lpmDBId, vrId, protocol, prefixType);
    }
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    /* avoid warnings */
    TGF_PARAM_NOT_USED(lpmDBId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(protocol);
    TGF_PARAM_NOT_USED(prefixType);

    return GT_OK;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpValidityCheckEnable function
* @endinternal
*
* @brief   This function enable/disable Patricia trie validity check.
*
* @param[in] validityFlag             - flag whether to check (GT_TRUE) Patricia trie validity
*                                       None.
*/
GT_VOID prvTgfIpValidityCheckEnable
(
    IN GT_BOOL validityFlag
)
{
    prvTgfIpValidityCheckFlag = validityFlag;
}

/**
* @internal prvTgfIpLpmValidityCheck function
* @endinternal
*
* @brief   Check IP LPM Shadow validity for specific LPM DB Id and virtual router Id
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops at first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - on bad state in patricia trie
*/
GT_STATUS prvTgfIpLpmValidityCheck
(
    IN GT_U32 lpmDBId,
    IN GT_U32 vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType,
    IN GT_BOOL                    returnOnFailure

)
{
    GT_STATUS rc = GT_OK;
#ifdef EXMXPM_FAMILY
    GT_BOOL isIpv4;
    GT_BOOL isUnicast;
#endif
    if (GT_FALSE == prvTgfIpValidityCheckFlag)
    {
        /* do not perform IP validation */
        return GT_OK;
    }

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    rc = cpssExMxPmIpLpmShadowValidityCheck(lpmDBId, vrId, protocol, prefixType,returnOnFailure);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpLpmShadowValidityCheck FAILED, rc = [%d]", rc);

        return rc;
    }

    switch(protocol)
    {
    case  CPSS_IP_PROTOCOL_IPV4_E:
        isIpv4=GT_TRUE;
        break;
    case  CPSS_IP_PROTOCOL_IPV6_E:
        isIpv4=GT_FALSE;
        break;
    case  CPSS_IP_PROTOCOL_IPV4V6_E:
    default:
        return GT_BAD_PARAM;

    }

    switch(prefixType)
    {
    case  CPSS_UNICAST_E:
        isUnicast=GT_TRUE;
        break;
    case  CPSS_MULTICAST_E:
        isUnicast=GT_FALSE;
        break;
    case  CPSS_UNICAST_MULTICAST_E:
    default:
        return GT_BAD_PARAM;

    }

    rc = cpssExMxPmIpLpmHwValidatation(prvTgfDevNum, vrId,isIpv4,isUnicast);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpLpmHwValidatation FAILED, rc = [%d]", rc);

        return rc;
    }
    return rc;

#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    /* call device specific API */
    rc = cpssDxChIpLpmDbgShadowValidityCheck(lpmDBId, vrId, protocol, prefixType, returnOnFailure);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgShadowValidityCheck FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChIpLpmDbgHwValidation(prvTgfDevNum, vrId, protocol, prefixType);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgHwValidation FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChIpLpmDbgHwShadowSyncValidityCheck(prvTgfDevNum, lpmDBId, vrId, protocol, prefixType,returnOnFailure);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgHwShadowSyncValidityCheck FAILED, rc = [%d]", rc);

        return rc;
    }
    return rc;

#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmDbgIpv4UcPrefixAddManyRandom function
* @endinternal
*
* @brief   This function tries to add many random IPv4 Unicast prefixes and
*         returns the number of prefixes successfully added.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] isWholeIpRandom          - GT_TRUE: all IP octets calculated by cpssOsRand
*                                      GT_FALSE: only 2 LSB octets calculated by cpssOsRand
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmDbgIpv4UcPrefixAddManyRandom
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_BOOL     isWholeIpRandom,
    IN  GT_BOOL     defragmentationEnable,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      numOfPrefixesAdded=0;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom(lpmDbId,
                                                    vrId,
                                                    startIpAddr,
                                                    routeEntryBaseMemAddr,
                                                    numOfPrefixesToAdd,
                                                    isWholeIpRandom,
                                                    defragmentationEnable,
                                                    &numOfPrefixesAdded);
    if ((GT_OK != rc)&&(rc != GT_OUT_OF_PP_MEM))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom FAILED, rc = [%d]", rc);

        return rc;
    }
    else
    {
        rc = GT_OK;
        *numOfPrefixesAddedPtr= numOfPrefixesAdded;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(isWholeIpRandom);
    TGF_PARAM_NOT_USED(numOfPrefixesAddedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmDbgPrefixLengthSet function
* @endinternal
*
* @brief   set prefix length used in addManyByOctet APIs
*
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
* @param[in] prefixLength             - prefix length
* @param[in] srcPrefixLength          - src prefix length
*/
GT_U32 prvTgfIpLpmDbgPrefixLengthSet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType,
    IN GT_U32                     prefixLength,
    IN GT_U32                     srcPrefixLength
)
{
    GT_STATUS   rc = GT_OK;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgPrefixLengthSet(protocolStack,prefixType,prefixLength,srcPrefixLength);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvCpssDxChIpLpmDbgPrefixLengthSet FAILED, rc = [%d]", rc);

        return rc;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(prefixType);
    TGF_PARAM_NOT_USED(prefixLength);
    TGF_PARAM_NOT_USED(srcPrefixLength);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 32). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPADDR   lastIpAddrAdded;
    GT_U32      numOfPrefixesAdded;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet(lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                                                     numOfPrefixesToAdd, firstOctetToChange,secondOctetToChange,
                                                     thirdOctetToChange,forthOctetToChange,&lastIpAddrAdded,
                                                     &numOfPrefixesAdded);
    if ((GT_OK != rc)&&(rc != GT_OUT_OF_PP_MEM))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4UcPrefixAddManyByOctet FAILED, rc = [%d]", rc);

        return rc;
    }
    else
    {
        rc = GT_OK;
        *lastIpAddrAddedPtr = lastIpAddrAdded;
        *numOfPrefixesAddedPtr= numOfPrefixesAdded;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrAddedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAddedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv4 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to delete
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of the prefixes
*       that was added by cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_IPADDR   *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPADDR   lastIpAddrDeleted;
    GT_U32      numOfPrefixesDeleted;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet(lpmDbId, vrId, startIpAddr, numOfPrefixesToDel, firstOctetToChange,secondOctetToChange,
                                                     thirdOctetToChange,forthOctetToChange,&lastIpAddrDeleted,&numOfPrefixesDeleted);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv4UcPrefixDelManyByOctet FAILED, rc = [%d]", rc);

        return rc;
    }
    else
    {
        *lastIpAddrDeletedPtr = lastIpAddrDeleted;
        *numOfPrefixesDeletedPtr= numOfPrefixesDeleted;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToDel);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrDeletedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesDeletedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixGetManyByOctet function
* @endinternal
*
* @brief   This function tries to get many sequential IPv4 Unicast prefixes (that were
*         added by cpssDxChIpLpmDbgIpv4UcPrefixAddManyByOctet) and returns the number of
*         successfully retrieved prefixes.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] numOfPrefixesToGet       - the number of prefixes to get
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] numOfRetrievedPrefixesPtr - points to the nubmer of prefixes that were
*                                      successfully retrieved (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that were be added.
*
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixGetManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPADDR   startIpAddr,
    IN  GT_U32      numOfPrefixesToGet,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    OUT GT_U32      *numOfRetrievedPrefixesPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      numOfRetrievedPrefixes;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet(lpmDbId, vrId, startIpAddr,
                                                     numOfPrefixesToGet, firstOctetToChange,secondOctetToChange,
                                                     thirdOctetToChange,forthOctetToChange,&numOfRetrievedPrefixes);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv4UcPrefixGetManyByOctet FAILED, rc = [%d]", rc);

        return rc;
    }
    else
    {
        *numOfRetrievedPrefixesPtr= numOfRetrievedPrefixes;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(numOfRetrievedPrefixesPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixAddManyByOctet function
* @endinternal
*
* @brief   This function tries to add many sequential IPv6 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully added.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] routeEntryBaseMemAddr    - base memory Address (offset) of the route entry
* @param[in] numOfPrefixesToAdd       - the number of prefixes to add
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrAddedPtr       - points to the last prefix successfully
*                                      added (NULL to ignore)
* @param[out] numOfPrefixesAddedPtr    - points to the nubmer of prefixes that were
*                                      successfully added (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to check the number of the prefixes
*       that can be added. All the prefixes are added with exact match (prefix
*       length 128). The route entry is not written.
*
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixAddManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR startIpAddr,
    IN  GT_U32      routeEntryBaseMemAddr,
    IN  GT_U32      numOfPrefixesToAdd,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrAddedPtr,
    OUT GT_U32      *numOfPrefixesAddedPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPV6ADDR   lastIpAddrAdded;
    GT_U32      numOfPrefixesAdded;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet(lpmDbId, vrId, startIpAddr, routeEntryBaseMemAddr,
                                                      numOfPrefixesToAdd, firstOctetToChange, secondOctetToChange,
                                                      thirdOctetToChange, forthOctetToChange, fifthOctetToChange,
                                                      sixthOctetToChange, seventhOctetToChange, eighthOctetToChange,
                                                      ninthOctetToChange, tenthOctetToChange, elevenOctetToChange,
                                                      twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                                                      fifteenOctetToChange, sixteenOctetToChange, &lastIpAddrAdded,
                                                      &numOfPrefixesAdded);
    if ((GT_OK != rc)&&(rc != GT_OUT_OF_PP_MEM))
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet FAILED, rc = [%d]", rc);

        return rc;
    }
    else
    {
        rc = GT_OK;
        *lastIpAddrAddedPtr = lastIpAddrAdded;
        *numOfPrefixesAddedPtr= numOfPrefixesAdded;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(routeEntryBaseMemAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToAdd);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(fifthOctetToChange);
    TGF_PARAM_NOT_USED(sixthOctetToChange);
    TGF_PARAM_NOT_USED(seventhOctetToChange);
    TGF_PARAM_NOT_USED(eighthOctetToChange);
    TGF_PARAM_NOT_USED(ninthOctetToChange);
    TGF_PARAM_NOT_USED(tenthOctetToChange);
    TGF_PARAM_NOT_USED(elevenOctetToChange);
    TGF_PARAM_NOT_USED(twelveOctetToChange);
    TGF_PARAM_NOT_USED(thirteenOctetToChange);
    TGF_PARAM_NOT_USED(fourteenOctetToChange);
    TGF_PARAM_NOT_USED(fifteenOctetToChange);
    TGF_PARAM_NOT_USED(sixteenOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrAddedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesAddedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixDelManyByOctet function
* @endinternal
*
* @brief   This function tries to delete many sequential IPv6 Unicast prefixes according
*         to octet order selection and returns the number of prefixes successfully deleted.
* @param[in] lpmDbId                  - the LPM DB id
* @param[in] vrId                     - the virtual router id
* @param[in] startIpAddr              - the first address to add
* @param[in] numOfPrefixesToDel       - the number of prefixes to deleted
* @param[in] firstOctetToChange       - first octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] secondOctetToChange      - second octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirdOctetToChange       - third octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] forthOctetToChange       - forth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifthOctetToChange       - fifth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixthOctetToChange       - sixth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] seventhOctetToChange     - seventh octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] eighthOctetToChange      - eighth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] ninthOctetToChange       - ninth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] tenthOctetToChange       - tenth octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] elevenOctetToChange      - eleven octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] twelveOctetToChange      - twelve octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] thirteenOctetToChange    - thirteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fourteenOctetToChange    - fourteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] fifteenOctetToChange     - fifteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
* @param[in] sixteenOctetToChange     - sixteen octet to change in the prefix address.
*                                      value of the octet can be up to 255
*
* @param[out] lastIpAddrDeletedPtr     - points to the last prefix successfully
*                                      deleted (NULL to ignore)
* @param[out] numOfPrefixesDeletedPtr  - points to the nubmer of prefixes that were
*                                      successfully deleted (NULL to ignore)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This debug function is intended to delete the number of prefixes
*       that was added by cpssDxChIpLpmDbgIpv6UcPrefixAddManyByOctet.
*
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixDelManyByOctet
(
    IN  GT_U32      lpmDbId,
    IN  GT_U32      vrId,
    IN  GT_IPV6ADDR startIpAddr,
    IN  GT_U32      numOfPrefixesToDel,
    IN  GT_U32      firstOctetToChange,
    IN  GT_U32      secondOctetToChange,
    IN  GT_U32      thirdOctetToChange,
    IN  GT_U32      forthOctetToChange,
    IN  GT_U32      fifthOctetToChange,
    IN  GT_U32      sixthOctetToChange,
    IN  GT_U32      seventhOctetToChange,
    IN  GT_U32      eighthOctetToChange,
    IN  GT_U32      ninthOctetToChange,
    IN  GT_U32      tenthOctetToChange,
    IN  GT_U32      elevenOctetToChange,
    IN  GT_U32      twelveOctetToChange,
    IN  GT_U32      thirteenOctetToChange,
    IN  GT_U32      fourteenOctetToChange,
    IN  GT_U32      fifteenOctetToChange,
    IN  GT_U32      sixteenOctetToChange,
    OUT GT_IPV6ADDR *lastIpAddrDeletedPtr,
    OUT GT_U32      *numOfPrefixesDeletedPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPV6ADDR   lastIpAddrDeleted;
    GT_U32      numOfPrefixesDeleted;

#ifdef CHX_FAMILY
     rc = cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet(lpmDbId, vrId, startIpAddr, numOfPrefixesToDel, firstOctetToChange,secondOctetToChange,
                                                     thirdOctetToChange, forthOctetToChange, fifthOctetToChange, sixthOctetToChange,
                                                     seventhOctetToChange, eighthOctetToChange, ninthOctetToChange, tenthOctetToChange,
                                                     elevenOctetToChange, twelveOctetToChange, thirteenOctetToChange, fourteenOctetToChange,
                                                     fifteenOctetToChange, sixteenOctetToChange,&lastIpAddrDeleted,&numOfPrefixesDeleted);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgIpv6UcPrefixDelManyByOctet FAILED, rc = [%d]", rc);

        return rc;
    }
    else
    {
        *lastIpAddrDeletedPtr = lastIpAddrDeleted;
        *numOfPrefixesDeletedPtr= numOfPrefixesDeleted;
    }

#else /* ! CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    TGF_PARAM_NOT_USED(vrId);
    TGF_PARAM_NOT_USED(startIpAddr);
    TGF_PARAM_NOT_USED(numOfPrefixesToDel);
    TGF_PARAM_NOT_USED(firstOctetToChange);
    TGF_PARAM_NOT_USED(secondOctetToChange);
    TGF_PARAM_NOT_USED(thirdOctetToChange);
    TGF_PARAM_NOT_USED(forthOctetToChange);
    TGF_PARAM_NOT_USED(fifthOctetToChange);
    TGF_PARAM_NOT_USED(sixthOctetToChange);
    TGF_PARAM_NOT_USED(seventhOctetToChange);
    TGF_PARAM_NOT_USED(eighthOctetToChange);
    TGF_PARAM_NOT_USED(ninthOctetToChange);
    TGF_PARAM_NOT_USED(tenthOctetToChange);
    TGF_PARAM_NOT_USED(elevenOctetToChange);
    TGF_PARAM_NOT_USED(twelveOctetToChange);
    TGF_PARAM_NOT_USED(thirteenOctetToChange);
    TGF_PARAM_NOT_USED(fourteenOctetToChange);
    TGF_PARAM_NOT_USED(fifteenOctetToChange);
    TGF_PARAM_NOT_USED(sixteenOctetToChange);
    TGF_PARAM_NOT_USED(lastIpAddrDeletedPtr);
    TGF_PARAM_NOT_USED(numOfPrefixesDeletedPtr);
    return GT_BAD_STATE;
#endif /* CHX_FAMILY */

    return rc;
}


/**
* @internal prvTgfIpLpmIpv4UcPrefixBulkAdd function
* @endinternal
*
* @brief   Add bulk of IPv4 UC prefixes
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on prefix length is too big
* @retval GT_ERROR                 - on the vrId was not created yet
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixBulkAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                *vrId,
    IN GT_IPADDR                             *ipAddr,
    IN GT_U32                                *prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    IN GT_BOOL                               *override,
    IN GT_U32                                sizeOfBulk
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       i;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *dxChNextHopInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    PRV_TGF_PCL_ACTION_STC                *pclIpUcActionPtr;
    PRV_TGF_IP_LTT_ENTRY_STC              *ipLttEntryPtr;
    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC   *ipPrefixArrayPtr;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC *exMxPmNextHopInfo;
    CPSS_EXMXPM_IP_LPM_IPV4_UC_ADD_PREFIX_STC *ipPrefixArrayPtr;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]\n", rc);

        return rc;
    }

    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    cpssOsMemSet(ipPrefixArrayPtr,0,sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));


    dxChNextHopInfo = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    if (dxChNextHopInfo == NULL)
    {
        cpssOsFree(ipPrefixArrayPtr);
        return GT_OUT_OF_CPU_MEM;
    }

    cpssOsMemSet(dxChNextHopInfo,0,sizeOfBulk * sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        switch(routingMode)
        {
            case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

                pclIpUcActionPtr = &nextHopInfoPtr[i].pclIpUcAction;

                rc = prvTgfConvertGenericToDxChRuleAction(pclIpUcActionPtr, &dxChNextHopInfo[i].pclIpUcAction);
                if (GT_OK != rc)
                {
                    cpssOsFree(ipPrefixArrayPtr);
                    cpssOsFree(dxChNextHopInfo);
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);
                    return rc;
                }

                break;

            case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

                ipLttEntryPtr = &nextHopInfoPtr[i].ipLttEntry;

                /* set route type */
                rc = prvTgfConvertGenericToDxChRouteType(ipLttEntryPtr->routeType, &(dxChNextHopInfo[i].ipLttEntry.routeType));
                if (GT_OK != rc)
                {
                    cpssOsFree(ipPrefixArrayPtr);
                    cpssOsFree(dxChNextHopInfo);
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);
                    return rc;
                }

                /* convert ltt entry into device specific format */
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, numOfPaths);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, routeEntryBaseIndex);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, ucRPFCheckEnable);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, sipSaCheckMismatchEnable);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, ipv6MCGroupScopeLevel);

                break;

            default:
                cpssOsFree(ipPrefixArrayPtr);
                cpssOsFree(dxChNextHopInfo);
                return GT_BAD_PARAM;
        }
        cpssOsMemCpy(&ipPrefixArrayPtr[i].nextHopInfo,
                     &dxChNextHopInfo[i],
                     sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        ipPrefixArrayPtr[i].override = override[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, sizeOfBulk, ipPrefixArrayPtr);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        cpssOsFree(dxChNextHopInfo);
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4UcPrefixAddBulk FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    cpssOsFree(dxChNextHopInfo);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* call device specific API */
    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_EXMXPM_IP_LPM_IPV4_UC_ADD_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }
    exMxPmNextHopInfo = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC));
    if (exMxPmNextHopInfo == NULL)
    {
        cpssOsFree(ipPrefixArrayPtr);
        return GT_OUT_OF_CPU_MEM;
    }
    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        rc = prvTgfConvertGenericToExMxPmRoutePointer(&nextHopInfoPtr[i].ipLttEntry,
                                                      &exMxPmNextHopInfo[i]);
        if (GT_OK != rc)
        {
            cpssOsFree(ipPrefixArrayPtr);
            cpssOsFree(exMxPmNextHopInfo);
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRoutePointer FAILED, rc = [%d]", rc);
            return rc;
        }
        cpssOsMemCpy(&ipPrefixArrayPtr[i].nextHopPointer,
                     &exMxPmNextHopInfo[i],
                     sizeof(CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC));
        ipPrefixArrayPtr[i].override = override[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv4UcPrefixBulkAdd(lpmDBId, sizeOfBulk, ipPrefixArrayPtr);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        cpssOsFree(exMxPmNextHopInfo);
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpLpmIpv4UcPrefixBulkAdd FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    cpssOsFree(exMxPmNextHopInfo);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E,GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv4UcPrefixBulkDel function
* @endinternal
*
* @brief   Deletes an existing IPv4 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv4UcPrefixBulkDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         *vrId,
    IN GT_IPADDR                      *ipAddr,
    IN GT_U32                         *prefixLen,
    IN GT_U32                         sizeOfBulk
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC   *ipPrefixArrayPtr;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_LPM_IPV4_UC_DELETE_PREFIX_STCT *ipPrefixArrayPtr;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    cpssOsMemSet(ipPrefixArrayPtr,0,sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC));

    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, sizeOfBulk, ipPrefixArrayPtr);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv4UcPrefixDelBulk FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;


#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_EXMXPM_IP_LPM_IPV4_UC_ADD_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc =  cpssExMxPmIpLpmIpv4UcPrefixBulkDelete(lpmDBId, sizeOfBulk, ipPrefixArrayPtr);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpLpmIpv4UcPrefixBulkDelete FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E,GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixBulkAdd function
* @endinternal
*
* @brief   Add bulk of IPv6 UC prefixes
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of this prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] nextHopInfoPtr           - the route entry info accosiated with this UC prefix
* @param[in] override                 -  an existing entry for this mask
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there is no
*                                      place to insert the prefix. To point of the process is
*                                      just to make space for this prefix.
*                                      relevant only if the LPM DB was created with
*                                      partitionEnable = GT_FALSE.
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on prefix length is too big
* @retval GT_ERROR                 - on the vrId was not created yet
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - on failed to allocate TCAM memory
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixBulkAdd
(
    IN GT_U32                                lpmDBId,
    IN GT_U32                                *vrId,
    IN GT_IPV6ADDR                           *ipAddr,
    IN GT_U32                                *prefixLen,
    IN PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  *nextHopInfoPtr,
    IN GT_BOOL                               *override,
    IN GT_BOOL                               defragmentationEnable,
    IN GT_U32                                sizeOfBulk
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       i;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *dxChNextHopInfo;
    PRV_TGF_IP_ROUTING_MODE_ENT            routingMode;
    PRV_TGF_PCL_ACTION_STC                *pclIpUcActionPtr;
    PRV_TGF_IP_LTT_ENTRY_STC              *ipLttEntryPtr;
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC   *ipPrefixArrayPtr;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC *exMxPmNextHopInfo;
    CPSS_EXMXPM_IP_LPM_IPV6_UC_ADD_PREFIX_STC *ipPrefixArrayPtr;
#endif /* EXMXPM_FAMILY */


#ifdef CHX_FAMILY
    rc = prvTgfIpRoutingModeGet(&routingMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpRoutingModeGet FAILED, rc = [%d]\n", rc);

        return rc;
    }

    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }
    dxChNextHopInfo = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    if (dxChNextHopInfo == NULL)
    {
        cpssOsFree(ipPrefixArrayPtr);
        return GT_OUT_OF_CPU_MEM;
    }
    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPV6ADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        switch(routingMode)
        {
            case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

                pclIpUcActionPtr = &nextHopInfoPtr[i].pclIpUcAction;

                rc = prvTgfConvertGenericToDxChRuleAction(pclIpUcActionPtr, &dxChNextHopInfo[i].pclIpUcAction);
                if (GT_OK != rc)
                {
                    cpssOsFree(ipPrefixArrayPtr);
                    cpssOsFree(dxChNextHopInfo);
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);
                    return rc;
                }

                break;

            case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

                ipLttEntryPtr = &nextHopInfoPtr[i].ipLttEntry;

                /* set route type */
                rc = prvTgfConvertGenericToDxChRouteType(ipLttEntryPtr->routeType, &(dxChNextHopInfo[i].ipLttEntry.routeType));
                if (GT_OK != rc)
                {
                    cpssOsFree(ipPrefixArrayPtr);
                    cpssOsFree(dxChNextHopInfo);
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRouteType FAILED, rc = [%d]", rc);
                    return rc;
                }

                /* convert ltt entry into device specific format */
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, numOfPaths);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, routeEntryBaseIndex);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, ucRPFCheckEnable);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, sipSaCheckMismatchEnable);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, ipv6MCGroupScopeLevel);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChNextHopInfo[i].ipLttEntry), ipLttEntryPtr, priority);

                break;

            default:
                cpssOsFree(ipPrefixArrayPtr);
                cpssOsFree(dxChNextHopInfo);
                return GT_BAD_PARAM;
        }
        cpssOsMemCpy(&ipPrefixArrayPtr[i].nextHopInfo,
                     &dxChNextHopInfo[i],
                     sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        ipPrefixArrayPtr[i].override = override[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, sizeOfBulk, ipPrefixArrayPtr, defragmentationEnable);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        cpssOsFree(dxChNextHopInfo);
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv6UcPrefixAddBulk FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    cpssOsFree(dxChNextHopInfo);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* prevent compilation warning */
    TGF_PARAM_NOT_USED(defragmentationEnable);
    /* call device specific API */
    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_EXMXPM_IP_LPM_IPV6_UC_ADD_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }
    exMxPmNextHopInfo = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC));
    if (exMxPmNextHopInfo == NULL)
    {
        cpssOsFree(ipPrefixArrayPtr);
        return GT_OUT_OF_CPU_MEM;
    }
    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPV6ADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        rc = prvTgfConvertGenericToExMxPmRoutePointer(&nextHopInfoPtr[i].ipLttEntry,
                                                      &exMxPmNextHopInfo[i]);
        if (GT_OK != rc)
        {
            cpssOsFree(ipPrefixArrayPtr);
            cpssOsFree(exMxPmNextHopInfo);
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToExMxPmRoutePointer FAILED, rc = [%d]", rc);
            return rc;
        }
        cpssOsMemCpy(&ipPrefixArrayPtr[i].nextHopPointer,
                     &exMxPmNextHopInfo[i],
                     sizeof(CPSS_EXMXPM_IP_ROUTE_ENTRY_POINTER_STC));
        ipPrefixArrayPtr[i].override = override[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc = cpssExMxPmIpLpmIpv6UcPrefixBulkAdd(lpmDBId, sizeOfBulk, ipPrefixArrayPtr);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        cpssOsFree(exMxPmNextHopInfo);
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpLpmIpv6UcPrefixBulkAdd FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    cpssOsFree(exMxPmNextHopInfo);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E,GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpLpmIpv6UcPrefixBulkDel function
* @endinternal
*
* @brief   Deletes an existing IPv6 prefix in a Virtual Router for the specified LPM DB
*
* @param[in] lpmDBId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] ipAddr                   - The destination IP address of the prefix
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr
* @param[in] sizeOfBulk               - number of prefixes in the bulk
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - prefix length is too big
* @retval GT_ERROR                 - vrId was not created yet
* @retval GT_NO_SUCH               - given prefix doesn't exitst in the VR
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvTgfIpLpmIpv6UcPrefixBulkDel
(
    IN GT_U32                         lpmDBId,
    IN GT_U32                         *vrId,
    IN GT_IPV6ADDR                    *ipAddr,
    IN GT_U32                         *prefixLen,
    IN GT_U32                         sizeOfBulk
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i;
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC   *ipPrefixArrayPtr;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    CPSS_EXMXPM_IP_LPM_IPV6_UC_DELETE_PREFIX_STC *ipPrefixArrayPtr;
#endif /* EXMXPM_FAMILY */

#ifdef CHX_FAMILY
    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPV6ADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc =  cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, sizeOfBulk, ipPrefixArrayPtr);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmIpv6UcPrefixDelBulk FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;


#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    ipPrefixArrayPtr = cpssOsMalloc(sizeOfBulk * sizeof(CPSS_EXMXPM_IP_LPM_IPV6_UC_ADD_PREFIX_STC));
    if (ipPrefixArrayPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    for (i = 0; i < sizeOfBulk; i++)
    {
        cpssOsMemCpy(&ipPrefixArrayPtr[i].ipAddr, &ipAddr[i], sizeof(GT_IPV6ADDR));
        ipPrefixArrayPtr[i].vrId = vrId[i];
        ipPrefixArrayPtr[i].prefixLen = prefixLen[i];
        ipPrefixArrayPtr[i].returnStatus = GT_OK;
    }
    /* call device specific API */
    rc =  cpssExMxPmIpLpmIpv6UcPrefixBulkDelete(lpmDBId, sizeOfBulk, ipPrefixArrayPtr);
    if (GT_OK != rc)
    {
        cpssOsFree(ipPrefixArrayPtr);
        PRV_UTF_LOG1_MAC("[TGF]: cpssExMxPmIpLpmIpv6UcPrefixBulkDelete FAILED, rc = [%d]", rc);
        return rc;
    }
    cpssOsFree(ipPrefixArrayPtr);
    for (i = 0; i < sizeOfBulk; i++)
    {
        rc = prvTgfIpLpmValidityCheck(lpmDBId,vrId[i],CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E,GT_TRUE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmValidityCheck FAILED, rc = [%d]", rc);
        }
    }
    return rc;

#endif /* EXMXPM_FAMILY */

#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}



/**
* @internal prvTgfIpv4PbrConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*
* @param[in] pbrConfigPtr             - pbr configuration structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfIpv4PbrConfigurationSet
(
    IN PRV_TGF_IP_ROUTE_CNFG_STC *pbrConfigPtr
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_ETHERADDR                            arpMacAddr;

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");

    cpssOsMemSet(&nextHopInfo,0,sizeof(nextHopInfo));

    /* -------------------------------------------------------------------------
     * 1. PCL Config
     */

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[pbrConfigPtr->sendPortNum],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_1_E,/* PBR must be in lookup 1 ! */
        PRV_TGF_PCL_PBR_ID_FOR_VIRTUAL_ROUTER_CNS, /* pclId */
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfPclDefPortInitExt1 FAILED, rc = [%d],devNum = [%d]", rc,pbrConfigPtr->devNum);
        return rc;
    }

    /* -------------------------------------------------------------------------
     * 2. IP Config
     */

    /* enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(pbrConfigPtr->sendVlanId, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("[TGF]: prvTgfIpVlanRoutingEnable FAILED, rc = [%d],devNum = [%d], portNum = [%d]", rc,pbrConfigPtr->devNum,prvTgfPortsArray[pbrConfigPtr->sendPortNum]);
        return rc;
    }

    if (pbrConfigPtr->isArpNeeded == GT_TRUE)
    {
        /* write ARP MAC address to the Router ARP Table */
        cpssOsMemCpy(arpMacAddr.arEther, pbrConfigPtr->prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        rc = prvTgfIpRouterArpAddrWrite(pbrConfigPtr->routerArpIndex, &arpMacAddr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpRouterArpAddrWrite FAILED, rc = [%d],devNum = [%d]", rc,pbrConfigPtr->devNum);
            return rc;
        }
    }

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = CPSS_NET_CONTROL_E;
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = pbrConfigPtr->nextHopVlanId;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = pbrConfigPtr->devNum;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[pbrConfigPtr->nextHopPortNum];
    if (pbrConfigPtr->isTunnelStart == GT_TRUE)
    {
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outlifType = PRV_TGF_OUTLIF_TYPE_TUNNEL_E;
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outlifPointer.tunnelStartPtr.tunnelType = PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E;
        nextHopInfo.pclIpUcAction.redirect.data.outIf.outlifPointer.tunnelStartPtr.ptr = pbrConfigPtr->tunnelStartIndex;
    }
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, pbrConfigPtr->dstIpAddr, sizeof(ipAddr.arIP));

    /* call CPSS function */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(pbrConfigPtr->lpmDbId,0, ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpLpmIpv4UcPrefixAdd FAILED, rc = [%d],devNum = [%d]", rc,pbrConfigPtr->devNum);
    }
    return rc;
}


/**
* @internal prvTgfIpv4LttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] lttConfigPtr             - ltt routing configuration structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - on general failure error
*/
GT_STATUS prvTgfIpv4LttRouteConfigurationSet
(
    IN PRV_TGF_IP_ROUTE_CNFG_STC *lttConfigPtr
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_INLIF_ENTRY_STC                 inlifEntry;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_ETHERADDR                            arpMacAddr;


    cpssOsMemSet(&inlifEntry,0,sizeof(inlifEntry));
    cpssOsMemSet(&nextHopInfo,0,sizeof(nextHopInfo));

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 0. Set Inlif Entry - that needs for Puma devices
     */

    /* Bridge Section */
    inlifEntry.bridgeEnable              = GT_TRUE;
    inlifEntry.autoLearnEnable           = GT_TRUE;
    inlifEntry.naMessageToCpuEnable      = GT_TRUE;
    inlifEntry.naStormPreventionEnable   = GT_FALSE;
    inlifEntry.unkSaUcCommand            = CPSS_PACKET_CMD_BRIDGE_E;
    inlifEntry.unkDaUcCommand            = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unkSaNotSecurBreachEnable = GT_TRUE;
    inlifEntry.untaggedMruIndex          = 0;
    inlifEntry.unregNonIpMcCommand       = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpMcCommand          = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregIpv4BcCommand        = CPSS_PACKET_CMD_FORWARD_E;
    inlifEntry.unregNonIpv4BcCommand     = CPSS_PACKET_CMD_FORWARD_E;

    /* Router Section */
    inlifEntry.ipv4UcRouteEnable = GT_TRUE;
    inlifEntry.ipv4McRouteEnable = GT_FALSE;
    inlifEntry.ipv6UcRouteEnable = GT_FALSE;
    inlifEntry.ipv6McRouteEnable = GT_FALSE;
    inlifEntry.mplsRouteEnable   = GT_FALSE;
    inlifEntry.vrfId             = 0;

    /* SCT Section */
    inlifEntry.ipv4IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.ipv6IcmpRedirectEnable         = GT_FALSE;
    inlifEntry.bridgeRouterInterfaceEnable    = GT_FALSE;
    inlifEntry.ipSecurityProfile              = 0;
    inlifEntry.ipv4IgmpToCpuEnable            = GT_FALSE;
    inlifEntry.ipv6IcmpToCpuEnable            = GT_FALSE;
    inlifEntry.udpBcRelayEnable               = GT_FALSE;
    inlifEntry.arpBcToCpuEnable               = GT_FALSE;
    inlifEntry.arpBcToMeEnable                = GT_FALSE;
    inlifEntry.ripv1MirrorEnable              = GT_FALSE;
    inlifEntry.ipv4LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6LinkLocalMcCommandEnable   = GT_FALSE;
    inlifEntry.ipv6NeighborSolicitationEnable = GT_FALSE;

    /* Misc Section */
    inlifEntry.mirrorToAnalyzerEnable = GT_FALSE;
    inlifEntry.mirrorToCpuEnable      = GT_FALSE;

    /* call api function */
    rc = prvTgfInlifEntrySet(PRV_TGF_INLIF_TYPE_EXTERNAL_E, 4096, &inlifEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfInlifEntrySet FAILED, rc = [%d],devNum = [%d]", rc,lttConfigPtr->devNum);
        return rc;
    }

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(lttConfigPtr->sendPortNum, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("[TGF]: prvTgfIpPortRoutingEnable FAILED, rc = [%d],devNum = [%d], portNum = [%d]",
                          rc,lttConfigPtr->devNum,prvTgfPortsArray[lttConfigPtr->sendPortNum]);
        return rc;
    }

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(lttConfigPtr->sendVlanId, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG3_MAC("[TGF]: prvTgfIpPortRoutingEnable FAILED, rc = [%d],devNum = [%d], portNum = [%d]",
                          rc,lttConfigPtr->devNum,prvTgfPortsArray[lttConfigPtr->sendPortNum]);
        return rc;
    }

    if (lttConfigPtr->isArpNeeded == GT_TRUE)
    {
        /* write ARP MAC address to the Router ARP Table */
        cpssOsMemCpy(arpMacAddr.arEther, lttConfigPtr->prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        rc = prvTgfIpRouterArpAddrWrite(lttConfigPtr->routerArpIndex, &arpMacAddr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpRouterArpAddrWrite FAILED, rc = [%d],devNum = [%d]", rc,lttConfigPtr->devNum);
            return rc;
        }
    }

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = lttConfigPtr->countSet;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = lttConfigPtr->isTunnelStart;
    regularEntryPtr->nextHopVlanId              = lttConfigPtr->nextHopVlanId;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = lttConfigPtr->devNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[lttConfigPtr->nextHopPortNum];
    regularEntryPtr->nextHopARPPointer          = lttConfigPtr->routerArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = lttConfigPtr->tunnelStartIndex;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(lttConfigPtr->routeEntryBaseIndex, routeEntriesArray, 1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpUcRouteEntriesWrite FAILED, rc = [%d],devNum = [%d]", rc,lttConfigPtr->devNum);
        return rc;
    }

    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(lttConfigPtr->devNum, lttConfigPtr->routeEntryBaseIndex, routeEntriesArray, 1);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpUcRouteEntriesRead FAILED, rc = [%d],devNum = [%d]", rc,lttConfigPtr->devNum);
        return rc;
    }

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = lttConfigPtr->routeEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, lttConfigPtr->dstIpAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(lttConfigPtr->lpmDbId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfIpLpmIpv4UcPrefixAdd FAILED, rc = [%d],devNum = [%d]", rc,lttConfigPtr->devNum);
    }

    return rc;
}

/**
* @internal prvTgfIpHeaderErrorMaskSet function
* @endinternal
*
* @brief   Mask or unmask an IP header error.
*
* @param[in] devNum                   - the device number
* @param[in] ipHeaderErrorType        - the IP header error type
* @param[in] protocolStack            - whether to mask/unmask the error for IPv4 or IPv6
* @param[in] prefixType               - whether to mask/unmask the error for unicast or
*                                      multicast
* @param[in] mask                     - GT_TRUE:  the error
*                                      GT_FALSE: unmask the error
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note If the error is masked, then an IPv4/6 UC/MC header exception will not
*       be triggered for the relevant header error.
*
*/
GT_STATUS prvTgfIpHeaderErrorMaskSet
(
    IN GT_U8                                devNum,
    IN PRV_TGF_IP_HEADER_ERROR_ENT          ipHeaderErrorType,
    IN CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT           prefixType,
    IN GT_BOOL                              mask
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_HEADER_ERROR_ENT           dxChIpHeaderErrorType;
    GT_STATUS                               rc;

    /* convert TTI header error type into device specific format */
    rc = prvTgfConvertGenericToDxChIpHeaderError(ipHeaderErrorType, &dxChIpHeaderErrorType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    return cpssDxChIpHeaderErrorMaskSet(devNum, dxChIpHeaderErrorType, protocolStack, prefixType, mask);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ipHeaderErrorType);
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(prefixType);
    TGF_PARAM_NOT_USED(mask);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpHeaderErrorMaskGet function
* @endinternal
*
* @brief   Mask or unmask an IP header error.
*
* @param[in] devNum                   - the device number
* @param[in] ipHeaderErrorType        - the IP header error type
* @param[in] protocolStack            - whether to mask/unmask the error for IPv4 or IPv6
* @param[in] prefixType               - whether to mask/unmask the error for unicast or
*                                      multicast
*
* @param[out] maskPtr                  - GT_TRUE: mask the error
*                                      GT_FALSE: unmask the error
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
*
* @note If the error is masked, then an IPv4/6 UC/MC header exception will not
*       be triggered for the relevant header error.
*
*/
GT_STATUS prvTgfIpHeaderErrorMaskGet
(
    IN  GT_U8                               devNum,
    IN  PRV_TGF_IP_HEADER_ERROR_ENT         ipHeaderErrorType,
    IN  CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    IN  CPSS_UNICAST_MULTICAST_ENT          prefixType,
    OUT GT_BOOL                             *maskPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_HEADER_ERROR_ENT           dxChIpHeaderErrorType;
    GT_STATUS                               rc;

    /* convert TTI header error type into device specific format */
    rc = prvTgfConvertGenericToDxChIpHeaderError(ipHeaderErrorType, &dxChIpHeaderErrorType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    return cpssDxChIpHeaderErrorMaskGet(devNum, dxChIpHeaderErrorType, protocolStack, prefixType, maskPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ipHeaderErrorType);
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(prefixType);
    TGF_PARAM_NOT_USED(maskPtr);
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet function
* @endinternal
*
* @brief   Sets device ID modification for Routed packets.
*         Enables/Disables FORWARD DSA tag modification of the <source device>
*         and <source port> fields of packets routed by the local device.
*         The <source device> is set to the local device ID and the <source port>
*         is set to 61 (the virtual router port).
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Port number (or CPU port)
* @param[in] modifyEnable             - Boolean value of the FORWARD DSA tag modification:
*                                      GT_TRUE  -  Device ID Modification is Enabled.
*                                      GT_FALSE -  Device ID Modification is Disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 modifyEnable
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
    if(!PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        PRV_UTF_LOG3_MAC("prvTgfCfgDsaTagSrcDevPortRoutedPcktModifySet : (%d,%d,%d) =======\n" ,
                            devNum ,
                            portNum,
                            modifyEnable);
        rc = cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(devNum,portNum,modifyEnable);
    }

    return rc;
#else
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(portNum);
    TGF_PARAM_NOT_USED(modifyEnable);
    return GT_NOT_IMPLEMENTED;
#endif

}

/**
* @internal prvTgfIpArpBcModeSet function
* @endinternal
*
* @brief   set a arp broadcast mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpArpBcModeSet
(
    IN CPSS_PACKET_CMD_ENT arpBcMode
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpArpBcModeSet(devNum, arpBcMode);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChIpArpBcModeSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(arpBcMode);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpArpBcModeGet function
* @endinternal
*
* @brief   get a arp broadcast mode.
*
* @param[out] arpBcModePtr             - the arp broadcast command. Possible Commands:
*                                      CPSS_PACKET_CMD_NONE_E,CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note
*       GalTis:
*
*/
GT_STATUS prvTgfIpArpBcModeGet
(
    OUT CPSS_PACKET_CMD_ENT *arpBcModePtr
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = prvTgfDevNum;
    return cpssDxChIpArpBcModeGet(devNum, arpBcModePtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(arpBcModePtr);
    /* call device specific API */
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */
#if !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) && !(defined EXMXPM_FAMILY) */
}

/**
* @internal prvTgfIpEcmpEntryWrite function
* @endinternal
*
* @brief   Write an ECMP entry
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
GT_STATUS prvTgfIpEcmpEntryWrite
(
    IN GT_U8                        devNum,
    IN GT_U32                       ecmpEntryIndex,
    IN PRV_TGF_IP_ECMP_ENTRY_STC    *ecmpEntryPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_ECMP_ENTRY_STC cpssEcmpEntry;

    cpssOsMemSet(&cpssEcmpEntry,0,sizeof(cpssEcmpEntry));

    cpssEcmpEntry.numOfPaths = ecmpEntryPtr->numOfPaths;
    cpssEcmpEntry.randomEnable = ecmpEntryPtr->randomEnable;
    cpssEcmpEntry.routeEntryBaseIndex = ecmpEntryPtr->routeEntryBaseIndex;
    switch (ecmpEntryPtr->multiPathMode) {
    case PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E:
        cpssEcmpEntry.multiPathMode = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        break;
    case PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E:
        cpssEcmpEntry.multiPathMode = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChIpEcmpEntryWrite(devNum, ecmpEntryIndex, &cpssEcmpEntry);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ecmpEntryIndex);
    TGF_PARAM_NOT_USED(ecmpEntryPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpEcmpEntryRead function
* @endinternal
*
* @brief   Read an ECMP entry
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
GT_STATUS prvTgfIpEcmpEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       ecmpEntryIndex,
    OUT PRV_TGF_IP_ECMP_ENTRY_STC    *ecmpEntryPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_ECMP_ENTRY_STC cpssEcmpEntry;
    GT_STATUS                   rc;

    cpssOsMemSet(&cpssEcmpEntry,0,sizeof(cpssEcmpEntry));

    /* call device specific API */
    rc = cpssDxChIpEcmpEntryRead(devNum, ecmpEntryIndex, &cpssEcmpEntry);
    if (rc == GT_OK)
    {
        ecmpEntryPtr->numOfPaths = cpssEcmpEntry.numOfPaths;
        ecmpEntryPtr->randomEnable = cpssEcmpEntry.randomEnable;
        ecmpEntryPtr->routeEntryBaseIndex = cpssEcmpEntry.routeEntryBaseIndex;
        switch (cpssEcmpEntry.multiPathMode) {
        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E:
            ecmpEntryPtr->multiPathMode = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
            break;
        case CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E:
            ecmpEntryPtr->multiPathMode = PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E;
            break;
        default:
            return GT_BAD_STATE;
        }
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(ecmpEntryIndex);
    TGF_PARAM_NOT_USED(ecmpEntryPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpRouterSourceIdSet function
* @endinternal
*
* @brief   set the router source id assignment
*
* @param[in] ucMcSet                  - whether to set it for unicast packets or multicast.
* @param[in] sourceId                 - the assigned source id.
* @param[in] sourceIdMask             - the assigned source id mask,
*                                      relevant for ucMcSet CPSS_IP_MULTICAST_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterSourceIdSet
(
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    IN  GT_U32                          sourceId,
    IN  GT_U32                          sourceIdMask
)
{
#ifdef CHX_FAMILY
    GT_U8       devNum  = 0;
    GT_STATUS   rc, rc1 = GT_OK;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RC_RESET_MAC(&devNum, rc, UTF_NONE_FAMILY_E);

    /* go over all active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* call device specific API */
        rc = cpssDxChIpRouterSourceIdSet(devNum, ucMcSet, sourceId, sourceIdMask);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC(
              "[TGF]: cpssDxChIpRouterSourceIdSet FAILED, rc = [%d]", rc);
            rc1 = rc;
        }
    }
    return rc1;
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(ucMcSet);
    TGF_PARAM_NOT_USED(sourceId);
    TGF_PARAM_NOT_USED(sourceIdMask);
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfIpRouterSourceIdGet function
* @endinternal
*
* @brief   get the router source id assignment
*
* @param[in] ucMcSet                  - whether to set it for unicast packets or multicast.
*
* @param[out] sourceIdPtr              - the assigned source id.
* @param[out] sourceIdMaskPtr          - the assigned source id mask,
*                                      relevant for ucMcSet CPSS_IP_MULTICAST_E.
*                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_INITIALIZED       - The library was not initialized.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpRouterSourceIdGet
(
    IN  CPSS_IP_UNICAST_MULTICAST_ENT   ucMcSet,
    OUT GT_U32                          *sourceIdPtr,
    OUT GT_U32                          *sourceIdMaskPtr
)
{
#ifdef CHX_FAMILY
    return cpssDxChIpRouterSourceIdGet(prvTgfDevNum, ucMcSet, sourceIdPtr, sourceIdMaskPtr);
#endif /* CHX_FAMILY */

#ifdef EXMXPM_FAMILY
    /* avoid warnings */
    TGF_PARAM_NOT_USED(ucMcSet);
    TGF_PARAM_NOT_USED(sourceIdPtr);
    TGF_PARAM_NOT_USED(sourceIdMaskPtr);
    return GT_NOT_SUPPORTED;
#endif /* EXMXPM_FAMILY */

#if (!defined CHX_FAMILY) && (!defined EXMXPM_FAMILY)
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfLpmLeafEntryWrite function
* @endinternal
*
* @brief   Write an LPM leaf entry, which is used for policy based routing, to the
*         HW
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
* @param[in] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
GT_STATUS prvTgfLpmLeafEntryWrite
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   leafIndex,
    IN PRV_TGF_LPM_LEAF_ENTRY_STC               *leafPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_LPM_LEAF_ENTRY_STC    leafEntry;

    cpssOsMemSet(&leafEntry,0,sizeof(leafEntry));

    /* call device specific API */
    switch (leafPtr->entryType)
    {
        case PRV_TGF_LPM_REGULAR_NODE_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
            break;
        case PRV_TGF_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
            break;
        case PRV_TGF_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E;
            break;
        case PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
            break;
        case PRV_TGF_LPM_ECMP_ENTRY_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E;
            break;
        case PRV_TGF_LPM_QOS_ENTRY_PTR_TYPE_E:
            leafEntry.entryType = CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E;
            break;
        default:
            return GT_BAD_PARAM;
    }
    leafEntry.index = leafPtr->index;
    leafEntry.ucRPFCheckEnable = leafPtr->ucRPFCheckEnable;
    leafEntry.sipSaCheckMismatchEnable = leafPtr->sipSaCheckMismatchEnable;
    leafEntry.ipv6MCGroupScopeLevel = leafPtr->ipv6MCGroupScopeLevel;
    leafEntry.priority = leafPtr->priority;
    return cpssDxChLpmLeafEntryWrite(devNum, leafIndex, &leafEntry);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(leafIndex);
    TGF_PARAM_NOT_USED(leafPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfLpmLeafEntryRead function
* @endinternal
*
* @brief   Read an LPM leaf entry, which is used for policy based routing, from the
*         HW
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
*
* @param[out] leafPtr                  - (pointer to) the leaf entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
GT_STATUS prvTgfLpmLeafEntryRead
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   leafIndex,
    OUT PRV_TGF_LPM_LEAF_ENTRY_STC              *leafPtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_LPM_LEAF_ENTRY_STC    leafEntry;
    GT_STATUS                   rc;

    /* call device specific API */
    rc = cpssDxChLpmLeafEntryRead(devNum, leafIndex, &leafEntry);
    if (rc == GT_OK)
    {
        switch (leafEntry.entryType)
        {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_REGULAR_NODE_PTR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_COMPRESSED_2_NODE_PTR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_ECMP_ENTRY_PTR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
            leafPtr->entryType = PRV_TGF_LPM_QOS_ENTRY_PTR_TYPE_E;
            break;
        default:
            return GT_BAD_PARAM;
        }

        leafPtr->index = leafEntry.index;
        leafPtr->ucRPFCheckEnable = leafEntry.ucRPFCheckEnable;
        leafPtr->sipSaCheckMismatchEnable = leafEntry.sipSaCheckMismatchEnable;
        leafPtr->ipv6MCGroupScopeLevel = leafEntry.ipv6MCGroupScopeLevel;
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(leafIndex);
    TGF_PARAM_NOT_USED(leafPtr);
    return GT_BAD_STATE;
#endif
}
/**
* @internal prvTgfIpMllMultiTargetShaperBaselineSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline,
*         the respective packet is not served.
* @param[in] devNum                   - device number.
* @param[in] baseline                 - Token Bucket Baseline value in bytes
*                                      (APPLICABLE RANGES: 0..0xFFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range baseline
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Token Bucket Baseline must be configured as follows:
*       1. At least MTU (the maximum expected packet size in the system).
*       2. When packet based shaping is enabled, the following used as
*       shaper's MTU:
*       cpssDxChIpMllMultiTargetShaperMtuSet.
*
*/
GT_STATUS   prvTgfIpMllMultiTargetShaperBaselineSet
(
    IN GT_U8    devNum,
    IN GT_U32   baseline
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperBaselineSet(devNum, baseline);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(baseline);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperBaselineGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Token Bucket Baseline.
*         The Token Bucket Baseline is the "zero" level of the token bucket.
*         When the token bucket fill level < Baseline, the respective packet
*         is not served.
* @param[in] devNum                   - device number.
*
* @param[out] baselinePtr              - (pointer to) Token Bucket Baseline value in bytes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperBaselineGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *baselinePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperBaselineGet(devNum, baselinePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(baselinePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperMtuSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set the packet length in bytes for updating the shaper token bucket.
*         Valid when <Token Bucket Mode>=Packets.
* @param[in] devNum                   - physical device number
* @param[in] mtu                      - MTU for egress rate shaper
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, mtu
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note <Token Bucket Mode> is configured in cpssDxChIpMllMultiTargetShaperConfigurationSet
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperMtuSet
(
    IN GT_U8   devNum,
    IN GT_U32  mtu
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperMtuSet(devNum, mtu);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mtu);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperMtuGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get the packet length in bytes for updating the shaper token bucket.
*         Valid when <Token Bucket Mode>=Packets.
* @param[in] devNum                   - physical device number
*
* @param[out] mtuPtr                   -  pointer to MTU for egress rate shaper
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note <Token Bucket Mode> is configured in cpssDxChIpMllMultiTargetShaperConfigurationSet
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperMtuGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *mtuPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperMtuGet(devNum, mtuPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mtuPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperTokenBucketModeSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set Token Bucket Mode Mll shaper.
* @param[in] devNum                   - device number.
* @param[in] tokenBucketMode          -   Defines the packet length used to decrease the shaper token bucket.
*                                      In Byte mode, the token bucket rate shaper is decreased according to the
*                                      packet length as received by the switch.
*                                      In Packet mode, the token bucket rate shaper is decreased by the packet
*                                      length specified in cpssDxChIpMllMultiTargetShaperMtuSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperTokenBucketModeSet
(
    IN  GT_U8                                        devNum,
    IN  CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            tokenBucketMode
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperTokenBucketModeSet(devNum, tokenBucketMode);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tokenBucketMode);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperTokenBucketModeGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Token Bucket Mode Mll shaper.
* @param[in] devNum                   - device number.
*
* @param[out] tokenBucketModePtr       - (pointer to)Defines the packet length used to decrease the shaper token bucket.
*                                      In Byte mode, the token bucket rate shaper is decreased according to the
*                                      packet length as received by the switch.
*                                      In Packet mode, the token bucket rate shaper is decreased by the packet
*                                      length specified in cpssDxChIpMllMultiTargetShaperMtuSet.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperTokenBucketModeGet
(
    IN  GT_U8                                        devNum,
    OUT CPSS_PORT_TX_DROP_SHAPER_MODE_ENT            *tokenBucketModePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperTokenBucketModeGet(devNum, tokenBucketModePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tokenBucketModePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpUcRpfModeSet function
* @endinternal
*
* @brief   Defines the uRPF check mode for a given VID.
*
* @param[in] devNum                   - the device number
* @param[in] vid                      - Vlan Id
* @param[in] uRpfMode                 - unicast RPF mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, vid or uRpfMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*/
GT_STATUS prvTgfIpUcRpfModeSet
(
    IN  GT_U8                           devNum,
    IN  GT_U16                          vid,
    IN  PRV_TGF_IP_URPF_MODE_ENT        uRpfMode
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_URPF_MODE_ENT          dxChIpVlanUrpfModeType;
    GT_STATUS                           rc;

    /* convert IP Vlan URPF Mode type into device specific format */
    rc = prvTgfConvertGenericToDxChUnicastRpfType(uRpfMode, &dxChIpVlanUrpfModeType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    return cpssDxChIpUcRpfModeSet(devNum, vid, dxChIpVlanUrpfModeType);

    /* call device specific API */
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vid);
    TGF_PARAM_NOT_USED(uRpfMode);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpUcRpfModeGet function
* @endinternal
*
* @brief   Read uRPF check mode for a given VID.
*
* @param[in] devNum                   - the device number
* @param[in] vid                      - Vlan Id
*
* @param[out] uRpfModePtr              -  (pointer to) unicast RPF mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note If ucRPFCheckEnable field configured in cpssDxChIpLttWrite is enabled
*       then VLAN-based uRPF check is performed, regardless of this configuration.
*       Otherwise, uRPF check is performed for this VID according to this
*       configuration.
*       Port-based uRPF mode is not supported if the address is associated
*       with an ECMP/QoS block of nexthop entries.
*
*/
GT_STATUS prvTgfIpUcRpfModeGet
(
    IN  GT_U8                           devNum,
    IN  GT_U16                          vid,
    OUT PRV_TGF_IP_URPF_MODE_ENT        *uRpfModePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_URPF_MODE_ENT          dxChIpVlanUrpfModeType;
    GT_STATUS                           rc;

    /* call device specific API */
    rc = cpssDxChIpUcRpfModeGet(devNum, vid, &dxChIpVlanUrpfModeType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* convert IP Vlan URPF Mode type from device specific format */
    rc = prvTgfConvertDxChToGenericUnicastRpfType(dxChIpVlanUrpfModeType, uRpfModePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;

    /* call device specific API */
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vid);
    TGF_PARAM_NOT_USED(uRpfModePtr);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfIpUrpfLooseModeTypeSet function
* @endinternal
*
* @brief   This function set type of Urpf loose mode
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; xCat3; xCat3x ; Lion2; Bobcat3; Aldrin2; Falcon
*
* @param[in] devNum                   -  the device number
* @param[in] looseModeType            - value of urpf loose mode
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvTgfIpUrpfLooseModeTypeSet
(
    IN  GT_U8                            devNum,
    IN  PRV_TGF_URPF_LOOSE_MODE_TYPE_ENT looseModeType
)
{
    #ifdef CHX_FAMILY
    CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT  dxChUrpfLooseModeType;
    GT_STATUS                           rc;

    /* convert URPF Loose Mode type into device specific format */
    rc = prvTgfConvertGenericToDxChUnicastRpfLooseModeType(looseModeType, &dxChUrpfLooseModeType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* call device specific API */
    return cpssDxChIpUrpfLooseModeTypeSet(devNum, dxChUrpfLooseModeType);

    /* call device specific API */
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(vid);
    TGF_PARAM_NOT_USED(uRpfMode);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperEnableSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Enable/Disable Token Bucket rate shaping.
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvTgfIpMllMultiTargetShaperEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperEnableSet(devNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperEnableGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get Enable/Disable Token Bucket rate shaping status
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - Pointer to Token Bucket rate shaping status.
*                                      - GT_TRUE, enable Shaping
*                                      - GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperEnableGet(devNum, enablePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfIpMllMultiTargetShaperConfigurationSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set configuration for Mll shaper.
* @param[in] devNum                   - device number.
* @param[in] maxBucketSize            - Maximum bucket size in bytes. The field resolution is 4096 bytes.
*                                      The actual burst size is approximately
*                                      <MaxBucketSize>4096 - <Token Bucket Base Line>
*                                      When the CPU writes to this field, the value is also written
*                                      to the Token Bucket counter.
*                                      <MaxBucketSize>4096 must be set to be greater than both <Tokens>
*                                      and <Token Bucket Base Line>.
*                                      0 means 4k and 0xFFF means 16M
*                                      The bucket size ranges from 4 KB to 16 MB, in steps of 4K.
*                                      (APPLICABLE RANGES: 0...0xFFF)
* @param[in,out] maxRatePtr               - Requested Rate in Kbps or packets per second
*                                      according to shaper mode.
* @param[in,out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When packet based shaping is enabled, the following used as shaper's MTU:
*       see:
*       cpssDxChIpMllMultiTargetShaperMtuSet
*       cpssDxChIpMllMultiTargetShaperBaselineSet.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperConfigurationSet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       maxBucketSize,
    INOUT GT_U32                                     *maxRatePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperConfigurationSet(devNum,
                                                          maxBucketSize,
                                                          maxRatePtr);
#endif /* CHX_FAMILY */
#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(maxBucketSize);
    TGF_PARAM_NOT_USED(maxRatePtr);
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfIpMllMultiTargetShaperConfigurationGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get configuration for shaper.
* @param[in] devNum                   - device number.
*
* @param[out] maxBucketSizePtr         - (pointer to) Maximum bucket size in bytes. The field resolution is 4096 bytes.
*                                      The actual burst size is approximately
*                                      <MaxBucketSize>4096 - <Token Bucket Base Line>
*                                      When the CPU writes to this field, the value is also written
*                                      to the Token Bucket counter.
*                                      <MaxBucketSize>4096 must be set to be greater than both <Tokens>
*                                      and <Token Bucket Base Line>.
*                                      0 equal 4k all 12'bFFF equal 16M
*                                      The bucket size ranges from 4 KB to 16 MB, in steps of 4K.
*                                      (APPLICABLE RANGES: 0...0xFFF)
* @param[out] maxRatePtr               - (pointer to) the actual Rate value in Kbps or packets per second.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperConfigurationGet
(
    IN  GT_U8                                        devNum,
    OUT GT_U32                                       *maxBucketSizePtr,
    OUT GT_U32                                       *maxRatePtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperConfigurationGet(devNum,
                                                          maxBucketSizePtr,
                                                          maxRatePtr);
#endif /* CHX_FAMILY */
#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(maxBucketSizePtr);
    TGF_PARAM_NOT_USED(maxRatePtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperIntervalConfigurationSet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Set configuration for Mll shaper Interval.
* @param[in] devNum                   - device number.
* @param[in] tokenBucketIntervalSlowUpdateRatio - Increases the token bucket update interval to
*                                      <TokenBucketIntervalSlowUpdateRatio>+1>  <TokenBucketUpdateInterval>
*                                      (64 or 1024 depends on <TokenBucketIntervalUpdateRatio>) core clock cycles.
*                                      Enabled by setting SlowRateEn = GT_TRUE.
*                                      (APPLICABLE RANGES: 1...16)
* @param[in] tokenBucketUpdateInterval -   Defines the token update interval for the egress rate shapers.
*                                      The update interval = <TokenBucketIntervalSlowUpdateRatio>+1>
*                                      <TokenBucketUpdateInterval>  (64 or 1024 depends on
*                                      <TokenBucketIntervalUpdateRatio>)2^(Port/PriorityTokenBucketEntry<TBIntervalUpdateRatio>) core clock cycles.
*                                      Upon every interval expiration, a configured amount of tokens is added to the
*                                      token bucket. The amount of tokens is configured in Port/PriorityTokenBucketEntry<Tokens>.
*                                      NOTE:
*                                      - This field must not be 0.
*                                      (APPLICABLE RANGES: 1...15)
* @param[in] tokenBucketIntervalUpdateRatio - Token Bucket Interval Update Ratio : 64 or 1024
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllMultiTargetShaperIntervalConfigurationSet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          tokenBucketIntervalSlowUpdateRatio,
    IN  GT_U32                                          tokenBucketUpdateInterval,
    IN  PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_ENT     tokenBucketIntervalUpdateRatio
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   dxchTokenBucketIntervalUpdateRatio;
    switch(tokenBucketIntervalUpdateRatio)
    {
    case PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E:
        dxchTokenBucketIntervalUpdateRatio = CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E;
        break;
    case PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E:
        dxchTokenBucketIntervalUpdateRatio = CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    /* call device specific API */
    return cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet(devNum,
                                                          tokenBucketIntervalSlowUpdateRatio,
                                                          tokenBucketUpdateInterval,
                                                          dxchTokenBucketIntervalUpdateRatio);
#endif /* CHX_FAMILY */
#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tokenBucketIntervalSlowUpdateRatio);
    TGF_PARAM_NOT_USED(tokenBucketUpdateInterval);
    TGF_PARAM_NOT_USED(tokenBucketIntervalUpdateRatio);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllMultiTargetShaperIntervalConfigurationGet function
* @endinternal
*
* @brief   Relevant for L2 and L3 MLL.
*         Get configuration for shaper Interval.
* @param[in] devNum                   - device number.
*
* @param[out] tokenBucketIntervalSlowUpdateRatioPtr - (pointer to) Increases the token bucket update interval to
*                                      <TokenBucketIntervalSlowUpdateRatio>+1>  <TokenBucketUpdateInterval>
*                                      (64 or 1024 depends on <TokenBucketIntervalUpdateRatio>) core clock cycles.
*                                      Enabled by setting SlowRateEn = GT_TRUE.
*                                      (APPLICABLE RANGES: 1...16)
* @param[out] tokenBucketUpdateIntervalPtr -   (pointer to) the token update interval for the egress rate shapers.
*                                      The update interval = <TokenBucketIntervalSlowUpdateRatio>+1>
*                                      <TokenBucketUpdateInterval>  (64 or 1024 depends on
*                                      <TokenBucketIntervalUpdateRatio>)2^(Port/PriorityTokenBucketEntry<TBIntervalUpdateRatio>) core clock cycles.
*                                      Upon every interval expiration, a configured amount of tokens is added to the
*                                      token bucket. The amount of tokens is configured in Port/PriorityTokenBucketEntry<Tokens>.
*                                      NOTE:
*                                      - This field must not be 0.
*                                      (APPLICABLE RANGES: 1...15)
* @param[out] tokenBucketIntervalUpdateRatioPtr - (pointer to) Token Bucket Interval Update Ratio : 64 or 1024
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvTgfIpMllMultiTargetShaperIntervalConfigurationGet
(
    IN   GT_U8                                           devNum,
    OUT  GT_U32                                          *tokenBucketIntervalSlowUpdateRatioPtr,
    OUT  GT_U32                                          *tokenBucketUpdateIntervalPtr,
    OUT  PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_ENT     *tokenBucketIntervalUpdateRatioPtr
)
{
#ifdef CHX_FAMILY

    GT_STATUS rc = GT_OK;
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT   dxchTokenBucketIntervalUpdateRatio;


    /* call device specific API */
    rc = cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet(devNum,
                                                          tokenBucketIntervalSlowUpdateRatioPtr,
                                                          tokenBucketUpdateIntervalPtr,
                                                          &dxchTokenBucketIntervalUpdateRatio);

    if(rc != GT_OK)
        return rc;

    switch(dxchTokenBucketIntervalUpdateRatio)
    {
    case CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E:
        *tokenBucketIntervalUpdateRatioPtr = PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E;
        break;
    case CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E:
        *tokenBucketIntervalUpdateRatioPtr = PRV_TGF_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E;
        break;
    default:
        return GT_BAD_VALUE;
    }

    return rc;

#endif /* CHX_FAMILY */
#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(tokenBucketIntervalSlowUpdateRatioPtr);
    TGF_PARAM_NOT_USED(tokenBucketUpdateIntervalPtr);
    TGF_PARAM_NOT_USED(tokenBucketIntervalUpdateRatioPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpPortSipSaEnableSet function
* @endinternal
*
* @brief   Enable SIP/SA check for packets received from the given port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (including CPU port)
* @param[in] enable                   - GT_FALSE: disable SIP/SA check on the port
*                                      GT_TRUE:  enable SIP/SA check on the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note SIP/SA check is triggered only if either this flag or
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*/
GT_STATUS prvTgfIpPortSipSaEnableSet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    IN  GT_BOOL                      enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpPortSipSaEnableSet(devNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPortSipSaEnableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpPortSipSaEnableGet function
* @endinternal
*
* @brief   Return the SIP/SA check status for packets received from the given port.
*
* @param[in] devNum                   - the device number
* @param[in] portNum                  - the port number (including CPU port)
*
* @param[out] enablePtr                - GT_FALSE: SIP/SA check on the port is disabled
*                                      GT_TRUE:  SIP/SA check on the port is enabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note SIP/SA check is triggered only if either this flag or
*       the sipSaCheckMismatchEnable field configured in cpssDxChIpLttWrite
*       are enabled.
*
*/
GT_STATUS prvTgfIpPortSipSaEnableGet
(
    IN  GT_U8                        devNum,
    IN  GT_PORT_NUM                  portNum,
    OUT GT_BOOL                      *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpPortSipSaEnableGet(devNum, portNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPortSipSaEnableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfIpEcmpHashNumBitsSet function
* @endinternal
*
* @brief   Set the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism
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
*
*/
GT_STATUS prvTgfIpEcmpHashNumBitsSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       startBit,
    IN GT_U32                       numOfBits
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpEcmpHashNumBitsSet(devNum, startBit, numOfBits);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpEcmpHashNumBitsSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(startBit);
    TGF_PARAM_NOT_USED(numOfBits);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfIpEcmpHashNumBitsGet function
* @endinternal
*
* @brief   Get the start bit and the number of bits needed by the L3 ECMP hash
*         mechanism
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
GT_STATUS prvTgfIpEcmpHashNumBitsGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *startBitPtr,
    OUT GT_U32                      *numOfBitsPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpEcmpHashNumBitsGet(devNum, startBitPtr, numOfBitsPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpEcmpHashNumBitsSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(startBitPtr);
    TGF_PARAM_NOT_USED(numOfBitsPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet function
* @endinternal
*
* @brief   Enable/disable bypassing the router triggering requirements for policy
*         based routing packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  bypassing the router triggering requirements for PBR
*                                      packets:
*                                      GT_FALSE: Policy based routing uses the normal router
*                                      triggering requirements
*                                      GT_TRUE:  Policy based routing bypasses the router triggering
*                                      requirement for the FDB DA entry <DA Route> to be
*                                      set for UC routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet(devNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet function
* @endinternal
*
* @brief   Get the enabling status of bypassing the router triggering requirements
*         for policy based routing packets
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - the router triggering requirements enabling status for PBR
*                                      packets:
*                                      GT_FALSE: Policy based routing uses the normal router
*                                      triggering requirements
*                                      GT_TRUE:  Policy based routing bypasses the router triggering
*                                      requirement for the FDB DA entry <DA Route> to be
*                                      set for UC routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet(devNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet FAILED, rc = [%d]", rc);
        return rc;
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}


#ifdef CHX_FAMILY
/**
* @internal prvTgfConvertGenericToDxChIpNatRouteEntry function
* @endinternal
*
* @brief   Convert generic IP NAT route entry into device specific entry
*
* @param[in] natType                  - NAT type
* @param[in] prvTgfNatEntryPtr        - (pointer to) IP NAT route entry
*
* @param[out] dxChIpNatRouteEntryPtr   - (pointer to) DxCh IP NAT route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChIpNatRouteEntry
(
    IN  PRV_TGF_IP_NAT_TYPE_ENT       natType,
    IN  PRV_TGF_IP_NAT_ENTRY_UNT      *prvTgfNatEntryPtr,
    OUT CPSS_DXCH_IP_NAT_ENTRY_UNT    *dxChIpNatRouteEntryPtr
)
{
    switch(natType)
    {
        case PRV_TGF_IP_NAT_TYPE_NAT44_E:
             /* convert IP MC route entry into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), macDa);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyDip);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newDip);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifySip);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newSip);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyTcpUdpDstPort);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newTcpUdpDstPort);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyTcpUdpSrcPort);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newTcpUdpSrcPort);
            break;
        case PRV_TGF_IP_NAT_TYPE_NAT66_E:
             /* convert IP MC route entry into device specific format */
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), macDa);
            switch (prvTgfNatEntryPtr->nat66Entry.modifyCommand)
            {
                case PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E:
                    dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E;
                    break;
                case PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E:
                    dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E;
                    break;
                case PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E:
                    dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E;
                    break;
                case PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E:
                    dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand = CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E;
                    break;
                default:
                    return GT_BAD_PARAM;
            }
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), address);
            PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), prefixSize);
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericIpNatRouteEntry function
* @endinternal
*
* @brief   Convert device specific IP NAT route entry into generic entry
*
* @param[in] natType                  - NAT type
* @param[in] dxChIpNatRouteEntryPtr   - (pointer to) DxCh IP NAT route entry
*
* @param[out] prvTgfNatEntryPtr        - (pointer to) IP NAT route entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericIpNatRouteEntry
(
    IN  PRV_TGF_IP_NAT_TYPE_ENT       natType,
    IN  CPSS_DXCH_IP_NAT_ENTRY_UNT   *dxChIpNatRouteEntryPtr,
    OUT PRV_TGF_IP_NAT_ENTRY_UNT     *prvTgfNatEntryPtr
)
{
    switch(natType)
    {
        case PRV_TGF_IP_NAT_TYPE_NAT44_E:
            /* convert IP MC route entry into device specific format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), macDa);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyDip);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newDip);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifySip);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newSip);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyTcpUdpDstPort);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newTcpUdpDstPort);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), modifyTcpUdpSrcPort);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat44Entry), &(prvTgfNatEntryPtr->nat44Entry), newTcpUdpSrcPort);
            break;
        case PRV_TGF_IP_NAT_TYPE_NAT66_E:

            /* convert IP MC route entry into device specific format */
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), macDa);
            switch (dxChIpNatRouteEntryPtr->nat66Entry.modifyCommand)
            {
                case CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E:
                    prvTgfNatEntryPtr->nat66Entry.modifyCommand = PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E;
                    break;
                case CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E:
                    prvTgfNatEntryPtr->nat66Entry.modifyCommand = PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E;
                    break;
                case CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E:
                    prvTgfNatEntryPtr->nat66Entry.modifyCommand = PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E;
                    break;
                case CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E:
                    prvTgfNatEntryPtr->nat66Entry.modifyCommand = PRV_TGF_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E;
                    break;
                default:
                    return GT_BAD_PARAM;
            }
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), address);
            PRV_TGF_STC_D2S_FIELD_COPY_MAC(&(dxChIpNatRouteEntryPtr->nat66Entry), &(prvTgfNatEntryPtr->nat66Entry), prefixSize);
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}
#endif /* CHX_FAMILY */


/**
* @internal prvTgfIpNatEntrySet function
* @endinternal
*
* @brief   Set a NAT entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] natIndex                 - index for the NAT entry
*                                      in the router ARP / tunnel start / NAT table
* @param[in] natType                  - type of the NAT
* @param[in] entryPtr                 - (pointer to) NAT entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table, Tunnel start entries table and router ARP addresses
*       table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS prvTgfIpNatEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              natIndex,
    IN  PRV_TGF_IP_NAT_TYPE_ENT             natType,
    IN  PRV_TGF_IP_NAT_ENTRY_UNT           *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    CPSS_IP_NAT_TYPE_ENT        natTypeCpss;
    CPSS_DXCH_IP_NAT_ENTRY_UNT  entryCpss;


    switch(natType)
    {
        case PRV_TGF_IP_NAT_TYPE_NAT44_E:
            natTypeCpss = CPSS_IP_NAT_TYPE_NAT44_E;
            break;
        case PRV_TGF_IP_NAT_TYPE_NAT66_E:
            natTypeCpss = CPSS_IP_NAT_TYPE_NAT66_E;
        default:
            return GT_BAD_PARAM;
    }

    prvTgfConvertGenericToDxChIpNatRouteEntry(natType, entryPtr, &entryCpss);

    /* call device specific API */
    rc = cpssDxChIpNatEntrySet(devNum, natIndex, natTypeCpss, &entryCpss);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpNatEntrySet FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(natIndex);
    TGF_PARAM_NOT_USED(natType);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpNatEntryGet function
* @endinternal
*
* @brief   Get a NAT entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] natIndex                 - index for the NAT entry
*                                      in the router ARP / tunnel start / NAT table
*
* @param[out] natTypePtr               - (pointer to) type of the NAT
* @param[out] entryPtr                 - (pointer to) NAT entry
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - on invalid tunnel type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NAT entries table, Tunnel start entries table and router ARP addresses
*       table reside at the same physical memory.
*       Each line can hold:
*       - 1 NAT entry
*       - 1 tunnel start entry
*       - 4 router ARP addresses entries
*       NAT entry / Tunnel start entry at index n and router ARP addresses at indexes
*       4n..4n+3 share the same memory. For example NAT entry/tunnel start entry at
*       index 100 and router ARP addresses at indexes 400..403 share the same
*       physical memory.
*
*/
GT_STATUS prvTgfIpNatEntryGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             natIndex,
    OUT  PRV_TGF_IP_NAT_TYPE_ENT            *natTypePtr,
    OUT  PRV_TGF_IP_NAT_ENTRY_UNT           *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    CPSS_IP_NAT_TYPE_ENT        natTypeCpss;
    CPSS_DXCH_IP_NAT_ENTRY_UNT  entryCpss;

    /* call device specific API */
    rc = cpssDxChIpNatEntryGet(devNum, natIndex, &natTypeCpss, &entryCpss);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpNatEntryGet FAILED, rc = [%d]", rc);
    }

    switch(natTypeCpss)
    {
        case CPSS_IP_NAT_TYPE_NAT44_E:
            *natTypePtr = PRV_TGF_IP_NAT_TYPE_NAT44_E;
            break;
        case CPSS_IP_NAT_TYPE_NAT66_E:
            *natTypePtr = PRV_TGF_IP_NAT_TYPE_NAT66_E;
        default:
            return GT_BAD_PARAM;
    }

    prvTgfConvertDxChToGenericIpNatRouteEntry(*natTypePtr, &entryCpss, entryPtr);

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(natIndex);
    TGF_PARAM_NOT_USED(natType);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpLpmDbgHwOctetPerBlockPrint function
* @endinternal
*
* @brief   Print Octet per Block debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerBlockPrint
(
    IN GT_U32                           lpmDbId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpLpmDbgHwOctetPerBlockPrint(lpmDbId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDbgHwOctetPerBlockPrint FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpLpmDbgHwOctetPerProtocolPrint function
* @endinternal
*
* @brief   Print Octet per Protocol debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerProtocolPrint
(
    IN GT_U32                           lpmDbId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpLpmDbgHwOctetPerProtocolPrint(lpmDbId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpLpmDbgHwOctetPerProtocolPrint FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    return GT_BAD_STATE;
#endif
}


/**
* @internal prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters
(
    IN GT_U32                           lpmDbId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(lpmDbId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines and Banks memory debug information for a
*         specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters
(
    IN GT_U32                           lpmDbId
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc =
        cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(lpmDbId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(lpmDbId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpMllBridgeEnable function
* @endinternal
*
* @brief   enable/disable MLL based bridging.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2.
*
* @param[in] devNum                   - the device number
* @param[in] mllBridgeEnable          - enable /disable MLL based bridging.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpMllBridgeEnable
(
    IN   GT_U8      devNum,
    IN   GT_BOOL    mllBridgeEnable
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChIpMllBridgeEnable(devNum, mllBridgeEnable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfIpMllBridgeEnable FAILED, rc = [%d]", rc);
    }

    return rc;
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(mllBridgeEnable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpBridgeServiceEnable function
* @endinternal
*
* @brief   enable/disable a router bridge service.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] bridgeService            - the router bridge service
* @param[in] enableDisableMode        - Enable/Disable mode of this function (weather
*                                      to enable/disable for ipv4/ipv6/arp)
* @param[in] enableService            - weather to enable the service for the above more.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpBridgeServiceEnable
(
    IN  GT_U8                                           devNum,
    IN  PRV_TGF_IP_BRG_SERVICE_ENT                      bridgeService,
    IN  PRV_TGF_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT  enableDisableMode,
    IN  GT_BOOL                                         enableService
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_BRG_SERVICE_ENT service;
    CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT mode;
    switch(bridgeService)
    {
    case PRV_TGF_IP_HEADER_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_HEADER_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_SIP_SA_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_SIP_FILTER_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_SIP_FILTER_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_LAST_E:
        service = CPSS_DXCH_IP_BRG_SERVICE_LAST_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    switch(enableDisableMode)
    {
    case PRV_TGF_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return cpssDxChIpBridgeServiceEnable(devNum, service, mode, enableService);
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(bridgeService);
    TGF_PARAM_NOT_USED(enableDisableMode);
    TGF_PARAM_NOT_USED(enableService);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpBridgeServiceEnableGet function
* @endinternal
*
* @brief   enable/disable a router bridge service.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; Bobcat2; Caelum; Aldrin;
* @note   NOT APPLICABLE DEVICES:  NOT APPLICABLE DEVICES:
*
* @param[in] devNum                   - the device number
* @param[in] bridgeService            - the router bridge service
* @param[in] enableDisableMode        - Enable/Disable mode of this function (weather
*                                      to enable/disable for ipv4/ipv6/arp)
*
* @param[out] enableServicePtr         - weather to enable the service for the above more.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpBridgeServiceEnableGet
(
    IN  GT_U8                                           devNum,
    IN  PRV_TGF_IP_BRG_SERVICE_ENT                      bridgeService,
    IN  PRV_TGF_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT  enableDisableMode,
    OUT GT_BOOL                                         *enableServicePtr
)
{
#ifdef CHX_FAMILY
    CPSS_DXCH_IP_BRG_SERVICE_ENT service;
    CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT mode;
    switch(bridgeService)
    {
    case PRV_TGF_IP_HEADER_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_HEADER_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_SIP_SA_CHECK_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_SIP_SA_CHECK_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_SIP_FILTER_BRG_SERVICE_E:
        service = CPSS_DXCH_IP_SIP_FILTER_BRG_SERVICE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_LAST_E:
        service = CPSS_DXCH_IP_BRG_SERVICE_LAST_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    switch(enableDisableMode)
    {
    case PRV_TGF_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E;
        break;
    case PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E:
        mode = CPSS_DXCH_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E;
        break;
    default:
        return GT_BAD_PARAM;
    }

    return cpssDxChIpBridgeServiceEnableGet(devNum, service, mode,
        enableServicePtr);
#else /* CHX_FAMILY */
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(bridgeService);
    TGF_PARAM_NOT_USED(enableDisableMode);
    TGF_PARAM_NOT_USED(enableServicePtr);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfIpPortFcoeForwardingEnableSet function
* @endinternal
*
* @brief   Enable FCoE Forwarding on a port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] portNum                  - the port to enable on
* @param[in] enable                   -  FCoE Forwarding for this port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpPortFcoeForwardingEnableSet
(
    IN GT_PORT_NUM                      portNum,
    IN GT_BOOL                          enable
)
{
    GT_STATUS   rc;

    rc = cpssDxChIpPortFcoeForwardingEnableSet(prvTgfDevNum, portNum, enable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPortFcoeForwardingEnableSet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfIpPortFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Get status of FCoE Forwarding on a port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @param[out] enablePtr                - (pointer to)enable FCoE Forwarding for this port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong protocolStack
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfIpPortFcoeForwardingEnableGet
(
    IN  GT_PORT_NUM                      portNum,
    OUT GT_BOOL                          *enablePtr
)
{
    GT_STATUS   rc;

    rc = cpssDxChIpPortFcoeForwardingEnableGet(prvTgfDevNum, portNum, enablePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpPortFcoeForwardingEnableGet FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfIpLpmFcoePrefixAdd function
* @endinternal
*
* @brief   This function adds a new FCoE prefix to a Virtual Router in a
*         specific LPM DB or overrides an existing existing FCoE prefix.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvTgfIpLpmFcoePrefixAdd
(
    IN  GT_U32                                      lpmDBId,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcoeAddr,
    IN  GT_U32                                      prefixLen,
    IN  CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT      *nextHopInfoPtr,
    IN  GT_BOOL                                     override,
    IN  GT_BOOL                                     defragmentationEnable
)
{
    GT_STATUS   rc;

    rc = cpssDxChIpLpmFcoePrefixAdd(lpmDBId, vrId,
                                    &fcoeAddr, prefixLen, nextHopInfoPtr, override, defragmentationEnable);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmFcoePrefixAdd FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfIpLpmFcoePrefixDel function
* @endinternal
*
* @brief   Deletes an existing FCoE prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2.
*
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NO_SUCH               - If the given prefix doesn't exitst in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvTgfIpLpmFcoePrefixDel
(
    IN  GT_U32                                  lpmDBId,
    IN  GT_U32                                  vrId,
    IN  GT_FCID                                 fcoeAddr,
    IN  GT_U32                                  prefixLen
)
{
    GT_STATUS   rc;

    rc = cpssDxChIpLpmFcoePrefixDel(lpmDBId, vrId, &fcoeAddr, prefixLen);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChIpLpmFcoePrefixDel FAILED, rc = [%d]", rc);
    }
    return rc;
}

/**
* @internal prvTgfIpFdbRoutePrefixLenSet function
* @endinternal
*
* @brief   set the IPv4/6 prefix length when accessing the FDB
*          table for IPv4/6 Route lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - the type of protocol
* @param[in] prefixLen                - The number of bits that
*                                       are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpFdbRoutePrefixLenSet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    IN GT_U32                       prefixLen
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpFdbRoutePrefixLenSet(devNum, protocolStack, prefixLen);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(prefixLen);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpFdbRoutePrefixLenGet function
* @endinternal
*
* @brief   get the IPv4/6 prefix length when accessing the FDB
*          table for IPv4/6 Route lookup
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - the type of protocol
*
* @param[out] prefixLen                - The number of bits that
*                                       are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_VALUE             - on bad output value
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfIpFdbRoutePrefixLenGet
(
    IN GT_U8                        devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT   protocolStack,
    OUT GT_U32                      *prefixLenPtr
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpFdbRoutePrefixLenGet(devNum, protocolStack, prefixLenPtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(protocolStack);
    TGF_PARAM_NOT_USED(prefixLenPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpFdbUnicastRouteForPbrEnableSet function
* @endinternal
*
* @brief   Enable/Disable FDB Unicast routing for PBR (Policy Based
*          Routed) packets
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable FDB
*                                       routing for PBR packets
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is useful for overriding the PBR forwarding
*    decision by a matching FDB Route entry.
*/
GT_STATUS prvTgfIpFdbUnicastRouteForPbrEnableSet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      enable
)
{
    #ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpFdbUnicastRouteForPbrEnableSet(devNum, enable);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enable);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfIpFdbUnicastRouteForPbrEnableGet function
* @endinternal
*
* @brief   Return if FDB Unicast routing for PBR value (Policy Based
*          Routed) packets is enabled
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*          Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr               - (pointer to) enabling status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This is useful for overriding the PBR forwarding
*    decision by a matching FDB Route entry.
*/
GT_STATUS prvTgfIpFdbUnicastRouteForPbrEnableGet
(
    IN GT_U8                        devNum,
    IN GT_BOOL                      *enablePtr
)
{
    #ifdef CHX_FAMILY
    /* call device specific API */
    return cpssDxChIpFdbUnicastRouteForPbrEnableGet(devNum, enablePtr);
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(devNum);
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif
}
