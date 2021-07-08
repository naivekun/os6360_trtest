--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ipv4.lua
--*
--* DESCRIPTION:
--*       ipv4, ipv4_bc, ipv4_mc, ipv4_uc, ipv4_bc_mc type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 8 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- ipv4         - IPv4 address
-- ipv4_bc      - The broadcast IPv4 address
-- ipv4_bc_mc   - The BC / MC IPv4 address
-- ipv4_mc      - The multicast IPv4 address
-- ipv4_uc      - The unicast IPv4 address
--
do


-- ************************************************************************
---
--  check_param_ipv4
--        @description  Check parameter that it is in the correct form,
--                      xxx.xxx.xxx.xxx
--
--        @param param          - Parameter string
--        @param data           - Not used
--
--        @return    boolean check status
--        @return    true:paramList or false:error_string if failed
--        @return    The format of paramList paramList["string"],
--                                           paramList["isUnicast"],
--                                           paramList["isMulticast"],
--                                           paramList["isBroadcast"]
--
local function check_param_ipv4(param, data)
	local ret
	local s, e, i

	ret = {}

	s, e, ret[1], ret[2], ret[3], ret[4] =
        string.find(param, "(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)")

	if (type(s) == "nil") then
		return false, "Invalid IP address"
	end
	if (s ~= 1) or (e ~= string.len(param)) then
		return false, "Invalid IP address"
	end

	ret["isBroadcast"] = true
	for i = 1, 4, 1 do
		if (tonumber(ret[i]) > 255) then
			return false, "Octet in IP address cannot be above 255"
		end
		if (tonumber(ret[i]) ~= 255) then
			ret["isBroadcast"] = false
		end
	end

	ret["string"] = param
	if (((tonumber(ret[1]) > 223) and (tonumber(ret[1]) < 241)) or (ret["isBroadcast"] == true)) then
		ret["isMulticast"] = true
		ret["isUnicast"] = false
	else
		ret["isMulticast"] = false
		ret["isUnicast"] = true
	end

	return true, ret
end


-- ************************************************************************
---
--  complete_param_ipv4
--        @description  complete ip v4
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--
--        @return       complete and help lists
--
local function complete_param_ipv4(param, name, desc)
    local values =
        {{"192.168.0.1",   "Default NAT ip"}}
    return complete_param_with_predefined_values(param,name,desc,values)
end


CLI_type_dict["ipv4"] = {
    checker     = check_param_ipv4,
    complete    = complete_param_ipv4,
    help        = "IPv4 address"
}


-- ************************************************************************
---
--  check_param_ipv4_bc
--        @description  Check parameter that it is in the correct form,
--                      xxx.xxx.xxx.xxx & BC
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
local function check_param_ipv4_bc(param, data)
	local stat, tmp

	stat, tmp = check_param_ipv4(param, data)
	return stat and tmp["isBroadcast"], tmp
end

CLI_type_dict["ipv4_bc"] = {
    checker = check_param_ipv4_bc,
    help = "IPv4 broadcast address"
}


-- ************************************************************************
---
--  check_param_ipv4_bc_mc
--        @description  Check parameter that it is in the correct form,
--                      xxx.xxx.xxx.xxx & BC / MC
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
local function check_param_ipv4_bc_mc(param, data)
	local stat, tmp

	stat, tmp = check_param_ipv4(param, data)

    if true == tmp["isMulticast"] then
        return stat, tmp
    else
        return false, "It is not multicast address"
    end
end

CLI_type_dict["ipv4_bc_mc"] = {
    checker = check_param_ipv4_bc_mc,
    help = "IPv4 broadcast/multicast address"
}


-- ************************************************************************
---
--  check_param_ipv4_mc
--        @description  Check parameter that it is in the correct form,
--                      xxx.xxx.xxx.xxx & MC
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
local function check_param_ipv4_mc(param, data)
	local stat, tmp

	stat, tmp = check_param_ipv4(param, data)

    if (true == tmp["isMulticast"]) and (false == tmp["isBroadcast"]) then
        return stat, tmp
    else
        return false, "It is not multicast address"
    end
end

CLI_type_dict["ipv4_mc"] = {
    checker = check_param_ipv4_mc,
    help = "IPv4 multicast address"
}


-- ************************************************************************
---
--  check_param_ipv4_uc
--        @description  Check parameter that it is in the correct form,
--                      xxx.xxx.xxx.xxx & UC
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
local function check_param_ipv4_uc(param, data)
	local stat, tmp

	stat, tmp = check_param_ipv4(param, data)
    if not stat then
        return stat, tmp
    end
    if not tmp["isUnicast"] then
        return false, "not an unicast address"
    end
	return stat, tmp
end


-- ************************************************************************
---
--  check_param_ipv4_gw
--        @description  check gareway ip v4 address
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
local function check_param_ipv4_gw(param, data)
	local stat, tmp

	stat, tmp = check_param_ipv4(param, data)

    if (false == tmp["isMulticast"]) and (false == tmp["isBroadcast"]) then
        return stat, tmp
    else
        return false, "It is not multicast address"
    end
end


-------------------------------------------------------
-- type registration: ipv4_uc
-------------------------------------------------------
CLI_type_dict["ipv4_uc"] = {
    checker = check_param_ipv4_uc,
    complete    = complete_param_ipv4,
    help = "The ipv4 unicast address"
}

-------------------------------------------------------
-- type registration: ipv4_gw
-------------------------------------------------------
CLI_type_dict["ipv4_gw"] = {
    checker = check_param_ipv4_gw,
    help = "The ipv4 gateway address"
}

CLI_type_dict["l4port"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=0,
    max=65535,
    help = "Layer 4 port (0-65535)"
}

end

-----------------------------------------------------------------------------------
-- IPV4 Prefix support
-----------------------------------------------------------------------------------

--------------------------------------------------------------------------------------------------
-- ************************************************************************
---
--  check_param_ipv4_prefix
--        @description  Check parameter that it is in the correct form,
--                      xxx.xxx.xxx.xxx/ll
--
--        @param param          - Parameter string
--        @param data           - Not used
--
--        @return    boolean check status
--        @return    true:paramList or false:error_string if failed
--        @return    The format of paramList paramList["string"],
--                                           paramList["isUnicast"],
--                                           paramList["isMulticast"],
--                                           paramList["isBroadcast"]
--
local function check_param_ipv4_prefix(param, data)
	local ret;
	local s, e, l, i;

	ret = {binary = {}};

	s, e, ret[1], ret[2], ret[3], ret[4], l =
        string.find(param, "(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)/(%d?%d?)");

	if (type(s) == "nil") then
		return false, "Invalid IPV4 address prefix";
	end
	if (s ~= 1) or (e ~= string.len(param)) then
		return false, "Invalid IPV4 address prefix";
	end

    ret["length"] = tonumber(l);
    if ret["length"] > 32 or ret["length"] <= 0 then
		return false, "Invalid IPV4 address prefix length out of 1..32 range";
    end

	for i = 1, 4, 1 do
        ret.binary[i] = tonumber(ret[i]);
		if (ret.binary[i] > 255) then
			return false, "Octet in IPV4 Address cannot be above 255";
		end
	end

	ret["string"] = param;

	if (((tonumber(ret[1]) > 223) and (tonumber(ret[1]) < 241))
        or (ret["isBroadcast"] == true))
    then
		ret["isMulticast"] = true;
		ret["isUnicast"] = false;
	else
		ret["isMulticast"] = false;
		ret["isUnicast"] = true;
	end

	return true, ret
end

local function check_param_ipv4_mc_prefix(param, data)
	local bool_rc, ret = check_param_ipv4_prefix(param, data);
    if not bool_rc then
        return bool_rc, ret;
    end
    if not ret["isMulticast"] then
        return false, "IPV4 Address is not Multicast";
    end
    return bool_rc, ret;
end

local function check_param_ipv4_uc_prefix(param, data)
    local bool_rc, ret = check_param_ipv4_prefix(param, data);
    if not bool_rc then
        return bool_rc, ret;
    end
    if not ret["isUnicast"] then
        return false, "IPV4 Address is not Unicast";
    end
    return bool_rc, ret;
end

CLI_type_dict["ipv4_prefix"] = {
    checker = check_param_ipv4_prefix,
    help = "IPV4 Address Prefix"
}

CLI_type_dict["ipv4_mc_prefix"] = {
    checker = check_param_ipv4_mc_prefix,
    help = "IPV4 Multicast Address Prefix"
}

CLI_type_dict["ipv4_uc_prefix"] = {
    checker = check_param_ipv4_uc_prefix,
    help = "IPV4 Unicast Address Prefix"
}



