--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ip_arp.lua
--*
--* DESCRIPTION:
--*       showing of the entries from the router ARP table
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_ip_arp_func
--        @description  shows the entries from the router ARP table
--
--        @param params         - params["all_device"]: all or given
--                                devices iterating property, could be
--                                irrelevant
--                                params["router_arp_index"]: router arp
--                                index
--
--        @return       true on success, otherwise false and error message
--
local function show_ip_arp_func(params)
    -- Common variables declaration
    local command_data = Command_Data()
    -- Command specific variables declaration
    local router_arp_index
    local table_info = {
        { head="Index",len=5,align="c",path="router_arp_index" },
        { head="Address",len=22,align="c",path="mac-address"}
    }

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceRange(params)
    command_data:initDeviceRouterArpEntries(params)

    command_data:initTable(table_info)

    -- Main Arp-entries handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, router_arp_index in
                                command_data:getRouterArpEntriesIterator() do
            command_data:clearEntryStatus()

            command_data:clearLocalStatus()

            command_data.router_arp_index = router_arp_index
            command_data:addTableRow(command_data)
            command_data:updateEntries()
        end
    end

    command_data:showTblResult("There is no ARP MAC address to show.")

    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: show ip arp
--------------------------------------------
CLI_addCommand("exec", "show ip arp", {
  func   = show_ip_arp_func,
  help   = "ARP information",
  params = {
      { type = "named",
          "#router_arp_index",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})
