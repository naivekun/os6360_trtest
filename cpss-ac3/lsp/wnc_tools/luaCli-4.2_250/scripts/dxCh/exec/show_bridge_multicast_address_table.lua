--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_bridge_multicast_address_table.lua
--*
--* DESCRIPTION:
--*       showing of multicast bridging mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 7 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgMcEntryRead")

--constants




function table.contains(tbl, element)
  local value
  for _, value in pairs(tbl) do
    if value == element then
      return true
    end
  end
  return false
end

-- ************************************************************************
--
-- prvGetPortChannelList
-- 		@description		get the list of the port-channels
--							and update the list of the ports according to the port-channels
--
--		@param				list of ports
--
--		@return				list of ports which not belong to a trunk, list of the number of trunks the param ports contains
--

local function prvGetPortChannelList(ports)
	local listOfPortChannelsBoolean = {}
	local portsInChannels = {}
	local portsNotBelongToChannels = {}
	local listOfPortChannels = {}
    
	for trunkId = 1,MAXIMUM_TRUNK_ID do -- runs over all the port channels, finds the ones that the ports contains them
		local result, vals = get_trunk_device_port_list(trunkId)
		if result == 0 and next(vals) ~= nil then
			for i,v in pairs(vals[0]) do
				if table.contains(ports, v) then
					table.insert(portsInChannels, v) -- builds the table which contains the ports that belong to a trunk
					listOfPortChannelsBoolean[trunkId] = true -- mark the trunk as used
				end
			end
		end
	end


	for i,v in pairs(ports) do -- builds the table of the rest of the ports
		if (not table.contains(portsInChannels, v)) then
			table.insert(portsNotBelongToChannels, v)
		end
	end


	for i,v in pairs(listOfPortChannelsBoolean) do -- builds the list of the port-channels
		table.insert(listOfPortChannels, i)
	end

	return portsNotBelongToChannels, listOfPortChannels

end

local function add_vidx_entry_to_show_table(command_data,vidx,devNum)
    local result,portChannelsList,values

    result, values = wrLogWrapper("wrlCpssDxChBrgMcEntryRead", "(devNum, vidx)",devNum, vidx)
    
    --print("vidx entry (1): " , to_string(values))
    if 0 ~= result then
        -- NOTE: on error we get values = "Could not read multicast entry"
        -- so need to make it 'empty table'
        values = {}
    end

    if command_data.mac_entry_index ~= -1 then
        command_data:handleCpssErrorDevice(result,
                string.format("read multicast entry %d (for FDB index %d) ", vidx , command_data.mac_entry_index), devNum)
    else
        command_data:handleCpssErrorDevice(result,
                string.format("read multicast entry %d (for NON-FDB entry) ", vidx), devNum)
    end


    values, portChannelsList = prvGetPortChannelList(values)

    if 0 == result and portChannelsList ~= nil then
        local vidx_ports_members = makeNestedTable({devNum}, values)

        command_data.mac_entry.ports_string   = devRangeStrGet(vidx_ports_members)

        local portChannelStr = ""
        local firstRunInLoop = true
        for i,v in pairs(portChannelsList) do -- concatenates the list of the port-channels to the output
            if (not firstRunInLoop) then
                portChannelStr = portChannelStr .. ", "
            else
                firstRunInLoop = false
            end
            portChannelStr = portChannelStr .. "port-channel " .. v
        end
        if portChannelStr ~= "" then
            local comma = ", "
            if command_data.mac_entry.ports_string == "" then
                comma = ""
            end
            command_data.mac_entry.ports_string   = command_data.mac_entry.ports_string .. comma .. portChannelStr
        end
    end
end

-- ************************************************************************
---
--  show_bridge_multicast_address_table_func
--        @description  show's multicast bridging mode
--
--        @param params         - params["all_device"]: all or given
--                                devices iterating property, could be
--                                irrelevant
--                                params["vlan"]: vlan Id, could be
--                                irrelevant;
--                                params["multicast_address"]: multicast
--                                address, could specify multicast mac-
--                                address, multicast ipv4 or ipv6, could be
--                                irrelevent;
--                                params["source_ip"]: ipv4 or ipv6 source
--                                address;
--                                params["ip"]: ip multicast address
--                                format;
--                                params["mac"]: mac multicast address
--                                format;
--
--        @return       true on success, otherwise false and error message
--
local function show_bridge_multicast_address_table_func(params)
    -- Common variables declaration
    local result, values
    local devNum, entry_index
    local command_data = Command_Data()
    local entryExist = false
    local fdbEntryType;

    -- printLog("DBG:show_bridge_multicast_address_table_func:params =  " .. to_string(params));

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceMacEntryIterator(
        params, { ["dstInterface"]  =
                    { ["type"]    = "CPSS_INTERFACE_VIDX_E" },
                  ["isStatic"]      = true,
                  ["daRoute"]       = false                         })

    -- Command specific variables initialization
    all_entries_showing         = getTrueIfNil(params["ip_mac"])
    only_mac_entries_showing    = isEquivalent(params["ip_mac"], "mac")

    -- Mac address entry getting.
    
    local function mac_or_dip(key)
        -- return macOrDipAddressStrGet(key.entryType, ((key["key"] or {})["macVlan"] or {})["macAddr"],key.dip)
        return macOrDipAddressStrGet(key.entryType, key.key.macVlan, key.key.ipMcast)
    end
    local function sip_str(key)
        -- return sipAddressStrGet(key.entryType, key.sip)
        return sipAddressStrGet(key.entryType, (key.key["ipMcast"] or {})["sip"])
    end
    local function getVlan(key)
        local res = ((key["key"] or {})["ipMcast"] or {})["vlanId"] or
                      ((key["key"] or {})["macVlan"] or {})["vlanId"]
        if res then
          return tostring(res)
        else 
          return "n/a"
        end
    end
    local table_info
    if all_entries_showing then
        table_info={
            { head="Index",len=5,align="c",path="mac_entry_index",type="number" },
            { head="Vlan",len=5,path="mac_entry.key", type=getVlan },
            { head="VID1",len=5,align="c",path="mac_entry.key.vid1" },
            { head="IP/MAC Address",len=22,align="c",path="mac_entry.key",type=mac_or_dip },
            { head="Source Address",len=16,align="c",path="mac_entry.key",type=sip_str },
            { head="Ports",len=21,path="mac_entry.ports_string" }
        }
    elseif only_mac_entries_showing then
        table_info={
            { head="Index",len=5,align="c",path="mac_entry_index",type="number" },
            { head="Vlan",len=5,path="mac_entry.key.key.macVlan.vlanId" },
            { head="MAC Address",len=22,align="c",path="mac_entry.key.key.macVlan.macAddr" },
            { head="Ports",len=21,path="mac_entry.ports_string" }
        }
    else
        table_info={
            { head="Index",len=5,align="c",path="mac_entry_index",type="number" },
            { head="Vlan",len=5,path="mac_entry.key.key.macVlan.vlanId" },
            { head="Group Address",len=22,align="c",path="mac_entry.key.key.ipMcast.sip" },
            { head="Source Address",len=16,align="c",path="mac_entry.key.key.ipMcast.dip" },
            { head="Ports",len=21,path="mac_entry.ports_string" }
        }
    end
    command_data:initTable(table_info)

    -- array of VIDXs that we 'show' by using the HW FDB table
    -- we want to see also other VIDXs that added by command 'bridge multicast mac address group'
    -- but the implementation used the TTI/IPCL as 'forwarding mechanism' instead of the FDB
    local vidx_from_fdb_array = {}

    -- Main entries handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, entry_index in
                                    command_data:getValidMacEntryIterator() do
            command_data:clearEntryStatus()

            command_data:clearLocalStatus()
            -- printMsg("DBG: mac_entry" .. to_string(command_data.mac_entry)) 
            command_data.mac_entry.ports_string   = "n/a"

            -- printMsg("DBG:MAC entry : " ,to_string(command_data.mac_entry))

            -- Multicast entry reading.
            if true == command_data["local_status"]             then
                -- 'extern' function bridge_multicast_mac_address_get_vidx_from_db(db_key_str)
                -- printMsg("DBG:FDB_KEY " .. to_string(command_data.mac_entry.key));
                local db_key_str =
                    command_data_getMacEntryFilterKeyString(
                        command_data.mac_entry.key);
                local vidxFromDb = bridge_multicast_mac_address_get_vidx_from_db(db_key_str)
                local vidxFromHw = command_data.mac_entry.dstInterface.vidx

                if vidxFromDb ~= nil then
                    -- support the option that the entry hold 'eVidx' that represented by 'vidx' for the physical ports
                    vidx = vidxFromDb
                else
                    vidx = vidxFromHw
                end
                -- indicate that this vidx is 'fdb' oriented.
                vidx_from_fdb_array[vidx] = true

                command_data.mac_entry_index = entry_index

                add_vidx_entry_to_show_table(command_data,vidx,devNum)
            end

            command_data:updateStatus()

            if params.entry == nil then
                command_data:addTableRow(command_data)
            else
                if entry_index == tonumber(params.entry) then
                    command_data:printTableRow(command_data)
                    entryExist = true
                    break
                end
            end

            command_data:updateEntries()
        end

        -- we want to see also other VIDXs that added by command 'bridge multicast mac address group'
        -- but the implementation used the TTI/IPCL as 'forwarding mechanism' instead of the FDB
        local last_index = bridge_multicast_get_last_valid_vidx_index()
        local devNum = 0
        for vidx = 0 , last_index do
            -- check that not 'showed' already
            -- check that DB hold {vlanId,mcMacAddr}
            if not vidx_from_fdb_array[vidx] then
               local db_key_str = bridge_multicast_db_key_str_from_db_get(vidx)
               if db_entry then
                    -- print this entry too
                    local key_by_str = command_data_getMacEntryFilterKeyStringToTable(db_key_str);
                    -- print("DBG:key from string: " .. to_string(key_by_str));
                    command_data:clearEntryStatus()

                    command_data:clearLocalStatus()

                    command_data.mac_entry = {
                        key = { entryType = key_by_str.entryType ,
                                key = { 
                                  macVlan = {
                                    vlanId  = tonumber(key_by_str.vlanId),
                                    macAddr = key_by_str.macAddr},
                                  ipMcast = {
                                    sip = key_by_str.sip,
                                    dip = key_by_str.dip}
                                }
                            },
                        ports_string   = "n/a"
                    }

                    command_data.mac_entry_index = -1
                    add_vidx_entry_to_show_table(command_data,vidx,devNum)

                    command_data:updateStatus()

                    if params.entry == nil then
                        command_data:addTableRow(command_data)
                    else
                        if entry_index == tonumber(params.entry) then
                            command_data:printTableRow(command_data)
                            entryExist = true
                            break
                        end
                    end

                    command_data:updateEntries()

               end
            end
        end
    end

    if params.entry == nil then
        command_data:showTblResult("There is no mac address information to show.")
    end

    if params.entry ~= nil and entryExist == false then
        print("Invalid entry index: "..to_string(params.entry))
    end

	return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: show bridge multicast address-table
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show bridge", "Bridging information")
CLI_addHelp("exec", "show bridge multicast", "Multicast bridging information")
CLI_addCommand("exec", "show bridge multicast address-table", {
  func   = show_bridge_multicast_address_table_func,
  help   = "Display Multicast forwarding table",
  params = {
      { type = "named",
          "#all_device",
          "#vlan",
          "#multicast_address",
          "#source_ip",
          "#ip_mac",
          { format="entry %num", name = "entry", help="To display a certain row" },
          {format = "vid1 %unchecked-vlan"},
        mandatory = { "all_device" }
    }
  }
})

