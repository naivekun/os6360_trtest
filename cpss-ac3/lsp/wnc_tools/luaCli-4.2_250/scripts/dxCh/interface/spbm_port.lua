--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* spbm_port.lua
--*
--* DESCRIPTION:
--*     'port and e-port' commands for the 'spbm' system
--*     according to doc:
--*     http://docil.marvell.com/webtop/drl/objectId/0900dd88801a3892
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- backup before overriding
local debug_print = print;

-- configure TS-entry and return it's index
-- on error returns nil
local function spbm_hw_ts_entry_for_eport(
    command_data, devNum, b_vid, mac, eport, is_uc_eport, unset)
    local ts_entry_name;
    local ts_entry_index;
    local ts_entry_cfg;
    local is_failed, result, OUT_values, apiName;

    if is_uc_eport == true then
        ts_entry_name = string.format("SPBM uc_eport=%d", eport);
    else
        ts_entry_name = string.format("SPBM mc_eport=%d", eport);
    end

    ts_entry_index = sharedResourceNamedTsMemoAlloc(
        ts_entry_name, 1 --[[size--]]);
    if ts_entry_index == nil then
        return command_data:addErrorAndPrint(
            "tunnel start entry allocation error");
    end

    ts_entry_cfg = {};
    ts_entry_cfg.mimCfg = {};

    if unset ~= true then
        ts_entry_cfg.mimCfg.tagEnable    = true;
        ts_entry_cfg.mimCfg.vlanId       = b_vid;
        ts_entry_cfg.mimCfg.macDa        = mac;
        -- The reserved bits to be set in the iTag
        -- Bits1-2: Res1, Bit3: Res2, Bit4: UCA (Use Customer Address)
        ts_entry_cfg.mimCfg.iTagReserved = 0;
        -- CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E
        ts_entry_cfg.mimCfg.iSidAssignMode = 1;
        if is_uc_eport == true then
            -- CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E
            ts_entry_cfg.mimCfg.bDaAssignMode = 0;
        else
            -- CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E
            ts_entry_cfg.mimCfg.bDaAssignMode = 1;
        end
    end

    apiName = "cpssDxChTunnelStartEntrySet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "routerArpTunnelStartLineIndex", ts_entry_index },
            { "IN", "CPSS_TUNNEL_TYPE_ENT", "tunnelType", "CPSS_TUNNEL_MAC_IN_MAC_E" },
            { "IN", "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_mimCfg", "configPtr", ts_entry_cfg }
    });

    if unset == true then
        sharedResourceNamedTsMemoFree(ts_entry_name);
    end
    if is_failed == true then
        return nil;
    end

    return ts_entry_index;
end

-- configure eport with TS-entry and return true
-- on error returns nil
local function spbm_create_eport_hw(
    command_data, devNum, portNum, ts_index, eport, unset)
    local is_failed, result, OUT_values, apiName;
    local hwDevNum;
    local physicalInfoPtr;
    local egressInfoPtr;
    local sum_result;

    apiName = "cpssDxChCfgHwDevNumGet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8", "devNum", devNum },
        { "OUT", "GT_HW_DEV_NUM", "hwDevNumPtr" }
    });
    if (is_failed == true) then
        sum_result = nil;
    end

    hwDevNum = OUT_values.hwDevNumPtr;

    sum_result = true;

    physicalInfoPtr = {};
    physicalInfoPtr.type = "CPSS_INTERFACE_PORT_E"
    physicalInfoPtr.devPort = {}
    physicalInfoPtr.devPort.devNum = hwDevNum;
    if unset ~= true then
        physicalInfoPtr.devPort.portNum = portNum;
    else
        physicalInfoPtr.devPort.portNum = 62; -- null port
    end

    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", eport },
        { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfoPtr }
    });
    if (is_failed == true) then
        sum_result = nil;
    end

    apiName = "cpssDxChBrgEgrPortMeshIdSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", eport },
        { "IN", "GT_U32", "meshId", 0 }
    });
    if (is_failed == true) then
        sum_result = nil;
    end

    egressInfoPtr = {};
    if unset ~= true then
        egressInfoPtr.tunnelStart = true;
        egressInfoPtr.tunnelStartPtr = ts_index;
        egressInfoPtr.tsPassengerPacketType = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E";
    end

    apiName = "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", eport },
        { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", egressInfoPtr }
    });
    if (is_failed == true) then
        sum_result = nil;
    end

    return sum_result;
end

local function spbm_pe_node_on_port_mc_replication_hw(
    command_data, devNum, portNum, unset)
    local is_failed, result, OUT_values, apiName;
    local mode;

    if unset ~= true then
        mode = "CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENABLE_ALL_E";
    else
        mode = "CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_DISABLE_E";
    end

    apiName = "cpssDxChTtiMcTunnelDuplicationModeSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",                                        "devNum",   devNum },
        { "IN", "GT_PORT_NUM",                                  "portNum",  portNum},
        { "IN", "CPSS_TUNNEL_MULTICAST_TYPE_ENT",                "protocol",
            "CPSS_TUNNEL_MULTICAST_PBB_E"},
        { "IN", "CPSS_DXCH_TTI_MULTICAST_DUPLICATION_MODE_ENT", "mode",     mode }
    });
    if (is_failed == true) then
        return nil;
    end
    return true;
end

local function spbm_pe_node_on_port_tti_enable_hw(
    command_data, devNum, portNum, unset)
    local is_failed, result, OUT_values, apiName;
    local enable;

    if unset ~= true then
       enable  = true;
    else
        enable = false;
    end

    apiName = "cpssDxChTtiPortLookupEnableSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",                        "devNum",   devNum },
        { "IN", "GT_PORT_NUM",                  "portNum",  portNum},
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT",   "keyType",
            spbm_pbb_tunneled_packet_tti_type},
        { "IN", "GT_BOOL",                      "enable",   enable}
    });
    if (is_failed == true) then
        return nil;
    end
    return true;
end

local function spbm_pe_node_on_port_pcl_enable_hw(
    command_data, devNum, uc_eport, unset)
    local is_failed, result, OUT_values, apiName;
    local enable = (unset ~= true);

    apiName = "cpssDxChPclPortIngressPolicyEnable";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",                        "devNum",   devNum },
        { "IN", "GT_PORT_NUM",                  "portNum",  uc_eport},
        { "IN", "GT_BOOL",                      "enable",   enable}
    });
    if (is_failed == true) then
        return nil;
    end
    return true;
end

local function spbm_pe_node_on_port_hw(
    command_data, devNum, portNum, b_vid, mac, uc_eport, do_phis_port_cfg, unset)
    local ts_index, res;
    ts_index = spbm_hw_ts_entry_for_eport(
        command_data, devNum, b_vid, mac, uc_eport,
        true --[[is_uc_eport--]], unset)
    if (ts_index == nil) and (unset ~= true) then
        return nil;
    end
    res = spbm_create_eport_hw(
        command_data, devNum, portNum, ts_index, uc_eport, unset)
    if (res == nil) and (unset ~= true) then
        return nil;
    end

    if  do_phis_port_cfg == true then
        res = spbm_pe_node_on_port_mc_replication_hw(
            command_data, devNum, portNum, unset)
        if (res == nil) and (unset ~= true) then
            return nil;
        end

        res = spbm_pe_node_on_port_tti_enable_hw(
            command_data, devNum, portNum, unset)
        if (res == nil) and (unset ~= true) then
            return nil;
        end
    end

    res = spbm_tti_rule_set(
        command_data, devNum,
        mac, b_vid, portNum, true --[[rule_for_uc--]],
        uc_eport, unset)
    if (res == nil) and (unset ~= true) then
        return nil;
    end

    res = spbm_tti_rule_set(
        command_data, devNum,
        mac, b_vid, portNum, false --[[rule_for_uc--]],
        uc_eport, unset)
    if (res == nil) and (unset ~= true) then
        return nil;
    end

    res = spbm_pe_node_on_port_pcl_enable_hw(
        command_data, devNum, uc_eport, unset)
    if (res == nil) and (unset ~= true) then
        return nil;
    end
end

local function spbm_pe_node_on_port(params)
    -- context data
    local command_data = Command_Data();
    command_data:initInterfaceDevPortRange();
    if command_data:isOnePortInPortRange() ~= true then
        return command_data:addErrorAndPrint("port range contains more then one port");
    end
    local dev_port_range = command_data:getDevicePortRange();
    local devNum   = next(dev_port_range, nil);
    local portNum  = dev_port_range[devNum][1];
    -- command parameters
    local unset    = params["unset"];
    local b_vid;
    local mac;
    local mac_str;

    -- DB nodes
    local uc_eport = tonumber(params["uc_eport"]);
    local eport_by_bvid_by_mac = spbm_internal_db.pe_nodes.eport_by_bvid_by_mac;
    local info_by_eport = spbm_internal_db.pe_nodes.info_by_eport;
    local b_vid_node;
    local info_by_given_eport;
    local phis_port_use_count, do_phis_port_cfg;
    local eport_key;

    -- check DB
    if unset ~= true then
        b_vid    = tonumber(params["b_vid"]);
        mac      = params["mac"];
        mac_str  = string.upper(mac.string);
        b_vid_node = eport_by_bvid_by_mac[b_vid];

        -- check that uc_eport is new
        if info_by_eport[uc_eport] ~= nil then
            return command_data:addErrorAndPrint("uc-eport already configured");
        end
        if spbm_internal_db.mc_groups.info_by_eport[uc_eport] ~= nil then
            return command_data:addErrorAndPrint("uc-eport used for mc-groups");
        end
        -- check that pair of b_vid and mac is new
        if b_vid_node ~= nil then
            if b_vid_node[mac_str] ~= nil then
                return command_data:addErrorAndPrint("b-vid and mac already have uc-eport");
            end
        end
        -- check that mac address is unicast mac_bit[40]==0
        local mac_43_40 = string.upper(string.sub(mac.string, 2, 2));
        if string.find("13579BDF", mac_43_40) ~= nil then
            return command_data:addErrorAndPrint("Mac Address is multicast");
        end
    else
        info_by_given_eport = info_by_eport[uc_eport];
        if info_by_given_eport == nil then
            return command_data:addErrorAndPrint("uc-eport info not found in DB");
        end
        b_vid    = info_by_given_eport.bvid;
        mac      = info_by_given_eport.mac;
        mac_str  = string.upper(mac.string);
        b_vid_node = eport_by_bvid_by_mac[b_vid];
    end

    phis_port_use_count = 0;
    eport_key = next(info_by_eport, nil);
    while eport_key ~= nil do
        if info_by_eport[eport_key].port == portNum then
            phis_port_use_count = 1 + phis_port_use_count;
        end
        eport_key = next(info_by_eport, eport_key);
    end

    if unset ~= true then
        -- first and will be added to DB
        do_phis_port_cfg = (phis_port_use_count == 0);
    else
        -- last and will be removed from DB
        do_phis_port_cfg = (phis_port_use_count == 1);
    end

    -- HW configuration, the problems will be stored to command_data
    spbm_global_configuration(command_data, devNum , unset);
    spbm_pe_node_on_port_hw(
        command_data, devNum, portNum, b_vid, mac, uc_eport, do_phis_port_cfg, unset);
    if true ~= command_data:getCommandExecutionResults() then
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
    end
    -- update DB
    if unset ~= true then
        -- adding data
        if b_vid_node == nil then
            eport_by_bvid_by_mac[b_vid] = {};
            b_vid_node = eport_by_bvid_by_mac[b_vid];
        end
        b_vid_node[mac_str] = uc_eport;
        info_by_eport[uc_eport] =
            {port = portNum, bvid = b_vid, mac = mac};
    else
        -- removing data
        b_vid_node[mac_str]     = nil;
        info_by_eport[uc_eport] = nil;
        if next(b_vid_node, nil) == nil then
            eport_by_bvid_by_mac[b_vid] = nil;
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

CLI_addCommand("interface", "spbm pe-node",
               { func = function(params)
                       params.unset = false;
                       return spbm_pe_node_on_port(params);
                   end,
      help = "SPBM define connection to remote PE-device via context port",
      params =
      {
            { type="named",
                { format = "b-vid %vlanId", name = "b_vid",
                   help = "Backbone edge VLAN Id" },
                { format = "mac %mac-address", name = "mac",
                    help = "remote PE-device mac-address" },
                { format = "uc-eport %s", name = "uc_eport",
                    help = "e-port mapped to context port with tunneling" },
                requirements = {
                    ["mac"] = {"b_vid"},
                    ["uc_eport"] = {"mac"}},
                mandatory = {"uc_eport"}
            }
      }
    });

CLI_addCommand("interface", "no spbm pe-node",
               { func = function(params)
                       params.unset = true;
                       return spbm_pe_node_on_port(params);
                   end,
      help = "SPBM remove connection to remote PE-device via context port",
      params =
      {
            { type="named",
                { format = "uc-eport %s", name = "uc_eport",
                    help = "e-port mapped to context port with tunneling" },
                mandatory = {"uc_eport"}
            }
      }
    });

local function spbm_provider_flood_phis_port_hw(
    command_data, devNum, portNum, b_vid, mac_prefix, flood_eport, unset)
    local ts_index, res;

    ts_index = spbm_hw_ts_entry_for_eport(
        command_data, devNum, b_vid, mac_prefix, flood_eport,
        false --[[is_uc_eport--]], unset)
    if (ts_index == nil) and (unset ~= true) then
        return nil;
    end
    res = spbm_create_eport_hw(
        command_data, devNum, portNum, ts_index, flood_eport, unset)
end

local function spbm_provider_flood_phis_port(params)
    -- context data
    local command_data = Command_Data();
    command_data:initInterfaceDevPortRange();
    if command_data:isOnePortInPortRange() ~= true then
        return command_data:addErrorAndPrint("port range contains more then one port");
    end
    local dev_port_range = command_data:getDevicePortRange();
    local devNum   = next(dev_port_range, nil);
    local portNum  = dev_port_range[devNum][1];
    -- command parameters
    local unset    = params["unset"];
    local b_vid;
    local mac_prefix;
    local mac_str;

    -- DB nodes
    local flood_eport = tonumber(params["flood_eport"]);
    local eport_by_bvid_mac_port = spbm_internal_db.mc_groups.flood_eport_by_bvid_mac_port;
    local info_by_eport = spbm_internal_db.mc_groups.info_by_eport;
    local b_vid_node, b_vid_mac_node;
    local info_by_given_eport;

    -- check DB
    if unset ~= true then
        b_vid       = tonumber(params["b_vid"]);
        mac_prefix  = params["mc_mac_prefix"];
        mac_str     = string.upper(mac_prefix.string);
        b_vid_node  = eport_by_bvid_mac_port[b_vid];

        -- check that uc_eport is new
        if info_by_eport[flood_eport] ~= nil then
            return command_data:addErrorAndPrint("flood-eport already configured");
        end
        if spbm_internal_db.pe_nodes.info_by_eport[flood_eport] ~= nil then
            return command_data:addErrorAndPrint("flood-eport used for pe-nodes");
        end
        -- check that pair of b_vid and mac is new
        if b_vid_node ~= nil then
            if b_vid_node[mac_str] ~= nil then
                if b_vid_node[mac_str][portNum] ~= nil then
                    return command_data:addErrorAndPrint(
                        "<b-vid, mc-group, phis_port> already has flood-eport");
                end
            end
        end
        -- check that mac address is multicast mac_bit[40]==1
        local mac_43_40 = string.upper(string.sub(mac_prefix.string, 2, 2));
        if string.find("02468ACE", mac_43_40) ~= nil then
            return command_data:addErrorAndPrint("Mac Address is unicast");
        end
    else
        info_by_given_eport = info_by_eport[flood_eport];
        if info_by_given_eport == nil then
            return command_data:addErrorAndPrint("flood-eport info not found in DB");
        end
        b_vid    = info_by_given_eport.bvid;
        mac_prefix      = info_by_given_eport.mac_prefix;
        mac_str  = string.upper(mac_prefix.string);
        b_vid_node = eport_by_bvid_mac_port[b_vid];
        if b_vid_node == nil then
            return command_data:addErrorAndPrint(
                "<b-vid, mc-group, phis_port> flood-eport not found in DB");
        elseif b_vid_node[mac_str] == nil then
            return command_data:addErrorAndPrint(
                "<b-vid, mc-group, phis_port> flood-eport not found in DB");
        elseif b_vid_node[mac_str][portNum] == nil then
            return command_data:addErrorAndPrint(
                "<b-vid, mc-group, phis_port> flood-eport not found in DB");
        end
        -- check that eport being removed not used in any sid
        local sid_eport_tab = spbm_internal_db.services.info_by_sid;
        local sid_eport_key = next(sid_eport_tab, nil);
        while sid_eport_key ~= nil do
            if sid_eport_tab[sid_eport_key].mc_groups[flood_eport] ~= nil then
                return command_data:addErrorAndPrint("flood-eport used by some SID");
            end
            sid_eport_key = next(sid_eport_tab, sid_eport_key);
        end
    end

    -- HW configuration, the problems will be stored to command_data
    spbm_global_configuration(command_data, devNum , unset);
    spbm_provider_flood_phis_port_hw(
        command_data, devNum, portNum, b_vid, mac_prefix, flood_eport, unset);
    if true ~= command_data:getCommandExecutionResults() then
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
    end

    -- update DB
    if unset ~= true then
        -- adding data
        if b_vid_node == nil then
            eport_by_bvid_mac_port[b_vid] = {};
            b_vid_node = eport_by_bvid_mac_port[b_vid];
        end
        b_vid_mac_node = b_vid_node[mac_str];
        if b_vid_mac_node == nil then
            b_vid_node[mac_str] = {};
            b_vid_mac_node = b_vid_node[mac_str];
        end
        b_vid_mac_node[portNum] = flood_eport;
        info_by_eport[flood_eport] =
            {port = portNum, bvid = b_vid, mac_prefix = mac_prefix};
    else
        -- removing data
        info_by_eport[flood_eport] = nil;
        b_vid_mac_node = b_vid_node[mac_str];
        b_vid_mac_node[portNum] = nil;
        if next(b_vid_mac_node, nil) == nil then
            b_vid_node[mac_str] = nil;
        end
        if next(b_vid_node, nil) == nil then
            eport_by_bvid_mac_port[b_vid] = nil;
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

CLI_addCommand("interface", "spbm provider-flood-port",
               { func = function(params)
                       params.unset = false;
                       return spbm_provider_flood_phis_port(params);
                   end,
      help = "SPBM define flooding registered multicast via backbone context port",
      params =
      {
            { type="named",
                { format = "b-vid %vlanId", name = "b_vid",
                   help = "Backbone edge VLAN Id" },
                { format = "mc-mac-prefix %mac-address-prefix", name = "mc_mac_prefix",
                    help = "flooded registered multicast mac-address prefix" },
                { format = "flood-eport %s", name = "flood_eport",
                    help = "e-port mapped to context port with tunneling" },
                requirements = {
                    ["mc_mac_prefix"] = {"b_vid"},
                    ["flood_eport"] = {"mc_mac_prefix"}},
                mandatory = {"flood_eport"}
            }
      }
    });

CLI_addCommand("interface", "no spbm provider-flood-port",
               { func = function(params)
                       params.unset = true;
                       return spbm_provider_flood_phis_port(params);
                   end,
      help = "SPBM remove flooding registered multicast via backbone context port",
      params =
      {
            { type="named",
                { format = "flood-eport %s", name = "flood_eport",
                    help = "e-port mapped to context port with tunneling" },
                mandatory = {"flood_eport"}
            }
      }
    });

local function spbm_provider_flood_sid_eport_hw(
    command_data, devNum, flood_eport, i_sid, e_vidx, unset)
    local is_failed, result, OUT_values, apiName;
    local mll_node, ltt_index, chain_name, node_name, hwDevNum;

    ltt_index = spbm_sevice_id_vlan_evidx_to_l2mll_ltt_index(e_vidx);
    chain_name = spbm_sevice_id_vlan_l2mll_chain_name(i_sid);
    node_name  = "FLOOD_EPORT " .. string.format("0x%04X", flood_eport)

    if unset ~= true then
        apiName = "cpssDxChCfgHwDevNumGet";
        is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
            command_data, apiName, {
            { "IN", "GT_U8", "devNum", devNum },
            { "OUT", "GT_HW_DEV_NUM", "hwDevNumPtr" }
        });
        if (is_failed == true) then
            return command_data:addErrorAndPrint("not found GT_HW_DEV_NUM");
        end
        hwDevNum = OUT_values.hwDevNumPtr;

        mll_node = {
            egressInterface = {
                type = "CPSS_INTERFACE_PORT_E",
                devPort = {
                    -- the CPSS inteface structure uses name "hwDevNum"
                    -- but LUA CLI uses name "devNum" (maybe it ia a bug)
                    hwDevNum = hwDevNum,
                    devNum = hwDevNum,
                    portNum  = flood_eport
                }
            },
            meshId = spbm_pbb_brg_flt_mesh_id_value
        };

        util_l2mll_add_node(
            command_data, devNum, chain_name, node_name, mll_node,
            ltt_index, {} --[[ltt_param--]]);
    else
        util_l2mll_remove_node(
            command_data, devNum, chain_name, node_name,
            ltt_index, {} --[[ltt_param--]]);
    end
end

local function spbm_provider_flood_sid_eport(params)
    local iterator, devNum, eport;
    local command_data = Command_Data();
    local unset   = params["unset"];
    local i_sid   = tonumber(params["i_sid"]);
    local info_by_sid = spbm_internal_db.services.info_by_sid;
    local info_by_given_sid = info_by_sid[i_sid];
    local mc_groups_db;
    local skip_eport;
    local info_by_mc_eport = spbm_internal_db.mc_groups.info_by_eport;
    local e_vidx;

    if info_by_given_sid == nil then
        return command_data:addErrorAndPrint("i-sid not found in DB");
    end

    mc_groups_db = info_by_given_sid.mc_groups;
    if mc_groups_db == nil then
        -- must be created by spbm service-vlan command
        return command_data:addErrorAndPrint("i-sid mc-groups not found in DB");
    end

    e_vidx = info_by_given_sid.evidx;
    if e_vidx == nil then
        -- must be created by spbm service-vlan command
        return command_data:addErrorAndPrint("i-sid e_vidx not found in DB");
    end

    command_data:initInterfaceDevPortRange();
    for iterator, devNum, eport in command_data:getInterfacePortIterator() do
        if info_by_mc_eport[eport] == nil then
            return command_data:addErrorAndPrint("eport not found in DB");
        end
        skip_eport = false;
        if unset ~= true then
            if mc_groups_db[eport] ~= nil then
                skip_eport = true;
                command_data:addWarning("eport found in DB and skipped 0x%X", eport);
            end
        else
            if mc_groups_db[eport] == nil then
                skip_eport = true;
                command_data:addWarning("eport not found in DB and skipped 0x%X", eport);
            end
        end

        if skip_eport == false then
            spbm_provider_flood_sid_eport_hw(
                command_data, devNum, eport, i_sid, e_vidx, unset);
            if true ~= command_data:getCommandExecutionResults() then
                command_data:analyzeCommandExecution()
                command_data:printCommandExecutionResults()
                return command_data:getCommandExecutionResults()
            end
            if unset ~= true then
                mc_groups_db[eport] = {};
            else
                mc_groups_db[eport] = nil;
            end
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

CLI_addCommand({"interface", "interface_eport"}, "spbm provider-flood-eport",
               { func = function(params)
                       params.unset = false;
                       return spbm_provider_flood_sid_eport(params);
                   end,
      help = "SPBM add registered multicast flooding eport to i-sid",
      params =
      {
            { type="named",
                { format = "i-sid %s", name = "i_sid",
                    help = "service-id (up to 24-bit)" },
                mandatory = {"i_sid"}
            }
      }
    });

CLI_addCommand({"interface", "interface_eport"}, "no spbm provider-flood-eport",
               { func = function(params)
                       params.unset = true;
                       return spbm_provider_flood_sid_eport(params);
                   end,
      help = "SPBM remove registered multicast flooding eport from i-sid",
      params =
      {
            { type="named",
                { format = "i-sid %s", name = "i_sid",
                    help = "service-id (up to 24-bit)" },
                mandatory = {"i_sid"}
            }
      }
    });

CLI_addHelp("interface", "spbm", "port and e-port commands for SPBM system")


