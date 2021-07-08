--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_arp.lua
--*
--* DESCRIPTION:
--*       Writing and deleting of the MAC address to the router ARP table.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants

-- ************************************************************************
--
--  ip_arp_func
--        @description  Writes the MAC address to the router ARP table after 
--                      performing a few checks against the lua ARP DB.
--
--        @param        params["all_device"]: device number.
--                      params["arp_entry_index"]: index of the ARP entry.
--                      params["mac-address"]: MAC address value
--
--        @return       true on success, otherwise false and error message
--
local function ip_arp_func(params)
    -- Common variables declaration
    local result, values
    local devNum, iterator
    local command_data = Command_Data()
    -- Command specific variables declaration
    local arp_index, mac_address
    local arp_table_params = {}
    local arp_table = getTableArp()
    local done
    -- Common variables initialization
    command_data:initAllDeviceRange(params)
    -- Command specific variables initialization
    arp_index      = params["arp_entry_index"]
    mac_address    = params["mac-address"]
    -- Iterate over devices specified in the command params
    if true == command_data["status"] then
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()
            command_data:clearLocalStatus()
            done = false
            if true == command_data["local_status"]     then
                -- Check if trying to delete non-existing entry
                if "00:00:00:00:00:00" == mac_address.string and nil == arp_table[devNum][arp_index] then
                    printf("Error deleting ARP entry %d on device %d: such entry does not exist.",arp_index,devNum)
                    done = true
                end
                -- Check if trying to delete an entry used by a next-hop
                if not done then
                    if "00:00:00:00:00:00" == mac_address.string and 0 ~= arp_table[devNum][arp_index].refCount then
                        printf("Error deleting ARP entry %d on device %d: this entry is associated with a next-hop entry.",arp_index,devNum)
                        done = true
                    end
                end
                -- Check if trying to add the same ARP entry twice
                if not done then
                    if nil ~= arp_table[devNum][arp_index] then
                       if mac_address.string == arp_table[devNum][arp_index].macAddr.string then
                           printf("Error creating ARP entry %d on device %d: this entry already exists.",arp_index,devNum)
                           done = true
                       end
                    end
                end
                -- If checks passed write ARP entry to HW
                if not done then
                    result, values = arp_entry_hw_write(devNum, arp_index, mac_address, command_data)
                    if      0 == result then
                        -- operation succeeded, update the lua ARP DB
                        arp_table_params = params
                        arp_table_params["is_static"] = true
                        arp_table_params["all_device"] = devNum
                        arp_table_update(arp_table_params) 
                    end  
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

--------------------------------------------------------
-- command registration: ip arp
--------------------------------------------------------
CLI_addCommand("config", "ip arp",
{
    func   = ip_arp_func,
    help   = "ARP configuration",
    params =
    {
        {type = "named",
        "#all_device",
        {format = "index %arp_entry_index", name = "arp_entry_index", help = "ARP entry index"},
        {format = "mac %mac-address", name = "mac-address", help = "ARP entry MAC address"},
        mandatory = {"all_device","arp_entry_index","mac-address"}}
    }
})
---------------------------------------------------------
-- command registration: no ip arp
--------------------------------------------------------
-- 'no ip arp' is simply an 'ip arp' with MAC address 00:00:00:00:00:00
CLI_addCommand("config", "no ip arp",
{
    func   =    function(params)
                    params["mac-address"] = NIL_UC_MAC_ADDRESS
                    return ip_arp_func(params)
                end,
    help   = "Delete ARP entry",
    params =
    {
        {type = "named",
        "#all_device",
        {format = "index %arp_entry_index", name = "arp_entry_index", help = "ARP entry index"},
        mandatory = {"all_device","arp_entry_index"}}
    }
})

