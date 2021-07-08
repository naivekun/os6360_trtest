--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ipv6.lua
--*
--* DESCRIPTION:
--*       ipv6, ipv6_bc, ipv6_mc, ipv6_uc, ipv6_bc_mc type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- ipv6         - ipv6 address
-- ipv6_bc      - The broadcast ipv6 address
-- ipv6_bc_mc   - The BC / MC ipv6 address
-- ipv6_mc      - The multicast ipv6 address
-- ipv6_uc      - The unicast ipv6 address
--
do


-- ************************************************************************
---
-- Check parameter that it is in the correct form,
--     2190:0190:0000:0000:0000:0000:0000:0000
--     2190:0190::
-- @param param Parameter string
-- @param data Not used
--        @return    boolean check status
--        @return    true:paramList or false:error_string if failed
--        @return    The format of paramList paramList["string"], 
--                                           paramList["isUnicast"], 
--                                           paramList["isMulticast"], 
--                                           paramList["isBroadcast"]
--
local function check_param_ipv6(param, data)
	--[[s, e, ret[1], ret[2], ret[3], ret[4], ret[5], ret[6], ret[7], ret[8] = 
        string.find(param, "(%x?%x?%x?%x?)%:(%x?%x?%x?%x?)%:(%x?%x?%x?%x?)%:" .. 
                           "(%x?%x?%x?%x?)%:(%x?%x?%x?%x?)%:(%x?%x?%x?%x?)%:" ..
                           "(%x?%x?%x?%x?)%:(%x?%x?%x?%x?)")
    2190:0190:0000:0000:0000:0000:0000:0000]]--

    if string.match(param, "[:%x]+") ~= param then
        return false, "Wrong character in IPv6 address"
    end

    local ncolons, i
    ncolons = 0
    for i = 0,string.len(param) do
        if string.sub(param,i,i) == ":" then
            ncolons = ncolons+1
        end
    end
    if ncolons > 7 then
        return false, "too many colons in IPv6 address"
    end
    local param_c
    i = string.find(param, "::")
    if i ~= nil then
        param_c = string.sub(param,1,i) .. "0"
        while ncolons < 7 do
            param_c = param_c .. ":0"
            ncolons = ncolons+1
        end
        param_c = param_c .. string.sub(param, i+1)
        if string.find(param_c, "::") ~= nil then
            return false, "Double column must appear only once in IPv6 address"
        end
        if string.sub(param_c,1,1) == ":" then
            param_c = "0" .. param_c
        end
        if string.sub(param_c,-1) == ":" then
            param_c = param_c .. "0"
        end
    else
        if ncolons < 7 then
            return false, "no enough colongs in IPv6 address"
        end
        param_c = param
    end

    local ret = {addr={}}
    for i in string.gmatch(param_c,"(%x+)") do
        if string.len(i) > 4 then
            return false, "Wrong IPv6 address"
        end
        if tonumber(i, 16) > 65535 then
            return false, "Wrong IPv6 address"
        end
        table.insert(ret.addr, tonumber(i, 16))
    end
    
    ret["string"] = param_c
	
	ret["isBroadcast"]  = false
    ret["isUnicast"]    = false
    
    if 0xFF00 <= ret.addr[1] then
        ret["isMulticast"] = true
    else
        ret["isMulticast"] = false
    end
	
	return true, ret
end

CLI_type_dict["ipv6"] = {
    checker = check_param_ipv6,
    help = "IPv6 address"
}


-- ************************************************************************
---
--  check_param_ipv6_bc
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
local function check_param_ipv6_bc(param, data)
	local stat, tmp
	
	stat, tmp = check_param_ipv6(param, data)
	return stat and tmp["isBroadcast"], tmp
end

CLI_type_dict["ipv6_bc"] = {
    checker = check_param_ipv6_bc,
    help = "IPv6 broadcast address" 
}


-- ************************************************************************
---
--  check_param_ipv6_bc_mc
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
local function check_param_ipv6_bc_mc(param, data)
	local stat, tmp
	
	stat, tmp = check_param_ipv6(param, data)
    
    if true == tmp["isMulticast"] then
        return stat, tmp
    else
        return false, "It is not broadcast/multicast address"
    end
end

CLI_type_dict["ipv6_bc_mc"] = {
    checker = check_param_ipv6_bc_mc,
    help = "IPv6 broadcast/multicast address"
}


-- ************************************************************************
---
--  check_param_ipv6_mc
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
local function check_param_ipv6_mc(param, data)
	local stat, tmp
	
	stat, tmp = check_param_ipv6(param, data)
    
    if (true == tmp["isMulticast"]) and (false == tmp["isBroadcast"]) then
        return stat, tmp
    else
        return false, "It is not multicast address"
    end    
end

CLI_type_dict["ipv6_mc"] = {
    checker = check_param_ipv6_mc,
    help = "IPv6 multicast address"
}


-- ************************************************************************
---
--  check_param_ipv6_uc
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
local function check_param_ipv6_uc(param, data)
	local stat, tmp
	
	stat, tmp = check_param_ipv6(param, data)
	return stat and tmp["isUnicast"], tmp
end


-- ************************************************************************
---
--  check_param_ipv6_gw
--        @description  check gareway ip v6 address
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
local function check_param_ipv6_gw(param, data)
	local stat, tmp
	
	stat, tmp = check_param_ipv6(param, data)
    
    if (false == tmp["isMulticast"]) and (false == tmp["isBroadcast"]) then
        return stat, tmp
    else
        return false, "It is not multicast address"
    end    
end


-------------------------------------------------------
-- type registration: ipv6_mc
-------------------------------------------------------
CLI_type_dict["ipv6_mc"] = {
    checker = check_param_ipv6_mc,
    help = "IPv6 multicast address"
}

-------------------------------------------------------
-- type registration: ipv6_uc
-------------------------------------------------------
CLI_type_dict["ipv6_uc"] = {
    checker = check_param_ipv6_mc,
    help = "IPv6 unicast address"
}

-------------------------------------------------------
-- type registration: ipv6_gw
-------------------------------------------------------
CLI_type_dict["ipv6_gw"] = {
    checker = check_param_ipv6_gw,
    help = "IPv6 gateway address"
}

-------------------------------------------------------
-- type registration: ipv4_ipv6
-------------------------------------------------------
CLI_type_dict["ipv4_ipv6"] = 
{
    checker  = CLI_check_param_compound,
    complete = CLI_complete_param_compound,
    compound = { "ipv4", "ipv6" }                                      
}

-------------------------------------------------------
-- type registration: ipv4_ipv6_gw
-------------------------------------------------------
CLI_type_dict["ipv4_ipv6_gw"] = 
{
    checker  = CLI_check_param_compound,
    complete = CLI_complete_param_compound,
    compound = { "ipv4_gw", "ipv6_gw" }
}

end