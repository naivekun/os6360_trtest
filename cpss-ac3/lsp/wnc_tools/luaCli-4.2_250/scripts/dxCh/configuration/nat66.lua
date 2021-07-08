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
--* nat66.lua
--*
--* DESCRIPTION:
--*       manage IPv6 NAT session
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixAdd")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixDel")
--require
--constants

local CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT = {
   ["CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E"] = 0,
   ["CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E"] = 1,
   ["CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E"] = 2,
   ["CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E"] = 3,
}

local debug_on = false
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
    end
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

    local nat66EntryPtr = {}
    nat66EntryPtr.nat66Entry = {}
    nat66EntryPtr.nat66Entry.prefixSize = 64

    -- unset a NAT entry
    apiName = "cpssDxChIpNatEntrySet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                    "devNum",   devNum },
        { "IN",     "GT_U32",                                   "natIndex", natIndex },
        { "IN",     "CPSS_IP_NAT_TYPE_ENT",                     "natType",  "CPSS_IP_NAT_TYPE_NAT66_E" },
        { "IN",     "CPSS_DXCH_IP_NAT_ENTRY_UNT_nat66Entry",    "entryPtr", nat66EntryPtr}
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

-- global number of NAT66 channels in the system
local global_num_nat66_channels = 0

--[[ define the table that will hold the 'NAT66 configuration done in the system'
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
                  14.prefix_length
                  15.ipNextHopIndex - calculated internally
                  16.natIndex - calculated internally
                  17.ipNextHopIndex1 - calculated internally
                  18.natIndex1 - calculated internally
--]]
local table_nat66_system_key -- the key to table_nat66_system_info is built from: 'nat name and physicalInfo'
local table_nat66_system_info = {}

function getTableNat66()
    return table_nat66_system_info
end

--[[ define the table that will hold the 'NAT66 server configuration done in the system'
     each line in the table hold info about server IP defined for each NAT66:
                  1.server IP - a unique ip (the key to the table)
                  2.nat_name - a unique name
                  3.public_ip
                  4.outer_mac_addr
--]]
local table_nat66_serverIP_info = {}

--[[ define the table that will hold the 'NAT66 public configuration done in the system'
     each line in the table hold info about public IP defined for each NAT66:
                  1.public IP - a unique ip (the key to the table)
                  2.nat_name - a unique name
--]]
local table_nat66_publicIP_info = {}

local function nat66_command_func(command_data,devNum,params)

    -- Common variables declaration
    local apiName,result,values
    local outerHwDevNum,localHwDevNum
    local ipNextHopIndex,natIndex -- private to public
    local ipNextHopIndex1,natIndex1 -- public to private
    local physicalInfo = {}
    local interfaceInfo = {}
    local egressInfo = {}
    local routeEntriesArray = {}
    local nat66EntryPtr = {}
    local system_mac_address = getGlobal("myMac")
    local nat66ModifiCmd_private_to_public
    local nat66ModifiCmd_public_to_private
    local define_server_entry_in_lpm = true
    local myIndexName

    local serverInfo, publicInfo

    if false == is_supported_feature(devNum, "NAT66_CMD_SUPPORT") then
        print("Command not supported for current device")
        return
    end


    serverInfo = table_nat66_serverIP_info[tostring(params.server_ip["string"])]
    -- if we have a full match then we look for the public_ip in the DB, if not then we look for the masked ip according to prefix length
    local ipAddressMasked
    if ((params.prefix_length == nil)or(params.prefix_length==128)) then
        ipAddressMasked = calcIpv6AddressMasked(tostring(params.public_ip["string"]),128);
    else
        ipAddressMasked = calcIpv6AddressMasked(tostring(params.public_ip["string"]),params.prefix_length);
    end

    publicInfo = table_nat66_publicIP_info[ipAddressMasked]
    if (serverInfo ~= nil) then
        if(serverInfo.nat_name ~= params.nat_name)then
            print(" This server IP = "..params.server_ip["string"].." ,was already defined in NAT66_db for nat_name="..serverInfo.nat_name)
            return nil
        else
            -- no need to define the server again in LPM since it was already defined in previous configuration of this nat
            define_server_entry_in_lpm=false

            -- check that the public_ip is the same as the one defined in previos nat configuration
            if (publicInfo ~= nil) then
                if(publicInfo.nat_name ~= params.nat_name) then
                    print(" This public IP = "..params.public_ip["string"].." ,is not legal for the given server_ip "..params.server_ip["string"].." since it should be the same as defined in previous configuration of nat_name="..publicInfo.nat_name)
                    return nil
                else
                    -- check that the outer_mac_addr is the same as the outer_mac_addr defined in previos nat configuration
                    if(publicInfo.outer_mac_addr["string"] ~= params.outer_mac_addr["string"]) then
                        print(" This outer_mac_addr = "..params.outer_mac_addr["string"].." ,is not legal for the given server_ip "..params.server_ip["string"].." since it should be the same as defined in previous configuration of nat_name="..publicInfo.nat_name.." outer_mac_addr = "..publicInfo.outer_mac_addr["string"])
                        return nil
                    end
                end

            end
        end
    end

   if (publicInfo ~= nil) then
        if(publicInfo.nat_name ~= params.nat_name)then
            print(" This public IP = "..params.public_ip["string"].." ,was already defined in NAT66_db for nat_name="..publicInfo.nat_name)
            return nil
        end
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

    if ((params.prefix_length == nil)or(params.prefix_length==128)) then
        params.prefix_length = 64
        nat66ModifiCmd_private_to_public = CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT["CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_ADDRESS_E"]
        nat66ModifiCmd_public_to_private = CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT["CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_ADDRESS_E"]
    else
        nat66ModifiCmd_private_to_public = CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT["CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_SIP_PREFIX_E"]
        nat66ModifiCmd_public_to_private = CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT["CPSS_DXCH_IP_NAT_TYPE_NAT66_MODIFY_DIP_PREFIX_E"]

        params.prefix_length = tonumber(params.prefix_length)
        if (params.prefix_length) < 1 or (params.prefix_length > 64) then
            print("Error: not valid prefix-length value")
            return nil
        end
    end

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
    myIndexName = "<NAT66: Private to Public> nat-name "..params.nat_name..","..params.local_interface.devId.."/"..params.local_interface.portNum..","..params.outer_interface.devId.."/"..params.outer_interface.portNum
    natIndex = sharedResourceNamedNatMemoAlloc(myIndexName,1)
    if natIndex == nil then
        return command_data:addErrorAndPrint(
            "tunnel start entry allocation error");
    end

    printLog("<NAT66: Private to Public> natIndex = " .. natIndex)
    
    nat66EntryPtr.nat66Entry = {}
    nat66EntryPtr.nat66Entry.macDa = params.outer_mac_addr
    nat66EntryPtr.nat66Entry.modifyCommand = nat66ModifiCmd_private_to_public
    nat66EntryPtr.nat66Entry.address = params.public_ip
    nat66EntryPtr.nat66Entry.prefixSize = params.prefix_length

    -- Set a NAT entry
    apiName = "cpssDxChIpNatEntrySet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                    "devNum",   devNum },
        { "IN",     "GT_U32",                                   "natIndex", natIndex },
        { "IN",     "CPSS_IP_NAT_TYPE_ENT",                     "natType",  "CPSS_IP_NAT_TYPE_NAT66_E" },
        { "IN",     "CPSS_DXCH_IP_NAT_ENTRY_UNT_nat66Entry",    "entryPtr", nat66EntryPtr}
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

    local nextHopInfoPtr = {}
    nextHopInfoPtr.ipLttEntry = {}
    nextHopInfoPtr.ipLttEntry.routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E"
    nextHopInfoPtr.ipLttEntry.numOfPaths = 0
    nextHopInfoPtr.ipLttEntry.ucRPFCheckEnable = false
    nextHopInfoPtr.ipLttEntry.sipSaCheckMismatchEnable = false
    nextHopInfoPtr.ipLttEntry.ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E"

    apiName = "cpssDxChCfgHwDevNumGet"
    result, outerHwDevNum = device_to_hardware_format_convert(params.outer_interface.devId)
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    if(define_server_entry_in_lpm==true)then
        myIndexName = "<NAT66: Private to Public> nat-name "..params.nat_name..","..params.local_interface.devId.."/"..params.local_interface.portNum..","..params.outer_interface.devId.."/"..params.outer_interface.portNum
        ipNextHopIndex = allocIpNextHopIndexFromPool(myIndexName,false)

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

        -- get members from the vlan entry
        apiName = "cpssDxChBrgVlanEntryRead"
        result, values = myGenWrapper(apiName, {
            { "IN",     "GT_U8",      "devNum",     devNum },
            { "IN",     "GT_U16",     "vlanId",   params.local_vid },
            { "OUT",     "CPSS_PORTS_BMP_STC",  "portsMembers"},
            { "OUT",     "CPSS_PORTS_BMP_STC",  "portsTagging"},
            { "OUT",     "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"},
            { "OUT",     "GT_BOOL",  "isValid"},
            { "OUT",     "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",  "portsTaggingCmd"}
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        nextHopInfoPtr.ipLttEntry.routeEntryBaseIndex = ipNextHopIndex

        --  Add LPM prefix
        _debug("vrfId " .. values.vlanInfo.vrfId)
        
        local vrfId_ = values.vlanInfo.vrfId
        local protocol_ = "CPSS_IP_PROTOCOL_IPV6_E"
        local server_ip_ = params.server_ip
        local is_sip_5_ = is_sip_5(devNum)
        result =wrLogWrapper("wrlCpssDxChIpLpmIpUcPrefixAdd", 
               "(vrfId_, protocol_, server_ip_, 128, ipNextHopIndex, is_sip_5_)",
               vrfId_, protocol_, server_ip_, 128, ipNextHopIndex, is_sip_5_)
        apiName = "wrlCpssDxChIpLpmIpUcPrefixAdd"
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)
    else
        -- just assign the ipNextHopIndex already defined in previous nat configuration
        local serverInfo_ = {}
        serverInfo_ = table_nat66_serverIP_info[tostring(params.server_ip["string"])]
        ipNextHopIndex = serverInfo_.ipNextHopIndex;
    end

    --  Enable unicast IPv6 routing on a port
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        params.local_interface.portNum       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV6_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  true }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --  Enable unicast IPv6 routing on a ePort
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        params.local_eport       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV6_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  true }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Enable IPv6 Unicast Routing on Vlan
    apiName = "cpssDxChBrgVlanIpUcRouteEnable"
    result = myGenWrapper(apiName, {
        { "IN",  "GT_U8",                       "devNum",       devNum },
        { "IN",  "GT_U16",                      "vlanId",       params.local_vid},
        { "IN",  "CPSS_IP_PROTOCOL_STACK_ENT",  "protocol",     "CPSS_IP_PROTOCOL_IPV6_E"},
        { "IN",  "GT_BOOL",                     "enable",       true },
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
    myIndexName = "<NAT66: Public to Private> nat-name "..params.nat_name..","..params.local_interface.devId.."/"..params.local_interface.portNum..","..params.outer_interface.devId.."/"..params.outer_interface.portNum
    natIndex1 = sharedResourceNamedNatMemoAlloc(myIndexName,1)
    if natIndex1 == nil then
        return command_data:addErrorAndPrint(
            "tunnel start entry allocation error");
    end

    printLog("<NAT66: Public to Private> natIndex1 = " .. natIndex1)
    
    nat66EntryPtr.nat66Entry = {}
    nat66EntryPtr.nat66Entry.macDa = params.local_mac_addr
    nat66EntryPtr.nat66Entry.modifyCommand = nat66ModifiCmd_public_to_private
    nat66EntryPtr.nat66Entry.address = params.private_ip
    nat66EntryPtr.nat66Entry.prefixSize = params.prefix_length

    -- Set a NAT entry
    apiName = "cpssDxChIpNatEntrySet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                    "devNum",   devNum },
        { "IN",     "GT_U32",                                   "natIndex", natIndex1 },
        { "IN",     "CPSS_IP_NAT_TYPE_ENT",                     "natType",  "CPSS_IP_NAT_TYPE_NAT66_E" },
        { "IN",     "CPSS_DXCH_IP_NAT_ENTRY_UNT_nat66Entry",    "entryPtr", nat66EntryPtr}
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

    myIndexName = "<NAT66: Public to Private> nat-name "..params.nat_name..","..params.local_interface.devId.."/"..params.local_interface.portNum..","..params.outer_interface.devId.."/"..params.outer_interface.portNum
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

    -- get members from the vlan entry
    apiName = "cpssDxChBrgVlanEntryRead"
    result, values = myGenWrapper(apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U16",     "vlanId",   params.local_vid },
        { "OUT",     "CPSS_PORTS_BMP_STC",  "portsMembers"},
        { "OUT",     "CPSS_PORTS_BMP_STC",  "portsTagging"},
        { "OUT",     "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"},
        { "OUT",     "GT_BOOL",  "isValid"},
        { "OUT",     "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",  "portsTaggingCmd"}
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    nextHopInfoPtr.ipLttEntry.routeEntryBaseIndex = ipNextHopIndex1

    --  Add LPM prefix
    local vrfId_ = values.vlanInfo.vrfId
    local protocol_ = "CPSS_IP_PROTOCOL_IPV6_E"
    local public_ip_ = params.public_ip
    local is_sip_5_ = is_sip_5(devNum)
    
    _debug("vrfId " .. values.vlanInfo.vrfId)
    result =wrLogWrapper("wrlCpssDxChIpLpmIpUcPrefixAdd", 
              "(vrfId_, protocol_, public_ip_, 128, ipNextHopIndex1, is_sip_5_)", 
              vrfId_, protocol_, public_ip_, 128, ipNextHopIndex1, is_sip_5_)
    apiName = "wrlCpssDxChIpLpmIpUcPrefixAdd"
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --  Enable unicast IPv6 routing on a port
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        params.outer_interface.portNum       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV6_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  true }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --  Enable unicast IPv6 routing on a ePort
    apiName = "cpssDxChIpPortRoutingEnable"
    result = myGenWrapper(apiName, {
        { "IN", "GT_U8",                         "devNum",         devNum                   },
        { "IN", "GT_PORT_NUM",                   "portNum",        params.outer_eport       },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV6_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  true }
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Enable IPv6 Unicast Routing on Vlan
    apiName = "cpssDxChBrgVlanIpUcRouteEnable"
    result = myGenWrapper(apiName, {
        { "IN",  "GT_U8",                       "devNum",       devNum },
        { "IN",  "GT_U16",                      "vlanId",       params.outer_vid},
        { "IN",  "CPSS_IP_PROTOCOL_STACK_ENT",  "protocol",     "CPSS_IP_PROTOCOL_IPV6_E"},
        { "IN",  "GT_BOOL",                     "enable",       true },
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
    entryInfo.prefix_length  = params.prefix_length
    entryInfo.ipNextHopIndex  = ipNextHopIndex
    entryInfo.natIndex  = natIndex
    entryInfo.ipNextHopIndex1  = ipNextHopIndex1
    entryInfo.natIndex1  = natIndex1

    -- use the 'nat name and physicalInfo' as a key to the table
    table_nat66_system_key = params.nat_name..","..params.local_interface.devId.."/"..params.local_interface.portNum..","..params.outer_interface.devId.."/"..params.outer_interface.portNum
    table_nat66_system_info[table_nat66_system_key] = entryInfo

     -- use the 'server_ip' as a key to the table
    local serverInfo = {}
    serverInfo.nat_name=params.nat_name
    serverInfo.ipNextHopIndex = ipNextHopIndex
    table_nat66_serverIP_info[tostring(params.server_ip["string"])] = serverInfo
    -- if the public_ip is with prefixLength =128 then we add the full ip to the DB,
    -- if not then we should calculate to masked prefix according to prefix length
    local publicInfo = {}
    publicInfo.nat_name = params.nat_name
    publicInfo.outer_mac_addr = params.outer_mac_addr
    table_nat66_publicIP_info[ipAddressMasked] = publicInfo

    -- another NAT66 channel was added
    global_num_nat66_channels = global_num_nat66_channels + 1

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- ************************************************************************
--
--  nat66_func
--        @description  Command to add an IPv6 NAT session
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
--                                  - params["prefix_length"]: prefix length, optional parameter
--
--        @return       true on success, otherwise false and error message
--

local function nat66_func(params)

    local command_data = Command_Data()

    command_data:initInterfaceDeviceRange()
    -- Common variables initialization.
    command_data:initAllAvailableDevicesRange()
    command_data:initAllAvaiblesPortIterator()

    -- now iterate
    command_data:iterateOverDevices(nat66_command_func,params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

local function no_nat66_name_function(command_data,devNum,params)

    -- Common variables declaration
    local apiName,result,values
    local localHwDevNum,outerHwDevNum
    local entryInfo = {}
    local interfaceInfo = {}
    local egressInfo = {}
    local zero = 0
    local myIndexName
    local nat66_entry_was_deleted=false

    if false == is_supported_feature(devNum, "NAT66_CMD_SUPPORT") then
        print("Command not supported for current device")
        return
    end

    local current_index

    -- print input params
    _debug("no NAT66 name command params \n "..to_string(params))

    current_index , entryInfo = next(table_nat66_system_info,nil)
    while entryInfo do
        if(entryInfo.nat_name ~= nil) then
            if(params.nat_name == entryInfo.nat_name)then
                -- delete this NAT entry and deconfigure all needed configuration that was done

                ------------- Disable private to public configuration ---------------------

                -- unset physical info for the given ePort
                default_cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,entryInfo.local_eport)

                -- unset a NAT66 entry
                default_cpssDxChIpNatEntrySet(devNum,entryInfo.natIndex)

                myIndexName = "<NAT66: Private to Public> nat-name "..entryInfo.nat_name..","..entryInfo.local_interface.devId.."/"..entryInfo.local_interface.portNum..","..entryInfo.outer_interface.devId.."/"..entryInfo.outer_interface.portNum

                sharedResourceNamedNatMemoFree(myIndexName);

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

                if(nat66_entry_was_deleted==false)then
                    -- unset a IP NEXT HOP entry
                    default_cpssDxChIpUcRouteEntriesWrite(devNum,entryInfo.ipNextHopIndex)
                    freeIpNextHopIndexToPool(entryInfo.ipNextHopIndex)

                    -- get members from the vlan entry
                    apiName = "cpssDxChBrgVlanEntryRead"
                    result, values = myGenWrapper(apiName, {
                        { "IN",     "GT_U8",      "devNum",     devNum },
                        { "IN",     "GT_U16",     "vlanId",   entryInfo.local_vid },
                        { "OUT",     "CPSS_PORTS_BMP_STC",  "portsMembers"},
                        { "OUT",     "CPSS_PORTS_BMP_STC",  "portsTagging"},
                        { "OUT",     "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"},
                        { "OUT",     "GT_BOOL",  "isValid"},
                        { "OUT",     "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",  "portsTaggingCmd"}
                    })
                    command_data:handleCpssErrorDevPort(result, apiName)
                    _debug(apiName .. "   result = " .. result)


                    --  Delete LPM prefix
                    local vrfId_ = values.vlanInfo.vrfId
                    local protocol_ = "CPSS_IP_PROTOCOL_IPV6_E"
                    local server_ip_ = entryInfo.server_ip
                    
                    _debug("vrfId " .. values.vlanInfo.vrfId)
                    result = wrLogWrapper("wrlCpssDxChIpLpmIpUcPrefixDel", 
                                           "(vrfId_, protocol_, server_ip_, 128)", 
                                           vrfId_, protocol_, server_ip_, 128)
                    apiName = "wrlCpssDxChIpLpmIpUcPrefixDel"
                    command_data:handleCpssErrorDevPort(result, apiName)
                    _debug(apiName .. "   result = " .. result)
                end

                --  Disable unicast IPv6 routing on a port
                apiName = "cpssDxChIpPortRoutingEnable"
                result = myGenWrapper(apiName, {
                    { "IN", "GT_U8",                         "devNum",         devNum                   },
                    { "IN", "GT_PORT_NUM",                   "portNum",        entryInfo.local_interface.portNum    },
                    { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
                    { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV6_E" },
                    { "IN", "GT_BOOL",                       "enableRouting",  false }
                })
                command_data:handleCpssErrorDevPort(result, apiName)
                _debug(apiName .. "   result = " .. result)

                --  Disable unicast IPv6 routing on a ePort
                apiName = "cpssDxChIpPortRoutingEnable"
                result = myGenWrapper(apiName, {
                    { "IN", "GT_U8",                         "devNum",         devNum                   },
                    { "IN", "GT_PORT_NUM",                   "portNum",        entryInfo.local_eport       },
                    { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
                    { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV6_E" },
                    { "IN", "GT_BOOL",                       "enableRouting",  false }
                })
                command_data:handleCpssErrorDevPort(result, apiName)
                _debug(apiName .. "   result = " .. result)

                -- Disable IPv4 Unicast Routing on Vlan
                apiName = "cpssDxChBrgVlanIpUcRouteEnable"
                result = myGenWrapper(apiName, {
                    { "IN",  "GT_U8",                       "devNum",       devNum },
                    { "IN",  "GT_U16",                      "vlanId",       entryInfo.local_vid},
                    { "IN",  "CPSS_IP_PROTOCOL_STACK_ENT",  "protocol",     "CPSS_IP_PROTOCOL_IPV6_E"},
                    { "IN",  "GT_BOOL",                     "enable",       false },
                })
                command_data:handleCpssErrorDevPort(result, apiName)
                _debug(apiName .. "   result = " .. result)

                apiName = "cpssDxChCfgHwDevNumGet"
                result, localHwDevNum = device_to_hardware_format_convert(entryInfo.local_interface.devId)
                command_data:handleCpssErrorDevPort(result, apiName.."()")
                _debug(apiName .. "   result = " .. result)

                interfaceInfo.type = "CPSS_INTERFACE_PORT_E"
                interfaceInfo.devPort = {}
                interfaceInfo.devPort.devNum  = localHwDevNum
                interfaceInfo.devPort.portNum = entryInfo.local_eport

                ------------- Public to private configuration ---------------------

                -- unset physical info for the given ePort
                default_cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,entryInfo.outer_eport)

                -- unset a NAT66 entry
                default_cpssDxChIpNatEntrySet(devNum,entryInfo.natIndex1)
                myIndexName = "<NAT66: Public to Private> nat-name "..params.nat_name..","..entryInfo.local_interface.devId.."/"..entryInfo.local_interface.portNum..","..entryInfo.outer_interface.devId.."/"..entryInfo.outer_interface.portNum
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

                -- get members from the vlan entry
                apiName = "cpssDxChBrgVlanEntryRead"
                result, values = myGenWrapper(apiName, {
                    { "IN",     "GT_U8",      "devNum",     devNum },
                    { "IN",     "GT_U16",     "vlanId",   entryInfo.local_vid },
                    { "OUT",     "CPSS_PORTS_BMP_STC",  "portsMembers"},
                    { "OUT",     "CPSS_PORTS_BMP_STC",  "portsTagging"},
                    { "OUT",     "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"},
                    { "OUT",     "GT_BOOL",  "isValid"},
                    { "OUT",     "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",  "portsTaggingCmd"}
                })
                command_data:handleCpssErrorDevPort(result, apiName)
                _debug(apiName .. "   result = " .. result)

                --  Delete LPM prefix
                local vrfId_ = values.vlanInfo.vrfId
                local protocol_ = "CPSS_IP_PROTOCOL_IPV6_E"
                local public_ip_ = entryInfo.public_ip
                _debug("vrfId " .. values.vlanInfo.vrfId)
                result = wrLogWrapper("wrlCpssDxChIpLpmIpUcPrefixDel", 
                                       "(vrfId_, protocol_, public_ip_, 128)", 
                                        vrfId_, protocol_, public_ip_, 128)
                apiName = "wrlCpssDxChIpLpmIpUcPrefixDel"
                command_data:handleCpssErrorDevPort(result, apiName)
                _debug(apiName .. "   result = " .. result)

                -- Disable unicast IPv6 routing on a port
                apiName = "cpssDxChIpPortRoutingEnable"
                result = myGenWrapper(apiName, {
                    { "IN", "GT_U8",                         "devNum",         devNum                   },
                    { "IN", "GT_PORT_NUM",                   "portNum",        entryInfo.outer_interface.portNum       },
                    { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
                    { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV6_E" },
                    { "IN", "GT_BOOL",                       "enableRouting",  false }
                })
                command_data:handleCpssErrorDevPort(result, apiName)
                _debug(apiName .. "   result = " .. result)

                --  Disable unicast IPv6 routing on a ePort
                apiName = "cpssDxChIpPortRoutingEnable"
                result = myGenWrapper(apiName, {
                    { "IN", "GT_U8",                         "devNum",         devNum                   },
                    { "IN", "GT_PORT_NUM",                   "portNum",        entryInfo.outer_eport       },
                    { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"       },
                    { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV6_E" },
                    { "IN", "GT_BOOL",                       "enableRouting",  false }
                })
                command_data:handleCpssErrorDevPort(result, apiName)
                _debug(apiName .. "   result = " .. result)

                -- Disable IPv6 Unicast Routing on Vlan
                apiName = "cpssDxChBrgVlanIpUcRouteEnable"
                result = myGenWrapper(apiName, {
                    { "IN",  "GT_U8",                       "devNum",       devNum },
                    { "IN",  "GT_U16",                      "vlanId",       entryInfo.outer_vid},
                    { "IN",  "CPSS_IP_PROTOCOL_STACK_ENT",  "protocol",     "CPSS_IP_PROTOCOL_IPV6_E"},
                    { "IN",  "GT_BOOL",                     "enable",       false },
                })
                command_data:handleCpssErrorDevPort(result, apiName)
                _debug(apiName .. "   result = " .. result)

                ------------- DB configuration ---------------------

                 -- operation succed remove the info from DB
                table_nat66_serverIP_info[tostring(entryInfo.server_ip["string"])] = nil
                table_nat66_publicIP_info[tostring(entryInfo.public_ip["string"])] = nil

                -- operation succed remove the info from DB
                table_nat66_system_key = params.nat_name..","..entryInfo.local_interface.devId.."/"..entryInfo.local_interface.portNum..","..entryInfo.outer_interface.devId.."/"..entryInfo.outer_interface.portNum
                table_nat66_system_info[table_nat66_system_key] = nil
                -- NAT66 channel was deleted
                global_num_nat66_channels = global_num_nat66_channels - 1

                nat66_entry_was_deleted=true
            end
        end
        current_index , entryInfo = next(table_nat66_system_info,current_index)
    end

    -- read params from DB
    if nat66_entry_was_deleted == false then
        print("nat name "..params.nat_name.." was not defined in NAT66_db")
        return nil
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- ************************************************************************
--
--  no_nat66_name
--        @description  Unset single NAT66 configurations for 'nat name', restore configuration
--
--        @param params   - params["nat_name"]: name of the NAT session, a unique name
--
--        @return       true on success, otherwise false and error message
--
local function no_nat66_name(params)
    local command_data = Command_Data()

    command_data:initInterfaceDeviceRange()

    -- Common variables initialization.
    command_data:initAllAvailableDevicesRange()
    command_data:initAllAvaiblesPortIterator()

    -- now iterate
    command_data:iterateOverDevices(no_nat66_name_function,params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

-- ************************************************************************
--  no_nat66_all_func
--        @description Unset all NAT66 configurations, restore configuration
--
--        @param params   - none
--
--        @return       true on success, otherwise false and error message
--
local function no_nat66_all_func()

    local command_data = Command_Data()
    -- Common variables declaration
    local current_index
    local current_entry = {}
    local params = {}
    local apiName,result

    if false == is_supported_feature(devNum, "NAT66_CMD_SUPPORT") then
        print("Command not supported for current device")
        return
    end
    -- print input params
    _debug("no NAT66 all command no params \n")

    -- get the first element in the table
    current_index , current_entry = next(table_nat66_system_info,nil)
    while current_entry do

        if current_entry.nat_name ~= nil then
            params.nat_name = current_entry.nat_name
            no_nat66_name(params)
        end

        current_index , current_entry = next(table_nat66_system_info,current_index)

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
--  nat66_show
--        @description  show NAT66 info for all interfaces
--
--        @param params         none
--
--        @return       true on success, otherwise false and error message
--
local function nat66_show(params)
    -- show all NAT66 channels in the system

    --  get the first element in the table
    local current_index , current_entry
    local current_nat66_name_data
    local temp
    local local_interface
    local outer_interface
    local local_mac_addr_string
    local outer_mac_addr_string
    local private_ip_string
    local public_ip_string
    local server_ip_string

    local iterator
    if false == is_supported_feature(devNum, "NAT66_CMD_SUPPORT") then
        print("Command not supported for current device")
        return
    end
    iterator = 1
    current_index , current_entry = next(table_nat66_system_info,nil)

    temp = getStringWithSpaces("index#",10).."| "..getStringWithSpaces("nat-name",42).."| "..  getStringWithSpaces("local-ethernet-interface",25).."| "
           .. getStringWithSpaces("local-eport",15)
    print(temp)
    temp = getStringWithSpaces(" ",10).."| "..getStringWithSpaces("private-ip-addr",42).."| "..getStringWithSpaces("outer-ethernet-interface",25).."| "
            ..getStringWithSpaces("outer-eport",15)
    print(temp)
    temp =getStringWithSpaces(" ",10).."| "..getStringWithSpaces("public-ip-addr",42).."| "..getStringWithSpaces("local-mac-addr",25).."| "..getStringWithSpaces("local-vid",15)
    print(temp)
    temp =getStringWithSpaces(" ",10).."| ".. getStringWithSpaces("server-ip-addr",42).."| "..getStringWithSpaces("outer-mac-addr",25).."| "..getStringWithSpaces("outer-vid",15)
    print(temp)
    temp =getStringWithSpaces(" ",10).."| ".. getStringWithSpaces("prefix-length",42)
    print(temp)
    print("-------------------------------------------------------------------------------------------------------")
    print(" ")

    while current_entry do
        current_nat66_name_data = current_entry["nat_name"]
        if current_nat66_name_data ~= nil then

            temp = getStringWithSpaces(current_nat66_name_data,42)
            current_nat66_name_data = temp
            temp = current_entry["local_interface"].devId .."/"..current_entry["local_interface"].portNum
            local_interface = getStringWithSpaces(temp,25)
            temp = current_entry["outer_interface"].devId .."/"..current_entry["outer_interface"].portNum
            outer_interface = getStringWithSpaces(temp,25)
            temp = tostring(current_entry["local_mac_addr"]["string"])
            local_mac_addr_string = getStringWithSpaces(temp,25)
            temp = tostring(current_entry["outer_mac_addr"]["string"])
            outer_mac_addr_string = getStringWithSpaces(temp,25)
            temp = tostring(current_entry["private_ip"]["string"])
            private_ip_string = getStringWithSpaces(temp,42)
            temp = tostring(current_entry["public_ip"]["string"])
            public_ip_string = getStringWithSpaces(temp,42)
            temp = tostring(current_entry["server_ip"]["string"])
            server_ip_string = getStringWithSpaces(temp,42)

            print(getStringWithSpaces(tostring(iterator),10).."| "..
                    current_nat66_name_data .. "| " ..
                    local_interface .. "| " ..
                    getStringWithSpaces(current_entry["local_eport"],15))
            print(getStringWithSpaces(" ",10).."| "..
                    private_ip_string  .. "| " ..
                    outer_interface .. "| " ..
                    getStringWithSpaces(current_entry["outer_eport"],15))

            print(getStringWithSpaces(" ",10).."| "..
                   public_ip_string  .. "| " ..
                   local_mac_addr_string  .. "| " ..
                   getStringWithSpaces(current_entry["local_vid"],15))

            print(getStringWithSpaces(" ",10).."| "..
                  server_ip_string.. "| "..
                  outer_mac_addr_string .. "| "..
                  getStringWithSpaces(current_entry["outer_vid"],15))

            print(getStringWithSpaces(" ",10).."| "..
                  getStringWithSpaces(current_entry["prefix_length"],42))

            print ("")
            print ("")
            iterator = iterator + 1
        end

        current_index , current_entry = next(table_nat66_system_info,current_index)

    end

    print("number of nat66 defined "..global_num_nat66_channels )
    print ("")
    print ("End of " .. (iterator - 1) .." nat66 configuration")
    print ("")
end

-- the command looks like :
-- Console(config)# nat66 nat-name NAT66:1 local-ethernet-interface 0/18 outer-ethernet-interface 0/36 local-eport 0x1018 outer-eport 0x1036
--                  local-mac-addr 00:01:02:03:34:02 outer-mac-addr 00:04:05:06:07:11 local-vid 5 outer-vid 6
--                  private-ip-addr FD01::3456 public-ip-addr 2001::5679 server-ip-addr 6545::5679 prefix-length 64
CLI_addCommand("config", "nat66", {
    func   = nat66_func,
    help   = "Add IPv6 NAT session",
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
            { format = "private-ip-addr %ipv6", name = "private_ip", help = "The IP address of the host"},
            { format = "public-ip-addr %ipv6", name = "public_ip", help = "The public IP address of the NAT router"},
            { format = "server-ip-addr %ipv6", name = "server_ip", help = "The IP address of the server"},
            { format = "prefix-length %prefix-length", name = "prefix_length", help = "The IP prefix length, optional parameter"},
            mandatory = {"nat_name","local_interface","local_eport","outer_interface","outer_eport","local_mac_addr","outer_mac_addr","local_vid","outer_vid","private_ip","public_ip","server_ip"},
        }
    }
})

-- the command looks like :
--Console(config)# no nat66 all
CLI_addCommand("config", "no nat66 all", {
    func   = no_nat66_all_func,
    help   = "Delete all NAT66 configuration, restore configurations",
    params = {}
})

-- the command looks like :
--Console(config)# no nat66 nat-name NAT66:1
CLI_addCommand("config", "no nat66", {
    func   = no_nat66_name,
    help   = "Delete single NAT66 configuration, restore configuration",
    params = {
        { type = "named",
            { format = "nat-name %s", name = "nat_name", help = "The name of the NAT session to be deleted"},
            mandatory = { "nat_name" }
        }
    }
})

-- the command looks like :
--Console# show nat66
CLI_addCommand("exec", "show nat66",
    {
        func   = nat66_show,
        help   = "Show NAT66 info configuration",
        params = {}
    }
)
