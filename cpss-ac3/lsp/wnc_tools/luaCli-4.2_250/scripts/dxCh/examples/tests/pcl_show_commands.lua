--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pcl_show_commands.lua
--*
--* DESCRIPTION:
--*       The test for PCL Show commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--------------------------------------------------------------------------------
NOT_SUPPORTED_DEV_DECLARE(
    devNum, "CPSS_PP_FAMILY_CHEETAH_E",
    "CPSS_PP_FAMILY_CHEETAH2_E", "CPSS_PP_FAMILY_CHEETAH3_E")

local function rule_delete_api_call(dev, ruleIndex, ruleSize)
    local ret, val;
    ret,val = myGenWrapper("cpssDxChPclRuleInvalidate",{
        {"IN","GT_U8","devNum",dev},
        {"IN",TYPE["ENUM"],"ruleSize",ruleSize},
        {"IN","GT_U32","ruleIndex", ruleIndex}
    });
    if ret ~= 0 then
        print("cpssDxChPclRuleInvalidate failed\n");
    end
end

local function rule_delete_std_rule(dev, ruleIndex)
    rule_delete_api_call(dev, ruleIndex, 0); --"CPSS_PCL_RULE_SIZE_STD_E"
end

local function rule_delete_ext_rule(dev, ruleIndex)
    rule_delete_api_call(dev, ruleIndex, 1); --"CPSS_PCL_RULE_SIZE_EXT_E"
end

local function rule_set_api_call(
    dev, ruleFormat, patternAndMaskType, ruleIndex, ruleMsk, rulePattern, actionSTC)
    local ret;
            ret = myGenWrapper("cpssDxChPclRuleSet",{
            {"IN","GT_U8","devNum",dev},
            {"IN",TYPE["ENUM"],"ruleFormat",ruleFormat},  --Ext UDB
            {"IN","GT_U32","ruleIndex",ruleIndex},   -- index of the rule
            {"IN",TYPE["ENUM"],"ruleOptionsBmp",0}, --Rule is valid
            {"IN",patternAndMaskType,"maskPtr",ruleMsk},  -- The ext UDB rules
            {"IN",patternAndMaskType,"patternPtr",rulePattern},  --The ext UDB mask
            {"IN","CPSS_DXCH_PCL_ACTION_STC","actionPtr",actionSTC} -- The action to be taken (permit/deny)
        })

        if ret ~= 0 then
            print(
                "cpssDxChPclRuleSet failed," ..
                " index: " .. to_string(ruleIndex) ..
                " format: " .. to_string(ruleFormat) .. "\n");
        end
end

local function get_permit_action()
    local actionSTC;
    actionSTC = {mirror={cpuCode="CPSS_NET_ETH_BRIDGED_LLT_E"}};
    actionSTC["pktCmd"] = 0;
    return actionSTC;
end

local function get_deny_action()
    local actionSTC;
    actionSTC = {mirror={cpuCode="CPSS_NET_ETH_BRIDGED_LLT_E"}};
    actionSTC["pktCmd"] = 3;
    return actionSTC;
end

local function get_action_by_str(action_str)
    if action_str == "permit" then
        return get_permit_action();
    elseif action_str == "deny" then
        return get_deny_action();
    else
        print("action should be permit or deny\n");
        return nil;
    end
end

local function mac_str_to_null_based_array(mac_str)
	local str_arr, num_arr;
	local i, s, e

	if (type(mac_str) ~= "string") then
        print("mac_str is not string\n");
        return nil;
    end

    mac_str = string.upper(mac_str);

    str_arr = {};

	s, e, str_arr[0], str_arr[1], str_arr[2], str_arr[3], str_arr[4], str_arr[5] =
        string.find(mac_str, "(%x%x):(%x%x):(%x%x):(%x%x):(%x%x):(%x%x)");

    if (type(s) == "nil") then
        s, e, str_arr[0], str_arr[1], str_arr[2], str_arr[3], str_arr[4], str_arr[5] =
            string.find(mac_str, "(%x%x)(%x%x)(%x%x)(%x%x)(%x%x)(%x%x)");
    end

	if (type(s) == "nil") then
        print("mac_str parsing error1\n");
		return nil;
	end

	if (s ~= 1) or (e ~= string.len(mac_str)) then
        print("mac_str parsing error2\n");
		return nil;
	end

    local ruleStc={}

    num_arr = {};
    for i = 0,5 do
        num_arr[i] = tonumber(str_arr[i],16);
    end

    return num_arr;
end

-- used both for pattern and for pask
-- both std_not_ip and ext_not_ipv6
local function get_rule_data(pcl_id, mac_sa, mac_da)
    local rule = {};
    rule["common"] = {};
    if pcl_id then
        rule["common"]["pclId"] = pcl_id;
    end
    if mac_sa then
        rule["macSa"] = mac_str_to_null_based_array(mac_sa);
    end
    if mac_da then
        rule["macDa"] = mac_str_to_null_based_array(mac_da);
    end

    return rule;
end


local function ext_not_ipv6_rule_set(
    dev, ruleIndex, action_str,
    pclIdMsk, pclIdPattern,
    macSaMsk, macSaPattern,
    macDaMsk, macDaPattern)
    local actionSTC, ruleMsk, rulePattern;
    local patternAndMaskType = "CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC";
    local ruleFormat = 4; --"CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E";

    if pclIdPattern and (pclIdMsk == nil) then pclIdMsk = 0x3FF end
    if macSaPattern and (macSaMsk == nil) then macSaMsk = "FF:FF:FF:FF:FF:FF" end
    if macDaPattern and (macDaMsk == nil) then macDaMsk = "FF:FF:FF:FF:FF:FF" end
    actionSTC = get_action_by_str(action_str);
    if actionSTC == nil then
        return;
    end
    ruleMsk = get_rule_data(pclIdMsk, macSaMsk, macDaMsk);
    rulePattern = get_rule_data(pclIdPattern, macSaPattern, macDaPattern);
    rule_set_api_call(
        dev, ruleFormat, patternAndMaskType, ruleIndex, ruleMsk, rulePattern, actionSTC)
end

local function std_not_ip_rule_set(
    dev, ruleIndex, action_str,
    pclIdMsk, pclIdPattern,
    macSaMsk, macSaPattern,
    macDaMsk, macDaPattern)
    local actionSTC, ruleMsk, rulePattern;
    local patternAndMaskType = "CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC";
    local ruleFormat = 0; --"CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E";

    if pclIdPattern and (pclIdMsk == nil) then pclIdMsk = 0x3FF end
    if macSaPattern and (macSaMsk == nil) then macSaMsk = "FF:FF:FF:FF:FF:FF" end
    if macDaPattern and (macDaMsk == nil) then macDaMsk = "FF:FF:FF:FF:FF:FF" end
    actionSTC = get_action_by_str(action_str);
    if actionSTC == nil then
        return;
    end
    ruleMsk = get_rule_data(pclIdMsk, macSaMsk, macDaMsk);
    rulePattern = get_rule_data(pclIdPattern, macSaPattern, macDaPattern);
    rule_set_api_call(
        dev, ruleFormat, patternAndMaskType, ruleIndex, ruleMsk, rulePattern, actionSTC)
end

local data_store = nil;
local function store_callback_data(data_type, data)
    if not data_store then data_store = {} end;
    if data_type == "pclRule" then
        if not data_store.pclRule then data_store.pclRule = {}; end;
    end
    data_store.pclRule[data.ruleIndex] = data;
end

local ext_rule_data0 = {
    ruleIndex   = 6,
    ruleFormat  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E",
    ruleAction  = {pktCmd = "CPSS_PACKET_CMD_FORWARD_E"},
    ruleMask    = {common = {pclId = 0x3FF}, macSa="FF:FF:FF:FF:FF:FF"},
    rulePattern = {common = {pclId = 1}, macSa="A1:B1:C1:D1:E1:F1"}
}

local ext_rule_data1 = {
    ruleIndex   = 12,
    ruleFormat  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E",
    ruleAction  = {pktCmd = "CPSS_PACKET_CMD_DROP_HARD_E"},
    ruleMask    = {common = {pclId = 0x3FF}, macDa="FF:FF:FF:FF:FF:FF"},
    rulePattern = {common = {pclId = 1}, macDa="A2:B2:C2:D2:E2:F2"}
}

local ext_rule_data2 = {
    ruleIndex   = 18,
    ruleFormat  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E",
    ruleAction  = {pktCmd = "CPSS_PACKET_CMD_FORWARD_E"},
    ruleMask    = {common = {pclId = 0x3FF}, macDa="FF:FF:FF:FF:FF:FF"},
    rulePattern = {common = {pclId = 2}, macDa="A3:B3:C3:D3:E3:F3"}
}

local ext_rules_012 = {
    [ext_rule_data0.ruleIndex] = ext_rule_data0,
    [ext_rule_data1.ruleIndex] = ext_rule_data1,
    [ext_rule_data2.ruleIndex] = ext_rule_data2
}

local ext_rules_01 = {
    [ext_rule_data0.ruleIndex] = ext_rule_data0,
    [ext_rule_data1.ruleIndex] = ext_rule_data1
}

local std_rule_data0 = {
    ruleIndex   = 60,
    ruleFormat  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E",
    ruleAction  = {pktCmd = "CPSS_PACKET_CMD_FORWARD_E"},
    ruleMask    = {common = {pclId = 0x3FF}, macSa="FF:FF:FF:FF:FF:FF"},
    rulePattern = {common = {pclId = 1}, macSa="11:12:13:14:15:16"}
}

local std_rule_data1 = {
    ruleIndex   = 63,
    ruleFormat  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E",
    ruleAction  = {pktCmd = "CPSS_PACKET_CMD_DROP_HARD_E"},
    ruleMask    = {common = {pclId = 0x3FF}, macDa="FF:FF:FF:FF:FF:FF"},
    rulePattern = {common = {pclId = 1}, macDa="21:22:23:24:25:26"}
}

local std_rule_data2 = {
    ruleIndex   = 66,
    ruleFormat  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E",
    ruleAction  = {pktCmd = "CPSS_PACKET_CMD_FORWARD_E"},
    ruleMask    = {common = {pclId = 0x3FF}, macDa="FF:FF:FF:FF:FF:FF"},
    rulePattern = {common = {pclId = 2}, macDa="31:32:33:34:35:36"}
}

local std_rules_012 = {
    [std_rule_data0.ruleIndex] = std_rule_data0,
    [std_rule_data1.ruleIndex] = std_rule_data1,
    [std_rule_data2.ruleIndex] = std_rule_data2
}

local std_rules_01 = {
    [std_rule_data0.ruleIndex] = std_rule_data0,
    [std_rule_data1.ruleIndex] = std_rule_data1
}

local function check_output(actual, expected)
    local res = table_is_equal_to_subtree(
        actual.pclRule, expected, 100, true, true);
    if not res then
        setFailState()
        print("test failed\n")
    end
end

local function main_test(dev)
    ext_not_ipv6_rule_set(
        dev, 6, "permit",
        nil, 1,
        nil, "A1:B1:C1:D1:E1:F1",
        nil, nil)
    ext_not_ipv6_rule_set(
        dev, 12, "deny",
        nil, 1,
        nil, nil,
        nil, "A2:B2:C2:D2:E2:F2")
    ext_not_ipv6_rule_set(
        dev, 18, "permit",
        nil, 2,
        nil, nil,
        nil, "A3:B3:C3:D3:E3:F3")
    std_not_ip_rule_set(
        dev, 60, "permit",
        nil, 1,
        nil, "11:12:13:14:15:16",
        nil, nil)
    std_not_ip_rule_set(
        dev, 63, "deny",
        nil, 1,
        nil, nil,
        nil, "21:22:23:24:25:26")
    std_not_ip_rule_set(
        dev, 66, "permit",
        nil, 2,
        nil, nil,
        nil, "31:32:33:34:35:36")

    local isOk, msg = pcall(
        executeStringCliCommands,
        "end")
    if not isOk then setFailState() end

    autocheckDataCallback = store_callback_data;
    local isOk, msg = pcall(
        executeStringCliCommands,
        "show access-list device ${dev} pcl-id all format ingress_std_not_ip")
    if not isOk then setFailState() end
    check_output(data_store, std_rules_012);
    autocheckDataCallback = nil;
    data_store = nil;

    autocheckDataCallback = store_callback_data;
    local isOk, msg = pcall(
        executeStringCliCommands,
        "show pcl device ${dev} rule-id all format ingress_std_not_ip")
    if not isOk then setFailState() end
    check_output(data_store, std_rules_012);
    autocheckDataCallback = nil;
    data_store = nil;

    autocheckDataCallback = store_callback_data;
    local isOk, msg = pcall(
        executeStringCliCommands,
        "show pcl device ${dev} rule-id 60-64 format ingress_std_not_ip")
    if not isOk then setFailState() end
    check_output(data_store, std_rules_01);
    autocheckDataCallback = nil;
    data_store = nil;

    autocheckDataCallback = store_callback_data;
    local isOk, msg = pcall(
        executeStringCliCommands,
        "show access-list device ${dev} pcl-id all format ingress_ext_not_ipv6")
    if not isOk then setFailState() end
    check_output(data_store, ext_rules_012);
    autocheckDataCallback = nil;
    data_store = nil;

    autocheckDataCallback = store_callback_data;
    local isOk, msg = pcall(
        executeStringCliCommands,
        "show pcl device ${dev} rule-id all format ingress_ext_not_ipv6")
    if not isOk then setFailState() end
    check_output(data_store, ext_rules_012);
    autocheckDataCallback = nil;
    data_store = nil;

    autocheckDataCallback = store_callback_data;
    local isOk, msg = pcall(
        executeStringCliCommands,
        " show pcl device ${dev} rule-id 1-13 format ingress_ext_not_ipv6")
    if not isOk then setFailState() end
    check_output(data_store, ext_rules_01);
    autocheckDataCallback = nil;
    data_store = nil;

    local isOk, msg = pcall(
        executeStringCliCommands,
        "end")
    if not isOk then setFailState() end

    rule_delete_ext_rule(dev, 6);
    rule_delete_ext_rule(dev, 12);
    rule_delete_ext_rule(dev, 18);
    rule_delete_std_rule(dev, 60);
    rule_delete_std_rule(dev, 63);
    rule_delete_std_rule(dev, 63);
end

-- devEnv.dev is global variable
main_test(devEnv.dev);
