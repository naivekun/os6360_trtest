--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vxlan_routing.lua
--*
--* DESCRIPTION:
--*       The test for vxlan routed ipv4 packets passing.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5")

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmVirtualRouterAdd")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixAdd")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixDel")

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

---------------------------------------------------------------------------------
-- VXLAN Tunnel Endpoint (VTEP)
-- The simulated system is 3 devices VTEP1,VTEP2 and VTEP3.
-- VTEP1 connects local network and VXLAN core1
-- VTEP2 connects local network, VXLAN core1 and VXLAN core2
-- VTEP3 connects local network and VXLAN core2
-- Host1 in Device VTEP1 local network
-- Host2 in Device VTEP2 local network
-- Host3 in Device VTEP3 local network
-- Case1 path of packet is Host1 -> VTEP1 -> VTEP2 -> VTEP3 -> Host3
-- Case2 path of packet is Host1 -> VTEP1 -> VTEP2 ->  Host2
-- The device VTEP2 packets passing only tested.
--
-- VTEP2 configuration
-- Configure the TTI TCAM:
-- Key: MAC2ME, DIP = 10.1.1.1, SIP = 10.1.1.2, IP protocol = UDP, L4 DPORT = VXLAN
-- Action: Tunnel Terminate
-- Action: Assign source ePort = 0x300 --***** used 0x01
-- Action: Assign IPCL configuration Table Index = 1 --***** used 0xFFE
-- Configure MAC2ME table
-- Pre TT: 0xAAAAAA
-- Post TT: 0x111111
-- Configure the IPCL lookup for IPCL configuration Table Index = 1
-- Key: VNI = 75987
-- Action: Assign eVLAN = 1000 --***** used 0xA01
-- Configure the eVLAN table for eVLAN =1000
-- VRF ID = 1 --***** used 7
-- Enable skipping the FDB for MAC2ME packets for ePort 0x300
-- Configure the source MAC address of the device for routing to 0x111111
-- Configure the host routes:
-- ----   VRF-ID    IP_address     MAC_DA    eVLAN  Target_ePort
-- Host2       1   172.116.1.2   0x222222     1001         Local
-- Host3       1   172.116.1.3   0x333333     1001          0x300
-- Map ePort 0x300 to the Trunk 1
-- Configure Trunk 1 to the physical core ports
--  Associate ePort 0x300 with IP TS:
-- MAC DA = 0xBBBBBB
-- MAC SA = 0xAAAAAA
-- DIP: 10.1.1.3
-- SIP: 10.1.1.1
-- VNI is automatically configured from the eVLAN<Service ID>
-- For eVLAN 1001 configure:
--  Service ID = 123789
-- VID = 19
--
-- Test case1
-- Send the following packet as the ingress VXLAN packet:
-- Tunnel MAC DA = 0xAAAAAA
-- Tunnel DIP = 10.1.1.1
-- Tunnel SIP = 10.1.1.2
-- VNI= 75987
-- Passenger MAC DA = 0x111111
-- Passenger DIP = 172.16.1.2
-- Check that the following packet egress at the local port:
-- MAC DA = 0x222222
-- MAC SA = 0x111111
-- VLAN = 19
-- DIP = 172.16.1.2
-- TTL was decreased by 1
--
-- Test case2
-- Send the following packet as the ingress VXLAN packet:
-- Tunnel MAC DA = 0xAAAAAA
-- Tunnel DIP = 10.1.1.1
-- Tunnel SIP = 10.1.1.2
-- VNI= 75987
-- Passenger MAC DA = 0x111111
-- Passenger DIP = 172.16.1.3
-- Check that the following packet egress at the LAG to the core network:
-- Tunnel MAC DA = 0xBBBBBB
-- Tunnel MAC SA = 0xAAAAAA
-- Tunnel DIP = 10.1.1.3
-- Tunnel SIP = 10.1.1.1
-- VNI= 123789
-- MAC DA = 0x333333
-- MAC SA = 0x111111
-- DIP = 172.16.1.3
-- TTL was decreased by 1
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

function example_vxlan_routing_param_reset_globals()
    example_vxlan_routing_param_host1_ip = "172.116.1.1"
    example_vxlan_routing_param_host2_ip = "172.116.1.2"
    example_vxlan_routing_param_host3_ip = "172.116.1.3"
    example_vxlan_routing_param_host1_mac = "00:00:00:11:11:11"
    example_vxlan_routing_param_host2_mac = "00:00:00:22:22:22"
    example_vxlan_routing_param_host3_mac = "00:00:00:33:33:33"
    example_vxlan_routing_param_host1_vid = 18
    example_vxlan_routing_param_host2_vid = 19
    example_vxlan_routing_param_host3_vid = 20
    example_vxlan_routing_param_vtep1_ip = "10.1.1.1"
    example_vxlan_routing_param_vtep2_ip = "10.1.1.2"
    example_vxlan_routing_param_vtep3_ip = "10.1.1.3"
    example_vxlan_routing_param_vtep1_mac = "00:00:00:AA:AA:AA"
    example_vxlan_routing_param_vtep2_mac = "00:00:00:BB:BB:BB"
    example_vxlan_routing_param_vtep3_mac = "00:00:00:CC:CC:CC"
    example_vxlan_routing_param_vtep1_vtep2_vid = 0x901
    example_vxlan_routing_param_vtep2_vtep3_vid = 0x903
    example_vxlan_routing_param_vtep1_vtep2_service_id = 0x128D3 --75987
    example_vxlan_routing_param_vtep2_vtep3_service_id = 0x1E38D --123789
    example_vxlan_routing_param_vtep1_link_port = port1
    example_vxlan_routing_param_vtep3_link_port = port3
    example_vxlan_routing_param_host2_link_port = port2
end
if not example_vxlan_routing_param_inited then
    example_vxlan_routing_param_reset_globals()
    example_vxlan_routing_param_inited = true
end

local host1_ip
local host2_ip
local host3_ip
local host1_mac
local host2_mac
local host3_mac
local host1_vid
local host2_vid
local host3_vid
local vtep1_ip
local vtep2_ip
local vtep3_ip
local vtep1_mac
local vtep2_mac
local vtep3_mac
local vtep1_vtep2_vid
local vtep2_vtep3_vid
local vtep1_vtep2_service_id
local vtep2_vtep3_service_id
local vtep1_link_port
local vtep3_link_port
local host2_link_port
--miscelanous
local vtep1_vtep2_vrf_id
local tunnel_hdr_ttl
local vtep1_vtep2_eport
local vtep2_vtep3_eport
local vtep1_vtep2_evid
local vtep2_vtep3_evid

-- MACs, VIDs and IPs, service_ids, ports
local function reinit_local_param()
    host1_ip  = example_vxlan_routing_param_host1_ip
    host2_ip  = example_vxlan_routing_param_host2_ip
    host3_ip  = example_vxlan_routing_param_host3_ip
    host1_mac = example_vxlan_routing_param_host1_mac
    host2_mac = example_vxlan_routing_param_host2_mac
    host3_mac = example_vxlan_routing_param_host3_mac
    host1_vid = example_vxlan_routing_param_host1_vid
    host2_vid = example_vxlan_routing_param_host2_vid
    host3_vid = example_vxlan_routing_param_host3_vid
    vtep1_ip  = example_vxlan_routing_param_vtep1_ip
    vtep2_ip  = example_vxlan_routing_param_vtep2_ip
    vtep3_ip  = example_vxlan_routing_param_vtep3_ip
    vtep1_mac = example_vxlan_routing_param_vtep1_mac
    vtep2_mac = example_vxlan_routing_param_vtep2_mac
    vtep3_mac = example_vxlan_routing_param_vtep3_mac
    vtep1_vtep2_vid = example_vxlan_routing_param_vtep1_vtep2_vid
    vtep2_vtep3_vid = example_vxlan_routing_param_vtep2_vtep3_vid
    vtep1_vtep2_service_id = example_vxlan_routing_param_vtep1_vtep2_service_id
    vtep2_vtep3_service_id = example_vxlan_routing_param_vtep2_vtep3_service_id
    vtep1_link_port = example_vxlan_routing_param_vtep1_link_port
    vtep3_link_port = example_vxlan_routing_param_vtep3_link_port
    host2_link_port = example_vxlan_routing_param_host2_link_port
    --miscelanous
    vtep1_vtep2_vrf_id = 7
    tunnel_hdr_ttl = 0x70
    vtep1_vtep2_eport = 0x301
    vtep2_vtep3_eport = 0x303
    vtep1_vtep2_evid = 0xA01
    vtep2_vtep3_evid = 0xA03
end

reinit_local_param();

-- TCAM memory
local tti_rules_base = myGenWrapper("appDemoDxChTcamTtiBaseIndexGet", {
    { "IN", "GT_U8", "devNum", devNum},
    { "IN", "GT_U32", "hitNum", 0 }
})
local tti_rule1_index = tti_rules_base + 12; -- at the same bank of floor
local pcl_rule1_index = 24;
-- miscelanous HW values
local pcl_cfg_table_index = 0xFFE; -- near bound of per-VLAN entries
local vtep1_vtep2_pcl_id = 0xAA
local host2_arp_index = 5
local host3_arp_index = 6
local vtep3_arp_index = 7
local host2_next_hop_index = 10
local host3_next_hop_index = 11
local vtep3_next_hop_index = 12
local vtep3_tunnel_index = 13
local vxlanTsProfileIndex = 0;

local ret, val = myGenWrapper("cpssDxChCfgHwDevNumGet",{
    {"IN","GT_U8","devNum", devNum},
    {"OUT","GT_HW_DEV_NUM","hwDevNumPtr"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChCfgHwDevNumGet")
    setFailState()
end
local hw_device_id_number = val["hwDevNumPtr"];

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

local function ip_addr2number(ip)
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
    local packetType, patternTable, patternMaskTable, actionType, actionTable;
    packetType = "CPSS_DXCH_TTI_RULE_IPV4_E"
    actionType = "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"
    patternTable = {ipv4 = {
        srcIp  = ip_addr2number(src_ip),
        destIp = ip_addr2number(dst_ip)
    }}
    patternMaskTable = {ipv4 = {
        srcIp  = 0xFFFFFFFF,
        destIp = 0xFFFFFFFF
    }}
    actionTable = {}
    actionTable["type2"] = {
        tag1VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
        command = "CPSS_PACKET_CMD_FORWARD_E",
        tunnelTerminate = true,
        -- ipv4, udp, vxlan; From L3 start in resolution of 2 bytes (CPSS will divide by 2)
        ttHeaderLength  = (20 + 8 + 8),
        enableDecrementTtl = true,
        ttPassengerPacketType = "CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E",
        --ttPassengerPacketType = "CPSS_DXCH_TTI_PASSENGER_IPV4V6_E",
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
                {"IN","CPSS_DXCH_TTI_RULE_UNT_ipv4","maskPtr",patternMaskTable},
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
        ]] .. "\n" .. "mac address " .. vtep2_mac .. "\n"
    end
    executeStringCliCommands(set_device_mac);

    mac_to_me(devNum, 0--[[index--]], vtep2_mac, vtep1_vtep2_vid, unset);
    mac_to_me(devNum, 1--[[index--]], host2_mac, vtep1_vtep2_vid, unset);
    mac_to_me(devNum, 2--[[index--]], host3_mac, vtep1_vtep2_vid, unset);
    tti_port_lookup_enable(devNum, vtep1_link_port, "CPSS_DXCH_TTI_KEY_IPV4_E", unset)
    tti_rule(
        devNum, tti_rule1_index, vtep1_ip, vtep2_ip,
        pcl_cfg_table_index, vtep1_vtep2_eport, unset)
end

local function pcl_rule(rule_index, pcl_id, service_id, e_vlan,vrfId, unset)
    -- udb 20,21,22 as L4+12,13,14 vxlan_for service id and pclId==vtep1_vtep2_pcl_id
    local ndx;
    local udb_anchor = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E";
    local udb_offset = (2 + 20 + 8 + 4); -- eth_protocol, IPV4_header, UDP_header, word in VXLAN_header
    if unset then
        udb_anchor = "CPSS_DXCH_PCL_OFFSET_INVALID_E";
    end
    for ndx = 0,2 do
        local ret, val = myGenWrapper("cpssDxChPclUserDefinedByteSet",{
                { "IN",     "GT_U8",                                "devNum",     devNum },
                { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat",
                    "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E"},
                { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType",
                    "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E"},
                { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",
                    "CPSS_PCL_DIRECTION_INGRESS_E"},
                { "IN",     "GT_U32",                               "udbIndex",   (20 + ndx) },
                { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", udb_anchor },
                { "IN",     "GT_U8",                                "offset",     (udb_offset + ndx) }
        })
        if ret~=0 then
            printLog ("Error in cpssDxChPclUserDefinedByteSet")
            setFailState()
        end
    end
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
        vlan = {
            ingress = {
                modifyVlan = "CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E",
                vlanId = e_vlan,
                precedence = "CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E"
            }
        }
    };

    if(is_sip_6(devNum)) then
        -- the <VRF-ID> per vlan moved from Bridge (L2i) to the TTI ,
        -- so modification of eVlan by the PCL action for packets that need <VRF-ID>
        -- must also assign the <VRF-ID>
        pclAction.redirect = { data = {} }
        pclAction.redirect.redirectCmd = "CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E"
        pclAction.redirect.data.vrfId = vrfId
    end


    local pclMask = {
        ruleStdIpv4L4 = {
            udb20_22 = {
                [0] = 0xFF,
                [1] = 0xFF,
                [2] = 0xFF
            },
            common = {pclId = 0x3FF}
        }
    };
    local pclPattern = {
        ruleStdIpv4L4 = {
            udb20_22 = {
                [0] = bit_and(bit_shr(service_id, 16), 0xFF),
                [1] = bit_and(bit_shr(service_id, 8), 0xFF),
                [2] = bit_and(service_id, 0xFF),
            },
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
        { "IN",     "GT_PORT_NUM", "portNum",   vtep1_vtep2_eport},
        { "IN",     "GT_BOOL",     "enable",    enable}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChPclPortIngressPolicyEnable")
        setFailState()
    end
    local interfaceInfo = {
        type = "CPSS_INTERFACE_INDEX_E",
        index = pcl_cfg_table_index
    };
    local lookupCfgPtr = {
        enableLookup = enable,
        pclId = vtep1_vtep2_pcl_id,
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
        pcl_rule1_index, vtep1_vtep2_pcl_id,
        vtep1_vtep2_service_id, vtep1_vtep2_evid, vtep1_vtep2_vrf_id ,unset);
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
                portNum = 61 -- tunnel, will be overriden
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
    configure_fdb_routed_entry(unset, vtep2_mac, vtep1_vtep2_evid);
    configure_fdb_routed_entry(unset, host2_mac, vtep1_vtep2_evid);
    configure_fdb_routed_entry(unset, host3_mac, vtep1_vtep2_evid);
end

local function configure_vlan(unset)
    if unset then
        local ret = cpssGenWrapper("cpssDxChBrgVlanEntryInvalidate",{
          { "IN", "GT_U8", "devNum", devNum },
          { "IN", "GT_U16","vlanId", vtep1_vtep2_evid }
        })
        if ret~=0 then
            printLog ("Error in cpssDxChBrgVlanEntryInvalidate")
            setFailState()
        end
        local ret = cpssGenWrapper("cpssDxChBrgVlanEntryInvalidate",{
          { "IN", "GT_U8", "devNum", devNum },
          { "IN", "GT_U16","vlanId", host2_vid }
        })
        if ret~=0 then
            printLog ("Error in cpssDxChBrgVlanEntryInvalidate")
            setFailState()
        end
        local ret = cpssGenWrapper("cpssDxChBrgVlanEntryInvalidate",{
          { "IN", "GT_U8", "devNum", devNum },
          { "IN", "GT_U16","vlanId", vtep2_vtep3_vid }
        })
        if ret~=0 then
            printLog ("Error in cpssDxChBrgVlanEntryInvalidate")
            setFailState()
        end
        local ret = cpssGenWrapper("cpssDxChBrgVlanEntryInvalidate",{
          { "IN", "GT_U8", "devNum", devNum },
          { "IN", "GT_U16","vlanId", host3_vid }
        })
        if ret~=0 then
            printLog ("Error in cpssDxChBrgVlanEntryInvalidate")
            setFailState()
        end
        return
    end

    -- VLAN with routing
    local ret, values
    ret,values = cpssGenWrapper("cpssDxChBrgVlanEntryRead",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", vtep1_vtep2_evid },
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
    values.vlanInfo.fidValue         = vtep1_vtep2_evid
    values.vlanInfo.ipv4UcastRouteEn = true
    values.vlanInfo.ipv6UcastRouteEn = true
    values.vlanInfo.vrfId            = vtep1_vtep2_vrf_id
    ret=cpssGenWrapper("cpssDxChBrgVlanEntryWrite",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", vtep1_vtep2_evid },
      { "IN", "CPSS_PORTS_BMP_STC", "portsMembers", values.portsMembers },
      { "IN", "CPSS_PORTS_BMP_STC", "portsTagging", values.portsTagging },
      { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo", values.vlanInfo},
      { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd", values.portsTaggingCmd}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChBrgVlanEntryWrite")
        setFailState()
    end
    ---- VLANs with ports
    ret,values = cpssGenWrapper("cpssDxChBrgVlanEntryRead",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", host2_vid },
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
    values.vlanInfo.fidValue         = host2_vid
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, host2_link_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, host2_link_port)
    if not values.portsTaggingCmd then values.portsTaggingCmd = {} end
    if not values.portsTaggingCmd.portsCmd then values.portsTaggingCmd.portsCmd = {} end
    values.portsTaggingCmd.portsCmd[host2_link_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";
    local ret=cpssGenWrapper("cpssDxChBrgVlanEntryWrite",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", host2_vid },
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
      { "IN", "GT_U16","vlanId", vtep2_vtep3_vid },
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
    values.vlanInfo.fidValue         = vtep2_vtep3_vid
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, vtep3_link_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, vtep3_link_port)
    if not values.portsTaggingCmd then values.portsTaggingCmd = {} end
    if not values.portsTaggingCmd.portsCmd then values.portsTaggingCmd.portsCmd = {} end
    values.portsTaggingCmd.portsCmd[vtep3_link_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";

    local ret=cpssGenWrapper("cpssDxChBrgVlanEntryWrite",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", vtep2_vtep3_vid },
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
      { "IN", "GT_U16","vlanId", host3_vid },
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
    values.vlanInfo.fidValue         = host3_vid
    values.portsMembers = wrlCpssSetBmpPort(values.portsMembers, vtep3_link_port)
    values.portsTagging = wrlCpssSetBmpPort(values.portsTagging, vtep3_link_port)
    if not values.portsTaggingCmd then values.portsTaggingCmd = {} end
    if not values.portsTaggingCmd.portsCmd then values.portsTaggingCmd.portsCmd = {} end
    values.portsTaggingCmd.portsCmd[vtep3_link_port] = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E";

    local ret=cpssGenWrapper("cpssDxChBrgVlanEntryWrite",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", host3_vid },
      { "IN", "CPSS_PORTS_BMP_STC", "portsMembers", values.portsMembers },
      { "IN", "CPSS_PORTS_BMP_STC", "portsTagging", values.portsTagging },
      { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo", values.vlanInfo},
      { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd", values.portsTaggingCmd}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChBrgVlanEntryWrite")
        setFailState()
    end

    local ret, val = myGenWrapper("cpssDxChTunnelStartEgessVlanTableServiceIdSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U16", "vlanId",  host3_vid},
        { "IN", "GT_U32", "vlanServiceId",  vtep2_vtep3_service_id}
    })
    if ret~=0 then
        printLog ("Error in cpssDxChTunnelStartEgessVlanTableServiceIdSet")
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
        profileDataPtr.templateDataBitsCfg[i] = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_2_E";
    end
    for i = (sidBaseBit + 8), (sidBaseBit + 15) do -- set bits 40 - 47 (VXLAN) or bits 8 - 15 (NVGRE)
        profileDataPtr.templateDataBitsCfg[i] = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_1_E";
    end
    for i = (sidBaseBit + 16), (sidBaseBit + 23) do -- set bits 48 - 55 (VXLAN) or bits 16 - 23 (NVGRE)
        profileDataPtr.templateDataBitsCfg[i] = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_0_E";
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
        print("Error while executing cpssDxChTunnelStartGenProfileTableEntrySet")
    end
end

local function configure_router(unset)
    local enable = (not unset);

    local ret = cpssGenWrapper("cpssDxChIpPortRoutingEnable",{
        { "IN", "GT_U8",                         "devNum",         devNum                     },
        { "IN", "GT_PORT_NUM",                   "portNum",        vtep1_link_port            },
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
        { "IN", "GT_PORT_NUM",                   "portNum",        vtep1_vtep2_eport          },
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
            {"IN", "GT_U32", "vrId", vtep1_vtep2_vrf_id}
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
			 ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",
			 priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_EXACT_MATCH_E"
	   }},
		ipv4Mc = {
		   routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
		   numOfPaths = 0,
		   routeEntryBaseIndex = 1,
		   ucRPFCheckEnable = false,
		   sipSaCheckMismatchEnable = false,
		   ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",
		   priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_EXACT_MATCH_E"
		},

	   -- CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
	   ipv6Uc = {
		  lttEntry = {
			 routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
			 numOfPaths = 0,
			 routeEntryBaseIndex = 0,
			 ucRPFCheckEnable = false,
			 sipSaCheckMismatchEnable = false,
			 ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",
			 priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_EXACT_MATCH_E"

	   }},
	   ipv6Mc = {
		  routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
		  numOfPaths = 0,
		  routeEntryBaseIndex = 1,
		  ucRPFCheckEnable = false,
		  sipSaCheckMismatchEnable = false,
		  ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",
		  priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_EXACT_MATCH_E"
	   },
	   fcoe = {
		   lttEntry = {
			  routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
			  routeEntryBaseIndex = 0,
			  ucRPFCheckEnable = false,
			  sipSaCheckMismatchEnable = false,
			  ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",
			  priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_EXACT_MATCH_E"
		}}
	}

    local vrConfig = {
       supportIpv4Uc = true,
       defIpv4UcNextHopInfo   = defaultRoutesInfo.ipv4Uc,
       supportIpv4Mc = false,
       defIpv4McRouteLttEntry = defaultRoutesInfo.ipv4Mc,
       supportIpv6Uc = true,
       defIpv6UcNextHopInfo   = defaultRoutesInfo.ipv6Uc,
       supportIpv6Mc = false,
       defIpv6McRouteLttEntry = defaultRoutesInfo.ipv6Mc,
	   supportFcoe = false,
	   defFcoeNextHopInfo   = defaultRoutesInfo.Fcoe
    }
    ret = wrlCpssDxChIpLpmVirtualRouterAdd(0,vtep1_vtep2_vrf_id,
										   vrConfig.supportIpv4Uc,defaultRoutesInfo.ipv4Uc.lttEntry,
										   vrConfig.supportIpv4Mc,defaultRoutesInfo.ipv4Mc,
										   vrConfig.supportIpv6Uc,defaultRoutesInfo.ipv6Uc.lttEntry,
										   vrConfig.supportIpv6Mc,defaultRoutesInfo.ipv6Mc,
										   vrConfig.supportFcoe,defaultRoutesInfo.fcoe.lttEntry)



    if ((ret ~= 0) and (ret ~= 0x1B --[[GT_ALREADY_EXIST--]]))then
        printLog ("Error in cpssDxChIpLpmVirtualRouterAdd")
        setFailState()
    end

    local ret = myGenWrapper("cpssDxChIpRouterArpAddrWrite",{
        { "IN",   "GT_U8",        "devNum",         devNum},
        { "IN",   "GT_U32",       "routerArpIndex", host2_arp_index},
        { "IN",   "GT_ETHERADDR", "arpMacAddr",     host2_mac }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChIpRouterArpAddrWrite")
        setFailState()
    end
    local ret = myGenWrapper("cpssDxChIpRouterArpAddrWrite",{
        { "IN",   "GT_U8",        "devNum",         devNum},
        { "IN",   "GT_U32",       "routerArpIndex", host3_arp_index},
        { "IN",   "GT_ETHERADDR", "arpMacAddr",     host3_mac }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChIpRouterArpAddrWrite")
        setFailState()
    end
    local ret = myGenWrapper("cpssDxChIpRouterArpAddrWrite",{
        { "IN",   "GT_U8",        "devNum",         devNum},
        { "IN",   "GT_U32",       "routerArpIndex", vtep3_arp_index},
        { "IN",   "GT_ETHERADDR", "arpMacAddr",     vtep3_mac }
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
    routeEntries.entry.regularEntry.nextHopInterface.devPort.portNum = host2_link_port;
    routeEntries.entry.regularEntry.nextHopVlanId = host2_vid;
    routeEntries.entry.regularEntry.nextHopARPPointer = host2_arp_index;
    local ret = myGenWrapper("cpssDxChIpUcRouteEntriesWrite", {
        { "IN",     "GT_U8",                            "devNum",               devNum },
        { "IN",     "GT_U32",                           "baseRouteEntryIndex",  host2_next_hop_index},
        { "IN",     "CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC",  "routeEntriesArray",    routeEntries },
        { "IN",     "GT_U32",                           "numOfRouteEntries",    1}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChIpUcRouteEntriesWrite")
        setFailState()
    end

    routeEntries = table_deep_copy(routeEntriesTemplate);
    routeEntries.entry.regularEntry.nextHopInterface.devPort.portNum = vtep2_vtep3_eport;
    routeEntries.entry.regularEntry.nextHopVlanId = host3_vid;
    routeEntries.entry.regularEntry.nextHopARPPointer = host3_arp_index;
    local ret = myGenWrapper("cpssDxChIpUcRouteEntriesWrite", {
        { "IN",     "GT_U8",                            "devNum",               devNum },
        { "IN",     "GT_U32",                           "baseRouteEntryIndex",  host3_next_hop_index},
        { "IN",     "CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC",  "routeEntriesArray",    routeEntries },
        { "IN",     "GT_U32",                           "numOfRouteEntries",    1}
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChIpUcRouteEntriesWrite")
        setFailState()
    end

    local ret, val = wrLogWrapper(
        "wrlCpssDxChIpLpmIpUcPrefixAdd",
        "(vrf_id, ip_protocol, ip_address, mask_length, next_hop_enrty_index, is_sip_5_)",
        vtep1_vtep2_vrf_id, "CPSS_IP_PROTOCOL_IPV4_E",
        host2_ip, 32, host2_next_hop_index, true)
    if (ret ~= 0) then
        printLog ("Error in wrlCpssDxChIpLpmIpUcPrefixAdd")
        setFailState()
    end
    local ret, val = wrLogWrapper(
        "wrlCpssDxChIpLpmIpUcPrefixAdd",
        "(vrf_id, ip_protocol, ip_address, mask_length, next_hop_enrty_index, is_sip_5_)",
        vtep1_vtep2_vrf_id, "CPSS_IP_PROTOCOL_IPV4_E",
        host3_ip, 32, host3_next_hop_index, true)
    if (ret ~= 0) then
        printLog ("Error in wrlCpssDxChIpLpmIpUcPrefixAdd")
        setFailState()
    end
end

local function configure_target_eport(unset)
    local enable = (not unset);

    if unset then
        return;
    end

    vxlanTsProfileInit(devNum, vxlanTsProfileIndex);

    local tsConfigPtr = {}
    tsConfigPtr.ipv4Cfg = {}
    tsConfigPtr.ipv4Cfg.tagEnable = true
    tsConfigPtr.ipv4Cfg.vlanId = vtep2_vtep3_vid
    tsConfigPtr.ipv4Cfg.macDa = vtep3_mac
    tsConfigPtr.ipv4Cfg.dontFragmentFlag = true
    tsConfigPtr.ipv4Cfg.ttl = tunnel_hdr_ttl
    tsConfigPtr.ipv4Cfg.destIp = vtep3_ip
    tsConfigPtr.ipv4Cfg.srcIp = vtep2_ip
    tsConfigPtr.ipv4Cfg.profileIndex = vxlanTsProfileIndex
    tsConfigPtr.ipv4Cfg.udpSrcPort = tunnel_UDP_Src_Port;
    tsConfigPtr.ipv4Cfg.udpDstPort = VXLAN_UDP_Port
    tsConfigPtr.ipv4Cfg.ipHeaderProtocol = "CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E"

    local ret, val = myGenWrapper("cpssDxChTunnelStartEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "routerArpTunnelStartLineIndex", vtep3_tunnel_index },
        { "IN", "CPSS_TUNNEL_TYPE_ENT", "tunnelType", "CPSS_TUNNEL_GENERIC_IPV4_E" },
        { "IN", "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_ipv4Cfg", "configPtr", tsConfigPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChTunnelStartEntrySet")
        setFailState()
    end

    physicalInfoPtr = {}
    physicalInfoPtr.type = "CPSS_INTERFACE_PORT_E"
    physicalInfoPtr.devPort = {}
    physicalInfoPtr.devPort.devNum = hw_device_id_number
    physicalInfoPtr.devPort.portNum = vtep3_link_port

    ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", vtep2_vtep3_eport },
        { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfoPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgEportToPhysicalPortTargetMappingTableSet")
        setFailState()
    end

    local egressInfoPtr = {}
    egressInfoPtr.tunnelStart = true
    egressInfoPtr.tunnelStartPtr = vtep3_tunnel_index
    egressInfoPtr.tsPassengerPacketType = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
    egressInfoPtr.arpPtr = 0
    egressInfoPtr.modifyMacSa = false
    egressInfoPtr.modifyMacDa = false

    ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_PORT_NUM", "portNum", vtep2_vtep3_eport },
        { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", egressInfoPtr }
    })
    if (ret ~= 0) then
        printLog ("Error in cpssDxChBrgEportToPhysicalPortEgressPortInfoSet")
        setFailState()
    end
end

local function configure(unset)
    configure_vlan(unset)
    configure_tti(unset);
    configure_pcl(unset);
    configure_fdb(unset);
    configure_router(unset);
    configure_target_eport(unset);
end
---------------------------------------------------------------------------------
local function ip2bin(ip)
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

    local values = {
        src = ip2bin(src),
        dst = ip2bin(dst),
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

-- packets
-- host1 to host2
local payload1_to_2 = "11121314"
local sent_passanger_ipv4_host1_to_host2 = ip_hdr_add(
    host1_ip, host2_ip, 64, payload_ip_protocol, payload1_to_2)
local sent_passanger_pkt_host1_to_host2  = mac_hdr_add(
    host1_mac, vtep2_mac, host1_vid, 0x800, sent_passanger_ipv4_host1_to_host2, false);
local sent_pkt_host1_to_host2  = vxlan_tunnel_hdr_add(
    vtep1_mac, vtep2_mac, vtep1_vtep2_vid,
    vtep1_ip, vtep2_ip, tunnel_hdr_ttl, tunnel_UDP_Src_Port, vtep1_vtep2_service_id,
    sent_passanger_pkt_host1_to_host2);
local received_passanger_ipv4_host1_to_host2 = ip_hdr_add(
    host1_ip, host2_ip, 63, payload_ip_protocol, payload1_to_2)
local received_pkt_host1_to_host2  = mac_hdr_add(
    vtep2_mac, host2_mac, host2_vid, 0x800, received_passanger_ipv4_host1_to_host2, true)
-- host1 to host3
local payload1_to_3 = "01020304"
local sent_passanger_ipv4_host1_to_host3 = ip_hdr_add(
    host1_ip, host3_ip, 64, payload_ip_protocol, payload1_to_3)
local sent_passanger_pkt_host1_to_host3  = mac_hdr_add(
    host1_mac, vtep2_mac, host1_vid, 0x800, sent_passanger_ipv4_host1_to_host3, false);
local sent_pkt_host1_to_host3  = vxlan_tunnel_hdr_add(
    vtep1_mac, vtep2_mac, vtep1_vtep2_vid,
    vtep1_ip, vtep2_ip, tunnel_hdr_ttl, tunnel_UDP_Src_Port, vtep1_vtep2_service_id,
    sent_passanger_pkt_host1_to_host3);
local received_passanger_ipv4_host1_to_host3 = ip_hdr_add(
    host1_ip, host3_ip, 63, payload_ip_protocol, payload1_to_3)
local received_passanger_pkt_host1_to_host3  = mac_hdr_add(
    vtep2_mac, host3_mac, host3_vid, 0x800, received_passanger_ipv4_host1_to_host3, false);
local received_pkt_host1_to_host3  = vxlan_tunnel_hdr_add(
    vtep2_mac, vtep3_mac, vtep2_vtep3_vid,
    vtep2_ip, vtep3_ip, tunnel_hdr_ttl, tunnel_UDP_Src_Port, vtep2_vtep3_service_id,
    received_passanger_pkt_host1_to_host3);

---------------------------------------------------------------------------------

--generate vxlan test packets
-- for vlan untagged packet
local function send_and_check_host1_to_host2()
    local transmitInfo;
    local egressInfoTable;
    local rc = 0;
    local my_maskedBytesTable = {
        {startByte = 20, endByte = 21 }-- 2 bytes 'identification field'
        ,{startByte = 26, endByte = 27 }-- 2 bytes 'checksum' in IPV4
    }

    -- from host1 to host3
    transmitInfo =
    {
        devNum = devNum, portNum = vtep1_link_port ,
        pktInfo = {fullPacket = sent_pkt_host1_to_host2}
    }
    egressInfoTable =
    {
        -- expected egress port (apply also mask on needed fields)
        {
            portNum = host2_link_port,
            pktInfo = {fullPacket = received_pkt_host1_to_host2,
                maskedBytesTable = my_maskedBytesTable}
        }
    }
    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        printLog ("Test passed \n")
    else
        printLog ("Test failed \n")
        setFailState()
    end
end

local function send_and_check_host1_to_host3()
    local transmitInfo;
    local egressInfoTable;
    local rc = 0;
    local my_maskedBytesTable = {
        {startByte = 22, endByte = 24 }-- 2 bytes 'identification field' and 1 byte flags
        ,{startByte = 28, endByte = 29 }-- 2 bytes IPV4  'checksum'
        ,{startByte = 44, endByte = 45 }-- 2 bytes UDP 'checksum'
    }

    -- from host1 to host3
    transmitInfo =
    {
        devNum = devNum, portNum = vtep1_link_port ,
        pktInfo = {fullPacket = sent_pkt_host1_to_host3}
    }
    egressInfoTable =
    {
        -- expected egress port (apply also mask on needed fields)
        {
            portNum = vtep3_link_port,
            pktInfo = {fullPacket = received_pkt_host1_to_host3,
                maskedBytesTable = my_maskedBytesTable}
        }
    }
    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        printLog ("Test passed \n")
    else
        printLog ("Test failed \n")
        setFailState()
    end
end

local function test()
    configure(false);
    send_and_check_host1_to_host2()
    send_and_check_host1_to_host3()
    configure(true);
end

-- reinit legacy key compatible UDB configuration
executeStringCliCommands("do shell-execute prvCpssDxChTtiDbTablesInit ${dev}\n")
--

test();

function example_vxlan_routing_config()
    configure(false);
end
function example_vxlan_routing_deconfig()
    configure(true);
end
function example_vxlan_routing_traffic()
    send_and_check_host1_to_host2()
    send_and_check_host1_to_host3()
end
function example_vxlan_routing_set_ports(p1, p2, p3)
    example_vxlan_routing_param_vtep1_link_port = tonumber(p1)
    example_vxlan_routing_param_host2_link_port = tonumber(p2)
    example_vxlan_routing_param_vtep3_link_port = tonumber(p3)
    reinit_local_param();
end
function example_vxlan_routing_set_host_ips(ip1, ip2, ip3)
    example_vxlan_routing_param_host1_ip = ip1
    example_vxlan_routing_param_host2_ip = ip2
    example_vxlan_routing_param_host3_ip = ip3
    reinit_local_param();
end
function example_vxlan_routing_set_host_macs(mac1, mac2, mac3)
    example_vxlan_routing_param_host1_mac = mac1
    example_vxlan_routing_param_host2_mac = mac2
    example_vxlan_routing_param_host3_mac = mac3
    reinit_local_param();
end
function example_vxlan_routing_set_host_vids(vid1, vid2, vid3)
    example_vxlan_routing_param_host1_vid = tonumber(vid1)
    example_vxlan_routing_param_host2_vid = tonumber(vid2)
    example_vxlan_routing_param_host3_vid = tonumber(vid3)
    reinit_local_param();
end
function example_vxlan_routing_set_vtep_ips(ip1, ip2, ip3)
    example_vxlan_routing_param_vtep1_ip = ip1
    example_vxlan_routing_param_vtep2_ip = ip2
    example_vxlan_routing_param_vtep3_ip = ip3
    reinit_local_param();
end
function example_vxlan_routing_set_vtep_macs(mac1, mac2, mac3)
    example_vxlan_routing_param_vtep1_mac = mac1
    example_vxlan_routing_param_vtep2_mac = mac2
    example_vxlan_routing_param_vtep3_mac = mac3
    reinit_local_param();
end
function example_vxlan_routing_set_vtep_vids(vid1, vid2)
    example_vxlan_routing_param_vtep1_vtep2_vid = tonumber(vid1)
    example_vxlan_routing_param_vtep2_vtep3_vid = tonumber(vid2)
    reinit_local_param();
end

function example_vxlan_routing_set_service_ids(service_id1, service_id2)
    example_vxlan_routing_param_vtep1_vtep2_service_id = tonumber(service_id1)
    example_vxlan_routing_param_vtep2_vtep3_service_id = tonumber(service_id2)
    reinit_local_param();
end

function example_vxlan_routing_print()
    print(
        " vtep1_link_port = " .. example_vxlan_routing_param_vtep1_link_port ..
        " host2_link_port = " .. example_vxlan_routing_param_host2_link_port ..
        " vtep3_link_port = " .. example_vxlan_routing_param_vtep3_link_port .. "\n");
    print("update by example_vxlan_routing_set_ports(p1, p2, p3)\n");
    print(
        " host1_ip = " .. example_vxlan_routing_param_host1_ip ..
        " host2_ip = " .. example_vxlan_routing_param_host2_ip ..
        " host3_ip = " .. example_vxlan_routing_param_host3_ip .. "\n");
    print("update by example_vxlan_routing_set_host_ips(ip1, ip2, ip3)\n");
    print(
        " host1_mac = " .. example_vxlan_routing_param_host1_mac ..
        " host2_mac = " .. example_vxlan_routing_param_host2_mac ..
        " host3_mac = " .. example_vxlan_routing_param_host3_mac .. "\n");
    print("update by example_vxlan_routing_set_host_macs(mac1, mac2, mac3)\n");
    print(
        " host1_vid = " .. example_vxlan_routing_param_host1_vid ..
        " host2_vid = " .. example_vxlan_routing_param_host2_vid ..
        " host3_vid = " .. example_vxlan_routing_param_host3_vid .. "\n");
    print("update by example_vxlan_routing_set_host_vids(vid1, vid2, vid3)\n");
    print(
        " vtep1_ip = " .. example_vxlan_routing_param_vtep1_ip ..
        " vtep2_ip = " .. example_vxlan_routing_param_vtep2_ip ..
        " vtep3_ip = " .. example_vxlan_routing_param_vtep3_ip .. "\n");
    print("update by example_vxlan_routing_set_vtep_ips(ip1, ip2, ip3)\n");
    print(
        " vtep1_mac = " .. example_vxlan_routing_param_vtep1_mac ..
        " vtep2_mac = " .. example_vxlan_routing_param_vtep2_mac ..
        " vtep3_mac = " .. example_vxlan_routing_param_vtep3_mac .. "\n");
    print("update by example_vxlan_routing_set_vtep_macs(mac1, mac2, mac3)\n");
    print(
        " vtep1_vtep2_vid = " .. example_vxlan_routing_param_vtep1_vtep2_vid ..
        " vtep2_vtep3_vid = " .. example_vxlan_routing_param_vtep2_vtep3_vid .. "\n");
    print("update by example_vxlan_routing_set_vtep_vids(vid1, vid2)\n");
    print(
        " vtep1_vtep2_service_id = " .. example_vxlan_routing_param_vtep1_vtep2_service_id ..
        " vtep2_vtep3_service_id = " .. example_vxlan_routing_param_vtep2_vtep3_service_id .. "\n");
    print("update by example_vxlan_routing_set_service_ids(service_id1, service_id2)\n");
    print("reset parameters by example_vxlan_routing_param_reset()\n");
    print("configure by example_vxlan_routing_config()\n");
    print("deconfigure by example_vxlan_routing_deconfig()\n");
    print("traffic by example_vxlan_routing_traffic()()\n");
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
example_vxlan_routing_print();
example_vxlan_routing_set_ports(8, 12, 16);
example_vxlan_routing_set_host_ips("172.116.99.1", "172.116.99.2", "172.116.99.3");
example_vxlan_routing_set_host_macs("00:99:99:11:11:11", "00:99:99:22:22:22", "00:99:99:33:33:33");
example_vxlan_routing_set_host_vids(98,99,100);
example_vxlan_routing_set_vtep_ips("10.1.99.1", "10.1.99.2", "10.1.99.3");
example_vxlan_routing_set_vtep_macs("00:99:99:AA:AA:AA", "00:99:99:BB:BB:BB", "00:99:99:CC:CC:CC");
example_vxlan_routing_set_vtep_vids(0x99A, 0x99B);
example_vxlan_routing_print();
example_vxlan_routing_set_service_ids(0x991122, 0x992233);
example_vxlan_routing_print();
.
run vxlan_routing
lua
example_vxlan_routing_config()
--send packets by smartbit and check
example_vxlan_routing_deconfig()
.
--]]
