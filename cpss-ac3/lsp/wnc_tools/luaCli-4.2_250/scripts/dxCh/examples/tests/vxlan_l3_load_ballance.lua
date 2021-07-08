--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vxlan_l3_load_ballance.lua
--*
--* DESCRIPTION:
--*       The test for vxlan routed ipv4 packets load ballancing.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5")

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixAdd")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixAddEcmpRange")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixDel")
cmdLuaCLI_registerCfunction("wrlPrvCpssDxChTrunkHashBitsSelectionSet")
cmdLuaCLI_registerCfunction("wrlPrvCpssDxChTrunkHashBitsSelectionGet")

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local port5   = devEnv.port[5]

---------------------------------------------------------------------------------
-- SCENARIO
-- VXLAN Tunnel Endpoint (VTEP)
-- The simulated system is 3 devices VTEP1,VTEP2 and VTEP3.
-- Host1 sends packets to VTEP1 via local network port
-- all packets contain DIP that identifies Host2
-- packets egressed from VTEP1 via two trunks (2 ports each)
-- incapsulated to reach VTEP2 or VTEP3 to continue their way to Host2
-- 8 packets will be sent different only by SIP Bits[5:3]
-- each of 4 ports (2 trunks * 2 members) should egress 2 packets
-- one with VTEP2 MAC_DA, one wits VTEP3 MAC_DA
-- The tested device simulates VTEP1 only.
--
-- VXLAN UDP Port is 4789(IANA official)
---------------------------------------------------------------------------------
-- configuration data
-- usable constant values
local payload_ip_protocol = 80; --unassigned by RFC790
local tunnel_UDP_Src_Port = 1;
local VXLAN_UDP_Port = 4789;

-- MACs, VIDs and IPs, ports
-- GLOBAL can be edited

function example_vxlan_l3_lb_param_reset_globals()
    example_vxlan_l3_lb_param_host1_ip = "172.116.1.1"
    example_vxlan_l3_lb_param_host2_ip = "172.116.2.1"
    example_vxlan_l3_lb_param_host1_mac = "00:00:00:11:11:11"
    example_vxlan_l3_lb_param_host2_mac = "00:00:00:22:22:22"
    example_vxlan_l3_lb_param_host1_vid = 18
    example_vxlan_l3_lb_param_host2_vid = 19
    example_vxlan_l3_lb_param_vtep1_ip = "10.1.1.1"
    example_vxlan_l3_lb_param_vtep2_ip = "10.1.1.2"
    example_vxlan_l3_lb_param_vtep3_ip = "10.1.1.3"
    example_vxlan_l3_lb_param_vtep1_mac = "00:00:00:AA:AA:AA"
    example_vxlan_l3_lb_param_vtep2_mac = "00:00:00:BB:BB:BB"
    example_vxlan_l3_lb_param_vtep3_mac = "00:00:00:CC:CC:CC"
    example_vxlan_l3_lb_param_vtep1_vtep2_vid = 0x901
    example_vxlan_l3_lb_param_vtep1_vtep3_vid = 0x903
    example_vxlan_l3_lb_param_vtep1_vtep2_service_id = 0x128D3 --75987
    example_vxlan_l3_lb_param_vtep1_vtep3_service_id = 0x1E38D --123789
    example_vxlan_l3_lb_param_host1_link_port = port1
    example_vxlan_l3_lb_param_trunk1_member1_port = port2
    example_vxlan_l3_lb_param_trunk1_member2_port = port3
    example_vxlan_l3_lb_param_trunk2_member1_port = port4
    example_vxlan_l3_lb_param_trunk2_member2_port = port5
end
if not example_vxlan_l3_lb_param_inited then
    example_vxlan_l3_lb_param_reset_globals()
    example_vxlan_l3_lb_param_inited = true
end
local user_cfg = {}
-- miscellaneous
-- MACs, VIDs and IPs, service_ids, ports
local function reinit_local_param()
    user_cfg.host1_ip  = example_vxlan_l3_lb_param_host1_ip
    user_cfg.host2_ip  = example_vxlan_l3_lb_param_host2_ip
    user_cfg.host1_mac = example_vxlan_l3_lb_param_host1_mac
    user_cfg.host2_mac = example_vxlan_l3_lb_param_host2_mac
    user_cfg.host1_vid = example_vxlan_l3_lb_param_host1_vid
    user_cfg.host2_vid = example_vxlan_l3_lb_param_host2_vid
    user_cfg.vtep1_ip  = example_vxlan_l3_lb_param_vtep1_ip
    user_cfg.vtep2_ip  = example_vxlan_l3_lb_param_vtep2_ip
    user_cfg.vtep3_ip  = example_vxlan_l3_lb_param_vtep3_ip
    user_cfg.vtep1_mac = example_vxlan_l3_lb_param_vtep1_mac
    user_cfg.vtep2_mac = example_vxlan_l3_lb_param_vtep2_mac
    user_cfg.vtep3_mac = example_vxlan_l3_lb_param_vtep3_mac
    user_cfg.vtep1_vtep2_vid = example_vxlan_l3_lb_param_vtep1_vtep2_vid
    user_cfg.vtep1_vtep3_vid = example_vxlan_l3_lb_param_vtep1_vtep3_vid
    user_cfg.vtep1_vtep2_service_id = example_vxlan_l3_lb_param_vtep1_vtep2_service_id
    user_cfg.vtep1_vtep3_service_id = example_vxlan_l3_lb_param_vtep1_vtep3_service_id
    user_cfg.host1_link_port     = example_vxlan_l3_lb_param_host1_link_port
    user_cfg.trunk1_member1_port = example_vxlan_l3_lb_param_trunk1_member1_port
    user_cfg.trunk1_member2_port = example_vxlan_l3_lb_param_trunk1_member2_port
    user_cfg.trunk2_member1_port = example_vxlan_l3_lb_param_trunk2_member1_port
    user_cfg.trunk2_member2_port = example_vxlan_l3_lb_param_trunk2_member2_port
end

reinit_local_param();

-- TCAM memory
local tti_rules_base = myGenWrapper("appDemoDxChTcamTtiBaseIndexGet", {
    { "IN", "GT_U8", "devNum", devNum},
    { "IN", "GT_U32", "hitNum", 0 }
})
printLog ("tti_rules_base: " .. tostring(tti_rules_base))

local prv_cfg = {
    common_vrf_id = 7,
    tunnel_hdr_ttl = 0x70,
    tti_rule1_index = (tti_rules_base + 12), -- at the same bank of floor
    pcl_rule1_index = 24,
    pcl_rule2_index = 27,
    pcl_rule3_index = 30,
    pcl_rule4_index = 33,
    pcl_cfg_table_index = 0xFFE, -- near bound of per-VLAN entries
    host1_vtep1_pcl_id = 0xAA,
    host2_arp_index = 5,
    vtep2_arp_index = 6,
    vtep3_arp_index = 7,
    trunk1_id = 11,
    trunk2_id = 12,
    ecmp_ip_entry_index = 8,
    vtep1_vtep2_next_hop_index = 10,
    vtep1_vtep3_next_hop_index = 11, -- must be  vtep1_vtep2_next_hop_index + 1
    vtep2_tunnel_index = 13,
    vtep3_tunnel_index = 14,
    vxlanTsProfileIndex = 0,
    saved_global_ecmp_enable,
    retrieved_ecmp_index_base_eport,
    saved_ecmp_index_base_eport,
    saved_cfg_global_eport_global,
    saved_cfg_global_eport_l2_ecmp,
    host1_eport,
    vtep1_vtep2_trunk1_eport,
    vtep1_vtep2_trunk2_eport,
    vtep1_vtep3_trunk1_eport,
    vtep1_vtep3_trunk2_eport,
    vtep1_vtep2_global_eport,
    vtep1_vtep3_global_eport,
    cfg_global_eport_global,
    cfg_global_eport_l2_ecmp,
    l2_ecmp_table_base_index = 2,
    saved_hash_bit_select_l3_ecmp_start_bit,
    saved_hash_bit_select_l3_ecmp_num_of_bits,
    saved_hash_bit_select_l2_ecmp_start_bit,
    saved_hash_bit_select_l2_ecmp_num_of_bits,
    saved_hash_bit_select_trunk_start_bit,
    saved_hash_bit_select_trunk_num_of_bits,
    hash_bit_select_l3_ecmp_start_bit   = 0,
    hash_bit_select_l3_ecmp_num_of_bits = 6,
    hash_bit_select_l2_ecmp_start_bit   = 0,
    hash_bit_select_l2_ecmp_num_of_bits = 5,
    hash_bit_select_trunk_start_bit     = 0,
    hash_bit_select_trunk_num_of_bits   = 6, -- max_trunk_members is 8 - swich member each time
    saved_vtep1_vtep2_trunk1_interface_info,
    saved_vtep1_vtep2_trunk2_interface_info,
    saved_vtep1_vtep3_trunk1_interface_info,
    saved_vtep1_vtep3_trunk2_interface_info,
    saved_vtep1_vtep2_trunk1_egress_port_info,
    saved_vtep1_vtep2_trunk2_egress_port_info,
    saved_vtep1_vtep3_trunk1_egress_port_info,
    saved_vtep1_vtep3_trunk2_egress_port_info,
}

local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(devNum)
if "CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily then
    prv_cfg.host1_eport              = 992
    prv_cfg.vtep1_vtep2_trunk1_eport = 993
    prv_cfg.vtep1_vtep2_trunk2_eport = 994
    prv_cfg.vtep1_vtep3_trunk1_eport = 995
    prv_cfg.vtep1_vtep3_trunk2_eport = 996
    prv_cfg.vtep1_vtep2_global_eport = 1009
    prv_cfg.vtep1_vtep3_global_eport = 1010
    prv_cfg.cfg_global_eport_global = {
        enable  = true,
        pattern = 0x3E0,
        mask    = 0x3F0
    }
    prv_cfg.cfg_global_eport_l2_ecmp = {
        enable  = true,
        pattern = 0x3F0,
        mask    = 0x3F0
    }
else
    prv_cfg.host1_eport              = 1000
    prv_cfg.vtep1_vtep2_trunk1_eport = 1001
    prv_cfg.vtep1_vtep2_trunk2_eport = 1002
    prv_cfg.vtep1_vtep3_trunk1_eport = 1003
    prv_cfg.vtep1_vtep3_trunk2_eport = 1004
    prv_cfg.vtep1_vtep2_global_eport = 2048
    prv_cfg.vtep1_vtep3_global_eport = 2049
    prv_cfg.cfg_global_eport_global = {
        enable  = true,
        pattern = 0x1C00,
        mask    = 0x1C00
    }
    prv_cfg.cfg_global_eport_l2_ecmp = {
        enable  = true,
        pattern = 0x0800,
        mask    = 0x0800
    }
end


local ret, val = myGenWrapper("cpssDxChCfgHwDevNumGet",{
    {"IN","GT_U8","devNum", devNum},
    {"OUT","GT_HW_DEV_NUM","hwDevNumPtr"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChCfgHwDevNumGet")
    setFailState()
end
local hw_device_id_number = val["hwDevNumPtr"];

local ret, val = myGenWrapper("cpssDxChTrunkHashGlobalModeGet",{
    {"IN","GT_U8","devNum", devNum},
    {"OUT","CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT","hashModePtr"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChTrunkHashGlobalModeGet")
    setFailState()
end
local trunk_hash_global_mode_enable_default = val["hashModePtr"];

local ret, val = myGenWrapper("cpssDxChTrunkHashIpModeGet",{
    {"IN","GT_U8","devNum", devNum},
    {"OUT","GT_BOOL","enablePtr"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChTrunkHashIpModeGet")
    setFailState()
end
local trunk_hash_ip_mode_enable_default = val["enablePtr"];

local ret, val = myGenWrapper("cpssDxChTrunkHashIpAddMacModeGet",{
    {"IN","GT_U8","devNum", devNum},
    {"OUT","GT_BOOL","enablePtr"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChTrunkHashIpAddMacModeGet")
    setFailState()
end
local trunk_hash_ip_add_mac_mode_enable_default = val["enablePtr"];

local ret, val = myGenWrapper("cpssDxChBrgL2EcmpEnableGet",{
    {"IN","GT_U8","devNum", devNum},
    {"OUT","GT_BOOL","enablePtr"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChBrgL2EcmpEnableGet")
    setFailState()
end
prv_cfg.saved_global_ecmp_enable = val["enablePtr"];

local ret, val = myGenWrapper("prvTgfBrgL2EcmpIndexBaseEportGet",{
    {"IN","GT_U8","devNum", devNum},
    {"OUT","GT_PORT_NUM","ecmpIndexBaseEportPtr"}
})
if (ret ~= 0) then
    printLog ("Error in prvTgfBrgL2EcmpIndexBaseEportGet")
    setFailState()
end

prv_cfg.saved_ecmp_index_base_eport = val["ecmpIndexBaseEportPtr"];
if "CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily then
    prv_cfg.retrieved_ecmp_index_base_eport = 0
else
    prv_cfg.retrieved_ecmp_index_base_eport = val["ecmpIndexBaseEportPtr"];
end

local ret, val = myGenWrapper("cpssDxChCfgGlobalEportGet",{
    {"IN","GT_U8","devNum", devNum},
    {"OUT","CPSS_DXCH_CFG_GLOBAL_EPORT_STC","globalPtr"},
    {"OUT","CPSS_DXCH_CFG_GLOBAL_EPORT_STC","l2EcmpPtr"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChCfgGlobalEportGet")
    setFailState()
end
prv_cfg.saved_cfg_global_eport_global  = val["globalPtr"];
prv_cfg.saved_cfg_global_eport_l2_ecmp = val["l2EcmpPtr"];

local ret
ret, prv_cfg.saved_hash_bit_select_l3_ecmp_start_bit,
    prv_cfg.saved_hash_bit_select_l3_ecmp_num_of_bits
    = wrlPrvCpssDxChTrunkHashBitsSelectionGet(
        devNum, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E")
if (ret ~= 0) then
    printLog ("Error in wrlPrvCpssDxChTrunkHashBitsSelectionGet")
    setFailState()
end
ret, prv_cfg.saved_hash_bit_select_l2_ecmp_start_bit,
    prv_cfg.saved_hash_bit_select_l2_ecmp_num_of_bits
    = wrlPrvCpssDxChTrunkHashBitsSelectionGet(
        devNum, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E")
if (ret ~= 0) then
    printLog ("Error in wrlPrvCpssDxChTrunkHashBitsSelectionGet")
    setFailState()
end
ret, prv_cfg.saved_hash_bit_select_trunk_start_bit,
    prv_cfg.saved_hash_bit_select_trunk_num_of_bits
    = wrlPrvCpssDxChTrunkHashBitsSelectionGet(
        devNum, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E")
if (ret ~= 0) then
    printLog ("Error in wrlPrvCpssDxChTrunkHashBitsSelectionGet")
    setFailState()
end

---------------------------------------------------------------------------------
-- VTEP2 configuration and de-configuration

local function mac_to_me(devNum, index, mac, vid, unset)
    local macToMeValue  = {};
    local macToMeMask   = {};

    macToMeMask.mac     = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    macToMeMask.vlanId  = 0xFFF;
    if unset ~= true then
        -- given mac and any VLAN
        macToMeValue.mac    = mac;
        macToMeValue.vlanId = vid;
    else
        -- never match
        macToMeValue.mac    = {0, 0, 0, 0, 0, 0};
        macToMeValue.vlanId = 0;
    end

    local macToMeSrcInterfaceInfo = {};
    macToMeSrcInterfaceInfo.includeSrcInterface = "CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E";

    local ret, val = myGenWrapper("cpssDxChTtiMacToMeSet",{
        { "IN",     "GT_U8",                                            "devNum",         devNum },
        { "IN",     "GT_U32",                                           "entryIndex",     macToMeEntryIndex},
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",                       "valuePtr",       macToMeValue},
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",                       "maskPtr",        macToMeMask},
        { "IN",     "CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC",   "interfaceInfoPtr",macToMeSrcInterfaceInfo},
    })
    if ret~=0 then
        printLog ("Error in cpssDxChTtiMacToMeSet")
        setFailState()
    end
end

local function tti_port_lookup_enable(devNum, port, keyType, unset)
    local enable = (not unset);
    local ret, val = myGenWrapper("cpssDxChTtiPortLookupEnableSet",{
        { "IN",     "GT_U8",                         "devNum",      devNum},
        { "IN",     "GT_PORT_NUM",                   "portNum",     port},
        { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",     keyType},
        { "IN",     "GT_BOOL",                       "enable",      enable}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChTtiPortLookupEnableSet")
        setFailState()
    end
end

local function ip_addr2swapped_number(ip)
    local s, e;
    local d_str = {};
    local num = {};
    local h_str = {};
    s, e, d_str[1], d_str[2], d_str[3], d_str[4] =
        string.find(ip, "(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)");
    num[1] = tonumber(d_str[1]);
    num[2] = tonumber(d_str[2]);
    num[3] = tonumber(d_str[3]);
    num[4] = tonumber(d_str[4]);
    return ((0x1000000 * num[4]) + (0x10000 * num[3]) + (0x100 * num[2]) + num[1]);
end

local function tti_rule(devNum, index, src_ip, dst_ip, pclConfigTableIndex, src_eport, unset)
    if unset then
        local ret, val = myGenWrapper("cpssDxChTtiRuleValidStatusSet",{
            { "IN",     "GT_U8",      "devNum",    devNum},
            { "IN",     "GT_U32",     "index",     index},
            { "IN",     "GT_BOOL",    "valid",     false}
        })
        if ret~=0 then
            printLog ("Error in cpssDxChTtiRuleValidStatusSet")
            setFailState()
        end
        return
    end
    local packetType, patternTable, maskTable, actionType, actionTable;
    packetType = "CPSS_DXCH_TTI_RULE_IPV4_E"
    actionType = "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"
    patternTable = {ipv4 = {
        srcIp  = bit_and(ip_addr2swapped_number(src_ip), 0x00FFFFFF),
        destIp = ip_addr2swapped_number(dst_ip)
    }}
    maskTable = {ipv4 = {
        srcIp  = 0x00FFFFFF,
        destIp = 0xFFFFFFFF
    }}
    actionTable = {}
    actionTable["type2"] = {
        tag1VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
        command = "CPSS_PACKET_CMD_FORWARD_E",
        enableDecrementTtl = true,
        pcl0OverrideConfigIndex = "CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E",
        iPclConfigIndex   = pclConfigTableIndex,
        sourceEPortAssignmentEnable = true,
        sourceEPort = src_eport
    }
    local ret,val = myGenWrapper("cpssDxChTtiRuleSet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U32","index",index},
                {"IN","CPSS_DXCH_TTI_RULE_TYPE_ENT","ruleType",packetType},
                {"IN","CPSS_DXCH_TTI_RULE_UNT_ipv4","patternPtr",patternTable},
                {"IN","CPSS_DXCH_TTI_RULE_UNT_ipv4","maskPtr",maskTable},
                {"IN","CPSS_DXCH_TTI_ACTION_TYPE_ENT","actionType",actionType},
                {"IN","CPSS_DXCH_TTI_ACTION_UNT_type2","actionPtr",actionTable}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChTtiRuleSet")
        setFailState()
    end
end

local function configure_tti(unset)
    local set_device_mac;
    if unset then
        set_device_mac =
        [[
        end
        configure
        no mac address
        ]]
    else
        set_device_mac =
        [[
            end
            configure
        ]] .. "\n" .. "mac address " .. user_cfg.vtep1_mac .. "\n"
    end
    executeStringCliCommands(set_device_mac);

    mac_to_me(devNum, 1--[[index--]], user_cfg.host2_mac, user_cfg.host1_vid, unset);
    tti_port_lookup_enable(devNum, user_cfg.host1_link_port, "CPSS_DXCH_TTI_KEY_IPV4_E", unset)
    tti_rule(
        devNum, prv_cfg.tti_rule1_index, user_cfg.host1_ip, user_cfg.host2_ip,
        prv_cfg.pcl_cfg_table_index, prv_cfg.host1_eport, unset)
end

local function pcl_rule(rule_index, pcl_id, vrf_id, unset)
    if unset then
        local ret, val = myGenWrapper("cpssDxChPclRuleInvalidate",{
            { "IN",  "GT_U8",                  "devNum",    devNum },
            { "IN",  "CPSS_PCL_RULE_SIZE_ENT", "ruleSize",  "CPSS_PCL_RULE_SIZE_STD_E" },
            { "IN",  "GT_U32",                 "ruleIndex", rule_index }
        })
        if ret~=0 then
            printLog ("Error in cpssDxChPclRuleInvalidate")
            setFailState()
        end
        return
    end

    local pclAction = {
        pktCmd       = "CPSS_PACKET_CMD_FORWARD_E",
        redirect = {
            redirectCmd = "CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E",
            data = {
                vrfId = vrf_id
            }
        }
    };
    local pclMask = {
        ruleStdIpv4L4 = {
            common = {pclId = 0x3FF}
        }
    };
    local pclPattern = {
        ruleStdIpv4L4 = {
            common = {pclId = pcl_id}
        }
    };
    local ret, val = myGenWrapper("cpssDxChPclRuleSet",{
        { "IN",     "GT_U8",                                        "devNum",            devNum },
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",           "ruleFormat",
            "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E"},
        { "IN",     "GT_U32",                                        "ruleIndex",        rule_index },
        { "IN",     "CPSS_DXCH_PCL_RULE_OPTION_ENT",                 "ruleOptionsBmp",   0},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleStdIpv4L4",   "maskPtr",          pclMask},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleStdIpv4L4",   "patternPtr",       pclPattern},
        { "IN",     "CPSS_DXCH_PCL_ACTION_STC",                      "actionPtr",        pclAction},
    })
    if ret~=0 then
        printLog ("Error in cpssDxChPclRuleSet")
        setFailState()
    end
end

local function configure_pcl(unset)
    local enable = (not unset);
    local ret, val = myGenWrapper("cpssDxCh3PclTunnelTermForceVlanModeEnableSet",{
        { "IN",     "GT_U8",       "devNum",    devNum},
        { "IN",     "GT_BOOL",     "enable",    unset}
    })
    if ret~=0 then
        printLog ("Error in cpssDxCh3PclTunnelTermForceVlanModeEnableSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChPclPortIngressPolicyEnable",{
        { "IN",     "GT_U8",       "devNum",    devNum},
        { "IN",     "GT_PORT_NUM", "portNum",   prv_cfg.host1_eport},
        { "IN",     "GT_BOOL",     "enable",    enable}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChPclPortIngressPolicyEnable")
        setFailState()
    end
    local interfaceInfo = {
        type = "CPSS_INTERFACE_INDEX_E",
        index = prv_cfg.pcl_cfg_table_index
    };
    local lookupCfgPtr = {
        enableLookup = enable,
        pclId = prv_cfg.host1_vtep1_pcl_id,
        groupKeyTypes = {
            nonIpKey = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E",
            ipv4Key  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E",
            ipv6Key  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E"
        }
    };
    ret,val = myGenWrapper("cpssDxChPclCfgTblSet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","CPSS_INTERFACE_INFO_STC","interfaceInfoPtr",interfaceInfo},
        {"IN","CPSS_PCL_DIRECTION_ENT","direction","CPSS_PCL_DIRECTION_INGRESS_E"},
        {"IN","CPSS_PCL_LOOKUP_NUMBER_ENT","lookupNum","CPSS_PCL_LOOKUP_NUMBER_0_E"},
        {"IN","CPSS_DXCH_PCL_LOOKUP_CFG_STC","lookupCfgPtr",lookupCfgPtr}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChPclCfgTblSet")
        setFailState()
    end
    pcl_rule(
        prv_cfg.pcl_rule1_index, prv_cfg.host1_vtep1_pcl_id,
        prv_cfg.common_vrf_id, unset);
end

local function configure_fdb_routed_entry(unset, mac, vlan)
    local mac_entry = {
        key = {
            entryType = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
            key = {
                macVlan = {
                    macAddr = mac,
                    vlanId = vlan
                }
            }
        },
        dstInterface = {
            type = "CPSS_INTERFACE_PORT_E",
            devPort = {
                hwDevNum = hw_device_id_number,
                portNum = 62 -- null-port, will be overriden
            }
        },
        isStatic = true,
        daRoute  = true,
        daCommand = "CPSS_MAC_TABLE_FRWRD_E"
    }
    if not unset then
        local ret, val = mac_entry_set(devNum, mac_entry);
        if ret~=0 then
            printLog ("Error in mac_entry_set")
            setFailState()
        end
    else
        local ret, val = mac_entry_delete(devNum, mac_entry);
        if ret~=0 then
            printLog ("Error in mac_entry_delete")
            setFailState()
        end
    end
end

local function configure_fdb(unset)
    -- destination mac of HOST2, but VIS yet from HOST1
    configure_fdb_routed_entry(unset, user_cfg.host2_mac, user_cfg.host1_vid);
end

local function configure_vlan(unset)
    if unset then
        local ret = cpssGenWrapper("cpssDxChBrgVlanEntryInvalidate",{
          { "IN", "GT_U8", "devNum", devNum },
          { "IN", "GT_U16","vlanId", user_cfg.host1_vid }
        })
        if ret~=0 then
            printLog ("Error in cpssDxChBrgVlanEntryInvalidate")
            setFailState()
        end
        local ret = cpssGenWrapper("cpssDxChBrgVlanEntryInvalidate",{
          { "IN", "GT_U8", "devNum", devNum },
          { "IN", "GT_U16","vlanId", user_cfg.host2_vid }
        })
        if ret~=0 then
            printLog ("Error in cpssDxChBrgVlanEntryInvalidate")
            setFailState()
        end
        local ret = cpssGenWrapper("cpssDxChBrgVlanEntryInvalidate",{
          { "IN", "GT_U8", "devNum", devNum },
          { "IN", "GT_U16","vlanId", user_cfg.vtep1_vtep2_vid }
        })
        if ret~=0 then
            printLog ("Error in cpssDxChBrgVlanEntryInvalidate")
            setFailState()
        end
        local ret = cpssGenWrapper("cpssDxChBrgVlanEntryInvalidate",{
          { "IN", "GT_U8", "devNum", devNum },
          { "IN", "GT_U16","vlanId", user_cfg.vtep1_vtep3_vid }
        })
        if ret~=0 then
            printLog ("Error in cpssDxChBrgVlanEntryInvalidate")
            setFailState()
        end
        return
    end

    local ret, values
    ---- VLANs with ports
    ret,values = cpssGenWrapper("cpssDxChBrgVlanEntryRead",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", user_cfg.host1_vid },
      { "OUT", "CPSS_PORTS_BMP_STC", "portsMembers"},
      { "OUT", "CPSS_PORTS_BMP_STC", "portsTagging"},
      { "OUT", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo" },
      { "OUT", "GT_BOOL", "isValid" },
      { "OUT", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd" }
    })
    if ret~=0 then
        printLog ("Error in cpssDxChBrgVlanEntryRead")
        setFailState()
    end
    values.vlanInfo.fidValue         = user_cfg.host1_vid
    values.vlanInfo.ipv4UcastRouteEn = true
    values.vlanInfo.ipv6UcastRouteEn = true
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.host1_link_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.host1_link_port)
    if not values.portsTaggingCmd then values.portsTaggingCmd = {} end
    if not values.portsTaggingCmd.portsCmd then values.portsTaggingCmd.portsCmd = {} end
    values.portsTaggingCmd.portsCmd[user_cfg.host1_link_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    local ret=cpssGenWrapper("cpssDxChBrgVlanEntryWrite",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", user_cfg.host1_vid },
      { "IN", "CPSS_PORTS_BMP_STC", "portsMembers", values.portsMembers },
      { "IN", "CPSS_PORTS_BMP_STC", "portsTagging", values.portsTagging },
      { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo", values.vlanInfo},
      { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd", values.portsTaggingCmd}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChBrgVlanEntryWrite")
        setFailState()
    end
    ret,values = cpssGenWrapper("cpssDxChBrgVlanEntryRead",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", user_cfg.host2_vid },
      { "OUT", "CPSS_PORTS_BMP_STC", "portsMembers"},
      { "OUT", "CPSS_PORTS_BMP_STC", "portsTagging"},
      { "OUT", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo" },
      { "OUT", "GT_BOOL", "isValid" },
      { "OUT", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd" }
    })
    if ret~=0 then
        printLog ("Error in cpssDxChBrgVlanEntryRead")
        setFailState()
    end
    values.vlanInfo.fidValue         = user_cfg.host2_vid
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk1_member1_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk1_member1_port)
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk1_member2_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk1_member2_port)
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk2_member1_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk2_member1_port)
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk2_member2_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk2_member2_port)
    if not values.portsTaggingCmd then values.portsTaggingCmd = {} end
    if not values.portsTaggingCmd.portsCmd then values.portsTaggingCmd.portsCmd = {} end
    values.portsTaggingCmd.portsCmd[user_cfg.trunk1_member1_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    values.portsTaggingCmd.portsCmd[user_cfg.trunk1_member2_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    values.portsTaggingCmd.portsCmd[user_cfg.trunk2_member1_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    values.portsTaggingCmd.portsCmd[user_cfg.trunk2_member2_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    local ret=cpssGenWrapper("cpssDxChBrgVlanEntryWrite",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", user_cfg.host2_vid },
      { "IN", "CPSS_PORTS_BMP_STC", "portsMembers", values.portsMembers },
      { "IN", "CPSS_PORTS_BMP_STC", "portsTagging", values.portsTagging },
      { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo", values.vlanInfo},
      { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd", values.portsTaggingCmd}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChBrgVlanEntryWrite")
        setFailState()
    end

    ret,values = cpssGenWrapper("cpssDxChBrgVlanEntryRead",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", user_cfg.vtep1_vtep2_vid },
      { "OUT", "CPSS_PORTS_BMP_STC", "portsMembers"},
      { "OUT", "CPSS_PORTS_BMP_STC", "portsTagging"},
      { "OUT", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo" },
      { "OUT", "GT_BOOL", "isValid" },
      { "OUT", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd" }
    })
    if ret~=0 then
        printLog ("Error in cpssDxChBrgVlanEntryRead")
        setFailState()
    end
    values.vlanInfo.fidValue         = user_cfg.vtep1_vtep2_vid
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk1_member1_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk1_member1_port)
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk1_member2_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk1_member2_port)
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk2_member1_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk2_member1_port)
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk2_member2_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk2_member2_port)
    if not values.portsTaggingCmd then values.portsTaggingCmd = {} end
    if not values.portsTaggingCmd.portsCmd then values.portsTaggingCmd.portsCmd = {} end
    values.portsTaggingCmd.portsCmd[user_cfg.trunk1_member1_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    values.portsTaggingCmd.portsCmd[user_cfg.trunk1_member2_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    values.portsTaggingCmd.portsCmd[user_cfg.trunk2_member1_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    values.portsTaggingCmd.portsCmd[user_cfg.trunk2_member2_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    ret=cpssGenWrapper("cpssDxChBrgVlanEntryWrite",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", user_cfg.vtep1_vtep2_vid },
      { "IN", "CPSS_PORTS_BMP_STC", "portsMembers", values.portsMembers },
      { "IN", "CPSS_PORTS_BMP_STC", "portsTagging", values.portsTagging },
      { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo", values.vlanInfo},
      { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd", values.portsTaggingCmd}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChBrgVlanEntryWrite")
        setFailState()
    end

    ret,values = cpssGenWrapper("cpssDxChBrgVlanEntryRead",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", user_cfg.vtep1_vtep3_vid },
      { "OUT", "CPSS_PORTS_BMP_STC", "portsMembers"},
      { "OUT", "CPSS_PORTS_BMP_STC", "portsTagging"},
      { "OUT", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo" },
      { "OUT", "GT_BOOL", "isValid" },
      { "OUT", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd" }
    })
    if ret~=0 then
        printLog ("Error in cpssDxChBrgVlanEntryRead")
        setFailState()
    end
    values.vlanInfo.fidValue         = user_cfg.vtep1_vtep3_vid
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk1_member1_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk1_member1_port)
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk1_member2_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk1_member2_port)
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk2_member1_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk2_member1_port)
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, user_cfg.trunk2_member2_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, user_cfg.trunk2_member2_port)
    if not values.portsTaggingCmd then values.portsTaggingCmd = {} end
    if not values.portsTaggingCmd.portsCmd then values.portsTaggingCmd.portsCmd = {} end
    values.portsTaggingCmd.portsCmd[user_cfg.trunk1_member1_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    values.portsTaggingCmd.portsCmd[user_cfg.trunk1_member2_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    values.portsTaggingCmd.portsCmd[user_cfg.trunk2_member1_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    values.portsTaggingCmd.portsCmd[user_cfg.trunk2_member2_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";

    local ret=cpssGenWrapper("cpssDxChBrgVlanEntryWrite",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", user_cfg.vtep1_vtep3_vid },
      { "IN", "CPSS_PORTS_BMP_STC", "portsMembers", values.portsMembers },
      { "IN", "CPSS_PORTS_BMP_STC", "portsTagging", values.portsTagging },
      { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo", values.vlanInfo},
      { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd", values.portsTaggingCmd}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChBrgVlanEntryWrite")
        setFailState()
    end
end

local function configure_router(unset)
    local enable = (not unset);

    local ret = cpssGenWrapper("cpssDxChIpPortRoutingEnable",{
        { "IN", "GT_U8",                         "devNum",         devNum                     },
        { "IN", "GT_PORT_NUM",                   "portNum",        user_cfg.host1_link_port   },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"         },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4V6_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  enable                     }
    })
    if ret~=0 then
        printLog ("Error in cpssDxChIpPortRoutingEnable")
        setFailState()
    end
    local ret = cpssGenWrapper("cpssDxChIpPortRoutingEnable",{
        { "IN", "GT_U8",                         "devNum",         devNum                     },
        { "IN", "GT_PORT_NUM",                   "portNum",        prv_cfg.host1_eport        },
        { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    "CPSS_IP_UNICAST_E"         },
        { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4V6_E" },
        { "IN", "GT_BOOL",                       "enableRouting",  enable                     }
    })
    if ret~=0 then
        printLog ("Error in cpssDxChIpPortRoutingEnable")
        setFailState()
    end
    local ret = cpssGenWrapper("cpssDxChIpSpecialRouterTriggerEnable",{
        { "IN", "GT_U8",                         "devNum",              devNum  },
        { "IN", "CPSS_DXCH_IP_BRG_EXCP_CMD_ENT", "bridgeExceptionCmd",
            "CPSS_DXCH_IP_BRG_UC_IPV4_SOFT_DROP_EXCP_CMD_E" },
        { "IN", "GT_BOOL",                       "enableRouterTrigger",  enable }
    })
    if ret~=0 then
        printLog ("Error in cpssDxChIpSpecialRouterTriggerEnable")
        setFailState()
    end

    if unset then
        -- restoring ARP table entries not needed
        -- restoring Next Hop table entries not needed
        -- restoring Tunnel start table entries not needed
        local ret = myGenWrapper("cpssDxChIpLpmVirtualRouterDel",{
        {"IN", "GT_U32", "lpmDbId", 0},
            {"IN", "GT_U32", "vrId", prv_cfg.common_vrf_id}
        })
        if ret~=0 then
            printLog ("Error in cpssDxChIpLpmVirtualRouterDel")
            setFailState()
        end
        return
    end

    -- default route entries info. Are used when Virtual Router creating.
    local defaultRoutesInfo = {
        -- CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
        ipv4Uc = {
          lttEntry = {
             routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
             routeEntryBaseIndex = 0,
             ucRPFCheckEnable = false,
             sipSaCheckMismatchEnable = false,
             ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E"
        }},
        -- CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
        ipv6Uc = {
          lttEntry = {
             routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
             numOfPaths = 0,
             routeEntryBaseIndex = 0,
             ucRPFCheckEnable = false,
             sipSaCheckMismatchEnable = false,
             ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E"
        }}
    }
    local vrConfig = {
       supportIpv4Uc = true,
       defIpv4UcNextHopInfo   = defaultRoutesInfo.ipv4Uc,
       supportIpv6Uc = true,
       defIpv6UcNextHopInfo   = defaultRoutesInfo.ipv6Uc,
    }
    local ret = myGenWrapper("cpssDxChIpLpmVirtualRouterAdd", {
        {"IN", "GT_U32",                         "lpmDbId",     0},
        {"IN", "GT_U32",                         "vrId",        prv_cfg.common_vrf_id},
        {"IN", "CPSS_DXCH_IP_LPM_VR_CONFIG_STC", "vrConfigPtr", vrConfig}
    });
    if ((ret ~= 0) and (ret ~= 0x1B --[[GT_ALREADY_EXIST--]]))then
        printLog ("Error in cpssDxChIpLpmVirtualRouterAdd")
        setFailState()
    end

    local ret = myGenWrapper("cpssDxChIpRouterArpAddrWrite",{
        { "IN",   "GT_U8",        "devNum",         devNum},
        { "IN",   "GT_U32",       "routerArpIndex", prv_cfg.host2_arp_index},
        { "IN",   "GT_ETHERADDR", "arpMacAddr",     user_cfg.host2_mac }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChIpRouterArpAddrWrite")
        setFailState()
    end
    local ret = myGenWrapper("cpssDxChIpRouterArpAddrWrite",{
        { "IN",   "GT_U8",        "devNum",         devNum},
        { "IN",   "GT_U32",       "routerArpIndex", prv_cfg.vtep2_arp_index},
        { "IN",   "GT_ETHERADDR", "arpMacAddr",     user_cfg.vtep2_mac }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChIpRouterArpAddrWrite")
        setFailState()
    end
    local ret = myGenWrapper("cpssDxChIpRouterArpAddrWrite",{
        { "IN",   "GT_U8",        "devNum",         devNum},
        { "IN",   "GT_U32",       "routerArpIndex", prv_cfg.vtep3_arp_index},
        { "IN",   "GT_ETHERADDR", "arpMacAddr",     user_cfg.vtep3_mac }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChIpRouterArpAddrWrite")
        setFailState()
    end

    local routeEntriesTemplate = {
        type = "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E",
        entry = {
            regularEntry = {
                cmd = "CPSS_PACKET_CMD_ROUTE_E",
                cpuCodeIdx = "CPSS_DXCH_IP_CPU_CODE_IDX_0_E",
                qosPrecedence = "CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E",
                modifyUp = "CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E",
                modifyDscp = "CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E",
                countSet = "CPSS_IP_CNT_SET0_E",
                siteId = "CPSS_IP_SITE_ID_INTERNAL_E",
                ttlHopLimitDecEnable = true,
                nextHopInterface = {
                    type = "CPSS_INTERFACE_PORT_E",
                    devPort = {devNum = hw_device_id_number}
                }
            }
        }
    }

    local routeEntries;

    routeEntries = table_deep_copy(routeEntriesTemplate);
    routeEntries.entry.regularEntry.nextHopInterface.devPort.portNum = prv_cfg.vtep1_vtep2_global_eport;
    routeEntries.entry.regularEntry.nextHopVlanId = user_cfg.host2_vid;
    routeEntries.entry.regularEntry.nextHopARPPointer = prv_cfg.host2_arp_index;
    local ret = myGenWrapper("cpssDxChIpUcRouteEntriesWrite", {
        { "IN",     "GT_U8",                            "devNum",               devNum },
        { "IN",     "GT_U32",                           "baseRouteEntryIndex",
            prv_cfg.vtep1_vtep2_next_hop_index},
        { "IN",     "CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC",  "routeEntriesArray",    routeEntries },
        { "IN",     "GT_U32",                           "numOfRouteEntries",    1}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChIpUcRouteEntriesWrite")
        setFailState()
    end
    routeEntries = table_deep_copy(routeEntriesTemplate);
    routeEntries.entry.regularEntry.nextHopInterface.devPort.portNum = prv_cfg.vtep1_vtep3_global_eport;
    routeEntries.entry.regularEntry.nextHopVlanId = user_cfg.host2_vid;
    routeEntries.entry.regularEntry.nextHopARPPointer = prv_cfg.host2_arp_index;
    local ret = myGenWrapper("cpssDxChIpUcRouteEntriesWrite", {
        { "IN",     "GT_U8",                            "devNum",               devNum },
        { "IN",     "GT_U32",                           "baseRouteEntryIndex",
            prv_cfg.vtep1_vtep3_next_hop_index},
        { "IN",     "CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC",  "routeEntriesArray",    routeEntries },
        { "IN",     "GT_U32",                           "numOfRouteEntries",    1}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChIpUcRouteEntriesWrite")
        setFailState()
    end

    local ret, val = wrLogWrapper(
        "wrlCpssDxChIpLpmIpUcPrefixAddEcmpRange",
        ("(vrf_id, ip_protocol, ip_address, mask_length, "
        .. "ecmp_ip_entry_index, next_hop_enrty_index, numOfPaths, randomEnable)"),
        prv_cfg.common_vrf_id, "CPSS_IP_PROTOCOL_IPV4_E", user_cfg.host2_ip, 32,
        prv_cfg.ecmp_ip_entry_index, prv_cfg.vtep1_vtep2_next_hop_index, 2, false)
    if (ret ~= 0) then
        printLog ("Error in wrlCpssDxChIpLpmIpUcPrefixAddEcmpRange")
        setFailState()
    end
end

local function configure_trunk(unset)
    local enable = (not unset)
    local trunk_hash_global_mode     = trunk_hash_global_mode_enable_default
    local trunk_hash_ip_mode         = trunk_hash_ip_mode_enable_default
    local trunk_hash_ip_add_mac_mode = trunk_hash_ip_add_mac_mode_enable_default
    if not unset then
        trunk_hash_global_mode     = "CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E"
        trunk_hash_ip_mode         = true
        trunk_hash_ip_add_mac_mode = false
    end

    -- init trunks to be empty, LUA does not support members array parsing
    local ret = myGenWrapper("cpssDxChTrunkMembersSet", {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_TRUNK_ID", "trunkId", prv_cfg.trunk1_id},
        { "IN", "GT_U32", "numOfEnabledMembers", 0 },
        { "IN", "CPSS_TRUNK_MEMBER_STC", "enabledMembersArray", nil },
        { "IN", "GT_U32", "numOfDisabledMembers", 0 },
        { "IN", "CPSS_TRUNK_MEMBER_STC", "disabledMembersArray", nil }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTrunkMembersSet")
        setFailState()
    end
    local ret = myGenWrapper("cpssDxChTrunkMembersSet", {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_TRUNK_ID", "trunkId", prv_cfg.trunk2_id},
        { "IN", "GT_U32", "numOfEnabledMembers", 0 },
        { "IN", "CPSS_TRUNK_MEMBER_STC", "enabledMembersArray", nil },
        { "IN", "GT_U32", "numOfDisabledMembers", 0 },
        { "IN", "CPSS_TRUNK_MEMBER_STC", "disabledMembersArray", nil }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTrunkMembersSet")
        setFailState()
    end

    -- add trunk members
    if not unset then
        local id, arr, ndx, data
        local members = {
            [prv_cfg.trunk1_id] = {
                {device = hw_device_id_number, port = user_cfg.trunk1_member1_port},
                {device = hw_device_id_number, port = user_cfg.trunk1_member2_port}
            },
            [prv_cfg.trunk2_id] = {
                {device = hw_device_id_number, port = user_cfg.trunk2_member1_port},
                {device = hw_device_id_number, port = user_cfg.trunk2_member2_port}
            }
        }
        for id, arr in pairs(members) do
            for ndx, data in pairs(arr) do
                local ret = myGenWrapper("cpssDxChTrunkMemberAdd", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_TRUNK_ID", "trunkId", id},
                    { "IN", "CPSS_TRUNK_MEMBER_STC", "memberPtr", data }
                })
                if (ret ~= 0) then
                    printLog ("Error in cpssDxChTrunkMemberAdd")
                    setFailState()
                end
            end
        end
    end

    local ret = myGenWrapper("cpssDxChTrunkHashGlobalModeSet", {
        { "IN", "GT_U8",                               "devNum", devNum},
        { "IN", "CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT", "enable", trunk_hash_global_mode },
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTrunkHashGlobalModeSet")
        setFailState()
    end
    local ret = myGenWrapper("cpssDxChTrunkHashIpModeSet", {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_BOOL", "enable", trunk_hash_ip_mode },
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTrunkHashIpModeSet")
        setFailState()
    end
    local ret = myGenWrapper("cpssDxChTrunkHashIpAddMacModeSet", {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_BOOL", "enable", trunk_hash_ip_add_mac_mode },
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTrunkHashIpAddMacModeSet")
        setFailState()
    end
end

local function getVxLanProfileData()
    local profileDataPtr = {}
    local sidBaseBit;

    profileDataPtr.templateDataBitsCfg = {}
    local indexOfTunnelStartTemplateData
    indexOfTunnelStartTemplateData = 3;
    sidBaseBit = 32;
    profileDataPtr.templateDataSize = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E"
    profileDataPtr.templateDataBitsCfg[indexOfTunnelStartTemplateData] =
        "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E"

    for i = sidBaseBit, (sidBaseBit + 7) do -- set bits 32 - 39 (VXLAN) or bits 0 - 7 (NVGRE)
        profileDataPtr.templateDataBitsCfg[i] = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_2_E";
    end
    for i = (sidBaseBit + 8), (sidBaseBit + 15) do -- set bits 40 - 47 (VXLAN) or bits 8 - 15 (NVGRE)
        profileDataPtr.templateDataBitsCfg[i] = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_1_E";
    end
    for i = (sidBaseBit + 16), (sidBaseBit + 23) do -- set bits 48 - 55 (VXLAN) or bits 16 - 23 (NVGRE)
        profileDataPtr.templateDataBitsCfg[i] = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_0_E";
    end

    profileDataPtr.udpSrcPortMode = "CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E";
    return profileDataPtr
end

local function vxlanTsProfileInit(devNum, profileIndex)
    local ret, val
    local profileDataPtr = getVxLanProfileData()

    ret, val = myGenWrapper("cpssDxChTunnelStartGenProfileTableEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "profileIndex", profileIndex },
        { "IN", "CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC", "profileDataPtr", profileDataPtr }
    })

    if (ret ~= 0) then
        printLog("Error while executing cpssDxChTunnelStartGenProfileTableEntrySet")
    end
end

local function configure_ecmp(unset)
    local enable;
    local cfg_eport_global, cfg_eport_l2_ecmp
    local l2_ecmp_ltt_entry_vtep2, l2_ecmp_ltt_entry_vtep3;
    local l2_ecmp_entry_vtep2_trunk1, l2_ecmp_entry_vtep2_trunk2;
    local l2_ecmp_entry_vtep3_trunk1, l2_ecmp_entry_vtep3_trunk2;
    local bit_select_l3_ecmp_start_bit;
    local bit_select_l3_ecmp_num_of_bits;
    local bit_select_l2_ecmp_start_bit;
    local bit_select_l2_ecmp_num_of_bits;
    local bit_select_trunk_start_bit;
    local bit_select_trunk_num_of_bits;

    enable = true;
    cfg_eport_global  = prv_cfg.cfg_global_eport_global
    cfg_eport_l2_ecmp = prv_cfg.cfg_global_eport_l2_ecmp
    l2_ecmp_ltt_entry_vtep2 = {
        ecmpStartIndex = prv_cfg.l2_ecmp_table_base_index,
        ecmpNumOfPaths = 2,
        ecmpEnable     = true,
        ecmpRandomPathEnable = false
    }
    l2_ecmp_ltt_entry_vtep3 = {
        ecmpStartIndex = (prv_cfg.l2_ecmp_table_base_index + 2),
        ecmpNumOfPaths = 2,
        ecmpEnable     = true,
        ecmpRandomPathEnable = false
    }
    l2_ecmp_entry_vtep2_trunk1 = {
        targetEport = prv_cfg.vtep1_vtep2_trunk1_eport,
        targetHwDevice = hw_device_id_number
    }
    l2_ecmp_entry_vtep2_trunk2 = {
        targetEport = prv_cfg.vtep1_vtep2_trunk2_eport,
        targetHwDevice = hw_device_id_number
    }
    l2_ecmp_entry_vtep3_trunk1 = {
        targetEport = prv_cfg.vtep1_vtep3_trunk1_eport,
        targetHwDevice = hw_device_id_number
    }
    l2_ecmp_entry_vtep3_trunk2 = {
        targetEport = prv_cfg.vtep1_vtep3_trunk2_eport,
        targetHwDevice = hw_device_id_number
    }
    bit_select_l3_ecmp_start_bit    = prv_cfg.hash_bit_select_l3_ecmp_start_bit
    bit_select_l3_ecmp_num_of_bits  = prv_cfg.hash_bit_select_l3_ecmp_num_of_bits
    bit_select_l2_ecmp_start_bit    = prv_cfg.hash_bit_select_l2_ecmp_start_bit
    bit_select_l2_ecmp_num_of_bits  = prv_cfg.hash_bit_select_l2_ecmp_num_of_bits
    bit_select_trunk_start_bit      = prv_cfg.hash_bit_select_trunk_start_bit
    bit_select_trunk_num_of_bits    = prv_cfg.hash_bit_select_trunk_num_of_bits
    if unset then
        enable = prv_cfg.saved_global_ecmp_enable;
        cfg_eport_global  = prv_cfg.saved_cfg_global_eport_global
        cfg_eport_l2_ecmp = prv_cfg.saved_cfg_global_eport_l2_ecmp
        l2_ecmp_ltt_entry_vtep2 = {ecmpNumOfPaths = 1}
        l2_ecmp_ltt_entry_vtep3 = {ecmpNumOfPaths = 1}
        l2_ecmp_entry_vtep2_trunk1 = {}
        l2_ecmp_entry_vtep2_trunk2 = {}
        l2_ecmp_entry_vtep3_trunk1 = {}
        l2_ecmp_entry_vtep3_trunk2 = {}
        bit_select_l3_ecmp_start_bit    = prv_cfg.saved_hash_bit_select_l3_ecmp_start_bit
        bit_select_l3_ecmp_num_of_bits  = prv_cfg.saved_hash_bit_select_l3_ecmp_num_of_bits
        bit_select_l2_ecmp_start_bit    = prv_cfg.saved_hash_bit_select_l2_ecmp_start_bit
        bit_select_l2_ecmp_num_of_bits  = prv_cfg.saved_hash_bit_select_l2_ecmp_num_of_bits
        bit_select_trunk_start_bit      = prv_cfg.saved_hash_bit_select_trunk_start_bit
        bit_select_trunk_num_of_bits    = prv_cfg.saved_hash_bit_select_trunk_num_of_bits
    end
    local ret, val = myGenWrapper("cpssDxChBrgL2EcmpEnableSet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","GT_BOOL","enable", enable}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgL2EcmpEnableSet")
        setFailState()
    end

    local ret
    ret = wrlPrvCpssDxChTrunkHashBitsSelectionSet(
            devNum, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E",
            bit_select_l3_ecmp_start_bit, bit_select_l3_ecmp_num_of_bits)
    if (ret ~= 0) then
        printLog ("Error in wrlPrvCpssDxChTrunkHashBitsSelectionSet")
        setFailState()
    end
    ret = wrlPrvCpssDxChTrunkHashBitsSelectionSet(
            devNum, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E",
            bit_select_l2_ecmp_start_bit, bit_select_l2_ecmp_num_of_bits)
    if (ret ~= 0) then
        printLog ("Error in wrlPrvCpssDxChTrunkHashBitsSelectionSet")
        setFailState()
    end
    ret = wrlPrvCpssDxChTrunkHashBitsSelectionSet(
            devNum, "CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E",
            bit_select_trunk_start_bit, bit_select_trunk_num_of_bits)
    if (ret ~= 0) then
        printLog ("Error in wrlPrvCpssDxChTrunkHashBitsSelectionSet")
        setFailState()
    end

    if unset then
        local ret, val = myGenWrapper("prvTgfBrgL2EcmpIndexBaseEportSet",{
            {"IN","GT_U8","devNum", devNum},
            {"IN","GT_PORT_NUM",prv_cfg.saved_ecmp_index_base_eport}
        })
        if (ret ~= 0) then
            printLog ("Error in prvTgfBrgL2EcmpIndexBaseEportSet")
            setFailState()
        end
    else
        local ret, val = myGenWrapper("prvTgfBrgL2EcmpIndexBaseEportSet",{
            {"IN","GT_U8","devNum", devNum},
            {"IN","GT_PORT_NUM", prv_cfg.retrieved_ecmp_index_base_eport}
        })
        if (ret ~= 0) then
            printLog ("Error in prvTgfBrgL2EcmpIndexBaseEportSet")
            setFailState()
        end
    end

    local ret, val = myGenWrapper("cpssDxChCfgGlobalEportSet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","CPSS_DXCH_CFG_GLOBAL_EPORT_STC","globalPtr", cfg_eport_global},
        {"IN","CPSS_DXCH_CFG_GLOBAL_EPORT_STC","l2EcmpPtr", cfg_eport_l2_ecmp}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChCfgGlobalEportSet")
        setFailState()
    end

    local ret, val = myGenWrapper("cpssDxChBrgL2EcmpLttTableSet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","GT_U32","index",
            (prv_cfg.vtep1_vtep2_global_eport - prv_cfg.retrieved_ecmp_index_base_eport)},
        {"IN","CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC","ecmpLttInfoPtr", l2_ecmp_ltt_entry_vtep2}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgL2EcmpLttTableSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChBrgL2EcmpLttTableSet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","GT_U32","index",
            (prv_cfg.vtep1_vtep3_global_eport - prv_cfg.retrieved_ecmp_index_base_eport)},
        {"IN","CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC","ecmpLttInfoPtr", l2_ecmp_ltt_entry_vtep3}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgL2EcmpLttTableSet")
        setFailState()
    end

    local ret, val = myGenWrapper("cpssDxChBrgL2EcmpTableSet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","GT_U32","index", prv_cfg.l2_ecmp_table_base_index},
        {"IN","CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC","ecmpEntryPtr", l2_ecmp_entry_vtep2_trunk1}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgL2EcmpTableSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChBrgL2EcmpTableSet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","GT_U32","index", (prv_cfg.l2_ecmp_table_base_index + 1)},
        {"IN","CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC","ecmpEntryPtr", l2_ecmp_entry_vtep2_trunk2}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgL2EcmpTableSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChBrgL2EcmpTableSet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","GT_U32","index", (prv_cfg.l2_ecmp_table_base_index + 2)},
        {"IN","CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC","ecmpEntryPtr", l2_ecmp_entry_vtep3_trunk1}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgL2EcmpTableSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChBrgL2EcmpTableSet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","GT_U32","index", (prv_cfg.l2_ecmp_table_base_index + 3)},
        {"IN","CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC","ecmpEntryPtr", l2_ecmp_entry_vtep3_trunk2}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgL2EcmpTableSet")
        setFailState()
    end
end

local function configure_target_eport(unset)
    local enable = (not unset);

    if enable then
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk1_eport },
            { "OUT", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr" }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableGet")
            setFailState()
        end
        prv_cfg.saved_vtep1_vtep2_trunk1_interface_info =
            val["physicalInfoPtr"]
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk2_eport },
            { "OUT", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr" }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableGet")
            setFailState()
        end
        prv_cfg.saved_vtep1_vtep2_trunk2_interface_info =
            val["physicalInfoPtr"]
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk1_eport },
            { "OUT", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr" }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableGet")
            setFailState()
        end
        prv_cfg.saved_vtep1_vtep3_trunk1_interface_info =
            val["physicalInfoPtr"]
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk2_eport },
            { "OUT", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr" }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableGet")
            setFailState()
        end
        prv_cfg.saved_vtep1_vtep3_trunk2_interface_info =
            val["physicalInfoPtr"]
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk1_eport },
            { "OUT", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr"}
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoGet")
            setFailState()
        end
        prv_cfg.saved_vtep1_vtep2_trunk1_egress_port_info = val["egressInfoPtr"]
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk2_eport },
            { "OUT", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr"}
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoGet")
            setFailState()
        end
        prv_cfg.saved_vtep1_vtep2_trunk2_egress_port_info = val["egressInfoPtr"]
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk1_eport },
            { "OUT", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr"}
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoGet")
            setFailState()
        end
        prv_cfg.saved_vtep1_vtep3_trunk1_egress_port_info = val["egressInfoPtr"]
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk2_eport },
            { "OUT", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr"}
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoGet")
            setFailState()
        end
        prv_cfg.saved_vtep1_vtep3_trunk2_egress_port_info = val["egressInfoPtr"]

    else
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk1_eport },
            { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr",
                prv_cfg.saved_vtep1_vtep2_trunk1_interface_info }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableSet")
            setFailState()
        end
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk2_eport },
            { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr",
                prv_cfg.saved_vtep1_vtep2_trunk2_interface_info }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableSet")
            setFailState()
        end
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk1_eport },
            { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr",
                prv_cfg.saved_vtep1_vtep3_trunk1_interface_info }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableSet")
            setFailState()
        end
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk2_eport },
            { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr",
                prv_cfg.saved_vtep1_vtep3_trunk2_interface_info }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableSet")
            setFailState()
        end
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk1_eport },
            { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr",
                prv_cfg.saved_vtep1_vtep2_trunk1_egress_port_info }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoSet")
            setFailState()
        end
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk2_eport },
            { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr",
                prv_cfg.saved_vtep1_vtep2_trunk2_egress_port_info }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoSet")
            setFailState()
        end
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk1_eport },
            { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr",
                prv_cfg.saved_vtep1_vtep3_trunk1_egress_port_info }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoSet")
            setFailState()
        end
        local ret, val = myGenWrapper(
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk2_eport },
            { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr",
                prv_cfg.saved_vtep1_vtep3_trunk2_egress_port_info }
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoSet")
            setFailState()
        end
        return;
    end

    vxlanTsProfileInit(devNum, prv_cfg.vxlanTsProfileIndex);

    local tsConfigPtr = {}
    tsConfigPtr.ipv4Cfg = {}
    tsConfigPtr.ipv4Cfg.tagEnable = true
    tsConfigPtr.ipv4Cfg.vlanId = user_cfg.vtep1_vtep2_vid
    tsConfigPtr.ipv4Cfg.macDa = user_cfg.vtep2_mac
    tsConfigPtr.ipv4Cfg.dontFragmentFlag = true
    tsConfigPtr.ipv4Cfg.ttl = prv_cfg.tunnel_hdr_ttl
    tsConfigPtr.ipv4Cfg.destIp = user_cfg.vtep2_ip
    tsConfigPtr.ipv4Cfg.srcIp = user_cfg.vtep1_ip
    tsConfigPtr.ipv4Cfg.profileIndex = prv_cfg.vxlanTsProfileIndex
    tsConfigPtr.ipv4Cfg.udpSrcPort = tunnel_UDP_Src_Port;
    tsConfigPtr.ipv4Cfg.udpDstPort = VXLAN_UDP_Port
    tsConfigPtr.ipv4Cfg.ipHeaderProtocol = "CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E"

    local ret, val = myGenWrapper("cpssDxChTunnelStartEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "routerArpTunnelStartLineIndex", prv_cfg.vtep2_tunnel_index },
        { "IN", "CPSS_TUNNEL_TYPE_ENT", "tunnelType", "CPSS_TUNNEL_GENERIC_IPV4_E" },
        { "IN", "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_ipv4Cfg", "configPtr", tsConfigPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTunnelStartEntrySet")
        setFailState()
    end

    local tsConfigPtr = {}
    tsConfigPtr.ipv4Cfg = {}
    tsConfigPtr.ipv4Cfg.tagEnable = true
    tsConfigPtr.ipv4Cfg.vlanId = user_cfg.vtep1_vtep3_vid
    tsConfigPtr.ipv4Cfg.macDa = user_cfg.vtep3_mac
    tsConfigPtr.ipv4Cfg.dontFragmentFlag = true
    tsConfigPtr.ipv4Cfg.ttl = prv_cfg.tunnel_hdr_ttl
    tsConfigPtr.ipv4Cfg.destIp = user_cfg.vtep3_ip
    tsConfigPtr.ipv4Cfg.srcIp = user_cfg.vtep1_ip
    tsConfigPtr.ipv4Cfg.profileIndex = prv_cfg.vxlanTsProfileIndex
    tsConfigPtr.ipv4Cfg.udpSrcPort = tunnel_UDP_Src_Port;
    tsConfigPtr.ipv4Cfg.udpDstPort = VXLAN_UDP_Port
    tsConfigPtr.ipv4Cfg.ipHeaderProtocol = "CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E"

    local ret, val = myGenWrapper("cpssDxChTunnelStartEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "routerArpTunnelStartLineIndex", prv_cfg.vtep3_tunnel_index },
        { "IN", "CPSS_TUNNEL_TYPE_ENT", "tunnelType", "CPSS_TUNNEL_GENERIC_IPV4_E" },
        { "IN", "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_ipv4Cfg", "configPtr", tsConfigPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTunnelStartEntrySet")
        setFailState()
    end

    local trunk1_physicalInfoPtr = {
        type = "CPSS_INTERFACE_TRUNK_E",
        trunkId = prv_cfg.trunk1_id
    }
    local trunk2_physicalInfoPtr = {
        type = "CPSS_INTERFACE_TRUNK_E",
        trunkId = prv_cfg.trunk2_id
    }

    local ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk1_eport },
        { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", trunk1_physicalInfoPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk2_eport },
        { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", trunk2_physicalInfoPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk1_eport },
        { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", trunk1_physicalInfoPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk2_eport },
        { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", trunk2_physicalInfoPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableSet")
        setFailState()
    end

    local vtep2_egressInfoPtr = {}
    vtep2_egressInfoPtr.tunnelStart = true
    vtep2_egressInfoPtr.tunnelStartPtr = prv_cfg.vtep2_tunnel_index
    vtep2_egressInfoPtr.tsPassengerPacketType = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
    vtep2_egressInfoPtr.arpPtr = 0
    vtep2_egressInfoPtr.modifyMacSa = false
    vtep2_egressInfoPtr.modifyMacDa = false

    local vtep3_egressInfoPtr = {}
    vtep3_egressInfoPtr.tunnelStart = true
    vtep3_egressInfoPtr.tunnelStartPtr = prv_cfg.vtep3_tunnel_index
    vtep3_egressInfoPtr.tsPassengerPacketType = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
    vtep3_egressInfoPtr.arpPtr = 0
    vtep3_egressInfoPtr.modifyMacSa = false
    vtep3_egressInfoPtr.modifyMacDa = false

    local ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk1_eport },
        { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", vtep2_egressInfoPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk2_eport },
        { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", vtep2_egressInfoPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk1_eport },
        { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", vtep3_egressInfoPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk2_eport },
        { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", vtep3_egressInfoPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoSet")
        setFailState()
    end

    local ret, val = myGenWrapper("cpssDxChTunnelStartEntryExtensionSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk1_eport },
        { "IN", "GT_U32", "tsExtension", user_cfg.vtep1_vtep2_service_id }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTunnelStartEntryExtensionSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChTunnelStartEntryExtensionSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep2_trunk2_eport },
        { "IN", "GT_U32", "tsExtension", user_cfg.vtep1_vtep2_service_id }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTunnelStartEntryExtensionSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChTunnelStartEntryExtensionSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk1_eport },
        { "IN", "GT_U32", "tsExtension", user_cfg.vtep1_vtep3_service_id }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTunnelStartEntryExtensionSet")
        setFailState()
    end
    local ret, val = myGenWrapper("cpssDxChTunnelStartEntryExtensionSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", prv_cfg.vtep1_vtep3_trunk2_eport },
        { "IN", "GT_U32", "tsExtension", user_cfg.vtep1_vtep3_service_id }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTunnelStartEntryExtensionSet")
        setFailState()
    end
end

local function configure(unset)
    configure_vlan(unset)
    configure_tti(unset);
    configure_pcl(unset);
    configure_fdb(unset);
    configure_router(unset);
    configure_trunk(unset);
    configure_target_eport(unset);
    configure_ecmp(unset);
end
---------------------------------------------------------------------------------
local function readAndClearMacCounters(devNum, portNum)
    local clearOnReadStatus, ret, val, macCounters
    ret, val = myGenWrapper("cpssDxChPortMacCountersClearOnReadGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"OUT","GT_BOOL","enablePtr"}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChPortMacCountersClearOnReadGet")
        setFailState()
        return;
    end
    clearOnReadStatus = val["enablePtr"]
    if not clearOnReadStatus then
        ret, val = myGenWrapper("cpssDxChPortMacCountersClearOnReadSet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"OUT","GT_BOOL","enable", true}
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChPortMacCountersClearOnReadSet")
            setFailState()
            return;
        end
    end

    ret, val =  myGenWrapper("cpssDxChPortMacCountersOnPortGet",{  --getting mac counters for current port
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"OUT","CPSS_PORT_MAC_COUNTER_SET_STC","portMacCounterSetArrayPtr"}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChPortMacCountersClearOnReadSet")
        setFailState()
    end
    macCounters = val["portMacCounterSetArrayPtr"];

    if not clearOnReadStatus then
        ret, val = myGenWrapper("cpssDxChPortMacCountersClearOnReadSet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"OUT","GT_BOOL","enable", false}
        })
        if (ret ~= 0) then
            printLog ("Error in cpssDxChPortMacCountersClearOnReadSet")
            setFailState()
        end
    end
    return macCounters
end

-- ucPktsSent + mcPktsSent + brdcPktsSent
local function get_egress_packet_counters(devNum, port_list)
    local idx, port, counters, cnt_list
    cnt_list = {}
    for idx, port in pairs(port_list) do
        counters = readAndClearMacCounters(devNum, port)
        cnt_list[port] =
            (counters.ucPktsSent["l"][0]
            + counters.mcPktsSent["l"][0]
            + counters.brdcPktsSent["l"][0]);
    end
    return cnt_list;
end
local function send_packet_and_get_egress_packet_counters(
    devNum, ingress_port, packet, egress_port_list)
    local ret;
    local packetInfo = { partsArray = {
        { type = "TGF_PACKET_PART_PAYLOAD_E", partPtr = packet }
    }}
    -- clear counters
    get_egress_packet_counters(devNum, egress_port_list);
    -- send packet
    ret = prvLuaTgfTransmitPackets(
        devNum, ingress_port, packetInfo, 1, --[[burstCount--]]
        true, --[[withLoopBack_transmit--]] nil --[[ vfdArray--]]);
    if (ret ~= 0) then
        printLog ("Error in prvLuaTgfTransmitPackets")
        setFailState()
    end
    -- get and clear counters
    return get_egress_packet_counters(devNum, egress_port_list);
end

local function ip2bin_arr_str_rep_bits_5_4_3(ip, rep)
	local s, e;
	local d_str = {};
	local num = {};
	local h_str = {};
	s, e, d_str[1], d_str[2], d_str[3], d_str[4] =
        string.find(ip, "(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)");
    num[1] = tonumber(d_str[1]);
    num[2] = tonumber(d_str[2]);
    num[3] = tonumber(d_str[3]);
    num[4] = tonumber(d_str[4]);
    num[4] = (bit_and(num[4], 0xC7) + (bit_and(rep, 0x7) * 8));
    h_str[1] = string.format("%02X", num[1]);
    h_str[2] = string.format("%02X", num[2]);
    h_str[3] = string.format("%02X", num[3]);
    h_str[4] = string.format("%02X", num[4]);
    return (h_str[1] .. h_str[2] .. h_str[3] .. h_str[4]);
end
local function ip2bin_arr_str(ip)
	local s, e;
	local d_str = {};
	local num = {};
	local h_str = {};
	s, e, d_str[1], d_str[2], d_str[3], d_str[4] =
        string.find(ip, "(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)%.(%d?%d?%d?)");
    num[1] = tonumber(d_str[1]);
    num[2] = tonumber(d_str[2]);
    num[3] = tonumber(d_str[3]);
    num[4] = tonumber(d_str[4]);
    h_str[1] = string.format("%02X", num[1]);
    h_str[2] = string.format("%02X", num[2]);
    h_str[3] = string.format("%02X", num[3]);
    h_str[4] = string.format("%02X", num[4]);
    return (h_str[1] .. h_str[2] .. h_str[3] .. h_str[4]);
end
local function ip_hdr_add(src, dst, ttl, protocol, payload)
    local src_ip, src_ip_inc;
    local src_ip_val, src_ip_final;
    if type(src) == "table" then
        src_ip = src[1]
        src_ip_inc = src[2]
    else
        src_ip = src
        src_ip_inc = nil
    end
    if src_ip_inc == nil then
        src_ip_val = ip2bin_arr_str(src_ip)
    else
        src_ip_val = ip2bin_arr_str_rep_bits_5_4_3(src_ip, src_ip_inc)
    end

    local values = {
        src = src_ip_val,
        dst = ip2bin_arr_str(dst),
        ttl = ttl,
        protocol = protocol
    };
    return proto_create_struct_ip(values, payload);
end
local function udp_hdr_add(src_port, dst_port, payload)
    local packet_len = (8 + (string.len(payload) / 2));
    local packet = (
        string.format("%04X", src_port) ..
        string.format("%04X", dst_port) ..
        string.format("%04X", packet_len) ..
        "0000" .. payload);
    packet = set_crc16(packet, packet_len, 6);
    return packet;
end
local function vxlan_hdr_add(service_id, payload)
    return ("08000000" .. string.format("%06X", service_id) .. "00" .. payload);
end
local function mac_hdr_add(src_mac, dst_mac, vid, proto, payload, padding)
    local macs =
        (string.gsub(dst_mac, ":", "")
        .. string.gsub(src_mac, ":", ""));
    local vlan = "";
    if vid then
        vlan = ("8100" .. string.format("%04X", vid));
    end
    local packet = macs .. vlan .. string.format("%04X", proto) .. payload;
    if padding then
        local len = string.len(packet); -- length in nibbles
        if len < 120 then
            packet = packet .. string.rep("0", (120 - len));
        end
    end
    return packet;
end
local function vxlan_tunnel_hdr_add(
    src_mac, dst_mac, vid,
    src_ip, dst_ip, ttl, src_port, service_id, payload)
    local packet = vxlan_hdr_add(service_id, payload);
    packet = udp_hdr_add(src_port, VXLAN_UDP_Port, packet)
    packet = ip_hdr_add(src_ip, dst_ip, ttl, 17--[[UDP--]], packet)
    packet = mac_hdr_add(src_mac, dst_mac, vid, 0x800 --[[IPV4--]], packet, true);
    return packet;
end

local payload = "01020304"

local function sent_packet(i)
    local sent_ip_l3 = ip_hdr_add(
        {user_cfg.host1_ip, i}, user_cfg.host2_ip, 64, payload_ip_protocol, payload);
    local sent_mac_l2 = mac_hdr_add(
        user_cfg.host1_mac, user_cfg.host2_mac, user_cfg.host1_vid, 0x800, sent_ip_l3, true);
    return sent_mac_l2;
end
local function received_packet(i)
    local rcv_passenger_l3 = ip_hdr_add(
        {user_cfg.host1_ip, i}, user_cfg.host2_ip, 63, payload_ip_protocol, payload);
    local rcv_passenger_l2 = mac_hdr_add(
        user_cfg.vtep1_mac, user_cfg.host2_mac, user_cfg.host2_vid, 0x800, rcv_passenger_l3, true);
    local rcv_packet;
    -- if i is 0,1,2,3 vtep2 tunnel, if i is 4,5,6,7 vtep3 tunnel
    if ((i % 8) < 4) then
        rcv_packet = vxlan_tunnel_hdr_add(
            user_cfg.vtep1_mac, user_cfg.vtep2_mac, user_cfg.vtep1_vtep2_vid,
            user_cfg.vtep1_ip, user_cfg.vtep2_ip, prv_cfg.tunnel_hdr_ttl,
            tunnel_UDP_Src_Port, user_cfg.vtep1_vtep2_service_id, rcv_passenger_l2);
    else
        rcv_packet = vxlan_tunnel_hdr_add(
            user_cfg.vtep1_mac, user_cfg.vtep3_mac, user_cfg.vtep1_vtep3_vid,
            user_cfg.vtep1_ip, user_cfg.vtep3_ip, prv_cfg.tunnel_hdr_ttl,
            tunnel_UDP_Src_Port, user_cfg.vtep1_vtep3_service_id, rcv_passenger_l2);
    end
    return rcv_packet
end

local function try_packet_receive_port(i)
    local egress_port_list = {
        user_cfg.trunk1_member1_port,
        user_cfg.trunk1_member2_port,
        user_cfg.trunk2_member1_port,
        user_cfg.trunk2_member2_port
    }
    local egress_counters, port, cnt, first_port;
    local packet = sent_packet(i);
    egress_counters = send_packet_and_get_egress_packet_counters(
        devNum, user_cfg.host1_link_port, packet, egress_port_list);
    for port, cnt in pairs (egress_counters) do
        if cnt ~= 0 then
            if first_port == nil then
                first_port = port
            end
            printLog ("packet " .. i .." egressed from port " .. port .. " counter " .. cnt)
        end
    end
    return first_port
end

---------------------------------------------------------------------------------

--generate vxlan test packets
local function send_and_check_host1_to_host2(i)
    local transmitInfo;
    local egressInfoTable;
    local rc = 0;
    local my_maskedBytesTable = {
        {startByte = 22, endByte = 24 }-- 2 bytes 'identification field' and 1 byte flags
        ,{startByte = 28, endByte = 29 }-- 2 bytes IPV4  'checksum'
        ,{startByte = 44, endByte = 45 }-- 2 bytes UDP 'checksum'
    }
    local egress_port = try_packet_receive_port(i);
    if egress_port == nil then
        printLog ("No egress to all ports: Test " .. i .." failed \n")
        setFailState()
        return
    end
    -- from host1 to host3
    transmitInfo =
    {
        devNum = devNum, portNum = user_cfg.host1_link_port,
        pktInfo = {fullPacket = sent_packet(i)}
    }
    egressInfoTable =
    {
        -- expected egress port (apply also mask on needed fields)
        {
            portNum = egress_port,
            pktInfo = {fullPacket = received_packet(i),
                maskedBytesTable = my_maskedBytesTable}
        }
    }
    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        printLog ("Test " .. i .." passed \n")
    else
        printLog ("Test " .. i .." failed \n")
        setFailState()
    end
end

local function test()
    local i;
    configure(false);
    for i = 0,7 do
        send_and_check_host1_to_host2(i)
    end
    configure(true);
end

-- reinit legacy key compatible UDB configuration
executeStringCliCommands("do shell-execute prvCpssDxChTtiDbTablesInit ${dev}\n")
--

test();

function example_vxlan_l3_lb_config()
    configure(false);
end
function example_vxlan_l3_lb_deconfig()
    configure(true);
end
function example_vxlan_l3_lb_traffic()
    for i = 0,7 do
        send_and_check_host1_to_host2(i)
    end
end

function example_vxlan_l3_lb_set_ports(p1, p2, p3, p4, p5)
    example_vxlan_l3_lb_param_host1_link_port     = tonumber(p1)
    example_vxlan_l3_lb_param_trunk1_member1_port = tonumber(p2)
    example_vxlan_l3_lb_param_trunk1_member2_port = tonumber(p3)
    example_vxlan_l3_lb_param_trunk2_member1_port = tonumber(p4)
    example_vxlan_l3_lb_param_trunk2_member2_port = tonumber(p5)
    reinit_local_param();
end
function example_vxlan_l3_lb_set_host_ips(ip1, ip2)
    example_vxlan_l3_lb_param_host1_ip = ip1
    example_vxlan_l3_lb_param_host2_ip = ip2
    reinit_local_param();
end
function example_vxlan_l3_lb_set_host_macs(mac1, mac2)
    example_vxlan_l3_lb_param_host1_mac = mac1
    example_vxlan_l3_lb_param_host2_mac = mac2
    reinit_local_param();
end
function example_vxlan_l3_lb_set_host_vids(vid1, vid2)
    example_vxlan_l3_lb_param_host1_vid = tonumber(vid1)
    example_vxlan_l3_lb_param_host2_vid = tonumber(vid2)
    reinit_local_param();
end
function example_vxlan_l3_lb_set_vtep_ips(ip1, ip2, ip3)
    example_vxlan_l3_lb_param_vtep1_ip = ip1
    example_vxlan_l3_lb_param_vtep2_ip = ip2
    example_vxlan_l3_lb_param_vtep3_ip = ip3
    reinit_local_param();
end
function example_vxlan_l3_lb_set_vtep_macs(mac1, mac2, mac3)
    example_vxlan_l3_lb_param_vtep1_mac = mac1
    example_vxlan_l3_lb_param_vtep2_mac = mac2
    example_vxlan_l3_lb_param_vtep3_mac = mac3
    reinit_local_param();
end
function example_vxlan_l3_lb_set_vtep_vids(vid1, vid2)
    example_vxlan_l3_lb_param_vtep1_vtep2_vid = tonumber(vid1)
    example_vxlan_l3_lb_param_vtep1_vtep3_vid = tonumber(vid2)
    reinit_local_param();
end
function example_vxlan_l3_lb_set_service_ids(service_id1, service_id2)
    example_vxlan_l3_lb_param_vtep1_vtep2_service_id = tonumber(service_id1)
    example_vxlan_l3_lb_param_vtep1_vtep3_service_id = tonumber(service_id2)
    reinit_local_param();
end


function example_vxlan_l3_lb_print()
    print(
        " host1_link_port     = " .. example_vxlan_l3_lb_param_host1_link_port     ..
        " trunk1_member1_port = " .. example_vxlan_l3_lb_param_trunk1_member1_port ..
        " trunk1_member2_port = " .. example_vxlan_l3_lb_param_trunk1_member2_port ..
        " trunk2_member1_port = " .. example_vxlan_l3_lb_param_trunk2_member1_port ..
        " trunk2_member2_port = " .. example_vxlan_l3_lb_param_trunk2_member2_port .. "\n");
    print("update by example_vxlan_l3_lb_set_ports(p1, p2, p3, p4, p5)\n");
    print(
        " host1_ip = " .. example_vxlan_l3_lb_param_host1_ip ..
        " host2_ip = " .. example_vxlan_l3_lb_param_host2_ip .. "\n");
    print("update by example_vxlan_l3_lb_set_host_ips(ip1, ip2)\n");
    print(
        " host1_mac = " .. example_vxlan_l3_lb_param_host1_mac ..
        " host3_mac = " .. example_vxlan_l3_lb_param_host2_mac .. "\n");
    print("update by example_vxlan_l3_lb_set_host_macs(mac1, mac2)\n");
    print(
        " host1_vid = " .. example_vxlan_l3_lb_param_host1_vid ..
        " host3_vid = " .. example_vxlan_l3_lb_param_host2_vid .. "\n");
    print("update by example_vxlan_l3_lb_set_host_vids(vid1, vid2)\n");
    print(
        " vtep1_ip = " .. example_vxlan_l3_lb_param_vtep1_ip ..
        " vtep2_ip = " .. example_vxlan_l3_lb_param_vtep2_ip ..
        " vtep3_ip = " .. example_vxlan_l3_lb_param_vtep3_ip .. "\n");
    print("update by example_vxlan_l3_lb_set_vtep_ips(ip1, ip2, ip3)\n");
    print(
        " vtep1_mac = " .. example_vxlan_l3_lb_param_vtep1_mac ..
        " vtep2_mac = " .. example_vxlan_l3_lb_param_vtep2_mac ..
        " vtep3_mac = " .. example_vxlan_l3_lb_param_vtep3_mac .. "\n");
    print("update by example_vxlan_l3_lb_set_vtep_macs(mac1, mac2, mac3)\n");
    print(
        " vtep1_vtep2_vid = " .. example_vxlan_l3_lb_param_vtep1_vtep2_vid ..
        " vtep1_vtep3_vid = " .. example_vxlan_l3_lb_param_vtep1_vtep3_vid .. "\n");
    print("update by example_vxlan_l3_lb_set_vtep_vids(vid1, vid2)\n");
    print(
        " vtep1_vtep2_service_id = " .. example_vxlan_l3_lb_param_vtep1_vtep2_service_id ..
        " vtep1_vtep3_service_id = " .. example_vxlan_l3_lb_param_vtep1_vtep3_service_id .. "\n");
    print("update by example_vxlan_l3_lb_set_service_ids(service_id1, service_id2)\n");
    print("reset parameters by example_vxlan_l3_lb_param_reset()\n");
    print("configure by example_vxlan_l3_lb_config()\n");
    print("deconfigure by example_vxlan_l3_lb_deconfig()\n");
    print("traffic by example_vxlan_l3_lb_traffic()\n");
end

--[[
Using example:
cpssInitSystem 29,1,0
examples
do cls
add-test-files vxlan_routing
log-mode-select verbose
run vxlan_routing
lua
example_vxlan_l3_lb_print();
example_vxlan_l3_lb_set_ports(8, 12, 16, 20, 24);
example_vxlan_l3_lb_set_host_ips("172.116.11.0", "172.116.99.2");
example_vxlan_l3_lb_set_host_macs("00:99:99:11:11:11", "00:99:99:22:22:22");
example_vxlan_l3_lb_set_host_vids(98,99);
example_vxlan_l3_lb_set_vtep_ips("10.1.99.1", "10.1.99.2", "10.1.99.3");
example_vxlan_l3_lb_set_vtep_macs("00:99:99:AA:AA:AA", "00:99:99:BB:BB:BB", "00:99:99:CC:CC:CC");
example_vxlan_l3_lb_set_vtep_vids(0x99A, 0x99B);
example_vxlan_l3_lb_print();
example_vxlan_l3_lb_set_service_ids(0x991122, 0x992233);
example_vxlan_l3_lb_print();
.
run vxlan_routing
lua
example_vxlan_l3_lb_config()
--send packets by smartbit and check
example_vxlan_l3_lb_deconfig()
.
--]]
