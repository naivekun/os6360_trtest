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
--*       The test for ARP entries, configures several ARP entries using 'ip arp'
--*       and 'ip next-hop' lua commands, verifies that the lua ARP DB has identical
--*       entries to those found in HW.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local error = false

--********************************************************************************
--
--  show_ip_arp_test
--        @description  tests 'show ip arp' command by comparing
--                      displayed values from CPSS and lua ARP DB.
--
--        @param        none.
--
--        @return       none.
--
local debug_on = false

local function _debug(debugString)
    if debug_on == true then
        print (debugString)
    end
end

local function show_ip_arp_test()
    -- Common variables declaration
    local command_data = Command_Data()
    local result, values
    -- Command specific variables declaration
    local macAddr_fromHW
    local macAddr_fromDB
    local ArpIteratorParams = {}
    local message_string, apiName
    local arp_table = getTableArp()
    local device_index, device_table = {}
    local iterator, devNum, arp_index, arp_entry = {}
    -- Command variables initialization
    ArpIteratorParams["all_device"] = "all"
    ArpIteratorParams["router_arp_index"] = nil
    ArpIteratorParams["arp_entry_index"] = nil
    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceRange(ArpIteratorParams)
    command_data:initDeviceRouterArpEntries(ArpIteratorParams)
    -- step 1: verify that all the entries in the lua ARP DB exist in the hardware
    -- iterate over all device tables in the lua ARP DB
    for device_index, device_table in pairs(arp_table) do
        -- for every device iterate over its ARP entries
        for arp_index, arp_entry in pairs(device_table) do
            -- get the matching ARP entry from hardware
            apiName = "cpssDxChIpRouterArpAddrRead"
            result, values = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",                        "devNum",  device_index },
                { "IN",     "GT_U32",                       "routerArpIndex",  arp_entry.arpIndex },
                { "OUT",    "GT_ETHERADDR",                 "arpMacAddr"},
            })
            _debug(apiName .. "   result = " .. result)
            if  0 == result        then
                macAddr_fromHW = tostring(values["arpMacAddr"])
                printStructure("",values["arpMacAddr"])
                macAddr_fromDB = arp_entry.macAddr.string
                -- compare the MAC from the lua ARP DB with the matching ARP entry in the hardware
                message_string = "mac address of entry "..tostring(arp_entry.arpIndex)
                ..", device "..tostring(device_index)
                check_expected_value(message_string, macAddr_fromDB, macAddr_fromHW)
            elseif 0x10 == result  then
                command_data:setFailDeviceStatus()
                command_data:addWarning("Arp mac-address getting " ..
                                                        "is not allowed on " ..
                                                        "device %d.", device_index)
            elseif    0 ~= result  then
                command_data:setFailDeviceAndLocalStatus()
                command_data:addError("Error at arp mac-address " ..
                                                      "getting on device %d: %s.",
                                                      device_index, returnCodes[result])
            end
        end
    end
--[[
-- step 2: verify that all the entries in the hardware exist in the lua ARP DB
    -- iterate over all ARP entries in the hardware using the ARP entries iterator
    if true == command_data["status"] then
        for iterator, devNum, arp_index in command_data:getRouterArpEntriesIterator() do
            -- for every ARP entry retrieved by the iterator, read its MAC using this API
            apiName = "cpssDxChIpRouterArpAddrRead"
            result, values = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",                        "devNum", devNum },
                { "IN",     "GT_U32",                       "routerArpIndex", arp_index },
                { "OUT",    "GT_ETHERADDR",                 "arpMacAddr"},
            })
            _debug(apiName .. "   result = " .. result)
            if  0 == result        then
                macAddr_fromHW = tostring(values["arpMacAddr"])
                if nil ~= arp_table[devNum][arp_index] then
                    macAddr_fromDB = arp_table[devNum][arp_index].macAddr.string
                else
                    macAddr_fromDB = ""
                end
                -- compare the MAC address from the hardware with the matching ARP entry in the lua ARP DB
                message_string = "mac address of entry "..tostring(arp_index)..", device "..tostring(devNum)
                check_expected_value(message_string, macAddr_fromHW, macAddr_fromDB)
                command_data:clearEntryStatus()
                command_data:clearLocalStatus()
            elseif 0x10 == result  then
                command_data:setFailDeviceStatus()
                command_data:addWarning("Arp mac-address getting " ..
                                                    "is not allowed on " ..
                                                    "device %d.", devNum)
            elseif    0 ~= result  then
                command_data:setFailDeviceAndLocalStatus()
                command_data:addError("Error at arp mac-address " ..
                                                  "getting on device %d: %s.",
                                                  devNum, returnCodes[result])
            end
        end
    end]]--
end

executeLocalConfig("dxCh/examples/configurations/ip_arp.txt")

show_ip_arp_test()

executeLocalConfig("dxCh/examples/configurations/ip_arp_deconfig.txt")
