--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* max_vlan_packet_size.lua
--*
--* DESCRIPTION:
--*       type of Maximum Vlan Packet size
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  check_max_vlan_packet_size
--        @description  checking of maximum vlan packed size
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter rang
--
--        @return       true and maximum vlan packed size on success, 
--                      otherwise false and error message
--
local function check_max_vlan_packet_size(param, name, desc)
    param = tonumber(param)
    
    if param == nil then
        return false, name .. " not a number"
    end
    
    if param ~= DEFAULT_RECOMENDED_MAXIMUM_VLAN_PACKET_SIZE then
        return false, "Typed Maximum Vlan Packet size is not allowed"
    end

    return true, param
end


-- ************************************************************************
---
--  complete_max_vlan_packet_size
--        @description  autocompleting of maximum vlan packed size
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter rang
--
--        @return       autocompleting and help array 
--
local function complete_max_vlan_packet_size(param, name, desc)
    local values = 
        {{DEFAULT_RECOMENDED_MAXIMUM_VLAN_PACKET_SIZE,  "Default recomended maximum received packet size"}}
    local compl = {}
    local help  = {}
    local k
    
    for k = 1, #values do
        if prefix_match(param, tostring(values[k][1])) then
            table.insert(compl, tostring(values[k][1]))
            help[#compl] = values[k][2]
        end
    end
    
    return compl, help
end


--------------------------------------------
-- type registration: max_vlan_packet_size
--------------------------------------------
CLI_type_dict["max_vlan_packet_size"] = {
    checker = check_max_vlan_packet_size,
    complete = complete_max_vlan_packet_size,
    help = "Maximum Vlan Packet size"
}

