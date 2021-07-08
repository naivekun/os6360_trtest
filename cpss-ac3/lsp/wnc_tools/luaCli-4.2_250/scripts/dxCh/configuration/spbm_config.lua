--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* spbm_config.lua
--*
--* DESCRIPTION:
--*     'config' commands for the 'spbm' system
--*     according to doc:
--*     http://docil.marvell.com/webtop/drl/objectId/0900dd88801a3892
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- saved from redefinition print function
local debug_print = print;

-- used for TTI UDB configuration
spbm_pbb_tunneled_packet_tti_type = "CPSS_DXCH_TTI_KEY_UDB_UDE6_E";
-- to specify etherType 0x88E7, default type ix 0xFFFF
local spbm_pbb_tunneled_packet_tunnel_type = "CPSS_TUNNEL_ETHERTYPE_TYPE_USER_DEFINED6_E";
local spbm_pbb_tti_rule_key_size = "CPSS_DXCH_TTI_KEY_SIZE_30_B_E";
local spbm_pbb_tunneled_packet_tti_pcl_id = 0x3BB;
local spbm_pbb_tunneled_packet_pcl_pcl_id = 0x3BC;
local spbm_pbb_pcl_cfg_entry_index_for_sid_to_vid = 0xFFE;
-- split horizon filtering constants
local spbm_pbb_brg_flt_mesh_id_offset     = 0;
local spbm_pbb_brg_flt_mesh_id_length     = 4;
spbm_pbb_brg_flt_mesh_id_value      = 0xB;
local spbm_pbb_brg_flt_assigned_source_id = 0xB;

-- this type should not be enabled and so not used
--spbm_mim_packet_tunnel_type = "CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E";

-- global database
spbm_internal_db =
{
    pe_nodes =
    {
        eport_by_bvid_by_mac =
        {
            -- [bvid] = {[mac] = eport}
        },
        info_by_eport =
        {
            -- [eport] =
            -- {
            --      port  = nil
            --      bvid  = nil
            --      mac   = nil
            -- }
        }
    },
    mc_groups =
    {
        flood_eport_by_bvid_mac_port =
        {
            -- [bvid] = {[mac_prefix] = {[phis_port] = eport}}
        },
        info_by_eport =
        {
            -- [eport] =
            -- {
            --      port        = nil
            --      bvid        = nil
            --      mac_prefix  = nil
            -- }
        }
    },
    services =
    {
        info_by_sid =
        {
            -- [sid] =
            -- {
            --     mc_groups =
            --     {
            --          [eport] = {}
            --     },
            --     evid = evid,
            --     evidx = evidx
            -- }
        },
        sid_by_evid =
        {
            -- [evid] = sid
        }
    }
};

-- returns true or false
local function spbm_is_global_configuration_needed(unset)
    local not_needed_limit;
    local found_objects;
    local tab_list, list_key, tab, key;

    if unset ~= true then
        -- found >= 1 objects - configuration already done
        not_needed_limit = 1;
    else
        -- found >= 2 objects - deconfiguration yet not needed
        -- only one object being removed
        not_needed_limit = 2;
    end

    found_objects = 0;
    tab_list =
    {
        spbm_internal_db.pe_nodes.info_by_eport,
        spbm_internal_db.mc_groups.info_by_eport,
        spbm_internal_db.services.info_by_sid
    };
    list_key = next(tab_list, nil);
    while list_key ~= nil do
        tab = tab_list[list_key];
        key = next(tab, nil);
        while key ~= nil do
            found_objects = found_objects + 1;
            if found_objects >= not_needed_limit then
                return false;
            end
            key = next(tab, key);
        end
        list_key = next(tab_list, list_key);
    end

    return true;
end

local function spbm_mac_to_me(command_data, devNum, unset)
    local mac = getGlobal("myMac");
    local macToMeEntryIndex;
    local mac2meEntryName = "SPBM global MAC to Me";
    local is_failed, result, OUT_values, apiName;

    if (mac == nil) and (unset ~= true) then
        return command_data:addErrorAndPrint("mac address command MAC not found in DB");
    end

    macToMeEntryIndex = sharedResourceNamedMac2meMemoAlloc(
        mac2meEntryName, 1 --[[size--]], false --[[fromEnd--]]);

    if macToMeEntryIndex == nil then
        return command_data:addErrorAndPrint("macToMeEntryIndex allocation failed");
    end

    local macToMeValue  = {};
    local macToMeMask  = {};

    if unset ~= true then
        -- given mac and any VLAN
        macToMeValue.mac    = mac;
        macToMeMask.mac     = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        macToMeValue.vlanId = 0;
        macToMeMask.vlanId  = 0;
    else
        -- never match
        macToMeValue.mac    = {0, 0, 0, 0, 0, 0};
        macToMeMask.mac     = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        macToMeValue.vlanId = 0;
        macToMeMask.vlanId  = 0xFFF;
        sharedResourceNamedMac2meMemoFree(mac2meEntryName);
    end

    local macToMeSrcInterfaceInfo = {};
    macToMeSrcInterfaceInfo.includeSrcInterface = "CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E";

    apiName = "cpssDxChTtiMacToMeSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN",     "GT_U8",                                            "devNum",         devNum },
        { "IN",     "GT_U32",                                           "entryIndex",     macToMeEntryIndex},
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",                       "valuePtr",       macToMeValue},
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",                       "maskPtr",        macToMeMask},
        { "IN",     "CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC",   "interfaceInfoPtr",macToMeSrcInterfaceInfo},
    });

    if is_failed == true then
        return nil;
    end

    return true;
end

local function spbm_l2mll_global(command_data, devNum, unset)
    local is_failed, result, OUT_values, apiName;

    -- both for set and unset - it must be default state

    apiName = "cpssDxChL2MllLookupForAllEvidxEnableSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",   "devNum",  devNum },
        { "IN", "GT_BOOL", "enable",  false }
    });

    apiName = "cpssDxChL2MllLookupMaxVidxIndexSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",  "devNum",        devNum },
        { "IN", "GT_U32", "maxVidxIndex",  0xFFF}
    });

end

local function spbm_eft_sht_global(command_data, devNum, unset)
    local is_failed, result, OUT_values, apiName;
    local egr_flt_enable, mesh_id_enable;

    if unset then
        egr_flt_enable = true;
        mesh_id_enable = false;
        spbm_pbb_brg_flt_mesh_id_offset=1;
        spbm_pbb_brg_flt_mesh_id_length=0;
    else
        egr_flt_enable = false;
        mesh_id_enable = true;
        spbm_pbb_brg_flt_mesh_id_offset=0;
        spbm_pbb_brg_flt_mesh_id_length=4;
    end

    apiName = "cpssDxChBrgVlanEgressFilteringEnable";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",   "devNum",  devNum },
        { "IN", "GT_BOOL", "enable",  egr_flt_enable }
    });

    apiName = "cpssDxChBrgRoutedUnicastEgressFilteringEnable";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",   "devNum",  devNum },
        { "IN", "GT_BOOL", "enable",  egr_flt_enable }
    });

    -- apiName = "cpssDxChBrgEgrMeshIdConfigurationSet";
    apiName = "cpssDxChL2MllSourceBasedFilteringConfigurationSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",   "devNum",         devNum },
        { "IN", "GT_BOOL", "enable",         mesh_id_enable},
        { "IN", "GT_U32",  "meshIdOffset",   spbm_pbb_brg_flt_mesh_id_offset},
        { "IN", "GT_U32",  "meshIdSize",     spbm_pbb_brg_flt_mesh_id_length}
    });
end

local function spbm_tti_rule_set_hw(
    command_data, devNum, rule_index,
    tunnel_mac_sa, b_vid, src_port,
    mac2me, tti_mc_copy, pcl_id,
    source_eport, tunnel_termination,
    pcl_cfg_index, packet_cmd_drop, assigned_source_id)

    local is_failed, result, OUT_values, apiName;
    local rule_format;
    local patternTable, maskTable, actionTable;

    if spbm_pbb_tti_rule_key_size == "CPSS_DXCH_TTI_KEY_SIZE_30_B_E" then
        rule_format = "CPSS_DXCH_TTI_RULE_UDB_30_E";
    elseif spbm_pbb_tti_rule_key_size == "CPSS_DXCH_TTI_KEY_SIZE_20_B_E" then
        rule_format = "CPSS_DXCH_TTI_RULE_UDB_20_E";
    elseif spbm_pbb_tti_rule_key_size == "CPSS_DXCH_TTI_KEY_SIZE_10_B_E" then
        rule_format = "CPSS_DXCH_TTI_RULE_UDB_10_E";
    else
        return command_data:addErrorAndPrint("spbm_tti_rule_set_hw: wrong rule size");
    end

    patternTable = {}
    patternTable.udbArray = {}
    maskTable = {}
    maskTable.udbArray = {}

    -- PCL ID and MAC to Me and is_src_trunk
    -- Metadata[180:176] - PCL_Id[4:0]
    -- Metadata[188:184] - PCL_Id[9:5]
    -- Metadata[183]     - mac to me
    -- Metadata[181]     - Local Device Source is Trunk
    -- {0, "METADATA", 22}, Metadata[183:176]
    -- {1, "METADATA", 23}, Metadata[191:184]

    patternTable.udbArray[0]     = 0;
    maskTable.udbArray[0] = 0;
    maskTable.udbArray[1] = 0;
    patternTable.udbArray[1]     = 0;

    if tti_mc_copy then
        -- the MAC2ME is implicitly set for 'instance 1'
        -- see FS : "MAC2ME Assignment for PPB Multicast Duplication"
        -- so adding here the requirement for it to be '1' too !
        -- this to check the fix for JIRA : CPSS-5031 : BC2 Simulation - In PBB Multicast Duplication the MAC2ME is not set
        mac2me = 1
    end

    if pcl_id then
        patternTable.udbArray[0]     =
            bit_or(patternTable.udbArray[0], bit_and(pcl_id, 0x1F));
        maskTable.udbArray[0] = bit_or(maskTable.udbArray[0], 0x1F);
        patternTable.udbArray[1]     =
            bit_or(patternTable.udbArray[1], bit_and(bit_shr(pcl_id, 5), 0x1F));
        maskTable.udbArray[1] = bit_or(maskTable.udbArray[1], 0x1F);
    end
    if mac2me then
        patternTable.udbArray[0]     =
            bit_or(patternTable.udbArray[0], bit_and(bit_shl(mac2me, 7), 0x80));
        maskTable.udbArray[0] = bit_or(maskTable.udbArray[0], 0x80);
    end
    if src_port then
        -- check that source is not trunk
        maskTable.udbArray[0] = bit_or(maskTable.udbArray[0], 0x20);
    end

    -- TUNNEL_MAC_SA
    -- {2, "L2", 6},
    -- {3, "L2", 7},
    -- {4, "L2", 8},
    -- {5, "L2", 9},
    -- {6, "L2", 10},
    -- {7, "L2", 11},
    if tunnel_mac_sa then
        patternTable.udbArray[2]     = tonumber("0x" .. tunnel_mac_sa[1]);
        maskTable.udbArray[2] = 0xFF;
        patternTable.udbArray[3]     = tonumber("0x" .. tunnel_mac_sa[2]);
        maskTable.udbArray[3] = 0xFF;
        patternTable.udbArray[4]     = tonumber("0x" .. tunnel_mac_sa[3]);
        maskTable.udbArray[4] = 0xFF;
        patternTable.udbArray[5]     = tonumber("0x" .. tunnel_mac_sa[4]);
        maskTable.udbArray[5] = 0xFF;
        patternTable.udbArray[6]     = tonumber("0x" .. tunnel_mac_sa[5]);
        maskTable.udbArray[6] = 0xFF;
        patternTable.udbArray[7]     = tonumber("0x" .. tunnel_mac_sa[6]);
        maskTable.udbArray[7] = 0xFF;
    end

    -- B_VID
    -- {8, "L2", 14}, -- high, mask 0x0F
    -- {9, "L2", 15}, -- low
    if b_vid then
        patternTable.udbArray[8]     = bit_and(bit_shr(b_vid, 8), 0x0F);
        maskTable.udbArray[8] = 0x0F;
        patternTable.udbArray[9]     = bit_and(b_vid, 0xFF);
        maskTable.udbArray[9] = 0xFF;
    end

    -- sorce port
    -- Metadata[220:208] Local Device Source ePort/TrunkID
    -- {10, "METADATA", 27}, -- high, mask 0x1F
    -- {11, "METADATA", 26}, -- low
    if src_port then
        patternTable.udbArray[10] = bit_and(bit_shr(src_port, 8), 0x1F);
        maskTable.udbArray[10] = 0x1F;
        patternTable.udbArray[11] = bit_and(src_port, 0xFF);
        maskTable.udbArray[11] = 0xFF;
    end

    -- TTI MC Copy
    -- Metadata[167] - TTI Multicast Duplication Instance
    -- {12, "METADATA", 20},
    if tti_mc_copy then
        patternTable.udbArray[12] = bit_and(bit_shl(tti_mc_copy, 7), 0x80);
        maskTable.udbArray[12] = 0x80;
    end

    -- actions
    actionTable = {}
    actionTable["type2"] = {}

    actionTable["type2"]["command"] = "CPSS_PACKET_CMD_FORWARD_E"
    if packet_cmd_drop then
        actionTable["type2"]["command"] = "CPSS_PACKET_CMD_DROP_HARD_E"
    end
    if source_eport then
        actionTable["type2"]["sourceEPortAssignmentEnable"] = true;
        actionTable["type2"]["sourceEPort"] = source_eport;
    end
    if tunnel_termination then
        actionTable["type2"]["tunnelTerminate"] = true;
        actionTable["type2"]["ttPassengerPacketType"] =
            "CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E";
        -- header length calculated from L3
        -- the packet parts below related to L2
        --     6 --[[mac_da--]] + 6 --[[mac_sa--]]
        --     + 2 --[[0x88A8--]] + 2 --[[Flags,b_vid--]]
        --     + 2 --[[0x88E7--]];
        actionTable["type2"]["ttHeaderLength"] = 4 --[[Flags, i_sid--]];
    end
    if pcl_cfg_index then
        actionTable["type2"]["pcl0OverrideConfigIndex"] =
            "CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E";
        actionTable["type2"]["iPclConfigIndex"] = pcl_cfg_index;
    end
    if assigned_source_id then
        actionTable["type2"]["sourceIdSetEnable"] = true;
        actionTable["type2"]["sourceId"] = assigned_source_id;
    end
    actionTable["type2"]["tag1VlanCmd"]  = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E";

    --print("rule_index" , rule_index)
    apiName = "cpssDxChTtiRuleSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "index", rule_index },
        { "IN", "CPSS_DXCH_TTI_RULE_TYPE_ENT",    "ruleType", rule_format },
        { "IN", "CPSS_DXCH_TTI_RULE_UNT_udbArray", "patternPtr", patternTable },
        { "IN", "CPSS_DXCH_TTI_RULE_UNT_udbArray", "maskPtr", maskTable },
        { "IN", "CPSS_DXCH_TTI_ACTION_TYPE_ENT",   "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT" },
        { "IN", "CPSS_DXCH_TTI_ACTION_UNT_type2",  "actionPtr", actionTable }
    });
    if (result ~= 0) or is_failed == true then
        return false
    end
    return true;
end

local function spbm_tti_global(command_data, devNum, unset)
    local is_failed, result, OUT_values, apiName;
    local key_size;
    local ethertype;
    local sum_result = true;
    local i;
    local anchor, offset;
    local alignment, size, rule_string, rule_index;


    if unset ~= true then
        key_size = spbm_pbb_tti_rule_key_size;
    else
        key_size = "CPSS_DXCH_TTI_KEY_SIZE_30_B_E";
    end

    apiName = "cpssDxChTtiPacketTypeKeySizeSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN",     "GT_U8",                  "devNum",   devNum },
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType",  spbm_pbb_tunneled_packet_tti_type },
        { "IN", "CPSS_DXCH_TTI_KEY_SIZE_ENT", "size",     key_size }
    });

    if is_failed == true then
        sum_result = nil;
    end

    if unset ~= true then
        ethertype = 0x88E7;
    else
        ethertype = 0xFFFF;
    end

    -- used to configue _UDE6 packet type for user defined bytes
    apiName = "cpssDxChTtiEthernetTypeSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN",     "GT_U8",                      "devNum",   devNum },
        { "IN", "CPSS_TUNNEL_ETHERTYPE_TYPE_ENT", "ethertypeType",
               spbm_pbb_tunneled_packet_tunnel_type },
        { "IN", "GT_U32",                         "ethertype", ethertype }
    });

    if is_failed == true then
        sum_result = nil;
    end

    if unset ~= true then
        -- used to configue _MIM in HA (and in TTI)
        -- the default value of TTI is correct,
        -- but of HA is 0x8100 - wrong according to Cider
        -- must not be restored on uset
        apiName = "cpssDxChTtiEthernetTypeSet";
        is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
            command_data, apiName, {
            { "IN",     "GT_U8",                      "devNum",   devNum },
            { "IN", "CPSS_TUNNEL_ETHERTYPE_TYPE_ENT", "ethertypeType",
                   "CPSS_TUNNEL_ETHERTYPE_TYPE_MIM_E" },
            { "IN", "GT_U32",                         "ethertype", 0x88E7 }
        });

        if is_failed == true then
            sum_result = nil;
        end
    end

    apiName = "cpssDxChTtiPclIdSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",                         "devNum",   devNum },
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",
               spbm_pbb_tunneled_packet_tti_type },
        { "IN", "GT_U32",                         "pclId",
               spbm_pbb_tunneled_packet_tti_pcl_id}
    });

    if is_failed == true then
        sum_result = nil;
    end

    -- set user defined bytes
    local udb_cfg =
    {
        -- {index, anchor, offset},
        -- anchor is substring: CPSS_DXCH_TTI_OFFSET_anchor_E

        -- PCL ID and MAC to Me and is_src_trunk
        -- Metadata[180:176] - PCL_Id[4:0]
        -- Metadata[188:184] - PCL_Id[9:5]
        -- Metadata[183]     - mac to me
        -- Metadata[181]     - Local Device Source is Trunk
        {0, "METADATA", 22},
        {1, "METADATA", 23},

        -- TUNNEL_MAC_SA
        {2, "L2", 6},
        {3, "L2", 7},
        {4, "L2", 8},
        {5, "L2", 9},
        {6, "L2", 10},
        {7, "L2", 11},

        -- B_VID
        {8, "L2", 14}, -- high, mask 0x0F
        {9, "L2", 15}, -- low

        -- sorce port
        -- Metadata[220:208] Local Device Source ePort/TrunkID
        {10, "METADATA", 27}, -- high, mask 0x1F
        {11, "METADATA", 26}, -- low

        -- TTI MC Copy
        -- Metadata[167] - TTI Multicast Duplication Instance
        {12, "METADATA", 20},
    };

    for i = 1, #udb_cfg do
        if unset ~= true then
            anchor = "CPSS_DXCH_TTI_OFFSET_" .. udb_cfg[i][2] .. "_E";
            offset = tonumber(udb_cfg[i][3]);
        else
            anchor = "CPSS_DXCH_TTI_OFFSET_INVALID_E";
            offset = 0;
        end

        apiName = "cpssDxChTtiUserDefinedByteSet";
        is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
            command_data, apiName, {
            { "IN",     "GT_U8",                      "devNum",      devNum },
            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT",     "keyType",
                   spbm_pbb_tunneled_packet_tti_type },
            { "IN", "GT_U32",                         "udbIndex",    udb_cfg[i][1] },
            { "IN", "CPSS_DXCH_TTI_OFFSET_TYPE_ENT",  "offsetType",  anchor},
            { "IN", "GT_U8",                          "offset",      offset}
        });

        if is_failed == true then
            sum_result = nil;
        end
    end

    if spbm_pbb_tti_rule_key_size == "CPSS_DXCH_TTI_KEY_SIZE_30_B_E" then
        alignment = 3;
        size      = 3;
    elseif spbm_pbb_tti_rule_key_size == "CPSS_DXCH_TTI_KEY_SIZE_20_B_E" then
        alignment = 2;
        size      = 2;
    elseif spbm_pbb_tti_rule_key_size == "CPSS_DXCH_TTI_KEY_SIZE_10_B_E" then
        alignment = 1;
        size      = 1;
    else
        return command_data:addErrorAndPrint("spbm_tti_global: wrong rule size");
    end
    rule_string = "SPBM_TTI_RULE Dropping Not Matched MC Duplications"

    rule_index = sharedResourceNamedTti0RulesMemoAlloc(
        rule_string, alignment, size, true --[[fromEnd--]]);

    if rule_index == nil then
        return command_data:addErrorAndPrint(
            "SPBM TTI Drop Not Matched MC Duplications : rule allocation failed");
    end

    if unset == true then
        sharedResourceNamedTti0RulesMemoFree(rule_string);

        apiName = "cpssDxChTtiRuleValidStatusSet";
        is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
            command_data, apiName, {
            { "IN", "GT_U8",     "devNum",   devNum },
            { "IN", "GT_U32",    "index",    rule_index },
            { "IN", "GT_BOOL",   "valid",    false }
        });

        if is_failed == true then
           sum_result = nil;
        end
    else
        sum_result = sum_result and spbm_tti_rule_set_hw(
            command_data, devNum, rule_index,
            nil --[[tunnel_mac_sa--]],
            nil --[[b_vid--]],
            nil --[[src_port--]],
            nil --[[mac2me--]],
            1   --[[tti_mc_copy--]],
            spbm_pbb_tunneled_packet_tti_pcl_id,
            nil --[[source_eport--]],
            nil --[[tunnel_termination--]],
            nil --[[pcl_cfg_index--]],
            true --[[packet_cmd_drop--]],
            nil  --[[assigned_source_id--]]);
    end
    return sum_result;
end

function spbm_tti_rule_set(
    command_data, devNum,
    tunnel_mac_sa, b_vid, src_port, rule_for_uc,
    source_eport, unset)

    local mac2me, tti_mc_copy;
    local type_str, rule_string;
    local rule_index, alignment, size;
    local is_failed, result, OUT_values, apiName;

    if rule_for_uc == true then
        mac2me      = 1;
        tti_mc_copy = nil;
        type_str = "mac2me"
    else
        mac2me      = nil;
        tti_mc_copy = 1;
        type_str = "tti_mc_copy"
    end

    rule_string = "SPBM_TTI_RULE"  ..
        " MAC_SA " .. tunnel_mac_sa.string ..
        " B_VID " .. string.format("0x%04X", b_vid) ..
        " SRC_PORT " .. string.format("0x%02X", src_port) ..
        " " .. type_str;

    if spbm_pbb_tti_rule_key_size == "CPSS_DXCH_TTI_KEY_SIZE_30_B_E" then
        alignment = 3;
        size      = 3;
    elseif spbm_pbb_tti_rule_key_size == "CPSS_DXCH_TTI_KEY_SIZE_20_B_E" then
        alignment = 2;
        size      = 2;
    elseif spbm_pbb_tti_rule_key_size == "CPSS_DXCH_TTI_KEY_SIZE_10_B_E" then
        alignment = 1;
        size      = 1;
    else
        return command_data:addErrorAndPrint("spbm_tti_rule_set: wrong rule size");
    end

    rule_index = sharedResourceNamedTti0RulesMemoAlloc(
        rule_string, alignment, size, false --[[fromEnd--]]);

    if rule_index == nil then
        return command_data:addErrorAndPrint("spbm_tti_rule_set: rule allocation failed");
    end

    if unset == true then
        sharedResourceNamedTti0RulesMemoFree(rule_string);

        apiName = "cpssDxChTtiRuleValidStatusSet";
        is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
            command_data, apiName, {
            { "IN", "GT_U8",     "devNum",   devNum },
            { "IN", "GT_U32",    "index",    rule_index },
            { "IN", "GT_BOOL",   "valid",    false }
        });

        if is_failed == true then
            return nil;
        end
        return true;
    end

    return spbm_tti_rule_set_hw(
        command_data, devNum, rule_index,
        tunnel_mac_sa, b_vid, src_port,
        mac2me, tti_mc_copy,
        spbm_pbb_tunneled_packet_tti_pcl_id,
        source_eport, true --[[tunnel_termination--]],
        spbm_pbb_pcl_cfg_entry_index_for_sid_to_vid,
        nil --[[packet_cmd_drop--]],
        spbm_pbb_brg_flt_assigned_source_id);
end

local function spbm_pcl_default_rule_set(command_data, devNum, pcl_id, unset)
    local is_failed, result, OUT_values, apiName;
    local rule_string = "SPBM_PCL_DEFAULT_RULE__hard_drop";
    local rule_index = sharedResourceNamedPclRulesMemoAlloc(
        rule_string, 3 --[[alignment--]], 3 --[[size--]], true --[[fromEnd--]]);
    if rule_index == nil then
        return command_data:addErrorAndPrint("spbm_pcl_default_rule_set: rule allocation failed");
    end

    if unset == true then
        sharedResourceNamedPclRulesMemoFree(rule_string);

        apiName = "cpssDxChPclRuleValidStatusSet";
        is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
            command_data, apiName, {
            { "IN", "GT_U8",                   "devNum",      devNum },
            { "IN", "CPSS_PCL_RULE_SIZE_ENT",  "ruleSize",    "CPSS_PCL_RULE_SIZE_STD_E" },
            { "IN", "GT_U32",                  "ruleIndex",    rule_index },
            { "IN", "GT_BOOL",                 "valid",        false }
        });

        if is_failed == true then
            return nil;
        end
        return true;
    end

    -- rule set
    local mask = {commonIngrUdb = {pclId = 0x3FF}};
    local pattern = {commonIngrUdb = {pclId = pcl_id}};
    local action = {
        pktCmd = "CPSS_PACKET_CMD_DROP_HARD_E",
        mirror = {cpuCode = "CPSS_NET_FIRST_USER_DEFINED_E"}
        };

    -- CPSS_DXCH_PCL_RULE_FORMAT_UNT replaced by this union member
    apiName = "cpssDxChPclRuleSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",                   "devNum",           devNum },
        { "IN", "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",  "ruleFormat",
            "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E" },
        { "IN", "GT_U32",                  "ruleIndex",        rule_index },
        { "IN", "CPSS_DXCH_PCL_RULE_OPTION_ENT", "ruleOptionsBmp", 0 },
        { "IN", "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC", "maskPtr",     mask },
        { "IN", "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC", "patternPtr",  pattern },
        { "IN", "CPSS_DXCH_PCL_ACTION_STC",                           "actionPtr",   action }
    });
    if is_failed == true then
        return nil;
    end
    return true;
end

local function spbm_pcl_global(command_data, devNum, unset)
    local is_failed, result, OUT_values, apiName;
    local sum_result = true;
    local interfaceInfo, lookupCfg, enable;
    local i, j;
    local anchor, offset;

    if unset ~= true then
        apiName = "cpssDxChPclIngressPolicyEnable";
        is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
            command_data, apiName, {
            { "IN", "GT_U8",     "devNum",   devNum },
            { "IN", "GT_BOOL",   "enable",   true }
        });

        if is_failed == true then
            return nil;
        end
    end

    -- disable on "set", enable on "unset"
    enable = (unset == true);
    apiName = "cpssDxCh3PclTunnelTermForceVlanModeEnableSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",     "devNum",   devNum },
        { "IN", "GT_BOOL",   "enable",   enable}
    });

    enable = (unset ~= true);
    interfaceInfo = {
        type = "CPSS_INTERFACE_INDEX_E",
        index = spbm_pbb_pcl_cfg_entry_index_for_sid_to_vid
    };

    lookupCfg = {
        enableLookup = enable,
        pclId = spbm_pbb_tunneled_packet_pcl_pcl_id,
        groupKeyTypes = {
           nonIpKey = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E",
           ipv4Key  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E",
           ipv6Key  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"
        }
    };

    apiName = "cpssDxChPclCfgTblSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",                        "devNum",   devNum },
        { "IN", "CPSS_INTERFACE_INFO_STC",      "interfaceInfoPtr",    interfaceInfo},
        { "IN", "CPSS_PCL_DIRECTION_ENT",       "direction",   "CPSS_PCL_DIRECTION_INGRESS_E" },
        { "IN", "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",   "CPSS_PCL_LOOKUP_0_0_E" },
        { "IN", "CPSS_DXCH_PCL_LOOKUP_CFG_STC", "lookupCfgPtr",    lookupCfg}
    });

    if is_failed == true then
        sum_result = nil;
    end

    local i_sid_offset = 6 --[[mac_da--]] + 6 --[[mac_sa--]]
        + 4 --[[0x88A8 vlan_tag--]] + 3 --[[0x88E7 and flags--]]
    local udb_cfg = {
        {0, "TUNNEL_L2", i_sid_offset},
        {1, "TUNNEL_L2", (i_sid_offset + 1)},
        {2, "TUNNEL_L2", (i_sid_offset + 2)}
    };
    local packetType_list = {
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_UDE_E",
        -- "CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E", - legacy devices only, not supported for SIP5
        "CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E",
        "CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E"
    };

    for j = 1, #packetType_list do
        for i = 1, #udb_cfg do
            if unset ~= true then
                anchor = "CPSS_DXCH_PCL_OFFSET_" .. udb_cfg[i][2] .. "_E";
                offset = tonumber(udb_cfg[i][3]);
            else
                anchor = "CPSS_DXCH_PCL_OFFSET_INVALID_E";
                offset = 0;
            end

            apiName = "cpssDxChPclUserDefinedByteSet";
            is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
                command_data, apiName, {
                { "IN",     "GT_U8",                      "devNum",      devNum },
                { "IN", "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",     "ruleFormat",
                    "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"},
                { "IN", "CPSS_DXCH_PCL_PACKET_TYPE_ENT",     "packetType", packetType_list[j]},
                { "IN", "CPSS_PCL_DIRECTION_ENT",            "direction",
                    "CPSS_PCL_DIRECTION_INGRESS_E" },
                    { "IN", "GT_U32",                         "udbIndex",    udb_cfg[i][1] },
                { "IN", "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",  "offsetType",  anchor},
                { "IN", "GT_U8",                          "offset",      offset}
            });

            if is_failed == true then
                sum_result = nil;
            end
        end
    end

    sum_result = sum_result and spbm_pcl_default_rule_set(
        command_data, devNum, spbm_pbb_tunneled_packet_pcl_pcl_id, unset)

    return sum_result;
end

local function spbm_pcl_rule_set(command_data, devNum, pcl_id, sid, vid, unset)
    local is_failed, result, OUT_values, apiName;
    local rule_string = "SPBM_PCL_RULE "  ..
        " SID " .. string.format("0x%04X", sid) ..
        " VID " .. string.format("0x%04X", vid);
    local rule_index = sharedResourceNamedPclRulesMemoAlloc(
        rule_string, 3 --[[alignment--]], 3 --[[size--]], false --[[fromEnd--]]);
    if rule_index == nil then
        return command_data:addErrorAndPrint("spbm_pcl_rule_set: rule allocation failed");
    end

    if unset == true then
        sharedResourceNamedPclRulesMemoFree(rule_string);

        apiName = "cpssDxChPclRuleValidStatusSet";
        is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
            command_data, apiName, {
            { "IN", "GT_U8",                   "devNum",      devNum },
            { "IN", "CPSS_PCL_RULE_SIZE_ENT",  "ruleSize",    "CPSS_PCL_RULE_SIZE_STD_E" },
            { "IN", "GT_U32",                  "ruleIndex",    rule_index },
            { "IN", "GT_BOOL",                 "valid",        false }
        });

        if is_failed == true then
            return nil;
        end
        return true;
    end

    -- rule set
    local mask = {
            commonIngrUdb = {pclId = 0x3FF},
            udb0_15 = {[0] = 0xFF, [1] = 0xFF, [2] = 0xFF}
        };
    local pattern = {
            commonIngrUdb = {pclId = pcl_id},
            udb0_15 = {
                [0] = bit_and(bit_shr(sid, 16), 0xFF),
                [1] = bit_and(bit_shr(sid, 8), 0xFF),
                [2] = bit_and(sid, 0xFF)}
        };
    local action = {
        pktCmd = "CPSS_PACKET_CMD_FORWARD_E",
        vlan = {ingress = {
                modifyVlan = "CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E",
                vlanId = vid,
                precedence = "CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E"
            }};
        };

    if(is_sip_6(devNum)) then
        -- the <VRF-ID> per vlan moved from Bridge (L2i) to the TTI , 
        -- so modification of eVlan by the PCL action for packets that need <VRF-ID>
        -- must also assign the <VRF-ID>
        
        -- get the vrf-id from the vlan entry
        local result, values = vlan_info_get(devNum, vid)
        local vrfId = values.vrfId

        if vrfId ~= 0 then
            action.redirect = { data = {} }
            action.redirect.redirectCmd = "CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E"
            action.redirect.data.vrfId = vrfId
        end
    end

    -- CPSS_DXCH_PCL_RULE_FORMAT_UNT replaced by this union member
    apiName = "cpssDxChPclRuleSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",                   "devNum",           devNum },
        { "IN", "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",  "ruleFormat",
            "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E" },
        { "IN", "GT_U32",                  "ruleIndex",        rule_index },
        { "IN", "CPSS_DXCH_PCL_RULE_OPTION_ENT", "ruleOptionsBmp", 0 },
        { "IN", "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC", "maskPtr",     mask },
        { "IN", "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC", "patternPtr",  pattern },
        { "IN", "CPSS_DXCH_PCL_ACTION_STC",                           "actionPtr",   action }
    });
    if is_failed == true then
        return nil;
    end
    return true;
end

local function spbm_bridge_multicast_address_group_flood_callback(
    command_data, devNum, db_key_str, port_range,
    param_vidx, param_eVidx, unset)

    local is_failed, result, OUT_values, apiName;
    local info_by_eport = spbm_internal_db.mc_groups.info_by_eport;
    local evidx_string, vidx_string;
    local l2mll_string, l2mll_ltt_index, vidx, evidx;
    local eport_list, eport, eport_idx, mll_node, hwDevNum, i;

    -- eports for flooding are common for all services:
    -- tunneling MAC address 24 least bytes is service id from vlan table
    -- l2mll chain can be built (using existing flooding eports)
    -- for service that yet not defined and destroyed after service destroying

    -- "set" function allocates vidx , includes it to l2mll chain and returnes it to caller
    -- caller addes physical ports to vidx.
    -- ports in the list that >= 512 treated as eport: checked in DB as flooding eports
    -- and included to l2mll chain
    -- if "set" function found existing l2mll chain it will be removed and rebuilt
    -- with the same vidx and evidx.

    vidx_string  = "SPBM_MAC_MC_VIDX"  .. db_key_str;
    evidx_string = "SPBM_MAC_MC_EVIDX" .. db_key_str;
    l2mll_string = "SPBM_MAC_MC_L2MLL" .. db_key_str;

    -- alloc or find anycase, will be freed by "unser"
    vidx = sharedResourceNamedVidxMemoAlloc(vidx_string, 1 --[[size--]]);
    evidx = sharedResourceNamedEVidxMemoAlloc(evidx_string, 1 --[[size--]]);
    l2mll_ltt_index = spbm_sevice_id_vlan_evidx_to_l2mll_ltt_index(evidx);

    -- remove any case. "set" will build it again.
    util_l2mll_remove_chain(
        command_data, devNum, l2mll_string, l2mll_ltt_index, {}--[[ltt_param--]])

    if unset then
        sharedResourceNamedVidxMemoFree(vidx_string);
        sharedResourceNamedEVidxMemoFree(evidx_string);
        -- FDB entry and vidx entry will be cleaned by caller
        return {vidx = vidx, eVidx = evidx};
    end

    -- "set" implementation
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

    eport_list = {};
    -- names = {};
    i = 1;
    eport_idx = next(port_range, nil)
    while eport_idx do
        eport = port_range[eport_idx];
        if eport >= 512  then
            if not info_by_eport[eport] then
                command_data:setFailLocalStatus()
                command_data:addWarning(
                    "Eport %d not in flood ports DB, omitted ", eport);
            else
                eport_list[i] = eport;
                i = i + 1;
            end
        end
        eport_idx = next(port_range, eport_idx);
    end

    mll_node = {
        egressInterface = {
            type = "CPSS_INTERFACE_VIDX_E",
            vidx = vidx
        }
    };
    util_l2mll_add_node(
        command_data, devNum, l2mll_string --[[chainName--]],
        "VIDX" --[[nodeName--]], mll_node --[[nodeData--]],
        l2mll_ltt_index --[[lttEntryIndex--]], {}--[[ltt_param--]]);

    eport_idx = next(eport_list, nil)
    while eport_idx do
        eport = eport_list[eport_idx];
        mll_node = {
            egressInterface = {
                type = "CPSS_INTERFACE_PORT_E",
                devPort = {
                    -- the CPSS inteface structure uses name "hwDevNum"
                    -- but LUA CLI uses name "devNum" (maybe it is a bug)
                    hwDevNum = hwDevNum,
                    devNum = hwDevNum,
                    portNum  = eport
                }
            },
            meshId = spbm_pbb_brg_flt_mesh_id_value
        };
        util_l2mll_add_node(
            command_data, devNum, l2mll_string --[[chainName--]],
            string.format("eport %d", eport) --[[nodeName--]],
            mll_node --[[nodeData--]],
            l2mll_ltt_index --[[lttEntryIndex--]], {}--[[ltt_param--]]);
        eport_idx = next(eport_list, eport_idx);
    end

    return {vidx = vidx, eVidx = evidx};
end

-- function should be called at the begining of adding/removing
-- any independent object: pe_nodes,mc_groups and services
function spbm_global_configuration(command_data, devNum , unset)
    local all_devices;
    local mc_callback;
    if not spbm_is_global_configuration_needed(unset) then
        return true;
    end

    spbm_mac_to_me(command_data, devNum, unset);
    spbm_tti_global(command_data, devNum, unset);
    spbm_pcl_global(command_data, devNum, unset);
    spbm_l2mll_global(command_data, devNum, unset);
    spbm_eft_sht_global(command_data, devNum, unset);
    if unset then
        mc_callback = nil;
    else
        mc_callback = spbm_bridge_multicast_address_group_flood_callback;
    end
    bridge_multicast_address_group_flood_callback_bind(mc_callback);
end

function spbm_sevice_id_vlan_l2mll_chain_name(i_sid)
    return "SPBM_l2mll_chain " ..
        "I_SID: " .. string.format("0x%04X", i_sid);
end

function spbm_sevice_id_vlan_evidx_to_l2mll_ltt_index(e_vidx)
    return (e_vidx - 0x1000);
end

local function spbm_sevice_id_vlan_entry_set(command_data, devNum, i_sid, e_vid, e_vidx, unset)
    local is_failed, result, OUT_values, apiName;
    local chain_name, node_name;
    local w_i_sid, w_e_vidx;
    local ltt_index, mll_node;

    ltt_index = spbm_sevice_id_vlan_evidx_to_l2mll_ltt_index(e_vidx);
    chain_name = spbm_sevice_id_vlan_l2mll_chain_name(i_sid);
    node_name  = "VLAN_FLOOD_VIDX";
    mll_node = {
        egressInterface = {
            type = "CPSS_INTERFACE_VIDX_E",
            vidx = 0xFFF
        }
    };

    if unset ~= true then
        w_i_sid   = i_sid;
        w_e_vidx  = e_vidx;
        util_l2mll_add_node(
            command_data, devNum, chain_name, node_name, mll_node,
            ltt_index, {} --[[ltt_param--]]);
    else
        w_i_sid   = 0;
        w_e_vidx  = 0xFFF;
        util_l2mll_remove_node(
            command_data, devNum, chain_name, node_name,
            ltt_index, {} --[[ltt_param--]]);
    end

    apiName = "cpssDxChTunnelStartEgessVlanTableServiceIdSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",    "devNum",           devNum },
        { "IN", "GT_U16",   "vlanId",           e_vid},
        { "IN", "GT_U32",   "vlanServiceId",    w_i_sid},
    });

    apiName = "cpssDxChBrgVlanFloodVidxModeSet";
    is_failed, result, OUT_values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
        { "IN", "GT_U8",    "devNum",           devNum },
        { "IN", "GT_U16",   "vlanId",           e_vid},
        { "IN", "GT_U32",   "floodVidx",        w_e_vidx},
        { "IN", "CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT",    "floodVidxMode",
            "CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E"},
    });

end

local function spbm_sevice_id_to_vlan_hw(command_data, devNum , i_sid, e_vid, e_vidx, unset)
    spbm_pcl_rule_set(
        command_data, devNum, spbm_pbb_tunneled_packet_pcl_pcl_id,
        i_sid, e_vid, unset);
    spbm_sevice_id_vlan_entry_set(command_data, devNum, i_sid, e_vid, e_vidx, unset);
end

local function spbm_sevice_id_to_vlan(params)
    local command_data = Command_Data();
    local unset   = params["unset"];
    local i_sid   = tonumber(params["i_sid"]);
    local e_vid   = tonumber(params["e_vid"]);
    local e_vidx, e_vidx_name;
    local mc_groups;
    local info_by_sid = spbm_internal_db.services.info_by_sid;
    local sid_by_evid = spbm_internal_db.services.sid_by_evid;

    e_vidx_name = "SPBM_flooding_evidx " ..
        "I_SID " .. string.format("0x%04X", i_sid);

    -- check DB
    if unset ~= true then
        -- check that i_sid and e_vid both are new
        if info_by_sid[i_sid] ~= nil then
            return command_data:addErrorAndPrint("i-sid already configured");
        end
        if sid_by_evid[e_vid] ~= nil then
            return command_data:addErrorAndPrint("e-vid already used for some i-sid");
        end
        if spbm_internal_db.pe_nodes.eport_by_bvid_by_mac[e_vid] ~= nil then
            return command_data:addErrorAndPrint("e-vid already used for some remote PE-device");
        end
        if spbm_internal_db.mc_groups.flood_eport_by_bvid_mac_port[e_vid] ~= nil then
            return command_data:addErrorAndPrint("e-vid already used for some remote MC-group");
        end
    else
        if info_by_sid[i_sid] == nil then
            return command_data:addErrorAndPrint("i-sid not found in DB");
        end
        e_vid = info_by_sid[i_sid].evid;
        if sid_by_evid[e_vid] ~= i_sid then
            return command_data:addErrorAndPrint("DB error - i_sid -> e_vid -> another i_sid");
        end
        mc_groups = info_by_sid[i_sid].mc_groups;
        if next(mc_groups) ~= nil then
            return command_data:addErrorAndPrint("Cannot remove i-sid, it yet has MC-groups");
        end
    end

    -- allocate or find allocated
    e_vidx = sharedResourceNamedEVidxMemoAlloc(e_vidx_name, 1 --[[size--]]);
    if not e_vidx then
        return command_data:addErrorAndPrint("e-vidx allocation failed");
    end

    -- HW configuration, the problems will be stored to command_data
    -- set all devices
    local devlist = wrLogWrapper("wrlDevList")
    local _index, devNum

    for _index, devNum in pairs(devlist) do
        spbm_global_configuration(command_data, devNum , unset);
        spbm_sevice_id_to_vlan_hw(command_data, devNum ,i_sid, e_vid, e_vidx, unset);
    end


    if true ~= command_data:getCommandExecutionResults() then
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
    end

    if unset ~= true then
        -- adding data
        info_by_sid[i_sid] = {mc_groups = {}, evid = e_vid, evidx = e_vidx};
        sid_by_evid[e_vid] = i_sid;
    else
        -- removing data
        info_by_sid[i_sid] = nil;
        sid_by_evid[e_vid] = nil;
        e_vidx = sharedResourceNamedEVidxMemoFree(e_vidx_name);
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

CLI_addCommand("config", "spbm service-vlan",
    { func = function(params)
            params.unset = false;
            return spbm_sevice_id_to_vlan(params);
        end,
      help = "SPBM map service Id to customer edge VLAN",
      params =
      {
            { type="named",
               { format = "i-sid %s", name = "i_sid",
                   help = "service-id (up to 24-bit)" },
                { format = "e-vid %s", name = "e_vid",
                    help = "e-vid representing service-id on customer edge" },
               requirements = {["e_vid"] = {"i_sid"}},
               mandatory = {"e_vid"}
            }
      }
    });

CLI_addCommand("config", "no spbm service-vlan",
    { func = function(params)
            params.unset = true;
            return spbm_sevice_id_to_vlan(params);
        end,
      help = "SPBM unmap service Id to customer edge VLAN",
      params =
      {
            { type="named",
               { format = "i-sid %s", name = "i_sid",
                   help = "service-id (up to 24-bit)" },
               mandatory = {"i_sid"}
            }
      }
    });

--[[
    'show' commands to see the SPBM configurations
--]]

local uniEportCol = {
    {{1}, "eport", "d"},
    {{2}, "phyPort", "d"},
    {{3}, "BVID", "d"},
    {{4}, "MAC", "s"}
};
local mcEportCol = {
    {{1}, "eport", "d"},
    {{2}, "phyPort", "d"},
    {{3}, "BVID", "d"},
    {{4}, "MAC Prefix", "s"}
};
local serviceVidCol = {
    {{1}, "I-SID   ", "d"},
    {{2}, "VID   ", "d"},
    {{3}, "EVIDX ", "d"},
    {{4}, "MC-GROUP-EPORTS", "s"}
};

local function spbm_show_config(params)
    local line, wrk_str;
    print("");
    print("--- SPBM configurations: ---");
    print("");
    print("SPBM unicast over PBB eports:");
    print(string_for_table_break(nil, uniEportCol));
    print(string_for_table_title(nil, uniEportCol));
    print(string_for_table_break(nil, uniEportCol));
    for _eport, _info in pairs(spbm_internal_db.pe_nodes.info_by_eport) do
        line  = {_eport, _info.port, _info.bvid, _info.mac.string};
        print(string_for_table_line(nil, uniEportCol, line));
    end
    print(string_for_table_break(nil, uniEportCol));
    print("");
    print("SPBM MC-groups over PBB eports:");
    print(string_for_table_break(nil, mcEportCol));
    print(string_for_table_title(nil, mcEportCol));
    print(string_for_table_break(nil, mcEportCol));
    for _eport, _info in pairs(spbm_internal_db.mc_groups.info_by_eport) do
        line  = {_eport, _info.port, _info.bvid, _info.mac_prefix.string};
        print(string_for_table_line(nil, mcEportCol, line));
    end
    print(string_for_table_break(nil, mcEportCol));
    print("");
    print("SPBM service VLANs:");
    print(string_for_table_break(nil, serviceVidCol));
    print(string_for_table_title(nil, serviceVidCol));
    print(string_for_table_break(nil, serviceVidCol));
    for _sid, _info in pairs(spbm_internal_db.services.info_by_sid) do
        wrk_str = "";
        for _eport, _dummy in pairs(_info.mc_groups) do
            wrk_str = wrk_str .. tostring(_eport) .. " ";
        end
        line  = {_sid, _info.evid, _info.evidx, wrk_str};
        print(string_for_table_line(nil, serviceVidCol, line));
    end
    print(string_for_table_break(nil, serviceVidCol));
    print("");
end

CLI_addHelp("exec", "show spbm configurations", "show SPBM configurations")

CLI_addCommand("exec", "show spbm configurations", {
    func = spbm_show_config,
    help = "show SPBM configurations"
})

--[[
    debug 'show' commands to see the internal DB of the SPBM
--]]
CLI_addHelp("debug", "show spbm", "show SPBM related info")

CLI_addCommand("debug", "show spbm internal-db", {
    func = function(params)

        print("\n");
        print("start SPBM  internal-db");
        print(to_string(spbm_internal_db));
        print("end SPBM  internal-db");
        print("\n");

    end,
    help = "Dump internal DB of SPBM"
})


