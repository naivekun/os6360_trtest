--********************************************************************************
--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* nat44.lua
--*
--* DESCRIPTION:
--*       manage IPv4 NAT session
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
--require
--constants
local pclRuleType = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"
local pclKeySize = "CPSS_PCL_RULE_SIZE_STD_E"
local pclPacketType

local groupKeyTypes = {
    nonIpKey = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E",
    ipv4Key = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E",
    ipv6Key = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"
}

-- reserved pclid for the lookup
local reserved_pclId = 0x135

local debug_on = false
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
    end
end

local savedIngressPolicyState = true

-- call cpssDxChIpRouterGlobalMacSaSet with %256 of index
function lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,routerMacSaIndex,macSaAddr)
    if routerMacSaIndex >= 256 then
        routerMacSaIndex = routerMacSaIndex % 256
    end
    -- Sets full 48-bit Router MAC SA in Global MAC SA table
    local apiName = "cpssDxChIpRouterGlobalMacSaSet"
    local result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",           "devNum",           devNum       },
        { "IN",     "GT_U32",          "routerMacSaIndex", routerMacSaIndex      },
        { "IN",     "GT_ETHERADDR",    "macSaAddrPtr",     macSaAddr},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)
end

-- call cpssDxChIpRouterGlobalMacSaSet with %256 of routerMacSaIndex
function lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,portNum,routerMacSaIndex)
    if routerMacSaIndex >= 256 then
        routerMacSaIndex = routerMacSaIndex % 256
    end
    -- Set router mac sa index refered to global MAC SA table
    local apiName = "cpssDxChIpRouterPortGlobalMacSaIndexSet"
    local result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",           "devNum",           devNum     },
        { "IN",     "GT_PORT_NUM",     "portNum",          portNum    },
        { "IN",     "GT_U32",          "routerMacSaIndex", routerMacSaIndex    },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)
end

-- call cpssDxChIpRouterGlobalMacSaGet with %256 of routerMacSaIndex
function lua_cpssDxChIpRouterGlobalMacSaGet(devNum,routerMacSaIndex)
    if routerMacSaIndex >= 256 then
        routerMacSaIndex = routerMacSaIndex % 256
    end

   -- Gets full 48-bit Router MAC SA in Global MAC SA table
    local apiName = "cpssDxChIpRouterGlobalMacSaGet"
    local result, values = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",           "devNum",           devNum },
        { "IN",     "GT_U32",          "routerMacSaIndex", routerMacSaIndex},
        { "OUT",     "GT_ETHERADDR",    "macSaAddrPtr"},
    })

    return result, values
end


---------------------------- Default private functions ---------------------------

-- set default values for api cpssDxChBrgEportToPhysicalPortTargetMappingTableSet.
--
-- inputs :
-- devNum - device number
-- portNum - port number
--
local function default_cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,portNum)

    local command_data = Command_Data()

    -- Common variables declaration
    local apiName,result

    local physicalInfo = {}

    physicalInfo.type = "CPSS_INTERFACE_PORT_E"
    physicalInfo.devPort = {}
    physicalInfo.devPort.devNum  = 0
    physicalInfo.devPort.portNum = 0

    -- unset physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum},
        { "IN",     "GT_PORT_NUM",             "portNum",         portNum},
        { "IN",     "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

end

-- set default values for api cpssDxChIpNatEntrySet.
--
-- inputs :
-- devNum - device number
-- natIndex - index for the NAT entry
--
local function default_cpssDxChIpNatEntrySet(devNum,natIndex)

    local command_data = Command_Data()

    -- Common variables declaration
    local apiName,result

    local nat44Entry = {}

    -- unset a NAT entry
    apiName = "cpssDxChIpNatEntrySet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                    "devNum",   devNum },
        { "IN",     "GT_U32",                                   "natIndex", natIndex },
        { "IN",     "CPSS_IP_NAT_TYPE_ENT",                     "natType",  "CPSS_IP_NAT_TYPE_NAT44_E" },
        { "IN",     "CPSS_DXCH_IP_NAT_ENTRY_UNT_nat44Entry",    "entryPtr", nat44Entry}
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

end

-- set default values for api cpssDxChIpUcRouteEntriesWrite.
--
-- inputs :
-- devNum - device number
-- ipNextHopIndex - the index from which to write the array
--
local function default_cpssDxChIpUcRouteEntriesWrite(devNum,ipNextHopIndex)

    local command_data = Command_Data()

    -- Common variables declaration
    local apiName,result

    local routeEntriesArray = {}

    routeEntriesArray.type = "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E"
    routeEntriesArray.entry = {}
    routeEntriesArray.entry.regularEntry = {}
    routeEntriesArray.entry.regularEntry.cmd = "CPSS_PACKET_CMD_ROUTE_E"
    routeEntriesArray.entry.regularEntry.cpuCodeIdx = "CPSS_DXCH_IP_CPU_CODE_IDX_0_E"
    routeEntriesArray.entry.regularEntry.qosPrecedence = "CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E"
    routeEntriesArray.entry.regularEntry.modifyUp = "CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E"
    routeEntriesArray.entry.regularEntry.modifyDscp = "CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E"
    routeEntriesArray.entry.regularEntry.countSet = "CPSS_IP_CNT_SET0_E"
    routeEntriesArray.entry.regularEntry.siteId = "CPSS_IP_SITE_ID_INTERNAL_E"
    routeEntriesArray.entry.regularEntry.nextHopInterface = {}
    routeEntriesArray.entry.regularEntry.nextHopInterface.type = "CPSS_INTERFACE_PORT_E"
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort = {}

    -- Writes default array of uc route entries to hw
    apiName = "cpssDxChIpUcRouteEntriesWrite"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                            "devNum",               devNum },
        { "IN",     "GT_U32",                           "baseRouteEntryIndex",  ipNextHopIndex},
        { "IN",     "CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC",  "routeEntriesArray",    routeEntriesArray },
        { "IN",     "GT_U32",                           "numOfRouteEntries",    1}
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

end

-- set default values for api cpssDxChLpmLeafEntryWrite.
--
-- inputs :
-- devNum - device number
-- leafIndex - leaf index within the LPM PBR block
--
local function default_cpssDxChLpmLeafEntryWrite(devNum,leafIndex)

    local command_data = Command_Data()

    -- Common variables declaration
    local apiName,result

    local leafPtr = {}

    leafPtr.entryType = "CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E"
    leafPtr.index = leafIndex
    leafPtr.ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E"
    leafPtr.ucRPFCheckEnable = GT_FALSE
    leafPtr.sipSaCheckMismatchEnable = GT_FALSE

    -- Write default LPM leaf entry, which is used for policy based routing, to the HW
    apiName = "cpssDxChLpmLeafEntryWrite"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                            "devNum",               devNum },
        { "IN",     "GT_U32",                           "leafIndex",            leafIndex},
        { "IN",     "CPSS_DXCH_LPM_LEAF_ENTRY_STC",     "leafPtr",              leafPtr }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

end

-- set default values for api cpssDxChPclCfgTblSet.
--
-- inputs :
-- devNum - device number
-- interfaceInfoPtr - interface data: port, VLAN, or index
-- direction - Policy direction: Ingress or Egress
-- lookupNum - Lookup number
--
local function default_cpssDxChPclCfgTblSet(devNum,interfaceInfoPtr,direction,lookupNum)

    local command_data = Command_Data()

    -- Common variables declaration
    local apiName,result

    local groupKeyTypes = {}
    local lookupCfg = {}

    groupKeyTypes.nonIpKey = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"
    groupKeyTypes.ipv4Key = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"
    groupKeyTypes.ipv6Key = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"

    lookupCfg.enableLookup = false
    lookupCfg.pclId = 0
    lookupCfg.groupKeyTypes = groupKeyTypes

    -- unset PCL Configuration table entry's lookup configuration for interface.
    apiName = "cpssDxChPclCfgTblSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",               devNum },
        { "IN",     "CPSS_INTERFACE_INFO_STC",      "interfaceInfoPtr",     interfaceInfoPtr},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",            direction},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",            lookupNum},
        { "IN",     "CPSS_DXCH_PCL_LOOKUP_CFG_STC", "lookupCfgPtr",         lookupCfg},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

end

-- set default values for api cpssDxChBrgEportToPhysicalPortEgressPortInfoSet.
--
-- inputs :
-- devNum - device number
-- portNum - port number
--
local function default_cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum,portNum)

    local command_data = Command_Data()

    -- Common variables declaration
    local apiName,result

    local egressInfo = {}

    egressInfo.tunnelStart = false
    egressInfo.tunnelStartPtr = 0
    egressInfo.tsPassengerPacketType  = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
    egressInfo.arpPtr = 0
    egressInfo.modifyMacSa = false
    egressInfo.modifyMacDa = false

    -- unset physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum},
        { "IN",     "GT_PORT_NUM",             "portNum",         portNum},
        { "IN",     "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", egressInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

end

CLI_type_dict["L4_protocol"] = {

    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,

    help = "UDP or TCP",
    enum = {
        ["UDP"] = { value="UDP", help="UDP protocol" },
        ["TCP"] = { value="TCP", help="TCP protocol" },
    }
}

CLI_type_dict["udp_tcp_port"] = {
    help = "UDP/TCP port number (1..65535)",
    checker = CLI_check_param_number,
    min = 1,
    max = 65535
}

-- define the UDBs that will be needed for the PCL classification private to private
--[[ we need next fields:
<private-ip-addr> SIP

        ipv4 - 4 bytes in network order L3 offset 14..17
        0.**> 'L3 offset' byte 14 bits 0..7 : private-ip-addr[7:0]
        1.**> 'L3 offset' byte 15 bits 0..7 : private-ip-addr[15:8]

<private-ip-addr> DIP

        ipv4 - 4 bytes in network order L3 offset 18..21
        2.**> 'L3 offset' byte 18 bits 0..7 : private-ip-addr[7:0]
        3.**> 'L3 offset' byte 19 bits 0..7 : private-ip-addr[15:8]
total: 4 udbs
--]]

local pclUdbArr_prv =
{   --byte#             offset type                                     offset
    -- L3 section
    { udbIndex = 0,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 14      }, -- bits 0..7  : private-ip-addr[7:0]
    { udbIndex = 1,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 15      }, -- bits 0..7  : private-ip-addr[15:8]
    { udbIndex = 4,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 18      }, -- bits 0..7  : private-ip-addr[7:0]
    { udbIndex = 5,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 19      },  -- bits 0..7  : private-ip-addr[15:8]
    -- metadata section
    { udbIndex = 35,   offsetType = "CPSS_DXCH_PCL_OFFSET_METADATA_E",    offset = 51      }  -- bit 1 = 0
}

-- pclUdbArray
local pclPatternPtr_prv = {ruleIngrStdUdb={
    commonIngrUdb={
        pclId = reserved_pclId,
        macToMe = 1,
        isIp = 1},
    isIpv4 = 1,
    udb0_15  = {
        [0] = FILLED_IN_RUN_TIME,                   -- bits 0..7  : private-ip-addr[7:0]
        [1] = FILLED_IN_RUN_TIME,                   -- bits 8..15 : private-ip-addr[15:8]
        [2] = 0,
        [3] = 0,
        [4] = FILLED_IN_RUN_TIME,                   -- bits 0..7  : private-ip-addr[7:0]
        [5] = FILLED_IN_RUN_TIME,                   -- bits 8..15 : private-ip-addr[15:8]
        [6] = 0,
        [7] = 0,
        [8] = 0,
        [9] = 0,
        [10] = 0,
        [11] = 0,
        [12] = 0,
        [13] = 0,
        [14] = 0,
        [15] = 0
    } }}

-- pclUdbArray
local pclMaskPtr_prv = {ruleIngrStdUdb={
    commonIngrUdb={
        pclId = 0x3ff,
        macToMe = 1,
        isIp = 1},
    isIpv4 = 1,
    udb0_15  = {
        [0] = 0xff,                     -- bits 0..7  : private-ip-addr[7:0]
        [1] = 0xff,                     -- bits 8..15 : private-ip-addr[15:8]
        [2] = 0,
        [3] = 0,
        [4] = 0xff,                     -- bits 0..7  : private-ip-addr[7:0]
        [5] = 0xff,                     -- bits 8..15 : private-ip-addr[15:8]
        [6] = 0,
        [7] = 0,
        [8] = 0,
        [9] = 0,
        [10] = 0,
        [11] = 0,
        [12] = 0,
        [13] = 0,
        [14] = 0,
        [15] = 0
    } } }

-- define the UDBs that will be needed for the PCL classification private to public
--[[ we need next fields:
<private-ip-addr> SIP

        ipv4 - 4 bytes in network order L3 offset 14..17
        0.**> 'L3 offset' byte 14 bits 0..7 : private-ip-addr[7:0]
        1.**> 'L3 offset' byte 15 bits 0..7 : private-ip-addr[15:8]
        2.**> 'L3 offset' byte 16 bits 0..7 : private-ip-addr[23:16]
        3.**> 'L3 offset' byte 17 bits 0..7 : private-ip-addr[31:24]

<server-ip-addr> DIP

        ipv4 - 4 bytes in network order L3 offset 18..21
        4.**> 'L3 offset' byte 18 bits 0..7 : server-ip-addr[7:0]
        5.**> 'L3 offset' byte 19 bits 0..7 : server-ip-addr[15:8]
        6.**> 'L3 offset' byte 20 bits 0..7 : server-ip-addr[23:16]
        7.**> 'L3 offset' byte 21 bits 0..7 : server-ip-addr[31:24]

<private-L4-port>

        8.**> 'L4 offset' byte 0 bits 0..7 : private-L4-port[15:8]
        9.**> 'L4 offset' byte 1 bits 0..7 : private-L4-port[7:0]

<server-L4-port>

        10.**> 'L4 offset' byte 2 bits 0..7 : server-L4-port[15:8]
        11.**> 'L4 offset' byte 3 bits 0..7 : server-L4-port[7:0]

<protocol> 'L4 Valid'

        12.**> 'metadata' byte 51 bit 1 = 1

total: 13 udbs
--]]

local pclUdbArr =
{   --byte#             offset type                                     offset
    -- L3 section
    { udbIndex = 0,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 14      }, -- bits 0..7  : private-ip-addr[7:0]
    { udbIndex = 1,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 15      }, -- bits 0..7  : private-ip-addr[15:8]
    { udbIndex = 2,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 16      }, -- bits 0..7  : private-ip-addr[23:16]
    { udbIndex = 3,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 17      }, -- bits 0..7  : private-ip-addr[31:24]
    { udbIndex = 4,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 18      }, -- bits 0..7  : server-ip-addr[7:0]
    { udbIndex = 5,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 19      }, -- bits 0..7  : server-ip-addr[15:8]
    { udbIndex = 6,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 20      }, -- bits 0..7  : server-ip-addr[23:16]
    { udbIndex = 7,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 21      }, -- bits 0..7  : server-ip-addr[31:24]
    -- L4 section
    { udbIndex = 8,    offsetType = "CPSS_DXCH_PCL_OFFSET_L4_E",          offset = 0       }, -- bits 0..7 : private-L4-port[15:8]
    { udbIndex = 9,    offsetType = "CPSS_DXCH_PCL_OFFSET_L4_E",          offset = 1       }, -- bits 0..7 : private-L4-port[7:0]
    { udbIndex = 10,   offsetType = "CPSS_DXCH_PCL_OFFSET_L4_E",          offset = 2       }, -- bits 0..7 : server-L4-port[15:8]
    { udbIndex = 11,   offsetType = "CPSS_DXCH_PCL_OFFSET_L4_E",          offset = 3       }, -- bits 0..7 : server-L4-port[7:0]
    -- metadata section
    { udbIndex = 35,   offsetType = "CPSS_DXCH_PCL_OFFSET_METADATA_E",    offset = 51      }  -- bit 1 = 1
}

local pclUdbArr_noL4 =
{   --byte#             offset type                                     offset
    -- L3 section
    { udbIndex = 0,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 14      }, -- bits 0..7  : private-ip-addr[7:0]
    { udbIndex = 1,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 15      }, -- bits 0..7  : private-ip-addr[15:8]
    { udbIndex = 2,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 16      }, -- bits 0..7  : private-ip-addr[23:16]
    { udbIndex = 3,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 17      }, -- bits 0..7  : private-ip-addr[31:24]
    { udbIndex = 4,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 18      }, -- bits 0..7  : server-ip-addr[7:0]
    { udbIndex = 5,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 19      }, -- bits 0..7  : server-ip-addr[15:8]
    { udbIndex = 6,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 20      }, -- bits 0..7  : server-ip-addr[23:16]
    { udbIndex = 7,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 21      }, -- bits 0..7  : server-ip-addr[31:24]
    -- metadata section
    { udbIndex = 35,   offsetType = "CPSS_DXCH_PCL_OFFSET_METADATA_E",    offset = 51      }  -- bit 1 = 1
}

-- pclUdbArray
local pclPatternPtr = {ruleIngrStdUdb={
    commonIngrUdb={
        pclId = reserved_pclId,
        macToMe = 1,
        isIp = 1},
    isIpv4 = 1,
    udb0_15  = {
        [0] = FILLED_IN_RUN_TIME,                   -- bits 0..7   : private-ip-addr[7:0]
        [1] = FILLED_IN_RUN_TIME,                   -- bits 8..15  : private-ip-addr[8:15]
        [2] = FILLED_IN_RUN_TIME,                   -- bits 16..23 : private-ip-addr[16:23]
        [3] = FILLED_IN_RUN_TIME,                   -- bits 24..31 : private-ip-addr[24:31]
        [4] = FILLED_IN_RUN_TIME,                   -- bits 0..7   : server-ip-addr[7:0]
        [5] = FILLED_IN_RUN_TIME,                   -- bits 8..15  : server-ip-addr[8:15]
        [6] = FILLED_IN_RUN_TIME,                   -- bits 16..23 : server-ip-addr[16:23]
        [7] = FILLED_IN_RUN_TIME,                   -- bits 24..31 : server-ip-addr[24:31]
        [8] = FILLED_IN_RUN_TIME,                   -- bits 0..7   : private-L4-port[15:8]
        [9] = FILLED_IN_RUN_TIME,                   -- bits 0..7   : private-L4-port[7:0]
        [10] = FILLED_IN_RUN_TIME,                  -- bits 0..7   : server-L4-port[15:8]
        [11] = FILLED_IN_RUN_TIME,                  -- bits 0..7   : server-L4-port[7:0]
        [12] = 0,
        [13] = 0,
        [14] = 0,
        [15] = 0
    } }}

-- pclUdbArray
local pclMaskPtr = {ruleIngrStdUdb={
    commonIngrUdb={
        pclId = 0x3ff,
        macToMe = 1,
        isIp = 1},
    isIpv4 = 1,
    udb0_15  = {
        [0] = 0xff,                     -- bits 0..7   : private-ip-addr[7:0]
        [1] = 0xff,                     -- bits 8..15  : private-ip-addr[8:15]
        [2] = 0xff,                     -- bits 16..23 : private-ip-addr[16:23]
        [3] = 0xff,                     -- bits 24..31 : private-ip-addr[24:31]
        [4] = 0xff,                     -- bits 0..7   : server-ip-addr[7:0]
        [5] = 0xff,                     -- bits 8..15  : server-ip-addr[8:15]
        [6] = 0xff,                     -- bits 16..23 : server-ip-addr[16:23]
        [7] = 0xff,                     -- bits 24..31 : server-ip-addr[24:31]
        [8] = 0xff,                     -- bits 0..7   : private-L4-port[15:8]
        [9] = 0xff,                     -- bits 0..7   : private-L4-port[7:0]
        [10] = 0xff,                    -- bits 0..7   : server-L4-port[15:8]
        [11] = 0xff,                    -- bits 0..7   : server-L4-port[7:0]
        [12] = 0,
        [13] = 0,
        [14] = 0,
        [15] = 0
    } }}

-- define the UDBs that will be needed for the PCL classification public to private
--[[ we need next fields:
<server-ip-addr> SIP

        ipv4 - 4 bytes in network order L3 offset 14..17
        1.**> 'L3 offset' byte 18 bits 0..7 : server-ip-addr[7:0]
        2.**> 'L3 offset' byte 19 bits 0..7 : server-ip-addr[15:8]
        3.**> 'L3 offset' byte 20 bits 0..7 : server-ip-addr[23:16]
        4.**> 'L3 offset' byte 21 bits 0..7 : server-ip-addr[31:24]

<private-ip-addr> DIP

        ipv4 - 4 bytes in network order L3 offset 18..21
        5.**> 'L3 offset' byte 14 bits 0..7 : public-ip-addr[7:0]
        6.**> 'L3 offset' byte 15 bits 0..7 : public-ip-addr[15:8]
        7.**> 'L3 offset' byte 16 bits 0..7 : public-ip-addr[23:16]
        8.**> 'L3 offset' byte 17 bits 0..7 : public-ip-addr[31:24]

<server-L4-port>

        9.**> 'L4 offset' byte 2 bits 0..7 : server-L4-port[15:8]
        10.**> 'L4 offset' byte 3 bits 0..7 : server-L4-port[7:0]

<private-L4-port>

        11.**> 'L4 offset' byte 0 bits 0..7 : public-L4-port[15:8]
        12.**> 'L4 offset' byte 1 bits 0..7 : public-L4-port[7:0]

<protocol> 'L4 Valid'

        12.**> 'metadata' byte 51 bit 1 = 1

total: 13 udbs
--]]
local pclUdbArr1 =
{   --byte#             offset type                                     offset
    -- L3 section
    { udbIndex = 0,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 14      }, -- bits 0..7  : server-ip-addr[7:0]
    { udbIndex = 1,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 15      }, -- bits 0..7  : server-ip-addr[15:8]
    { udbIndex = 2,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 16      }, -- bits 0..7  : server-ip-addr[23:16]
    { udbIndex = 3,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 17      }, -- bits 0..7  : server-ip-addr[31:24]
    { udbIndex = 4,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 18      }, -- bits 0..7  : public-ip-addr[7:0]
    { udbIndex = 5,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 19      }, -- bits 0..7  : public-ip-addr[15:8]
    { udbIndex = 6,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 20      }, -- bits 0..7  : public-ip-addr[23:16]
    { udbIndex = 7,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 21      }, -- bits 0..7  : public-ip-addr[31:24]
    -- L4 section
    { udbIndex = 8,    offsetType = "CPSS_DXCH_PCL_OFFSET_L4_E",          offset = 0       }, -- bits 0..7 : server-L4-port[15:8]
    { udbIndex = 9,    offsetType = "CPSS_DXCH_PCL_OFFSET_L4_E",          offset = 1       }, -- bits 0..7 : server-L4-port[7:0]
    { udbIndex = 10,   offsetType = "CPSS_DXCH_PCL_OFFSET_L4_E",          offset = 2       }, -- bits 0..7 : public-L4-port[15:8]
    { udbIndex = 11,   offsetType = "CPSS_DXCH_PCL_OFFSET_L4_E",          offset = 3       }, -- bits 0..7 : public-L4-port[7:0]
    -- metadata section
    { udbIndex = 35,   offsetType = "CPSS_DXCH_PCL_OFFSET_METADATA_E",    offset = 51      }  -- bit 1 = 1
}

local pclUdbArr1_noL4 =
{   --byte#             offset type                                     offset
    -- L3 section
    { udbIndex = 0,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 14      }, -- bits 0..7  : server-ip-addr[7:0]
    { udbIndex = 1,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 15      }, -- bits 0..7  : server-ip-addr[15:8]
    { udbIndex = 2,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 16      }, -- bits 0..7  : server-ip-addr[23:16]
    { udbIndex = 3,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 17      }, -- bits 0..7  : server-ip-addr[31:24]
    { udbIndex = 4,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 18      }, -- bits 0..7  : public-ip-addr[7:0]
    { udbIndex = 5,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 19      }, -- bits 0..7  : public-ip-addr[15:8]
    { udbIndex = 6,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 20      }, -- bits 0..7  : public-ip-addr[23:16]
    { udbIndex = 7,    offsetType = "CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E",  offset = 21      }, -- bits 0..7  : public-ip-addr[31:24]
    -- metadata section
    { udbIndex = 35,   offsetType = "CPSS_DXCH_PCL_OFFSET_METADATA_E",    offset = 51      }  -- bit 1 = 1
}

-- pclUdbArray
local pclPatternPtr1 = {ruleIngrStdUdb={
    commonIngrUdb={
        pclId = reserved_pclId,
        macToMe = 1,
        isIp = 1},
    isIpv4 = 1,
    udb0_15  = {
        [0] = FILLED_IN_RUN_TIME,                    -- bits 0..7   : server-ip-addr[7:0]
        [1] = FILLED_IN_RUN_TIME,                    -- bits 8..15  : server-ip-addr[8:15]
        [2] = FILLED_IN_RUN_TIME,                    -- bits 16..23 : server-ip-addr[16:23]
        [3] = FILLED_IN_RUN_TIME,                    -- bits 24..31 : server-ip-addr[24:31]
        [4] = FILLED_IN_RUN_TIME,                    -- bits 0..7   : public-ip-addr[7:0]
        [5] = FILLED_IN_RUN_TIME,                    -- bits 8..15  : public-ip-addr[8:15]
        [6] = FILLED_IN_RUN_TIME,                    -- bits 16..23 : public-ip-addr[16:23]
        [7] = FILLED_IN_RUN_TIME,                    -- bits 24..31 : public-ip-addr[24:31]
        [8] = FILLED_IN_RUN_TIME,                    -- bits 0..7   : server-L4-port[15:8]
        [9] = FILLED_IN_RUN_TIME,                    -- bits 0..7   : server-L4-port[7:0]
        [10] = FILLED_IN_RUN_TIME,                   -- bits 0..7   : public-L4-port[15:8]
        [11] = FILLED_IN_RUN_TIME,                   -- bits 0..7   : public-L4-port[7:0]
        [12] = 0,
        [13] = 0,
        [14] = 0,
        [15] = 0
    } }}

-- pclUdbArray
local pclMaskPtr1 = {ruleIngrStdUdb={
    commonIngrUdb={
        pclId = 0x3ff,
        macToMe = 1,
        isIp = 1},
    isIpv4 = 1,
    udb0_15  = {
        [0] = 0xff,                     -- bits 0..7   : server-ip-addr[7:0]
        [1] = 0xff,                     -- bits 8..15  : server-ip-addr[8:15]
        [2] = 0xff,                     -- bits 16..23 : server-ip-addr[16:23]
        [3] = 0xff,                     -- bits 24..31 : server-ip-addr[24:31]
        [4] = 0xff,                     -- bits 0..7   : public-ip-addr[7:0]
        [5] = 0xff,                     -- bits 8..15  : public-ip-addr[8:15]
        [6] = 0xff,                     -- bits 16..23 : public-ip-addr[16:23]
        [7] = 0xff,                     -- bits 24..31 : public-ip-addr[24:31]
        [8] = 0xff,                     -- bits 0..7   : server-L4-port[15:8]
        [9] = 0xff,                     -- bits 0..7   : server-L4-port[7:0]
        [10] = 0xff,                    -- bits 0..7   : public-L4-port[15:8]
        [11] = 0xff,                    -- bits 0..7   : public-L4-port[7:0]
        [12] = 0,
        [13] = 0,
        [14] = 0,
        [15] = 0
    } } }

-- Default PCL0 Rule configuration
-- define the UDBs that will be needed for the NAT44 default PCL0 Rule classification
--[[ we need next fields:
pclid = 0x135 --> see reserved_pclId
command = trap
--]]

-- udbArray
local patternDefaultIpclRulePtr = {ruleIngrStdUdb={
    commonIngrUdb={
        pclId = reserved_pclId,
        macToMe = 1,
        isIp = 1},
    isIpv4 = 1
    } }

-- udbArray
local maskDefaultIpclRulePtr = {ruleIngrStdUdb={
    commonIngrUdb={
        pclId = 0x3ff,
        macToMe = 1,
        isIp = 1},
    isIpv4 = 1
    } }

-- global number of NAT44 channels in the system
local global_num_nat44_channels = 0

--[[ define the table that will hold the 'NAT44 configuration done in the system'
     each line in the table hold info about different device
     each device hold next info :
                  1.nat_name - a unique name
                  2.local_interface
                  3.outer_interface
                  4.local_eport
                  5.outer_eport
                  6.local_mac_addr
                  7.outer_mac_addr
                  8.local_vid
                  9.outer_vid
                  10.private_ip
                  11.public_ip
                  12.server_ip
                  13.protocol - optional param
                  14.private_L4_port - optional param
                  15.public_L4_port - optional param
                  16.server_L4_port - optional param
                  17.defaultIpclIndex - calculated internally
                  -- private to public
                  18.ipclIndex - calculated internally
                  19.ipNextHopIndex - calculated internally
                  20.natIndex - calculated internally
                  21.lpmLeafIndex - calculated internally
                  -- public to private
                  22.ipclIndex1 - calculated internally
                  23.ipNextHopIndex1 - calculated internally
                  24.natIndex1 - calculated internally
                  25.lpmLeafIndex1 - calculated internally
                  -- private to private
                  26.ipclIndex_prv - calculated internally
--]]
local table_nat44_system_info = {}


local function nat44_command_func(command_data,devNum,params)


    -- Common variables declaration
    local apiName,result
    local outerHwDevNum,localHwDevNum
    local defaultIpclIndex
    local ipclIndex,ipNextHopIndex,natIndex,lpmLeafIndex -- private to public
    local ipclIndex1,ipNextHopIndex1,natIndex1,lpmLeafIndex1 -- public to private
    local ipclIndex_prv -- private to private
    local physicalInfo = {}
    local interfaceInfo = {}
    local pclActionPtr = {} -- private to public
    local pclActionPtr1 = {} -- public to private
    local pclActionPtr_prv = {} -- private to private
    local actionDefaultIpclRulePtr = {}
    local redirect = {}
    local egressInfo = {}
    local routeEntriesArray = {}
    local nat44EntryPtr = {}
    local leafPtr = {}
    local lookupCfg = {}
    local system_mac_address = getGlobal("myMac")

    local myIndexName

    if false == is_supported_feature(devNum, "NAT44_CMD_SUPPORT") then
        print("Command not supported for current device")
        return
    end

    pcl_manager_range_set(getPclRange(devNum)) -- getPclRange return two values

    -- print input params
    _debug("NAT44 command params \n "..to_string(params))

    -- check optional params
    if params.protocol ~= nil and (params.private_L4_port == nil or params.public_L4_port == nil or params.server_L4_port == nil) then
        print("NAT44 command error in optional params")
        return nil
    end

    -- change type for input params
    params.local_eport = tonumber(params.local_eport)
    params.outer_eport = tonumber(params.outer_eport)
    params.local_vid = tonumber(params.local_vid)
    params.outer_vid = tonumber(params.outer_vid)
    params.local_interface.devId = tonumber(params.local_interface.devId)
    params.local_interface.portNum = tonumber(params.local_interface.portNum)
    params.outer_interface.devId = tonumber(params.outer_interface.devId)
    params.outer_interface.portNum = tonumber(params.outer_interface.portNum)

    -- if optional params are unset
    if params.protocol == nil then
        params.protocol = 0
        params.private_L4_port = 0
        params.public_L4_port = 0
        params.server_L4_port = 0
    else
        -- change type for input params
        params.private_L4_port = tonumber(params.private_L4_port)
        params.public_L4_port = tonumber(params.public_L4_port)
        params.server_L4_port = tonumber(params.server_L4_port)
    end

    ------------- Private to private configuration --------------------

    -- configure the UDBs needed for the match
    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr_prv) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", udbEntry.offsetType },
            { "IN",     "GT_U8",                                "offset",     udbEntry.offset }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr_prv) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", udbEntry.offsetType },
            { "IN",     "GT_U8",                                "offset",     udbEntry.offset }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E"
    -- no match on L4 section
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr_prv) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", udbEntry.offsetType },
            { "IN",     "GT_U8",                                "offset",     udbEntry.offset }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    myIndexName = "<NAT44: Private to Private PCL rule > nat-name ".. params.nat_name
    ipclIndex_prv = allocPclIndexFromPool(3,myIndexName,false)

    -- set 'runtime' action params
    pclActionPtr_prv.pktCmd       = "CPSS_PACKET_CMD_FORWARD_E"

    -- set 'runtime' pattern params
    pclPatternPtr_prv.ruleIngrStdUdb.udb0_15 = {
        [0] = bit_and(params.private_ip[1],0xff),                    -- bits 0..7   : private-ip-addr[7:0]
        [1] = bit_and(params.private_ip[2],0xff),                    -- bits 8..15  : private-ip-addr[8:15]
        [2] = 0,
        [3] = 0,
        [4] = bit_and(params.private_ip[1],0xff),                    -- bits 0..7   : private-ip-addr[7:0]
        [5] = bit_and(params.private_ip[2],0xff),                    -- bits 8..15  : private-ip-addr[8:15]
        [6] = 0,
        [7] = 0,
        [8] = 0,
        [9] = 0,
        [10] = 0,
        [11] = 0
    }

    pclMaskPtr_prv.ruleIngrStdUdb.udb0_15 = {
        [0] = 0xff,                                                  -- bits 0..7   : private-ip-addr[7:0]
        [1] = 0xff,                                                  -- bits 8..15  : private-ip-addr[8:15]
        [2] = 0,
        [3] = 0,
        [4] = 0xff,                                                  -- bits 0..7   : private-ip-addr[7:0]
        [5] = 0xff,                                                  -- bits 8..15  : private-ip-addr[8:15]
        [6] = 0,
        [7] = 0,
        [8] = 0,
        [9] = 0,
        [10] = 0,
        [11] = 0
    }

    -- set the Policy Rule Mask, Pattern and Action
    apiName = "cpssDxChPclRuleSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                "devNum",           devNum },
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat",       pclRuleType},
        { "IN",     "GT_U32",                               "ruleIndex",        ipclIndex_prv },
        { "IN",     "CPSS_DXCH_PCL_RULE_OPTION_ENT",        "ruleOptionsBmp",   0},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb",        "maskPtr",          pclMaskPtr_prv},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb",        "patternPtr",       pclPatternPtr_prv},
        { "IN",     "CPSS_DXCH_PCL_ACTION_STC",             "actionPtr",        pclActionPtr_prv},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------- Private to public configuration ---------------------

    apiName = "cpssDxChCfgHwDevNumGet"
    result, localHwDevNum = device_to_hardware_format_convert(params.local_interface.devId)
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    physicalInfo.type = "CPSS_INTERFACE_PORT_E"
    physicalInfo.devPort = {}
    physicalInfo.devPort.devNum  = localHwDevNum
    physicalInfo.devPort.portNum = params.local_interface.portNum

    -- set physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum},
        { "IN",     "GT_PORT_NUM",             "portNum",         params.local_eport},
        { "IN",     "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)


    -- nat is in the same table as arp and ts
    myIndexName = "<NAT44: Private to Public> nat-name ".. params.nat_name
    natIndex = sharedResourceNamedNatMemoAlloc(myIndexName,1)
    if natIndex == nil then
        return command_data:addErrorAndPrint(
            "tunnel start entry allocation error");
    end


    nat44EntryPtr.nat44Entry = {}
    nat44EntryPtr.nat44Entry.macDa = params.outer_mac_addr
    nat44EntryPtr.nat44Entry.modifySip = GT_TRUE
    nat44EntryPtr.nat44Entry.newSip = params.public_ip
    if params.protocol ~= 0 then
        nat44EntryPtr.nat44Entry.modifyTcpUdpSrcPort = GT_TRUE
        nat44EntryPtr.nat44Entry.newTcpUdpSrcPort = params.public_L4_port
    end

    -- Set a NAT entry
    apiName = "cpssDxChIpNatEntrySet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                    "devNum",   devNum },
        { "IN",     "GT_U32",                                   "natIndex", natIndex },
        { "IN",     "CPSS_IP_NAT_TYPE_ENT",                     "natType",  "CPSS_IP_NAT_TYPE_NAT44_E" },
        { "IN",     "CPSS_DXCH_IP_NAT_ENTRY_UNT_nat44Entry",    "entryPtr", nat44EntryPtr}
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- per Egress port bit Enable Routed packets MAC SA Modification
    apiName = "cpssDxChIpRouterMacSaModifyEnable"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                "devNum",   devNum},
        { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  params.outer_interface.portNum},
        { "IN",     "GT_BOOL",              "enable",   true},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- Set the mode, per port, in which the device sets the packet's MAC SA least significant bytes
    apiName = "cpssDxChIpPortRouterMacSaLsbModeSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                "devNum",   devNum},
        { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  params.outer_interface.portNum},
        { "IN",     "CPSS_MAC_SA_LSB_MODE_ENT","saLsbMode", "CPSS_SA_LSB_FULL_48_BIT_GLOBAL"},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- Sets full 48-bit Router MAC SA in Global MAC SA table
    lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,params.outer_interface.portNum,system_mac_address)

    -- Set router mac sa index refered to global MAC SA table
    lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,params.outer_eport,params.outer_interface.portNum)

    myIndexName = "<NAT44: Private to Public> nat-name ".. params.nat_name
    ipNextHopIndex = allocIpNextHopIndexFromPool(myIndexName,false)

    apiName = "cpssDxChCfgHwDevNumGet"
    result, outerHwDevNum = device_to_hardware_format_convert(params.outer_interface.devId)
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    routeEntriesArray.type = "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E"
    routeEntriesArray.entry = {}
    routeEntriesArray.entry.regularEntry = {}
    routeEntriesArray.entry.regularEntry.cmd = "CPSS_PACKET_CMD_ROUTE_E"
    routeEntriesArray.entry.regularEntry.isTunnelStart = GT_TRUE
    routeEntriesArray.entry.regularEntry.isNat = GT_TRUE
    routeEntriesArray.entry.regularEntry.nextHopNatPointer = natIndex
    routeEntriesArray.entry.regularEntry.nextHopVlanId = params.outer_vid
    routeEntriesArray.entry.regularEntry.nextHopInterface = {}
    routeEntriesArray.entry.regularEntry.nextHopInterface.type = "CPSS_INTERFACE_PORT_E"
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort = {}
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.devNum = outerHwDevNum
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.portNum = params.outer_eport

    -- Writes an array of uc route entries to hw
    apiName = "cpssDxChIpUcRouteEntriesWrite"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                            "devNum",               devNum },
        { "IN",     "GT_U32",                           "baseRouteEntryIndex",  ipNextHopIndex},
        { "IN",     "CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC",  "routeEntriesArray",    routeEntriesArray },
        { "IN",     "GT_U32",                           "numOfRouteEntries",    1}
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    myIndexName = "<NAT44: Private to Public> nat-name ".. params.nat_name
    lpmLeafIndex = allocLpmLeafIndexFromPool(myIndexName,false)

    leafPtr.entryType = "CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E"
    leafPtr.index = ipNextHopIndex

    -- Write an LPM leaf entry, which is used for policy based routing, to the HW
    apiName = "cpssDxChLpmLeafEntryWrite"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                            "devNum",               devNum },
        { "IN",     "GT_U32",                           "leafIndex",            lpmLeafIndex},
        { "IN",     "CPSS_DXCH_LPM_LEAF_ENTRY_STC",     "leafPtr",              leafPtr }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Enable bypassing the router triggering requirements for policy based routing packets
    apiName = "cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_BOOL",       "enable",        true },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --  Enable unicast IPv4 routing on a port
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        params.local_interface.portNum       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  true }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --  Enable unicast IPv4 routing on a ePort
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        params.local_eport       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  true }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Enable IPv4 Unicast Routing on Vlan
    apiName = "cpssDxChBrgVlanIpUcRouteEnable"
    result = myGenWrapper(apiName, {
        { "IN",  "GT_U8",                       "devNum",       devNum },
        { "IN",  "GT_U16",                      "vlanId",       params.local_vid},
        { "IN",  "CPSS_IP_PROTOCOL_STACK_ENT",  "protocol",     "CPSS_IP_PROTOCOL_IPV4_E"},
        { "IN",  "GT_BOOL",                     "enable",       true },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- enables ingress policy per port
    apiName = "cpssDxChPclPortIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_PORT_NUM",   "portNum",       params.local_interface.portNum},
        { "IN",     "GT_BOOL",       "enable",        true },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- configure the UDBs needed for the match
    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", udbEntry.offsetType },
            { "IN",     "GT_U8",                                "offset",     udbEntry.offset }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", udbEntry.offsetType },
            { "IN",     "GT_U8",                                "offset",     udbEntry.offset }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E"
    -- no match on L4 section
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr_noL4) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", udbEntry.offsetType },
            { "IN",     "GT_U8",                                "offset",     udbEntry.offset }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    myIndexName = "<NAT44: Private to Public PCL rule > nat-name ".. params.nat_name
    ipclIndex = allocPclIndexFromPool(3,myIndexName,false)

    -- set 'runtime' action params
    redirect.redirectCmd = "CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E"
    redirect.data = {}
    redirect.data.routerLttIndex = lpmLeafIndex

    pclActionPtr.pktCmd       = "CPSS_PACKET_CMD_FORWARD_E"
    pclActionPtr.bypassBridge = GT_TRUE
    pclActionPtr.redirect     = redirect

    -- set 'runtime' pattern params
    if params.protocol == 0 then
        pclPatternPtr.ruleIngrStdUdb.udb0_15 = {
            [0] = bit_and(params.private_ip[1],0xff),                    -- bits 0..7   : private-ip-addr[7:0]
            [1] = bit_and(params.private_ip[2],0xff),                    -- bits 8..15  : private-ip-addr[8:15]
            [2] = bit_and(params.private_ip[3],0xff),                    -- bits 16..23 : private-ip-addr[16:23]
            [3] = bit_and(params.private_ip[4],0xff),                    -- bits 24..31 : private-ip-addr[24:31]
            [4] = bit_and(params.server_ip[1],0xff),                     -- bits 0..7   : server-ip-addr[7:0]
            [5] = bit_and(params.server_ip[2],0xff),                     -- bits 8..15  : server-ip-addr[8:15]
            [6] = bit_and(params.server_ip[3],0xff),                     -- bits 16..23 : server-ip-addr[16:23]
            [7] = bit_and(params.server_ip[4],0xff),                     -- bits 24..31 : server-ip-addr[24:31]
            [8] = 0,                                                     -- bits 0..7   : private-L4-port[15:8]
            [9] = 0,                                                     -- bits 0..7   : private-L4-port[7:0]
            [10] = 0,                                                    -- bits 0..7   : server-L4-port[15:8]
            [11] = 0                                                     -- bits 0..7   : server-L4-port[7:0]
        }

        pclMaskPtr.ruleIngrStdUdb.udb0_15 = {
            [0] = 0xff,                                                  -- bits 0..7   : private-ip-addr[7:0]
            [1] = 0xff,                                                  -- bits 8..15  : private-ip-addr[8:15]
            [2] = 0xff,                                                  -- bits 16..23 : private-ip-addr[16:23]
            [3] = 0xff,                                                  -- bits 24..31 : private-ip-addr[24:31]
            [4] = 0xff,                                                  -- bits 0..7   : server-ip-addr[7:0]
            [5] = 0xff,                                                  -- bits 8..15  : server-ip-addr[8:15]
            [6] = 0xff,                                                  -- bits 16..23 : server-ip-addr[16:23]
            [7] = 0xff,                                                  -- bits 24..31 : server-ip-addr[24:31]
            [8] = 0,                                                     -- bits 0..7   : private-L4-port[15:8]
            [9] = 0,                                                     -- bits 0..7   : private-L4-port[7:0]
            [10] = 0,                                                    -- bits 0..7   : server-L4-port[15:8]
            [11] = 0                                                     -- bits 0..7   : server-L4-port[7:0]
        }
    else
        pclPatternPtr.ruleIngrStdUdb.udb0_15 = {
                [0] = bit_and(params.private_ip[1],0xff),                    -- bits 0..7   : private-ip-addr[7:0]
                [1] = bit_and(params.private_ip[2],0xff),                    -- bits 8..15  : private-ip-addr[8:15]
                [2] = bit_and(params.private_ip[3],0xff),                    -- bits 16..23 : private-ip-addr[16:23]
                [3] = bit_and(params.private_ip[4],0xff),                    -- bits 24..31 : private-ip-addr[24:31]
                [4] = bit_and(params.server_ip[1],0xff),                     -- bits 0..7   : server-ip-addr[7:0]
                [5] = bit_and(params.server_ip[2],0xff),                     -- bits 8..15  : server-ip-addr[8:15]
                [6] = bit_and(params.server_ip[3],0xff),                     -- bits 16..23 : server-ip-addr[16:23]
                [7] = bit_and(params.server_ip[4],0xff),                     -- bits 24..31 : server-ip-addr[24:31]
                [8] = bit_and(bit_shr(params.private_L4_port,8),0xff),       -- bits 0..7   : private-L4-port[15:8]
                [9] = bit_and(params.private_L4_port,0xff),                  -- bits 0..7   : private-L4-port[7:0]
                [10] = bit_and(bit_shr(params.server_L4_port,8),0xff),       -- bits 0..7   : server-L4-port[15:8]
                [11] = bit_and(params.server_L4_port,0xff)                   -- bits 0..7   : server-L4-port[7:0]
        }

        pclMaskPtr.ruleIngrStdUdb.udb0_15 = {
            [0] = 0xff,                                                  -- bits 0..7   : private-ip-addr[7:0]
            [1] = 0xff,                                                  -- bits 8..15  : private-ip-addr[8:15]
            [2] = 0xff,                                                  -- bits 16..23 : private-ip-addr[16:23]
            [3] = 0xff,                                                  -- bits 24..31 : private-ip-addr[24:31]
            [4] = 0xff,                                                  -- bits 0..7   : server-ip-addr[7:0]
            [5] = 0xff,                                                  -- bits 8..15  : server-ip-addr[8:15]
            [6] = 0xff,                                                  -- bits 16..23 : server-ip-addr[16:23]
            [7] = 0xff,                                                  -- bits 24..31 : server-ip-addr[24:31]
            [8] = 0xff,                                                  -- bits 0..7   : private-L4-port[15:8]
            [9] = 0xff,                                                  -- bits 0..7   : private-L4-port[7:0]
            [10] = 0xff,                                                 -- bits 0..7   : server-L4-port[15:8]
            [11] = 0xff,                                                 -- bits 0..7   : server-L4-port[7:0]
        }
     end

    -- set the Policy Rule Mask, Pattern and Action
    apiName = "cpssDxChPclRuleSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                "devNum",           devNum },
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat",       pclRuleType},
        { "IN",     "GT_U32",                               "ruleIndex",        ipclIndex },
        { "IN",     "CPSS_DXCH_PCL_RULE_OPTION_ENT",        "ruleOptionsBmp",   0},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb",        "maskPtr",          pclMaskPtr},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb",        "patternPtr",       pclPatternPtr},
        { "IN",     "CPSS_DXCH_PCL_ACTION_STC",             "actionPtr",        pclActionPtr},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    myIndexName = "<NAT44: Private to Public default PCL rule > nat-name ".. params.nat_name
    defaultIpclIndex = allocPclIndexFromPool(3,myIndexName,true)

    actionDefaultIpclRulePtr.pktCmd       = "CPSS_PACKET_CMD_TRAP_TO_CPU_E"
    actionDefaultIpclRulePtr.mirror = {}
    actionDefaultIpclRulePtr.mirror.cpuCode = "CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E"

    -- set the Policy Rule Mask, Pattern and Action for default PCL action
    apiName = "cpssDxChPclRuleSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                "devNum",           devNum },
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat",       pclRuleType},
        { "IN",     "GT_U32",                               "ruleIndex",        defaultIpclIndex },
        { "IN",     "CPSS_DXCH_PCL_RULE_OPTION_ENT",        "ruleOptionsBmp",   0},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb",        "maskPtr",          maskDefaultIpclRulePtr},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb",        "patternPtr",       patternDefaultIpclRulePtr},
        { "IN",     "CPSS_DXCH_PCL_ACTION_STC",             "actionPtr",        actionDefaultIpclRulePtr},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    interfaceInfo.type = "CPSS_INTERFACE_PORT_E"
    interfaceInfo.devPort = {}
    interfaceInfo.devPort.devNum  = localHwDevNum
    interfaceInfo.devPort.portNum = params.local_interface.portNum

    lookupCfg.enableLookup = true
    lookupCfg.pclId = reserved_pclId
    lookupCfg.groupKeyTypes = groupKeyTypes

    -- set PCL Configuration table entry's lookup configuration for interface.
    apiName = "cpssDxChPclCfgTblSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",               devNum },
        { "IN",     "CPSS_INTERFACE_INFO_STC",      "interfaceInfoPtr",     interfaceInfo},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",            "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",            "CPSS_PCL_LOOKUP_0_E"},
        { "IN",     "CPSS_DXCH_PCL_LOOKUP_CFG_STC", "lookupCfgPtr",         lookupCfg},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Save Ingress Policy State
    apiName = "cpssDxChPclIngressPolicyEnableGet"
    result,value = myGenWrapper(apiName, {
        { "IN",     "GT_U8",        "devNum",     devNum},
        { "OUT",    "GT_BOOL",      "enable"},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)
    if value.enable ~= nil then
        savedIngressPolicyState = value.enable
    end

    -- Enable Ingress Policy
    apiName = "cpssDxChPclIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",        "devNum",     devNum},
        { "IN",     "GT_BOOL",      "enable",     true},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Configures port access mode to Ingress PCL configuration table per lookup.
    apiName = "cpssDxChPclPortLookupCfgTabAccessModeSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",       devNum},
        { "IN",     "GT_PORT_NUM",                  "portNum",      params.local_interface.portNum },
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",    "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",    "CPSS_PCL_LOOKUP_0_E"},
        { "IN",     "GT_U32",                       "subLookupNum", 0},
        { "IN",     "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT",  "mode", "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E"},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)


    ------------- Public to private configuration ---------------------

    physicalInfo.type = "CPSS_INTERFACE_PORT_E"
    physicalInfo.devPort = {}
    physicalInfo.devPort.devNum  = outerHwDevNum
    physicalInfo.devPort.portNum = params.outer_interface.portNum

    -- set physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum},
        { "IN",     "GT_PORT_NUM",             "portNum",         params.outer_eport},
        { "IN",     "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)


    -- nat is in the same table as arp and ts
    myIndexName = "<NAT44: Public to Private> nat-name ".. params.nat_name
    natIndex1 = sharedResourceNamedNatMemoAlloc(myIndexName,1)
    if natIndex1 == nil then
        return command_data:addErrorAndPrint(
            "tunnel start entry allocation error");
    end


    nat44EntryPtr.nat44Entry = {}
    nat44EntryPtr.nat44Entry.macDa = params.local_mac_addr
    nat44EntryPtr.nat44Entry.modifyDip = GT_TRUE
    nat44EntryPtr.nat44Entry.newDip = params.private_ip
    if params.protocol ~= 0 then
        nat44EntryPtr.nat44Entry.modifyTcpUdpDstPort = GT_TRUE
        nat44EntryPtr.nat44Entry.newTcpUdpDstPort = params.private_L4_port
    end

    -- Set a NAT entry
    apiName = "cpssDxChIpNatEntrySet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                    "devNum",   devNum },
        { "IN",     "GT_U32",                                   "natIndex", natIndex1 },
        { "IN",     "CPSS_IP_NAT_TYPE_ENT",                     "natType",  "CPSS_IP_NAT_TYPE_NAT44_E" },
        { "IN",     "CPSS_DXCH_IP_NAT_ENTRY_UNT_nat44Entry",    "entryPtr", nat44EntryPtr}
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- per Egress port bit Enable Routed packets MAC SA Modification
    apiName = "cpssDxChIpRouterMacSaModifyEnable"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                "devNum",   devNum},
        { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  params.local_interface.portNum},
        { "IN",     "GT_BOOL",              "enable",   true},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- Set the mode, per port, in which the device sets the packet's MAC SA least significant bytes
    apiName = "cpssDxChIpPortRouterMacSaLsbModeSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                "devNum",   devNum},
        { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  params.local_interface.portNum},
        { "IN",     "CPSS_MAC_SA_LSB_MODE_ENT","saLsbMode", "CPSS_SA_LSB_FULL_48_BIT_GLOBAL"},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- Sets full 48-bit Router MAC SA in Global MAC SA table
    lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,params.local_interface.portNum,system_mac_address)

    -- Set router mac sa index refered to global MAC SA table
    lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,params.local_eport,params.local_interface.portNum)

    myIndexName = "<NAT44: Public to Private> nat-name ".. params.nat_name
    ipNextHopIndex1 = allocIpNextHopIndexFromPool(myIndexName,false)

    routeEntriesArray.type = "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E"
    routeEntriesArray.entry = {}
    routeEntriesArray.entry.regularEntry = {}
    routeEntriesArray.entry.regularEntry.cmd = "CPSS_PACKET_CMD_ROUTE_E"
    routeEntriesArray.entry.regularEntry.isTunnelStart = GT_TRUE
    routeEntriesArray.entry.regularEntry.isNat = GT_TRUE
    routeEntriesArray.entry.regularEntry.nextHopNatPointer = natIndex1
    routeEntriesArray.entry.regularEntry.nextHopVlanId = params.local_vid
    routeEntriesArray.entry.regularEntry.nextHopInterface = {}
    routeEntriesArray.entry.regularEntry.nextHopInterface.type = "CPSS_INTERFACE_PORT_E"
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort = {}
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.devNum = localHwDevNum
    routeEntriesArray.entry.regularEntry.nextHopInterface.devPort.portNum = params.local_eport

    -- Writes an array of uc route entries to hw
    apiName = "cpssDxChIpUcRouteEntriesWrite"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                            "devNum",               devNum },
        { "IN",     "GT_U32",                           "baseRouteEntryIndex",  ipNextHopIndex1},
        { "IN",     "CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC",  "routeEntriesArray",    routeEntriesArray },
        { "IN",     "GT_U32",                           "numOfRouteEntries",    1}
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    myIndexName = "<NAT44: Public to Private> nat-name ".. params.nat_name
    lpmLeafIndex1 = allocLpmLeafIndexFromPool(myIndexName,false)

    leafPtr.entryType = "CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E"
    leafPtr.index = ipNextHopIndex1

    -- Write an LPM leaf entry, which is used for policy based routing, to the HW
    apiName = "cpssDxChLpmLeafEntryWrite"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                            "devNum",               devNum },
        { "IN",     "GT_U32",                           "leafIndex",            lpmLeafIndex1},
        { "IN",     "CPSS_DXCH_LPM_LEAF_ENTRY_STC",     "leafPtr",              leafPtr }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --  Enable unicast IPv4 routing on a port
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        params.outer_interface.portNum       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  true }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --  Enable unicast IPv4 routing on a ePort
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        params.outer_eport       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  true }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Enable IPv4 Unicast Routing on Vlan
    apiName = "cpssDxChBrgVlanIpUcRouteEnable"
    result = myGenWrapper(apiName, {
        { "IN",  "GT_U8",                       "devNum",       devNum },
        { "IN",  "GT_U16",                      "vlanId",       params.outer_vid},
        { "IN",  "CPSS_IP_PROTOCOL_STACK_ENT",  "protocol",     "CPSS_IP_PROTOCOL_IPV4_E"},
        { "IN",  "GT_BOOL",                     "enable",       true },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- enables ingress policy per port
    apiName = "cpssDxChPclPortIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_PORT_NUM",   "portNum",       params.outer_interface.portNum},
        { "IN",     "GT_BOOL",       "enable",        true },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- enables ingress policy per ePort
    apiName = "cpssDxChPclPortIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_PORT_NUM",   "portNum",       params.outer_eport},
        { "IN",     "GT_BOOL",       "enable",        true },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- configure the UDBs needed for the match
    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr1) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", udbEntry.offsetType },
            { "IN",     "GT_U8",                                "offset",     udbEntry.offset }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr1) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", udbEntry.offsetType },
            { "IN",     "GT_U8",                                "offset",     udbEntry.offset }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E"
    -- no match on L4 section
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr1_noL4) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", udbEntry.offsetType },
            { "IN",     "GT_U8",                                "offset",     udbEntry.offset }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    myIndexName = "<NAT44: Public to Private> nat-name ".. params.nat_name
    ipclIndex1 = allocPclIndexFromPool(3,myIndexName,false)

    -- set 'runtime' action params
    redirect.redirectCmd = "CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E"
    redirect.data = {}
    redirect.data.routerLttIndex = lpmLeafIndex1

    pclActionPtr1.pktCmd       = "CPSS_PACKET_CMD_FORWARD_E"
    pclActionPtr1.bypassBridge = GT_TRUE
    pclActionPtr1.redirect     = redirect

    -- set 'runtime' pattern params
    if params.protocol == 0 then
        pclPatternPtr1.ruleIngrStdUdb.udb0_15 = {
                [0] = bit_and(params.server_ip[1],0xff),                    -- bits 0..7   : server-ip-addr[7:0]
                [1] = bit_and(params.server_ip[2],0xff),                    -- bits 8..15  : server-ip-addr[8:15]
                [2] = bit_and(params.server_ip[3],0xff),                    -- bits 16..23 : server-ip-addr[16:23]
                [3] = bit_and(params.server_ip[4],0xff),                    -- bits 24..31 : server-ip-addr[24:31]
                [4] = bit_and(params.public_ip[1],0xff),                    -- bits 0..7   : public-ip-addr[7:0]
                [5] = bit_and(params.public_ip[2],0xff),                    -- bits 8..15  : public-ip-addr[8:15]
                [6] = bit_and(params.public_ip[3],0xff),                    -- bits 16..23 : public-ip-addr[16:23]
                [7] = bit_and(params.public_ip[4],0xff),                    -- bits 24..31 : public-ip-addr[24:31]
                [8] = 0,                                                    -- bits 0..7   : server-L4-port[15:8]
                [9] = 0,                                                    -- bits 0..7   : server-L4-port[7:0]
                [10] = 0,                                                   -- bits 0..7   : public-L4-port[15:8]
                [11] = 0,                                                   -- bits 0..7   : public-L4-port[7:0]
        }

        pclMaskPtr1.ruleIngrStdUdb.udb0_15 = {
                [0] = 0xff,                                                 -- bits 0..7   : server-ip-addr[7:0]
                [1] = 0xff,                                                 -- bits 8..15  : server-ip-addr[8:15]
                [2] = 0xff,                                                 -- bits 16..23 : server-ip-addr[16:23]
                [3] = 0xff,                                                 -- bits 24..31 : server-ip-addr[24:31]
                [4] = 0xff,                                                 -- bits 0..7   : public-ip-addr[7:0]
                [5] = 0xff,                                                 -- bits 8..15  : public-ip-addr[8:15]
                [6] = 0xff,                                                 -- bits 16..23 : public-ip-addr[16:23]
                [7] = 0xff,                                                 -- bits 24..31 : public-ip-addr[24:31]
                [8] = 0,                                                    -- bits 0..7   : server-L4-port[15:8]
                [9] = 0,                                                    -- bits 0..7   : server-L4-port[7:0]
                [10] = 0,                                                   -- bits 0..7   : public-L4-port[15:8]
                [11] = 0,                                                   -- bits 0..7   : public-L4-port[7:0]
        }
    else
        pclPatternPtr1.ruleIngrStdUdb.udb0_15 = {
            [0] = bit_and(params.server_ip[1],0xff),                    -- bits 0..7   : server-ip-addr[7:0]
            [1] = bit_and(params.server_ip[2],0xff),                    -- bits 8..15  : server-ip-addr[8:15]
            [2] = bit_and(params.server_ip[3],0xff),                    -- bits 16..23 : server-ip-addr[16:23]
            [3] = bit_and(params.server_ip[4],0xff),                    -- bits 24..31 : server-ip-addr[24:31]
            [4] = bit_and(params.public_ip[1],0xff),                    -- bits 0..7   : public-ip-addr[7:0]
            [5] = bit_and(params.public_ip[2],0xff),                    -- bits 8..15  : public-ip-addr[8:15]
            [6] = bit_and(params.public_ip[3],0xff),                    -- bits 16..23 : public-ip-addr[16:23]
            [7] = bit_and(params.public_ip[4],0xff),                    -- bits 24..31 : public-ip-addr[24:31]
            [8] = bit_and(bit_shr(params.server_L4_port,8),0xff),       -- bits 0..7   : server-L4-port[15:8]
            [9] = bit_and(params.server_L4_port,0xff),                  -- bits 0..7   : server-L4-port[7:0]
            [10] = bit_and(bit_shr(params.public_L4_port,8),0xff),      -- bits 0..7   : public-L4-port[15:8]
            [11] = bit_and(params.public_L4_port,0xff),                 -- bits 0..7   : public-L4-port[7:0]
        }
        pclMaskPtr1.ruleIngrStdUdb.udb0_15 = {
            [0] = 0xff,                                                 -- bits 0..7   : server-ip-addr[7:0]
            [1] = 0xff,                                                 -- bits 8..15  : server-ip-addr[8:15]
            [2] = 0xff,                                                 -- bits 16..23 : server-ip-addr[16:23]
            [3] = 0xff,                                                 -- bits 24..31 : server-ip-addr[24:31]
            [4] = 0xff,                                                 -- bits 0..7   : public-ip-addr[7:0]
            [5] = 0xff,                                                 -- bits 8..15  : public-ip-addr[8:15]
            [6] = 0xff,                                                 -- bits 16..23 : public-ip-addr[16:23]
            [7] = 0xff,                                                 -- bits 24..31 : public-ip-addr[24:31]
            [8] = 0xff,                                                 -- bits 0..7   : server-L4-port[15:8]
            [9] = 0xff,                                                 -- bits 0..7   : server-L4-port[7:0]
            [10] = 0xff,                                                -- bits 0..7   : public-L4-port[15:8]
            [11] = 0xff,                                                -- bits 0..7   : public-L4-port[7:0]
        }
     end

    -- set the Policy Rule Mask, Pattern and Action
    apiName = "cpssDxChPclRuleSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                "devNum",           devNum },
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat",       pclRuleType},
        { "IN",     "GT_U32",                               "ruleIndex",        ipclIndex1 },
        { "IN",     "CPSS_DXCH_PCL_RULE_OPTION_ENT",        "ruleOptionsBmp",   0},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb",        "maskPtr",          pclMaskPtr1},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb",        "patternPtr",       pclPatternPtr1},
        { "IN",     "CPSS_DXCH_PCL_ACTION_STC",             "actionPtr",        pclActionPtr1},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    interfaceInfo.type = "CPSS_INTERFACE_PORT_E"
    interfaceInfo.devPort = {}
    interfaceInfo.devPort.devNum  = outerHwDevNum
    interfaceInfo.devPort.portNum = params.outer_interface.portNum

    lookupCfg.enableLookup = true
    lookupCfg.pclId = reserved_pclId
    lookupCfg.groupKeyTypes = groupKeyTypes

    -- set PCL Configuration table entry's lookup configuration for interface.
    apiName = "cpssDxChPclCfgTblSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",               devNum },
        { "IN",     "CPSS_INTERFACE_INFO_STC",      "interfaceInfoPtr",     interfaceInfo},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",            "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",            "CPSS_PCL_LOOKUP_0_E"},
        { "IN",     "CPSS_DXCH_PCL_LOOKUP_CFG_STC", "lookupCfgPtr",         lookupCfg},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Configures port access mode to Ingress PCL configuration table per lookup.
    apiName = "cpssDxChPclPortLookupCfgTabAccessModeSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",       devNum},
        { "IN",     "GT_PORT_NUM",                  "portNum",      params.outer_interface.portNum},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",    "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",    "CPSS_PCL_LOOKUP_0_E"},
        { "IN",     "GT_U32",                       "subLookupNum", 0},
        { "IN",     "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT",  "mode", "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E"},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    interfaceInfo.type = "CPSS_INTERFACE_INDEX_E"
    interfaceInfo.index = params.outer_eport

    lookupCfg.enableLookup = true
    lookupCfg.pclId = reserved_pclId
    lookupCfg.groupKeyTypes = groupKeyTypes

    -- set PCL Configuration table entry's lookup configuration for interface.
    apiName = "cpssDxChPclCfgTblSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",               devNum },
        { "IN",     "CPSS_INTERFACE_INFO_STC",      "interfaceInfoPtr",     interfaceInfo},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",            "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",            "CPSS_PCL_LOOKUP_0_E"},
        { "IN",     "CPSS_DXCH_PCL_LOOKUP_CFG_STC", "lookupCfgPtr",         lookupCfg},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- enable forcing of the PCL ID configuration for all TT packets according to the VLAN assignment
    apiName = "cpssDxCh3PclTunnelTermForceVlanModeEnableSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                    "devNum",      devNum},
        { "IN",     "GT_BOOL",                  "enable",      false},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------- DB configuration ---------------------

    -- operation succed save the info into DB
    local entryInfo = {}

    entryInfo.nat_name = params.nat_name
    entryInfo.local_interface = {}
    entryInfo.local_interface.devId = params.local_interface.devId
    entryInfo.local_interface.portNum = params.local_interface.portNum
    entryInfo.outer_interface = {}
    entryInfo.outer_interface.devId = params.outer_interface.devId
    entryInfo.outer_interface.portNum = params.outer_interface.portNum
    entryInfo.local_eport = params.local_eport
    entryInfo.outer_eport = params.outer_eport
    entryInfo.local_mac_addr = params.local_mac_addr
    entryInfo.outer_mac_addr = params.outer_mac_addr
    entryInfo.local_vid = params.local_vid
    entryInfo.outer_vid = params.outer_vid
    entryInfo.private_ip  = params.private_ip
    entryInfo.public_ip  = params.public_ip
    entryInfo.server_ip  = params.server_ip
    entryInfo.protocol  = params.protocol
    entryInfo.private_L4_port  = params.private_L4_port
    entryInfo.public_L4_port  = params.public_L4_port
    entryInfo.server_L4_port  = params.server_L4_port
    entryInfo.defaultIpclIndex  = defaultIpclIndex
    entryInfo.ipclIndex  = ipclIndex
    entryInfo.ipNextHopIndex  = ipNextHopIndex
    entryInfo.natIndex  = natIndex
    entryInfo.lpmLeafIndex  = lpmLeafIndex
    entryInfo.ipclIndex1  = ipclIndex1
    entryInfo.ipNextHopIndex1  = ipNextHopIndex1
    entryInfo.natIndex1  = natIndex1
    entryInfo.lpmLeafIndex1  = lpmLeafIndex1
    entryInfo.ipclIndex_prv  = ipclIndex_prv

    -- use the 'nat name' as a key to the table
    table_nat44_system_info[params.nat_name] = entryInfo

    -- another NAT44 channel was added
    global_num_nat44_channels = global_num_nat44_channels + 1

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- ************************************************************************
--
--  nat44_func
--        @description  Command to add an IPv4 NAT session
--
--        @param params             - params["nat_name"]: name of the NAT session, a unique name
--                                  - params["local_interface"]: ethernet port number in the private network
--                                  - params["outer_interface"]: ethernet port number in the public network
--                                  - params["local_eport"]: eport number in the private network
--                                  - params["outer_eport"]: eport number in the public network
--                                  - params["local_mac_addr"]: MAC Address of the host/next-hop router in the private network
--                                  - params["outer_mac_addr"]: MAC Address of the next hop router in the public network
--                                  - params["local_vid"]: VLAN in the private network
--                                  - params["outer_vid"]: VLAN in the public network
--                                  - params["private_ip"]: IP address of the host
--                                  - params["public_ip"]: public IP address of the NAT router
--                                  - params["server_ip"]: IP address of the server
--                                  - params["protocol"]: TCP or UDP, optional parameter
--                                  - params["private_L4_port"]: TCP/UDP port of the host, optional parameter
--                                  - params["public_L4_port"]: TCP/UDP port allocated by the NAT router, optional parameter
--                                  - params["server_L4_port"]: TCP/UDP port of the server, optional parameter
--
--        @return       true on success, otherwise false and error message
--

local function nat44_func(params)

    local command_data = Command_Data()

    command_data:initInterfaceDeviceRange()
    -- Common variables initialization.
    command_data:initAllAvailableDevicesRange()
    command_data:initAllAvaiblesPortIterator()

    -- now iterate
    command_data:iterateOverDevices(nat44_command_func,params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

local function no_nat44_name_function(command_data,devNum,params)

    -- Common variables declaration
    local apiName,result
    local localHwDevNum,outerHwDevNum
    local entryInfo = {}
    local interfaceInfo = {}
    local egressInfo = {}
    local zero = 0
    local myIndexName

    if false == is_supported_feature(devNum, "NAT44_CMD_SUPPORT") then
        print("Command not supported for current device")
        return
    end

    -- print input params
    _debug("no NAT44 name command params \n "..to_string(params))

    -- read params from DB
    entryInfo = table_nat44_system_info[params.nat_name]
    if entryInfo == nil then
        print("nat name "..params.nat_name.." was not defined in NAT44_db")
        return nil
    end

    ------------- Disable private to private configuration ---------------------

    -- configure the UDBs needed for the match back to default
    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr_prv) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", "CPSS_DXCH_PCL_OFFSET_INVALID_E" },
            { "IN",     "GT_U8",                                "offset",     zero }
        })
    end

    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr_prv) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", "CPSS_DXCH_PCL_OFFSET_INVALID_E" },
            { "IN",     "GT_U8",                                "offset",     zero }
        })
    end

    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr_prv) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", "CPSS_DXCH_PCL_OFFSET_INVALID_E" },
            { "IN",     "GT_U8",                                "offset",     zero }
        })
    end

    -- unset the Policy Rule Mask, Pattern and Action
    apiName = "cpssDxChPclRuleInvalidate"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                "devNum",           devNum },
        { "IN",     "CPSS_PCL_RULE_SIZE_ENT",               "ruleSize",         pclKeySize },
        { "IN",     "GT_U32",                               "ruleIndex",        entryInfo.ipclIndex_prv }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    freePclIndexToPool(entryInfo.ipclIndex_prv)

    ------------- Disable private to public configuration ---------------------

    -- unset physical info for the given ePort
    default_cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,entryInfo.local_eport)


    -- unset a NAT44 entry
    default_cpssDxChIpNatEntrySet(devNum,entryInfo.natIndex)
    myIndexName = "<NAT44: Private to Public> nat-name ".. params.nat_name
    sharedResourceNamedNatMemoFree(myIndexName);


    -- restore configuration for last name only
    local first_name = next(table_nat44_system_info); -- first
    local second_name = nil;
    if first_name then
        second_name = next(table_nat44_system_info, first_name);
    end
    if not second_name then
        -- enable forcing of the PCL ID configuration for all TT packets according to the VLAN assignment
        apiName = "cpssDxCh3PclTunnelTermForceVlanModeEnableSet"
            result = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                    "devNum",      devNum},
                { "IN",     "GT_BOOL",                  "enable",      true},
            })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)
    end

    -- unset the mode, per port
    apiName = "cpssDxChIpPortRouterMacSaLsbModeSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                "devNum",   devNum},
        { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  entryInfo.outer_interface.portNum},
        { "IN",     "CPSS_MAC_SA_LSB_MODE_ENT","saLsbMode", "CPSS_SA_LSB_PER_PKT_VID_E"},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- unSets full 48-bit Router MAC SA in Global MAC SA table
    lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,entryInfo.outer_interface.portNum,zero)

    -- unSet router mac sa index refered to global MAC SA table
    lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,entryInfo.outer_interface.portNum,zero)

    -- unset a IP NEXT HOP entry
    default_cpssDxChIpUcRouteEntriesWrite(devNum,entryInfo.ipNextHopIndex)
    freeIpNextHopIndexToPool(entryInfo.ipNextHopIndex)

    -- unset a LPM leaf entry
    default_cpssDxChLpmLeafEntryWrite(devNum,entryInfo.lpmLeafIndex)
    freeLpmLeafIndexToPool(entryInfo.lpmLeafIndex)

    -- Disable bypassing the router triggering requirements for policy based routing packets
    apiName = "cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_BOOL",       "enable",        false },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --  Disable unicast IPv4 routing on a port
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        entryInfo.local_interface.portNum    },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  false }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --  Disable unicast IPv4 routing on a ePort
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        entryInfo.local_eport       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  false }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Disable IPv4 Unicast Routing on Vlan
    apiName = "cpssDxChBrgVlanIpUcRouteEnable"
    result = myGenWrapper(apiName, {
        { "IN",  "GT_U8",                       "devNum",       devNum },
        { "IN",  "GT_U16",                      "vlanId",       entryInfo.local_vid},
        { "IN",  "CPSS_IP_PROTOCOL_STACK_ENT",  "protocol",     "CPSS_IP_PROTOCOL_IPV4_E"},
        { "IN",  "GT_BOOL",                     "enable",       false },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Disable ingress policy per eport
    apiName = "cpssDxChPclPortIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_PORT_NUM",   "portNum",       entryInfo.local_interface.portNum},
        { "IN",     "GT_BOOL",       "enable",        false },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- configure the UDBs needed for the match back to default
    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", "CPSS_DXCH_PCL_OFFSET_INVALID_E" },
            { "IN",     "GT_U8",                                "offset",     zero }
        })
    end

    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", "CPSS_DXCH_PCL_OFFSET_INVALID_E" },
            { "IN",     "GT_U8",                                "offset",     zero }
        })
    end

    pclPacketType = "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E"
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", "CPSS_DXCH_PCL_OFFSET_INVALID_E" },
            { "IN",     "GT_U8",                                "offset",     zero }
        })
    end

    -- unset the Policy Rule Mask, Pattern and Action
    apiName = "cpssDxChPclRuleInvalidate"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                "devNum",           devNum },
        { "IN",     "CPSS_PCL_RULE_SIZE_ENT",               "ruleSize",         pclKeySize },
        { "IN",     "GT_U32",                               "ruleIndex",        entryInfo.ipclIndex }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    freePclIndexToPool(entryInfo.ipclIndex)

    -- unset the Policy Rule Mask, Pattern and Action
    apiName = "cpssDxChPclRuleInvalidate"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                "devNum",           devNum },
        { "IN",     "CPSS_PCL_RULE_SIZE_ENT",               "ruleSize",         pclKeySize },
        { "IN",     "GT_U32",                               "ruleIndex",        entryInfo.defaultIpclIndex }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    freePclIndexToPool(entryInfo.defaultIpclIndex)

    apiName = "cpssDxChCfgHwDevNumGet"
    result, localHwDevNum = device_to_hardware_format_convert(entryInfo.local_interface.devId)
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    interfaceInfo.type = "CPSS_INTERFACE_PORT_E"
    interfaceInfo.devPort = {}
    interfaceInfo.devPort.devNum  = localHwDevNum
    interfaceInfo.devPort.portNum = entryInfo.local_interface.portNum

    -- unset PCL Configuration table entry's lookup configuration for interface.
    default_cpssDxChPclCfgTblSet(devNum,interfaceInfo,"CPSS_PCL_DIRECTION_INGRESS_E","CPSS_PCL_LOOKUP_0_E")

    -- Restore Ingress Policy State
    apiName = "cpssDxChPclIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",        "devNum",     devNum},
        { "IN",     "GT_BOOL",      "enable",     savedIngressPolicyState},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- unconfigure ePort access mode to Ingress PCL configuration table per lookup.
    apiName = "cpssDxChPclPortLookupCfgTabAccessModeSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",       devNum},
        { "IN",     "GT_PORT_NUM",                  "portNum",      entryInfo.local_interface.portNum},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",    "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",    "CPSS_PCL_LOOKUP_0_E"},
        { "IN",     "GT_U32",                       "subLookupNum", 0},
        { "IN",     "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT",  "mode", "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E"},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------- Public to private configuration ---------------------

    -- unset physical info for the given ePort
    default_cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,entryInfo.outer_eport)


    -- unset a NAT44 entry
    default_cpssDxChIpNatEntrySet(devNum,entryInfo.natIndex1)
    myIndexName = "<NAT44: Public to Private> nat-name ".. params.nat_name
    sharedResourceNamedNatMemoFree(myIndexName);


    -- unset the mode, per port
    apiName = "cpssDxChIpPortRouterMacSaLsbModeSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                "devNum",   devNum},
        { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  entryInfo.local_interface.portNum},
        { "IN",     "CPSS_MAC_SA_LSB_MODE_ENT","saLsbMode", "CPSS_SA_LSB_PER_PKT_VID_E"},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- unSets full 48-bit Router MAC SA in Global MAC SA table
    lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,entryInfo.local_interface.portNum,zero)

    -- unSet router mac sa index refered to global MAC SA table
    lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,entryInfo.local_interface.portNum,zero)

    -- unset a IP NEXT HOP entry
    default_cpssDxChIpUcRouteEntriesWrite(devNum,entryInfo.ipNextHopIndex1)
    freeIpNextHopIndexToPool(entryInfo.ipNextHopIndex1)

    -- unset a LPM leaf entry
    default_cpssDxChLpmLeafEntryWrite(devNum,entryInfo.lpmLeafIndex1)
    freeLpmLeafIndexToPool(entryInfo.lpmLeafIndex1)

    -- Disable unicast IPv4 routing on a port
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        entryInfo.outer_interface.portNum       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  false }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --  Disable unicast IPv4 routing on a ePort
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        entryInfo.outer_eport       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  false }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Disable IPv4 Unicast Routing on Vlan
    apiName = "cpssDxChBrgVlanIpUcRouteEnable"
    result = myGenWrapper(apiName, {
        { "IN",  "GT_U8",                       "devNum",       devNum },
        { "IN",  "GT_U16",                      "vlanId",       entryInfo.outer_vid},
        { "IN",  "CPSS_IP_PROTOCOL_STACK_ENT",  "protocol",     "CPSS_IP_PROTOCOL_IPV4_E"},
        { "IN",  "GT_BOOL",                     "enable",       false },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Disable ingress policy per port
    apiName = "cpssDxChPclPortIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_PORT_NUM",   "portNum",       entryInfo.outer_interface.portNum},
        { "IN",     "GT_BOOL",       "enable",        false },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Disable ingress policy per eport
    apiName = "cpssDxChPclPortIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_PORT_NUM",   "portNum",       entryInfo.outer_eport},
        { "IN",     "GT_BOOL",       "enable",        false },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- configure the UDBs needed for the match back to default
    apiName = "cpssDxChPclUserDefinedByteSet"
    for dummy,udbEntry in pairs(pclUdbArr) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                                "devNum",     devNum },
            { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
            { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "GT_U32",                               "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", "CPSS_DXCH_PCL_OFFSET_INVALID_E" },
            { "IN",     "GT_U8",                                "offset",     zero }
        })
    end

    -- unset the Policy Rule Mask, Pattern and Action
    apiName = "cpssDxChPclRuleInvalidate"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                "devNum",           devNum },
        { "IN",     "CPSS_PCL_RULE_SIZE_ENT",               "ruleSize",         pclKeySize },
        { "IN",     "GT_U32",                               "ruleIndex",        entryInfo.ipclIndex1 }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    freePclIndexToPool(entryInfo.ipclIndex1)

    apiName = "cpssDxChCfgHwDevNumGet"
    result, outerHwDevNum = device_to_hardware_format_convert(entryInfo.outer_interface.devId)
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    interfaceInfo.type = "CPSS_INTERFACE_PORT_E"
    interfaceInfo.devPort = {}
    interfaceInfo.devPort.devNum  = outerHwDevNum
    interfaceInfo.devPort.portNum = entryInfo.outer_interface.portNum

    -- unset PCL Configuration table entry's lookup configuration for interface.
    default_cpssDxChPclCfgTblSet(devNum,interfaceInfo,"CPSS_PCL_DIRECTION_INGRESS_E","CPSS_PCL_LOOKUP_0_E")

    interfaceInfo.type = "CPSS_INTERFACE_INDEX_E"
    interfaceInfo.index = entryInfo.outer_eport

    -- unset PCL Configuration table entry's lookup configuration for interface.
    default_cpssDxChPclCfgTblSet(devNum,interfaceInfo,"CPSS_PCL_DIRECTION_INGRESS_E","CPSS_PCL_LOOKUP_0_E")

    -- Restore Ingress Policy State
    apiName = "cpssDxChPclIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",        "devNum",     devNum},
        { "IN",     "GT_BOOL",      "enable",     savedIngressPolicyState},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- unconfigure port access mode to Ingress PCL configuration table per lookup.
    apiName = "cpssDxChPclPortLookupCfgTabAccessModeSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",       devNum},
        { "IN",     "GT_PORT_NUM",                  "portNum",      entryInfo.outer_interface.portNum},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",    "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",    "CPSS_PCL_LOOKUP_0_E"},
        { "IN",     "GT_U32",                       "subLookupNum", 0},
        { "IN",     "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT",  "mode", "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E"},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------- DB configuration ---------------------

    -- operation succed remove the info from DB
    table_nat44_system_info[params.nat_name] = nil

    -- NAT44 channel was deleted
    global_num_nat44_channels = global_num_nat44_channels - 1


    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- ************************************************************************
--
--  no_nat44_name
--        @description  Unset single NAT44 configurations for 'nat name', restore configuration
--
--        @param params   - params["nat_name"]: name of the NAT session, a unique name
--
--        @return       true on success, otherwise false and error message
--
local function no_nat44_name(params)

    local command_data = Command_Data()

    command_data:initInterfaceDeviceRange()

    -- Common variables initialization.
    command_data:initAllAvailableDevicesRange()
    command_data:initAllAvaiblesPortIterator()

    -- now iterate
    command_data:iterateOverDevices(no_nat44_name_function,params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

-- ************************************************************************
--  no_nat44_all_func
--        @description Unset all NAT44 configurations, restore configuration
--
--        @param params   - none
--
--        @return       true on success, otherwise false and error message
--
local function no_nat44_all_func()

    local command_data = Command_Data()
    -- Common variables declaration
    local current_index
    local current_entry = {}
    local params = {}
    local apiName,result

    if false == is_supported_feature(devNum, "NAT44_CMD_SUPPORT") then
        print("Command not supported for current device")
        return
    end

    -- print input params
    _debug("no NAT44 all command no params \n")

    -- get the first element in the table
    current_index , current_entry = next(table_nat44_system_info,nil)

    while current_entry do

        if current_entry.nat_name ~= nil then
            params.nat_name = current_entry.nat_name
            no_nat44_name(params)
        end

        current_index , current_entry = next(table_nat44_system_info,current_index)

    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- this function add to a given string spaces in order to get to a specific string length
-- inputs:
-- stringValue       - the string to add spaces to
-- totalSizeOfString - the size of the string after adding the spaces
--
-- returns:
-- the string with spaces
--

local function getStringWithSpaces(stringValue,totalSizeOfString)
    local originalStringSize
    local numberOfSpacesToAdd
    local stringWithSpaces
    local temp

    originalStringSize = string.len(stringValue)
    if originalStringSize > totalSizeOfString then
        return stringValue
    end

    numberOfSpacesToAdd = totalSizeOfString - originalStringSize
    temp = stringValue

    for i=0,numberOfSpacesToAdd  do
        stringWithSpaces = temp .." "
        temp = stringWithSpaces
    end
    return stringWithSpaces
end

-- ************************************************************************
---
--  nat44_show
--        @description  show NAT44 info for all interfaces
--
--        @param params         none
--
--        @return       true on success, otherwise false and error message
--
local function nat44_show(params)
    -- show all NAT44 channels in the system

    --  get the first element in the table
    local current_index , current_entry
    local current_nat44_name_data
    local temp
    local local_interface
    local outer_interface
    local local_mac_addr_string
    local outer_mac_addr_string
    local private_ip_string
    local public_ip_string
    local server_ip_string
    local iterator

    if false == is_supported_feature(devNum, "NAT44_CMD_SUPPORT") then
        print("Command not supported for current device")
        return
    end

    iterator = 1
    current_index , current_entry = next(table_nat44_system_info,nil)

    temp = getStringWithSpaces("index#",10).."| "..getStringWithSpaces("nat-name",18).."| "..  getStringWithSpaces("local-ethernet-interface",25).."| "
            ..getStringWithSpaces("outer-ethernet-interface",25).."| ".. getStringWithSpaces("local-eport",15).."| "
            ..getStringWithSpaces("outer-eport",15)
    print(temp)
    temp = getStringWithSpaces(" ",10).."| "..getStringWithSpaces("local-mac-addr",18).."| " ..getStringWithSpaces("outer-mac-addr",25)
            .."| "..getStringWithSpaces("local-vid",25).."| "..getStringWithSpaces("outer-vid",15)

    print(temp)

    temp =getStringWithSpaces(" ",10).."| "..getStringWithSpaces("private-ip-addr",18).."| "..getStringWithSpaces("public-ip-addr",25).."| "
            ..getStringWithSpaces("server-ip-addr",25)
    print(temp)
    temp =getStringWithSpaces(" ",10).."| "..getStringWithSpaces("(optional) Protocol",18)
            .."| "..getStringWithSpaces("private_L4_port",25).."| "..getStringWithSpaces("public_L4_port",25)
            .."| "..getStringWithSpaces("server_L4_port",15)
    print(temp)
    print("---------------------------------------------------------------------------------------------------------------------------")
    print(" ")

    while current_entry do
        current_nat44_name_data = current_entry["nat_name"]
        if current_nat44_name_data ~= nil then

            temp = getStringWithSpaces(current_nat44_name_data,18)
            current_nat44_name_data = temp
            temp = current_entry["local_interface"].devId .."/"..current_entry["local_interface"].portNum
            local_interface = getStringWithSpaces(temp,25)
            temp = current_entry["outer_interface"].devId .."/"..current_entry["outer_interface"].portNum
            outer_interface = getStringWithSpaces(temp,25)
            temp = tostring(current_entry["local_mac_addr"]["string"])
            local_mac_addr_string = getStringWithSpaces(temp,18)
            temp = tostring(current_entry["outer_mac_addr"]["string"])
            outer_mac_addr_string = getStringWithSpaces(temp,25)
            temp = tostring(current_entry["private_ip"]["string"])
            private_ip_string = getStringWithSpaces(temp,18)
            temp = tostring(current_entry["public_ip"]["string"])
            public_ip_string = getStringWithSpaces(temp,25)
            temp = tostring(current_entry["server_ip"]["string"])
            server_ip_string = getStringWithSpaces(temp,25)

            print(getStringWithSpaces(tostring(iterator),10).."| "..
                    current_nat44_name_data .. "| " ..
                    local_interface .. "| " ..
                    outer_interface .. "| " ..
                    getStringWithSpaces(current_entry["local_eport"],15).. "| " ..
                    getStringWithSpaces(current_entry["outer_eport"],15))
            print(getStringWithSpaces(" ",10).."| "..
                    local_mac_addr_string  .. "| " ..
                    outer_mac_addr_string .. "| "..
                    getStringWithSpaces(current_entry["local_vid"],25)  .. "| " ..
                    getStringWithSpaces(current_entry["outer_vid"],25))
            print(getStringWithSpaces(" ",10).."| "..
                    private_ip_string  .. "| " ..
                    public_ip_string  .. "| " ..
                    server_ip_string)

            -- if the protocal was set, meaning wih value UDP or TCP
            if current_entry["protocol"]=="UDP" or current_entry["protocol"]=="TCP" then
                print(getStringWithSpaces(" ",10).."| "..
                        getStringWithSpaces(current_entry["protocol"],18)  .. "| " ..
                        getStringWithSpaces(current_entry["private_L4_port"],25) .. "| " ..
                        getStringWithSpaces(current_entry["public_L4_port"],25)  .. "| " ..
                        getStringWithSpaces(current_entry["server_L4_port"],15))
            end

            print ("")
            print ("")
            iterator = iterator + 1
        end

        current_index , current_entry = next(table_nat44_system_info,current_index)

    end

    print("number of nat44 defined "..global_num_nat44_channels )
    print ("")
    print ("End of " .. (iterator - 1) .." nat44 configuration")
    print ("")
end


-- the command looks like :
-- Console(config)# nat44 nat-name NAT44:1 local-ethernet-interface 0/18 outer-ethernet-interface 0/36 local-eport 0x1018 outer-eport 0x1036
--                  local-mac-addr 00:01:02:03:34:02 outer-mac-addr 00:04:05:06:07:11 local-vid 5 outer-vid 6
--                  private-ip-addr 192.168.1.1 public-ip-addr 198.51.100.1 server-ip-addr 17.18.19.20
CLI_addCommand("config", "nat44", {
    func   = nat44_func,
    help   = "Add IPv4 NAT session",
    params = {
        { type = "named",
            { format = "nat-name %s", name = "nat_name", help = "The name of the NAT session to be configured"},
            { format = "local-ethernet-interface %dev_port", name = "local_interface", help = "Specifies Ethernet port number in the private network"},
            { format = "outer-ethernet-interface %dev_port", name = "outer_interface", help = "Specifies Ethernet port number in the public network"},
            { format = "local-eport %ePortNat", name = "local_eport", help = "Specifies eport number in the private network"},
            { format = "outer-eport %ePortNat", name = "outer_eport", help = "Specifies eport number in the public network"},
            { format = "local-mac-addr %mac-address", name="local_mac_addr", help="Specifies the MAC Address of the host or of the next-hop router in the private network"},
            { format = "outer-mac-addr %mac-address", name="outer_mac_addr", help="Specifies the MAC Address of the next hop router in the public network"},
            { format = "local-vid %vid", name="local_vid", help="Specifies the VLAN in the private network"},
            { format = "outer-vid %vid", name="outer_vid", help="Specifies the VLAN in the public network"},
            { format = "private-ip-addr %ipv4", name = "private_ip", help = "The IP address of the host"},
            { format = "public-ip-addr %ipv4", name = "public_ip", help = "The public IP address of the NAT router"},
            { format = "server-ip-addr %ipv4", name = "server_ip", help = "The IP address of the server"},
            { format = "protocol %L4_protocol", name="protocol", help = "TCP or UDP, optional parameter"},
            { format = "private-L4-port %udp_tcp_port", name="private_L4_port", help="The TCP/UDP port of the host, optional parameter"},
            { format = "public-L4-port %udp_tcp_port", name="public_L4_port", help="The TCP/UDP port allocated by the NAT router, optional parameter"},
            { format = "server-L4-port %udp_tcp_port", name="server_L4_port", help="The TCP/UDP port of the server, optional parameter"},
            requirements={
                ["private_L4_port"]={"protocol"},
                ["public_L4_port"]={"private_L4_port"},
                ["server_L4_port"]={"public_L4_port"}},
            mandatory = {"nat_name","local_interface","local_eport","outer_interface","outer_eport","local_mac_addr","outer_mac_addr","local_vid","outer_vid","private_ip","public_ip","server_ip"},
        }
    }
})

-- the command looks like :
--Console(config)# no nat44 all
CLI_addCommand("config", "no nat44 all", {
    func   = no_nat44_all_func,
    help   = "Delete all NAT44 configuration, restore configurations",
    params = {}
})

-- the command looks like :
--Console(config)# no nat44 nat-name NAT44:1
CLI_addCommand("config", "no nat44", {
    func   = no_nat44_name,
    help   = "Delete single NAT44 configuration, restore configuration",
    params = {
        { type = "named",
            { format = "nat-name %s", name = "nat_name", help = "The name of the NAT session to be deleted"},
            mandatory = { "nat_name" }
        }
    }
})

-- the command looks like :
--Console# show nat44
CLI_addCommand("exec", "show nat44",
    {
        func   = nat44_show,
        help   = "Show NAT44 info configuration",
        params = {}
    }
)
