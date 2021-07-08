--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mac.lua
--*
--* DESCRIPTION:
--*       mac-address, mac-address-mc, mac-address-uc, mac-address-bc,
--*       mac-address-bc-mc type definitions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- mac-address         - MAC address
-- mac-address-bc      - The broadcast MAC address
-- mac-address-bc-mc   - The BC / MC MAC address
-- mac-address-mc      - The multicast MAC address
-- mac-address-uc      - The unicast MAC address
--
-- ************************************************************************
---
--  check_param_mac
--        @description  Check parameter that it is in the correct form,
--                      xx:xx:xx:xx:xx:xx
--
--        @param param          - Parameter string
--        @param data           - Not used
--
--        @return       boolean check status
--        @return       true:paramList or false:error_string if failed
--        @return       The format of paramList paramList["string"],
--                                              paramList["isUnicast"],
--                                              paramList["isMulticast"],
--                                              paramList["isBroadcast"]
--
function check_param_mac(param, data)
	local ret
	local i, s, e, val

	ret = {}
	tmp = {}

	if (type(param)=="string") then param=string.upper(param) end
	s, e, ret[1], ret[2], ret[3], ret[4], ret[5], ret[6] = string.find(param, "(%x%x):(%x%x):(%x%x):(%x%x):(%x%x):(%x%x)")

    if (type(s) == "nil") then
  	    s, e, ret[1], ret[2], ret[3], ret[4], ret[5], ret[6] = string.find(param, "(%x%x)(%x%x)(%x%x)(%x%x)(%x%x)(%x%x)")
    end

	if (type(s) == "nil") then
		return false, "This is not a valid mac address"
	end

	if (s ~= 1) or (e ~= string.len(param)) then
		return false, "Invalid MAC address"
	end


	ret["string"] = param
	if ((tonumber(ret[1], 16) % 2) == 1) then
		ret["isUnicast"] = false
		ret["isMulticast"] = true
		ret["isBroadcast"] = true
		for i = 1,6,1 do
			if (ret[i] ~= "FF") then
				ret["isBroadcast"] = false
			end
		end
	else
		ret["isUnicast"] = true
		ret["isMulticast"] = false
		ret["isBroadcast"] = false
	end

	return true, ret
end


-- ************************************************************************
---
--  complete_param_mac
--        @description  complete mac-address value
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
function complete_param_mac(param, name, desc)
    local values =
        {{"00:00:00:00:00:00",  "Unused mac-address"}}
    return complete_param_with_predefined_values(param,name,desc,values)
end


CLI_type_dict["mac-address"] = {
    checker = check_param_mac,
    complete    = complete_param_mac,
    help = "MAC address"
}


-- ************************************************************************
---
--  check_param_mac_bc
--        @description  Check parameter that it is in the correct form,
--                      xx:xx:xx:xx:xx:xx & BC
--
--        @param param          - Parameter string
--        @param data           - Not used
--
--        @return       boolean check status
--        @return       true:paramList or false:error_string if failed
--        @return       The format of paramList paramList["string"],
--                                              paramList["isUnicast"],
--                                              paramList["isMulticast"],
--                                              paramList["isBroadcast"]
--
local function check_param_mac_bc(param, data)
	local stat, tmp

	stat, tmp = check_param_mac(param, data)
	return stat and tmp["isBroadcast"], tmp
end

CLI_type_dict["mac-address-bc"] = {
    checker = check_param_mac_bc,
    help = "MAC broadcast address"
}


-- ************************************************************************
---
--  check_param_mac_bc_mc
--        @description  Check parameter that it is in the correct form,
--                      xx:xx:xx:xx:xx:xx & BC / MC
--
--        @param param          - Parameter string
--        @param data           - Not used
--
--        @return       boolean check status
--        @return       true:paramList or false:error_string if failed
--        @return       The format of paramList paramList["string"],
--                                              paramList["isUnicast"],
--                                              paramList["isMulticast"],
--                                              paramList["isBroadcast"]
--
local function check_param_mac_bc_mc(param, data)
	local stat, tmp

	stat, tmp = check_param_mac(param, data)
	return stat and tmp["isMulticast"], tmp
end

CLI_type_dict["mac-address-bc-mc"] = {
    checker = check_param_mac_bc_mc,
    help = "MAC broadcast/multicast address"
}


-- ************************************************************************
---
--  check_param_mac_mc
--        @description  Check parameter that it is in the correct form,
--                      xx:xx:xx:xx:xx:xx & MC
--
--        @param param          - Parameter string
--        @param data           - Not used
--
--        @return       boolean check status
--        @return       true:paramList or false:error_string if failed
--        @return       The format of paramList paramList["string"],
--                                              paramList["isUnicast"],
--                                              paramList["isMulticast"],
--                                              paramList["isBroadcast"]
--
local function check_param_mac_mc(param, data)
	local stat, tmp

	stat, tmp = check_param_mac(param, data)

    if (true == tmp["isMulticast"]) and (false == tmp["isBroadcast"]) then
        return stat, tmp
    else
        return false, "It is not multicast address"
    end
end

CLI_type_dict["mac-address-mc"] = {
    checker = check_param_mac_mc,
    help = "MAC multicast address"
}


-- ************************************************************************
---
--  functions
--        @description  Check parameter that it is in the correct form,
--                      xx:xx:xx:xx:xx:xx & UC
--
--        @param param          - Parameter string
--        @param data           - Not used
--
--        @return       boolean check status
--        @return       true:paramList or false:error_string if failed
--        @return       The format of paramList paramList["string"],
--                                              paramList["isUnicast"],
--                                              paramList["isMulticast"],
--                                              paramList["isBroadcast"]
--
local function check_param_mac_uc(param, data)
	local stat, tmp

	stat, tmp = check_param_mac(param, data)
	if not stat then
        return stat, tmp
    end
    if not tmp["isUnicast"] then
        return false, "Not an unicast mac"
    end
    return stat, tmp
end

-- ************************************************************************
---
--  complete_param_mac_uc
--        @description  complete mac-address value
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
local function complete_param_mac_uc(param, name, desc)
    local values =
        {{"00:00:00:00:00:01",  "Unicast mac-address"}}
    return complete_param_with_predefined_values(param,name,desc,values)
end

CLI_type_dict["mac-address-uc"] = {
    checker = check_param_mac_uc,
    complete = complete_param_mac_uc,
    help = "MAC unicast address"
}

-- zero-pads mac-prefix and checks the result mac
local function check_param_mac_prefix(param, data)
    local param1;
    local zeroes = "00:00:00:00:00:00";

    if type(param) == "string" then
        local len_p = string.len(param);
        local len_z = string.len(zeroes);
        param1 = param .. string.sub(zeroes, (len_p + 1), len_z);
    else
        param1 = param;
    end
    return check_param_mac(param1, data);
end

CLI_type_dict["mac-address-prefix"] = {
    checker = check_param_mac_prefix,
    help = "MAC address prefix padded by zeroes"
}

