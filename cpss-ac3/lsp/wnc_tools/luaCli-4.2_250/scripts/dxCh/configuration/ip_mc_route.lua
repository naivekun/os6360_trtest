--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_mc_route.lua
--*
--* DESCRIPTION:
--*       creating/destroing of IPV4 MC Routes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
--constants

local function ltt_entry_for_mc_next_hop(next_hop_index)
    return {
        routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
        numOfPaths = 1,
        routeEntryBaseIndex = next_hop_index,
        ucRPFCheckEnable = false,
        sipSaCheckMismatchEnable = false,
        ipv6MCGroupScopeLevel = 0  -- IPV6 only
    };
end

-- ************************************************************************
---
--  ip_mc_route_func
--        @description  creates IP Mc Routes
--
--        @param params - params["group-ipv4-prefix"]: prefix of group ipv4 address;
--                        params["group-ipv4-prefix"]["length"]: - prefix length
--                        params["nexthop"]: next-hop entry index
--                        params["source"]: prefix of source ipv4 address;
--                        params["source"]["length"]: - prefix length
--                        params["vrf-id"]: Virtual Router ID
--
--        @return       true on success, otherwise false and error message
--
local function ip_mc_route_func(params)
    --print(to_string(params));
    -- Common variables declaration
    local result, values;
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local group_ip_address, group_mask_length, next_hop_enrty_index, vrf_id;
    local src_ip_address, src_mask_length;
    local apiName, is_failed, result, OUT_values;

    -- Command specific variables initialization.
    group_ip_address        = params["group-ipv4-prefix"];
    group_mask_length       = group_ip_address["length"];
    next_hop_enrty_index    = params["nexthop"]
    src_ip_address          = params["source"]
        or {string="0.0.0.0/0", length=0, "0","0","0","0"} ;
    src_mask_length         = src_ip_address["length"];
    vrf_id                  = params["vrf-id"] or 0 -- set 0 if nil

    -- Ip prefix adding.
    apiName = "cpssDxChIpLpmIpv4McEntryAdd";
    is_failed, result, OUT_values =
        genericCpssApiWithErrorHandler(
        command_data, apiName ,{
            { "IN",     "GT_U32",      "lpmDBId",              0 },
            { "IN",     "GT_U32",      "vrId",                 vrf_id  },
            { "IN",     "GT_IPADDR",   "ipGroup",              group_ip_address},
            { "IN",     "GT_U32",      "ipGroupPrefixLen",     group_mask_length  },
            { "IN",     "GT_IPADDR",   "ipSrc",                src_ip_address},
            { "IN",     "GT_U32",      "ipSrcPrefixLen",       src_mask_length},
            { "IN",     "CPSS_DXCH_IP_LTT_ENTRY_STC",     "mcRouteLttEntryPtr",
                        ltt_entry_for_mc_next_hop(next_hop_enrty_index)},
            { "IN",     "GT_BOOL",     "override",    true },
            { "IN",     "GT_BOOL",     "defragmentationEnable",    true }
        });

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  no_ip_mc_route_func
--        @description  destroes IP Mc Routes
--
--                        params["group-ipv4-prefix"]["length"]: - prefix length
--                        params["nexthop"]: next-hop entry index
--                        params["source"]: prefix of source ipv4 address;
--                        params["source"]["length"]: - prefix length
--                        params["vrf-id"]: Virtual Router ID
--
--        @return       true on success, otherwise false and error message
--
local function no_ip_mc_route_func(params)
    --print(to_string(params));
    -- Common variables declaration
    local result, values;
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local group_ip_address, group_mask_length, vrf_id;
    local src_ip_address, src_mask_length;
    local apiName, is_failed, result, OUT_values;

    -- Command specific variables initialization.
    group_ip_address        = params["group-ipv4-prefix"];
    group_mask_length       = group_ip_address["length"];
    src_ip_address          = params["source"]
        or {string="0.0.0.0/0", length=0, "0","0","0","0"} ;
    src_mask_length         = src_ip_address["length"];
    vrf_id                  = params["vrf-id"] or 0 -- set 0 if nil

    -- Ip prefix adding.
    apiName = "cpssDxChIpLpmIpv4McEntryDel";
    is_failed, result, OUT_values =
        genericCpssApiWithErrorHandler(
        command_data, apiName ,{
            { "IN",     "GT_U32",      "lpmDBId",              0 },
            { "IN",     "GT_U32",      "vrId",                 vrf_id  },
            { "IN",     "GT_IPADDR",   "ipGroup",              group_ip_address},
            { "IN",     "GT_U32",      "ipGroupPrefixLen",     group_mask_length  },
            { "IN",     "GT_IPADDR",   "ipSrc",                src_ip_address},
            { "IN",     "GT_U32",      "ipSrcPrefixLen",       src_mask_length}
        });

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------------------------


--------------------------------------------------------------------------------
-- command registration: ip route
--------------------------------------------------------------------------------
-- ip mc-route <ip-addr /prefix-len>
--     nexthop <next_hop_index>
--     [source <ip-addr/prefix-len>]
--     [vrf-id = <vrf-id>]
CLI_addCommand("config", "ip mc-route", {
func   = ip_mc_route_func,
help   = "Creating of MC IPV4 Routes",
  params = {
      { type = "values",
          {format = "%ipv4_mc_prefix", name = "group-ipv4-prefix",
              help = "Ipv4 group address prefix"},
      },
      { type="named",
          {format = "nexthop %next-hop-id",
              help = "Apply to given next-hop id",
              requirements = {"group-ipv4-prefix"}},
          -- optional parameters
          {format="source %ipv4_uc_prefix",
              help = "Source Ipv4 pddress prefix",
              requirements = {"nexthop"}},
          {format="vrf-id %vrf_id",
              help = "a Virtual Router Id. The default is 0",
              requirements = {"nexthop"}},
          mandatory = { "nexthop" },
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no ip route
--------------------------------------------------------------------------------
-- no ip mc-route <ip-addr/prefix-len>
--     [source <ip-addr/prefix-len>]
--     [vrf-id = <vrf-id>]
CLI_addCommand("config", "no ip mc-route", {
func   = no_ip_mc_route_func,
help   = "Removing of MC IPV4 Routes",
    params = {
        { type = "values",
            {format = "%ipv4_mc_prefix", name = "group-ipv4-prefix",
                help = "Ipv4 group address prefix"},
        },
        { type="named",
            -- optional parameters
            {format="source %ipv4_uc_prefix",
                help = "Source Ipv4 pddress prefix",
                requirements = {"group-ipv4-prefix"}},
            {format="vrf-id %vrf_id",
                help = "a Virtual Router Id. The default is 0",
                requirements = {"group-ipv4-prefix"}},
            mandatory = { "group-ipv4-prefix" },
        }
    }
})

-- show command ---------------------------------------------------------------
-- "Search" should be called before "GetNext" to check that VRID initialized and
-- Prefix presents. "GetNext" must be called with Prefix presents in memory.

-- initializes entry_inout_data used as iterator
local function init_get_next_mc_route_inout_data()
    return {
        group_ip_address  = "224.0.0.0",
        group_mask_length = 4,
        src_ip_address    = "0.0.0.0",
        src_mask_length   = 0,
        };
end

-- returns (not is_failed), CPSS_DXCH_IP_LTT_ENTRY_STC of current entry
local function search_mc_route_entry(command_data, vrf_id, entry_inout_data)
    local apiName, is_failed, result, OUT_values, values;
    apiName = "cpssDxChIpLpmIpv4McEntrySearch";
    result, OUT_values = myGenWrapper(
        apiName,
        {
            { "IN",     "GT_U32",      "lpmDBId",              0 },
            { "IN",     "GT_U32",      "vrId",                 vrf_id  },
            { "IN",     "GT_IPADDR",   "ipGroup",              entry_inout_data.group_ip_address},
            { "IN",     "GT_U32",      "ipGroupPrefixLen",     entry_inout_data.group_mask_length  },
            { "IN",     "GT_IPADDR",   "ipSrc",                entry_inout_data.src_ip_address},
            { "IN",     "GT_U32",      "ipSrcPrefixLen",       entry_inout_data.src_mask_length},
            { "OUT",    "CPSS_DXCH_IP_LTT_ENTRY_STC",      "mcRouteLttEntryPtr"},
            { "OUT",    "GT_U32",      "tcamGroupRowIndexPtr"},
            { "OUT",    "GT_U32",      "tcamGroupColumnIndexPtr"},
            { "OUT",    "GT_U32",      "tcamSrcRowIndexPtr"},
            { "OUT",    "GT_U32",      "tcamSrcColumnIndexPtr"},
        });
    --print("OUT_values " .. to_string(OUT_values));
    is_failed = false;
    if result ~= 0 then
        is_failed = true;
        if (result ~= 0x0B) and (result ~= 0x12) then
            -- GT_NOT_FOUND             (0x0B) /* Item not found                        */
            -- GT_NOT_INITIALIZED       (0x12) /* The item is not initialized           */
            command_data:addErrorAndPrint(apiName .. " failed, rc = " .. tonumber(result));
        end
    end
    values = nil;
    if not is_failed then
        values = OUT_values.mcRouteLttEntryPtr;
    end
    return (not is_failed), values;
end


-- updates entry_inout_data used as iterator
-- returns (not is_failed), CPSS_DXCH_IP_LTT_ENTRY_STC of current entry
local function get_next_mc_route_entry(command_data, vrf_id, entry_inout_data)
    local apiName, is_failed, result, OUT_values, values;
    apiName = "cpssDxChIpLpmIpv4McEntryGetNext";
    result, OUT_values = myGenWrapper(
        apiName,
        {
            { "IN",     "GT_U32",      "lpmDBId",              0 },
            { "IN",     "GT_U32",      "vrId",                 vrf_id  },
            -- inout parameters
            { "INOUT",     "GT_IPADDR",   "ipGroupPtr",              entry_inout_data.group_ip_address},
            { "INOUT",     "GT_U32",      "ipGroupPrefixLenPtr",     entry_inout_data.group_mask_length  },
            { "INOUT",     "GT_IPADDR",   "ipSrcPtr",                entry_inout_data.src_ip_address},
            { "INOUT",     "GT_U32",      "ipSrcPrefixLenPtr",       entry_inout_data.src_mask_length},
            { "OUT",    "CPSS_DXCH_IP_LTT_ENTRY_STC",      "mcRouteLttEntryPtr"},
            { "OUT",    "GT_U32",      "tcamGroupRowIndexPtr"},
            { "OUT",    "GT_U32",      "tcamGroupColumnIndexPtr"},
            { "OUT",    "GT_U32",      "tcamSrcRowIndexPtr"},
            { "OUT",    "GT_U32",      "tcamSrcColumnIndexPtr"},
        });
    -- print("OUT_values " .. to_string(OUT_values));
    is_failed = false;
    if result ~= 0 then
        is_failed = true;
        if (result ~= 0x0B) and (result ~= 0x12) then
            -- GT_NOT_FOUND             (0x0B) /* Item not found                        */
            -- GT_NOT_INITIALIZED       (0x12) /* The item is not initialized           */
            command_data:addErrorAndPrint(apiName .. " failed, rc = " .. tonumber(result));
        end
    end
    values = nil;
    if not is_failed then
        -- update iterator
        entry_inout_data.group_ip_address  = OUT_values.ipGroupPtr;
        entry_inout_data.group_mask_length = OUT_values.ipGroupPrefixLenPtr;
        entry_inout_data.src_ip_address    = OUT_values.ipSrcPtr;
        entry_inout_data.src_mask_length   = OUT_values.ipSrcPrefixLenPtr;
        values = OUT_values.mcRouteLttEntryPtr;
    end
    return (not is_failed), values;
end

local mcPrefixCol = {
    {{1}, "Group Ip   ",  "s"},
    {{2}, "/bits",        "d"},
    {{3}, "Source Ip  ",  "s"},
    {{4}, "/bits",        "d"},
    {{5}, "Next Hop",     "d"}
};

local function show_ip_mc_route_func(params)
    local command_data = Command_Data()
    local vrf_id = params["vrf-id"];
    if not vrf_id then
        vrf_id = 0;
    end
    local entry_inout_data = init_get_next_mc_route_inout_data();
    local res_ok, ip_ltt_entry, line;
    print("MC LPM entries:");
    print(string_for_table_break(nil, mcPrefixCol));
    print(string_for_table_title(nil, mcPrefixCol));
    print(string_for_table_break(nil, mcPrefixCol));

    res_ok, ip_ltt_entry = search_mc_route_entry(
        command_data, vrf_id, entry_inout_data);
    while res_ok do
        line = {
            entry_inout_data.group_ip_address,
            entry_inout_data.group_mask_length,
            entry_inout_data.src_ip_address,
            entry_inout_data.src_mask_length,
            ip_ltt_entry.routeEntryBaseIndex,
            };
        print(string_for_table_line(nil, mcPrefixCol, line));
        res_ok, ip_ltt_entry = get_next_mc_route_entry(
            command_data, vrf_id, entry_inout_data);
        if not res_ok then
            break;
        end
    end
    print(string_for_table_break(nil, mcPrefixCol));
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: show ip route
--------------------------------------------------------------------------------
-- show ip mc-route
CLI_addCommand("exec", "show ip mc-route", {
    func   = show_ip_mc_route_func,
    help   = "Show all MC IPV4 Routes",
    params = {
        -- parameter vrf-id is optional
        { type="named",
            {format="vrf-id %vrf_id",
                help = "a Virtual Router Id. The default is 0"},
        }
    }
})


