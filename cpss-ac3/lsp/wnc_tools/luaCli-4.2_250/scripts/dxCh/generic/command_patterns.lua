--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* command_patterns.lua
--*
--* DESCRIPTION:
--*       command processing subroutines
--*
--* FILE REVISION NUMBER:
--*       $Revision: 27 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChMaximumRouterArpIndex")
cmdLuaCLI_registerCfunction("wrlCpssDxChMaximumRouterNextHopEntryIndex")
cmdLuaCLI_registerCfunction("wrlCpssDxChValidVlansGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChNextBrgPortVlanGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgFdbTableSizeGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgFdbNextEntryGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgFdbNextValidEntryGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgFdbEntryGreateOrGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChNextRouterArpEntryGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChNextIpUcRouteEntryGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChNextValidIpPrefixDataGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgFirstEmptyVidxGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChNextUnusedMRUIndexGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgMcEntryRead")
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgMcEntryWrite")
cmdLuaCLI_registerCfunction("wrlDevFilterRangeSet")
cmdLuaCLI_registerCfunction("wrlCpssDxChSerdesConfigurationGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChSerdesConfigurationSet")

-- ************************************************************************
---
--  Command_Data:initInterfaceInfoStructure
--        @description  initialization of CPSS_INTERFACE_INFO_STC according to entry params
--
--        @param params         - params["ethernet"]["devId"]: device number
--                                of ethernet port;
--                                params["ethernet"]["portNum"]: port number
--                                of ethernet port
--                                params["port-channel"]: trunk id
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initInterfaceInfoStructure(params, alternative)
    local result, values, error_message
    local no_need_alternative = true

    self["interface_info"]  = { fabricVidx  = 0,
                                type        = "CPSS_INTERFACE_PORT_E",
                                index       = 0,
                                vidx        = 0,
                                vlanId      = 0,
                                devNum      = 0,
                                trunkId     = 0,
                                devPort     = { portNum   = 0,
                                                devNum    = 0 }}

    if  params["eport"] or params["ethernet"] then
       local devNum, portNum
       if params["eport"] then
          devNum  = params["eport"]["devId"]
          portNum = params["eport"]["portNum"]
       else
          devNum  = params["ethernet"]["devId"]
          portNum = params["ethernet"]["portNum"]
       end

       local devNum, portNum = self:getHWDevicePort(devNum, portNum)

       self["interface_info"]["type"] = "CPSS_INTERFACE_PORT_E"
       self["interface_info"]["devPort"]["devNum"]     = devNum
       self["interface_info"]["devPort"]["portNum"]    = portNum

    elseif "number" == type(params["port-channel"]) then
        self["interface_info"]["type"]    = "CPSS_INTERFACE_TRUNK_E"
        self["interface_info"]["trunkId"] = params["port-channel"]
    elseif true == alternative then
        result                = 0
        no_need_alternative   = false
    else
        result                = 1
        error_message         = "Such kind of interface is not supported."
        no_need_alternative   = true
    end

    return no_need_alternative
end

-- ************************************************************************
---
--  Command_Data:initDevicesPortCounts
--        @description  device/port counts range initialization in command
--                      execution data object according to entry parameters
--
--        @param params         - not used
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @usage self["dev_range"]
--                              - device range of command execution data
--                                object
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initDevicesPortCounts(params, alternative)
    local result, error_message
    local no_need_alternative = true

    result = 0

    if  nil ~= self["dev_range"] then
        self:setDevicesPortCounts(self["dev_range"])
    else
        result                = 1
        error_message         = "Such kind of devive ports counts is not supported."
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initGivenDevicePortCounts
--        @description  device/port counts range initialization of given
--                      device in command execution data object
--
--        @param devId          - given device number
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initGivenDevicePortCounts(devId, alternative)
    local result, error_message
    local no_need_alternative = true

    result = 0

    if  nil ~= devId then
        self:setDevicesPortCounts({ devId })
    else
        result                = 1
        error_message         = "Such kind of devive ports counts is not supported."
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initMirroredPorts
--        @description  mirrored ports interation initialization in command
--                      execution data object according to entry parameters
--
--        @param rx_or_tx       - "rx" - iterating over rx ports,
--                                "tx" - iterating over tx ports,
--                                otherwise iterating over tx and rx ports
--
--
--        @return       operation succed
--
function Command_Data:initMirroredPorts(rx_or_tx)
    self["rx_or_tx"] = rx_or_tx
end


-- ************************************************************************
---
--  Command_Data:initDeviceRouterArpEntries
--        @description  router arp entries iterator initialization in
--                      command execution data object according to entry
--                      parameters
--                      Requires:
--                          initAllDeviceRange(params).
--
--        @param params         - params["router_arp_index"],
--                                params["arp_entry_index"]: entry router
--                                arp index, could be irrelevant
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @usage self["dev_range"]
--                              - device range of command execution data
--                                object
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initDeviceRouterArpEntries(params, alternative)
    local result, error_message
    local no_need_alternative = true

    result = 0

    if  nil ~= self["dev_range"] then
        self:setEmptyRouterArpEntries(self["dev_range"])
        self:setDevicesRouterArpEntriesCounts(self["dev_range"])

        if nil ~= params["router_arp_index"] then
            self:setDevicesRouterArpEntriesIndexes(self["dev_range"],
                                                   {params["router_arp_index"]})
        elseif nil ~= params["arp_entry_index"] then
            self:setDevicesRouterArpEntriesIndexes(self["dev_range"],
                                                   {params["arp_entry_index"]})

        end
    else
        result                = 1
        error_message         = "Such kind of router ARP entries is not supported"
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initDeviceIpUcRouteEntries
--        @description  ip unicast route entries iterator initialization in
--                      command execution data object according to entry
--                      parameters
--                      Requires:
--                          initAllDeviceRange(params).
--
--        @param params         - not used
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @usage self["dev_range"]
--                              - device range of command execution data
--                                object
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initDeviceIpUcRouteEntries(params, alternative)
    local result, error_message
    local no_need_alternative = true

    result = 0

    if  nil ~= self["dev_range"] then
        self:setEmptyIpUcRouteEntries(self["dev_range"])
        self:setDevicesIpUcRouteEntriesCounts(self["dev_range"])
    else
        result                = 1
        error_message         = "Such kind of ip unicast route entries is " ..
                                "not supported"
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initIpPrefixEntries
--        @description  ip v4 or v6 prefix entries interation
--                      initialization
--
--        @param params         - params["ip_protocol"]: ip v4 or v6
--                                protocol
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initIpPrefixEntries(params, alternative)
    local result, error_message
    local no_need_alternative = true

    result = 0

    if  nil ~= self["dev_range"] then
        self:setIpPrefixEntries(params["ip_protocol"])
    else
        result                = 1
        error_message         = "Such kind of ip prefix entries is not " ..
                                "supported"
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initDeviceMirrorAnalyzerEntries
--        @description  mirror analyser entries initialization in command
--                      execution data object according to entry parameters
--                      Requires:
--                          initAllDeviceRange(params).
--
--        @param params         - not used
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @usage self["dev_range"]
--                              - device range of command execution data
--                                object
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initDeviceMirrorAnalyzerEntries(params, alternative)
    local result, error_message
    local no_need_alternative = true

    result = 0

    if  nil ~= self["dev_range"] then
        self:setEmptyMirrorAnalyzerEntries(self["dev_range"])
        self:setDevicesMirrorAnalyzerEntriesCounts(self["dev_range"])
    elseif true == alternative   then
        result                = 0
        no_need_alternative   = false
    else
        result                = 1
        error_message         = "Such kind of mirror analyzer entries is not supported."
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initGivenDeviceMirrorAnalyzerEntries
--        @description  mirror analyser entries initialization of given
--                      device in command execution data object according
--                      to entry parameters
--                      Requires:
--                          initAllDeviceRange(params).
--
--        @param devId          - given device number
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @usage self["dev_range"]
--                              - device range of command execution data
--                                object
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initGivenDeviceMirrorAnalyzerEntries(devId, alternative)
    local result, error_message
    local no_need_alternative = true

    result = 0

    if  nil ~= self["dev_range"] then
        self:setEmptyMirrorAnalyzerEntries({ devId })
        self:setDevicesMirrorAnalyzerEntriesCounts({ devId })
    elseif true == alternative   then
        result                = 0
        no_need_alternative   = false
    else
        result                = 1
        error_message         = "Such kind of mirror analyzer entries is not supported."
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initDevicesVidxRange
--        @description  vidxes initialization in command execution data
--                      object according to entry parameters
--                      Requires:
--                          initAllDeviceRange(params).
--
--        @param params         - not used
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @usage self["dev_range"]
--                              - device range of command execution data
--                                object
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initDevicesVidxRange(params, alternative)
    self:setDevicesEmptyVidxRange(self["dev_range"])

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initDevTrunkWithPortsRange
--        @description  initialization of dev/trunk range of command
--                      execution data object according to entry
--                      parameters; all trunks should contains ports
--
--
--        @param params         - params["port-channel"]: predefined port
--                                channels to add to range
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initDevTrunkWithPortsRange(params, alternative)
    local result, error_message
    local no_need_alternative = true

    result = 0

    if     nil ~= params["port-channel"]                then
        if does_trunk_contains_ports(devNum, params["port-channel"])    then
            self:setDevTrunkRange(makeNestedTable(self["dev_range"],
                                                  params["port-channel"]))
        else
            self:setDevTrunkRange(self:getEmptyDeviceNestedTable())
        end
    elseif nil ~= self["dev_range"]                     then
        self:setDevTrunkWithPortsRange(self["dev_range"])
    elseif true == alternative                          then
        result                = 0
        no_need_alternative   = false
        else
        result                = 1
        error_message         = "Such kind of dev/trunk range is not supported."
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initVlanRange
--        @description  sets vlans range in command execution data object
--                      according to command params
--
--        @param params         - params["vlan-range"],
--                                params["vlan_range"]: vlan range
--        @param params         - params["vlanId"]: vlan id
--        @param alternative    - alternative interface behavior
--                                checking flag
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initVlanRange(params, alternative)
    local result, values, error_message
    local no_need_alternative = true

    result = 0

    if      nil ~= params["vlan-range"] then
        result = self:setVlanRange(params["vlan-range"])
    elseif  nil ~= params["vlan_range"] then
        result = self:setVlanRange(params["vlan_range"] )
    elseif  nil ~= params["vlanId"]         then
        result = self:setVlanRange({params["vlanId"]})
    elseif true == alternative        then
        result                = 0
        no_need_alternative   = false
    else
        result                = 1
        error_message         = "Such kind of vlan range is not supported."
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initDeviceVlanRange
--        @description  sets device/vlans range in command execution data
--                      object according to device range of command
--                      execution data object and command params
--                      requares: command_data:initAllDeviceRange(params)
--                                                               or
--                                initAllAvailableDevicesRange() or
--                                initInterfaceDeviceRange()
--
--        @param params         - params["all_vlans"]: property of all
--                                avaible vlans iteration
--        @param params         - params["vlan-range"],
--                                params["vlan_range"]: vlan range
--        @param params         - params["vlanId"]: vlan id
--        @param params         - params["configurable_vlan"]: single id of
--                                new vlan
--        @param params         - params["deleted_vlan"]: single id of
--                                deleted vlan
--        @param alternative    - alternative interface behavior
--                                checking flag
--        @usage self           - self["dev_range"]: processed devices
--                                range
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initDeviceVlanRange(params, alternative)
    local result, values, error_message
    local no_need_alternative = true

    result = 0

    if      nil ~= params["all_vlans"] then
        result = self:setDevAllAvaibleVlanRange(self["dev_range"])
    elseif  nil ~= params["vlan-range"] then
        result = self:setDevVlanRange(makeNestedTable(self["dev_range"],
                                                      params["vlan-range"]))
    elseif  nil ~= params["vlan_range"] then
        result = self:setDevVlanRange(makeNestedTable(self["dev_range"],
                                                      params["vlan_range"]))
    elseif  nil ~= params["vlan"]           then
        result = self:setDevVlanRange(makeNestedTable(self["dev_range"],
                                                      params["vlan"]))
    elseif  nil ~= params["vlanId"]         then
        result = self:setDevVlanRange(makeNestedTable(self["dev_range"],
                                                      params["vlanId"]))
    elseif  nil ~= params["configurable_vlan"] then
        result = self:setDevVlanRange(makeNestedTable(
                                          self["dev_range"],
                                          params["configurable_vlan"]))
    elseif  nil ~= params["deleted_vlan"] then
        result = self:setDevVlanRange(makeNestedTable(self["dev_range"],
                                                      params["deleted_vlan"]))
    elseif  nil ~= self["dev_range"]        then
        result = self:setDevAllVlanRange(self["dev_range"])
    elseif true == alternative        then
        result                = 0
        no_need_alternative   = false
    else
        result                = 1
        error_message         = "Such kind of device/vlan range is not supported."
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end


-- ************************************************************************
---
--  Command_Data:initInterfaceDevVlanRange
--        @description  dev/vlan range of command execution data
--                      object according to initializes interface range
--
--        @usage __global           - __global["ifRange"]: iterface range
--                                    dev/vlan table
--
--        @return       operation succed
--
function Command_Data:initInterfaceDevVlanRange()
    self["dev_vlan_range"] = getGlobal("ifRange")

    self:checkIterfaceType("vlan")

    return 0
end

-- ************************************************************************
---
--  Command_Data:initInterfaceVlanIterator
--        @description  initializes interface vlan iterator
--
--        @usage __global       - __global["ifRange"]: iterface dev/vlan
--                                table
--
--        @return       operation successed
--
function Command_Data:initInterfaceVlanIterator()
    self["dev_vlan_range"] = getGlobal("ifRange")

    self:checkIterfaceType("vlan")

    return 0
end


-- ************************************************************************
---
--  Command_Data:initDevVlanInfoIterator
--        @description  initializes iterator over vlan info of dev/vlan
--                      pairs in command execution data object.
--                      Requires:
--                          command_data:initInterfaceDeviceRange()
--
--
--        @usage __global       - __global["dev_vlan_range"]: dev/vlan
--                                range
--
--        @return       operation execution status
--
function Command_Data:initDevVlanInfoIterator()
    local result, values, error_message
    local index, devId
    local vlans, vlanId

    result  = 0

    self["dev_vlan_info_range"] = self:getEmptyDeviceNestedTable()

    for devId, vlans in pairs(self["dev_vlan_range"]) do
        for index, vlanId in pairs(vlans) do
            result, value   = vlan_info_get(devId, vlanId)
            if 0 == result  then
                table.insert(self["dev_vlan_info_range"][devId],
                             { ["vlanId"] = vlanId, ["info"] = value })
            else
                error_message   = value
                break
            end
        end

        if 0 ~= result then
            break
        end
    end

    if 0 ~= result then
        self:setFailGlobalAndLocalStatus()
        self:addError(error_message)
    end

    return result
end

-- ************************************************************************
---
--  Command_Data:ipv6_to_sip_or_dip
--        @description  Covert ipv6 address to 4byte sip / dip
--                      pairs in command execution data object.
--                      Requires:
--                          command_data:initInterfaceDeviceRange()
--
--
--        @param  ipv6  ipv6 address
--
--        @return       4 byte array
--
function Command_Data:ipv6_to_sip_or_dip(ipv6)
--[[
ipv6={
    addr={
        8592, 400, 0, 0,
        0, 0, 0, 0 },
    isBroadcast=false,
    isUnicast=false,
    isMulticast=false,
    string="2190:0190:0:0:0:0:0:0"
}
--]]
    -- now return bytes 12,13,14,15
    return {
        [0] = bit_and(bit_shr(ipv6.addr[7],8),0x0ff),
        [1] = bit_and(ipv6.addr[7],0x0ff),
        [2] = bit_and(bit_shr(ipv6.addr[8],8),0x0ff),
        [3] = bit_and(ipv6.addr[8],0x0ff)
    }
end

-- ************************************************************************
---
--  Command_Data:initAllDeviceMacEntryIterator
--        @description  sets mac entry according to all_deveice params
--
--        @param params         - params["static_dynamic"]: static
--                                property, could be irrelevant;
--                                params["static_dynamic_all"]: static
--                                entry property,could be irrelevant;
--                                params["mac"]: mac addresses filtering
--                                properties, could be irrelevant;
--                                params["ip_mac"]: ip/mac addresses
--                                filtering properties, could be
--                                irrelevant;
--                                params["vlan"]: vlan Id, could be
--                                irrelevant;
--                                params["ethernet"]: interface dev/port
--                                pair, could be irrelevant;
--                                params["port-channel"]: trunk id,could be
--                                irrelevant;
--                                params["mac-address"]: mac-address, could
--                                be irrelevant;
--                                params["mac-address-bc-mc"]: mac-address,
--                                could be irrelevant;
--                                params["ipv4"]: ip v4, could be
--                                irrelevant;
--                                params["ipv4_mc"]: multicast ip v4
--                                address, could be irrelevant (related to
--                                params["sip"] or params["dip"]);
--                                params["ipv4_dip"]: ip v4 destination ip
--                                address, could be irrelevant;
--                                params["ipv6"]: ip v6, could be
--                                irrelevant;
--                                params["ipv6_mc"]: multicast ip v6
--                                address, could be irrelevant (related to
--                                params["sip"] or params["dip"]);
--                                params["dip"]: multicast dip property,
--                                could be irrelevant (related to
--                                params["ipv4_mc"] or params["ipv6_mc"]);
--                                params["sip"]: multicast sip property,
--                                could be irrelevant (related to
--                                params["ipv6_mc"] or params["ipv6_mc"]);
--                                params["multicast_mac_address"]:
--                                multicast mac address, could be
--                                irrelevent;
--                                params["multicast_ip_address"]: multicast
--                                address, could specify multicast ipv4 or
--                                ipv6, could be irrelevent;
--                                params["multicast_address"]: multicast
--                                address, could specify multicast mac-
--                                address, multicast ipv4 or ipv6, could be
--                                irrelevent;
--                                params["ipv6_dip"]: ip v6 destination ip
--                                address, could be irrelevant;
--                                params["source_ip"]: ipv4 or ipv6 source
--                                address;
--                                params["skipped"]: skipping property,
--                                could be irrelevant;
--                                params["all_device"]: all or given
--                                devices iterating property, could be
--                                irrelevant
--        @param filter         - predefined mac entry filter
--        @param alternative    - alternative interface behavior checking
--                                flag
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initAllDeviceMacEntryIterator(params, filter, alternative)
    local result, values, error_message
    local index, devNum
    local entry_index
    local all_devices = wrlDevList()
    local no_need_alternative
    local static_dynamic, filter_interface_type
    local vid1, entryTypeMac, entryTypeIpv4Mc, entryTypeIpv6Mc


    -- printMsg("DBG:initAllDeviceMacEntryIterator++++++++++++++++++++++++++++\n")
    -- printMsg("params=".. to_string(params))
    -- printMsg("filter=".. to_string(filter))
    -- printMsg("alternate=".. to_string(alternate))
    -- printMsg("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n")

    self["mac_filter"] = nil;
    vid1 = nil;
    if not vid1 and params["vid1"] then
        vid1 = params["vid1"]
    end
    if type(filter) == "table" and not vid1 and filter["vid1"] then
        vid1 = params["vid1"]
    end

    if vid1 then
        entryTypeMac    = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E"
        entryTypeIpv4Mc = "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E"
        entryTypeIpv6Mc = "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E"
    else
        entryTypeMac    = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E"
        entryTypeIpv4Mc = "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E"
        entryTypeIpv6Mc = "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E"
    end

    self:addToMacEntryFilter(
        { ["key"] = { ["vid1"] = vid1 }});

    result = 0

    if "table" == type(filter)  then
        if "table" == type(filter["dstInterface"])  then
            if nil ~= filter["dstInterface"]["type"]    then
                filter_interface_type = filter["dstInterface"]["type"]
            end
        end
    end

    self:addToMacEntryFilter(filter)

    if     nil ~= params["static_dynamic"] then
        static_dynamic = params["static_dynamic"]
    else
        static_dynamic = params["static_dynamic_all"]
    end

    if     nil ~= static_dynamic then
        if     "static"  == static_dynamic then
            self:addToMacEntryFilter({["isStatic"] = true})
        elseif "dynamic" == static_dynamic then
            self:addToMacEntryFilter({["isStatic"] = false})
        elseif "all"     ~= static_dynamic then
            result        = 1
            error_message = "Wrong static/dynamic identifier."
        end
    end

    if     (nil ~= params["mac"]) or
           ("mac" == params["ip_mac"])  then
        self:addToMacEntryFilter(
            { ["key"] =
                { ["entryType"] = entryTypeMac}})
    elseif "ip" == params["ip_mac"]     then
        self:addToMacEntryFilter(
            { ["key"]   =
                { ["entryType"] = { entryTypeIpv4Mc,
                                    entryTypeIpv6Mc }}})
    end

    if nil ~= params["vlan"] then
        self:addToMacEntryFilter(
            { ["key"] =
                { ["entryType"] = entryTypeMac,
                  ["key"] = {
                      ["macVlan"] = {
                          ["vlanId"]    = params["vlan"]}} }})
    end

    if nil ~= params["vlan_range"] then
        self:addToMacEntryFilter({ ["key"] = { ["key"] = {["macVlan"] = {["vlanId"] = params["vlan_range"]}} }})
    end

    if (    nil == filter_interface_type)                       or
       ("table" == type(filter_interface_type))                 then
    if     nil ~= params["ethernet"] then
        self:addToMacEntryFilter(
            { ["dstInterface"] =
                {["type"]    = "CPSS_INTERFACE_PORT_E",
                 ["devPort"] = {["devNum"] = params["ethernet"]["devId"],
                                ["portNum"] = params["ethernet"]["portNum"]}}})
    elseif nil ~= params["port-channel"] then
        self:addToMacEntryFilter(
            { ["dstInterface"] =
                {["type"]    = "CPSS_INTERFACE_TRUNK_E",
                 ["trunkId"] = params["port-channel"]}})
        elseif "table" == type(filter_interface_type)   then
            self:addToMacEntryFilter(
                { ["dstInterface"] =
                    { ["type"]    = filter_interface_type  }})
    end
    end

    if     (nil ~= params["mac-address"])                                   or
           (nil ~= params["mac-address-bc-mc"])                             or
           (nil ~= params["multicast_mac_address"])                         or
           ((nil ~= params["multicast_address"])                            and
            ("mac-address-bc-mc"
                == params["multicast_address"]["compound"]))    then
        local mac_address = getFirstDefinedValue(params["mac-address"],
                                                 params["mac-address-bc-mc"],
                                                 params["multicast_mac_address"],
                                                 params["multicast_address"])
        self:addToMacEntryFilter(
            { ["key"]  = {["entryType"]  = entryTypeMac,
                          ["key"] = {
                            ["macVlan"] = {
                              ["macAddr"]  = mac_address }} }})



    elseif (nil ~= params["ipv4"])                                          or
           (nil ~= params["ipv4_mc"])                                       then
        local ipv4 = getFirstDefinedValue(params["ipv4"], params["ipv4_mc"])
        if     nil ~= params["sip"]  then
            self:addToMacEntryFilter(
                { ["key"]   = { ["entryType"]   = entryTypeIpv4Mc,
                                ["key"] = {
                                  ["ipMcast"] = {
                                    ["sip"]         = ipv4,
                                    ["dip"]         = "0.0.0.0" }} }})

        elseif nil ~= params["dip"]  then
            self:addToMacEntryFilter(
                { ["key"]   = { ["entryType"]   = entryTypeIpv4Mc,
                                ["key"] = {
                                  ["ipMcast"] = {
                                    ["sip"]         = "0.0.0.0",
                                    ["dip"]         = ipv4  }} }})
        end
    elseif (nil ~= params["ipv6"])                                          or
           (nil ~= params["ipv6_mc"])                                       then
        local ipv6 = getFirstDefinedValue(params["ipv6"], params["ipv6_mc"])
        if     nil ~= params["sip"]  then
            self:addToMacEntryFilter(
                { ["key"]   = { ["entryType"]   = entryTypeIpv6Mc,
                                ["key"] = {
                                  ["ipMcast"] = {
                                    ["sip"]         = self:ipv6_to_sip_or_dip(ipv6),
                                    ["dip"]         = {[0]=0,0,0,0} }} }})
        elseif nil ~= params["dip"]  then
            self:addToMacEntryFilter(
                { ["key"]   = { ["entryType"]   = entryTypeIpv6Mc,
                                ["key"] = {
                                  ["ipMcast"] = {
                                    ["sip"]         = {[0]=0,0,0,0},
                                    ["dip"]         = self:ipv6_to_sip_or_dip(ipv6) }} }})
        end
    end

    if     (nil ~= params["source_ip"])                                     and
           ("ipv4" == params["source_ip"]["compound"])                      then
        self:addToMacEntryFilter(
            { ["key"]   = { ["entryType"]   = entryTypeIpv4Mc,
                            ["key"] = {
                              ["ipMcast"] = {
                                ["sip"]     = params["source_ip"] }} }})
    elseif (nil ~= params["source_ip"])                                     and
           ("ipv6" == params["source_ip"]["compound"])                      then
        self:addToMacEntryFilter(
            { ["key"]   = { ["entryType"]   = entryTypeIpv6Mc,
                            ["key"] = {
                              ["ipMcast"] = {
                                ["sip"]     = params["source_ip"] }} }})
    end

    if     (nil ~= params["ipv4_dip"])                                      or
           ((nil ~= params["multicast_ip_address"])                         and
            ("ipv4_mc" == params["multicast_ip_address"]["compound"]))      or
           ((nil ~= params["multicast_address"])                            and
            ("ipv4_mc" == params["multicast_address"]["compound"]))         then
        local dip = getFirstDefinedValue(params["ipv4_dip"],
                                         params["multicast_ip_address"],
                                         params["multicast_address"])
        self:addToMacEntryFilter(
            { ["key"]   = { ["entryType"]   = entryTypeIpv4Mc,
                            ["key"] = {
                              ["ipMcast"] = {
                                ["dip"]         = dip }} }})
    elseif  nil ~= params["ipv6_dip"]                                       or
           ((nil ~= params["multicast_ip_address"])                         and
            ("ipv6_mc" == params["multicast_ip_address"]["compound"]))      or
           ((nil ~= params["multicast_address"])                            and
            ("ipv6_mc" == params["multicast_address"]["compound"]))         then
        local dip = getFirstDefinedValue(params["ipv6_dip"],
                                         params["multicast_ip_address"],
                                         params["multicast_address"])
        self:addToMacEntryFilter(
            { ["key"]   = { ["entryType"]   = entryTypeIpv6Mc,
                            ["key"] = {
                              ["ipMcast"] = {
                                ["dip"]         = dip }} }})
        -- add into ["dip"] [1] = byte12, [2] = byte13, [3] = byte14, [4] = byte15
        self:addToMacEntryFilter(
            {["key"] = {["key"] = {["ipMcast"] = {["dip"] = self:ipv6_to_sip_or_dip(dip) }} }})
    end

    if     nil ~= params["skipped"] then
        self["mac_aged"] = true
    end

    if   "all" == params["all_device"] then
        result = self:setAllDevicesMacEntryIterator()
    elseif nil ~= params["all_device"] then
        result = self:setMacEntryIterator({params["all_device"]})
    elseif nil ~= self["dev_range"] then
        result = self:setMacEntryIterator(self["dev_range"])
    elseif true == alternative then
        no_need_alternative   = false
    else
        result                = 1
        error_message         = "Such kind of device range is not supported."
        no_need_alternative   = true
    end

    if 0 ~= result then
        self:setFailStatus()
        self:addError(error_message)
    end

    return no_need_alternative
end

-- gets string of Mac entry contents for using as key in DBs
-- when fails return pair of nil and error message string
function command_data_getMacEntryFilterKeyString(key)
local res = "";
local entryType;
    local isIpv4, isIpv6;

    isIpv4 = false;
    isIpv6 = false;
    if key["entryType"] == "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E" then
        entryType = "MAC_VID1";
    elseif key["entryType"] == "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E" then
        entryType = "IPV4_MC_VID1";
        isIpv4 = true;
    elseif key["entryType"] == "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E" then
        entryType = "IPV6_MC_VID1";
        isIpv6 = true;
    elseif key["entryType"] == "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E" then
        entryType = "MAC";
    elseif key["entryType"] == "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E" then
        entryType = "IPV4_MC";
        isIpv4 = true;
    elseif key["entryType"] == "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E" then
        entryType = "IPV6_MC";
        isIpv6 = true;
    else
        return nil, "unsupported FDB key type";
    end

    res = entryType;
    if ((key["key"] or {})["macVlan"] or {})["vlanId"] then
        res = res .. ",vid=" .. tostring(key["key"]["macVlan"]["vlanId"]);
    end
    if key["vid1"] then
        local vid1_num = tonumber(key["vid1"]);
        if vid1_num and (vid1_num ~= 0)then
            res = res .. ",vid1=" .. tostring(vid1_num);
        end
    end
    if ((key["key"] or {})["macVlan"] or {})["macAddr"] then
        if type(key["key"]["macVlan"]["macAddr"]) == "string" then
            res = res .. ",mac=" .. string.lower(key["key"]["macVlan"]["macAddr"]);
        elseif type(key["key"]["macVlan"]["macAddr"]) == "table" then
            res = res .. ",mac=" .. string.lower(key["key"]["macVlan"]["macAddr"].string);
        end
    end
    if ((key["key"] or {})["ipMcast"] or {})["sip"] then
        if type(key["key"]["ipMcast"]["sip"]) == "string" then
            res = res .. ",sip=" .. string.lower(key["key"]["ipMcast"]["sip"]);
        elseif isIpv4 then
            res = res .. ",sip=" .. string.lower(key["key"]["ipMcast"]["sip"].string);
        elseif isIpv6 then
            res = res .. ",sip=" .. string.format(
                "::%02X%02X:%02X%02X", key["key"]["ipMcast"]["sip"][0],
                key["key"]["ipMcast"]["sip"][1], key["key"]["ipMcast"]["sip"][2], key["key"]["ipMcast"]["sip"][3]);
        else
            return nil, "unsuported sip";
        end
    end
    if ((key["key"] or {})["ipMcast"] or {})["dip"] then
        if type(key["key"]["ipMcast"]["dip"]) == "string" then
            res = res .. ",dip=" .. string.lower(key["key"]["ipMcast"]["dip"]);
        elseif isIpv4 then
            res = res .. ",dip=" .. string.lower(key["key"]["ipMcast"]["dip"].string);
        elseif isIpv6 then
            res = res .. ",dip=" .. string.format(
                "::%02X%02X:%02X%02X", key["key"]["ipMcast"]["dip"][0],
                key["key"]["ipMcast"]["dip"][1], key["key"]["ipMcast"]["dip"][2], key["key"]["ipMcast"]["dip"][3]);
        else
            return nil, "unsuported dip";
        end
    end

    return res;
end

function command_data_getMacEntryFilterKeyStringToTable(key_string)
    local ret = {};
    local entryType, k1;
    -- first word is a type
    for k in string.gmatch(key_string, "([%w_]+)") do
        entryType = k;
        break;
    end
    if entryType == "MAC_VID1" then
        ret.entryType = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E";
    elseif entryType == "IPV4_MC_VID1" then
        ret.entryType = "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E";
    elseif entryType == "IPV6_MC_VID1" then
        ret.entryType = "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E";
    elseif entryType == "MAC" then
        ret.entryType = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E";
    elseif entryType == "IPV4_MC" then
        ret.entryType = "CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E";
    elseif entryType == "IPV6_MC" then
        ret.entryType = "CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E";
    end

    -- pairs name=value (value can contain HEX digits, point, semicolumns)
    for k, v in string.gmatch(key_string, ",(%w+)=([%x:\\.]+)") do
        if k == "mac" then
            local t = {}
            t["key"]["macVlan"]["macAddr"]=v
            mergeTablesRightJoin(ret, t)
        elseif k == "vid"  then
            local t = {}
            t["key"]["macVlan"]["vlanId"]=v
            mergeTablesRightJoin(ret, t)
        else
            k1 = k; -- vid1, sip, dip
            ret[k1] = v;
        end
    end
    return ret;
end

-- bacward compatible VlanId+macAddress key sting
function command_data_getVidMacKeyString(vlanId, macString)
    local key = {
        ["entryType"] = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
        ["key"]= {["macVlan"]={["vlanId"] = vlanId}}
    };
    if type(macString) == "string" then
        key["key"]["macVlan"]["macAddr"] = {string = macString};
    end
    return command_data_getMacEntryFilterKeyString(key);
end

-- Key String caculation by ["mac_filter"]
function Command_Data:getMacEntryFilterKeyString()
    local key;
    if (not self["mac_filter"]) or
        (type(self["mac_filter"].key) ~= "table") then
        return nil, "no filter or key in filter";
    end
    key = self["mac_filter"].key;
    return command_data_getMacEntryFilterKeyString(key);
end

-- ************************************************************************
---
--  Command_Data:initMacEntry
--        @description  initialize mac entry according to entry command
--                      params
--
--        @param params         - params["mac-address"]: mac address;
--                                params["vlan"]: vlan number;
--                                params["ethernet"]["devId"]: device number
--                                of ethernet port;
--                                params["ethernet"]["portNum"]: port number
--                                of ethernet port
--        @param pattern        - predefined mac entry pattern
--
--        @return       true is success or alternative != true otherwise
--                      false
--
function Command_Data:initMacEntry(params, pattern)
    local result, values
    local vid1, entryType

    vid1 = nil;
    if not vid1 and params["vid1"] then
        vid1 = params["vid1"]
    end
    if type(pattern) == "table" and not vid1 and pattern["vid1"] then
        vid1 = params["vid1"]
    end

    if vid1 then
        entryType = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E"
    else
        entryType = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E"
    end

    self["mac_entry"] =
        mergeTablesRightJoin(
            { key = { entryType = entryType, vid1 = vid1 }},
            pattern)
    self["mac_entry"]["dstInterface"] = get_default_interface_info_stc()

    if nil ~= params["mac-address"] then
        self["mac_entry"]["key"]["key"] = {["macVlan"] = {["macAddr"] = params["mac-address"]}}
    end

    if (nil ~= params["vlan"]) or (nil ~= params["vid"])    then
        local vlan = getFirstDefinedValue(params["vlan"], params["vid"])
        self["mac_entry"]["key"]["key"] = {["macVlan"] = {["vlanId"] = vlan}}
    end

    if     nil ~= params["ethernet"] then
        local hwDevNum, hwPortNum

        hwDevNum, hwPortNum =
            self:getHWDevicePort(params["ethernet"]["devId"],
                                 params["ethernet"]["portNum"],
                                 (nil ~= params["interface_port_channel_remote"]))

        self["mac_entry"]["dstInterface"]["type"]               =
                                                        "CPSS_INTERFACE_PORT_E"
        self["mac_entry"]["dstInterface"]["devPort"]["devNum"]  =   hwDevNum
        self["mac_entry"]["dstInterface"]["devPort"]["portNum"] =   hwPortNum
    elseif nil ~= params["eport"] then

        local hwDevNum, hwPortNum

        hwDevNum, hwPortNum =
        self:getHWDevicePort(params["eport"]["devId"],
            params["eport"]["portNum"],
            false)

        self["mac_entry"]["dstInterface"]["type"]               =
        "CPSS_INTERFACE_PORT_E"
        self["mac_entry"]["dstInterface"]["devPort"]["devNum"]  =   hwDevNum
        self["mac_entry"]["dstInterface"]["devPort"]["portNum"] =   hwPortNum
    elseif nil ~= params["port-channel"] then
        self["mac_entry"]["dstInterface"]["type"]               =
                                                        "CPSS_INTERFACE_TRUNK_E"
        self["mac_entry"]["dstInterface"]["trunkId"]            =
                                                          params["port-channel"]
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:initPriorityData
--        @description  initialization of priority data of command
--                      execution data object
--
--        @param min_priority   - minumum value of priority range (could be
--                                irrelevant)
--        @param max_priority   - maximum value of priority range (could be
--                                irrelevant)
--
--        @return       operation succed
--
function Command_Data:initPriorityData(min_priority, max_priority)
    if nil == min_priority  then
        min_priority    = 0
    end

    if nil == max_priority  then
        max_priority    = 7
    end

    self:setPriorityData(min_priority, max_priority)

    return 0
end


-- ************************************************************************
---
--  Command_Data:initDevicesPriorityDataRange
--        @description  initialization of dev/priority data range of
--                      command execution data object
--
--        @param min_priority   - minumum value of priority range (could be
--                                irrelevant)
--        @param max_priority   - maximum value of priority range (could be
--                                irrelevant)
--
--        @return       operation succed
--
function Command_Data:initDevicesPriorityDataRange(min_priority, max_priority)
    if nil == min_priority  then
        min_priority    = 0
    end

    if nil == max_priority  then
        max_priority    = 7
    end

    self:setDevicesPriorityDataRange(self["dev_range"], min_priority,
                                     max_priority)

    return 0
end





-- ************************************************************************
---
--  Command_Data:setEmptyRouterArpEntries
--        @description  setting of enty ARP entries indexes of command
--                      execution data object according to given device
--                      range;
--
--        @param dev_range     - device tange
--
--        @return       operation succeeded
--
function Command_Data:setEmptyRouterArpEntries(dev_range)
    local index, devNum

    self["dev_arp_entries"] = {}

    for index, devNum in pairs(dev_range) do
        self["dev_arp_entries"][devNum] = {}
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setDevicesRouterArpEntriesIndexes
--        @description  setting of the router ARP entries indexes of
--                      command execution data object according to given
--                      device range arp indexes range;
--                      Requirets:
--                          setEmptyRouterArpEntries(dev_range).
--
--        @param dev_range     - device tange
--        @param router_arp_indexes
--                             - router arp indexes array
--
--        @return       operation succeeded
--
function Command_Data:setDevicesRouterArpEntriesIndexes(dev_range, router_arp_indexes)
    local index, devNum

    table.sort(router_arp_indexes)

    for index, devNum in pairs(dev_range) do
        self["dev_arp_entries"][devNum]["arp_indexes"] = router_arp_indexes
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:setDevicesRouterArpEntriesCounts
--        @description  setting of the router ARP entries counts of command
--                      execution data object according to given device
--                      range;
--                      Requirets:
--                          setEmptyRouterArpEntries(dev_range).
--
--        @param dev_range     - device tange
--
--        @return       operation succeeded
--
function Command_Data:setDevicesRouterArpEntriesCounts(dev_range)
    local result, values
    local index, devNum

    for index, devNum in pairs(dev_range) do
        result, values = wrlCpssDxChMaximumRouterArpIndex(devNum)
        if     0 == result then
            self["dev_arp_entries"][devNum]["max_index"] = values
        elseif 0 ~= result then
            command_data:setFailStatus()
            command_data:addError("Error at maximum router ARP index getting " ..
                                  "of device %d: %s.", devNum, values)
        end
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setEmptyIpUcRouteEntries
--        @description  setting of ip unicast route entries indexes of
--                      command execution data object according to given
--                      device range
--
--        @param dev_range     - device tange
--
--        @return       operation succeeded
--
function Command_Data:setEmptyIpUcRouteEntries(dev_range)
    local index, devNum

    self["dev_ip_uc_route_entries"] = {}

    for index, devNum in pairs(dev_range) do
        self["dev_ip_uc_route_entries"][devNum] = {}
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setDevicesIpUcRouteEntriesCounts
--        @description  setting of the ip unicast route entries counts of
--                      command execution data object according to given
--                      device range;
--                      Requirets:
--                          setEmptyIpUcRouteEntries(dev_range).
--
--        @param dev_range     - device tange
--
--        @return       operation succeeded
--
function Command_Data:setDevicesIpUcRouteEntriesCounts(dev_range)
    local result, values
    local index, devNum

    for index, devNum in pairs(dev_range) do
        result, values = wrlCpssDxChMaximumRouterNextHopEntryIndex(devNum)
        if     0 == result then
            self["dev_ip_uc_route_entries"][devNum]["max_index"] = values
        elseif 0 ~= result then
            command_data:setFailStatus()
            command_data:addError("Error at maximum router next hop entry " ..
                                  "index getting of device %d: %s.", devNum,
                                  values)
        end
    end

    return 0
end



-- ************************************************************************
---
--  Command_Data:setDevicesMirrorAnalyzerEntriesCounts
--        @description  setting of the mirror analyzer entries counts of
--                      command execution data object according to given
--                      device range;
--                      Requirets:
--                          setEmptyMirrorAnalyzerEntries(dev_range).
--
--        @param dev_range     - device tange
--
--        @return       operation succeeded
--
function Command_Data:setDevicesMirrorAnalyzerEntriesCounts(dev_range)
    local result, values
    local index, devNum

    for index, devNum in pairs(dev_range) do
        result, values = wrlCpssDxChMaximumMirrorAnalyzerIndex(devNum)
        if     0 == result then
            self["mirror_analyzer_entries"][devNum]["max_index"] = values
        elseif 0 ~= result then
            command_data:setFailStatus()
            command_data:addError("Error at maximum mirror analyzer index " ..
                                  "getting of device %d: %s.", devNum, values)
        end
    end

    return 0
end



-- ************************************************************************
---
--  Command_Data:setDevicePortRangeIterator
--        @description  initialize iterator over the dev/port pairs to go
--                      through given device port range
--
--        @param devId          - device
--        @param trunkId        - The trunk Id
--
--        @return       operation succed or operation error code and error
--                      message
--
function Command_Data:setDevicePortRangeIterator(devId, trunkId)
    local result, values = get_trunk_device_port_list(trunkId, {devId})

    if 0 == result then
        self["dev_port_range"] = values

        return 0
    else
        return result, values
    end
end


-- ************************************************************************
---
--  Command_Data:setDevTrunkRange
--        @description  set's device/trunk range in command execution data
--                      object to given range
--
--        @param range          - given range
--
--        @return       operation succeeded
--
function Command_Data:setDevTrunkRange(range)
    self["dev_trunk_range"] = range

    return 0
end


-- ************************************************************************
---
--  Command_Data:setDevTrunkWithPortsRange
--        @description  set's device/trunk range in command execution data
--                      object to ranke with valid trunks, that contains
--                      ports
--
--        @param dev_range     - device range
--
--        @return       operation succeeded
--
function Command_Data:setDevTrunkWithPortsRange(dev_range)
    local result, values
    local trunkId, maximumTrunkId
    local index, devNum

    self["dev_trunk_range"]   = self:getEmptyDeviceNestedTable()

    for index, devNum in pairs(dev_range) do
        maximumTrunkId  = system_capability_get_table_size(devNum, "TRUNK")

        for trunkId = 1, maximumTrunkId do
            if does_trunk_contains_ports(devNum, trunkId)   then
                table.insert(self["dev_trunk_range"][devNum], trunkId)
            end
        end
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setVlanRange
--        @description  set's vlans range in command execution data object
---                     to given range
--
--        @param range          - given range
--
--        @return       operation succeeded
--
function Command_Data:setVlanRange(range)
    self["vlan_range"] = range

    return 0
end


-- ************************************************************************
---
--  Command_Data:setDevVlanRange
--        @description  set's dev/vlans range in command execution data
---                     object to given range
--
--        @param range          - given range
--
--        @return       operation succeeded
--
function Command_Data:setDevVlanRange(range)
    self["dev_vlan_range"] = range

    return 0
end


-- ************************************************************************
---
--  Command_Data:setDevAllAvaibleVlanRange
--        @description  set's dev/vlans range in command execution data
--                      object to all valid and invalid vlans of given
--                      devices
--
--        @param dev_range      - device range
--
--        @return       operation succeeded
--
function Command_Data:setDevAllAvaibleVlanRange(dev_range)
    local index, devNum, vlanId, maxVlanId

    self["dev_vlan_range"]  = {}

    for index, devNum in pairs(dev_range) do
        maxVlanId = system_capability_get_table_size(devNum, "VLAN") - 1
        self["dev_vlan_range"][devNum] = {}

        for vlanId = 1, maxVlanId do
            table.insert(self["dev_vlan_range"][devNum], vlanId)
        end
    end

    return 0
end



-- ************************************************************************
---
--  Command_Data:setDevAllVlanRange
--        @description  set's dev/vlans range in command execution data
---                     object to all avaible vlans of given devices
--
--        @param dev_range      - device range
--
--        @return       operation succeeded
--
function Command_Data:setDevAllVlanRange(dev_range)
    local result, values
    local index, devNum

    self["dev_vlan_range"]  = {}

    for index, devNum in pairs(dev_range) do
        result, values = wrlCpssDxChValidVlansGet(devNum, 0)
        if     0x10 == result then
            self:setFailStatus()
            self:addWarning("Getting of vlan entries information is not " ..
                            "allowed on device %d.", devNum)
        elseif    0 ~= result then
            self:setFailStatus()
            self:addError("Error at getting of valid static vlan entries in " ..
                          "device %d: %s", devNum, returnCodes[result])
            break
        end

        self["dev_vlan_range"][devNum] = values
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setMacEntryIterator
--        @description  initialize iterator over mac addresses table
--                      entries of given devices
--
--        @param devices        - iterated devices array
--        @param filter         - mac entry on which valid fields iterated
--                                mac entries will be filtered (could be
--                                nill)
--
--        @return       operation succed or error code
--
function Command_Data:setMacEntryIterator(devices, filter)
    local result, values
    local index, devNum
    local entry_index

    result                  = 0
    self["dev_mac_entries"] = {}

    for index, devNum in pairs(devices) do
        result, values = wrlCpssDxChBrgFdbTableSizeGet(devNum)

        if     0 == result then
            self["dev_mac_entries"][devNum] = {['count'] = values}
        elseif 0 ~= result then
            self:setFailStatus()
            self:addError(values)
            break
        end
    end

    return result
end


-- ************************************************************************
---
--  Command_Data:setAllDevicesMacEntryIterator
--        @description  initialize iterator over mac addresses table
--                      entries of all avaible devices
--
--        @param filter         - mac entry on which valid fields iterated
--                                mac entries  will be filtered (could be
--                                null)
--
--        @return       operation succed or error code
--
function Command_Data:setAllDevicesMacEntryIterator(filter)
    local all_devices = wrlDevList()

    return self:setMacEntryIterator(all_devices, filter)
end


-- ************************************************************************
---
--  Command_Data:setResultOnDevicesCount
--        @description  set's result in command execution data object to
--                      given value, if devices count more than 0 otherwise
--                      to alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setResultOnDevicesCount(value, alternative)
    if     0 < self["devices_count"] then
        self["result"] = value
    elseif nil ~= alternative then
        self["result"] = alternative
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:setResultOnPortsAndTrunksCount
--        @description  set's result in command execution data object to
--                      given value, if ports and trunks aggregate count
--                      more than 0 otherwise to alternative, if it is
--                      relevant
--
--        @return       operation succed
--
function Command_Data:setResultOnPortsAndTrunksCount(value, alternative)
    if     0 < self["ports_count"] + self["trunks_count"] then
        self["result"] = value
    elseif nil ~= alternative then
        self["result"] = alternative
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setResultOnVlansCount
--        @description  set's result in command execution data object to
--                      given value, if vlans count more than 0 otherwise
--                      to alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setResultOnVlansCount(value, alternative)
    if     0 < self["vlans_count"] then
        self["result"] = value
    elseif nil ~= alternative then
        self["result"] = alternative
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:setResultOnTrunksCount
--        @description  set's result in command execution data object to
--                      given value, if trunks count more than 0 otherwise
--                      to alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setResultOnTrunksCount(value, alternative)
    if     0 < self["trunks_count"] then
        self["result"] = value
    elseif nil ~= alternative then
        self["result"] = alternative
    end

    return 0
end

--  Command_Data:setResultOnVidxCount
--        @description  set's result in command execution data object to
--                      given value, if vidix count more than 0 otherwise
--                      to alternative, if it is relevant
--
--        @return       operation succed
--
function Command_Data:setResultOnVidxCount(value, alternative)
    if     0 < self["vidxes_count"] then
        self["result"] = value
    elseif nil ~= alternative then
        self["result"] = alternative
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:setResultStr
--        @description  sets result in command execution data object to
--                      concatenation of string header, body and footer, if
--                      it is relevant
--
--        @return       operation succed
--
function Command_Data:setResultStr(header, body, footer)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    self["result"] = body
end


-- ************************************************************************
---
--  Command_Data:setResultStrOnDevicesCount
--        @description  sets result in command execution data object to
--                      concatenation of string header, body and footer, of
--                      devices count more than 0 otherwise to alternative,
--                      if it is relevant footer could be irrelevant
--
--        @return       operation succed
--
function Command_Data:setResultStrOnDevicesCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    self:setResultOnDevicesCount(body, alternative)

    return 0
end

--  Command_Data:setResultStrOnPortsAndTrunksCount
--        @description  sets result in command execution data object to
--                      concatenation of string header, body and footer, of
--                      ports and trunks aggregate count more than 0
--                      otherwise to alternative, if it is relevant footer
--                      could be irrelevant
--
--        @return       operation succed
--
function Command_Data:setResultStrOnPortsAndTrunksCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    self:setResultOnPortsAndTrunksCount(body, alternative)

    return 0
end


-- ************************************************************************
---
--  Command_Data:setResultStrOnVlansCount
--        @description  sets result in command execution data object to
--                      concatenation of string header, body and footer, of
--                      vlans count more than 0 otherwise to alternative,
--                      if it is relevant footer could be irrelevant
--
--        @return       operation succed
--
function Command_Data:setResultStrOnVlansCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    self:setResultOnVlansCount(body, alternative)

    return 0
end


-- ************************************************************************
---
--  Command_Data:setResultStrOnTrunksCount
--        @description  sets result in command execution data object to
--                      concatenation of string header, body and footer, of
--                      trunks count more than 0 otherwise to alternative,
--                      if it is relevant footer could be irrelevant
--
--        @return       operation succeed
--
function Command_Data:setResultStrOnTrunksCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    self:setResultOnTrunksCount(body, alternative)

    return 0
end

-- ************************************************************************
---
--  Command_Data:setPriorityData
--        @description  setting of priority data in command execution data
--                      object
--
--        @param min_priority   - minumum value of priority range (could be
--                                irrelevant)
--        @param max_priority   - maximum value of priority range (could be
--                                irrelevant)
--
--        @return       operation succeeded
--
function Command_Data:setPriorityData(min_priority, max_priority)
    self["priority"]        = { }

    self["priority"]["min"] = min_priority

    self["priority"]["max"] = max_priority
end


-- ************************************************************************
---
--  Command_Data:setDevicesPriorityDataRange
--        @description  setting of dev/priority data range in command
--                      execution data object
--
--        @param dev_range      - device range
--        @param min_priority   - minumum value of priority range (could be
--                                irrelevant)
--        @param max_priority   - maximum value of priority range (could be
--                                irrelevant)
--
--        @return       operation succeeded
--
function Command_Data:setDevicesPriorityDataRange(dev_range, min_priority, max_priority)
    local index, devNum

    self["dev_priority_data"] = { }

    for index, devNum in pairs(dev_range) do
        self["dev_priority_data"][devNum]   = { ["min"] = min_priority,
                                                ["max"] = max_priority  }
    end

    return 0
end


-- ************************************************************************
---
--   Command_Data:checkNotExistedPorts
--        @description  checks invalid port presents in the dev/port pairs
--                      in self["dev_port_range"] list
--
--        @return       count of invalid ports
--
function Command_Data:checkNotExistedPorts()
    local count = 0
    local dev, ports, port

    for dev, ports in pairs(self["dev_port_range"]) do
        for port = 1, #ports do
            if does_port_exist(dev,ports[port]) then
                count = count + 1
            end
        end
    end

    return count
end


-- ************************************************************************
---
--  Command_Data:isOneDeviceInDevRange
--        @description  checks if there is one device in device range
--
--        @return       true if there is one device, otherwise false
--
function Command_Data:isOneDeviceInDevRange()
    if "table" == type(self["dev_range"]) then
        local index, devNum
        local count = 0

        for index, devNum in pairs(self["dev_range"]) do
            count = count + 1

            if 1 < count then
                break
            end
        end

        return 1 == count
    else
        return false
    end
end


-- ************************************************************************
---
--  Command_Data:isOneOrNoDevicesInDevRange
--        @description  checks if there is one device or ni devices in
--                      device range
--
--        @return       true if there is one device, otherwise false
--
function Command_Data:isOneOrNoDevicesInDevRange()
    if "table" == type(self["dev_range"]) then
        local index, devNum
        local count = 0

        for index, devNum in pairs(self["dev_range"]) do
            count = count + 1

            if 1 < count then
                break
            end
        end

        return 1 >= count
    else
        return false
    end
end


-- ************************************************************************
---
--  Command_Data:isNoPortsInDevPortRange
--        @description  checks if there is no porta in dev/port range of
--                      command execution data object
--
--        @usage self               - self["dev_port_range"]: dev/port
--                                    range
--
--        @return       true if there is one port, otherwise false
--
function Command_Data:isNoPortsInDevPortRange()
    if "table" == type(self["dev_port_range"]) then
        local count = 0
        local dev, ports, index, port

        for dev, ports in pairs(self["dev_port_range"]) do
            for index, port in pairs(ports) do
                return false
            end
        end

        return true
    else
        return false
    end
end


-- ************************************************************************
---
--  Command_Data:getValueIfFlagNoIsNil
--        @description  getting of value or its default according to
--                      command no-flag
--
--        @param params             - params["flagNo"]: command no-flag
--        @param value              - given value
--        @param default            - default value
--
--        @return       value if no-flag is nil, otherwise default
--
function Command_Data:getValueIfFlagNoIsNil(params, value, default)
    if nil == params["flagNo"] then
        return value
    else
        return default
    end
end


-- ************************************************************************
---
--  Command_Data:getValueIfSuccessVlans
--        @description  getting of value or its default according to
--                      success processed vlans count
--
--        @param value              - given value
--        @param default            - default value (if threre is no
--                                    success processed entries count),
--                                    could be irrelevant
--
--        @return       given value if success processed vlans count > 0,
--                      otherwise success processed entries count
--
function Command_Data:getValueIfSuccessVlans(value, default)
    if 0 < self["vlans_with_success_count"] then
        return value
    else
        return default
    end
end


-- ************************************************************************
---
--  Command_Data:getValueIfNoSuccessVlans
--        @description  getting of value or its default according to
--                      success processed vlans count
--
--        @param value              - given value
--        @param default            - default value (if threre is no
--                                    success processed entries count),
--                                    could be irrelevant
--
--        @return       given value if success processed vlans
--                      count == 0, otherwise success processed entries
--                      count
--
function Command_Data:getValueIfNoSuccessVlans(value, default)
    if 0 == self["vlans_with_success_count"] then
        return value
    else
        return default
    end
end


-- ************************************************************************
---
--  Command_Data:getValueIfSuccessEntries
--        @description  getting of value or its default according to
--                      success processed entries count
--
--        @param value              - given value
--        @param default            - default value (if threre is no
--                                    success processed entries count),
--                                    could be irrelevant
--
--        @return       given value if success processed entries count > 0,
--                      otherwise success processed entries count
--
function Command_Data:getValueIfSuccessEntries(value, default)
    if 0 < self["entries_with_success_count"] then
        return value
    else
        return default
    end
end


-- ************************************************************************
---
--  Command_Data:getValueIfNoSuccessEntries
--        @description  getting of value or its default according to
--                      success processed entries count
--
--        @param value              - given value
--        @param default            - default value (if threre is no
--                                    success processed entries count),
--                                    could be irrelevant
--
--        @return       given value if success processed entries
--                      count == 0, otherwise success processed entries
--                      count
--
function Command_Data:getValueIfNoSuccessEntries(value, default)
    if 0 == self["entries_with_success_count"] then
        return value
    else
        return default
    end
end


-- ************************************************************************
---
--  Command_Data:getFalseIfFlagNoIsNil
--        @description  getting of bollean value (true or false) according
--                      to command no-flag
--
--        @param params             - params["flagNo"]: command no-flag
--
--        @return       false if no-flag is nil, otherwise true
--
function Command_Data:getFalseIfFlagNoIsNil(params)
    if nil == params["flagNo"] then
        return false
    else
        return true
    end
end

-- ************************************************************************
---
--  Command_Data:getHWDevicePort
--        @description  get's device/port pair in hardware format
--
--        @param devNum             - software device
--        @param portNum            - software port number
--        @param remote            - when true: devNum may be a remote device number
--
--        @return       device port range
--
function Command_Data:getHWDevicePort(devNum, portNum, remote)
    local result, values
    local hwDevNum, hwPortNum

    result, values, hwPortNum =
        device_port_to_hardware_format_convert(devNum, portNum)
    if        0 == result then
        hwDevNum    = values
    elseif 0x10 == result then
        self:setFailStatus()
        self:addError("Device id %d and port number %d " ..
                              "converting is not allowed.", devNum, portNum)
    elseif    true == remote then
    	-- devNum is not a local device, return it as is.
        hwDevNum    = devNum
        hwPortNum    = portNum
    elseif    0 ~= result then
        self:setFailStatus()
        self:addError("Error at device id %d and port number %d " ..
                              "converting: %s.", devNum, portNum, values)
    end

    return hwDevNum, hwPortNum
end


-- ************************************************************************
---
--  Command_Data:getDevicePortRangeIfConditionNil
--        @description  getting of device port range of command execution
--                      data object or default value according to condition
--                      determination
--
--        @param cond               - condition
--        @param default            - default value
--
--        @return       device port range if condition is nil, otherwise
--                      default value
--
function Command_Data:getDevicePortRangeIfConditionNil(cond, default)
    if nil == cond then
        return self["dev_port_range"]
    else
        return default
    end
end


-- ************************************************************************
---
--  Command_Data:getDevicePortRangeIfConditionNotNil
--        @description  getting of device port range of command execution
--                      data object or default value according to condition
--                      determination
--
--        @param cond               - condition
--        @param default            - default value
--
--        @return       device port range if condition is not nil, otherwise
--                      default value
--
function Command_Data:getDevicePortRangeIfConditionNotNil(cond, default)
    if nil ~= cond then
        return self["dev_port_range"]
    else
        return default
    end
end


-- ************************************************************************
---
--  Command_Data:getEmptyDeviceNestedTable
--        @description  get's array with devices from device range of
--                      command execution data object as keys and empty
--                      tables as values
--                      need: initAllAvailableDevicesRange() or
--                            initAllDeviceRange()
--
--        @usage self["dev_range"]  - device range
--
--        @return       nested table with devices as keys and empty tables
--                      as values
--
function Command_Data:getEmptyDeviceNestedTable()
    return makeNestedTable(self["dev_range"])
end






-- ************************************************************************
---
--  Command_Data:getPortCounts
--        @description  get's count of dev/port pairs in command execution
--                      data object
--
--        @return       interator over the dev/port pairs
--
function Command_Data:getPortCounts()
    local devNum, ports, index, port
    local ret_count = 0

    for devNum, ports in pairs(self["dev_port_range"]) do
        for index, port in pairs(ports) do
            if does_port_exist(devNum, port) then
                ret_count = ret_count + 1
            end
        end
    end

    return ret_count
end


-- ************************************************************************
---
--  Command_Data:getFirstOrCreateMacEntry
--        @description  getting of first mac entry that is correspondent to
--                      mac entry filter of command execution data object
--                      or creating of it.
--                      Sets skip and mac-entry properties of command
--                      execution data object
--                      Requires:
--                          initAllDeviceMacEntryIterator(params).
--
--        @param devId              - applied device number
--        @param alternative        - alternative values, if error occures
--                                    (could be irrelevant)
--
--        @return       mac entry index
--
function Command_Data:getFirstOrCreateMacEntry(devId, alternative)
    local result, values
    local returned_value = 0

    result, values, self["skip"], self["mac_entry"] =
        wrlCpssDxChBrgFdbEntryGreateOrGet(devId, 0, self["mac_filter"])

    if    (0 == result) and (nil ~= values) then
        returned_value = values
    elseif nil ~= alternative               then
        returned_value = alternative
    elseif 0 ~= result then
        self:setFailGeneralAndLocalStatus()
        self:addError(values)
    else
        self:setFailGeneralAndLocalStatus()
        self:addError(string.format("Could not add new mac entry in " ..
                                    "device %d.", devId))
    end

    return returned_value
end


-- ************************************************************************
---
--  Command_Data:getFirstEmptyVidx
--        @description  getting of first empty vidx or generating of error,
--                      if there is not avaible emtry vidx's
--
--        @param devId              - applied device number
--        @param start_vidx         - first vidx for searching (could be
--                                    irrelevent)
--        @param alternative        - alternative values, if error occures
--                                    (could be irrelevant)
--
--        @return       empty vidx iterator, or alternative if error
--                      occures
--
function Command_Data:getFirstEmptyVidx(devId, start_vidx, alternative)
    local result, values
    local returned_value = 0

    if nil == start_vidx then
        start_vidx = 0
    end

    result, values = wrlCpssDxChBrgFirstEmptyVidxGet(devId, start_vidx)

    if    (0 == result) and (nil ~= values) then
        returned_value = values
    elseif nil ~= alternative               then
        returned_value = alternative
    elseif 0 ~= result then
        self:setFailStatus()
        self:addError(values)
    else
        self:setFailStatus()
        self:addError(string.format("There is no avaible unused vidx " ..
                                    "on device %d.", devId))
    end

    return returned_value
end


-- ************************************************************************
---
--  Command_Data:getAllDevicePorts
--        @description  gets all ports from port range of command execution
--                      data object applied to given device
--
--        @param devId              - applied device number
--
--        @return       port-range
--
function Command_Data:getAllDevicePorts(devId)
    return self["dev_port_range"][devId]
end



-- ************************************************************************
---
--  Command_Data:getVlanIterator
--        @description  iterating over the dev/vlan of current device
--                      for iterator, devNum, vlanId in
--                                          command_data:getVlanIterator()
--                      do
--                          print(devNum, vlanId)
--                      end
--
--        @return       interator over the dev/vlan pairs
--
function Command_Data:getVlanIterator()
    local devNum
    local all_vlans

    function iterator(v, i)
        if nil == i then
            i = 0
            devNum = getGlobal("devID")
            all_vlans = wrlCpssDxChVlanList(devNum)
        end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        if i <= #all_vlans then
            return i, devNum, all_vlans[i - 1]
        else
            return nil, nil, nil
        end
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getInterfaceVlanUnsafeIterator
--        @description  iterating over the dev/vlan pairs in device/vlans
--                      range of command execution data object without
--                      checking of vlanId
--                      command_data:initDeviceVlanRange()
--                      for iterator, devNum, vlanId in
--                                  command_data:getInterfaceVlanUnsafeIterator()
--                      do
--                          print(devNum, vlanId)
--                      end
--
--        @return       interator over the dev/vlan pairs
--
function Command_Data:getInterfaceVlanUnsafeIterator()
    function iterator(v, i)
        local dev, vlans
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, vlans in pairs(self["dev_vlan_range"]) do
            if v > #vlans then
                v = v - #vlans
            else
                return i, dev, vlans[v]
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getInterfaceVlanIterator
--        @description  iterating over the dev/vlan pairs in device/vlans
--                      range of command execution data object
--                      command_data:initInterfaceDevVlanRange() or
--                      command_data:initInterfaceVlanIterator()
--                      for iterator, devNum, vlanId in
--                                  command_data:getInterfaceVlanIterator()
--                      do
--                          print(devNum, vlanId)
--                      end
--
--        @return       interator over the dev/vlan pairs
--
function Command_Data:getInterfaceVlanIterator()
    function iterator(v, i)
        local dev, vlans
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, vlans in pairs(self["dev_vlan_range"]) do
            if v > #vlans then
                v = v - #vlans
            else
                if does_vlan_exists(dev, vlans[v]) then
                    return i, dev, vlans[v]
                else
                    self:addNotExistendVlan(dev, vlans[v])
                end
                v = v + 1
                i = i + 1
                while v <= #vlans do
                    if does_vlan_exists(dev, vlans[v]) then
                        return i, dev, vlans[v]
                    else
                        self:addNotExistendVlan(dev, vlans[v])
                    end
                    v = v + 1
                    i = i + 1
                end
                v = v - #vlans
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getDeviceVlanIterator
--        @description  iterating over the dev/vlan pairs in device/vlans
--                      range of command execution data object
--                      command_data:initDeviceVlanRange(params) or
--                      for iterator, devNum, vlanId in
--                                  command_data:getDeviceVlanIterator()
--                      do
--                          print(devNum, vlanId)
--                      end
--
--        @return       interator over the dev/vlan pairs
--
function Command_Data:getDeviceVlanIterator()
    return self:getInterfaceVlanIterator()
end


-- ************************************************************************
---
--  Command_Data:getAllVlansFromDevVlansRange
--        @description  getting of all vlans from dev/vlan range of command
--                      execution  data object
--
--        @usage self["dev_vlan_range"]
--                              - dev/vlan range
--
--        @return       all vlans range
--
function Command_Data:getAllVlansFromDevVlansRange()
    local item, vlans
    local all_vlans = {}

    for item, vlans in pairs(self["dev_vlan_range"]) do
        all_vlans = mergeLists(all_vlans, vlans)
    end

    return all_vlans
end


-- ************************************************************************
---
--  Command_Data:getDevicePortVlanIterator
--        @description  iterating over vlans of given dev/port pairs
--                      command_data:initInterfaceVlanIterator()
--                      for iterator, vlanId, vlanInfo in
--                          command_data:getDevicePortVlanIterator(devId,
--                                                                 portNum)
--                      do
--                          print(devId, portNum, vlanId, vlanInfo)
--                      end
--
--        @param devId          - given device number
--        @param portNum        - given port number
--
--        @return       interator over the dev/vlan pairs
--
function Command_Data:getDevicePortVlanIterator(devId, portNum)
    function iterator(v, i)
        local result, values
        local vlanInfo
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        result, values, vlanInfo = wrlCpssDxChNextBrgPortVlanGet(devId, portNum,
                                                                 i)
        if 0 ~= result then
            self:addError("Error in port vlan iterator: %s.", values)
            self:setFailStatus()
            return nil, nil, nil
        end

        if nil ~=values then
            return values + 1, values, vlanInfo
        else
            return nil, nil, nil
        end
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getDeviceVlanInfoIterator
--        @description  iterating over the dev/vlan/port triplets of
--                      command execution data object
--                      command_data:initDevVlanInfoIterator()
--                      for iterator, devNum, vlanId, vlan_info in
--                              command_data:getDeviceVlanInfoIterator() do
--                      do
--                          print(devNum, vlanId, vlan_info)
--                      end
--
--        @return       interator over the dev/vlan/port triplets
--
function Command_Data:getDeviceVlanInfoIterator()
    function iterator(v, i)
        local dev, vlan_infos
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i
        for dev, vlan_infos in pairs(self["dev_vlan_info_range"]) do
            if v > #vlan_infos then
                v = v - #vlan_infos
            else
                if does_vlan_exists(dev, vlan_infos[v]["vlanId"]) then
                    return i, dev, vlan_infos[v]["vlanId"],
                           vlan_infos[v]["info"]
                else
                    self:addNotExistendVlan(dev, vlan_infos[v]["vlanId"])
                end
                v = v + 1
                i = i + 1
                while v <= #vlan_infos do
                    if does_vlan_exists(dev, vlan_infos[v]["vlanId"]) then
                        return i, dev, vlan_infos[v]["vlanId"],
                               vlan_infos[v]["info"]
                    else
                        self:addNotExistendVlan(dev, vlan_infos[v]["vlanId"])
                    end
                    v = v + 1
                    i = i + 1
                end
                v = v - #vlan_infos
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getDeviceVlanPortIterator
--        @description  iterating over the dev/vlan/port triplets of
--                      command execution data object
--                      command_data:initDevVlanInfoIterator()
--                      for iterator, devNum, vlanId, portNum in
--                              command_data:getDeviceVlanPortIterator() do
--                      do
--                          print(devNum, vlanId, portNum)
--                      end
--
--        @return       interator over the dev/vlan/port triplets
--
function Command_Data:getDeviceVlanPortIterator()
    function iterator(v, i)
        local index, dev, ports
        local vlan_infos, vlan_info
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, vlan_infos in pairs(self["dev_vlan_info_range"]) do
            for index, vlan_info in pairs(vlan_infos) do
                if does_vlan_exists(dev, vlan_info["vlanId"]) then
                    ports = vlan_info["info"]["portsMembers"]

                    if v > #ports then
                        v = v - #ports
                    else
                        if does_port_exist(dev, ports[v]) then
                            return i, dev, vlan_info["vlanId"], ports[v]
                        else
                            self:addNotExistendPort(dev, ports[v])
                        end
                        v = v + 1
                        i = i + 1
                        while v <= #ports do
                            if does_port_exist(dev, ports[v]) then
                                return i, dev, vlan_info["vlanId"], ports[v]
                            else
                                self:addNotExistendPort(dev, ports[v])
                            end
                            v = v + 1
                            i = i + 1
                        end
                        v = v - #ports
                    end
                else
                    self:addNotExistendVlan(dev, vlan_info["vlanId"])
                end
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getTrunkDevPortsIterator
--        @description  iterating over the existend trunks and get's its
--                      dev/port list
--                      for trunkId, trunk_dev_ports in
--                                  command_data:getTrunkDevPortsIterator()
--                      do
--                          print(trunkId)
--                      end
--
--        @return       trunk id and its dev/port list, nil at and of
--                      iteration
--
function Command_Data:getTrunkDevPortsIterator()
    local all_devices = wrlDevList()

    function iterator(v, i)
        local result, values, error_message
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1

        if MAXIMUM_TRUNK_ID >= i then
            result, values = get_trunk_device_port_list(i, all_devices)

            if 0 == result and values and values[0] then
                return i, i, values
            else
                while ((0 ~= result) or (not values or not values[0])) and (MAXIMUM_TRUNK_ID > i) do
                    i = i + 1
                    result, values = get_trunk_device_port_list(i, all_devices)
                end

                if MAXIMUM_TRUNK_ID >= i then
                    return i, i, values
                end
            end
        end

        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getMacEntryIterator
--        @description  iterating over the all entries (valid and invalid)
--                      in Hardware MAC address table
--                      command_data:initAllDeviceMacEntryIterator(params)
--                      for iterator, devNum, entry_index in
--                                      command_data:getMacEntryIterator()
--                      do
--                          print(devNum, entry_index,
--                                self["skip_valid_aged"],
--                                self["mac_entry"])
--                      end
--
--        @return       interator over MAC address records
--
function Command_Data:getMacEntryIterator()
    if fdb_use_shadow == true then
        -- when using the shadow .. only valid entries exists
        return self:getValidMacEntryIterator()
    end

    function iterator_error(values)
        self:addError("Error in mac entry iterator. " .. values)
        self:setFailStatus()
        return nil, nil, nil
    end

    function iterator(v, i)
        local result, values
        local dev, dev_mac_entry
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, dev_mac_entry in pairs(self["dev_mac_entries"]) do
            if v > dev_mac_entry['count'] then
                v = v - dev_mac_entry['count']
            else
                result, values, self["skip_valid_aged"], self["mac_entry"] =
                    wrlCpssDxChBrgFdbNextEntryGet(dev, v - 1, self["mac_aged"],
                                                  self["mac_filter"])
                if 0 ~= result then
                    return iterator_error(values)
                end

                if nil ~= values then
                    return i - v + values + 1, dev, values
                else
                    v = dev_mac_entry['count']
                end

                v = v + 1
                i = i + 1
                while v <= dev_mac_entry['count'] do
                    result, values, self["skip_valid_aged"], self["mac_entry"] =
                        wrlCpssDxChBrgFdbNextEntryGet(dev, v - 1,
                                                      self["mac_aged"],
                                                      self["mac_filter"])
                    if 0 ~= result then
                        return iterator_error(values)
                    end

                    if nil ~= values then
                        return i - v + values + 1, dev, values
                    else
                        v = dev_mac_entry['count']
                    end

                    v = v + 1
                    i = i + 1
                end
                v = v - dev_mac_entry['count']
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getValidMacEntryIterator
--        @description  iterating over the valid entries in Hardware MAC
--                      address table
--                      command_data:initAllDeviceMacEntryIterator(params)
--                      for iterator, devNum, index in
--                                  command_data:getValidMacEntryIterator()
--                      do
--                          print(devNum, entry_index,
--                                self["skip_valid_aged"],
--                                self["mac_entry"])
--                      end
--
--        @return       interator over MAC address records
--
function Command_Data:getValidMacEntryIterator()
    if fdb_use_shadow == true then
        print("-- use FDB shadow -- \n")

        --printCallStack()
    end

    function iterator_error(values)
        if fdb_use_shadow == true and values == 0xFFFFFFFF then
            -- this is not error.
            -- the FDB shadow holds no more entries
            return nil, nil, nil
        end
        self:addError("Error in mac entry iterator. " .. values)
        self:setFailStatus()
        return nil, nil, nil
    end

    function iterator(v, i)
        local result, values
        local dev, dev_mac_entry
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, dev_mac_entry in pairs(self["dev_mac_entries"]) do
            if v > dev_mac_entry['count'] then
                v = v - dev_mac_entry['count']
            else
                result, values, self["skip_valid_aged"], self["mac_entry"] =
                    next_valid_mac_entry_get(dev, v - 1, self["mac_aged"],
                                                       self["mac_filter"])
                if 0 ~= result then
                    return iterator_error(values)
                end

                if nil ~= values then
                    return i - v + values + 1, dev, values
                else
                    v = dev_mac_entry['count']
                end

                v = v + 1
                i = i + 1
                while v <= dev_mac_entry['count'] do
                    result, values, self["skip_valid_aged"], self["mac_entry"] =
                        next_valid_mac_entry_get(dev, v - 1, self["mac_aged"],
                                                           self["mac_filter"])
                    if 0 ~= result then
                        return iterator_error(values)
                    end

                    if nil ~= values then
                        return i - v + values + 1, dev, values
                    else
                        v = dev_mac_entry['count']
                    end

                    v = v + 1
                    i = i + 1
                end
                v = v - dev_mac_entry['count']
            end
        end
        return nil, nil, nil
    end

    return iterator
end

-- ************************************************************************
---
--  Command_Data:getMirroredPortsIterator
--        @description  iterating over the all enabled mirrored ports
--                      command_data:initDevicesPortCounts()
--                      command_data:initMirroredPorts("rx") (optional)
--                      for iterator, devNum, portNum,
--                                  mirror_analyzer_entry_index in
--                                  command_data:getMirroredPortsIterator()
--                      do
--                          print(devNum, portNum)
--                      end
--
--        @return       interator over enabled mirrored ports
--
function Command_Data:getMirroredPortsIterator()
    function iterator_error(values)
        self:addError("Error in mirrored ports iterator. " .. values)
        self:setFailStatus()
        return nil, nil, nil
    end

    function iterator(v, i)
        local result, values
        local dev, portCount, mirror_analyzer_entry_index
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, portCount in pairs(self["dev_port_counts"]) do
            if v > portCount then
                v = v - portCount
            else
                if does_port_exist(dev, v - 1) then
                    result, values, mirror_analyzer_entry_index =
                            next_mirrored_port_get(self["rx_or_tx"], dev, v - 1)
                    if 0 ~= result then
                        return iterator_error(values)
                    end

                    if nil ~= values then
                        return values + 1, dev, values,
                               mirror_analyzer_entry_index
                    else
                        v = portCount
                    end
                end

                v = v + 1
                i = i + 1
                while v <= portCount do
                    if does_port_exist(dev, v - 1) then
                        result, values, mirror_analyzer_entry_index =
                            next_mirrored_port_get(self["rx_or_tx"], dev, v - 1)
                        if 0 ~= result then
                            return iterator_error(values)
                        end

                        if nil ~= values then
                            return values + 1, dev, values,
                                   mirror_analyzer_entry_index
                        else
                            v = portCount
                        end
                    end

                    v = v + 1
                    i = i + 1
                end
                v = v - portCount
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getRouterArpEntriesIterator
--        @description  iterating over the router arp indexes
--                      command_data:initAllDeviceRange(params)
--                      command_data:initDeviceRouterArpEntries(params)
--                      for iterator, devNum, routerArpIndex in
--                                  command_data:getRouterArpEntriesIterator()
--                      do
--                          print(devNum, routerArpIndex,
--                                self["mac-address"])
--                      end
--
--        @return       interator over router arp indexes
--
function Command_Data:getRouterArpEntriesIterator()
    function iterator_error(values)
        self:addError("Error in router arp indexes iterator. " .. values)
        self:setFailStatus()
        return nil, nil, nil
    end

    function iterator(v, i)
        local result, values
        local dev, routerArpEntry
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, routerArpEntry in pairs(self["dev_arp_entries"]) do
            if nil ~= routerArpEntry["arp_indexes"] then
                local index = 1

                -- arp indexes should be sorted
                while (index + 1 <= #routerArpEntry["arp_indexes"])  and
                      (v - 1 < routerArpEntry["arp_indexes"][index]) do
                    index = index + 1
                end

                if index <= #routerArpEntry["arp_indexes"] then
                    if v - 1 <= routerArpEntry["arp_indexes"][index] then
                        v = routerArpEntry["arp_indexes"][index]

                        result, values =
                            myGenWrapper("cpssDxChIpRouterArpAddrRead",
                                         {{ "IN",  "GT_U8",        "devNum", dev },
                                          { "IN",  "GT_U32",       "routerArpIndex",
                                                                   v             },
                                          { "OUT", "GT_ETHERADDR", "arpMacAddr"  }})

                        if 0 ~= result then
                            return iterator_error(values)
                        end

                        self["mac-address"] = values["arpMacAddr"]
                        return v + 1, dev, v
                    end
                end

                v = 1
            elseif v > routerArpEntry["max_index"] then
                v = v - routerArpEntry["max_index"]
            else
                result, values, self["mac-address"] =
                    wrlCpssDxChNextRouterArpEntryGet(dev, v - 1)
                if 0 ~= result then
                    return iterator_error(values)
                end

                if nil ~= values then
                    return i - v + values + 1, dev, values
                else
                    v = routerArpEntry["max_index"]
                end

                v = v + 1
                i = i + 1
                while v <= routerArpEntry["max_index"] do
                    result, values, self["mac-address"] =
                        wrlCpssDxChNextRouterArpEntryGet(dev, v - 1)
                    if 0 ~= result then
                        return iterator_error(values)
                    end

                    if nil ~= values then
                        return i - v + values + 1, dev, values
                    else
                        v = routerArpEntry["max_index"]
                    end

                    v = v + 1
                    i = i + 1
                end
                v = v - routerArpEntry["max_index"]
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getIpUcRouteEntriesIterator
--        @description  iterating over the ip unicast route entry indexes
--                      command_data:initAllDeviceRange(params)
--                      command_data:initDeviceIpUcRouteEntries(params)
--                      for iterator, devNum,
--                                          ip_unicast_route_entry_index in
--                              command_data:getIpUcRouteEntriesIterator()
--                      do
--                          print(devNum, ip_unicast_route_entry_index,
--                                self["ip-unicast-entry"])
--                      end
--
--        @return       interator over ip unicast route entry indexes
--
function Command_Data:getIpUcRouteEntriesIterator()
    function iterator_error(values)
        self:addError("Error in router unicast route entry iterator: " ..
                      values)
        self:setFailStatus()
        return nil, nil, nil
    end

    function iterator(v, i)
        local result, values
        local dev, ipUcRouteEntry
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, ipUcRouteEntry in pairs(self["dev_ip_uc_route_entries"]) do
            if nil ~= ipUcRouteEntry["ip_uc_route_indexes"] then
                local index = 1

                -- ip unicast route entry indexes should be sorted
                while (index + 1 <= #ipUcRouteEntry["ip_uc_route_indexes"])  and
                      (v - 1 < ipUcRouteEntry["ip_uc_route_indexes"][index]) do
                    index = index + 1
                end

                if index <= #ipUcRouteEntry["ip_uc_route_indexes"] then
                    if v - 1 <= ipUcRouteEntry["ip_uc_route_indexes"][index] then
                        v = ipUcRouteEntry["ip_uc_route_indexes"][index]

                        result, values = wrlCpssDxChIpUcRouteEntryRead(dev, v)
                        if 0 ~= result then
                            return iterator_error(values)
                        end

                        self["ip-unicast-entry"] = values
                        return v + 1, dev, v
                    end
                end

                v = 1
            elseif v > ipUcRouteEntry["max_index"] then
                v = v - ipUcRouteEntry["max_index"]
            else
                result, values, self["ip-unicast-entry"] =
                    wrlCpssDxChNextIpUcRouteEntryGet(dev, v - 1)
                if 0 ~= result then
                    return iterator_error(values)
                end

                if nil ~= values then
                    return i - v + values + 1, dev, values
                else
                    v = ipUcRouteEntry["max_index"]
                end

                v = v + 1
                i = i + 1
                while v <= ipUcRouteEntry["max_index"] do
                    result, values, self["ip-unicast-entry"] =
                        wrlCpssDxChNextIpUcRouteEntryGet(dev, v - 1)
                    if 0 ~= result then
                        return iterator_error(values)
                    end

                    if nil ~= values then
                        return i - v + values + 1, dev, values
                    else
                        v = ipUcRouteEntry["max_index"]
                    end

                    v = v + 1
                    i = i + 1
                end
                v = v - ipUcRouteEntry["max_index"]
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getIpPrefixIterator
--        @description  iterating over the ip v4 or v6 prefix entries
--                      command_data:initIpPrefixEntries(params)
--                      for iterator, prefix_ip_address, prefix_length, vrId in
--                                		  command_data:getIpPrefixIterator()
--                      do
--                          print()
--                      end
--
--        @return       iterator over ip unicast route entry indexes
--
function Command_Data:getIpPrefixIterator()
    function iterator_error(values)
        self:addError("Error in ip prefix iterator" .. values)
        self:setFailStatus()
        return nil, nil, nil, nil
    end

    function iterator(v, prefix)
        local result, values, vrId
        local new_prefix_length
        if nil == prefix then
            prefix  = { ["length"] = 0 }
			prefix["vrId"] = 0
            if "CPSS_IP_PROTOCOL_IPV4_E" ==
                                self["ip_prefix_entries"]["ip_protocol"]    then
                prefix["ip_address"]    = "0.0.0.0"
            else
                prefix["ip_address"]    = "0::0"
            end
        end
        if true ~= self["stop_execution"] then  return nil, nil, nil, nil end

        result, values, new_prefix_length, self["ip-unicast-entry-index"], vrId =
            wrlCpssDxChNextValidIpPrefixDataGet(
                self["ip_prefix_entries"]["ip_protocol"], prefix["ip_address"],
                prefix["length"], prefix["vrId"])
		if nil ~= values then
            return { ["ip_address"] = values, ["length"] = new_prefix_length, ["vrId"] = vrId },
                   values, new_prefix_length, vrId
        else
            return nil, nil, nil, nil
        end
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getMirrorAnalyzerEntriesIterator
--        @description  iterating over the mirror analyzer indexes
--                      command_data:initAllDeviceRange(params)
--                      command_data:initDeviceMirrorAnalyzerEntries(params)
--                      for iterator, devNum, mirror_analyzer_entry_index
--                          in
--                          command_data:getMirrorAnalyzerEntriesIterator()
--                      do
--                          print(devNum, mirror_analyzer_entry_index,
--                                self["mirror-analyzer-entry"])
--                      end
--
--        @return       interator over mirror analyzer indexes
--
function Command_Data:getMirrorAnalyzerEntriesIterator()
    function iterator_error(values)
        self:addError("Error in mirror analyzer indexes iterator. " .. values)
        self:setFailStatus()
        return nil, nil, nil
    end

    function iterator(v, i)
        local result, values
        local dev, mirrorAnalyzerEntry
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, mirrorAnalyzerEntry in pairs(self["mirror_analyzer_entries"]) do
            if v > mirrorAnalyzerEntry["max_index"] then
                v = v - mirrorAnalyzerEntry["max_index"]
            else
                result, values, self["mirror-analyzer-entry"] =
                    next_mirror_analyzer_index_get(dev, v - 1, true,
                                                   self["mirror-analyzer-filter"])
                if 0 ~= result then
                    return iterator_error(values)
                end

                if nil ~= values then
                    return i - v + values + 1, dev, values
                else
                    v = mirrorAnalyzerEntry["max_index"]
                end

                v = v + 1
                i = i + 1
                while v <= mirrorAnalyzerEntry["max_index"] do
                    result, values, self["mirror-analyzer-entry"] =
                        next_mirror_analyzer_index_get(dev, v - 1, true,
                                                       self["mirror-analyzer-filter"])
                    if 0 ~= result then
                        return iterator_error(values)
                    end

                    if nil ~= values then
                        return i - v + values + 1, dev, values
                    else
                        v = mirrorAnalyzerEntry["max_index"]
                    end

                    v = v + 1
                    i = i + 1
                end
                v = v - mirrorAnalyzerEntry["max_index"]
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getDevicesVidxIterator
--        @description  iterating over the dev/vidx range
--                      command_data:initDevicesVidxRange()
--                      for iterator, devNum, vidx in
--                                  command_data:getDevicesVidxIterator()
--                      do
--                          print(devNum, vidx)
--                      end
--
--        @return       interator over enabled mirrored ports
--
function Command_Data:getDevicesVidxIterator()
    function iterator(v, i)
        local dev, vidx
        if nil == i then i = 0 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        i = i + 1
        v = i

        for dev, vidx in pairs(self["dev_vidx_range"]) do
            if v > #vidx then
                v = v - #vidx
            else
                return i, dev, vidx[v]
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getPriorityIterator
--        @description  iterating over priorities of command execution
--                      data object
--                      command_data:initPriorityData()
--                      command_data:initPriorityRange()
--                      for iterator, priority in
--                                  command_data:getPriorityIterator()
--                      do
--                          print(priority)
--                      end
--
--        @return       interator over enabled mirrored ports
--
function Command_Data:getPriorityIterator()
    function iterator(v, i)
        if nil == i then i = self["priority"]["min"] else   i = i + 1 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        v = i

        if v <= self["priority"]["max"] then
            return v, i
        else
            return nil
        end
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getDevicesPriorityIterator
--        @description  iterating over the dev/priority range
--                      command_data:initDevicesPriorityDataRange()
--                      for iterator, devNum, priority in
--                                  command_data:getDevicesPriorityIterator()
--                      do
--                          print(devNum, priority)
--                      end
--
--        @return       interator over device priorities
--
function Command_Data:getDevicesPriorityIterator()
    function iterator(v, i)
        local dev, priority_data
        if nil == i then i = 0 else i = i + 1 end
        if true ~= self["stop_execution"] then  return nil, nil, nil end

        v = i

        for dev, priority_data in pairs(self["dev_priority_data"]) do
            if     v > priority_data["max"] then
                v = v - priority_data["max"]
            elseif v < priority_data["min"] then
                v = priority_data["min"]
                return v, dev, v
            else
                return v, dev, v
            end
        end
        return nil, nil, nil
    end

    return iterator
end


-- ************************************************************************
---
--  Command_Data:getStrOnDevicesCount
--        @description  gets concatenation of string header, body and
--                      footer, if devices count more of command execution
--                      data object than 0 otherwise to aternative, if it
--                      is relevant; footer could be irrelevant
--
--        @return       operation succed
--
function Command_Data:getStrOnDevicesCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    return self:setStrOnDevicesCount(body, footer)
end


-- ************************************************************************
---
--  Command_Data:getStrOnPortsCount
--        @description  gets concatenation of string header, body and
--                      footer, if ports count of command execution
--                      data object more than 0 otherwise to aternative, if
--                      it is relevant; footer could be irrelevant
--
--        @return       operation succed
--
function Command_Data:getStrOnPortsCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    return self:setStrOnPortsCount(body, alternative)
end


-- ************************************************************************
---
--  Command_Data:getStrOnPrioritiesCount
--        @description  gets concatenation of string header, body and
--                      footer, if priorities count of command execution
--                      data object more than 0 otherwise to aternative, if
--                      it is relevant; footer could be irrelevant
--
--        @return       operation succed
--
function Command_Data:getStrOnPrioritiesCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    return self:setStrOnPrioritiesCount(body, alternative)
end


-- ************************************************************************
---
--  Command_Data:getStrOnEntriesCount
--        @description  gets concatenation of string header, body and
--                      footer, if Entries count of command execution
--                      data object more than 0 otherwise to aternative, if
--                      it is relevant; footer could be irrelevant
--
--        @return       operation succed
--
function Command_Data:getStrOnEntriesCount(header, body, footer, alternative)
    body    = concatHeaderBodyAndFooter(header, body, footer)

    return self:setStrOnEntriesCount(body, alternative)
end


-- ************************************************************************
---
--  Command_Data:addNotExistendVlan
--        @description  processes not existend vlan
--
--        @param devId                - device number
--        @param vlanId               - vlan Id
--
--        @return       operation succed
--
function Command_Data:addNotExistendVlan(devId, vlanId)
    self:addWarning("Vlan %d of device %d does not initialized.", vlanId, devId)

    return 0
end


-- ************************************************************************
---
--  Command_Data:addToMacEntry
--        @description  add fields to object Fdb mac-entry of command
--                      execution data object
--
--        @param fields         - table that contains added fields
--
--        @return       operation succed
--
function Command_Data:addToMacEntry(fields)
    self["mac_entry"] = mergeTablesRightJoin(self["mac_entry"], fields)

    return 0
end


-- ************************************************************************
---
--  Command_Data:addToMacEntryFilter
--        @description  add fields to object Fdb mac-entry filter of
--                      command execution data object
--
--        @param filter_fields  - table that contains added fields
--
--        @return       operation succed
--
function Command_Data:addToMacEntryFilter(filter_fields)
    self["mac_filter"] = mergeTablesRightJoin(self["mac_filter"], filter_fields)

    return 0
end


-- ************************************************************************
---
--  Command_Data:addConditionalError
--        @description  add's new error message to command warning array
--                      in command execution data object if condition is
--                      true and object local status is ok
--
--        @param cond           - Boolean condition
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addConditionalError(cond, formatString, ...)
    if true == cond     then
        self:addError(formatString, ...)
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:addErrorIfFailedStatus
--        @description  add's new error message to command warning array in
--                      command execution data object, if its status is
--                      false
--
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addErrorIfFailedStatus(formatString, ...)
    if false == self["status"] then
        self:addError(formatString, ...)
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:addWarningIfNoSuccessVlans
--        @description  add's new warning message to command warning array
--                      in command execution data object, if there is no
--                      successfully processed vlans
--
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addWarningIfNoSuccessVlans(formatString, ...)
    if 0 == self["vlans_with_success_count"] then
        self:addWarning(formatString, ...)
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:addWarningIfNoSuccessTrunks
--        @description  add's new warning message to command warning array
--                      in command execution data object, if there is no
--                      successfully processed trunks
--
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addWarningIfNoSuccessTrunks(formatString, ...)
    if 0 == self["trunks_with_success_count"] then
        self:addWarning(formatString, ...)
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:addWarningIfNoSuccessEntries
--        @description  add's new warning message to command warning array
--                      in command execution data object, if there is no
--                      successfully processed entries
--
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addWarningIfNoSuccessEntries(formatString, ...)
    if 0 == self["entries_with_success_count"] then
        self:addWarning(formatString, ...)
    end
end

-- ************************************************************************
---
--  Command_Data:addErrorIfNoSuccessEntries
--        @description  add's new error message to command warning array
--                      in command execution data object, if there is no
--                      successfully processed entries
--
--        @param formatString   - The error message
--        @param ...            - The error message parameters
--
--        @return       operation succed
--
function Command_Data:addErrorIfNoSuccessEntries(formatString, ...)

    if 0 == self["entries_with_success_count"] then
        self:setFailStatus()
        self:addError(formatString, ...)
    end
end


-- ************************************************************************
---
--  Command_Data:addWarningIfNoSuccessVidixes
--        @description  add's new warning message to command warning array
--                      in command execution data object, if there is no
--                      successfully processed vidixes
--
--        @param formatString   - The warning message
--        @param ...            - The warning message parameters
--
--        @return       operation succed
--
function Command_Data:addWarningIfNoSuccessVidixes(formatString, ...)
    if 0 == self["vidixes_with_success_count"] then
        self:addWarning(formatString, ...)
    end
end


-- ************************************************************************
---
--  Command_Data:addToResultStr
--        @description  add's to result string
--
--        @param ...            - added strings
--
--        @return       operation succed
--
function Command_Data:addToResultStr(...)
    local items = {...}
    local index, item

    for index, item in ipairs(items) do
        self["result"] = self["result"] .. item
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:addToResultStrOnCondition
--        @description  add's to result string
--
--        @param cond           - condition
--        @param ...            - added strings
--
--        @return       operation succed
--
function Command_Data:addToResultStrOnCondition(cond, ...)
    if true == cond then
        return self:addToResultStr(...)
    else
        return 0
    end
end




-- ************************************************************************
---
--  Command_Data:addNotEmptyResultToResultArray
--        @description  add's result, if it is not empty, of command
--                      execution data object to result array in command
--                      execution data object
--
--        @return       operation succed
--
function Command_Data:addNotEmptyResultToResultArray()
    if false == isEmptyStr(self["result"]) then
        self:addToResultArray(self["result"])
    end

    return 0
end

-- ************************************************************************
---
--  Command_Data:addToEmergencyPrintingHeader
--        @description  contact's emergency printing header with its prefix
--
--        @param header_prefix  - emergency header prefix
--
function Command_Data:addToEmergencyPrintingHeader(header_prefix)
    self["emergency_header"]   = header_prefix .. self["emergency_header"]
end


-- ************************************************************************
---
--  Command_Data:updateStatusOnCondition
--        @description  updates cammand execution status according to its
--                      local status and givren condition in command
--                      execution data object
--
--        @param cond                - Condition
--
--        @return       operation succed
--
function Command_Data:updateStatusOnCondition(cond)
    if (true ~= self["local_status"]) or (true ~= cond) then
        self["status"] = false
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:configureDeviceVlanRange
--        @description  configures device vlans of command execution data
--                      object
--
--        @param dev_ports          - dev_ports["added"]: added pots;
--                                    dev_ports["removed"]: removed ports;
--                                    dev_ports["tagged"]: tagged ports;
--                                    dev_ports["untagged"]: untagged
--                                    ports;
--                                    dev_ports["tagging_command"]: ports
--                                    tagging commands array;
--
--        @return       operation succeed
--
function Command_Data:configureDeviceVlanRange(dev_ports)
    -- Common variables declaration
    local result, values
    local devNum, vlanId
    local iterator
    -- Function specific variables declaration
    local vlanInfo, port_tagging_command_supporting
    local used_unsupported_tagging_command
    local key, tab;

    -- Main vlans handling cycle
    for iterator, devNum, vlanId in self:getInterfaceVlanIterator() do
        self:clearVlanStatus()

        self:clearLocalStatus()

        -- Vlan information getting.
        if true == self["local_status"]                 then
            result, values = vlan_info_get(devNum, vlanId)
            if        0 == result then
                vlanInfo = values
            elseif 0x10 == result then
                self:setFailVlanAndLocalStatus()
                self:addWarning("Information getting of vlan %d is not " ..
                                "allowed on device %d.", vlanId, devNum)
            elseif    0 ~= result then
                self:setFailVlanAndLocalStatus()
                self:addError("Error at information getting of vlan %d " ..
                              "on device %d: %s.", vlanId, devNum, values)
            end
        end

        -- Port tagging command checking.
        if (true == self["local_status"])               and
           (nil  ~= dev_ports["tagging_command"])       then
            result, values = wrlCpssDxChIsPortTaggingCmdSupported(devNum)
            if       0 == result then
                port_tagging_command_supporting = values
            elseif 0x10 == result then
                self:setFailVlanAndLocalStatus()
                self:addWarning("It is not allowed to get port " ..
                                        "tagging command checking for " ..
                                        "vlan %d in device %d.", vlanId, devNum)
            elseif 0 ~= result then
                self:setFailVlanAndLocalStatus()
                self:addError("Error at port tagging command " ..
                                      "checking for vlan %d in device %d: %s.",
                              vlanId, devNum, values)
            end
        end

        used_unsupported_tagging_command = false;
        tab = dev_ports["tagging_command"]
        if tab ~= nil then
            tab = tab[devNum];
        end
        if tab ~= nil then
            key = next(tab, nil)
            while key ~= nil do
                if tab[key] ~= "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E" then
                    used_unsupported_tagging_command = true;
                end
                key = next(tab, key)
            end
        end

        self:addConditionalWarning(
            logicalNot(port_tagging_command_supporting) and
            (used_unsupported_tagging_command == true),
            "Vlan %d ports tagging command setting is not supported on " ..
            "device %d.", vlanId, devNum)
--print(1, to_string(vlanInfo))
        -- Adding of new ports to vlans.
        if (true == self["local_status"])               and
           (nil  ~= dev_ports["added"])                 then
            vlanInfo["portsMembers"] =
                mergeAndSortLists(vlanInfo["portsMembers"],
                                  dev_ports["added"][devNum])
        end

        -- Removing of ports from vlans.
        if (true == self["local_status"])               and
           (nil  ~= dev_ports["removed"])               then
            vlanInfo["portsMembers"] =
                excludeList(vlanInfo["portsMembers"],
                            dev_ports["removed"][devNum])

            vlanInfo["portsTagging"] =
                excludeList(vlanInfo["portsTagging"],
                            dev_ports["removed"][devNum])
        end

        -- Tagging of vlans ports.
        if (true == self["local_status"])               and
           (nil  ~= dev_ports["tagged"])                then
            vlanInfo["portsTagging"] =
                mergeAndSortLists(vlanInfo["portsTagging"],
                                  dev_ports["tagged"][devNum])
        end

        -- Untagging of vlans ports.
        if (true == self["local_status"])               and
           (nil  ~= dev_ports["untagged"])              then
            vlanInfo["portsTagging"] =
                excludeList(vlanInfo["portsTagging"],
                            dev_ports["untagged"][devNum])
        end

        -- Vlans ports tagging command setting.
        if (true == self["local_status"])               and
           (nil  ~= dev_ports["tagging_command"])       then
            vlanInfo["portsTaggingCmd"] =
                mergeTablesRightJoin(vlanInfo["portsTaggingCmd"],
                                     dev_ports["tagging_command"][devNum])
        end
--print(2, to_string(vlanInfo))
        -- Vlan setting.
        if  true == self["local_status"]                then
            result, values = vlan_info_set(devNum, vlanId, vlanInfo)
            if     0x10 == result then
                self:setFailVlanAndLocalStatus()
                self:addWarning("Information setting of vlan %d is not " ..
                                "allowed on device %d.", vlanId, devNum)
            elseif    0 ~= result then
                self:setFailVlanAndLocalStatus()
                self:addError("Error at information setting of vlan %d " ..
                              "on device %d: %s.", vlanId, devNum, values)
            end
        end

        self:updateStatusOnCondition(port_tagging_command_supporting or
                                     (used_unsupported_tagging_command ~= true))

        self:updateVlansOnCondition(port_tagging_command_supporting or
                                    (used_unsupported_tagging_command ~= true))
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:configure_vlan_mru
--        @description  configures vlan mru profile
--                      command_data:initInterfaceDevVlanRange()
--                      command_data:initInterfaceDeviceRange("dev_vlan_range")
--                      command_data:configure_vlan_mru(mru_value)
--
--        @param mru_value          - MRU
--
--        @return       operation succeed
--
function Command_Data:configure_vlan_mru(mru_value)
    -- Common variables declaration
    local result, values
    local devNum, vlanId
    local iterator
    -- Command specific variables declaration
    local mruIndexes

    -- Command specific variables initialization
    mruIndexes  = self:getEmptyDeviceNestedTable()

    -- Main device handling cycle
    for iterator, devNum in self:getDevicesIterator() do
        self:clearDeviceStatus()

        self:clearLocalStatus()

        -- Getting of unuded MRU index.
        if true == self["local_status"]                 then
            result, values = next_unused_mru_index_get(devNum, 0)
            if       (0 == result)      and
                     (nil ~= values)    then
                mruIndexes[devNum]  = values
            elseif   (0 == result)      and
                     (nil == values)    then
                self:setFailDeviceAndLocalStatus()
                self:addWarning("There is no more unused MRU indexed " ..
                                "on device %d.", devNum)
            elseif 0x10 == result       then
                self:setFailDeviceAndLocalStatus()
                self:addWarning("It is not allowed to get unused MRU index " ..
                                "on device %d.", devNum)
            elseif    0 ~= result       then
                self:setFailDeviceAndLocalStatus()
                self:addError("Error at getting of unused MRU index on " ..
                              "device %d: %s", devNum, returnCodes[result])
            end
        end

        -- Setting of MRU value for a VLAN MRU profile.
        if true == self["local_status"]                 then
            result, values =
                myGenWrapper("cpssDxChBrgVlanMruProfileValueSet",
                             {{ "IN", "GT_U8",  "devNum",   devNum},
                              { "IN", "GT_U32", "mruIndex", mruIndexes[devNum]},
                              { "IN", "GT_U32", "mruValue", mru_value}})
            if     0x10 == result then
                self:setFailVlanStatus()
                self:addWarning("It is not allowed to set MRU value  %d " ..
                                "for a VLAN MRU profile %d for device %d " ..
                                "vlan %d.", mru_value, mruIndexes[devNum],
                                devNum, vlanId)
            elseif 0 ~= result then
                self:setFailVlanAndLocalStatus()
                self:addError("Error at %d MRU value setting for a VLAN MRU " ..
                              "profile %d for device %d vlan %d: %s", mru_value,
                              mruIndexes[devNum], devNum, vlanId,
                              returnCodes[result])
            end
        end

        self:updateStatus()

        self:updateDevices()
    end

    self:addWarningIfNoSuccessDevices("Can not configure all processed devices.")

    -- Main vlan handling cycle
    if true == self["status"]                           then
        for iterator, devNum, vlanId in self:getInterfaceVlanIterator() do
            self:clearVlanStatus()

            -- Setting of Maximum Receive Unit MRU profile index for a VLAN.
            self:clearLocalStatus()

            if true == self["local_status"]             then
                result, values =
                    cpssPerVlanParamSet("cpssDxChBrgVlanMruProfileIdxSet",
                                        devNum, vlanId, mruIndexes[devNum],
                                        "mruIndex", "GT_U32")
                if     0x10 == result then
                    self:setFailVlanStatus()
                    self:addWarning("It is not allowed to set Maximum " ..
                                            "Receive Unit MRU profile index " ..
                                            "%d  for device %d vlan %d.",
                                            mruIndexes[devNum], devNum, vlanId)
                elseif 0 ~= result then
                    self:setFailVlanAndLocalStatus()
                    self:addError("Error at setting of Maximum Receive Unit " ..
                                  "MRU profile index %d for device %d " ..
                                  "vlan %d: %s", mruIndexes[devNum], devNum,
                                  vlanId, returnCodes[result])
                end
            end

            self:updateStatus()

            self:updateVlans()
        end

        self:addWarningIfNoSuccessVlans(
            "Can not configure all processed vlans.")
    end

    return 0
end


-- ************************************************************************
---
--  Command_Data:configureDeviceVidxRange
--        @description  configures device vidx of command execution data
--                      object
--
--        @param dev_ports          - dev_ports["setted"]: setted ports;
--                                    dev_ports["added"]: added pots;
--                                    dev_ports["removed"]: removed ports;
--
--        @return       operation succeed
--
function Command_Data:configureDeviceVidxRange(dev_ports)
    -- Common variables declaration
    local result, values
    local devNum, vidx
    local iterator
    -- Function specific variables declaration
    local ports_members

    -- Main vidx handling cycle
    if true == self["status"]                   then
        for iterator, devNum, vidx in self:getDevicesVidxIterator() do
            self:clearVidxStatus()

            -- Multicast entry reading.
            self:clearLocalStatus()

            if true == self["local_status"]                 then
                result, values = wrlCpssDxChBrgMcEntryRead(devNum, vidx)
                if        0 == result then
                    ports_members = values
                elseif 0x10 == result then
                    self:setFailVidxStatus()
                    self:addWarning("It is not allowed to read multicast " ..
                                    "entry %d on device %d.", vidx, devNum)
                elseif 0 ~= result then
                    self:setFailVidxAndLocalStatus()
                    self:addError("Error at %d multicast entry reading " ..
                                  "on device %d: %s.", vidx, devNum, values)
                end
            end
--print(1, to_string(ports_members))
            -- Setting of port members.
            if (true == self["local_status"])               and
               (nil  ~= dev_ports["setted"])                then
                ports_members = dev_ports["setted"][devNum]
            end

            -- Adding of new ports to port members.
            if (true == self["local_status"])               and
               (nil  ~= dev_ports["added"])                 then
                ports_members = mergeAndSortLists(ports_members,
                                                  dev_ports["added"][devNum])
            end

            -- Removing of ports from port members.
            if (true == self["local_status"])               and
               (nil  ~= dev_ports["removed"])               then
                ports_members = excludeList(ports_members,
                                            dev_ports["removed"][devNum])
            end
--print(2, devNum, vidx, to_string(ports_members))
            -- Multicast entry writing.
            if true == self["local_status"]                 then
                result, values = wrlCpssDxChBrgMcEntryWrite(devNum, vidx,
                                                            ports_members)
                if     0x10 == result then
                    self:setFailVidxStatus()
                    self:addWarning("It is not allowed to write multicast " ..
                                    "entry %d on device %d.", vidx, devNum)
                elseif 0 ~= result then
                    self:setFailVidxAndLocalStatus()
                    self:addError("Error at %d multicast entry writing " ..
                                  "on device %d: %s.", vidx, devNum, values)
                end
            end

            self:updateStatus()

            self:updateVidxes()
        end
    end
end


-- ************************************************************************
---
--  Command_Data:updateTrunks
--        @description  updates trunks count statistic in command execution
--                      data object
--
function Command_Data:updateTrunks()
    self["trunks_count"] = self["trunks_count"] + 1
    if true == self["port_status"] then
        self["trunks_with_success_count"] = self["trunks_with_success_count"] + 1
    end
end


-- ************************************************************************
---
--  Command_Data:updateVlans
--        @description  updates vlans count statistic in command execution
--                      data object; increase success processed vlans count
--                      according to vlan status
--
function Command_Data:updateVlans()
    self["vlans_count"] = self["vlans_count"] + 1
    if true == self["vlan_status"] then
        self["vlans_with_success_count"] = self["vlans_with_success_count"] + 1
    end
end


-- ************************************************************************
---
--  Command_Data:updateVlansOnCondition
--        @description  updates vlans count statistic in command execution
--                      data object; increase success processed vlans count
--                      according to vlan status and condition
--
function Command_Data:updateVlansOnCondition(cond)
    self["vlans_count"] = self["vlans_count"] + 1
    if (true == self["vlan_status"]) and (true == cond) then
        self["vlans_with_success_count"] = self["vlans_with_success_count"] + 1
    end
end


-- ************************************************************************
---
--  Command_Data:updateVidxes
--        @description  updates vidx count statistic in command
--                      execution data object
--
function Command_Data:updateVidxes()
    self["vidxes_count"] = self["vidxes_count"] + 1
    if true == self["vidx_status"] then
        self["vidxes_with_success_count"] = self["vidxes_with_success_count"] + 1
    end
end


-- ************************************************************************
---
--  Command_Data:updatePriorities
--        @description  updates priorities count statistic in command
--                      execution data object
--
function Command_Data:updatePriorities()
    self["priorities_count"] = self["priorities_count"] + 1
    if true == self["priority_status"] then
        self["priorities_with_success_count"] =
            self["priorities_with_success_count"] + 1
    end
end

-- ************************************************************************
---
--  Command_Data:stopCommandExecution
--        @description  set's command execution stopping property
--
--        @return       operation succed
--
function Command_Data:stopCommandExecution()
    self["stop_execution"] = false
    self:clearResultArray()

    return 0
end

-- ************************************************************************
---
--  stopCommandExecution
--        @description  set's command execution stopping property for
--                      given command execution data object
--
--        @param command_data       - ommand execution data object
--
--        @return       operation succed
--
function stopCommandExecution(command_data)
    return command_data:stopCommandExecution()
end

-- ************************************************************************
---
--  Command_Data:initTable
--        @description  initialize table
--
--        @param  desc  - table columns description
--
--        @return       operation succed
--

function Command_Data:initTable(desc)
    self.table_desc = desc
    self.ntable = Nice_Table(desc)

    self:enablePausedPrinting()
    self:setEmergencyPrintingHeaderAndFooter(
        self.ntable.header_string .. "\n",
        self.ntable.footer_string .. "\n")
end

function Command_Data:addTableRow(tbl)
    local s = self.ntable:formatRow(tbl)

    self:updateStatus()
    self.result=s
    self:addResultToResultArray()
end

function Command_Data:printTableRow(tbl)
    local s = self.ntable:formatTableRows(tbl)
    print(s)

    self:updateStatus()
    self.result=""
    self:addResultToResultArray()
end

function Command_Data:showTblResult(emptyTableMsg)
    self:setResultArrayToResultStr()
    self:setResultStrOnEntriesCount(self.ntable.header_string.."\n",
            self.result, self.ntable.footer_string.."\n",
            emptyTableMsg.."\n")

    self:analyzeCommandExecution()

    self:printCommandExecutionResults()
end


local function device_range_filter(params)
    local min_dev_id = params["min_dev_id"];
    local max_dev_id = params["max_dev_id"];
    local enable = tonumber(params["enable"]);
    wrlDevFilterRangeSet(min_dev_id, max_dev_id, enable);
end

CLI_addCommand("config", "device-range-filter", {
    help = "Adds/removes range of device id-s to/from device filter",
    func = device_range_filter,
    params={
        { -- parameters node
            type = "values",
            { format="%GT_U8",name="min_dev_id",
                help="minimal device number in the range",
            },
            { format="%GT_U8",name="max_dev_id",
                help="maximal device number in the range",
            },
            { format="%bool",name="enable",
                help="enable or disable",
            },
        },
        mandatory={"enable"},
        requirements = {
            enable  = {"min_dev_id", "max_dev_id"},
            max_dev_id = {"min_dev_id"}
        },
    }
})
