--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_mc_next_hop.lua
--*
--* DESCRIPTION:
--*       IP Multicast Next Hop support
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
--constants

-- DB contents:
-- DB = {
--    [next_hop_index] = {
--        [vid] = {
--            vidx = ,
--            ports = {port1 = {}, ...}}
--            eports = {eport1 = {} ...},
--            trunks = {trunk1 = {}, ...}
--        }
--    }
-- }
local ipMcNextHopDb = {};

local function name_of_vidx(next_hop, vid)
    return (
        "IP_MC_Next_Hop_VIDX_"
        .. "_next_hop_" .. tostring(next_hop)
        .. "_vid_" .. tostring(vid));
end

local function name_of_l3mll_chain(next_hop)
    return (
        "IP_MC_Next_Hop_L3_MLL_Chain_"
        .. "_next_hop_" .. tostring(next_hop));
end

local function clear_vidx(command_data, devNum, vidx)
    local apiName = "cpssDxChBrgMcGroupDelete"
    local is_failed, result, OUT_values =
        genericCpssApiWithErrorHandler(
        command_data, apiName ,{
            { "IN",     "GT_U8",      "devNum",  devNum },
            { "IN",     "GT_U16",     "vidx",    vidx }
        });
end

local function add_port_to_vidx(command_data, devNum, vidx, port)
    local apiName = "cpssDxChBrgMcMemberAdd"
    local is_failed, result, OUT_values =
        genericCpssApiWithErrorHandler(
        command_data, apiName ,{
            { "IN",     "GT_U8",      "devNum",                   devNum },
            { "IN",     "GT_U16",     "vidx",                     vidx },
            { "IN",     "GT_PHYSICAL_PORT_NUM",     "portNum",    port }
        });
end

--ip mc-next-hop <next_hop_index>
--    vid <vlan-id>
--    {ethhernet|eport|port-channel} <port/eport range or trunk id>
--
--   First call creates next-hop entry and first IP_MLL node
--   each next call adds node to IP_MLL chain - no override
--   No support of tunnel start in IP_MLL entry, tunneling only
--   by using eports in the chain.
--   Local physical port range implemented via VIDX - one VIDX per used VLAN.
--
--no ip mc-next-hop <next_hop_index>
--   full clean-up IP_MLL chain and next-hop entry
--


---
--  mc_next_hop_func
--        @description  configuring of multicast route next hop entry
--
--        @param:
--        param["next-hop-id"]:  (IP multicast entry)
--        param["vid"]:          vlan id;
--        param["ethernet"]:     dev/port pair or range
--        param["eport"]:        dev/eport pair or range
--        param["port-channel"]: dummy_dev/trunk-id pair or range;
--
--        @return       true on success, otherwise false and error message

local function ip_mc_next_hop_func(param)
    --print("ip mc-next-hop param: " .. to_string(param));
    local next_hop = param["next-hop-id"];
    local chain_name = name_of_l3mll_chain(next_hop);
    local command_data;
    local all_devices, devNum, hwDevNum;
    local is_failed, result, OUT_values, apiName;
    local vidx_name, vidx, next_hop_data;

    all_devices = wrLogWrapper("wrlDevList");
    devNum = all_devices[1];
    command_data = Command_Data();
    if param.unset then
        -- unset - destroy next-hop
        next_hop_data = ipMcNextHopDb[next_hop];
        if not next_hop_data then
            return command_data:addErrorAndPrint(
                "Next Hop not found in DB.");
        end
        util_l3mll_remove_chain(
                command_data, devNum, chain_name);
        for vid,vid_data in pairs(next_hop_data) do
            vidx_name = name_of_vidx(next_hop, vid);
            vidx = sharedResourceNamedVidxMemoAlloc(vidx_name, 1--[[size--]]);
            if vidx then
                clear_vidx(command_data, devNum, vidx);
            end
            sharedResourceNamedVidxMemoFree(vidx_name);
        end
        ipMcNextHopDb[next_hop] = nil;
        return;
    end

    -- set - create next-hop or add replicate-interfaces to it
    apiName = "cpssDxChCfgHwDevNumGet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8", "devNum", devNum },
        { "OUT", "GT_HW_DEV_NUM", "hwDevNumPtr" }
    });
    if (is_failed == true) then
        command_data:analyzeCommandExecution();
        command_data:printCommandExecutionResults();
        return;
    end
    hwDevNum = OUT_values.hwDevNumPtr;
    -- add next_hop to DB if needed
    next_hop_data = ipMcNextHopDb[next_hop];
    if not next_hop_data then
        ipMcNextHopDb[next_hop] = {};
        next_hop_data = ipMcNextHopDb[next_hop];
    end
    local vid = param["vid"];
    local vid_data = next_hop_data[vid];
    if not vid_data then
        next_hop_data[vid] = {ports = {}, eports = {}, trunks = {}, vidx = nil};
        vid_data = next_hop_data[vid];
    end
    -- retrieve added interfaces from parameters
    local new_ports = {};
    local new_eports = {};
    local new_trunks = {};
    local vidx;
    local params_count = 0;
    if param["ethernet"] then
        for dev, list in pairs(param["ethernet"]) do
            for dummy, port in pairs(list) do
                new_ports[port] = {};
            end
        end
        params_count = params_count + 1;
    end
    if param["eport"] then
        for dev, list in pairs(param["eport"]) do
            for dummy, eport  in pairs(list) do
                new_eports[eport] = {};
            end
        end
        params_count = params_count + 1;
    end
    if param["port-channel"] then
        for dev, list in pairs(param["port-channel"]) do
            for dummy, trunk  in pairs(list) do
                new_trunks[trunk] = {};
            end
        end
        params_count = params_count + 1;
    end
    if params_count > 1 then
        return command_data:addErrorAndPrint(
            "Use one of ethernet|eport|port-channel.");
    end
    -- exclude already added interfaces
    for port, dummy in pairs(vid_data.ports) do
        new_ports[port] = nil;
    end
    for eport, dummy in pairs(vid_data.eports) do
        new_eports[eport] = nil;
    end
    for trunk, dummy in pairs(vid_data.trunks) do
        new_trunks[trunk] = nil;
    end
    -- check there are really new interfaces
    if (not next(new_ports))
        and (not next(new_eports))
        and (not next(new_trunks))
    then
        return command_data:addErrorAndPrint(
            "No new interfaces should be added to Next Hop.");
    end
    --print("new_ports: " .. to_string(new_ports));
    --print("new_eports: " .. to_string(new_eports));
    --print("new_trunks: " .. to_string(new_trunks));
    local mc_next_hop_entry = {
        cmd = "CPSS_PACKET_CMD_ROUTE_E",
        RPFFailCommand = "CPSS_PACKET_CMD_BRIDGE_E"
    };
    -- local ports adding
    if next(new_ports) then
        vidx_name = name_of_vidx(next_hop, vid);
        vidx = sharedResourceNamedVidxMemoAlloc(vidx_name, 1--[[size--]]);
        if not next(vid_data.ports) then
            --first time vidx using
            clear_vidx(command_data, devNum, vidx);
            local mll_entry = {
                mllRPFFailCommand = "CPSS_PACKET_CMD_BRIDGE_E",
                nextHopInterface = {type = "CPSS_INTERFACE_VIDX_E", vidx = vidx},
                nextHopVlanId = vid,
                ttlHopLimitThreshold = 64,
            }
            util_l3mll_append_node(
                command_data, devNum, chain_name,
                ("Node_VIDX_"..tostring(vidx).."_for_local_ports"), mll_entry,
                next_hop, mc_next_hop_entry);
            vid_data.vidx = vidx;
        end
        for port, dummy in pairs(new_ports) do
            add_port_to_vidx(command_data, devNum, vidx, port);
        end
    end
    -- eports adding
    for eport, dummy in pairs(new_eports) do
        local mll_entry = {
            mllRPFFailCommand = "CPSS_PACKET_CMD_BRIDGE_E",
            nextHopInterface = {
                type = "CPSS_INTERFACE_PORT_E",
                devPort = {
                    -- the CPSS inteface structure uses name "hwDevNum"
                    -- but LUA CLI uses name "devNum" (as in cpss 4.0)
                    hwDevNum = hwDevNum,
                    devNum = hwDevNum,
                    portNum = eport
                }
            },
            nextHopVlanId = vid,
            ttlHopLimitThreshold = 64,
        }
        util_l3mll_append_node(
            command_data, devNum, chain_name,
            ("Node_for_eport_" .. tostring(eport)), mll_entry,
            next_hop, mc_next_hop_entry);
    end
    -- trunks adding
    for trunk, dummy in pairs(new_trunks) do
        local mll_entry = {
            mllRPFFailCommand = "CPSS_PACKET_CMD_BRIDGE_E",
            nextHopInterface = {
                type = "CPSS_INTERFACE_TRUNK_E",
                trunkId = trunk
            },
            nextHopVlanId = vid,
            ttlHopLimitThreshold = 64,
        }
        util_l3mll_append_node(
            command_data, devNum, chain_name,
            ("Node_for_trunk_" .. tostring(trunk)), mll_entry,
            next_hop, mc_next_hop_entry);
    end
    -- add new interface to DB
    for port, dummy in pairs(new_ports) do
        vid_data.ports[port] = {};
    end
    for eport, dummy in pairs(new_eports) do
        vid_data.eports[eport] = {};
    end
    for trunk, dummy in pairs(new_trunks) do
        vid_data.trunks[trunk] = {};
    end
end

--------------------------------------------------------
-- command registration:
--------------------------------------------------------
CLI_addCommand("config", "ip mc-next-hop", {
    func   = ip_mc_next_hop_func,
    help   = "Create or add interface to IP Multicast next-hop",
    params = {
        { type = "values",  "%next-hop-id"},
        { type = "named",
            { format = "vid %unchecked-vlan",   help = "Specific IEEE 802.1Q VLAN ID" },
            { format = "ethernet %port-range",
              help = "Ethernet interface range (with dummy device id)"},
            { format = "eport %port-range",
              help = "Eport interface range (with dummy device id)"},
            { format = "port-channel %port-range",
              help = "Port-Channel ids range (with dummy device id)"},
            requirements = {
              vid = {"next-hop-id"},
              ethernet = {"vid"},
              eport = {"vid"},
              ["port-channel"] = {"vid"},
            },
            mandatory = {"vid"},
        },
    }
})

CLI_addCommand("config", "no ip mc-next-hop", {
    func   = function(param)
        param.unset = true;
        ip_mc_next_hop_func(param);
    end,
    help   = "Remove IP Multicast next-hop",
    params = {
        { type = "values",  "%next-hop-id", mandatory = {"next-hop-id"}},
    }
})

CLI_addCommand("debug", "show ip-mc-next-hop-db", {
    func = function(param)
        print("IP Multicast Next Hop DB");
        print(to_string(ipMcNextHopDb));
    end,
    help="Dump IP Multicast Next Hop DB"
})

local mcNextHopCol = {
    {{1}, "next-hop", "d"},
    {{2}, "VID  ", "d"},
    {{3}, "ports                ", "s"},
    {{4}, "eports               ", "s"},
    {{5}, "trunks", "s"}
};

local function show_ip_mc_next_hop()
    print("--- IP Mulicast Next Hop Entries: ---");
    print("");
    print(string_for_table_break(nil, mcNextHopCol));
    print(string_for_table_title(nil, mcNextHopCol));
    print(string_for_table_break(nil, mcNextHopCol));
    for next_hop, data1 in pairs(ipMcNextHopDb) do
        for vid, data2 in pairs(data1) do
            local ports_str = "";
            if data2.ports then
                for port,dummy in pairs(data2.ports) do
                    if ports_str ~= "" then
                        ports_str = ports_str .. ",";
                    end
                    ports_str = ports_str .. tostring(port);
                end
            end
            local eports_str = "";
            if data2.eports then
                for eport,dummy in pairs(data2.eports) do
                    if eports_str ~= "" then
                        eports_str = eports_str .. ",";
                    end
                    eports_str = eports_str .. tostring(eport);
                end
            end
            local trunks_str  = "";
            if data2.trunks then
                for trunk,dummy in pairs(data2.trunks) do
                    if trunks_str ~= "" then
                        trunks_str = trunks_str .. ",";
                    end
                    trunks_str = trunks_str .. tostring(trunk);
                end
            end
            local line = {next_hop, vid, ports_str, eports_str, trunks_str};
            print(string_for_table_line(nil, mcNextHopCol, line));
        end
    end
    print(string_for_table_break(nil, mcNextHopCol));
end

CLI_addCommand("exec", "show ip mc-next-hop", {
    func   = show_ip_mc_next_hop,
    help   = "Show IP Multicast next-hop",
})


