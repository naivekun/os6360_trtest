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
* @file cstBridge.c
*
* @brief Generic API implementation for Bridge
*
* @version   135
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <extUtils/auEventHandler/auFdbUpdateLock.h>

#include <common/tgfBridgeGen.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
    #include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgNestVlan.h>
    #include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgCount.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
    #include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>
    #include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbRouting.h>

/* part 1 : max number of messages to store */
#define PART1_MAX_MESSAGE_NUM_CNS     _2K
/* part 2 : max number of messages to store*/
#define PART2_MAX_MESSAGE_NUM_CNS     _2K


/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/* did we initialized the local variables */
static GT_BOOL isInitialized = GT_FALSE;

/* default mac entry */
static CPSS_MAC_ENTRY_EXT_STC   defMacEntry;

/* default vlan entry */
static CPSS_DXCH_BRG_VLAN_INFO_STC   defVlanInfo;

/* indication that the FDB shadow used */
extern GT_BOOL prvTgfFdbShadowUsed;

/**
* @internal cstBrgInit function
* @endinternal
*
* @brief   Initialize local default settings
*/
GT_VOID cstBrgInit
(
    GT_VOID
)
{
    if(GT_TRUE == isInitialized)
    {
        return;
    }

    isInitialized = GT_TRUE;

    /* clear entry */
    cpssOsMemSet(&defMacEntry, 0, sizeof(defMacEntry));

    /* set default mac entry */
    defMacEntry.key.entryType            = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    defMacEntry.key.key.macVlan.vlanId   = 0;
    defMacEntry.dstInterface.type        = CPSS_INTERFACE_VIDX_E;
    defMacEntry.dstInterface.vidx        = 0;
    defMacEntry.isStatic                 = GT_FALSE;
    defMacEntry.daCommand                = CPSS_MAC_TABLE_FRWRD_E;
    defMacEntry.saCommand                = CPSS_MAC_TABLE_FRWRD_E;
    defMacEntry.daRoute                  = GT_FALSE;
    defMacEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;
    defMacEntry.userDefined              = 0;
    defMacEntry.daQosIndex               = 0;
    defMacEntry.saQosIndex               = 0;
    defMacEntry.daSecurityLevel          = 0;
    defMacEntry.saSecurityLevel          = 0;
    defMacEntry.appSpecificCpuCode       = GT_FALSE;
    defMacEntry.spUnknown                = GT_FALSE;
    defMacEntry.sourceID                 = 0;
    /* clear entry */
    cpssOsMemSet(&defVlanInfo, 0, sizeof(defVlanInfo));

    /* set default Vlan info entry */
    defVlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    defVlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    defVlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    defVlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    defVlanInfo.mirrToTxAnalyzerEn   = GT_FALSE;
    defVlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    defVlanInfo.ipCtrlToCpuEn        = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    defVlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    defVlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    defVlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    defVlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    defVlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    defVlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    defVlanInfo.ipv4McastRouteEn     = GT_FALSE;
    defVlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    defVlanInfo.ipv6McastRouteEn     = GT_FALSE;
    defVlanInfo.stgId                = 0;
    defVlanInfo.autoLearnDisable     = GT_TRUE;
    defVlanInfo.naMsgToCpuEn         = GT_TRUE;
    defVlanInfo.mruIdx               = 0;
    defVlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    defVlanInfo.vrfId                = 0;
    defVlanInfo.floodVidx            = 0xFFF;/* flood into vlan */
}

/**
* @internal cstBrgDefVlanEntryWithPortsSet function
* @endinternal
*
* @brief   Set vlan entry, with the requested ports, tagging
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - the vlan id
* @param[in] portsArray[]             - array of ports
* @param[in] devArray[]               - array of devices (when NULL -> assume all ports belong to devNum)
* @param[in] tagArray[]               - array of tagging for the ports (when NULL -> assume all ports untagged)
* @param[in] numPorts                 - number of ports in the arrays
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cstBrgDefVlanEntryWithPortsSet
(
    IN GT_U8                          devNum,
    IN GT_U16                         vlanId,
    IN GT_U32                          portsArray[],
    IN GT_U8                          devArray[],
    IN GT_U8                          tagArray[],
    IN GT_U32                         numPorts
)
{
    GT_STATUS                            rc;
    GT_U32                               portIter;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;


    /* initialize default settings */
    cstBrgInit();

    /* start with no ports */
    /* all ports are 'untagged' */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* clear portsTaggingCmd */
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* add the needed ports to the vlan */
    for(portIter = 0; portIter < numPorts; portIter++)
    {
        if(devArray)
        {
            if(devArray[portIter] != devNum)
            {
                /* this port not belong to current device */
                continue;
            }
        }

        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,portsArray[portIter]);

        if(tagArray)
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsTagging,portsArray[portIter]);

            portsTaggingCmd.portsCmd[portsArray[portIter]] = (tagArray[portIter] & 1) ?
                CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E :
                CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        }
    }

    /* set default vlan entry */
    cpssOsMemCpy(&vlanInfo, &defVlanInfo, sizeof(vlanInfo));

    vlanInfo.unregIpmEVidx = 0xFFF;
    if(vlanInfo.fidValue == 0)
        vlanInfo.fidValue = vlanId;


    rc =  cpssDxChBrgVlanEntryWrite(devNum,
                                    vlanId,
                                    &portsMembers,
                                    &portsTagging,
                                    &vlanInfo,
                                    &portsTaggingCmd);
    return rc;
}


/**
* @internal cstBrgDefVlanEntryInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN entry and reset all it's content
*
* @param[in] devNum                   - device number
* @param[in] vlanId                   - VLAN id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
*/
GT_STATUS cstBrgDefVlanEntryInvalidate
(
    IN GT_U8                devNum,
    IN GT_U16               vlanId
)
{
    GT_STATUS                             rc = GT_OK;
    CPSS_PORTS_BMP_STC                    portsMembers;
    CPSS_PORTS_BMP_STC                    portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC           vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;


    /* initialize default settings */
    cstBrgInit();

    /* start with no ports */
    /* all ports are 'untagged' */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* reset vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));

    /* reset portsTaggingCmd */
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* write Vlan entry */
    rc = cpssDxChBrgVlanEntryWrite(devNum, vlanId,
                                   &portsMembers, &portsTagging,
                                   &vlanInfo, &portsTaggingCmd);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* AUTODOC: invalidate vlan entry */
    rc = cpssDxChBrgVlanEntryInvalidate(devNum,vlanId);

    return rc;
}


/**
* @internal cstBrgFdbFlush function
* @endinternal
*
* @brief   Flush FDB table (with/without static entries).
*         function sets actDev and it's mask to 'dont care' so all entries can be
*         flushed (function restore default values at end of operation).
* @param[in] includeStatic            - include static entries
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS cstBrgFdbFlush
(
    IN GT_BOOL                        includeStatic
)
{
    /* DO NOT COPY ALL THE IMPLEMENTATION FROM 'prvTgfBrgFdbFlush' !!! */
    /* BECAUSE FUNCTION prvTgfBrgFdbFlush() IS COMPLEX */
    return prvTgfBrgFdbFlush(includeStatic);
}

#endif /* CHX_FAMILY */


