/*******************************************************************************
*              Copyright 2001, GALILEO TECHNOLOGY, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* AND GALILEO TECHNOLOGY, INC. (GTI).
********************************************************************************
* wrapBridgeCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for Bridge cpss.dxCh functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 57 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>

/* DxCh max number of entries to be read from tables\classes */
#define  CPSS_DXCH_VLAN_MAX_ENTRY_CNS 4096

/* MACRO for VLAN table size */
#define  CPSS_DXCH_VLAN_MAX_ENTRY_MAC(_devNum)\
((_devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)? CPSS_DXCH_VLAN_MAX_ENTRY_CNS : \
  (1 + PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(_devNum)))
/*
NOTE: the emulation of DB and functions that start with prefix : wrSip5Emulate_
are COPIED from : <common\tgfBridgeGen.c> --> 'MAIN UT'  --> named sip5Emulate_
to remove the dependencies between the 2 libraries.
*/
static GT_U32 wrSip5Emulate_tpidBmpTable[2][8] = {{0},{0}};
static GT_U32 wrSip5Emulate_tpidBmpTable_used[2][8] = {{0},{0}};

/**
* @internal cmdLibResetCpssDxChBridgeVlan function
* @endinternal
*
* @brief   Library database reset function.
*
* @note none
*
*/
static GT_VOID cmdLibResetCpssDxChBridgeVlan
(
    GT_VOID
)
{
    cpssOsBzero((GT_CHAR *)wrSip5Emulate_tpidBmpTable_used, sizeof(wrSip5Emulate_tpidBmpTable_used));
    cpssOsBzero((GT_CHAR *)wrSip5Emulate_tpidBmpTable, sizeof(wrSip5Emulate_tpidBmpTable));
}

static GT_STATUS wrSip5Emulate_IngressTpidProfileGet
(
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp,
    OUT GT_U32               *profilePtr
)
{
    GT_U32  ii;
    GT_U32  iiFirstFree = 0xFF;
    GT_U32  tag = (ethMode ==  CPSS_VLAN_ETHERTYPE0_E) ? 0 : 1;

    /* look for the tpidBmp in the table */
    for(ii = 0 ; ii < 8; ii++)
    {
        if(wrSip5Emulate_tpidBmpTable_used[tag][ii] == 0)
        {
            if(iiFirstFree == 0xFF)
            {
                iiFirstFree = ii;
            }

            continue;
        }

        if(wrSip5Emulate_tpidBmpTable[tag][ii] == tpidBmp)
        {
            break;
        }
    }

    if(ii == 8) /* not found in the table */
    {
        if(iiFirstFree == 0xFF)/*not found empty entry*/
        {
            return GT_FULL;
        }

        ii = iiFirstFree;
    }

    if(ii >= 8)
    {
        return GT_BAD_STATE;
    }

    *profilePtr = ii;

    wrSip5Emulate_tpidBmpTable_used[tag][ii] = 1;
    wrSip5Emulate_tpidBmpTable[tag][ii] = tpidBmp;

    return GT_OK;
}

static GT_STATUS wrSip5Emulate_cpssDxChBrgVlanPortIngressTpidSet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    IN  GT_U32               tpidBmp
)
{
    GT_STATUS   rc;
    GT_U32  profile;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    /* look for the tpidBmp in the table */
    rc = wrSip5Emulate_IngressTpidProfileGet(ethMode,tpidBmp,&profile);
    if(rc != GT_OK)
    {
       return rc;
    }

    rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum,profile,ethMode,tpidBmp);
    if(rc != GT_OK)
    {
       return rc;
    }

    /* write to pre-tti-lookup-ingress-eport table */
    rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum,
                portNum,
                ethMode,
                GT_TRUE,
                profile);
    if(rc != GT_OK)
    {
       return rc;
    }

   /* write to post-tti-lookup-ingress-eport table */
    rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum,
                portNum,
                ethMode,
                GT_FALSE,
                profile);

    return rc;
}

static GT_STATUS wrSip5Emulate_cpssDxChBrgVlanPortIngressTpidGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_NUM          portNum,
    IN  CPSS_ETHER_MODE_ENT  ethMode,
    OUT GT_U32               *tpidBmpPtr
)
{
    GT_STATUS   rc;
    GT_U32  profile;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    /* read from pre-tti-lookup-ingress-eport table */
    rc = cpssDxChBrgVlanPortIngressTpidProfileGet(devNum,
                portNum,
                ethMode,
                GT_TRUE,/* default value */
                &profile);
    if(rc != GT_OK)
    {
       return rc;
    }


    rc = cpssDxChBrgVlanIngressTpidProfileGet(devNum,profile,ethMode,tpidBmpPtr);

    return rc;
}


/**
* @internal wrCpssDxChBrgVlanInit function
* @endinternal
*
* @brief   Initialize VLAN for specific device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on system init error.
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
*/
static CMD_STATUS wrCpssDxChBrgVlanInit

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgVlanInit(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanEntriesRangeWrite function
* @endinternal
*
* @brief   Function set multiple vlans with the same configuration
*         function needed for performances , when the device use "indirect" access
*         to the vlan table.
*
* @note   APPLICABLE DEVICES:      all DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong vbMode
* @retval GT_NOT_SUPPORTED         - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanEntriesRangeWrite

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;

    GT_U8                            devNum;
    GT_U16                           vlanId;
    GT_U32                           numOfEntries;
    CPSS_PORTS_BMP_STC               portsMembersPtr;
    CPSS_PORTS_BMP_STC               portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_INFO_STC      vlanInfoPtr;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_U32                           i; /* loop iterator */
    GT_U32                           fieldOffset = 0;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    vlanId = (GT_U16)inFields[0];
    vlanInfoPtr.fidValue = vlanId;

    numOfEntries = (GT_U32)inFields[1];

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembersPtr);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTaggingPtr);
    portsMembersPtr.ports[0] = (GT_U32)inFields[2];
    portsMembersPtr.ports[1] = (GT_U32)inFields[3];
    portsTaggingPtr.ports[0] = (GT_U32)inFields[4];
    portsTaggingPtr.ports[1] = (GT_U32)inFields[5];

    if (numFields >= 50)
    {
        portsMembersPtr.ports[2] = (GT_U32)inFields[4];
        portsMembersPtr.ports[3] = (GT_U32)inFields[5];
        portsTaggingPtr.ports[0] = (GT_U32)inFields[6];
        portsTaggingPtr.ports[1] = (GT_U32)inFields[7];
        portsTaggingPtr.ports[2] = (GT_U32)inFields[8];
        portsTaggingPtr.ports[3] = (GT_U32)inFields[9];
        fieldOffset = 4;
    }

    cpssOsMemSet(&vlanInfoPtr, 0, sizeof(vlanInfoPtr));
    vlanInfoPtr.unregIpmEVidx = 0xFFF;

    vlanInfoPtr.unkSrcAddrSecBreach = (GT_BOOL)inFields[6+fieldOffset];
    vlanInfoPtr.unregNonIpMcastCmd = (CPSS_PACKET_CMD_ENT)inFields[7+fieldOffset];
    vlanInfoPtr.unregIpv4McastCmd = (CPSS_PACKET_CMD_ENT)inFields[8+fieldOffset];
    vlanInfoPtr.unregIpv6McastCmd = (CPSS_PACKET_CMD_ENT)inFields[9+fieldOffset];
    vlanInfoPtr.unkUcastCmd = (CPSS_PACKET_CMD_ENT)inFields[10+fieldOffset];
    vlanInfoPtr.unregIpv4BcastCmd = (CPSS_PACKET_CMD_ENT)inFields[11+fieldOffset];
    vlanInfoPtr.unregNonIpv4BcastCmd = (CPSS_PACKET_CMD_ENT)inFields[12+fieldOffset];
    vlanInfoPtr.ipv4IgmpToCpuEn = (GT_BOOL)inFields[13+fieldOffset];
    vlanInfoPtr.mirrToRxAnalyzerEn = (GT_BOOL)inFields[14+fieldOffset];
    vlanInfoPtr.ipv6IcmpToCpuEn = (GT_BOOL)inFields[15+fieldOffset];
    vlanInfoPtr.ipCtrlToCpuEn = (CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT)inFields[16+fieldOffset];
    vlanInfoPtr.ipv4IpmBrgMode = (CPSS_BRG_IPM_MODE_ENT)inFields[17+fieldOffset];
    vlanInfoPtr.ipv6IpmBrgMode = (CPSS_BRG_IPM_MODE_ENT)inFields[18+fieldOffset];
    vlanInfoPtr.ipv4IpmBrgEn = (GT_BOOL)inFields[19+fieldOffset];
    vlanInfoPtr.ipv6IpmBrgEn = (GT_BOOL)inFields[20+fieldOffset];
    vlanInfoPtr.ipv6SiteIdMode = (CPSS_IP_SITE_ID_ENT)inFields[21+fieldOffset];
    vlanInfoPtr.ipv4UcastRouteEn = (GT_BOOL)inFields[22+fieldOffset];
    vlanInfoPtr.ipv4McastRouteEn = (GT_BOOL)inFields[23+fieldOffset];
    vlanInfoPtr.ipv6UcastRouteEn = (GT_BOOL)inFields[24+fieldOffset];
    vlanInfoPtr.ipv6McastRouteEn = (GT_BOOL)inFields[25+fieldOffset];
    vlanInfoPtr.stgId = (GT_U32)inFields[26+fieldOffset];
    vlanInfoPtr.autoLearnDisable = (GT_BOOL)inFields[27+fieldOffset];
    vlanInfoPtr.naMsgToCpuEn = (GT_BOOL)inFields[28+fieldOffset];
    vlanInfoPtr.mruIdx = (GT_U32)inFields[29+fieldOffset];
    vlanInfoPtr.bcastUdpTrapMirrEn = (GT_BOOL)inFields[30+fieldOffset];

    if (numFields < 37)
    {
        vlanInfoPtr.vrfId = 0;
        vlanInfoPtr.floodVidx = 0xFFF;
        vlanInfoPtr.floodVidxMode = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;
        vlanInfoPtr.portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
        vlanInfoPtr.ucastLocalSwitchingEn = GT_FALSE;
        vlanInfoPtr.mcastLocalSwitchingEn = GT_FALSE;
    }
    else
    {
        vlanInfoPtr.vrfId = (GT_U32)inFields[31+fieldOffset];
        vlanInfoPtr.floodVidx = (GT_U16)inFields[32+fieldOffset];
        vlanInfoPtr.floodVidxMode = (CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT)inFields[33+fieldOffset];
        vlanInfoPtr.portIsolationMode = (CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT)inFields[34+fieldOffset];
        vlanInfoPtr.ucastLocalSwitchingEn = (GT_BOOL)inFields[35+fieldOffset];
        vlanInfoPtr.mcastLocalSwitchingEn = (GT_BOOL)inFields[36+fieldOffset];
    }

    if (numFields >= 50)
    {
        vlanInfoPtr.mirrToRxAnalyzerIndex = (GT_U32)inFields[41];
        vlanInfoPtr.mirrToTxAnalyzerEn = (GT_BOOL)inFields[42];
        vlanInfoPtr.mirrToTxAnalyzerIndex = (GT_U32)inFields[43];
        vlanInfoPtr.fidValue = (GT_U32)inFields[44];
        vlanInfoPtr.unknownMacSaCmd = (CPSS_PACKET_CMD_ENT)inFields[45];
        vlanInfoPtr.ipv4McBcMirrToAnalyzerEn = (GT_BOOL)inFields[46];
        vlanInfoPtr.ipv4McBcMirrToAnalyzerIndex = (GT_U32)inFields[47];
        vlanInfoPtr.ipv6McMirrToAnalyzerEn = (GT_BOOL)inFields[48];
        vlanInfoPtr.ipv6McMirrToAnalyzerIndex = (GT_U32)inFields[49];
    }

    if (numFields >= 53)
    {
        vlanInfoPtr.fcoeForwardingEn  = (GT_BOOL)inFields[50];
        vlanInfoPtr.unregIpmEVidxMode = (CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT)inFields[51];
        vlanInfoPtr.unregIpmEVidx     = (GT_U32)inFields[52];
    }
    if (numFields >= 54)
    {
        vlanInfoPtr.fdbLookupKeyMode  = (CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT)inFields[53];
        vlanInfoPtr.mirrToTxAnalyzerMode  = (CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)inFields[54];
    }
    for (i = 0; i < CPSS_MAX_PORTS_NUM_CNS; i++)
    {
        portsTaggingCmd.portsCmd[i] = (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsTaggingPtr,i))
                                    ? CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E
                                    : CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* call cpss api function */
    result = cpssDxChBrgVlanEntriesRangeWrite(devNum, vlanId, numOfEntries,
                                              &portsMembersPtr, &portsTaggingPtr,
                                              &vlanInfoPtr, &portsTaggingCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}




/*cpssDxChBrgVlan Table*/
static GT_U16   vlanIdCnt;

/**
* @internal wrCpssDxChBrgVlanEntryWrite function
* @endinternal
*
* @brief   Builds and writes vlan entry to HW .
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanEntryWrite

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                        result;

    GT_U8                            devNum;
    GT_U16                           vlanId;
    CPSS_PORTS_BMP_STC               portsMembersPtr;
    CPSS_PORTS_BMP_STC               portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_INFO_STC      vlanInfoPtr;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_U32                           i; /* loop iterator */
    GT_U32                           fieldOffset = 0;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    vlanId = (GT_U16)inFields[0];

    cmdOsMemSet(&vlanInfoPtr, 0, sizeof(vlanInfoPtr));
    vlanInfoPtr.unregIpmEVidx = 0xFFF;

    vlanInfoPtr.fidValue = vlanId;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembersPtr);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTaggingPtr);
    portsMembersPtr.ports[0] = (GT_U32)inFields[1];
    portsMembersPtr.ports[1] = (GT_U32)inFields[2];
    portsTaggingPtr.ports[0] = (GT_U32)inFields[3];
    portsTaggingPtr.ports[1] = (GT_U32)inFields[4];

    if (numFields >= 49)
    {

        portsMembersPtr.ports[2] = (GT_U32)inFields[3];
        portsMembersPtr.ports[3] = (GT_U32)inFields[4];
        portsTaggingPtr.ports[0] = (GT_U32)inFields[5];
        portsTaggingPtr.ports[1] = (GT_U32)inFields[6];
        portsTaggingPtr.ports[2] = (GT_U32)inFields[7];
        portsTaggingPtr.ports[3] = (GT_U32)inFields[8];
        fieldOffset = 4;
    }

    CONVERT_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsMembersPtr);
    CONVERT_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsTaggingPtr);

    vlanInfoPtr.unkSrcAddrSecBreach = (GT_BOOL)inFields[5+fieldOffset];
    vlanInfoPtr.unregNonIpMcastCmd = (CPSS_PACKET_CMD_ENT)inFields[6+fieldOffset];
    vlanInfoPtr.unregIpv4McastCmd = (CPSS_PACKET_CMD_ENT)inFields[7+fieldOffset];
    vlanInfoPtr.unregIpv6McastCmd = (CPSS_PACKET_CMD_ENT)inFields[8+fieldOffset];
    vlanInfoPtr.unkUcastCmd = (CPSS_PACKET_CMD_ENT)inFields[9+fieldOffset];
    vlanInfoPtr.unregIpv4BcastCmd = (CPSS_PACKET_CMD_ENT)inFields[10+fieldOffset];
    vlanInfoPtr.unregNonIpv4BcastCmd = (CPSS_PACKET_CMD_ENT)inFields[11+fieldOffset];
    vlanInfoPtr.ipv4IgmpToCpuEn = (GT_BOOL)inFields[12+fieldOffset];
    vlanInfoPtr.mirrToRxAnalyzerEn = (GT_BOOL)inFields[13+fieldOffset];
    vlanInfoPtr.ipv6IcmpToCpuEn = (GT_BOOL)inFields[14+fieldOffset];
    vlanInfoPtr.ipCtrlToCpuEn = (CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT)inFields[15+fieldOffset];
    vlanInfoPtr.ipv4IpmBrgMode = (CPSS_BRG_IPM_MODE_ENT)inFields[16+fieldOffset];
    vlanInfoPtr.ipv6IpmBrgMode = (CPSS_BRG_IPM_MODE_ENT)inFields[17+fieldOffset];
    vlanInfoPtr.ipv4IpmBrgEn = (GT_BOOL)inFields[18+fieldOffset];
    vlanInfoPtr.ipv6IpmBrgEn = (GT_BOOL)inFields[19+fieldOffset];
    vlanInfoPtr.ipv6SiteIdMode = (CPSS_IP_SITE_ID_ENT)inFields[20+fieldOffset];
    vlanInfoPtr.ipv4UcastRouteEn = (GT_BOOL)inFields[21+fieldOffset];
    vlanInfoPtr.ipv4McastRouteEn = (GT_BOOL)inFields[22+fieldOffset];
    vlanInfoPtr.ipv6UcastRouteEn = (GT_BOOL)inFields[23+fieldOffset];
    vlanInfoPtr.ipv6McastRouteEn = (GT_BOOL)inFields[24+fieldOffset];
    vlanInfoPtr.stgId = (GT_U32)inFields[25+fieldOffset];
    vlanInfoPtr.autoLearnDisable = (GT_BOOL)inFields[26+fieldOffset];
    vlanInfoPtr.naMsgToCpuEn = (GT_BOOL)inFields[27+fieldOffset];
    vlanInfoPtr.mruIdx = (GT_U32)inFields[28+fieldOffset];
    vlanInfoPtr.bcastUdpTrapMirrEn = (GT_BOOL)inFields[29+fieldOffset];
    vlanInfoPtr.vrfId = 0;/* table don't support virtual router - default*/
    if (numFields < 36)
    {
        if (numFields < 31)
        {
            vlanInfoPtr.vrfId = 0;/* table don't support virtual router */
        }
        else
        {/* table support virtual router */
            vlanInfoPtr.vrfId =(GT_U32)inFields[30+fieldOffset];
        }
        vlanInfoPtr.floodVidx = 0xFFF;
        vlanInfoPtr.floodVidxMode = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;
        vlanInfoPtr.portIsolationMode = CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
        vlanInfoPtr.ucastLocalSwitchingEn = GT_TRUE;
        vlanInfoPtr.mcastLocalSwitchingEn = GT_TRUE;
    }
    else
    {
        vlanInfoPtr.vrfId =(GT_U32)inFields[30+fieldOffset];
        vlanInfoPtr.floodVidx = (GT_U16)inFields[31+fieldOffset];
        vlanInfoPtr.floodVidxMode = (CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT)inFields[32+fieldOffset];
        vlanInfoPtr.portIsolationMode = (CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT)inFields[33+fieldOffset];
        vlanInfoPtr.ucastLocalSwitchingEn = (GT_BOOL)inFields[34+fieldOffset];
        vlanInfoPtr.mcastLocalSwitchingEn = (GT_BOOL)inFields[35+fieldOffset];
    }

    if (numFields >= 49)
    {
        vlanInfoPtr.mirrToRxAnalyzerIndex = (GT_U32)inFields[40];
        vlanInfoPtr.mirrToTxAnalyzerEn = (GT_BOOL)inFields[41];
        vlanInfoPtr.mirrToTxAnalyzerIndex = (GT_U32)inFields[42];
        vlanInfoPtr.fidValue = (GT_U32)inFields[43];
        vlanInfoPtr.unknownMacSaCmd = (CPSS_PACKET_CMD_ENT)inFields[44];
        vlanInfoPtr.ipv4McBcMirrToAnalyzerEn = (GT_BOOL)inFields[45];
        vlanInfoPtr.ipv4McBcMirrToAnalyzerIndex = (GT_U32)inFields[46];
        vlanInfoPtr.ipv6McMirrToAnalyzerEn = (GT_BOOL)inFields[47];
        vlanInfoPtr.ipv6McMirrToAnalyzerIndex = (GT_U32)inFields[48];
    }

    if (numFields >= 52)
    {
        vlanInfoPtr.fcoeForwardingEn  = (GT_BOOL)inFields[49];
        vlanInfoPtr.unregIpmEVidxMode = (CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT)inFields[50];
        vlanInfoPtr.unregIpmEVidx     = (GT_U32)inFields[51];
    }

    if (numFields >= 53)
    {
        vlanInfoPtr.fdbLookupKeyMode  = (CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT)inFields[52];
        vlanInfoPtr.mirrToTxAnalyzerMode  = (CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)inFields[53];
    }

    for (i = 0; i < CPSS_MAX_PORTS_NUM_CNS; i++)
    {
        portsTaggingCmd.portsCmd[i] = CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsTaggingPtr, i)
                                    ? CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E
                                    : CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* call cpss api function */
    result = cpssDxChBrgVlanEntryWrite(devNum,
                                       vlanId,
                                       &portsMembersPtr,
                                       &portsTaggingPtr,
                                       &vlanInfoPtr,
                                       &portsTaggingCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

static CMD_STATUS internal_wrCpssDxChBrgVlanEntryRead
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
IN  GT_BOOL vrfIdSupported,
OUT GT_8  outArgs[CMD_MAX_BUFFER]

)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    CPSS_PORTS_BMP_STC              portsMembersPtr;
    CPSS_PORTS_BMP_STC              portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_INFO_STC     vlanInfoPtr;
    GT_BOOL                         isValidPtr = GT_FALSE;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_U32                          i; /* loop iterator */

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    while ((isValidPtr == GT_FALSE) && (vlanIdCnt < CPSS_DXCH_VLAN_MAX_ENTRY_MAC(devNum)))
    {
        /* call cpss api function */
        result = cpssDxChBrgVlanEntryRead(devNum, vlanIdCnt, &portsMembersPtr,
                                          &portsTaggingPtr, &vlanInfoPtr, &isValidPtr, &portsTaggingCmd);

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported == GT_TRUE)
        {
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTaggingPtr);
            for (i = 0; i < CPSS_MAX_PORTS_NUM_CNS; i++)
            {
                if (portsTaggingCmd.portsCmd[i] != CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E)
                {
                    CPSS_PORTS_BMP_PORT_SET_MAC(&portsTaggingPtr,i);
                }
            }
        }

        if( (vlanIdCnt != 1) ||
            (0 == CPSS_PORTS_BMP_ARE_EQUAL_MAC(&portsMembersPtr,&PRV_CPSS_PP_MAC(devNum)->existingPorts)))
        {
            CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsMembersPtr);
            CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsTaggingPtr);
        }

        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }

        vlanIdCnt++;
    }

    if ((vlanIdCnt >= CPSS_DXCH_VLAN_MAX_ENTRY_MAC(devNum)) &&
        (isValidPtr == GT_FALSE) /* need to check valid to catch valid entry 4095
                                 for this entry vlanIdCnt will be ==
                                 CPSS_DXCH_VLAN_MAX_ENTRY_CNS */)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }


    inFields[0] = vlanIdCnt - 1;/* the vlanIdCnt was already incremented */
    inFields[1] = portsMembersPtr.ports[0];
    inFields[2] = portsMembersPtr.ports[1];
    inFields[3] = portsTaggingPtr.ports[0];
    inFields[4] = portsTaggingPtr.ports[1];
    inFields[5] = vlanInfoPtr.unkSrcAddrSecBreach;
    inFields[6] = vlanInfoPtr.unregNonIpMcastCmd;
    inFields[7] = vlanInfoPtr.unregIpv4McastCmd;
    inFields[8] = vlanInfoPtr.unregIpv6McastCmd;
    inFields[9] = vlanInfoPtr.unkUcastCmd;
    inFields[10] = vlanInfoPtr.unregIpv4BcastCmd;
    inFields[11] = vlanInfoPtr.unregNonIpv4BcastCmd;
    inFields[12] = vlanInfoPtr.ipv4IgmpToCpuEn;
    inFields[13] = vlanInfoPtr.mirrToRxAnalyzerEn;
    inFields[14] = vlanInfoPtr.ipv6IcmpToCpuEn;
    inFields[15] = vlanInfoPtr.ipCtrlToCpuEn;
    inFields[16] = vlanInfoPtr.ipv4IpmBrgMode;
    inFields[17] = vlanInfoPtr.ipv6IpmBrgMode;
    inFields[18] = vlanInfoPtr.ipv4IpmBrgEn;
    inFields[19] = vlanInfoPtr.ipv6IpmBrgEn;
    inFields[20] = vlanInfoPtr.ipv6SiteIdMode;
    inFields[21] = vlanInfoPtr.ipv4UcastRouteEn;
    inFields[22] = vlanInfoPtr.ipv4McastRouteEn;
    inFields[23] = vlanInfoPtr.ipv6UcastRouteEn;
    inFields[24] = vlanInfoPtr.ipv6McastRouteEn;
    inFields[25] = vlanInfoPtr.stgId;
    inFields[26] = vlanInfoPtr.autoLearnDisable;
    inFields[27] = vlanInfoPtr.naMsgToCpuEn;
    inFields[28] = vlanInfoPtr.mruIdx;
    inFields[29] = vlanInfoPtr.bcastUdpTrapMirrEn;


    if (vrfIdSupported){

        inFields[30] = vlanInfoPtr.vrfId;
        /* pack and output table fields */
        fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                    inFields[0],  inFields[1],  inFields[2],
                    inFields[3],  inFields[4],  inFields[5],
                    inFields[6],  inFields[7],  inFields[8],
                    inFields[9],  inFields[10], inFields[11],
                    inFields[12], inFields[13], inFields[14],
                    inFields[15], inFields[16], inFields[17],
                    inFields[18], inFields[19], inFields[20],
                    inFields[21], inFields[22], inFields[23],
                    inFields[24], inFields[25], inFields[26],
                    inFields[27], inFields[28], inFields[29],inFields[30]);


    }
    else{

        /* pack and output table fields */
        fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d\
            %d%d%d", inFields[0],  inFields[1],  inFields[2],
                    inFields[3],  inFields[4],  inFields[5],
                    inFields[6],  inFields[7],  inFields[8],
                    inFields[9],  inFields[10], inFields[11],
                    inFields[12], inFields[13], inFields[14],
                    inFields[15], inFields[16], inFields[17],
                    inFields[18], inFields[19], inFields[20],
                    inFields[21], inFields[22], inFields[23],
                    inFields[24], inFields[25], inFields[26],
                    inFields[27], inFields[28], inFields[29]);




    }

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

static CMD_STATUS internal_wrCpssDxChBrgVlanEntryExt1Read
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]

)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    CPSS_PORTS_BMP_STC              portsMembersPtr;
    CPSS_PORTS_BMP_STC              portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_INFO_STC     vlanInfoPtr;
    GT_BOOL                         isValidPtr = GT_FALSE;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_U32                          i; /* loop iterator */

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    while ((isValidPtr == GT_FALSE) && (vlanIdCnt < CPSS_DXCH_VLAN_MAX_ENTRY_MAC(devNum)))
    {
        /* call cpss api function */
        result = cpssDxChBrgVlanEntryRead(devNum, vlanIdCnt, &portsMembersPtr,
                                          &portsTaggingPtr, &vlanInfoPtr, &isValidPtr, &portsTaggingCmd);

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported == GT_TRUE)
        {
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTaggingPtr);
            for (i = 0; i < CPSS_MAX_PORTS_NUM_CNS; i++)
            {
                if (portsTaggingCmd.portsCmd[i] != CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E)
                {
                    CPSS_PORTS_BMP_PORT_SET_MAC(&portsTaggingPtr,i);
                }
            }
        }

        if( (vlanIdCnt != 1) ||
            (0 == CPSS_PORTS_BMP_ARE_EQUAL_MAC(&portsMembersPtr,&PRV_CPSS_PP_MAC(devNum)->existingPorts)))
        {
            CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsMembersPtr);
            CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsTaggingPtr);
        }

        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }

        vlanIdCnt++;
    }

    if ((vlanIdCnt >= CPSS_DXCH_VLAN_MAX_ENTRY_MAC(devNum)) &&
        (isValidPtr == GT_FALSE) /* need to check valid to catch valid entry 4095
                                 for this entry vlanIdCnt will be ==
                                 CPSS_DXCH_VLAN_MAX_ENTRY_CNS */)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }


    inFields[0] = vlanIdCnt - 1;/* the vlanIdCnt was already incremented */
    inFields[1] = portsMembersPtr.ports[0];
    inFields[2] = portsMembersPtr.ports[1];
    inFields[3] = portsTaggingPtr.ports[0];
    inFields[4] = portsTaggingPtr.ports[1];
    inFields[5] = vlanInfoPtr.unkSrcAddrSecBreach;
    inFields[6] = vlanInfoPtr.unregNonIpMcastCmd;
    inFields[7] = vlanInfoPtr.unregIpv4McastCmd;
    inFields[8] = vlanInfoPtr.unregIpv6McastCmd;
    inFields[9] = vlanInfoPtr.unkUcastCmd;
    inFields[10] = vlanInfoPtr.unregIpv4BcastCmd;
    inFields[11] = vlanInfoPtr.unregNonIpv4BcastCmd;
    inFields[12] = vlanInfoPtr.ipv4IgmpToCpuEn;
    inFields[13] = vlanInfoPtr.mirrToRxAnalyzerEn;
    inFields[14] = vlanInfoPtr.ipv6IcmpToCpuEn;
    inFields[15] = vlanInfoPtr.ipCtrlToCpuEn;
    inFields[16] = vlanInfoPtr.ipv4IpmBrgMode;
    inFields[17] = vlanInfoPtr.ipv6IpmBrgMode;
    inFields[18] = vlanInfoPtr.ipv4IpmBrgEn;
    inFields[19] = vlanInfoPtr.ipv6IpmBrgEn;
    inFields[20] = vlanInfoPtr.ipv6SiteIdMode;
    inFields[21] = vlanInfoPtr.ipv4UcastRouteEn;
    inFields[22] = vlanInfoPtr.ipv4McastRouteEn;
    inFields[23] = vlanInfoPtr.ipv6UcastRouteEn;
    inFields[24] = vlanInfoPtr.ipv6McastRouteEn;
    inFields[25] = vlanInfoPtr.stgId;
    inFields[26] = vlanInfoPtr.autoLearnDisable;
    inFields[27] = vlanInfoPtr.naMsgToCpuEn;
    inFields[28] = vlanInfoPtr.mruIdx;
    inFields[29] = vlanInfoPtr.bcastUdpTrapMirrEn;
    inFields[30] = vlanInfoPtr.vrfId;
    inFields[31] = vlanInfoPtr.floodVidx;
    inFields[32] = vlanInfoPtr.floodVidxMode;
    inFields[33] = vlanInfoPtr.portIsolationMode;
    inFields[34] = vlanInfoPtr.ucastLocalSwitchingEn;
    inFields[35] = vlanInfoPtr.mcastLocalSwitchingEn;

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],  inFields[2],
                inFields[3],  inFields[4],  inFields[5],
                inFields[6],  inFields[7],  inFields[8],
                inFields[9],  inFields[10], inFields[11],
                inFields[12], inFields[13], inFields[14],
                inFields[15], inFields[16], inFields[17],
                inFields[18], inFields[19], inFields[20],
                inFields[21], inFields[22], inFields[23],
                inFields[24], inFields[25], inFields[26],
                inFields[27], inFields[28], inFields[29],
                inFields[30], inFields[31], inFields[32],
                inFields[33], inFields[34], inFields[35]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/* bounds for Vlan refresh tables */
static GT_U32 vlanEntryIndexStart = 0;
static GT_U32 vlanEntryIndexHighBound = 4096;

static CMD_STATUS internal_wrCpssDxChBrgVlanEntryExt2Read
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]

)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    CPSS_PORTS_BMP_STC              portsMembers;
    CPSS_PORTS_BMP_STC              portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC     vlanInfo;
    GT_BOOL                         isValid = GT_FALSE;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_U32                          i; /* loop iterator */

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* to prevent compiler warning only */
    cpssOsMemSet(&portsMembers, 0, sizeof(portsMembers));
    cpssOsMemSet(&portsTagging, 0, sizeof(portsTagging));
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    while ((isValid == GT_FALSE) && (vlanIdCnt < vlanEntryIndexHighBound))
    {
        /* call cpss api function */
        result = cpssDxChBrgVlanEntryRead(devNum, vlanIdCnt, &portsMembers,
                                          &portsTagging, &vlanInfo, &isValid, &portsTaggingCmd);

        if (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.tr101Supported == GT_TRUE)
        {
            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
            for (i = 0; i < CPSS_MAX_PORTS_NUM_CNS; i++)
            {
                if (portsTaggingCmd.portsCmd[i] != CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E)
                {
                    CPSS_PORTS_BMP_PORT_SET_MAC(&portsTagging,i);
                }
            }
        }

        if( (vlanIdCnt != 1) /*||
            (0 == CPSS_PORTS_BMP_ARE_EQUAL_MAC(&portsMembers,&PRV_CPSS_PP_MAC(devNum)->existingPorts))*/)
        {
            CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsMembers);
            CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsTagging);
        }

        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }

        vlanIdCnt++;
    }

    if ((vlanIdCnt >= vlanEntryIndexHighBound) &&
        (isValid == GT_FALSE) /* need to check valid to catch valid entry 4095
                                 for this entry vlanIdCnt will be ==
                                 CPSS_DXCH_VLAN_MAX_ENTRY_CNS */)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }


    inFields[0] = vlanIdCnt - 1;/* the vlanIdCnt was already incremented */
    inFields[1] = portsMembers.ports[0];
    inFields[2] = portsMembers.ports[1];
    inFields[3] = portsMembers.ports[2];
    inFields[4] = portsMembers.ports[3];
    inFields[5] = portsTagging.ports[0];
    inFields[6] = portsTagging.ports[1];
    inFields[7] = portsTagging.ports[2];
    inFields[8] = portsTagging.ports[3];
    inFields[9] = vlanInfo.unkSrcAddrSecBreach;
    inFields[10] = vlanInfo.unregNonIpMcastCmd;
    inFields[11] = vlanInfo.unregIpv4McastCmd;
    inFields[12] = vlanInfo.unregIpv6McastCmd;
    inFields[13] = vlanInfo.unkUcastCmd;
    inFields[14] = vlanInfo.unregIpv4BcastCmd;
    inFields[15] = vlanInfo.unregNonIpv4BcastCmd;
    inFields[16] = vlanInfo.ipv4IgmpToCpuEn;
    inFields[17] = vlanInfo.mirrToRxAnalyzerEn;
    inFields[18] = vlanInfo.ipv6IcmpToCpuEn;
    inFields[19] = vlanInfo.ipCtrlToCpuEn;
    inFields[20] = vlanInfo.ipv4IpmBrgMode;
    inFields[21] = vlanInfo.ipv6IpmBrgMode;
    inFields[22] = vlanInfo.ipv4IpmBrgEn;
    inFields[23] = vlanInfo.ipv6IpmBrgEn;
    inFields[24] = vlanInfo.ipv6SiteIdMode;
    inFields[25] = vlanInfo.ipv4UcastRouteEn;
    inFields[26] = vlanInfo.ipv4McastRouteEn;
    inFields[27] = vlanInfo.ipv6UcastRouteEn;
    inFields[28] = vlanInfo.ipv6McastRouteEn;
    inFields[29] = vlanInfo.stgId;
    inFields[30] = vlanInfo.autoLearnDisable;
    inFields[31] = vlanInfo.naMsgToCpuEn;
    inFields[32] = vlanInfo.mruIdx;
    inFields[33] = vlanInfo.bcastUdpTrapMirrEn;
    inFields[34] = vlanInfo.vrfId;
    inFields[35] = vlanInfo.floodVidx;
    inFields[36] = vlanInfo.floodVidxMode;
    inFields[37] = vlanInfo.portIsolationMode;
    inFields[38] = vlanInfo.ucastLocalSwitchingEn;
    inFields[39] = vlanInfo.mcastLocalSwitchingEn;

    inFields[40] = vlanInfo.mirrToRxAnalyzerIndex;
    inFields[41] = vlanInfo.mirrToTxAnalyzerEn;
    inFields[42] = vlanInfo.mirrToTxAnalyzerIndex;
    inFields[43] = vlanInfo.fidValue;
    inFields[44] = vlanInfo.unknownMacSaCmd;
    inFields[45] = vlanInfo.ipv4McBcMirrToAnalyzerEn;
    inFields[46] = vlanInfo.ipv4McBcMirrToAnalyzerIndex;
    inFields[47] = vlanInfo.ipv6McMirrToAnalyzerEn;
    inFields[48] = vlanInfo.ipv6McMirrToAnalyzerIndex;

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],  inFields[2],
                inFields[3],  inFields[4],  inFields[5],
                inFields[6],  inFields[7],  inFields[8],
                inFields[9],  inFields[10], inFields[11],
                inFields[12], inFields[13], inFields[14],
                inFields[15], inFields[16], inFields[17],
                inFields[18], inFields[19], inFields[20],
                inFields[21], inFields[22], inFields[23],
                inFields[24], inFields[25], inFields[26],
                inFields[27], inFields[28], inFields[29],
                inFields[30], inFields[31], inFields[32],
                inFields[33], inFields[34], inFields[35],
                inFields[36], inFields[37], inFields[38],
                inFields[39], inFields[40], inFields[41],
                inFields[42], inFields[43], inFields[44],
                inFields[45], inFields[46], inFields[47], inFields[48]);
    if (numFields > 49)
    {
        fieldOutputSetAppendMode();
        fieldOutput(
            "%d%d%d",
            vlanInfo.fcoeForwardingEn,
            vlanInfo.unregIpmEVidxMode,
            vlanInfo.unregIpmEVidx);
    }
    if (numFields > 52)
    {
        fieldOutputSetAppendMode();
        fieldOutput("%d%d", vlanInfo.fdbLookupKeyMode, vlanInfo.mirrToTxAnalyzerMode);
    }
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

static CMD_STATUS internal_wrCpssDxChBrgVlanEntryPortMembersBmpRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    CPSS_PORTS_BMP_STC              portsMembers;
    CPSS_PORTS_BMP_STC              portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC     vlanInfo;
    GT_BOOL                         isValid = GT_FALSE;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_U32                          i; /* loop iterator */

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* to prevent compiler warning only */
    cpssOsMemSet(&portsMembers, 0, sizeof(portsMembers));
    cpssOsMemSet(&portsTagging, 0, sizeof(portsTagging));
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    while ((isValid == GT_FALSE) && (vlanIdCnt < vlanEntryIndexHighBound))
    {
        /* call cpss api function */
        result = cpssDxChBrgVlanEntryRead(
            devNum, vlanIdCnt, &portsMembers,
            &portsTagging, &vlanInfo, &isValid, &portsTaggingCmd);

        if( (vlanIdCnt != 1) /*||
            (0 == CPSS_PORTS_BMP_ARE_EQUAL_MAC(&portsMembers,&PRV_CPSS_PP_MAC(devNum)->existingPorts))*/)
        {
            CONVERT_BACK_DEV_PHYSICAL_PORTS_BMP_MAC(devNum,portsMembers);
        }

        if (result != GT_OK)
        {
            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }

        vlanIdCnt++;
    }

    if ((vlanIdCnt >= vlanEntryIndexHighBound) &&
        (isValid == GT_FALSE) /* need to check valid to catch valid entry 4095
                                 for this entry vlanIdCnt will be ==
                                 CPSS_DXCH_VLAN_MAX_ENTRY_CNS */)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }


    inFields[0] = vlanIdCnt - 1;/* the vlanIdCnt was already incremented */
    for (i = 0; (i < 32); i++)
    {
        inFields[i + 1] = portsMembers.ports[i];
    }

    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0],  inFields[1],  inFields[2],
                inFields[3],  inFields[4],  inFields[5],
                inFields[6],  inFields[7],  inFields[8],
                inFields[9],  inFields[10], inFields[11],
                inFields[12], inFields[13], inFields[14],
                inFields[15], inFields[16], inFields[17],
                inFields[18], inFields[19], inFields[20],
                inFields[21], inFields[22], inFields[23],
                inFields[24], inFields[25], inFields[26],
                inFields[27], inFields[28], inFields[29],
                inFields[30], inFields[31], inFields[32]);
    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

static CMD_STATUS internal_wrCpssDxChBrgVlanEntryPortMembersBmpWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;

    GT_U8                           devNum;
    CPSS_PORTS_BMP_STC              portsMembers;
    CPSS_PORTS_BMP_STC              portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC     vlanInfo;
    GT_BOOL                         isValid = GT_FALSE;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_U32                          i; /* loop iterator */
    GT_U16                          vlanId;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId =  (GT_U16)inFields[0];

    /* to prevent compiler warning only */
    cpssOsMemSet(&portsMembers, 0, sizeof(portsMembers));
    cpssOsMemSet(&portsTagging, 0, sizeof(portsTagging));
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    /* call cpss api function */
    result = cpssDxChBrgVlanEntryRead(
        devNum, vlanId, &portsMembers,
        &portsTagging, &vlanInfo, &isValid, &portsTaggingCmd);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d");
        return CMD_OK;
    }

    /* replace member ports read from HW by fiels data */
    for (i = 0; (i < 32); i++)
    {
        portsMembers.ports[i] = inFields[i + 1];
    }

    /* write back */
    result = cpssDxChBrgVlanEntryWrite(
        devNum, vlanId, &portsMembers,
        &portsTagging, &vlanInfo, &portsTaggingCmd);

    galtisOutput(outArgs, result, "%d");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanEntryReadFirst function
* @endinternal
*
* @brief   Read vlan entry.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanEntryReadFirst

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    vlanIdCnt = 0;

    return internal_wrCpssDxChBrgVlanEntryRead(inArgs,inFields,numFields,GT_FALSE,outArgs);
}


/*******************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryReadNext
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internal_wrCpssDxChBrgVlanEntryRead(inArgs,inFields,numFields,GT_FALSE,outArgs);
}
/********************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExtReadFirst

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    vlanIdCnt = 0;

    return internal_wrCpssDxChBrgVlanEntryRead(inArgs,inFields,numFields,GT_TRUE,outArgs);
}


/*******************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExtReadNext
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internal_wrCpssDxChBrgVlanEntryRead(inArgs,inFields,numFields,GT_TRUE,outArgs);
}



/********************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExt1ReadFirst

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    vlanIdCnt = 0;

    return internal_wrCpssDxChBrgVlanEntryExt1Read(inArgs, inFields, numFields, outArgs);
}


/*******************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExt1ReadNext
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internal_wrCpssDxChBrgVlanEntryExt1Read(inArgs, inFields, numFields, outArgs);
}

/********************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExt2ReadFirst

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8 devNum;

    devNum = (GT_U8)inArgs[0];
    vlanEntryIndexStart     = 0;
    vlanEntryIndexHighBound = CPSS_DXCH_VLAN_MAX_ENTRY_MAC(devNum);
    vlanIdCnt               = vlanEntryIndexStart;

    return internal_wrCpssDxChBrgVlanEntryExt2Read(inArgs, inFields, 49 /*numFields*/, outArgs);
}


/*******************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExt2ReadNext
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internal_wrCpssDxChBrgVlanEntryExt2Read(inArgs, inFields, 49 /*numFields*/, outArgs);
}

/********************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExt3ReadFirst

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8 devNum;

    devNum = (GT_U8)inArgs[0];
    vlanEntryIndexStart     = 0;
    vlanEntryIndexHighBound = CPSS_DXCH_VLAN_MAX_ENTRY_MAC(devNum);
    vlanIdCnt               = vlanEntryIndexStart;

    return internal_wrCpssDxChBrgVlanEntryExt2Read(inArgs, inFields, 52 /*numFields*/, outArgs);
}


/*******************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExt3ReadNext
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internal_wrCpssDxChBrgVlanEntryExt2Read(inArgs, inFields, 52 /*numFields*/, outArgs);
}

/********************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExt4ReadFirst

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8 devNum;

    devNum = (GT_U8)inArgs[0];
    vlanEntryIndexStart     = 0;
    vlanEntryIndexHighBound = CPSS_DXCH_VLAN_MAX_ENTRY_MAC(devNum);
    vlanIdCnt               = vlanEntryIndexStart;

    return internal_wrCpssDxChBrgVlanEntryExt2Read(inArgs, inFields, 54 /*numFields*/, outArgs);
}


/*******************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExt4ReadNext
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internal_wrCpssDxChBrgVlanEntryExt2Read(inArgs, inFields, 54 /*numFields*/, outArgs);
}

/********************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryExt4ReadRangeFirst

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    vlanEntryIndexStart     = (GT_U32)inArgs[1];
    vlanEntryIndexHighBound = vlanEntryIndexStart + (GT_U32)inArgs[2];
    vlanIdCnt               = vlanEntryIndexStart;

    return internal_wrCpssDxChBrgVlanEntryExt2Read(inArgs, inFields, 54 /*numFields*/, outArgs);
}

/********************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanEntryMembersBmpReadRangeFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    vlanEntryIndexStart     = (GT_U32)inArgs[1];
    vlanEntryIndexHighBound = vlanEntryIndexStart + (GT_U32)inArgs[2];
    vlanIdCnt               = vlanEntryIndexStart;

    return internal_wrCpssDxChBrgVlanEntryPortMembersBmpRead(
        inArgs, inFields, numFields, outArgs);
}

static CMD_STATUS wrCpssDxChBrgVlanEntryMembersBmpReadRangeNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internal_wrCpssDxChBrgVlanEntryPortMembersBmpRead(
        inArgs, inFields, numFields, outArgs);
}

static CMD_STATUS wrCpssDxChBrgVlanEntryMembersBmpWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return internal_wrCpssDxChBrgVlanEntryPortMembersBmpWrite(
        inArgs, inFields, numFields, outArgs);
}

/**
* @internal wrCpssDxChBrgVlanMemberAdd function
* @endinternal
*
* @brief   Add new port member to vlan entry. This function can be called only for
*         add port belongs to device that already member of the vlan.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vid
*
* @note In case that added port belongs to device that is new device for vlan
*       other function is used - cpssDxChBrgVlanEntryWrite.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanMemberAdd
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_PHYSICAL_PORT_NUM                         portNum;
    GT_BOOL                       isTagged;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT taggingCmd;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];
    isTagged = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    taggingCmd = (isTagged == GT_TRUE) ? CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E : CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;

    /* call cpss api function */
    result = cpssDxChBrgVlanMemberAdd(devNum, vlanId, portNum, isTagged, taggingCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanMemberAdd1 function
* @endinternal
*
* @brief   Add new port member to vlan entry. This function can be called only for
*         add port belongs to device that already member of the vlan.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vid or taggingCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In case that added port belongs to device that is new device for vlan
*       other function is used - cpssDxChBrgVlanEntryWrite.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanMemberAdd1
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_PHYSICAL_PORT_NUM          portNum;
    GT_BOOL                       isTagged;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT taggingCmd;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];
    isTagged = (GT_BOOL)inArgs[3];
    taggingCmd = (CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT)inArgs[4];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanMemberAdd(devNum, vlanId, portNum, isTagged, taggingCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanPortDelete function
* @endinternal
*
* @brief   Delete port member from vlan entry.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
*/
static CMD_STATUS wrCpssDxChBrgVlanPortDelete

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_PHYSICAL_PORT_NUM                         portNum;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortDelete(devNum, vlanId, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanMemberSet function
* @endinternal
*
* @brief   Set specific member at VLAN entry.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or taggingCmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanMemberSet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_PHYSICAL_PORT_NUM                         portNum;
    GT_BOOL                       isMember;
    GT_BOOL                       isTagged;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT taggingCmd;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[2];
    isMember = (GT_BOOL)inArgs[3];
    isTagged = (GT_BOOL)inArgs[4];
    taggingCmd = (CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT)inArgs[5];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanMemberSet(devNum, vlanId, portNum, isMember, isTagged, taggingCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal prv_wrCpssDxChBrgVlanPortVidGet function
* @endinternal
*
* @brief   For ingress direction : Get port's default VID0.
*         For egress direction : Get port's VID0 when egress port <VID0 Command>=Enabled
*         APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number
*         portNum - port number
*         direction - ingress/egress direction
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or port or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS prv_wrCpssDxChBrgVlanPortVidGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN GT_U32 version
)
{
    GT_STATUS           result;

    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    CPSS_DIRECTION_ENT  direction;
    GT_U16              vidPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    if ( 0 == version )
    {
        direction = CPSS_DIRECTION_INGRESS_E;
    }
    else
    {
        direction = (CPSS_DIRECTION_ENT)inArgs[2];
    }

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortVidGet(devNum, portNum, direction, &vidPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", vidPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortVidGet function
* @endinternal
*
* @brief   Get port's default VLAN Id.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVidGet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prv_wrCpssDxChBrgVlanPortVidGet(inArgs, inFields, numFields,
                                           outArgs, 0);
}

/**
* @internal wrCpssDxChBrgVlanPortVidGetExt function
* @endinternal
*
* @brief   For ingress direction : Get port's default VID0.
*         For egress direction : Get port's VID0 when egress port <VID0 Command>=Enabled
*         APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number
*         portNum - port number
*         direction - ingress/egress direction
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or port or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVidGetExt
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prv_wrCpssDxChBrgVlanPortVidGet(inArgs, inFields, numFields,
                                           outArgs, 1);
}

/**
* @internal prv_wrCpssDxChBrgVlanPortVidSet function
* @endinternal
*
* @brief   For ingress direction : Set port's default VID0.
*         For egress direction : Set port's VID0 when egress port <VID0 Command>=Enabled
*         APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number
*         portNum - port number
*         direction - ingress/egress direction
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS prv_wrCpssDxChBrgVlanPortVidSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN GT_U32 version
)
{
    GT_STATUS           result;

    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    CPSS_DIRECTION_ENT  direction;
    GT_U16              vlanId;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    if ( 0 == version )
    {
        direction = CPSS_DIRECTION_INGRESS_E;
        vlanId = (GT_U16)inArgs[2];
    }
    else
    {
        direction = (CPSS_DIRECTION_ENT)inArgs[2];
        vlanId = (GT_U16)inArgs[3];
    }

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortVidSet(devNum, portNum, direction, vlanId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortVidSet function
* @endinternal
*
* @brief   Set port's default VLAN Id.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVidSet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prv_wrCpssDxChBrgVlanPortVidSet(inArgs, inFields, numFields,
                                           outArgs, 0);
}

/**
* @internal wrCpssDxChBrgVlanPortVidSetExt function
* @endinternal
*
* @brief   For ingress direction : Set port's default VID0.
*         For egress direction : Set port's VID0 when egress port <VID0 Command>=Enabled
*         APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         None.
*         INPUTS:
*         devNum  - device number
*         portNum - port number
*         direction - ingress/egress direction
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum port, or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVidSetExt
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return prv_wrCpssDxChBrgVlanPortVidSet(inArgs, inFields, numFields,
                                           outArgs, 1);
}

/**
* @internal wrCpssDxChBrgVlanPortIngFltEnable function
* @endinternal
*
* @brief   Enable/disable Ingress Filtering for specific port
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
*/
static CMD_STATUS wrCpssDxChBrgVlanPortIngFltEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_PORT_NUM                         portNum;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortIngFltEnable(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortIngFltEnableGet function
* @endinternal
*
* @brief   Get status of Ingress Filtering for specific port
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortIngFltEnableGet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;

    GT_U8            devNum;
    GT_PORT_NUM            portNum;
    GT_BOOL          enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortIngFltEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortProtoClassVlanEnable function
* @endinternal
*
* @brief   Enable/Disable Port Protocol VLAN assignment to packets received
*         on this port,according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
*/
static CMD_STATUS wrCpssDxChBrgVlanPortProtoClassVlanEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_PORT_NUM                   portNum;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortProtoClassVlanEnable(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortProtoClassVlanEnableGet function
* @endinternal
*
* @brief   Get status of Port Protocol VLAN assignment to packets received
*         on this port,according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChBrgVlanPortProtoClassVlanEnableGet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_PORT_NUM                   portNum;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortProtoClassVlanEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortProtoClassQosEnable function
* @endinternal
*
* @brief   Enable/Disable Port Protocol QoS Assignment to packets received on this
*         port, according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
*/
static CMD_STATUS wrCpssDxChBrgVlanPortProtoClassQosEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_PORT_NUM                   portNum;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortProtoClassQosEnable(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortProtoClassQosEnableGet function
* @endinternal
*
* @brief   Get status of Port Protocol QoS Assignment to packets received on this
*         port, according to it's Layer2 protocol
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChBrgVlanPortProtoClassQosEnableGet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_PORT_NUM                   portNum;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortProtoClassQosEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}


/*cpssDxChBrgVlanProtoClass Table*/
static GT_U32   entryCnt;


/**
* @internal wrCpssDxChBrgVlanProtoClassSet function
* @endinternal
*
* @brief   Write etherType and encapsulation of Protocol based classification
*         for specific device.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanProtoClassSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;

    GT_U8                          devNum;
    GT_U32                         entryNum;
    GT_U16                         etherType;
    CPSS_PROT_CLASS_ENCAP_STC      encListPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    entryNum = (GT_U32)inFields[0];
    etherType = (GT_U16)inFields[1];

    encListPtr.ethV2 = (GT_BOOL)inFields[2];
    encListPtr.nonLlcSnap = (GT_BOOL)inFields[3];
    encListPtr.llcSnap = (GT_BOOL)inFields[4];

    /* call cpss api function */
    result = cpssDxChBrgVlanProtoClassSet(devNum, entryNum, etherType,
                                          &encListPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/*******************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanProtoClassGetNext
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;

    GT_U8                          devNum;
    GT_U16                         etherTypePtr;
    CPSS_PROT_CLASS_ENCAP_STC      encListPtr;
    GT_BOOL                        validEntryPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    if (entryCnt >= CPSS_DXCH_VLAN_MAX_ENTRY_MAC(devNum))
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChBrgVlanProtoClassGet(devNum, entryCnt, &etherTypePtr,
                                          &encListPtr, &validEntryPtr);
    if (result == GT_BAD_PARAM)
    {
        /* no more protocols */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if (!validEntryPtr)
    {
        /* entry is not valid. Use default HW output */
        etherTypePtr            = 0;
        encListPtr.ethV2        = GT_FALSE;
        encListPtr.nonLlcSnap   = GT_FALSE;
        encListPtr.llcSnap      = GT_FALSE;
    }

    inFields[0] = entryCnt;
    inFields[1] = etherTypePtr;
    inFields[2] = encListPtr.ethV2;
    inFields[3] = encListPtr.nonLlcSnap;
    inFields[4] = encListPtr.llcSnap;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d", inFields[0], inFields[1],
                inFields[2], inFields[3], inFields[4]);

    galtisOutput(outArgs, GT_OK, "%f");

    entryCnt++;

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanProtoClassGetFirst function
* @endinternal
*
* @brief   Read etherType and encapsulation of Protocol based classification
*         for specific device.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanProtoClassGetFirst

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    entryCnt = 0;

    return wrCpssDxChBrgVlanProtoClassGetNext(inArgs, inFields, numFields, outArgs);
}


/**
* @internal wrCpssDxChBrgVlanProtoClassInvalidate function
* @endinternal
*
* @brief   Invalidates entry of Protocol based classification for specific device.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanProtoClassInvalidate

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U32                        entryNum;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    entryNum = (GT_U32)inFields[0];

    /* call cpss api function */
    result = cpssDxChBrgVlanProtoClassInvalidate(devNum, entryNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/*cpssDxChBrgVlanProtoClass Table*/
static GT_U32   entryNumCnt;

/**
* @internal wrCpssDxChBrgVlanPortProtoVlanQosSet function
* @endinternal
*
* @brief   Write Port Protocol VID and QOS Assignment values for specific device
*         and port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
*
* @note It is possible to update only VLAN or QOS parameters, when one
*       of those input pointers is NULL.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanPortProtoVlanQosSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                  result;

    GT_U8                                      devNum;
    GT_PORT_NUM                       port;
    GT_U32                                     entryNum;
    CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC      vlanCfgPtr;
    CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC       qosCfgPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    entryNum = (GT_U32)inFields[0];

    vlanCfgPtr.vlanId = (GT_U16)inFields[1];
    vlanCfgPtr.vlanIdAssignCmd =
    (CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT)inFields[2];
    vlanCfgPtr.vlanIdAssignPrecedence =
    (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[3];

    qosCfgPtr.qosAssignCmd =  (CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT)inFields[4];
    qosCfgPtr.qosProfileId = (GT_U32)inFields[5];
    qosCfgPtr.qosAssignPrecedence =
    (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inFields[6];
    qosCfgPtr.enableModifyUp = (GT_BOOL)inFields[7];
    qosCfgPtr.enableModifyDscp = (GT_BOOL)inFields[8];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortProtoVlanQosSet(devNum, port, entryNum,
                                                &vlanCfgPtr, &qosCfgPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrPrvCpssDxChBrgVlanPortProtoVlanQosGetNext function
* @endinternal
*
* @brief   Get Port Protocol VID and QOS Assignment values for specific device
*         and port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
*
* @note It is possible to get only one of VLAN or QOS parameters by receiving
*       one of those pointers as NULL pointer.
*
*/
static CMD_STATUS wrPrvCpssDxChBrgVlanPortProtoVlanQosGetNext

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                result;

    GT_U8                                    devNum;
    GT_PORT_NUM                              port;
    CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC    vlanCfgPtr;
    CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC     qosCfgPtr;
    GT_BOOL                                  validEntry;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    validEntry = GT_FALSE;

    while (validEntry == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChBrgVlanPortProtoVlanQosGet(devNum, port, entryNumCnt,
                                                    &vlanCfgPtr, &qosCfgPtr, &validEntry);

        if (result != GT_OK)
        {
            if ((entryNumCnt > 7) && (result == GT_BAD_PARAM))
            {
                /* this is end of table */
                result = GT_OK;
            }

            galtisOutput(outArgs, result, "%d", -1);
            return CMD_OK;
        }
        if (validEntry == GT_FALSE)
        {
            /* check next entry */
            entryNumCnt++;
        }
    }

    inFields[0] = entryNumCnt;
    inFields[1] = vlanCfgPtr.vlanId;
    inFields[2] = vlanCfgPtr.vlanIdAssignCmd;
    inFields[3] = vlanCfgPtr.vlanIdAssignPrecedence;
    inFields[4] = qosCfgPtr.qosAssignCmd;
    inFields[5] = qosCfgPtr.qosProfileId;
    inFields[6] = qosCfgPtr.qosAssignPrecedence;
    inFields[7] = qosCfgPtr.enableModifyUp;
    inFields[8] = qosCfgPtr.enableModifyDscp;

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1], inFields[2],
                inFields[3], inFields[4], inFields[5],
                inFields[6], inFields[7], inFields[8]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortProtoVlanQosGetFirst function
* @endinternal
*
* @brief   Get Port Protocol VID and QOS Assignment values for specific device
*         and port.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or vlanId
*
* @note It is possible to get only one of VLAN or QOS parameters by receiving
*       one of those pointers as NULL pointer.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanPortProtoVlanQosGetFirst

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    entryNumCnt = 0;
    return wrPrvCpssDxChBrgVlanPortProtoVlanQosGetNext(inArgs, inFields,
                                                       numFields, outArgs);
}


/*******************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanPortProtoVlanQosGetNext
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    entryNumCnt++;

    return wrPrvCpssDxChBrgVlanPortProtoVlanQosGetNext(inArgs, inFields,
                                                       numFields, outArgs);
}


/**
* @internal wrCpssDxChBrgVlanPortProtoVlanQosInvalidate function
* @endinternal
*
* @brief   Invalidate protocol based QoS and VLAN assignment entry for
*         specific port
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgVlanPortProtoVlanQosInvalidate

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_PORT_NUM                         portNum;
    GT_U32                        entryNum;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    entryNum = (GT_U32)inFields[0];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortProtoVlanQosInvalidate(devNum, portNum,
                                                       entryNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanLearningStateSet function
* @endinternal
*
* @brief   Sets state of VLAN based learning to specified VLAN on specified device.
*
* @note   APPLICABLE DEVICES:      98DX2x5 only
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
*/
static CMD_STATUS wrCpssDxChBrgVlanLearningStateSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        portNum;
    GT_BOOL                       status;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_U16)inArgs[1];
    status = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanLearningStateSet(devNum, portNum, status);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}



/**
* @internal wrCpssDxChBrgVlanIsDevMember function
* @endinternal
*
* @brief   Checks if specified device has members belong to specified vlan
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_TRUE                  - if port is member of vlan
* @retval GT_FALSE                 - if port does not member of vlan
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
*/
static CMD_STATUS wrCpssDxChBrgVlanIsDevMember

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanIsDevMember(devNum, vlanId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanToStpIdBind function
* @endinternal
*
* @brief   Bind VLAN to STP Id.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
*/
static CMD_STATUS wrCpssDxChBrgVlanToStpIdBind

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_U16                        stpId;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    stpId = (GT_U16)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanToStpIdBind(devNum, vlanId, stpId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanStpIdGet function
* @endinternal
*
* @brief   Read STP Id that bind to specified VLAN.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanStpIdGet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_U16                        stpIdPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanStpIdGet(devNum, vlanId, &stpIdPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", stpIdPtr);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanEtherTypeSet function
* @endinternal
*
* @brief   Sets vlan Ether Type mode and vlan range.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal vidRange value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vidRange
*
* @note If the vid of the incoming packet is greater or equal (>=) than the vlan
*       range value, the packet is filtered.
*       Configuration of vid and range for CPSS_VLAN_ETHERTYPE1_E should be done
*       only when nested vlan is used. User can alter the etherType field of
*       the encapsulating vlan by setting a number other then 0x8100 in the
*       parameter <ethertype>.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanEtherTypeSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    CPSS_ETHER_MODE_ENT           etherTypeMode;
    GT_U16                        etherType;
    GT_U16                        vidRange;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    etherTypeMode = (CPSS_ETHER_MODE_ENT)inArgs[1];
    etherType = (GT_U16)inArgs[2];
    vidRange = (GT_U16)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanEtherTypeSet(devNum, etherTypeMode,
                                         etherType, vidRange);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanEtherTypeGet function
* @endinternal
*
* @brief   Gets vlan Ether Type mode and vlan range.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal vidRange value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChBrgVlanEtherTypeGet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    CPSS_ETHER_MODE_ENT           etherTypeMode;
    GT_U16                        etherType;
    GT_U16                        vidRange;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    etherTypeMode = (CPSS_ETHER_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanEtherTypeGet(devNum, etherTypeMode,
                                         &etherType, &vidRange);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", etherType, vidRange);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanTableInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN Table entries by writing 0 to the
*         first word.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum
*/
static CMD_STATUS wrCpssDxChBrgVlanTableInvalidate

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgVlanTableInvalidate(devNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanEntryInvalidate function
* @endinternal
*
* @brief   This function invalidates VLAN entry.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
*/
static CMD_STATUS wrCpssDxChBrgVlanEntryInvalidate

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanEntryInvalidate(devNum, vlanId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanUnkUnregFilterSet function
* @endinternal
*
* @brief   Set per VLAN filtering command for specified Unknown or Unregistered
*         packet type
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or vlanId
*/
static CMD_STATUS wrCpssDxChBrgVlanUnkUnregFilterSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;

    GT_U8                                   devNum;
    GT_U16                                  vlanId;
    CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT      packetType;
    CPSS_PACKET_CMD_ENT                     cmd;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    packetType = (CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT)inArgs[2];
    cmd = (CPSS_PACKET_CMD_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanUnkUnregFilterSet(devNum, vlanId, packetType, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanForcePvidEnable function
* @endinternal
*
* @brief   Set Port VID Assignment mode.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgVlanForcePvidEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_PORT_NUM                         port;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanForcePvidEnable(devNum, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanForcePvidEnableGet function
* @endinternal
*
* @brief   Get Port VID Assignment mode.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanForcePvidEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_PORT_NUM                         port;
    GT_BOOL                       enable;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanForcePvidEnableGet(devNum, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;

}

/**
* @internal wrCpssDxChBrgVlanPortVidPrecedenceSet function
* @endinternal
*
* @brief   Set Port VID Precedence.
*         Relevant for packets with assigned VLAN ID of Pvid.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVidPrecedenceSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;

    GT_U8                                          devNum;
    GT_PORT_NUM                                          port;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT    precedence;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    precedence = (CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortVidPrecedenceSet(devNum, port, precedence);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortVidPrecedenceGet function
* @endinternal
*
* @brief   Get Port VID Precedence.
*         Relevant for packets with assigned VLAN ID of Pvid.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVidPrecedenceGet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                      result;

    GT_U8                                          devNum;
    GT_PORT_NUM                                          port;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT    precedence;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortVidPrecedenceGet(devNum, port, &precedence);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", precedence);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIpUcRouteEnable function
* @endinternal
*
* @brief   Enable/Disable IPv4/Ipv6 Unicast Routing on Vlan
*         APPLIOCABLE DEVICES: All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgVlanIpUcRouteEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    CPSS_IP_PROTOCOL_STACK_ENT    protocol;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    protocol = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpUcRouteEnable(devNum, vlanId, protocol, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanIpMcRouteEnable function
* @endinternal
*
* @brief   Enable/Disable IPv4/Ipv6 Multicast Routing on Vlan
*
* @note   APPLICABLE DEVICES:      98DX2x5 only
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgVlanIpMcRouteEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    CPSS_IP_PROTOCOL_STACK_ENT    protocol;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    protocol = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpMcRouteEnable(devNum, vlanId, protocol, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanNASecurEnable function
* @endinternal
*
* @brief   This function enables/disables per VLAN the generation of
*         security breach event for packets with unknown Source MAC addresses.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgVlanNASecurEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanNASecurEnable(devNum, vlanId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanIgmpSnoopingEnable function
* @endinternal
*
* @brief   Enable/Disable IGMP trapping or mirroring to the CPU according to the
*         global setting.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device number
*/
static CMD_STATUS wrCpssDxChBrgVlanIgmpSnoopingEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanIgmpSnoopingEnable(devNum, vlanId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanIgmpSnoopingEnableGet function
* @endinternal
*
* @brief   Gets the status of IGMP trapping or mirroring to the CPU according to the
*         global setting.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on on bad device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIgmpSnoopingEnableGet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanIgmpSnoopingEnableGet(devNum, vlanId, &enable);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIpCntlToCpuSet function
* @endinternal
*
* @brief   Enable/disable IP control traffic trapping/mirroring to CPU.
*         When set, this enables the following control traffic to be eligible
*         for mirroring/trapping to the CPU:
*         - ARP
*         - IPv6 Neighbor Solicitation
*         - IPv4/v6 Control Protocols Running Over Link-Local Multicast
*         - RIPv1 MAC Broadcast
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP control type
* @retval GT_NOT_SUPPORTED         - request is not supported
*/
static CMD_STATUS wrCpssDxChBrgVlanIpCntlToCpuSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    GT_U16                              vlanId;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      ipCntrlType;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    ipCntrlType = (CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpCntlToCpuSet(devNum, vlanId, ipCntrlType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIpCntlToCpuGet function
* @endinternal
*
* @brief   Gets IP control traffic trapping/mirroring to CPU status.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or IP control type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIpCntlToCpuGet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    GT_U16                              vlanId;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT      ipCntrlType;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpCntlToCpuGet(devNum, vlanId, &ipCntrlType);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", ipCntrlType);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIpV6IcmpToCpuEnable function
* @endinternal
*
* @brief   Enable/Disable ICMPv6 trapping or mirroring to
*         the CPU, according to global ICMPv6 message type
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
*/
static CMD_STATUS wrCpssDxChBrgVlanIpV6IcmpToCpuEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpV6IcmpToCpuEnable(devNum, vlanId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIpV6IcmpToCpuEnableGet function
* @endinternal
*
* @brief   Gets status of ICMPv6 trapping or mirroring to
*         the CPU, according to global ICMPv6 message type
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIpV6IcmpToCpuEnableGet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpV6IcmpToCpuEnableGet(devNum, vlanId, &enable);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanUdpBcPktsToCpuEnable function
* @endinternal
*
* @brief   Enable or disable per Vlan the Broadcast UDP packets Mirror/Trap to the
*         CPU based on their destination UDP port. Destination UDP port, command
*         (trap or mirror) and CPU code configured by
*         cpssDxChBrgVlanUdpBcPktsToCpuEnable
*
* @note   APPLICABLE DEVICES:      98DX2x5 only
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
*/
static CMD_STATUS wrCpssDxChBrgVlanUdpBcPktsToCpuEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanUdpBcPktsToCpuEnable(devNum, vlanId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanUdpBcPktsToCpuEnableGet function
* @endinternal
*
* @brief   Gets per Vlan if the Broadcast UDP packets are Mirror/Trap to the
*         CPU based on their destination UDP port.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanUdpBcPktsToCpuEnableGet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanUdpBcPktsToCpuEnableGet(devNum, vlanId, &enable);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIpv6SourceSiteIdSet function
* @endinternal
*
* @brief   Sets a vlan ipv6 site id
*
* @note   APPLICABLE DEVICES:      98DX2x5 only
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
*
* @note none
*
*/
static CMD_STATUS wrCpssDxChBrgVlanIpv6SourceSiteIdSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    CPSS_IP_SITE_ID_ENT           siteId;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    siteId = (CPSS_IP_SITE_ID_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpv6SourceSiteIdSet(devNum, vlanId, siteId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanIpmBridgingEnable function
* @endinternal
*
* @brief   IPv4/Ipv6 Multicast Bridging Enable
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
*
* @note In xCat3, Caelum, Aldrin, AC3X and Bobcat3; Aldrin2; Falcon devices, this API also takes part
*       in enabling the IP multicast routing by VLAN feature according to the
*       following logic:
*       BrgEnable  |     BrgMode     |  Bridging  |  Routing
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_SGV_E (0) |  MAC DA   |  VRF-ID
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_GV_E (1) |  MAC DA   |   VID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_SGV_E (0) | (S,G,V)   |  VRF-ID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_GV_E (1) | (,G,V)   |  VRF-ID
*       explanation for the table:
*       'BrgEnable' - set by this API (per VLAN, per IP protocol).
*       'BrgMode' - set by cpssDxChBrgVlanIpmBridgingModeSet (per VLAN, per IP protocol).
*       'Bridging' - multicast bridging mode, relevant to all devices.
*       'Routing' - multicast routing mode, relevant only to xCat3, Caelum, Aldrin, AC3X and Bobcat3
*       devices. There are 2 possible vrf-id assignments for the ingress packet descriptor:
*       1. VRF-ID: IP multicast routing by VLAN feature is off and vrf-id is assigned according
*       to the VRF-ID field in the VLAN entry.
*       2. VID: IP multicast routing by VLAN feature is on and vrf-id is assigned to be the
*       ingress packet's VID itself.
*       In devices that do not support IP multicast routing by VLAN feature, routing mode is always VRF-ID.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanIpmBridgingEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    CPSS_IP_PROTOCOL_STACK_ENT    ipVer;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    ipVer = (GT_U8)inArgs[2];
    enable = (GT_U8)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpmBridgingEnable(devNum, vlanId, ipVer, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanIpmBridgingModeSet function
* @endinternal
*
* @brief   Sets the IPM bridging mode of Vlan. Relevant when IPM Bridging enabled.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
*
* @note In xCat3, Caelum, Aldrin, AC3X and Bobcat3; Aldrin2; Falcon devices, this API also takes part
*       in enabling the IP multicast routing by VLAN feature according to the
*       following logic:
*       BrgEnable  |     BrgMode     |  Bridging  |  Routing
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_SGV_E (0) |  MAC DA   |  VRF-ID
*       ======================================================================
*       GT_FALSE (0) | CPSS_BRG_IPM_GV_E (1) |  MAC DA   |   VID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_SGV_E (0) | (S,G,V)   |  VRF-ID
*       ======================================================================
*       GT_TRUE (1) | CPSS_BRG_IPM_GV_E (1) | (,G,V)   |  VRF-ID
*       explanation for the table:
*       'BrgEnable' - set by cpssDxChBrgVlanIpmBridgingEnable (per VLAN, per IP protocol).
*       'BrgMode' - set by this API (per VLAN, per IP protocol).
*       'Bridging' - multicast bridging mode, relevant to all devices.
*       'Routing' - multicast routing mode, relevant only to xCat3, Caelum, Aldrin, AC3X and Bobcat3
*       devices. There are 2 possible vrf-id assignments for the ingress packet descriptor:
*       1. VRF-ID: IP multicast routing by VLAN feature is off and vrf-id is assigned according
*       to the VRF-ID field in the VLAN entry.
*       2. VID: IP multicast routing by VLAN feature is on and vrf-id is assigned to be the
*       ingress packet's VID itself.
*       In devices that do not support IP multicast routing by VLAN feature, routing mode is always VRF-ID.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanIpmBridgingModeSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    CPSS_IP_PROTOCOL_STACK_ENT    ipVer;
    CPSS_BRG_IPM_MODE_ENT         ipmMode;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    ipVer = (CPSS_IP_PROTOCOL_STACK_ENT)inArgs[2];
    ipmMode = (CPSS_BRG_IPM_MODE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpmBridgingModeSet(devNum, vlanId, ipVer, ipmMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanIngressMirrorEnable function
* @endinternal
*
* @brief   Enable/Disable ingress mirroring of packets to the Ingress Analyzer
*         port for packets assigned the given VLAN-ID.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
*/
static CMD_STATUS wrCpssDxChBrgVlanIngressMirrorEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                         enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanIngressMirrorEnable(devNum, vlanId, enable,0);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIngressMirrorEnableExt function
* @endinternal
*
* @brief   Enable/Disable ingress mirroring of packets to the Ingress Analyzer
*         port for packets assigned the given VLAN-ID.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIngressMirrorEnableExt
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;
    GT_U32                        index;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    index  = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanIngressMirrorEnable(
        devNum, vlanId, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanMirrToTxAnalyzerSet function
* @endinternal
*
* @brief   Function sets Tx Analyzer port index to given VLAN.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanMirrToTxAnalyzerSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;
    GT_U32                        index;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    index  = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanMirrorToTxAnalyzerSet(
        devNum, vlanId, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanForwardingIdSet function
* @endinternal
*
* @brief   Set Forwarding ID value.
*         Used for virtual birdges per eVLAN.
*         Used by the bridge engine for entry lookup and entry match (replaces VID
*         in bridge entry)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanForwardingIdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_U32                        fid;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    fid  = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanForwardingIdSet(
        devNum, vlanId, fid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanForwardingIdGet function
* @endinternal
*
* @brief   Get Forwarding ID value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanForwardingIdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_U32                        fid;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanForwardingIdGet(
        devNum, vlanId, &fid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", fid);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanUnknownMacSaCommandSet function
* @endinternal
*
* @brief   Set bridge forwarding decision for packets with unknown Source Address.
*         Only relevant in controlled address learning mode, when <Auto-learning
*         enable> = Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanUnknownMacSaCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_U16                        vlanId;
    CPSS_PACKET_CMD_ENT           cmd;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    cmd    = (CPSS_PACKET_CMD_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanUnknownMacSaCommandSet(
        devNum, vlanId, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanUnknownMacSaCommandGet function
* @endinternal
*
* @brief   Get bridge forwarding decision for packets with unknown Source Address.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanUnknownMacSaCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_U16                        vlanId;
    CPSS_PACKET_CMD_ENT           cmd;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanUnknownMacSaCommandGet(
        devNum, vlanId, &cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", cmd);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet function
* @endinternal
*
* @brief   When enabled, IPv4 MC/BC packets in this eVLAN are mirrored to the
*         analyzer specified in this field. This is independent of the analyzer
*         configuration of other traffic in this eVLAN.
*         0x0 is a reserved value that means Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;
    GT_U32                        index;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    index  = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet(
        devNum, vlanId, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet function
* @endinternal
*
* @brief   Get Ipv4 MC/BC Mirror To Analyzer Index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;
    GT_U32                        index;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet(
        devNum, vlanId, &enable, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, index);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet function
* @endinternal
*
* @brief   When enabled, IPv6 MC packets in this eVLAN are mirrored to the
*         analyzer. This is independent of the analyzer configuration of other
*         traffic in this eVLAN.
*         0x0 is a reserved value that means Disabled.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;
    GT_U32                        index;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    index  = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet(
        devNum, vlanId, enable, index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet function
* @endinternal
*
* @brief   Get Ipv6 MC Mirror To Analyzer Index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;
    GT_U32                        index;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet(
        devNum, vlanId, &enable, &index);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", enable, index);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortAccFrameTypeSet function
* @endinternal
*
* @brief   Set port access frame type.
*         There are three types of port configuration:
*         - Admit only Vlan tagged frames
*         - Admit only tagged and priority tagged,
*         - Admit all frames.
*
* @note   APPLICABLE DEVICES:      98DX2x5 devices only
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
*/
static CMD_STATUS wrCpssDxChBrgVlanPortAccFrameTypeSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;

    GT_U8                               devNum;
    GT_PORT_NUM                               portNum;
    CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     frameType;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    frameType = (CPSS_PORT_ACCEPT_FRAME_TYPE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortAccFrameTypeSet(devNum, portNum, frameType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanPortAccFrameTypeGet function
* @endinternal
*
* @brief   Get port access frame type.
*         There are three types of port configuration:
*         - Admit only Vlan tagged frames
*         - Admit only tagged and priority tagged,
*         - Admit all frames.
*
* @note   APPLICABLE DEVICES:      DxCh2 and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortAccFrameTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_PORT_NUM                               portNum;
    CPSS_PORT_ACCEPT_FRAME_TYPE_ENT     frameType;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortAccFrameTypeGet(devNum, portNum, &frameType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", frameType);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanMruProfileIdxSet function
* @endinternal
*
* @brief   Set Maximum Receive Unit MRU profile index for a VLAN.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         Value of MRU for profile is set by gvlnSetMruProfileValue.
*
* @note   APPLICABLE DEVICES:      98DX2x5 devices only
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
*/
static CMD_STATUS wrCpssDxChBrgVlanMruProfileIdxSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_U32                        mruIndex;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    mruIndex = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanMruProfileIdxSet(devNum, vlanId, mruIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanMruProfileValueSet function
* @endinternal
*
* @brief   Set MRU value for a VLAN MRU profile.
*         MRU VLAN profile sets maximum packet size that can be received
*         for the given VLAN.
*         gvlnSetMruProfileIdx set index of profile for a specific VLAN
*
* @note   APPLICABLE DEVICES:      98DX2x5 devices only
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
*/
static CMD_STATUS wrCpssDxChBrgVlanMruProfileValueSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U32                        mruIndex;
    GT_U32                        mruValue;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mruIndex = (GT_U32)inArgs[1];
    mruValue = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanMruProfileValueSet(devNum, mruIndex, mruValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanMruProfileValueGet function
* @endinternal
*
* @brief   Get MRU value for a VLAN MRU profile.
*
* @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChBrgVlanMruProfileValueGet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U32                        mruIndex;
    GT_U32                        mruValue;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    mruIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanMruProfileValueGet(devNum, mruIndex, &mruValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mruValue);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanNaToCpuEnable function
* @endinternal
*
* @brief   Enable/Disable New Address (NA) Message Sending to CPU per VLAN .
*         To send NA to CPU both VLAN and port must be set to send NA to CPU.
*
* @note   APPLICABLE DEVICES:      98DX2x5 devices only
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_SUPPORTED         - This request is not supportted.
*/
static CMD_STATUS wrCpssDxChBrgVlanNaToCpuEnable

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanNaToCpuEnable(devNum, vlanId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanBridgingModeSet function
* @endinternal
*
* @brief   Set bridging mode either VLAN-aware (802.1Q) or VLAN-unaware (802.1D) mode
*         The device supports a VLAN-unaware mode for 802.1D bridging.
*         When this mode is enabled:
*         - In VLAN-unaware mode, the device does not perform any packet
*         modifications. Packets are always transmitted as-received, without any
*         modification (i.e., packets received tagged are transmitted tagged;
*         packets received untagged are transmitted untagged).
*         - Packets are implicitly assigned with VLAN-ID 1, regardless of
*         VLAN-assignment mechanisms.
*         - Packets are implicitly assigned a VIDX Multicast group index 0xFFF,
*         indicating that the packet flood domain is according to the VLAN-in
*         this case VLAN 1. Registered Multicast is not supported in this mode.
*         All other features are operational in this mode, including internal
*         packet QoS, trapping, filtering, policy, etc.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
*/
static CMD_STATUS wrCpssDxChBrgVlanBridgingModeSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    CPSS_BRG_MODE_ENT             brgMode;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    brgMode = (CPSS_BRG_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanBridgingModeSet(devNum, brgMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanBridgingModeGet function
* @endinternal
*
* @brief   Get bridging mode either VLAN-aware (802.1Q) or VLAN-unaware (802.1D) mode
*         The device supports a VLAN-unaware mode for 802.1D bridging.
*         When this mode is enabled:
*         - In VLAN-unaware mode, the device does not perform any packet
*         modifications. Packets are always transmitted as-received, without any
*         modification (i.e., packets received tagged are transmitted tagged;
*         packets received untagged are transmitted untagged).
*         - Packets are implicitly assigned with VLAN-ID 1, regardless of
*         VLAN-assignment mechanisms.
*         - Packets are implicitly assigned a VIDX Multicast group index 0xFFF,
*         indicating that the packet flood domain is according to the VLAN-in
*         this case VLAN 1. Registered Multicast is not supported in this mode.
*         All other features are operational in this mode, including internal
*         packet QoS, trapping, filtering, policy, etc.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static CMD_STATUS wrCpssDxChBrgVlanBridgingModeGet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;

    GT_U8                         devNum;
    CPSS_BRG_MODE_ENT             brgMode;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgVlanBridgingModeGet(devNum, &brgMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", brgMode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanVrfIdSet function
* @endinternal
*
* @brief   Sets vlan Virtual Router ID
*
* @note   APPLICABLE DEVICES:      DXCH3 devices only
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_OUT_OF_RANGE          - when vrfId is out of range
*
* @note none
*
*/
static CMD_STATUS wrCpssDxChBrgVlanVrfIdSet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U16 vlanId;
    GT_U32 vrfId;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    vrfId = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanVrfIdSet(devNum, vlanId, vrfId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortTranslationEnableSet function
* @endinternal
*
* @brief   Enable/Disable Vlan Translation per ingress or egress port.
*
* @note   APPLICABLE DEVICES:      DXCH3 devices only
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
*/
static CMD_STATUS wrCpssDxChBrgVlanPortTranslationEnableSet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    CPSS_DIRECTION_ENT direction;
    GT_BOOL enable;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    direction = (CPSS_DIRECTION_ENT)inArgs[2];
    enable = (CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortTranslationEnableSet(devNum, port, direction, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortTranslationEnableGet function
* @endinternal
*
* @brief   Get the status of Vlan Translation (Enable/Disable) per ingress or
*         egress port.
*
* @note   APPLICABLE DEVICES:      DXCH3 devices only
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanPortTranslationEnableGet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_NUM port;
    CPSS_DIRECTION_ENT direction;
    CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT enable;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    port = (GT_PORT_NUM)inArgs[1];
    direction = (CPSS_DIRECTION_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortTranslationEnableGet(devNum, port, direction, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanTranslationEntryWrite function
* @endinternal
*
* @brief   Write an entry into Vlan Translation Table Ingress or Egress
*         This mapping avoids collisions between a VLAN-ID
*         used for a logical interface and a VLAN-ID used by the local network.
*
* @note   APPLICABLE DEVICES:      DXCH3 devices only
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_OUT_OF_RANGE          - when transVlanId is out of range
*/
static CMD_STATUS wrCpssDxChBrgVlanTranslationEntryWrite
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U16 vlanId;
    CPSS_DIRECTION_ENT direction;
    GT_U16 transVlanId;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    direction = (CPSS_DIRECTION_ENT)inArgs[2];
    transVlanId = (GT_U16)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanTranslationEntryWrite(devNum, vlanId, direction, transVlanId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanTranslationEntryRead function
* @endinternal
*
* @brief   Read an entry from Vlan Translation Table, Ingress or Egress.
*
* @note   APPLICABLE DEVICES:      DXCH3 devices only
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or vlanId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanTranslationEntryRead
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U16 vlanId;
    CPSS_DIRECTION_ENT direction;
    GT_U16 transVlanId;


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    direction = (CPSS_DIRECTION_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanTranslationEntryRead(devNum, vlanId, direction, &transVlanId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", transVlanId);

    return CMD_OK;
}
/**
* @internal wrCpssDxChBrgVlanValidCheckEnableSet function
* @endinternal
*
* @brief   Enable/disable check of "Valid" field in the VLAN entry
*         When check enable and VLAN entry is not valid then packets are dropped.
*         When check disable and VLAN entry is not valid then packets are
*         not dropped and processed like in case of valid VLAN.
*
* @note   APPLICABLE DEVICES:      All DxCh2 and above
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanValidCheckEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanValidCheckEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanValidCheckEnableGet function
* @endinternal
*
* @brief   Get check status of "Valid" field in the VLAN entry
*         When check enable and VLAN entry is not valid then packets are dropped.
*         When check disable and VLAN entry is not valid then packets are
*         not dropped and processed like in case of valid VLAN.
*
* @note   APPLICABLE DEVICES:      All DxCh2 and above
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - failed to read from hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanValidCheckEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgVlanValidCheckEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanTpidEntrySet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) table entry.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, entryIndex, direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanTpidEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    CPSS_DIRECTION_ENT  direction;
    CPSS_DXCH_ETHERTYPE_TABLE_ENT   tableType;
    GT_U32              entryIndex;
    GT_U16              etherType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_DIRECTION_ENT)inArgs[1];
    entryIndex = (GT_U32)inArgs[2];
    etherType = (GT_U16)inArgs[3];

    switch (direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            tableType =
                CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E;
            break;

        case CPSS_DIRECTION_EGRESS_E:
            tableType =
                CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call cpss api function */
    result = cpssDxChBrgVlanTpidEntrySet(devNum, tableType, entryIndex, etherType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanTpidEntryGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) table entry.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, entryIndex, direction
* @retval GT_BAD_PTR               - etherTypePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanTpidEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    CPSS_DIRECTION_ENT  direction;
    CPSS_DXCH_ETHERTYPE_TABLE_ENT   tableType;
    GT_U32              entryIndex;
    GT_U16              etherTypePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    direction = (CPSS_DIRECTION_ENT)inArgs[1];
    entryIndex = (GT_U32)inArgs[2];

    switch (direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            tableType =
                CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E;
            break;

        case CPSS_DIRECTION_EGRESS_E:
            tableType =
                CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* call cpss api function */
    result = cpssDxChBrgVlanTpidEntryGet(devNum, tableType, entryIndex, &etherTypePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", etherTypePtr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortIngressTpidSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more then 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that for the ingress direction multiple TPID may be use to
*       identify Tag0 and Tag1. Each bit at the bitmap represent one of
*       the 8 entries at the TPID Table.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanPortIngressTpidSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    GT_PORT_NUM               portNum;
    CPSS_ETHER_MODE_ENT ethMode;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT ethTblMode;
    GT_U32              tpidBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_ETHER_MODE_ENT)inArgs[2];
    tpidBmp = (GT_U32)inArgs[3];

    switch (ethMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            ethTblMode =
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E;
            break;

        case CPSS_VLAN_ETHERTYPE1_E:
            ethTblMode =
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        result = GT_BAD_PARAM;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChBrgVlanPortIngressTpidSet(devNum, portNum, ethTblMode, tpidBmp);
    }
    else
    {
        result = wrSip5Emulate_cpssDxChBrgVlanPortIngressTpidSet(devNum, portNum, ethMode, tpidBmp);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortIngressTpidGet function
* @endinternal
*
* @brief   Function gets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidBmpPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that for the ingress direction multiple TPID may be use to
*       identify Tag0 and Tag1. Each bit at the bitmap represent one of
*       the 8 entries at the TPID Table.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanPortIngressTpidGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    GT_PORT_NUM         portNum;
    CPSS_ETHER_MODE_ENT ethMode;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT ethTblMode;
    GT_U32              tpidBmpPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_ETHER_MODE_ENT)inArgs[2];

    switch (ethMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            ethTblMode =
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E;
            break;

        case CPSS_VLAN_ETHERTYPE1_E:
            ethTblMode =
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        result = GT_BAD_PARAM;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChBrgVlanPortIngressTpidGet(devNum, portNum, ethTblMode, &tpidBmpPtr);
    }
    else
    {
        result = wrSip5Emulate_cpssDxChBrgVlanPortIngressTpidGet(devNum, portNum, ethMode, &tpidBmpPtr);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tpidBmpPtr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortEgressTpidSet function
* @endinternal
*
* @brief   Function sets index of TPID (Tag protocol ID) table per egress port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex (not 0-7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortEgressTpidSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    GT_PORT_NUM               portNum;
    CPSS_ETHER_MODE_ENT ethMode;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT ethTblMode;
    GT_U32              tpidEntryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_ETHER_MODE_ENT)inArgs[2];
    tpidEntryIndex = (GT_U32)inArgs[3];

    switch (ethMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            ethTblMode =
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E;
            break;

        case CPSS_VLAN_ETHERTYPE1_E:
            ethTblMode =
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortEgressTpidSet(devNum, portNum, ethTblMode, tpidEntryIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanPortEgressTpidGet function
* @endinternal
*
* @brief   Function gets index of TPID (Tag Protocol ID) table per egress port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidEntryIndexPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortEgressTpidGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    GT_PORT_NUM               portNum;
    CPSS_ETHER_MODE_ENT ethMode;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT ethTblMode;
    GT_U32              tpidEntryIndexPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_ETHER_MODE_ENT)inArgs[2];

    switch (ethMode)
    {
        case CPSS_VLAN_ETHERTYPE0_E:
            ethTblMode =
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E;
            break;

        case CPSS_VLAN_ETHERTYPE1_E:
            ethTblMode =
                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortEgressTpidGet(devNum, portNum, ethTblMode, &tpidEntryIndexPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tpidEntryIndexPtr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanRangeSet function
* @endinternal
*
* @brief   Function configures the valid VLAN Range.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - illegal vidRange
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanRangeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U16    vidRange;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    vidRange = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanRangeSet(devNum, vidRange);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanRangeGet function
* @endinternal
*
* @brief   Function gets the valid VLAN Range.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - vidRangePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanRangeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U16    vidRangePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgVlanRangeGet(devNum, &vidRangePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", vidRangePtr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortIsolationCmdSet function
* @endinternal
*
* @brief   Function sets port isolation command per egress VLAN.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or cmd
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortIsolationCmdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U16     vlanId;
    CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT  cmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    cmd = (CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanPortIsolationCmdSet(devNum, vlanId, cmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanLocalSwitchingEnableSet function
* @endinternal
*
* @brief   Function sets local switching of Multicast, known and unknown Unicast,
*         and Broadcast traffic per VLAN.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or trafficType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To enable local switching of Multicast, unknown Unicast, and
*       Broadcast traffic, both this field in the VLAN entry and the
*       egress port configuration must be enabled for Multicast local switching
*       in function cpssDxChBrgPortEgressMcastLocalEnable.
*       2. To enable local switching of known Unicast traffic, both this
*       field in the VLAN entry and the ingress port configuration must
*       be enabled for Unicast local switching in function
*       cpssDxChBrgGenUcLocalSwitchingEnable.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanLocalSwitchingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  result;
    GT_U8   devNum;
    GT_U16  vlanId;
    CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT  trafficType;
    GT_BOOL enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    trafficType = (CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanLocalSwitchingEnableSet(devNum,vlanId, trafficType, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanFloodVidxModeSet function
* @endinternal
*
* @brief   Function sets Flood VIDX and Flood VIDX Mode per VLAN.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or floodVidxMode
* @retval GT_OUT_OF_RANGE          - illegal floodVidx
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanFloodVidxModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   devNum;
    GT_U16                                  vlanId;
    GT_U16                                  floodVidx;
    CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT  floodVidxMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    floodVidx = (GT_U16)inArgs[2];
    floodVidxMode = (CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanFloodVidxModeSet(devNum, vlanId, floodVidx, floodVidxMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}



/*cpssDxChBrgVlanTagCommand Table*/
static GT_U32   tagCmdCnt;

/**
* @internal wrCpssDxChBrgVlanTagCommandSet function
* @endinternal
*
* @brief   Biulds and writes tagging commands.
*
* @note   APPLICABLE DEVICES:      All DxChXcat and above devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*/
static CMD_STATUS wrCpssDxChBrgVlanTagCommandSet
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;
    GT_U8                          devNum;
    GT_U16                         vlanId;
    GT_PHYSICAL_PORT_NUM           portNum;
    CPSS_PORTS_BMP_STC             portsMembersPtr;
    CPSS_PORTS_BMP_STC             portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_INFO_STC    vlanInfoPtr;
    GT_BOOL                        isValidPtr;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmdPtr;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT taggingCommand;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    portNum = (GT_PHYSICAL_PORT_NUM)inFields[0];
    taggingCommand = (CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT)inFields[1];

    if(portNum >= CPSS_MAX_PORTS_NUM_CNS)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChBrgVlanEntryRead(devNum,
                                      vlanId,
                                      &portsMembersPtr,
                                      &portsTaggingPtr,
                                      &vlanInfoPtr,
                                      &isValidPtr,
                                      &portsTaggingCmdPtr);

    if ((result != GT_OK) || (isValidPtr != GT_TRUE))
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    portsTaggingCmdPtr.portsCmd[portNum] = taggingCommand;

    result = cpssDxChBrgVlanEntryWrite(devNum,
                                       vlanId,
                                       &portsMembersPtr,
                                       &portsTaggingPtr,
                                       &vlanInfoPtr,
                                       &portsTaggingCmdPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanTagCommandGetFirst function
* @endinternal
*
* @brief   Read tagging command per VLAN entry.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static CMD_STATUS wrCpssDxChBrgVlanTagCommandGetFirst
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;
    GT_U8                          devNum;
    GT_U16                         vlanId;
    GT_BOOL                        displayMembersOnly;
    CPSS_PORTS_BMP_STC             portsMembersPtr;
    CPSS_PORTS_BMP_STC             portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_INFO_STC    vlanInfoPtr;
    GT_BOOL                        isValidPtr;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmdPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    tagCmdCnt = 0;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    displayMembersOnly = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanEntryRead(devNum,
                                      vlanId,
                                      &portsMembersPtr,
                                      &portsTaggingPtr,
                                      &vlanInfoPtr,
                                      &isValidPtr,
                                      &portsTaggingCmdPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if (isValidPtr == GT_TRUE)
    {
        inFields[0] = tagCmdCnt;
        inFields[1] = portsTaggingCmdPtr.portsCmd[tagCmdCnt];

        /* pack and output table fields */
        if ((displayMembersOnly == GT_TRUE) && (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsMembersPtr,tagCmdCnt) == GT_FALSE))
        {
            /* do nothing*/
        }
        else
        {
            fieldOutput("%d%d", inFields[0], inFields[1]);
        }

        galtisOutput(outArgs, GT_OK, "%f");
    }
    else
    {
        galtisOutput(outArgs, result, "");
    }

    return CMD_OK;
}


/*******************************************************************************/
static CMD_STATUS wrCpssDxChBrgVlanTagCommandGetNext
(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                      result;
    GT_U8                          devNum;
    GT_U16                         vlanId;
    GT_BOOL                        displayMembersOnly;
    CPSS_PORTS_BMP_STC             portsMembersPtr;
    CPSS_PORTS_BMP_STC             portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_INFO_STC    vlanInfoPtr;
    GT_BOOL                        isValidPtr;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmdPtr;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    displayMembersOnly = (GT_BOOL)inArgs[2];

    tagCmdCnt++;

    if (tagCmdCnt >= CPSS_MAX_PORTS_NUM_CNS)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChBrgVlanEntryRead(devNum,
                                      vlanId,
                                      &portsMembersPtr,
                                      &portsTaggingPtr,
                                      &vlanInfoPtr,
                                      &isValidPtr,
                                      &portsTaggingCmdPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    if (isValidPtr == GT_TRUE)
    {
        inFields[0] = tagCmdCnt;
        inFields[1] = portsTaggingCmdPtr.portsCmd[tagCmdCnt];

        /* pack and output table fields */
        if ((displayMembersOnly == GT_TRUE) && (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsMembersPtr,tagCmdCnt) == GT_FALSE))
        {
            /* do nothing*/
        }
        else
        {
            fieldOutput("%d%d", inFields[0], inFields[1]);
        }

        galtisOutput(outArgs, GT_OK, "%f");
    }
    else
    {
        galtisOutput(outArgs, result, "");
    }

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet function
* @endinternal
*
* @brief   Set Tag1 removal mode from the egress port Tag State if Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;
    GT_U8                                           dev;
    CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev    = (GT_U8)inArgs[0];
    mode   = (CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(dev, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet function
* @endinternal
*
* @brief   Get Tag1 removal mode from the egress port Tag State when Tag1 VID
*         is assigned a value of 0.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                       result;
    GT_U8                                           dev;
    CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    dev    = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet(dev, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanForceNewDsaToCpuEnableSet function
* @endinternal
*
* @brief   Enable / Disable preserving the original VLAN tag
*         and VLAN EtherType (or tags),
*         and including the new VLAN Id assignment in the TO_CPU DSA tag
*         for tagged packets that are sent to the CPU.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanForceNewDsaToCpuEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       dev;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev    = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanForceNewDsaToCpuEnableSet(dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanForceNewDsaToCpuEnableGet function
* @endinternal
*
* @brief   Get Force New DSA To Cpu mode for enabling / disabling
*         preserving the original VLAN tag and VLAN EtherType (or tags),
*         and including the new VLAN Id assignment in the TO_CPU DSA tag
*         for tagged packets that are sent to the CPU.
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanForceNewDsaToCpuEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       dev;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev    = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanForceNewDsaToCpuEnableGet(dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanKeepVlan1EnableSet function
* @endinternal
*
* @brief   Enable/Disable keeping VLAN1 in the packet, for packets received with VLAN1,
*         even-though the tag-state of this egress-port is configured in the
*         VLAN-table to "untagged" or "VLAN0".
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or up
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanKeepVlan1EnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U8       up;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev    = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    up = (GT_U8)inArgs[2];
    enable = (GT_BOOL)inArgs[3];

    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanKeepVlan1EnableSet(dev, portNum, up, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanKeepVlan1EnableGet function
* @endinternal
*
* @brief   Get status of keeping VLAN1 in the packet, for packets received with VLAN1.
*         (Enabled/Disabled).
*
* @note   APPLICABLE DEVICES:      Lion and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, port or up
* @retval GT_BAD_PTR               - modePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanKeepVlan1EnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM       portNum;
    GT_U8       up;
    GT_BOOL     enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev    = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    up = (GT_U8)inArgs[2];

    CONVERT_DEV_PHYSICAL_PORT_MAC(dev, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanKeepVlan1EnableGet(dev, portNum, up, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) per profile .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more then 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. For the ingress direction multiple TPID may be used to
*       identify Tag0 and Tag1. Each bit at the bitmap represents one of
*       the 8 entries at the TPID Table.
*       2. bind 'port' to 'profile' using function cpssDxChBrgVlanPortIngressTpidProfileSet
*
*/
static CMD_STATUS wrCpssDxChBrgVlanIngressTpidProfileSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_U32               profile;
    CPSS_ETHER_MODE_ENT  ethMode;
    GT_U32               tpidBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    profile = (GT_U32)inArgs[1];
    ethMode = (CPSS_ETHER_MODE_ENT)inArgs[2];
    tpidBmp = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanIngressTpidProfileSet(devNum, profile, ethMode, tpidBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gts bitmap of TPID (Tag Protocol ID) per profile .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIngressTpidProfileGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS            result;
    GT_U8                devNum;
    GT_U32               profile;
    CPSS_ETHER_MODE_ENT  ethMode;
    GT_U32               tpidBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    profile = (GT_U32)inArgs[1];
    ethMode = (CPSS_ETHER_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanIngressTpidProfileGet(devNum, profile, ethMode, &tpidBmp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tpidBmp);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortIngressTpidProfileSet function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - profile > 7
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For setting TPID bmp per profile use function cpssDxChBrgVlanIngressTpidProfileSet
*
*/
static CMD_STATUS wrCpssDxChBrgVlanPortIngressTpidProfileSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PORT_NUM             portNum;
    CPSS_ETHER_MODE_ENT     ethMode;
    GT_BOOL                 isDefaultProfile;
    GT_U32                  profile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_ETHER_MODE_ENT)inArgs[2];
    isDefaultProfile = (GT_BOOL)inArgs[3];
    profile = (GT_U32)inArgs[4];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, portNum, ethMode,
                                                      isDefaultProfile, profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortIngressTpidProfileGet function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) profile per ingress port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortIngressTpidProfileGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PORT_NUM             portNum;
    CPSS_ETHER_MODE_ENT     ethMode;
    GT_BOOL                 isDefaultProfile;
    GT_U32                  profile;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_ETHER_MODE_ENT)inArgs[2];
    isDefaultProfile = (GT_BOOL)inArgs[3];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortIngressTpidProfileGet(devNum, portNum, ethMode,
                                                      isDefaultProfile, &profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profile);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortVid1Set function
* @endinternal
*
* @brief   For ingress direction : Set port's default Vid1.
*         For egress direction : Set port's Vid1 when egress port <VID1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVid1Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PORT_NUM             portNum;
    CPSS_DIRECTION_ENT      direction;
    GT_U16                  vid1;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    direction = (CPSS_DIRECTION_ENT)inArgs[2];
    vid1 = (GT_U16)inArgs[3];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortVid1Set(devNum, portNum, direction, vid1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortVid1Get function
* @endinternal
*
* @brief   For ingress direction : Get port's default Vid1.
*         For egress direction : Get port's Vid1 when egress port <VID1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVid1Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PORT_NUM             portNum;
    CPSS_DIRECTION_ENT      direction;
    GT_U16                  vid1;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    direction = (CPSS_DIRECTION_ENT)inArgs[2];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortVid1Get(devNum, portNum, direction, &vid1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", vid1);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanEgressTagTpidSelectModeSet function
* @endinternal
*
* @brief   Set Tag 0/1 TPID (Tag Protocol Identifies) Select Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanEgressTagTpidSelectModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PORT_NUM             portNum;
    CPSS_ETHER_MODE_ENT     ethMode;
    CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT  mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_ETHER_MODE_ENT)inArgs[2];
    mode = (CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT)inArgs[3];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanEgressTagTpidSelectModeSet(devNum, portNum, ethMode, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanEgressTagTpidSelectModeGet function
* @endinternal
*
* @brief   Returns Tag 0/1 TPID (Tag Protocol Identifies) Select Mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanEgressTagTpidSelectModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PORT_NUM             portNum;
    CPSS_ETHER_MODE_ENT     ethMode;
    CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT  mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_ETHER_MODE_ENT)inArgs[2];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanEgressTagTpidSelectModeGet(devNum, portNum, ethMode, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",mode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanEgressPortTagStateModeSet function
* @endinternal
*
* @brief   Set the egress port tag state mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanEgressPortTagStateModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PORT_NUM             portNum;
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT      stateMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    stateMode = (CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT)inArgs[2];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanEgressPortTagStateModeSet(devNum, portNum, stateMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanEgressPortTagStateModeGet function
* @endinternal
*
* @brief   Get the egress port tag state mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanEgressPortTagStateModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PORT_NUM             portNum;
    CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT      stateMode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanEgressPortTagStateModeGet(devNum, portNum, &stateMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", stateMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanEgressPortTagStateSet function
* @endinternal
*
* @brief   Determines the egress port state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanEgressPortTagStateSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PORT_NUM             portNum;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT      tagCmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    tagCmd = (CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT)inArgs[2];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanEgressPortTagStateSet(devNum, portNum, tagCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanEgressPortTagStateGet function
* @endinternal
*
* @brief   Returns the egress port state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanEgressPortTagStateGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PORT_NUM             portNum;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT      tagCmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];

    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanEgressPortTagStateGet(devNum, portNum, &tagCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",tagCmd);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanFcoeForwardingEnableSet function
* @endinternal
*
* @brief   Set FCoE Forwarding Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanFcoeForwardingEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U16                  vlanId;
    GT_BOOL                 enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    vlanId  = (GT_U16)inArgs[1];
    enable  = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanFcoeForwardingEnableSet(
        devNum, vlanId, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanFcoeForwardingEnableGet function
* @endinternal
*
* @brief   Return FCoE Forwarding Enable status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanFcoeForwardingEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                     result;
    GT_U8                         devNum;
    GT_U16                        vlanId;
    GT_BOOL                       enable;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanFcoeForwardingEnableGet(devNum, vlanId, &enable);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }
    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanUnregisteredIpmEVidxSet function
* @endinternal
*
* @brief   Set Unregistered IPM eVidx Assignment Mode and Value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - on out-of-range parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanUnregisteredIpmEVidxSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                    result;
    GT_U8                                        devNum;
    GT_U16                                       vlanId;
    CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT  unregIpmEVidxMode;
    GT_U32                                       unregIpmEVidx;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum             = (GT_U8)inArgs[0];
    vlanId             = (GT_U16)inArgs[1];
    unregIpmEVidxMode  = (CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT)inArgs[2];
    unregIpmEVidx      = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanUnregisteredIpmEVidxSet(
        devNum, vlanId, unregIpmEVidxMode, unregIpmEVidx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet function
* @endinternal
*
* @brief   Enable Learning Of Original Tag1 VID Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;
    GT_U8         dev;
    GT_PORT_NUM   port;
    GT_BOOL       enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PORT_NUM)inArgs[1];
    enable    = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet function
* @endinternal
*
* @brief   Get Learning mode Of Original Tag1 VID Per Port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;
    GT_U8         dev;
    GT_PORT_NUM   port;
    GT_BOOL       enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortEgressVlanFilteringEnableSet function
* @endinternal
*
* @brief   Determines if egress VLAN filtering is performed for this target port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortEgressVlanFilteringEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;
    GT_U8         dev;
    GT_PORT_NUM   port;
    GT_BOOL       enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PORT_NUM)inArgs[1];
    enable    = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    /* NOTE: calling CPSS API of 'BrgEgrFlt' */
    result = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortEgressVlanFilteringEnableGet function
* @endinternal
*
* @brief   Return if egress VLAN filtering is performed for this target Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or port.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortEgressVlanFilteringEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS     result;
    GT_U8         dev;
    GT_PORT_NUM   port;
    GT_BOOL       enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    /* NOTE: calling CPSS API of 'BrgEgrFlt' */
    result = cpssDxChBrgEgrFltVlanPortFilteringEnableGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortPushVlanCommandSet function
* @endinternal
*
* @brief   Set Push Vlan Command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortPushVlanCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       dev;
    GT_PORT_NUM                                 port;
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT    vlanCmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PORT_NUM)inArgs[1];
    vlanCmd   = (CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortPushVlanCommandSet(
        dev, port, vlanCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortPushVlanCommandGet function
* @endinternal
*
* @brief   Get Push Vlan Command
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortPushVlanCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                   result;
    GT_U8                                       dev;
    GT_PORT_NUM                                 port;
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT    vlanCmd;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev       = (GT_U8)inArgs[0];
    port      = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortPushVlanCommandGet(
        dev, port, &vlanCmd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", vlanCmd);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortPushedTagTpidSelectEgressSet function
* @endinternal
*
* @brief   Set TPID select table index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortPushedTagTpidSelectEgressSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U32           tpidEntryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    tpidEntryIndex   = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(
        dev, port, tpidEntryIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortPushedTagTpidSelectEgressGet function
* @endinternal
*
* @brief   Get TPID select table index
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortPushedTagTpidSelectEgressGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U32           tpidEntryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(
        dev, port, &tpidEntryIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tpidEntryIndex);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortPushedTagValueSet function
* @endinternal
*
* @brief   Set the tag value to push
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortPushedTagValueSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U16           tagValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    tagValue         = (GT_U16)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortPushedTagValueSet(
        dev, port, tagValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortPushedTagValueGet function
* @endinternal
*
* @brief   Get the tag value to push
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortPushedTagValueGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U16           tagValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortPushedTagValueGet(
        dev, port, &tagValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tagValue);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet function
* @endinternal
*
* @brief   Determines if UP and CFI values will be assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    enable           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet function
* @endinternal
*
* @brief   Get if UP and CFI values will be assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUpSet function
* @endinternal
*
* @brief   Set UP value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUpSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U32           up;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    up               = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUpSet(
        dev, port, up);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUpGet function
* @endinternal
*
* @brief   Get UP value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUpGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U32           up;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUpGet(
        dev, port, &up);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", up);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortCfiEnableSet function
* @endinternal
*
* @brief   Set CFI value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortCfiEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    enable           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortCfiEnableSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortCfiEnableGet function
* @endinternal
*
* @brief   Get CFI value to assign
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortCfiEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortCfiEnableGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrcpssDxChBrgVlanPortNumOfTagWordsToPopSet function
* @endinternal
*
* @brief   Set the number of tag words to pop for traffic that ingress from the port.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*         INPUTS:
*         devNum      - device number
*         portNum      - port number
*         numberOfTagWords - number of tag 4-byte words to pop.
*         value 0 means - no popping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when parameter numberOfTagWords is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssDxChBrgVlanPortNumOfTagWordsToPopSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U32           numberOfBytesInTag;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    numberOfBytesInTag = (GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortNumOfTagWordsToPopSet(
        dev, port, numberOfBytesInTag);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrcpssDxChBrgVlanPortNumOfTagWordsToPopGet function
* @endinternal
*
* @brief   Get the number of tag words to pop for traffic that ingress from the port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrcpssDxChBrgVlanPortNumOfTagWordsToPopGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT numberOfBytesInTag;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortNumOfTagWordsToPopGet(
        dev, port, &numberOfBytesInTag);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", (GT_U32)numberOfBytesInTag);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortVid0CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing VID0 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVid0CommandEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    enable           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortVid0CommandEnableSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortVid0CommandEnableGet function
* @endinternal
*
* @brief   Return if changing VID0 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVid0CommandEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortVid0CommandEnableGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortVid1CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing VID1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVid1CommandEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    enable           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortVid1CommandEnableSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortVid1CommandEnableGet function
* @endinternal
*
* @brief   Return if changing VID1 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortVid1CommandEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortVid1CommandEnableGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUp0CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing UP1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUp0CommandEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    enable           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUp0CommandEnableSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUp0CommandEnableGet function
* @endinternal
*
* @brief   Return if changing UP0 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUp0CommandEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUp0CommandEnableGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUp1CommandEnableSet function
* @endinternal
*
* @brief   Enable/Disable changing UP1 of the egress packet
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUp1CommandEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    enable           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUp1CommandEnableSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUp1CommandEnableGet function
* @endinternal
*
* @brief   Return if changing UP1 of the egress packet is enabled/disabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUp1CommandEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUp1CommandEnableGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUp0Set function
* @endinternal
*
* @brief   Set Up0 assigned to the egress packet if <UP0 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUp0Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U32           up0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    up0              = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUp0Set(
        dev, port, up0);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUp0Get function
* @endinternal
*
* @brief   Return the UP0 assigned to the egress packet if <UP0 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUp0Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U32           up0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUp0Get(
        dev, port, &up0);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", up0);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUp1Set function
* @endinternal
*
* @brief   Set Up1 assigned to the egress packet if <UP1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUp1Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U32           up1;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    up1              = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUp1Set(
        dev, port, up1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortUp1Get function
* @endinternal
*
* @brief   Return the UP1 assigned to the egress packet if <UP1 Command>=Enabled
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortUp1Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_U32           up1;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortUp1Get(
        dev, port, &up1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", up1);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIndependentNonFloodVidxEnableSet function
* @endinternal
*
* @brief   Enable/Disable Independent non-flood VIDX
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIndependentNonFloodVidxEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    enable           = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanIndependentNonFloodVidxEnableSet(
        dev, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanIndependentNonFloodVidxEnableGet function
* @endinternal
*
* @brief   Get Independent non-flood VIDX status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanIndependentNonFloodVidxEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    enable           = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanIndependentNonFloodVidxEnableGet(
        dev, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet function
* @endinternal
*
* @brief   Enable/Disable VLAN Tag1 removing from packets arrived without VLAN Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];
    enable           = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(
        dev, port, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of VLAN Tag1 removing from packets arrived without VLAN Tag1.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS        result;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev              = (GT_U8)inArgs[0];
    port             = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(dev, port);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet(
        dev, port, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanTpidTagTypeSet function
* @endinternal
*
* @brief   Set TPID tag type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanTpidTagTypeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_U8                        dev;
    CPSS_DIRECTION_ENT           direction;
    GT_U32                       index;
    CPSS_BRG_TPID_SIZE_TYPE_ENT  type;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];
    direction           = (CPSS_DIRECTION_ENT)inArgs[1];
    index               = (GT_U32)inArgs[2];
    type                = (CPSS_BRG_TPID_SIZE_TYPE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanTpidTagTypeSet(dev, direction, index, type);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanTpidTagTypeGet function
* @endinternal
*
* @brief   Get TPID tag type
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, profile, ethMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanTpidTagTypeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                    result;
    GT_U8                        dev;
    CPSS_DIRECTION_ENT           direction;
    GT_U32                       index;
    CPSS_BRG_TPID_SIZE_TYPE_ENT  type;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];
    direction           = (CPSS_DIRECTION_ENT)inArgs[1];
    index               = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanTpidTagTypeGet(dev, direction, index, &type);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", type);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanMembersTableIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanMembersTableIndexingModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   dev;
    CPSS_DIRECTION_ENT                      direction;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];
    direction           = (CPSS_DIRECTION_ENT)inArgs[1];
    mode                = (CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanMembersTableIndexingModeSet(dev, direction, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanMembersTableIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN Members table.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanMembersTableIndexingModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   dev;
    CPSS_DIRECTION_ENT                      direction;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];
    direction           = (CPSS_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanMembersTableIndexingModeGet(dev, direction, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanStgIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanStgIndexingModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   dev;
    CPSS_DIRECTION_ENT                      direction;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];
    direction           = (CPSS_DIRECTION_ENT)inArgs[1];
    mode                = (CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanStgIndexingModeSet(dev, direction, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanStgIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch span state
*         group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanStgIndexingModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   dev;
    CPSS_DIRECTION_ENT                      direction;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];
    direction           = (CPSS_DIRECTION_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanStgIndexingModeGet(dev, direction, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanTagStateIndexingModeSet function
* @endinternal
*
* @brief   Set the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanTagStateIndexingModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   dev;
    GT_PHYSICAL_PORT_NUM                    portNum;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];
    portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mode                = (CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanTagStateIndexingModeSet(dev, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanTagStateIndexingModeGet function
* @endinternal
*
* @brief   Get the indexing mode for accessing the VLAN table to fetch tag state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanTagStateIndexingModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                               result;
    GT_U8                                   dev;
    GT_PHYSICAL_PORT_NUM                    portNum;
    CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT   mode;


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];
    portNum             = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanTagStateIndexingModeGet(dev, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanBpeTagSourceCidDefaultForceSet function
* @endinternal
*
* @brief   Set the BPE E-Tag Ingress_E-CID_base calculation algorithm
*         for Looped Back Forwarded Bridged Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For regular multicast packets E-Tag Ingress_E-CID_base is always S_VID.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanBpeTagSourceCidDefaultForceSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           dev;
    GT_PORT_NUM     portNum;
    GT_BOOL         forceSrcCidDefault;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];
    portNum             = (GT_PORT_NUM)inArgs[1];
    forceSrcCidDefault  = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet(
        dev, portNum, forceSrcCidDefault);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanBpeTagSourceCidDefaultForceGet function
* @endinternal
*
* @brief   Get the BPE E-Tag Ingress_E-CID_base calculation algorithm
*         for Looped Back Forwarded Bridged Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For regular multicast packets E-Tag Ingress_E-CID_base is always S_VID.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanBpeTagSourceCidDefaultForceGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       result;
    GT_U8           dev;
    GT_PORT_NUM     portNum;
    GT_BOOL         forceSrcCidDefault;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    dev                 = (GT_U8)inArgs[0];
    portNum             = (GT_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet(
        dev, portNum, &forceSrcCidDefault);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", forceSrcCidDefault);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanBpeTagMcCfgSet function
* @endinternal
*
* @brief   Set the BPE E-Tag Configuration for Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - when one of parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanBpeTagMcCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS         result;
    GT_U8             devNum;
    GT_BOOL           eCidOffsetNegative;
    GT_U32            eCidOffset;
    GT_U32            defaultSrcECid;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    eCidOffsetNegative  = (GT_BOOL)inArgs[1];
    eCidOffset          = (GT_U32)inArgs[2];
    defaultSrcECid      = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanBpeTagMcCfgSet(
        devNum, eCidOffsetNegative, eCidOffset, defaultSrcECid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanBpeTagMcCfgGet function
* @endinternal
*
* @brief   Get the BPE E-Tag Configuration for Multicast packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanBpeTagMcCfgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS         result;
    GT_U8             devNum;
    GT_BOOL           eCidOffsetNegative;
    GT_U32            eCidOffset;
    GT_U32            defaultSrcECid;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgVlanBpeTagMcCfgGet(
        devNum, &eCidOffsetNegative, &eCidOffset, &defaultSrcECid);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d",
        eCidOffsetNegative, eCidOffset, defaultSrcECid);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanBpeTagReservedFieldsSet function
* @endinternal
*
* @brief   Set the values for BPE E-Tag Reserved Fields of all packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_RANGE          - when one of parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanBpeTagReservedFieldsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS         result;
    GT_U8             devNum;
    GT_U32            reValue;
    GT_U32            ingressECidExtValue;
    GT_U32            eCidExtValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];
    reValue             = (GT_U32)inArgs[1];
    ingressECidExtValue = (GT_U32)inArgs[2];
    eCidExtValue        = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanBpeTagReservedFieldsSet(
        devNum, reValue, ingressECidExtValue, eCidExtValue);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanBpeTagReservedFieldsGet function
* @endinternal
*
* @brief   Get the values for BPE E-Tag Reserved Fields of all packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanBpeTagReservedFieldsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS         result;
    GT_U8             devNum;
    GT_U32            reValue;
    GT_U32            ingressECidExtValue;
    GT_U32            eCidExtValue;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum              = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgVlanBpeTagReservedFieldsGet(
        devNum, &reValue, &ingressECidExtValue, &eCidExtValue);

    /* pack output arguments to galtis string */
    galtisOutput(
        outArgs, result, "%d%d%d",
        reValue, ingressECidExtValue, eCidExtValue);
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanTpidEntrySet_1 function
* @endinternal
*
* @brief   Function sets TPID (Tag Protocol ID) table entry.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, entryIndex, direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanTpidEntrySet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    CPSS_DXCH_ETHERTYPE_TABLE_ENT  tableType;
    GT_U32              entryIndex;
    GT_U16              etherType;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    tableType = (CPSS_DXCH_ETHERTYPE_TABLE_ENT)inArgs[1];
    entryIndex = (GT_U32)inArgs[2];
    etherType = (GT_U16)inArgs[3];

    /* call cpss api function */
    result = cpssDxChBrgVlanTpidEntrySet(devNum, tableType, entryIndex, etherType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanTpidEntryGet_1 function
* @endinternal
*
* @brief   Function gets TPID (Tag Protocol ID) table entry.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, entryIndex, direction
* @retval GT_BAD_PTR               - etherTypePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanTpidEntryGet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS           result;
    GT_U8               devNum;
    CPSS_DXCH_ETHERTYPE_TABLE_ENT  tableType;
    GT_U32              entryIndex;
    GT_U16              etherTypePtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    tableType = (CPSS_DXCH_ETHERTYPE_TABLE_ENT)inArgs[1];
    entryIndex = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanTpidEntryGet(devNum, tableType, entryIndex, &etherTypePtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", etherTypePtr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortIngressTpidSet_1 function
* @endinternal
*
* @brief   Function sets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - tpidBmp initialized with more then 8 bits
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that for the ingress direction multiple TPID may be use to
*       identify Tag0 and Tag1. Each bit at the bitmap represent one of
*       the 8 entries at the TPID Table.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanPortIngressTpidSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                                result;
    GT_U8                                                    devNum;
    GT_PORT_NUM                                              portNum;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT ethMode;
    GT_U32                                                   tpidBmp;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT)inArgs[2];
    tpidBmp = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        result = GT_BAD_PARAM;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChBrgVlanPortIngressTpidSet(devNum, portNum, ethMode, tpidBmp);
    }
    else
    {
        result = wrSip5Emulate_cpssDxChBrgVlanPortIngressTpidSet(devNum, portNum, ethMode, tpidBmp);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortIngressTpidGet_1 function
* @endinternal
*
* @brief   Function gets bitmap of TPID (Tag Protocol ID) table indexes per
*         ingress port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidBmpPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note that for the ingress direction multiple TPID may be use to
*       identify Tag0 and Tag1. Each bit at the bitmap represent one of
*       the 8 entries at the TPID Table.
*
*/
static CMD_STATUS wrCpssDxChBrgVlanPortIngressTpidGet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                                result;
    GT_U8                                                    devNum;
    GT_PORT_NUM                                              portNum;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT ethMode;
    GT_U32                                                   tpidBmpPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        tpidBmpPtr = 0;
        result = GT_BAD_PARAM;
    }
    else if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* call cpss api function */
        result = cpssDxChBrgVlanPortIngressTpidGet(devNum, portNum, ethMode, &tpidBmpPtr);
    }
    else
    {
        result = wrSip5Emulate_cpssDxChBrgVlanPortIngressTpidGet(devNum, portNum, ethMode, &tpidBmpPtr);
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tpidBmpPtr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanPortEgressTpidSet_1 function
* @endinternal
*
* @brief   Function sets index of TPID (Tag protocol ID) table per egress port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_OUT_OF_RANGE          - illegal tpidEntryIndex (not 0-7)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortEgressTpidSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                                   result;
    GT_U8                                                       devNum;
    GT_PORT_NUM                                                 portNum;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT    ethMode;
    GT_U32                                                  tpidEntryIndex;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT)inArgs[2];
    tpidEntryIndex = (GT_U32)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortEgressTpidSet(devNum, portNum, ethMode, tpidEntryIndex);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChBrgVlanPortEgressTpidGet_1 function
* @endinternal
*
* @brief   Function gets index of TPID (Tag Protocol ID) table per egress port.
*
* @note   APPLICABLE DEVICES:      DxChXcat and above
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, portNum, ethMode
* @retval GT_BAD_PTR               - tpidEntryIndexPtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanPortEgressTpidGet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                                                result;
    GT_U8                                                    devNum;
    GT_PORT_NUM                                              portNum;
    CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT ethMode;
    GT_U32                                                   tpidEntryIndexPtr;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PORT_NUM)inArgs[1];
    ethMode = (CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChBrgVlanPortEgressTpidGet(devNum, portNum, ethMode, &tpidEntryIndexPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tpidEntryIndexPtr);

    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanFdbLookupKeyModeSet function
* @endinternal
*
* @brief   Set an FDB Lookup key mode
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanFdbLookupKeyModeSet

(
IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
IN  GT_32 numFields,
OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                             result;

    GT_U8                                 devNum;
    GT_U16                                vlanId;
    CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT mode;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    mode   = (CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanFdbLookupKeyModeSet(devNum, vlanId, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlanMirrorToTxAnalyzerModeSet function
* @endinternal
*
* @brief   Set Egress Mirroring mode to given VLAN.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, vlanId or egressMirroringMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlanMirrorToTxAnalyzerModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                             result;

    GT_U8                                 devNum;
    GT_U16                                vlanId;
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT      egressMirroringMode;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    vlanId = (GT_U16)inArgs[1];
    egressMirroringMode   = (CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssDxChBrgVlanMirrorToTxAnalyzerModeSet(devNum, vlanId, egressMirroringMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlan6BytesTagConfigSet function
* @endinternal
*
* @brief   Set global configurations for 6 bytes v-tag.
*         NOTE: not relevant to standard 8 bytes BPE 802.1BR.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter devNum
* @retval GT_OUT_OF_RANGE          - out of range in value of lBitInSrcId or dBitInSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlan6BytesTagConfigSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8    devNum;
    GT_BOOL  isPortExtender;
    GT_U32   lBitInSrcId;
    GT_U32   dBitInSrcId;

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    isPortExtender = (GT_BOOL)inArgs[1];
    lBitInSrcId = (GT_U32)inArgs[2];
    dBitInSrcId = (GT_U32)inArgs[3];


    /* call cpss api function */
    result = cpssDxChBrgVlan6BytesTagConfigSet(devNum, isPortExtender, lBitInSrcId , dBitInSrcId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChBrgVlan6BytesTagConfigGet function
* @endinternal
*
* @brief   Get global configurations for 6 bytes v-tag.
*         NOTE: not relevant to standard 8 bytes BPE 802.1BR.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChBrgVlan6BytesTagConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8    devNum;
    GT_BOOL  isPortExtender=GT_FALSE;
    GT_U32   lBitInSrcId=0;
    GT_U32   dBitInSrcId=0;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChBrgVlan6BytesTagConfigGet(
        devNum, &isPortExtender, &lBitInSrcId , &dBitInSrcId);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d",
        isPortExtender , lBitInSrcId , dBitInSrcId);
    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{

    {"cpssDxChBrgVlanInit",
        &wrCpssDxChBrgVlanInit,
        1, 0},

    {"cpssDxChBrgVlanEntriesRangeSet",
        &wrCpssDxChBrgVlanEntriesRangeWrite,
        1, 31},

    {"cpssDxChBrgVlanEntriesRangeExt1Set",
        &wrCpssDxChBrgVlanEntriesRangeWrite,
        1, 36},

    {"cpssDxChBrgVlanEntriesRangeExt2Set",
        &wrCpssDxChBrgVlanEntriesRangeWrite,
        1, 49},

    {"cpssDxChBrgVlanEntriesRangeExt3Set",
        &wrCpssDxChBrgVlanEntriesRangeWrite,
        1, 52},

    {"cpssDxChBrgVlanEntriesRangeExt4Set",
        &wrCpssDxChBrgVlanEntriesRangeWrite,
        1, 54},

    {"cpssDxChBrgVlanEntrySet",
        &wrCpssDxChBrgVlanEntryWrite,
        1, 30},

    {"cpssDxChBrgVlanEntryGetFirst",
        &wrCpssDxChBrgVlanEntryReadFirst,
        1, 0},

    {"cpssDxChBrgVlanEntryGetNext",
        &wrCpssDxChBrgVlanEntryReadNext,
        1, 0},

    {"cpssDxChBrgVlanEntryExtSet",
        &wrCpssDxChBrgVlanEntryWrite,
        1, 31},

    {"cpssDxChBrgVlanEntryExtGetFirst",
        &wrCpssDxChBrgVlanEntryExtReadFirst,
        1, 0},

    {"cpssDxChBrgVlanEntryExtGetNext",
        &wrCpssDxChBrgVlanEntryExtReadNext,
        1, 0},

    {"cpssDxChBrgVlanEntryExt1Set",
        &wrCpssDxChBrgVlanEntryWrite,
        1, 36},

    {"cpssDxChBrgVlanEntryExt1GetFirst",
        &wrCpssDxChBrgVlanEntryExt1ReadFirst,
        1, 0},

    {"cpssDxChBrgVlanEntryExt1GetNext",
        &wrCpssDxChBrgVlanEntryExt1ReadNext,
        1, 0},

    {"cpssDxChBrgVlanEntryExt2Set",
        &wrCpssDxChBrgVlanEntryWrite,
        1, 49},

    {"cpssDxChBrgVlanEntryExt3Set",
        &wrCpssDxChBrgVlanEntryWrite,
        1, 52},

    {"cpssDxChBrgVlanEntryExt4Set",
        &wrCpssDxChBrgVlanEntryWrite,
        1, 54},

    {"cpssDxChBrgVlanEntryExt2GetFirst",
        &wrCpssDxChBrgVlanEntryExt2ReadFirst,
        1, 0},

    {"cpssDxChBrgVlanEntryExt2GetNext",
        &wrCpssDxChBrgVlanEntryExt2ReadNext,
        1, 0},

    {"cpssDxChBrgVlanEntryExt3GetFirst",
        &wrCpssDxChBrgVlanEntryExt3ReadFirst,
        1, 0},

    {"cpssDxChBrgVlanEntryExt3GetNext",
        &wrCpssDxChBrgVlanEntryExt3ReadNext,
        1, 0},

    {"cpssDxChBrgVlanEntryExt4GetFirst",
        &wrCpssDxChBrgVlanEntryExt4ReadFirst,
        1, 0},

    {"cpssDxChBrgVlanEntryExt4GetNext",
        &wrCpssDxChBrgVlanEntryExt4ReadNext,
        1, 0},

    {"cpssDxChBrgVlanEntryGetExt4GetFirst",
        &wrCpssDxChBrgVlanEntryExt4ReadRangeFirst,
        3, 0},

    {"cpssDxChBrgVlanEntryGetExt4GetNext",
        &wrCpssDxChBrgVlanEntryExt4ReadNext,
        3, 0},

    {"cpssDxChBrgVlanMembersGetFirst",
        &wrCpssDxChBrgVlanEntryMembersBmpReadRangeFirst,
        3, 0},

    {"cpssDxChBrgVlanMembersGetNext",
        &wrCpssDxChBrgVlanEntryMembersBmpReadRangeNext,
        3, 0},

    {"cpssDxChBrgVlanMembersSet",
        &wrCpssDxChBrgVlanEntryMembersBmpWrite,
        3, 33},

    {"cpssDxChBrgVlanMemberAdd",
        &wrCpssDxChBrgVlanMemberAdd,
        4, 0},

    {"cpssDxChBrgVlanMemberAdd1",
        &wrCpssDxChBrgVlanMemberAdd1,
        5, 0},

    {"cpssDxChBrgVlanPortDelete",
        &wrCpssDxChBrgVlanPortDelete,
        3, 0},

    {"cpssDxChBrgVlanMemberSet",
        &wrCpssDxChBrgVlanMemberSet,
        6, 0},

    {"cpssDxChBrgVlanPortVidGet",
        &wrCpssDxChBrgVlanPortVidGet,
        2, 0},

    {"cpssDxChBrgVlanPortVidGetExt",
        &wrCpssDxChBrgVlanPortVidGetExt,
        3, 0},

    {"cpssDxChBrgVlanPortVidSet",
        &wrCpssDxChBrgVlanPortVidSet,
        3, 0},

    {"cpssDxChBrgVlanPortVidSetExt",
        &wrCpssDxChBrgVlanPortVidSetExt,
        4, 0},

    {"cpssDxChBrgVlanPortIngFltEnable",
        &wrCpssDxChBrgVlanPortIngFltEnable,
        3, 0},

    {"cpssDxChBrgVlanPortIngFltEnableGet",
        &wrCpssDxChBrgVlanPortIngFltEnableGet,
        2, 0},

    {"cpssDxChBrgVlanPortProtoClassVlanEnable",
        &wrCpssDxChBrgVlanPortProtoClassVlanEnable,
        3, 0},

    {"cpssDxChBrgVlanPortProtoClassVlanEnableGet",
        &wrCpssDxChBrgVlanPortProtoClassVlanEnableGet,
        2, 0},

    {"cpssDxChBrgVlanPortProtoClassQosEnable",
        &wrCpssDxChBrgVlanPortProtoClassQosEnable,
        3, 0},

    {"cpssDxChBrgVlanPortProtoClassQosEnableGet",
        &wrCpssDxChBrgVlanPortProtoClassQosEnableGet,
        2, 0},

    {"cpssDxChBrgVlanProtoClassSet",
        &wrCpssDxChBrgVlanProtoClassSet,
        1, 5},

    {"cpssDxChBrgVlanProtoClassGetFirst",
        &wrCpssDxChBrgVlanProtoClassGetFirst,
        1, 0},

    {"cpssDxChBrgVlanProtoClassGetNext",
        &wrCpssDxChBrgVlanProtoClassGetNext,
        1, 0},

    {"cpssDxChBrgVlanProtoClassDelete",
        &wrCpssDxChBrgVlanProtoClassInvalidate,
        1, 1},

    {"cpssDxChBrgVlanPortProtoVlanQosSet",
        &wrCpssDxChBrgVlanPortProtoVlanQosSet,
        2, 9},

    {"cpssDxChBrgVlanPortProtoVlanQosGetFirst",
        &wrCpssDxChBrgVlanPortProtoVlanQosGetFirst,
        2, 0},

    {"cpssDxChBrgVlanPortProtoVlanQosGetNext",
        &wrCpssDxChBrgVlanPortProtoVlanQosGetNext,
        2, 0},

    {"cpssDxChBrgVlanPortProtoVlanQosDelete",
        &wrCpssDxChBrgVlanPortProtoVlanQosInvalidate,
        2, 1},

    {"cpssDxChBrgVlanLearningStateSet",
        &wrCpssDxChBrgVlanLearningStateSet,
        3, 0},

    {"cpssDxChBrgVlanIsDevMember",
        &wrCpssDxChBrgVlanIsDevMember,
        2, 0},

    {"cpssDxChBrgVlanToStpIdBind",
        &wrCpssDxChBrgVlanToStpIdBind,
        3, 0},

    {"cpssDxChBrgVlanStpIdGet",
        &wrCpssDxChBrgVlanStpIdGet,
        2, 0},

    {"cpssDxChBrgVlanEtherTypeSet",
        &wrCpssDxChBrgVlanEtherTypeSet,
        4, 0},

    {"cpssDxChBrgVlanEtherTypeGet",
        &wrCpssDxChBrgVlanEtherTypeGet,
        2, 0},

    {"cpssDxChBrgVlanTableInvalidate",
        &wrCpssDxChBrgVlanTableInvalidate,
        1, 0},

    {"cpssDxChBrgVlanEntryInvalidate",
        &wrCpssDxChBrgVlanEntryInvalidate,
        2, 0},

    {"cpssDxChBrgVlanUnkUnregFilterSet",
        &wrCpssDxChBrgVlanUnkUnregFilterSet,
        4, 0},

    {"cpssDxChBrgVlanForcePvidEnable",
        &wrCpssDxChBrgVlanForcePvidEnable,
        3, 0},

    {"cpssDxChBrgVlanForcePvidEnableGet",
        &wrCpssDxChBrgVlanForcePvidEnableGet,
        2, 0},

    {"cpssDxChBrgVlanPortVidPrecedenceSet",
        &wrCpssDxChBrgVlanPortVidPrecedenceSet,
        3, 0},

    {"cpssDxChBrgVlanPortVidPrecedenceGet",
        &wrCpssDxChBrgVlanPortVidPrecedenceGet,
        2, 0},

    {"cpssDxChBrgVlanIpUcRouteEnable",
        &wrCpssDxChBrgVlanIpUcRouteEnable,
        4, 0},

    {"cpssDxChBrgVlanIpMcRouteEnable",
        &wrCpssDxChBrgVlanIpMcRouteEnable,
        4, 0},

    {"cpssDxChBrgVlanNASecurEnable",
        &wrCpssDxChBrgVlanNASecurEnable,
        3, 0},

    {"cpssDxChBrgVlanIgmpSnoopingEnableGet",
        &wrCpssDxChBrgVlanIgmpSnoopingEnableGet,
        2, 0},

    {"cpssDxChBrgVlanIgmpSnoopingEnable",
        &wrCpssDxChBrgVlanIgmpSnoopingEnable,
        3, 0},

    {"cpssDxChBrgVlanIpCntlToCpuSet",
        &wrCpssDxChBrgVlanIpCntlToCpuSet,
        3, 0},

    {"cpssDxChBrgVlanIpCntlToCpuGet",
        &wrCpssDxChBrgVlanIpCntlToCpuGet,
        2, 0},

    {"cpssDxChBrgVlanIpV6IcmpToCpuEnable",
        &wrCpssDxChBrgVlanIpV6IcmpToCpuEnable,
        3, 0},

    {"cpssDxChBrgVlanIpV6IcmpToCpuEnableGet",
        &wrCpssDxChBrgVlanIpV6IcmpToCpuEnableGet,
        2, 0},

    {"cpssDxChBrgVlanUdpBcPktsToCpuEnable",
        &wrCpssDxChBrgVlanUdpBcPktsToCpuEnable,
        3, 0},

    {"cpssDxChBrgVlanUdpBcPktsToCpuEnableGet",
        &wrCpssDxChBrgVlanUdpBcPktsToCpuEnableGet,
        2, 0},

    {"cpssDxChBrgVlanIpv6SourceSiteIdSet",
        &wrCpssDxChBrgVlanIpv6SourceSiteIdSet,
        3, 0},

    {"cpssDxChBrgVlanIpmBridgingEnable",
        &wrCpssDxChBrgVlanIpmBridgingEnable,
        4, 0},

    {"cpssDxChBrgVlanIpmBridgingModeSet",
        &wrCpssDxChBrgVlanIpmBridgingModeSet,
        4, 0},

    {"cpssDxChBrgVlanIngressMirrorEnable",
        &wrCpssDxChBrgVlanIngressMirrorEnable,
        3, 0},

    {"cpssDxChBrgVlanIngressMirrorEnableExt",
        &wrCpssDxChBrgVlanIngressMirrorEnableExt,
        4, 0},

    {"cpssDxChBrgVlanMirrToTxAnalyzerSet",
        &wrCpssDxChBrgVlanMirrToTxAnalyzerSet,
        4, 0},

    {"cpssDxChBrgVlanForwardingIdSet",
        &wrCpssDxChBrgVlanForwardingIdSet,
        3, 0},

    {"cpssDxChBrgVlanForwardingIdGet",
        &wrCpssDxChBrgVlanForwardingIdGet,
        2, 0},

    {"cpssDxChBrgVlanUnknownMacSaCommandSet",
        &wrCpssDxChBrgVlanUnknownMacSaCommandSet,
        3, 0},

    {"cpssDxChBrgVlanUnknownMacSaCommandGet",
        &wrCpssDxChBrgVlanUnknownMacSaCommandGet,
        2, 0},

    {"cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet",
        &wrCpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet,
        4, 0},

    {"cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet",
        &wrCpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet,
        2, 0},

    {"cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet",
        &wrCpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet,
        4, 0},

    {"cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet",
        &wrCpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet,
        2, 0},

    {"cpssDxChBrgVlanPortAccFrameTypeGet",
        &wrCpssDxChBrgVlanPortAccFrameTypeGet,
        2, 0},

    {"cpssDxChBrgVlanPortAccFrameTypeSet",
        &wrCpssDxChBrgVlanPortAccFrameTypeSet,
        3, 0},

    {"cpssDxChBrgVlanMruProfileIdxSet",
        &wrCpssDxChBrgVlanMruProfileIdxSet,
        3, 0},

    {"cpssDxChBrgVlanMruProfileValueSet",
        &wrCpssDxChBrgVlanMruProfileValueSet,
        3, 0},

    {"cpssDxChBrgVlanMruProfileValueGet",
        &wrCpssDxChBrgVlanMruProfileValueGet,
        2, 0},

    {"cpssDxChBrgVlanNaToCpuEnable",
        &wrCpssDxChBrgVlanNaToCpuEnable,
        3, 0},

    {"cpssDxChBrgVlanBridgingModeSet",
        &wrCpssDxChBrgVlanBridgingModeSet,
        2, 0},

    {"cpssDxChBrgVlanBridgingModeGet",
        &wrCpssDxChBrgVlanBridgingModeGet,
        1, 0},

    {"cpssDxChBrgVlanVrfIdSet",
        &wrCpssDxChBrgVlanVrfIdSet,
        3, 0},

    {"cpssDxChBrgVlanPortTranslationEnableSet",
        &wrCpssDxChBrgVlanPortTranslationEnableSet,
        4, 0},

    {"cpssDxChBrgVlanPortTranslationEnableGet",
        &wrCpssDxChBrgVlanPortTranslationEnableGet,
        3, 0},

    {"cpssDxChBrgVlanPortTranslationEnableSet_1",
        &wrCpssDxChBrgVlanPortTranslationEnableSet,
        4, 0},

    {"cpssDxChBrgVlanPortTranslationEnableGet_1",
        &wrCpssDxChBrgVlanPortTranslationEnableGet,
        3, 0},

    {"cpssDxChBrgVlanTranslationEntryWrite",
        &wrCpssDxChBrgVlanTranslationEntryWrite,
        4, 0},

    {"cpssDxChBrgVlanTranslationEntryRead",
        &wrCpssDxChBrgVlanTranslationEntryRead,
        3, 0},
    {"cpssDxChBrgVlanValidCheckEnableSet",
         &wrCpssDxChBrgVlanValidCheckEnableSet,
         2, 0},
    {"cpssDxChBrgVlanValidCheckEnableGet",
         &wrCpssDxChBrgVlanValidCheckEnableGet,
         1, 0},

    {"cpssDxChBrgVlanTpidEntrySet",
         &wrCpssDxChBrgVlanTpidEntrySet,
         4, 0},

    {"cpssDxChBrgVlanTpidEntryGet",
         &wrCpssDxChBrgVlanTpidEntryGet,
         3, 0},

    {"cpssDxChBrgVlanPortIngressTpidSet",
         &wrCpssDxChBrgVlanPortIngressTpidSet,
         4, 0},

    {"cpssDxChBrgVlanPortIngressTpidGet",
         &wrCpssDxChBrgVlanPortIngressTpidGet,
         3, 0},

    {"cpssDxChBrgVlanPortEgressTpidSet",
         &wrCpssDxChBrgVlanPortEgressTpidSet,
         4, 0},

    {"cpssDxChBrgVlanPortEgressTpidGet",
         &wrCpssDxChBrgVlanPortEgressTpidGet,
         3, 0},

    {"cpssDxChBrgVlanRangeSet",
         &wrCpssDxChBrgVlanRangeSet,
         2, 0},

    {"cpssDxChBrgVlanRangeGet",
         &wrCpssDxChBrgVlanRangeGet,
         1, 0},

    {"cpssDxChBrgVlanPortIsolationCmdSet",
         &wrCpssDxChBrgVlanPortIsolationCmdSet,
         3, 0},

    {"cpssDxChBrgVlanLocalSwitchingEnableSet",
         &wrCpssDxChBrgVlanLocalSwitchingEnableSet,
         4, 0},

    {"cpssDxChBrgVlanFloodVidxModeSet",
         &wrCpssDxChBrgVlanFloodVidxModeSet,
         4, 0},

    {"cpssDxChBrgVlanTagCommandSet",
        &wrCpssDxChBrgVlanTagCommandSet,
        3, 2},

    {"cpssDxChBrgVlanTagCommandGetFirst",
        &wrCpssDxChBrgVlanTagCommandGetFirst,
        3, 0},

    {"cpssDxChBrgVlanTagCommandGetNext",
        &wrCpssDxChBrgVlanTagCommandGetNext,
        3, 0},

    {"cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet",
        &wrCpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet,
        2, 0},

    {"cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet",
        &wrCpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet,
        1, 0},

    {"cpssDxChBrgVlanForceNewDsaToCpuEnableSet",
        &wrCpssDxChBrgVlanForceNewDsaToCpuEnableSet,
        2, 0},

    {"cpssDxChBrgVlanForceNewDsaToCpuEnableGet",
        &wrCpssDxChBrgVlanForceNewDsaToCpuEnableGet,
        1, 0},

    {"cpssDxChBrgVlanKeepVlan1EnableSet",
        &wrCpssDxChBrgVlanKeepVlan1EnableSet,
        4, 0},

    {"cpssDxChBrgVlanKeepVlan1EnableGet",
        &wrCpssDxChBrgVlanKeepVlan1EnableGet,
        3, 0},

    {"cpssDxChBrgVlanIngressTpidProfileSet",
        &wrCpssDxChBrgVlanIngressTpidProfileSet,
        4, 0},

    {"cpssDxChBrgVlanIngressTpidProfileGet",
        &wrCpssDxChBrgVlanIngressTpidProfileGet,
        3, 0},

    {"cpssDxChBrgVlanPortIngressTpidProfileSet",
        &wrCpssDxChBrgVlanPortIngressTpidProfileSet,
        5, 0},

    {"cpssDxChBrgVlanPortIngressTpidProfileGet",
        &wrCpssDxChBrgVlanPortIngressTpidProfileGet,
        4, 0},

    {"cpssDxChBrgVlanPortVid1Set",
        &wrCpssDxChBrgVlanPortVid1Set,
        4, 0},

    {"cpssDxChBrgVlanPortVid1Get",
        &wrCpssDxChBrgVlanPortVid1Get,
        3, 0},

    {"cpssDxChBrgVlanEgressTagTpidSelectModeSet",
        &wrCpssDxChBrgVlanEgressTagTpidSelectModeSet,
        4, 0},

    {"cpssDxChBrgVlanEgressTagTpidSelectModeGet",
        &wrCpssDxChBrgVlanEgressTagTpidSelectModeGet,
        3, 0},

    {"cpssDxChBrgVlanEgressPortTagStateModeSet",
        &wrCpssDxChBrgVlanEgressPortTagStateModeSet,
        3, 0},

    {"cpssDxChBrgVlanEgressPortTagStateModeGet",
        &wrCpssDxChBrgVlanEgressPortTagStateModeGet,
        2, 0},

    {"cpssDxChBrgVlanEgressPortTagStateSet",
        &wrCpssDxChBrgVlanEgressPortTagStateSet,
        3, 0},

    {"cpssDxChBrgVlanEgressPortTagStateGet",
        &wrCpssDxChBrgVlanEgressPortTagStateGet,
        2, 0},

    {"cpssDxChBrgVlanFcoeForwardingEnableSet",
        &wrCpssDxChBrgVlanFcoeForwardingEnableSet,
        3, 0},

    {"cpssDxChBrgVlanFcoeForwardingEnableGet",
        &wrCpssDxChBrgVlanFcoeForwardingEnableGet,
        2, 0},

    {"cpssDxChBrgVlanUnregisteredIpmEVidxSet",
        &wrCpssDxChBrgVlanUnregisteredIpmEVidxSet,
        4, 0},

    {"cpssDxChBrgVlanPortEnLearningOfOriginalTag1VidSet",
        &wrCpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet,
        3, 0},

    {"cpssDxChBrgVlanPortEnLearningOfOriginalTag1VidGet",
        &wrCpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet,
        2, 0},

    {"cpssDxChBrgVlanPortEgressVlanFilteringEnableSet",
        &wrCpssDxChBrgVlanPortEgressVlanFilteringEnableSet,
        3, 0},

    {"cpssDxChBrgVlanPortEgressVlanFilteringEnableGet",
        &wrCpssDxChBrgVlanPortEgressVlanFilteringEnableGet,
        2, 0},

    {"cpssDxChBrgVlanPortPushVlanCommandSet",
        &wrCpssDxChBrgVlanPortPushVlanCommandSet,
        3, 0},

    {"cpssDxChBrgVlanPortPushVlanCommandGet",
        &wrCpssDxChBrgVlanPortPushVlanCommandGet,
        2, 0},

    {"cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet",
        &wrCpssDxChBrgVlanPortPushedTagTpidSelectEgressSet,
        3, 0},

    {"cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet",
        &wrCpssDxChBrgVlanPortPushedTagTpidSelectEgressGet,
        2, 0},

    {"cpssDxChBrgVlanPortPushedTagValueSet",
        &wrCpssDxChBrgVlanPortPushedTagValueSet,
        3, 0},

    {"cpssDxChBrgVlanPortPushedTagValueGet",
        &wrCpssDxChBrgVlanPortPushedTagValueGet,
        2, 0},

    {"cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet",
        &wrCpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet,
        3, 0},

    {"cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet",
        &wrCpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet,
        2, 0},

    {"cpssDxChBrgVlanPortUpSet",
        &wrCpssDxChBrgVlanPortUpSet,
        3, 0},

    {"cpssDxChBrgVlanPortUpGet",
        &wrCpssDxChBrgVlanPortUpGet,
        2, 0},

    {"cpssDxChBrgVlanPortCfiEnableSet",
        &wrCpssDxChBrgVlanPortCfiEnableSet,
        3, 0},

    {"cpssDxChBrgVlanPortCfiEnableGet",
        &wrCpssDxChBrgVlanPortCfiEnableGet,
        2, 0},

    {"cpssDxChBrgVlanPortNumOfTagWordsToPopSet",
        &wrcpssDxChBrgVlanPortNumOfTagWordsToPopSet,
        3, 0},

    {"cpssDxChBrgVlanPortNumOfTagWordsToPopGet",
        &wrcpssDxChBrgVlanPortNumOfTagWordsToPopGet,
        2, 0},

    {"cpssDxChBrgVlanPortVid0CommandEnableSet",
        &wrCpssDxChBrgVlanPortVid0CommandEnableSet,
        3, 0},

    {"cpssDxChBrgVlanPortVid0CommandEnableGet",
        &wrCpssDxChBrgVlanPortVid0CommandEnableGet,
        2, 0},

    {"cpssDxChBrgVlanPortVid1CommandEnableSet",
        &wrCpssDxChBrgVlanPortVid1CommandEnableSet,
        3, 0},

    {"cpssDxChBrgVlanPortVid1CommandEnableGet",
        &wrCpssDxChBrgVlanPortVid1CommandEnableGet,
        2, 0},

    {"cpssDxChBrgVlanPortUp0CommandEnableSet",
        &wrCpssDxChBrgVlanPortUp0CommandEnableSet,
        3, 0},

    {"cpssDxChBrgVlanPortUp0CommandEnableGet",
        &wrCpssDxChBrgVlanPortUp0CommandEnableGet,
        2, 0},

    {"cpssDxChBrgVlanPortUp1CommandEnableSet",
        &wrCpssDxChBrgVlanPortUp1CommandEnableSet,
        3, 0},

    {"cpssDxChBrgVlanPortUp1CommandEnableGet",
        &wrCpssDxChBrgVlanPortUp1CommandEnableGet,
        2, 0},

    {"cpssDxChBrgVlanPortUp0Set",
        &wrCpssDxChBrgVlanPortUp0Set,
        3, 0},

    {"cpssDxChBrgVlanPortUp0Get",
        &wrCpssDxChBrgVlanPortUp0Get,
        2, 0},

    {"cpssDxChBrgVlanPortUp1Set",
        &wrCpssDxChBrgVlanPortUp1Set,
        3, 0},

    {"cpssDxChBrgVlanPortUp1Get",
        &wrCpssDxChBrgVlanPortUp1Get,
        2, 0},

    {"cpssDxChBrgVlanIndependentNonFloodVidxEnableSet",
        &wrCpssDxChBrgVlanIndependentNonFloodVidxEnableSet,
        2, 0},

    {"cpssDxChBrgVlanIndependentNonFloodVidxEnableGet",
        &wrCpssDxChBrgVlanIndependentNonFloodVidxEnableGet,
        1, 0},

    {"cpssDxChBrgVlanPortRemoveTag1IfRxWOTag1EnableSet",
        &wrCpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet,
        3, 0},

    {"cpssDxChBrgVlanPortRemoveTag1IfRxWOTag1EnableGet",
        &wrCpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet,
        2, 0},

    {"cpssDxChBrgVlanTpidTagTypeSet",
        &wrCpssDxChBrgVlanTpidTagTypeSet,
        4, 0},

    {"cpssDxChBrgVlanTpidTagTypeGet",
        &wrCpssDxChBrgVlanTpidTagTypeGet,
        3, 0},

    {"cpssDxChBrgVlanMembersTableIndexingModeSet",
        &wrCpssDxChBrgVlanMembersTableIndexingModeSet,
        3, 0},

    {"cpssDxChBrgVlanMembersTableIndexingModeGet",
        &wrCpssDxChBrgVlanMembersTableIndexingModeGet,
        2, 0},

    {"cpssDxChBrgVlanStgIndexingModeSet",
        &wrCpssDxChBrgVlanStgIndexingModeSet,
        3, 0},

    {"cpssDxChBrgVlanStgIndexingModeGet",
        &wrCpssDxChBrgVlanStgIndexingModeGet,
        2, 0},

    {"cpssDxChBrgVlanTagStateIndexingModeSet",
        &wrCpssDxChBrgVlanTagStateIndexingModeSet,
        3, 0},

    {"cpssDxChBrgVlanTagStateIndexingModeGet",
        &wrCpssDxChBrgVlanTagStateIndexingModeGet,
        2, 0},

    {"cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet",
        &wrCpssDxChBrgVlanBpeTagSourceCidDefaultForceSet,
        3, 0},

    {"cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet",
        &wrCpssDxChBrgVlanBpeTagSourceCidDefaultForceGet,
        2, 0},

    {"cpssDxChBrgVlanBpeTagMcCfgSet",
        &wrCpssDxChBrgVlanBpeTagMcCfgSet,
        4, 0},

    {"cpssDxChBrgVlanBpeTagMcCfgGet",
        &wrCpssDxChBrgVlanBpeTagMcCfgGet,
        1, 0},

    {"cpssDxChBrgVlanBpeTagReservedFieldsSet",
        &wrCpssDxChBrgVlanBpeTagReservedFieldsSet,
        4, 0},

    {"cpssDxChBrgVlanBpeTagReservedFieldsGet",
        &wrCpssDxChBrgVlanBpeTagReservedFieldsGet,
        1, 0},

    {"cpssDxChBrgVlanTpidEntrySet_1",
         &wrCpssDxChBrgVlanTpidEntrySet_1,
         4, 0},

    {"cpssDxChBrgVlanTpidEntryGet_1",
         &wrCpssDxChBrgVlanTpidEntryGet_1,
         3, 0},

    {"cpssDxChBrgVlanPortIngressTpidSet_1",
         &wrCpssDxChBrgVlanPortIngressTpidSet_1,
         4, 0},

    {"cpssDxChBrgVlanPortIngressTpidGet_1",
         &wrCpssDxChBrgVlanPortIngressTpidGet_1,
         3, 0},

    {"cpssDxChBrgVlanPortEgressTpidSet_1",
         &wrCpssDxChBrgVlanPortEgressTpidSet_1,
         4, 0},

    {"cpssDxChBrgVlanPortEgressTpidGet_1",
         &wrCpssDxChBrgVlanPortEgressTpidGet_1,
         3, 0},

    {"cpssDxChBrgVlanFdbLookupKeyModeSet",
        &wrCpssDxChBrgVlanFdbLookupKeyModeSet,
        3, 0},

    {"cpssDxChBrgVlan6BytesTagConfigSet",
        &wrCpssDxChBrgVlan6BytesTagConfigSet,
        4, 0},

    {"cpssDxChBrgVlan6BytesTagConfigGet",
        &wrCpssDxChBrgVlan6BytesTagConfigGet,
        1, 0},

    {"cpssDxChBrgVlanMirrorToTxAnalyzerModeSet",
        &wrCpssDxChBrgVlanMirrorToTxAnalyzerModeSet,
        3, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChBridgeVlan function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitCpssDxChBridgeVlan
(
GT_VOID
)
{
    GT_STATUS rc;

    rc = wrCpssRegisterResetCb(cmdLibResetCpssDxChBridgeVlan);
    if(rc != GT_OK)
    {
        return rc;
    }

    return cmdInitLibrary(dbCommands, numCommands);
}

