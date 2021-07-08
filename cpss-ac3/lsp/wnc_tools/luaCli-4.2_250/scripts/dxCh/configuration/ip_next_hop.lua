--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_next_hop.lua
--*
--* DESCRIPTION:
--*       Writing and deleting next-hop entries to the hardware.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 7 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChIpUcRouteEntryRead")

--constants

--[[ define the table that will hold the 'ARP configuration done in the system'
     each line in the table hold info about different device
     each device hold next info :
                  1.arp index - next-hop-id, a unique name
                  2.arp name
--]]

-- ************************************************************************
--
--  next_hop_entries_partial_compare
--
--        @description  Compare two tables of next-hop entries based on a
--                      limited number of keys (those keys are the only ones
--                      set explicitly in the 'ip next-hop' command.
--
--        @param        tbl1, tbl2: lua tables representing ip next-hop entries.
--
--        @return       true if entries are the same, false otherwise.
--
local function next_hop_entries_partial_compare(tbl1, tbl2)
    if tbl1["entry"]["regularEntry"]["nextHopARPPointer"] ~= tbl2["entry"]["regularEntry"]["nextHopARPPointer"] then
        return false
    elseif interfaceInfoStrGet(tbl1["entry"]["regularEntry"]["nextHopInterface"]) ~= interfaceInfoStrGet(tbl2["entry"]["regularEntry"]["nextHopInterface"]) then
        return false
    elseif tbl1["entry"]["regularEntry"]["nextHopVlanId"] ~= tbl2["entry"]["regularEntry"]["nextHopVlanId"] then
        return false
    elseif tbl1["entry"]["regularEntry"]["isTunnelStart"] ~= tbl2["entry"]["regularEntry"]["isTunnelStart"] then
        return false
    elseif tbl1["entry"]["regularEntry"]["cmd"] ~= tbl2["entry"]["regularEntry"]["cmd"] then
        return false
    else
        return true
    end
end

-- ************************************************************************
--
--  ip_next_hop_func
--        @description  Configuring of unicast/multicast route next hop
--                      entry.
--
--        @param params             - params["next-hop-id"]: Ip unicast
--                                    entry index;
--                                    params["packet_cmd"]: packet command;
--                                    params["vid"]: vlan id;
--                                    params["ethernet"]: interface
--                                    dev/port pair, alternative
--                                    params["port-channel"];
--                                    params["port-channel"]: trunk id,
--                                    alternative params["ethernet"];
--                                    params["mac-address"]: mac-address
--
--        @return       true on success, otherwise false and error message
--
-- ************************************************************************

local function ip_next_hop_func(params)
    -- Common variables declaration
    local result, values
    local iterator, devNum
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local nexthop_id, new_nexthop_entry, old_nexthop_entry
    local arp_index, mac_address, packet_command
    local arp_table = getTableArp()
    local arp_table_params = {}
    local done, nexthop_exists, cmdIsRouteType
    local arp_exists, old_arp_index, same_mac
    -- Common variables initialization
    --command_data:initAllDeviceRange(params)  this line is for per-device next-hop configuration (instead of the next one)
    command_data:initAllAvailableDevicesRange()
    packet_command    = params["packet_cmd"]
    nexthop_id        = params["next-hop-id"]
    new_nexthop_entry =
    {
        ["type"]         = "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E",
        ["entry"] = {["regularEntry"] = {["cmd"] = packet_command, ["isTunnelStart"] = false}}
    }
    -- determine if packet command is of routed type
    if packet_command == "CPSS_PACKET_CMD_ROUTE_E" or
       packet_command == "CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E" or
       packet_command == "CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E" then
        cmdIsRouteType = true
    else
        cmdIsRouteType = false
    end
    -- set entry attributes: if packet command is of routed type use command params otherwise use dummy params
    if cmdIsRouteType then
        mac_address                                        = params["mac-address"]
        if mac_address == nil then
            print("Missing mandatory parameter for route: mac")
            return false
        end
        new_nexthop_entry["entry"]["regularEntry"]["nextHopVlanId"] = params["vid"]
    else
        new_nexthop_entry["entry"]["regularEntry"]["nextHopVlanId"] = 0
        params["mac-address"] = NIL_UC_MAC_ADDRESS
        params["eport"] = nil
        params["port-channel"] = nil
        params["ethernet"] = {}
        params["ethernet"]["devId"] = devEnv.dev
        params["ethernet"]["portNum"] = 0
    end

    command_data:initMacEntry(params, {["isStatic"] = true, ["daRoute"] = true })
    command_data:initInterfaceInfoStructure(params)
    new_nexthop_entry["entry"]["regularEntry"]["nextHopInterface"] = command_data["interface_info"]

    -- Main device handling cycle
    if true == command_data["status"] then
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()
            command_data:clearLocalStatus()
            done = false
            if cmdIsRouteType then
                arp_exists = false
                same_mac = false
                -- Check if a next-hop entry with the given ID already exists
                if true == command_data["local_status"]     then
                    result, old_nexthop_entry = wrLogWrapper("wrlCpssDxChIpUcRouteEntryRead",
                                                   "(devNum,nexthop_id)",
                                                    devNum, nexthop_id)
                    nexthop_exists = 0 ~= old_nexthop_entry["entry"]["regularEntry"]["nextHopVlanId"]
                end
                if true == command_data["local_status"]     then
                    arp_index = arp_table_index_lookup(devNum, mac_address)
                    arp_exists = -1 ~= arp_index
                    if nexthop_exists then
                        -- next-hop entry exists
                        new_nexthop_entry["entry"]["regularEntry"]["nextHopARPPointer"] = arp_index
                        -- Check if the existing next-hop entry is identical to the new one
                        if true == next_hop_entries_partial_compare(old_nexthop_entry, new_nexthop_entry) then
                            -- next-hop entries are identical: do nothing
                            printf("Error creating IP next-hop entry %d on device %d: this entry already exists.",nexthop_id,devNum)
                            done = true
                        else
                            -- next-hop entries are not identical: Check if MAC addresses are identical
                            old_arp_index = old_nexthop_entry["entry"]["regularEntry"]["nextHopARPPointer"]
                            same_mac = mac_address.string == arp_table[devNum][old_arp_index].macAddr.string
                            if not same_mac then
                                -- MAC addresses of new and existing next-hop entries are different, update refCount of existing ARP entry
                                arp_table[devNum][old_arp_index].refCount = arp_table[devNum][old_arp_index].refCount - 1
                                if 0 == arp_table[devNum][old_arp_index].refCount and false == arp_table[devNum][old_arp_index].isStatic then
                                    -- Delete old ARP entry which is no longer needed
                                    result, values = arp_entry_hw_write(devNum, old_arp_index, NIL_UC_MAC_ADDRESS, command_data)
                                    if      0 == result then
                                        sharedResourceNamedArpMemoFree("ARP_MAC " .. arp_table[devNum][old_arp_index].macAddr.string)
                                        -- ARP entry deleted successfully, update the lua ARP DB
                                        arp_table_params = {}
                                        arp_table_params["all_device"] = devNum
                                        arp_table_params["arp_entry_index"] = old_arp_index
                                        arp_table_params["mac-address"] = NIL_UC_MAC_ADDRESS
                                        arp_table_update(arp_table_params)
                                    end
                                end
                            end
                        end
                    end
                end
                if not done and true == command_data["local_status"]     then
                    if arp_exists and not same_mac then
                        -- ARP entry exists: use it and just update the refCount
                        arp_table[devNum][arp_index].refCount = arp_table[devNum][arp_index].refCount + 1
                    end
                    if not arp_exists then
                        -- ARP entry does not exist: create new ARP entry
                        arp_index = sharedResourceNamedArpMemoAlloc("ARP_MAC " .. mac_address.string, 1)
                        new_nexthop_entry["entry"]["regularEntry"]["nextHopARPPointer"] = arp_index
                        result, values = arp_entry_hw_write(devNum, arp_index, mac_address, command_data)
                        if 0 == result then
                            -- operation succeeded, update the lua ARP DB
                            arp_table_params = {}
                            arp_table_params["all_device"] = devNum
                            arp_table_params["arp_entry_index"] = arp_index
                            arp_table_params["mac-address"] = mac_address
                            arp_table_params["is_static"] = false
                            arp_table_update(arp_table_params)
                        end
                    end
                end
            end
            if not done and true == command_data["local_status"]     then
                -- Write next-hop entry to hardware
                result, values = wrLogWrapper("wrlCpssDxChIpUcRouteEntryWrite",
                                 "(devNum, nexthop_id, new_nexthop_entry)",
                                  devNum, nexthop_id, new_nexthop_entry)
                if     0x10 == result                   then
                    command_data:setFailDeviceStatus()
                    command_data:addWarning("It is not allowed to write ip " ..
                                            "unicast route entry %d on " ..
                                            "device %d.",
                                            nexthop_id, devNum)
                elseif 0x1E == result                       then
                    command_data:setFailDeviceStatus()
                    command_data:addError("Writing of ip unicast route " ..
                                          "entry %d does not supported on " ..
                                          "device %d: %s.",
                                          nexthop_id, devNum, values)
                elseif    0 ~= result                       then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at writing of ip unicast " ..
                                          "route entry %d on device %d: %s." ,
                                          nexthop_id, devNum, values)
                end
            end
            command_data:updateStatus()
            command_data:updateDevices()
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- ************************************************************************
---
--  no_ip_next_hop_func
--        @description  invalidating of unicast/multicast route next hop
--                      entry
--
--        @param        params      - params["next-hop-id"]: Ip unicast
--                                    entry index;
--
--        @return       true on success, otherwise false and error message
--
-- ************************************************************************

local function no_ip_next_hop_func(params)
    -- Common variables declaration
    local result, values
    local iterator, devNum
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local nexthop_id, nexthop_entry, packet_command
    local arp_index, done, cmdIsRouteType
    local arp_table = getTableArp()
    local arp_table_params = {}
    -- Common variables initialization
    --command_data:initAllDeviceRange(params)  this line is for per-device next-hop configuration (instead of the next one)
    command_data:initAllAvailableDevicesRange()
    -- Command specific variables initialization.
    nexthop_id = params["next-hop-id"]
    -- Main device handling cycle
    if true == command_data["status"] then
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()
            command_data:clearLocalStatus()
            done = false
            -- Read the desired next-hop entry from hardware
            if true == command_data["local_status"]     then
                result, values = wrLogWrapper("wrlCpssDxChIpUcRouteEntryRead", "(devNum,nexthop_id)", devNum,nexthop_id)
                if        0 == result                   then
                    nexthop_entry = values
                    arp_index = nexthop_entry["entry"]["regularEntry"]["nextHopARPPointer"]
                    packet_command = nexthop_entry["entry"]["regularEntry"]["cmd"]
                    if packet_command == "CPSS_PACKET_CMD_ROUTE_E" or
                       packet_command == "CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E" or
                       packet_command == "CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E" then
                        cmdIsRouteType = true
                    else
                        cmdIsRouteType = false
                    end
                elseif 0x10 == result                   then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addWarning("It is not allowed to read ip " ..
                                            "unicast route entry %d on " ..
                                            "device %d.",
                                            nexthop_id, devNum)
                elseif 0x1E == result                   then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Reading of ip unicast route " ..
                                          "entry %d does not suppoeted on " ..
                                          "device %d: %s." ,
                                          nexthop_id, devNum, values)
                elseif    0 ~= result                   then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at reading of ip unicast " ..
                                          "route entry %d on device %d: %s." ,
                                          nexthop_id, devNum, values)
                end
            end
            -- Check if the desired next-hop entry is valid, if it is invalid do nothing
            if cmdIsRouteType and true == command_data["local_status"]     then
                if 0 == nexthop_entry["entry"]["regularEntry"]["nextHopVlanId"] then
                    done = true
                    printf("Error deleting IP next-hop entry %d on device %d: such entry does not exist.",nexthop_id,devNum)
                end
            end
            -- If entry is valid, change its nextHopVlanId to 0 and write it back to hardware
            if not done and true == command_data["local_status"]     then
                nexthop_entry["entry"]["regularEntry"]["nextHopVlanId"] = 0
                result, values = wrLogWrapper("wrlCpssDxChIpUcRouteEntryWrite",
                                              "(devNum, nexthop_id, nexthop_entry)",
                                                devNum, nexthop_id, nexthop_entry)
                if          0 == result                     then
                    if cmdIsRouteType then
                        -- Update the lua ARP DB and if necessary delete this ARP entry
                        arp_table[devNum][arp_index].refCount = arp_table[devNum][arp_index].refCount - 1
                        if 0 == arp_table[devNum][arp_index].refCount and false == arp_table[devNum][arp_index].isStatic then
                            -- Delete old ARP entry which is no longer needed
                            result, values = arp_entry_hw_write(devNum, arp_index, NIL_UC_MAC_ADDRESS, command_data)
                            if      0 == result then
                                sharedResourceNamedArpMemoFree("ARP_MAC " .. arp_table[devNum][arp_index].macAddr.string)
                                -- ARP entry deleted successfully, update the lua ARP DB
                                arp_table_params = {}
                                arp_table_params["all_device"] = devNum
                                arp_table_params["arp_entry_index"] = arp_index
                                arp_table_params["mac-address"] = NIL_UC_MAC_ADDRESS
                                arp_table_update(arp_table_params)
                            end
                        end
                    end
                elseif     0x10 == result                   then
                    command_data:setFailDeviceStatus()
                    command_data:addWarning("It is not allowed to write ip " ..
                                            "unicast route entry %d on " ..
                                            "device %d.",
                                            nexthop_id, devNum)
                elseif 0x1E == result                       then
                    command_data:setFailDeviceStatus()
                    command_data:addError("Writing of ip unicast route " ..
                                          "entry %d does not supported on " ..
                                          "device %d: %s." ,
                                          nexthop_id, devNum, values)
                elseif    0 ~= result                       then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at writing of ip unicast " ..
                                          "route entry %d on device %d: %s." ,
                                          nexthop_id, devNum, values)
                end
            end
            command_data:updateStatus()
            command_data:updateDevices()
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: ip next-hop
--------------------------------------------------------------------------------
CLI_addCommand("config", "ip next-hop", {
  func   = ip_next_hop_func,
  help   = "Configuring of unicast/multicast route next hop entry",
  params =
  {
      { type = "values", "%next-hop-id"},
      { type = "named",
          "#cmd",
          "#vid_unchecked_cmd_def",
          "#interface_port_channel_cmd_def",
          "#mac_cmd_def",
        mandatory = { "packet_cmd" },
        requirements = {
                        ["ethernet"] = {"vid"},
                        ["port-channel"] = {"vid"},
                        ["eport"] = {"vid"},
                        ["mac-address"] = {"vid","interface_port_channel_cmd_def"}
                       }
      }
   }
})

--------------------------------------------------------------------------------
-- command registration: no ip next-hop
--------------------------------------------------------------------------------
CLI_addCommand("config", "no ip next-hop", {
  func   = no_ip_next_hop_func,
  help   = "Deleting of unicast/multicast route next hop entry",
  params = {
      { type = "values",
          "%next-hop-id"
    }
  }
})

--[[   these command registrations are for supporting per-device configuration of next-hops
--------------------------------------------------------------------------------
-- command registration: ip next-hop
--------------------------------------------------------------------------------
CLI_addCommand("config", "ip next-hop", {
  func   = ip_next_hop_func,
  help   = "Configuring of unicast/multicast route next hop entry",
  params = {
      { type = "values",
          "%next-hop-id"
      },
      { type = "named",
          "#all_device",
          "#vid_unchecked",
          "#interface_port_channel",
          "#mac",
        mandatory = {"all_device", "vid", "interface_port_channel", "mac-address" },
        requirements = {["mac-address"] = {"interface_port_channel"}}
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no ip next-hop
--------------------------------------------------------------------------------
CLI_addCommand("config", "no ip next-hop", {
  func   = no_ip_next_hop_func,
  help   = "Deleting of unicast/multicast route next hop entry",
  params = {
               {type = "values", "%next-hop-id"},
               {type = "named", "#all_device", mandatory = {"all_device"}}
           }
})]]--
