
--********************************************************************************
--*              (C), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* arp_utils.lua
--*
--* DESCRIPTION:
--*       Any functions, variables etc that are required in several tests
--        and in maintaining the lua ARP DB.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- constants
NIL_UC_MAC_ADDRESS =   {[1] = "00", [2] = "00", [3] = "00", [4] = "00", [5] = "00", [6] = "00",
                        ["isMulticast"] = false, ["string"] = "00:00:00:00:00:00", 
                        ["isBroadcast"] = false, ["isUnicast"] = true}


--********************************************************************************
--
--  getTableArp
--        @description  Get a "pointer" to the global table which holds the lua ARP DB.
--                      Upon the first call to getTableArp() the global table is created
--                      and an empty table is created for every device number.
--
--        @param        none.       
--
--        @return       lua ARP DB table.
--
function getTableArp()
    if nil == global_arp_table then
        global_arp_table = {}
        local iterator, devNum
        local devlist = wrLogWrapper("wrlDevList")
        for iterator, devNum in pairs(devlist) do
            global_arp_table[devNum] = {}
        end
    end
    return global_arp_table
end

--********************************************************************************
--
--  arp_table_print
--        @description  Print the entire lua ARP DB.
--
--        @param        none.       
--
--        @return       none.
--
function arp_table_print()
    local arp_table = getTableArp()
    print("Dev","ARP index","MAC address","","Ref count","is Static")
    print("===","=========","===========","","=========","=========")
    for keyDev,valDev in pairs(arp_table) do
        for key, val in pairs(valDev) do
            print(keyDev,key,"",val.macAddr["string"],val.refCount,"",val.isStatic)
        end
    end
end

--********************************************************************************
--
--  arp_table_update
--        @description  Add or remove an entry to/from the lua ARP DB. 
--                      The lua ARP DB is a 2-D table, the first key is the device
--                      number and the second key is the ARP index. 
--
--        @param        params["arp_entry_index"]: ARP index of the entry. 
--                      params["mac-address"]: MAC address of the entry (in table form).
--                      params["all_device"]: Device number.
--                      params["is_static"]: identifies caller of function:
--                      equals 'true' if this entry was created statically using 'ip arp' command
--                      equals 'false' if this entry was created dynamically using 'ip next-hop'
--                      command for example.       
--
--        @return       none.
--
function arp_table_update(params)
    local arp_index    = params["arp_entry_index"]
    local mac_address  = params["mac-address"]
    local devNum       = params["all_device"]
    local is_static    = params["is_static"]
    local arp_table = getTableArp()
    local entryInfo = {}
    -- in case of 'no ip arp' remove entry from lua ARP DB
    if "00:00:00:00:00:00" == mac_address.string then
        arp_table[devNum][arp_index] = nil
    -- otherwise add an entry to the DB
    else
        entryInfo.arpIndex = arp_index
        entryInfo.macAddr = mac_address
        entryInfo.isStatic = is_static 
        if true == is_static then
            entryInfo.refCount = 0
        else
            entryInfo.refCount = 1
        end
        -- refCount is the number of nexthops that use this ARP entry
        arp_table[devNum][arp_index] = entryInfo
    end
end

--********************************************************************************
--
--  arp_table_index_lookup
--        @description  find the index of the ARP entry with the given MAC address.
--
--        @param        devNum:       Device number.
--                      mac_address:  MAC address of the ARP entry
--                                    (in table form).
--
--        @return       if matching entry is found its index is returned,
--                      otherwise -1 is returned.
--
function arp_table_index_lookup(devNum, mac_address)
    local arp_table = getTableArp()
    local key,val
    for key,val in pairs(arp_table[devNum]) do
        if val.macAddr.string == mac_address.string  then
            return key
        end
    end
    return -1
end

--********************************************************************************
--
--  arp_table_unused_index_get
--        @description  find the first index which is not used by an ARP entry
--                      (using the lua ARP DB) and return it.
--
--        @param        devNum:       Device number.
--
--        @return       if maximum ARP index resolution is successsful and an unused 
--                      ARP index exists this unsued index will be returned,
--                      otherwise, -1 will be returned.
--
function arp_table_unused_index_get(devNum)
    local arp_table = getTableArp()
    local max_index, arp_index = 0
    local result, values = wrLogWrapper("wrlCpssDxChMaximumRouterArpIndex","(devNum)",devNum)
    if 0 == result then
        max_index = values
    else
        print("MaximumRouterArpIndex failed")
        return -1
    end
    while nil ~= arp_table[devNum][arp_index] and arp_index <= max_index do
        arp_index = arp_index + 1
    end
    if arp_index > max_index then
        return -1
    else
        return arp_index
    end
end

--********************************************************************************
--
--  arp_entry_hw_write
--        @description  Write a given ARP entry to hardware using the CPSS API.
--                      Notice that this function does not perform any checks on the 
--                      input parameters prior to writing to hardware.
--
--        @param        devNum:       Device number.
--                      arp_index:    ARP entry index.
--                      mac_address:  MAC address for the ARP entry
--                                    (in table form).
--                      command_data: command data object from the function
--                                    calling arp_entry_hw_write().
--
--        @return       result, values (returned by 'cpssDxChIpRouterArpAddrWrite')
--
function arp_entry_hw_write(devNum, arp_index, mac_address, command_data)
    local result, values
    if true == command_data["status"] then
        command_data:clearDeviceStatus()
        -- Writing of ARP MAC address to the router ARP/Tunnel start Table.
        command_data:clearLocalStatus()
        if true == command_data["local_status"]     then
            result, values =
                myGenWrapper("cpssDxChIpRouterArpAddrWrite",{
                            { "IN",   "GT_U8",        "devNum", devNum},
                            { "IN",   "GT_U32",       "routerArpIndex", arp_index},
                            { "IN",   "GT_ETHERADDR", "arpMacAddr", mac_address }})
            if     0x10 == result then
                command_data:setFailDeviceStatus()
                command_data:addWarning("It is not allowed to write ARP " ..
                                        "MAC address to the router ARP/" ..
                                        "Tunnel start Table on device %d.",
                                        devNum)
            elseif 0x1E == result then
                command_data:setFailDeviceStatus()
                command_data:addError("Writing of ARP MAC address to " ..
                                        "the router ARP/Tunnel start " ..
                                        "Table does not supported on " ..
                                        "device %d: %s.", devNum,
                                        returnCodes[result])
            elseif 0 ~= result then
                command_data:setFailDeviceAndLocalStatus()
                command_data:addError("Error at writing of ARP MAC address " ..
                                        "to the router ARP/Tunnel start " ..
                                        "Table on device %d: %s.", devNum,
                                        returnCodes[result])
            end
        end
        command_data:updateStatus()
        command_data:updateDevices()
        command_data:addWarningIfNoSuccessDevices(
            "Can not write ARP MAC address to the router ARP/Tunnel start " ..
            "Table of all processed devices.")
    end
    return result, values
end

