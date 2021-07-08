--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ip_next_hop.lua
--*
--* DESCRIPTION:
--*       showing of the next-hop table
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_ip_next_hop_func
--        @description  show's of the next-hop table
--
--        @param params             - params["all_device"]: all or given 
--                                    devices iterating property, could be 
--                                    irrelevant
--
--        @return       true on success, otherwise false and error message
-- 
local function show_ip_next_hop_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()

    local function next_hop_table_cmd_view(str)
        if not prefix_match("CPSS_PACKET_CMD_",str) then
            return str
        end
        str = prefix_del("CPSS_PACKET_CMD_", str)
        if string.sub(str,-2) == "_E" then
            str = string.sub(str,1,-3)
        end
        return string.lower(str)
    end

    local table_info = {
        { head="next-hop\nID",len=8,align="c",path="entry_index",type="number" },
        { head="vid",len=5,align="c",path="ip-unicast-entry.entry.regularEntry.nextHopVlanId",type="number"},
        { head="interface",len=14,align="c", path="ip-unicast-entry.entry.regularEntry.nextHopInterface", type=interfaceInfoStrGet },
        { head="cmd",len=10,path="ip-unicast-entry.entry.regularEntry.cmd", type=next_hop_table_cmd_view }
    }

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceRange(params)
    command_data:initDeviceIpUcRouteEntries(params)    
    command_data:initTable(table_info)

    -- Main ip-unicast handling cycle
    if true == command_data["status"] then
        local iterator, ip_unicast_route_entry_index
        for iterator, devNum, ip_unicast_route_entry_index in 
                                command_data:getIpUcRouteEntriesIterator() do
            command_data:clearEntryStatus()

            command_data:clearLocalStatus()
            if true == command_data["local_status"] then   
                command_data.entry_index = ip_unicast_route_entry_index
            end

            command_data:addTableRow(command_data)
            command_data:updateEntries()
        end
    end

    command_data:showTblResult("There is no ip next-hop information to show.")
    
	return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: show ip next-hop
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show ip next-hop", {
  func   = show_ip_next_hop_func,
  help   = "Showing of the next-hop table",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})
