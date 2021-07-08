--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_list.lua
--*
--* DESCRIPTION:
--*       vlanRange type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 7 $
--*
--********************************************************************************

--includes

--constants

do
-- ************************************************************************
---
--  add_vlan_to_list
--        @description  Adds the vlan from start to stop in to the table
--                      tab
--
--        @param tab            - The table to add the ports to, they can
--                                be accessed using by
--                               tab["vlanRange"][1]...tab["vlanRange"][n]]
--        @param start          - The string from which to retrieve the
--                                start of the vlan range
--        @param stop           - The string from which to retrieve the
--                                ending of the vlan range
--        @param unchecked_vlans
--                              - unexisted vlans array
--        @param vlan_checking_function
--                              - function for vlan checking (by default -
--                                does_vlan_exists)
--
--        @return       boolean check status
--
local function add_vlan_to_list(tab, start, stop, unchecked_vlans, vlan_checking_function)
    local i, index, devId, vlan_exists
    local dev_range = wrLogWrapper("wrlDevList")

	if (stop < start) then
		start, stop = stop, start
	end

		if type(tab) == "nil" then
			tab = {}
		end

    if nil == vlan_checking_function    then
        vlan_checking_function  = does_vlan_exists
    end

    for i = start, stop do
        if nil ~= unchecked_vlans   then
            vlan_exists = false
            for index, devId in pairs(dev_range) do
                if vlan_checking_function(devId, i) then
                    vlan_exists = true
                    break
                end
            end

            if false == vlan_exists then
                table.insert(unchecked_vlans, i)
            end
        end

		table.insert(tab, i)
	end
end


-- ************************************************************************
---
--  check_param_vlan_list
--        @description  Check vlan parameter that it is in the correct form
--
--        @param param          - parameter string
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--        @param vlan_checking_function
--                              - function for vlan checking (by default -
--                                does_vlan_exists)
--
--        @return       boolean check status
--        @return       number:paramList or string:error_string if failed
--        @return       The format of paramList
--                          paramList["vlanRange"][1]...
--                          paramList["vlanRange"][n]
--
local function check_param_vlan_list(param, name, desc, vlan_checking_function, params)
	local ret
	local i, c, state, digits, start, stop
    local unchecked_vlans = {}

	--Initialize parameters
	s = 1
	state = 1
	devID = 0
	digits = 0
	start = 0
	stop = 0
	ret = {}
	--Add a terminator just to prevent an additional check
	param = param .. ","
	for i = 1,string.len(param) do
		c = string.sub(param, i, i)
		digits = digits + 1
		if (state == 1) then
			--Check the first part of the port range format, or if it is a singular port
			if type(tonumber(c)) ~= "nil" then
				--The digit is valid, add it
				start = (start * 10) + tonumber(c)
			elseif ((c == "-") and (digits > 1)) then
				--It is a range, change state to 2
				state = 2
				digits = 0
			elseif (((c == ",") and (digits > 1)) and ((start >= 0) and (start < max_vlanID(params["all-device"])+1))) then
				--It is a singular VLAN add it to the list
                add_vlan_to_list(ret, start, start, unchecked_vlans, vlan_checking_function)
				state = 1
				start = 0
				stop = 0
				digits = 0
			else
				--There was an error, why?
				if (start < 0) then
					return false, "VLAN ID cannot be below 0"
				elseif (start > max_vlanID(params["all-device"])) then
					return false, "VLAN ID cannot be above "..max_vlanID(params["all-device"])
				else
					return false, "Bad format"
				end
			end
		elseif (state == 2) then
			--Check that the stop number is valid
			if type(tonumber(c)) ~= "nil" then
				--The digit is valid, add it
				stop = (stop * 10) + tonumber(c)
			elseif (((c == ",") and (digits > 1)) and ((stop >= 0) and (stop < max_vlanID(params["all-device"])+1))) then
				--It is a vlan range, add it to the range and switch back to state 1
                add_vlan_to_list(ret, start, stop, unchecked_vlans, vlan_checking_function)
				state = 1
				start = 0
				stop = 0
				digits = 0
			else
				if (stop < 0) then
					return false, "VLAN ID cannot be below 0"
				elseif (stop > max_vlanID(params["all-device"])) then
					return false, "VLAN ID cannot be above "..max_vlanID(params["all-device"])
				else
					return false, "Bad format"
				end
			end
		end
	end

    if isEmptyTable(unchecked_vlans) then
	return true, ret
    else
        return false, "Vlan(s) " .. itemsStrGet(unchecked_vlans) ..
                      " do not exist on all avaible devices"
    end
end


-- ************************************************************************
---
--  check_param_configurable_vlan_list
--        @description  Check new or reconfigurable vlan parameter that it
--                      is in the correct form
--
--        @param param          - parameter string
--
--        @return       boolean check status
--        @return       number:paramList or string:error_string if failed
--        @return       The format of paramList
--                          paramList["vlanRange"][1]...
--                          paramList["vlanRange"][n]
--
local function check_param_configurable_vlan_list(param, name, desc, varray, params)
    local status, ret

    status, ret = check_param_vlan_list(param, name, desc, is_vlan_active, params)

    if true == status then
        if true == isItemInTable(0, ret) then
            return false, "It is not allowed to create Vlan 0"
        end
    end

    return status, ret
end


-------------------------------------------------------
-- type registration: vlanRange
-------------------------------------------------------
CLI_type_dict["vlanRange"] = {
    checker = check_param_vlan_list,
    help = "Enter VLAN list"
}

-------------------------------------------------------
-- type registration: vlan-range
-------------------------------------------------------
CLI_type_dict["vlan-range"] = {
    checker = check_param_vlan_list,
    help = "Enter VLAN list"
}

-------------------------------------------------------
-- type registration: configurable-vlan-range
-------------------------------------------------------
CLI_type_dict["configurable-vlan-range"] = {
    checker = check_param_configurable_vlan_list,
    help = "Enter VLAN list"
}
end
