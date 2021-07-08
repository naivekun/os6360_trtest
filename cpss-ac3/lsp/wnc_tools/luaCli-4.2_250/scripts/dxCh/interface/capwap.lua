--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* capwap.lua
--*
--* DESCRIPTION:
--*       Capwap command implementation
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local debug_on = false
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
    end
end

-- number of capwap channel per physical port
local num_ipv4_capwap_channel_db = {}
local num_ipv6_capwap_channel_db = {}
-- global number of ipv4/ipv6 capwap channels in the system
local global_capwap_channels={}
global_capwap_channels.ipv4 = 0
global_capwap_channels.ipv6 = 0

local hwDevNum = nil

-- flags to specify first capwap call for ipv4/ipv6
local ipv4FirstCapwapCall = true
local ipv4PortLookupEnableGet
local ipv4TtiPacketTypeKeySizeGet
local ipv4TtiPclIdGet

local ipv6FirstCapwapCall = true
local ipv6PortLookupEnableGet
local ipv6TtiPacketTypeKeySizeGet
local ipv6TtiPclIdGet

--used for both set and restore
local udpDstPort_dataTunnel = 5247
local udpDstPort_controlTunnel = 5246
local ttiIndex   = 0
local tunnelStartIndex   = 0
local assignEport = 0
local egressInfoPtrOrig = nil
local templateProfileIndex = 6
local profileDataGet
local macToMeEntryIndex = nil

-- for pcl restore
local ipclIndex = nil
local ipv4LookupCfgGet = nil
local ipv6LookupCfgGet = nil
local lookupCfgTabAccessModeGet = nil

-- capwap reserved pclid for the lookup
local reserved_pclId = 1000

-- capwap reserved ipclid for the tti action
local ipv4_reserved_pcl_config_index = 2345
local ipv6_reserved_pcl_config_index = 2346

local ruleType = "CPSS_DXCH_TTI_RULE_UDB_30_E"
local keySize = "CPSS_DXCH_TTI_KEY_SIZE_30_B_E"
local keyType = "" -- CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E or CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E
local udbArr
local patternPtr
local maskPtr
local actionPtr

local pclRuleType = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"
local pclKeySize = "CPSS_PCL_RULE_SIZE_STD_E"
local pclPacketType = {
                    "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E"       ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E"       ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E"           ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E"  ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E"     ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E" ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_UDE_E"            ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E"       ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E"       ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E"     ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E"           ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E"           ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E"           ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E"           ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E"           ,
                    "CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E"
                }
local pclUdbArr
local pclPatternPtr
local pclMaskPtr
local pclActionPtr={}

-- local defenition for Control Tunnel
local patternControlTunnelPtr
local maskControlTunnelPtr

local ipv4ControlTtiParams ={}
ipv4ControlTtiParams.ipv4ControlTunnelDefined = false
ipv4ControlTtiParams.ipv4ControlTunnelTtiIndex = 0

local ipv6ControlTtiParams ={}
ipv6ControlTtiParams.ipv6ControlTunnelDefined = false
ipv6ControlTtiParams.ipv6ControlTunnelTtiIndex = 0

-- local defenition for Default TTI Rule
local patternDefaultTtiRulePtr
local maskDefaultTtiRulePtr

local ipv4DefaultRuleTtiParams ={}
ipv4DefaultRuleTtiParams.ipv4DefaultTtiRuleDefined = false
ipv4DefaultRuleTtiParams.ipv4DefaultTtiRuleIndex = 0

local ipv6DefaultRuleTtiParams ={}
ipv6DefaultRuleTtiParams.ipv6DefaultTtiRuleDefined = false
ipv6DefaultRuleTtiParams.ipv6DefaultTtiRuleIndex = 0

-- local defenition for Default PCL Rule
local patternDefaultIpclRulePtr
local maskDefaultIpclRulePtr

local defaultIpclRuleParams ={}
defaultIpclRuleParams.defaultIpclRuleDefined = false
defaultIpclRuleParams.defaultIpclRuleIndex = 0

local configPtr
local tunnelType
local tunnelStartStruct

local apiName

local CPSS_NET_FIRST_USER_DEFINED_E = 500

-- define the tables that will hold the 'capwap configuration done in the system'
-- each line in the table hold info about diferent interface (device/port)
-- each interface (device/port) hold next info :
--                  1.protocol - {ipv4|ipv6}
--                  2.devNum - device number of the interface
--                  3.portNum - port number of the interface
--                  4.assign_eport
--                  5.egressInfoPtrOrig - used for restore
--                  6.vlan
--                  6.ap_mac_addr
--                  7.ac_mac_addr
--                  9.ap_ip
--                  10.ac_ip
--                  11.ttiIndex - tti index calculated internally
--                  12.tsndex - tti index calculated internally
--                  13.macToMeEntryIndex - mac2me index calculated internally
--                  14 ipclIndex - ipcl index calculated internally

local entry_format_example = 0
local table_capwap_ipv4_system_info = {
--    [entry_format_example] = {
--        protocol = "ipv4" ,
--        devNum = 128 ,
--        portNum = 18 ,
--        assign_eport = 0xFFF ,
--        egressInfoPtrOrig = 0, --used for restore
--        vlan = 0xFFFF ,
--        ap_mac_addr = "00:00:00:00:00:11" ,
--        ac_mac_addr = "00:00:00:00:00:22" ,
--        ap_ip = "111.1111.111.111" ,
--        ac_ip = "222.222.222.222",
--        ttiIndex=1800,
--        tsIndex=20,
--        macToMeEntryIndex=1,
--        ipclIndex=2
--    }
}

local table_capwap_ipv6_system_info = {
--    [entry_format_example] = {
--        protocol = "ipv6" ,
--        devNum = 128 ,
--        portNum = 58 ,
--        assign_eport = 0xFFF ,
--        egressInfoPtrOrig = 0, --used for restore
--        vlan = 0xFFF ,
--        ap_mac_addr = "00:00:00:00:00:33" ,
--        ac_mac_addr = "00:00:00:00:00:44" ,
--        ap_ip = "3333:0000:0000:0000:0000:0000:0000:3333" ,
--        ac_ip = "4444:0000:0000:0000:0000:0000:0000:4444",
--        ttiIndex=1900,
--        tsIndex=40,
--        macToMeEntryIndex=1,
--        ipclIndex=3
--    }
}
---------------------------------------------------------------------------------------------------
local function protocol_specific_pcl(devNum, portNum, params)

    _debug("Hello, here is params \n "..to_string(params))

    -- this function define pcl parameters to be used in capwap_func_do_port and no_capwap_eport_func
    -- according to the paramts given in the capwap command

    local result
    local command_data = Command_Data()
    ------------------------------------------------------------
    -- PCL Rule configuration
    -- define the UDBs that will be needed for the pcl capwap classification
    --[[ we need next fields:
     <ap-ip> SIP , Tunnel-L3(16B for ipv6 or 4B for ipv4)

        ipv4 - 4 bytes in network order L3 offset 14..17
        0.**> 'L3 offset' byte 14 bits 0..7 : ap-ip[7:0]
        1.**> 'L3 offset' byte 15 bits 0..7 : ap-ip[15:8]
        2.**> 'L3 offset' byte 16 bits 0..7 : ap-ip[23:16]
        3.**> 'L3 offset' byte 17 bits 0..7 : ap-ip[31:24]

        ipv6 - 16 bytes in network order L3 offset 26..41
        0.**> 'L3 offset' byte 10 bits 0..7 : ap-ip[7:0]
        1.**> 'L3 offset' byte 11 bits 0..7 : ap-ip[15:8]
        2.**> 'L3 offset' byte 12 bits 0..7 : ap-ip[23:16]
        3.**> 'L3 offset' byte 13 bits 0..7 : ap-ip[31:24]
        4.**> 'L3 offset' byte 14 bits 0..7 : ap-ip[39:32]
        5.**> 'L3 offset' byte 15 bits 0..7 : ap-ip[47:40]
        6.**> 'L3 offset' byte 16 bits 0..7 : ap-ip[55:48]
        7.**> 'L3 offset' byte 17 bits 0..7 : ap-ip[63:56]
        8.**> 'L3 offset' byte 18 bits 0..7 : ap-ip[71:64]
        9.**> 'L3 offset' byte 19 bits 0..7 : ap-ip[79:72]
        10.**> 'L3 offset' byte 20 bits 0..7 : ap-ip[87:80]
        11.**> 'L3 offset' byte 21 bits 0..7 : ap-ip[95:88]
        12.**> 'L3 offset' byte 22 bits 0..7 : ap-ip[103:96]
        13.**> 'L3 offset' byte 23 bits 0..7 : ap-ip[111:104]
        14.**> 'L3 offset' byte 24 bits 0..7 : ap-ip[119:112]
        15**> 'L3 offset' byte 25 bits 0..7 : ap-ip[127:120]

    total: 16 udbs for ipv6 and 4 udbs for ipv4
    --]]

    local sourcePortVal={}
    sourcePortVal.assignSourcePortEnable=true
    sourcePortVal.sourcePortValue=tonumber(params.assign_eport)

    pclActionPtr.pktCmd       = "CPSS_PACKET_CMD_FORWARD_E"
    pclActionPtr.sourcePort   = sourcePortVal

    if params.protocol=="ipv6" then -- ipv6

        pclUdbArr =
        {    --byte#           offset type                                               offset
            { udbIndex = 0,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 10      }, -- bits 0..7  : ap-ip[0:7]
            { udbIndex = 1,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 11      }, -- bits 0..7  : ap-ip[8:15]
            { udbIndex = 2,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 12      }, -- bits 0..7  : ap-ip[16:23]
            { udbIndex = 3,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 13      }, -- bits 0..7  : ap-ip[24:31]
            { udbIndex = 4,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 14      }, -- bits 0..7  : ap-ip[32:39]
            { udbIndex = 5,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 15      }, -- bits 0..7  : ap-ip[40:47]
            { udbIndex = 6,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 16      }, -- bits 0..7  : ap-ip[48:55]
            { udbIndex = 7,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 17      }, -- bits 0..7  : ap-ip[56:63]
            { udbIndex = 8,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 18      }, -- bits 0..7  : ap-ip[64:71]
            { udbIndex = 9,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 19      }, -- bits 0..7  : ap-ip[72:79]
            { udbIndex = 10,   offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 20      }, -- bits 0..7  : ap-ip[80:87]
            { udbIndex = 11,   offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 21      }, -- bits 0..7  : ap-ip[88:95]
            { udbIndex = 12,   offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 22      }, -- bits 0..7  : ap-ip[96:103]
            { udbIndex = 13,   offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 23      }, -- bits 0..7  : ap-ip[104:111]
            { udbIndex = 14,   offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 24      }, -- bits 0..7  : ap-ip[112:119]
            { udbIndex = 15,   offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 25      }, -- bits 0..7  : ap-ip[120:127]
        }

        -- pclUdbArray
        pclPatternPtr = {ruleIngrStdUdb={
                            commonIngrUdb={pclId=reserved_pclId},
                            udb0_15  = {
                                [0] = bit_and(bit_shr(params.ap_ip.addr[1],8),0xff),    -- bits 0..7   : ap-ip[0:7]
                                [1] = bit_and(params.ap_ip.addr[1],0xff),               -- bits 0..7   : ap-ip[8:15]
                                [2] = bit_and(bit_shr(params.ap_ip.addr[2],8),0xff),    -- bits 0..7   : ap-ip[16:23]
                                [3] = bit_and(params.ap_ip.addr[2],0xff),              -- bits 0..7   : ap-ip[24:31]
                                [4] = bit_and(bit_shr(params.ap_ip.addr[3],8),0xff),   -- bits 0..7   : ap-ip[32:39]
                                [5] = bit_and(params.ap_ip.addr[3],0xff),              -- bits 0..7   : ap-ip[40:47]
                                [6] = bit_and(bit_shr(params.ap_ip.addr[4],8),0xff),   -- bits 0..7   : ap-ip[48:55]
                                [7] = bit_and(params.ap_ip.addr[4],0xff),              -- bits 0..7   : ap-ip[56:63]
                                [8] = bit_and(bit_shr(params.ap_ip.addr[5],8),0xff),   -- bits 0..7   : ap-ip[64:71]
                                [9] = bit_and(params.ap_ip.addr[5],0xff),              -- bits 0..7   : ap-ip[72:79]
                                [10] = bit_and(bit_shr(params.ap_ip.addr[6],8),0xff),   -- bits 0..7   : ap-ip[80:87]
                                [11] = bit_and(params.ap_ip.addr[6],0xff),              -- bits 0..7   : ap-ip[88:95]
                                [12] = bit_and(bit_shr(params.ap_ip.addr[7],8),0xff),   -- bits 0..7   : ap-ip[96:103]
                                [13] = bit_and(params.ap_ip.addr[7],0xff),              -- bits 0..7   : ap-ip[104:111]
                                [14] = bit_and(bit_shr(params.ap_ip.addr[8],8),0xff),   -- bits 0..7   : ap-ip[112:119]
                                [15] = bit_and(params.ap_ip.addr[8],0xff)              -- bits 0..7   : ap-ip[120:127]
                            }
                    }
            }

        -- pclUdbArray
        pclMaskPtr = {ruleIngrStdUdb={
                        commonIngrUdb={pclId=0x3ff},
                        udb0_15  = {
                            [0] = 0xff,    -- bits 0..7   : ap-ip[0:7]
                            [1] = 0xff,    -- bits 0..7   : ap-ip[8:15]
                            [2] = 0xff,    -- bits 0..7   : ap-ip[16:23]
                            [3] = 0xff,    -- bits 0..7   : ap-ip[24:31]
                            [4] = 0xff,    -- bits 0..7   : ap-ip[32:39]
                            [5] = 0xff,    -- bits 0..7   : ap-ip[40:47]
                            [6] = 0xff,    -- bits 0..7   : ap-ip[48:55]
                            [7] = 0xff,    -- bits 0..7   : ap-ip[56:63]
                            [8] = 0xff,    -- bits 0..7   : ap-ip[64:71]
                            [9] = 0xff,    -- bits 0..7   : ap-ip[72:79]
                            [10] = 0xff,   -- bits 0..7   : ap-ip[80:87]
                            [11] = 0xff,   -- bits 0..7   : ap-ip[88:95]
                            [12] = 0xff,   -- bits 0..7   : ap-ip[96:103]
                            [13] = 0xff,   -- bits 0..7   : ap-ip[104:111]
                            [14] = 0xff,   -- bits 0..7   : ap-ip[112:119]
                            [15] = 0xff   -- bits 0..7   : ap-ip[120:127]
                        }
                    }
        }

    elseif params.protocol=="ipv4" then -- ipv4

        pclUdbArr =
        {   --byte#             offset type                                     offset
            { udbIndex = 0,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 14      }, -- bits 0..7  : ap-ip[0:7]
            { udbIndex = 1,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 15      }, -- bits 0..7  : ap-ip[8:15]
            { udbIndex = 2,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 16      }, -- bits 0..7  : ap-ip[16:23
            { udbIndex = 3,    offsetType = "CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E",  offset = 17      }, -- bits 0..7  : ap-ip[24:31]
        }

        -- pclUdbArray
        pclPatternPtr = {ruleIngrStdUdb={
                            commonIngrUdb={pclId=reserved_pclId},
                            udb0_15  = {
                            [0] = bit_and(params.ap_ip[1],0xff),                    -- bits 0..7   : ap-ip[7:0]
                            [1] = bit_and(params.ap_ip[2],0xff),                    -- bits 8..15  : ap-ip[8:15]
                            [2] = bit_and(params.ap_ip[3],0xff),                    -- bits 16..23 : ap-ip[16:23]
                            [3] = bit_and(params.ap_ip[4],0xff),                    -- bits 24..31 : ap-ip[24:31]
                            [4] = 0,
                            [5] = 0,
                            [6] = 0,
                            [7] = 0,
                            [8] = 0,
                            [9] = 0,
                            [10] = 0,
                            [11] = 0,
                            [12] = 0,
                            [13] = 0,
                            [14] = 0,
                            [15] = 0
                        } }}

        -- pclUdbArray
        pclMaskPtr = {ruleIngrStdUdb={
                        commonIngrUdb={pclId=0x3ff},
                        udb0_15  = {
                        [0] = 0xff,    -- bits 0..7   : ap-ip[0:7]
                        [1] = 0xff,    -- bits 0..7   : ap-ip[8:15]
                        [2] = 0xff,    -- bits 0..7   : ap-ip[16:23]
                        [3] = 0xff,    -- bits 0..7   : ap-ip[24:31]
                        [4] = 0,
                        [5] = 0,
                        [6] = 0,
                        [7] = 0,
                        [8] = 0,
                        [9] = 0,
                        [10] = 0,
                        [11] = 0,
                        [12] = 0,
                        [13] = 0,
                        [14] = 0,
                        [15] = 0
                    } }}
    ------------------------------------------------------------
    else --should never happen
        command_data:handleCpssErrorDevPort(4, "wrong ip type given ")
    end

    ------------------------------------------------------------
    -- Default PCL0 Rule configuration
    -- define the UDBs that will be needed for the capwap default PCL0 Rule classification
    --[[ we need next fields:
    pclid = 1000 --> see reserved_pclId
    command = hard drop

    --]]

    if defaultIpclRuleParams.defaultIpclRuleDefined==false then
        -- udbArray
        patternDefaultIpclRulePtr = {ruleIngrStdUdb={
                                        commonIngrUdb={pclId=reserved_pclId},
                                        udb0_15  = {
                                            [0] = 0,
                                            [1] = 0,
                                            [2] = 0,
                                            [3] = 0,
                                            [4] = 0,
                                            [5] = 0,
                                            [6] = 0,
                                            [7] = 0,
                                            [8] = 0,
                                            [9] = 0,
                                            [10] = 0,
                                            [11] = 0,
                                            [12] = 0,
                                            [13] = 0,
                                            [14] = 0,
                                            [15] = 0
                                        } }}

        -- udbArray
        maskDefaultIpclRulePtr = {ruleIngrStdUdb={
                                    commonIngrUdb={pclId=0x3ff},
                                    udb0_15  = {
                                        [0] = 0,
                                        [1] = 0,
                                        [2] = 0,
                                        [3] = 0,
                                        [4] = 0,
                                        [5] = 0,
                                        [6] = 0,
                                        [7] = 0,
                                        [8] = 0,
                                        [9] = 0,
                                        [10] = 0,
                                        [11] = 0,
                                        [12] = 0,
                                        [13] = 0,
                                        [14] = 0,
                                        [15] = 0
                                    } }}
    end
end

---------------------------------------------------------------------------------------------------
local function protocol_specific(devNum, portNum, params)

    _debug("Hello, here is params \n "..to_string(params))

    -- this function define ts and tti parameters to be used in capwap_func_do_port and no_capwap_eport_func
    -- according to the paramts given in the capwap command

    local result
    local command_data = Command_Data()
    local tsConfigStcArr  = {
        tagEnable        = true,
        vlanId           = tonumber(params.vlan_id),
        ipHeaderProtocol = "CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E",
        profileIndex     = templateProfileIndex,--6
        udpDstPort       = udpDstPort_dataTunnel,--5247
        macDa            = params.ap_mac_addr,
        destIp           = params.ap_ip,
        srcIp            = params.ac_ip,
        dontFragmentFlag = false,
        ttl              = 0x22
    }


    ------------------------------------------------------------
    -- Data Tunnel configuration
    -- define the UDBs that will be needed for the capwap classification
    --[[ we need next fields:
    pclid = 1000 --> see reserved_pclId

      0.  ***> metadata byte 22 bits 0..4 --> 5 LSB bits of pclid
      1.  ***> metadata byte 23 bits 0..4 --> 5 MSB bits of pclid

    <Physical port>

      2.  ***> metadata byte 26 bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
      3.  ***> metadata byte 27 bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
          ***> metadata byte 22 bit  5 --> Local Device Source Is Trunk -- will be updated in udbindex=0

    <vlan-id>

      4.  ***> metadata byte 24 bits 0..7 --> eVLAN[7:0]
      5.  ***> metadata byte 25 bits 0..4 --> eVLAN[12:8]

     VLAN Tag0 Exists = 1

      6.  ***> metadata byte 11 bit 4 --> VLAN Tag0 Exists

   MAC2ME = 1

        ***> metadata byte 22 bit 7 --> MAC2ME will be updated in udbindex=0

    <ac-ip> DIP , L3(16B for ipv6 or 4B for ipv4)

        ipv4 - 4 bytes in network order L3 offset 18..21
        7.**> metadata byte 1 bits 0..7 : ac-ip[7:0]
        8.**> metadata byte 2 bits 0..7 : ac-ip[15:8]
        9.**> metadata byte 3 bits 0..7 : ac-ip[23:16]
        10.**> metadata byte 4 bits 0..7 : ac-ip[31:24]

        ipv6 - 16 bytes in network order L3 offset 26..41
        7.**> 'L3 offset' byte 8 bits 0..7 : ac-ip[7:0]
        8.**> 'L3 offset' byte 9 bits 0..7 : ac-ip[15:8]
        9.**> 'L3 offset' byte 10 bits 0..7 : ac-ip[23:16]
        10.**> 'L3 offset' byte 11 bits 0..7 : ac-ip[31:24]
        11.**> 'L3 offset' byte 12 bits 0..7 : ac-ip[39:32]
        12.**> 'L3 offset' byte 13 bits 0..7 : ac-ip[47:40]
        13.**> 'L3 offset' byte 14 bits 0..7 : ac-ip[55:48]
        14.**> 'L3 offset' byte 15 bits 0..7 : ac-ip[63:56]
        15.**> 'L3 offset' byte 16 bits 0..7 : ac-ip[71:64]
        16.**> 'L3 offset' byte 17 bits 0..7 : ac-ip[79:72]
        17.**> 'L3 offset' byte 18 bits 0..7 : ac-ip[87:80]
        18.**> 'L3 offset' byte 19 bits 0..7 : ac-ip[95:88]
        19.**> 'L3 offset' byte 20 bits 0..7 : ac-ip[103:96]
        20.**> 'L3 offset' byte 21 bits 0..7 : ac-ip[111:104]
        21.**> 'L3 offset' byte 22 bits 0..7 : ac-ip[119:112]
        22**> 'L3 offset' byte 23 bits 0..7 : ac-ip[127:120]

    UDPDstPort = 5247 --> see udpDstPort_dataTunnel L4(2B)

        11/23.**> 'L4 offset' byte 2 bits 0..7 : udpDstPort_dataTunnel[15:8]
        12/24.**> 'L4 offset' byte 3 bits 0..7 : udpDstPort_dataTunnel[7:0]

    <Capwap Header> --> check 32 most significant bits of the CAPWAP header: Preamble Type, T bit,
                            F bit, K bit, M bit should be zero - L4 (4B)

        13/25.**> 'L4 offset' byte 8 bits 0..7 : CAPWAP preamble should be 0
        14/26.**> 'L4 offset' byte 9 bits 0..7 : CAPWAP don't care
        15/27.**> 'L4 offset' byte 10 bits 0..7 : CAPWAP T bit should be 0
        16/28.**> 'L4 offset' byte 11 bits 0..7 : CAPWAP F,K,M bits should be 0


    total: 29 udbs for ipv6 and 17 udbs for ipv4
    --]]

    if params.protocol=="ipv6" then -- ipv6
        keyType = "CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E"

        actionPtr = { type2  = {
            tunnelTerminate       = true,
            ttPassengerPacketType = "CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E",
            command               = "CPSS_PACKET_CMD_FORWARD_E",
            bridgeBypass          = false,
            redirectCommand       = "CPSS_DXCH_TTI_NO_REDIRECT_E",
            keepPreviousQoS       = true,
            tunnelStart           = false,
            tunnelStartPtr        = 0,
            tag0VlanCmd           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
            tag1VlanCmd           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
            tag1UpCommand         = "CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E",
            pcl0OverrideConfigIndex = "CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E",
            iPclConfigIndex         = ipv6_reserved_pcl_config_index,
        } }

        udbArr =
        {   --byte#             offset type                                     offset
            -- metadata section
            { udbIndex = 0,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 22      }, -- bits 0..4 --> 5 LSB bits of pclid,
            -- bit 5 --> Local Device Source Is Trunk
            -- bit 7 --> MAC2ME
            { udbIndex = 1,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 23      }, -- bit 0..4 --> 5 MSB bits of pclid
            { udbIndex = 2,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 24      }, -- bits 0..7 --> eVLAN[7:0]
            { udbIndex = 3,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 25      }, -- bits 0..4 --> eVLAN[12:8]
            { udbIndex = 4,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 26      }, -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
            { udbIndex = 5,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 27      }, -- bits 0..4 --> Loca9l Device Source ePort/TrunkID[12:8]
            { udbIndex = 6,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 11      }, -- bits 4 : VLAN Tag0 Exists
            -- offsets in the packet section
            { udbIndex = 7,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",   offset = 26    }, -- bits 0..7  : ac-ip[0:7]
            { udbIndex = 8,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",   offset = 27    }, -- bits 0..7  : ac-ip[8:15]
            { udbIndex = 9,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",   offset = 28   }, -- bits 0..7  : ac-ip[16:23]
            { udbIndex = 10,   offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",   offset = 29   }, -- bits 0..7  : ac-ip[24:31]
            { udbIndex = 11,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",  offset = 30   }, -- bits 0..7  : ac-ip[32:39]
            { udbIndex = 12,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",  offset = 31   }, -- bits 0..7  : ac-ip[40:47]
            { udbIndex = 13,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",  offset = 32   }, -- bits 0..7  : ac-ip[48:55]
            { udbIndex = 14,   offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",   offset = 33   }, -- bits 0..7  : ac-ip[56:63]
            { udbIndex = 15,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",  offset = 34   }, -- bits 0..7  : ac-ip[64:71]
            { udbIndex = 16,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",  offset = 35   }, -- bits 0..7  : ac-ip[72:79]
            { udbIndex = 17,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",  offset = 36   }, -- bits 0..7  : ac-ip[80:87]
            { udbIndex = 18,   offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",   offset = 37   }, -- bits 0..7  : ac-ip[88:95]
            { udbIndex = 19,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",  offset = 38   }, -- bits 0..7  : ac-ip[96:103]
            { udbIndex = 20,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",  offset = 39   }, -- bits 0..7  : ac-ip[104:111]
            { udbIndex = 21,    offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",  offset = 40   }, -- bits 0..7  : ac-ip[112:119]
            { udbIndex = 22,   offsetType = "CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E",   offset = 41   }, -- bits 0..7  : ac-ip[120:127]

            { udbIndex = 23,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 2      }, -- bits 0..7 : udpDstPort_dataTunnel[15:8]
            { udbIndex = 24,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 3     }, --  bits 0..7 : udpDstPort_dataTunnel[7:0]
            { udbIndex = 25,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 8     }, -- bits 0..7 : CAPWAP preamble should be 0
            { udbIndex = 26,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 9     }, -- bits 0..7 : CAPWAP don't care
            { udbIndex = 27,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 10     }, -- bits 0..7 : CAPWAP T bit should be 0
            { udbIndex = 28,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 11     }, -- bits 0..7 : CAPWAP F,K,M bits should be 0
        }

        -- udbArray
        patternPtr = { udbArray  = {
            [0] = bit_or(bit_and(reserved_pclId,0x1f),0x80),        -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk=0, bit 7 --> MAC2ME=1
            [1] = bit_and(bit_shr(reserved_pclId,5),0x1f),          -- bits 0..4 --> 5 MSB bits of pclid
            [2] = bit_and(params.vlan_id,0xff),                     -- bits 0..7 --> eVLAN[7:0]
            [3] =  bit_and(bit_shr(params.vlan_id,8),0x1f),          -- bits 0..4 --> eVLAN[12:8]
            [4] = bit_and(portNum,0xff),                            -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
            [5] = bit_and(bit_shr(portNum,8),0x1f),                 -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
            [6] = 0x10,                                             -- bits 4 : VLAN Tag0 Exists = 1
            -- offsets in the packet section
            [7] = bit_and(bit_shr(params.ac_ip.addr[1],8),0xff),    -- bits 0..7   : ac-ip[0:7]
            [8] = bit_and(params.ac_ip.addr[1],0xff),               -- bits 0..7   : ac-ip[8:15]
            [9] = bit_and(bit_shr(params.ac_ip.addr[2],8),0xff),    -- bits 0..7   : ac-ip[16:23]
            [10] = bit_and(params.ac_ip.addr[2],0xff),              -- bits 0..7   : ac-ip[24:31]
            [11] = bit_and(bit_shr(params.ac_ip.addr[3],8),0xff),   -- bits 0..7   : ac-ip[32:39]
            [12] = bit_and(params.ac_ip.addr[3],0xff),              -- bits 0..7   : ac-ip[40:47]
            [13] = bit_and(bit_shr(params.ac_ip.addr[4],8),0xff),   -- bits 0..7   : ac-ip[48:55]
            [14] = bit_and(params.ac_ip.addr[4],0xff),              -- bits 0..7   : ac-ip[56:63]
            [15] = bit_and(bit_shr(params.ac_ip.addr[5],8),0xff),   -- bits 0..7   : ac-ip[64:71]
            [16] = bit_and(params.ac_ip.addr[5],0xff),              -- bits 0..7   : ac-ip[72:79]
            [17] = bit_and(bit_shr(params.ac_ip.addr[6],8),0xff),   -- bits 0..7   : ac-ip[80:87]
            [18] = bit_and(params.ac_ip.addr[6],0xff),              -- bits 0..7   : ac-ip[88:95]
            [19] = bit_and(bit_shr(params.ac_ip.addr[7],8),0xff),   -- bits 0..7   : ac-ip[96:103]
            [20] = bit_and(params.ac_ip.addr[7],0xff),              -- bits 0..7   : ac-ip[104:111]
            [21] = bit_and(bit_shr(params.ac_ip.addr[8],8),0xff),   -- bits 0..7   : ac-ip[112:119]
            [22] = bit_and(params.ac_ip.addr[8],0xff),              -- bits 0..7   : ac-ip[120:127]

            [23] = bit_and(bit_shr(udpDstPort_dataTunnel,8),0xff),  -- bits 0..7 : udpDstPort_dataTunnel[15:8]
            [24] = bit_and(udpDstPort_dataTunnel,0xff),             -- bits 0..7 : udpDstPort_dataTunnel[7:0]
            [25] = 0,                                               -- bits 0..7 : CAPWAP preamble should be 0
            [26] = 0,                                               -- bits 0..7 : CAPWAP don't care
            [27] = 0,                                               -- bits 0..7 : CAPWAP T bit should be 0
            [28] = 0,                                               -- bits 0..7 : CAPWAP F,K,M bits should be 0
            [29] = 0
        } }

        -- udbArray
        maskPtr = { udbArray  = {
            [0] = 0xbf,   -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk, bit 7 --> MAC2ME
            [1] = 0x1f,   -- bits 0..4 --> 5 MSB bits of pclid
            [2] = 0xff,   -- bits 0..7 --> eVLAN[7:0]
            [3] = 0x1f,   -- bits 0..4 --> eVLAN[12:8]
            [4] = 0xff,   -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
            [5] = 0x1f,   -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
            [6] = 0x10,   -- bit 4 : VLAN Tag0 Exists
            [7] = 0xff,   -- bits 0..7   : ac-ip[7:0]
            [8] = 0xff,   -- bits 8..15  : ac-ip[8:15]
            [9] = 0xff,   -- bits 16..23 : ac-ip[16:23]
            [10] = 0xff,  -- bits 24..31 : ac-ip[24:31]
            [11] = 0xff,  -- bits 0..7   : ac-ip[7:0]
            [12] = 0xff,  -- bits 8..15  : ac-ip[8:15]
            [13] = 0xff,  -- bits 16..23 : ac-ip[16:23]
            [14] = 0xff,  -- bits 24..31 : ac-ip[24:31]
            [15] = 0xff,  -- bits 0..7   : ac-ip[7:0]
            [16] = 0xff,  -- bits 8..15  : ac-ip[8:15]
            [17] = 0xff,  -- bits 16..23 : ac-ip[16:23]
            [18] = 0xff,  -- bits 24..31 : ac-ip[24:31]
            [19] = 0xff,  -- bits 0..7   : ac-ip[7:0]
            [20] = 0xff,  -- bits 8..15  : ac-ip[8:15]
            [21] = 0xff,  -- bits 16..23 : ac-ip[16:23]
            [22] = 0xff,  -- bits 24..31 : ac-ip[24:31]
            [23] = 0xff,  -- bits 0..7 : udpDstPort_dataTunnel[7:0]
            [24] = 0xff,  -- bits 0..7 : udpDstPort_dataTunnel[15:8]
            [25] = 0xff,  -- bits 0..7 : CAPWAP preamble should be 0
            [26] = 0,     -- bits 0..7 : CAPWAP don't care
            [27] = 0x1,   -- bits 0..7 : CAPWAP T bit should be 0
            [28] = 0x98,   -- bits 0..7 : CAPWAP F,K,M bits should be 0
            [29] = 0
        } }

        ------------------------------------------------------------
        -- Control Tunnel configuration
        -- define the UDBs that will be needed for the capwap control tunnel classification
        --[[ we need next fields:
        pclid = 1000 --> see reserved_pclId

            0.**> metadata byte 22 bits 0..4 --> 5 LSB bits of pclid
            1.**> metadata byte 23 bits 0..4 --> 5 MSB bits of pclid

        UDPDstPort = 5246 --> see udpDstPort_controlTunnel L4(2B)

            2.**> 'L4 offset' byte 2 bits 0..7 : udpDstPort_dataTunnel[15:8]
            3.**> 'L4 offset' byte 3 bits 0..7 : udpDstPort_dataTunnel[7:0]

        total: 4 udbs
        --]]

        if ipv6ControlTtiParams.ipv6ControlTunnelDefined==false then
            -- udbArray
            patternControlTunnelPtr = { udbArray  = {
                [0] = bit_and(reserved_pclId,0x1f),             -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk=0, bit 7 --> MAC2ME =0
                [1] = bit_and(bit_shr(reserved_pclId,5),0x1f),  -- bits 0..4 --> 5 MSB bits of pclid
                [2] = 0,                                        -- bits 0..7 --> eVLAN[7:0]
                [3] = 0,                                        -- bits 0..4 --> eVLAN[12:8]
                [4] = 0,                                        -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
                [5] = 0,                                        -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
                [6] = 0,                                        -- bits 4 : VLAN Tag0 Exists

                -- offsets in the packet section
                [7] = 0,                                        -- bits 0..7   : ac-ip[0:7]
                [8] = 0,                                        -- bits 0..7   : ac-ip[8:15]
                [9] = 0,                                        -- bits 0..7   : ac-ip[16:23]
                [10] = 0,                                       -- bits 0..7   : ac-ip[24:31]
                [11] = 0,                                       -- bits 0..7   : ac-ip[32:39]
                [12] = 0,                                       -- bits 0..7   : ac-ip[40:47]
                [13] = 0,                                       -- bits 0..7   : ac-ip[48:55]
                [14] = 0,                                       -- bits 0..7   : ac-ip[56:63]
                [15] = 0,                                       -- bits 0..7   : ac-ip[64:71]
                [16] = 0,                                       -- bits 0..7   : ac-ip[72:79]
                [17] = 0,                                       -- bits 0..7   : ac-ip[80:87]
                [18] = 0,                                       -- bits 0..7   : ac-ip[88:95]
                [19] = 0,                                       -- bits 0..7   : ac-ip[96:103]
                [20] = 0,                                       -- bits 0..7   : ac-ip[104:111]
                [21] = 0,                                       -- bits 0..7   : ac-ip[112:119]
                [22] = 0,                                       -- bits 0..7   : ac-ip[120:127]

                [23] = bit_and(bit_shr(udpDstPort_controlTunnel,8),0xff),  -- bits 0..7 : udpDstPort_dataTunnel[15:8]
                [24] = bit_and(udpDstPort_controlTunnel,0xff),             -- bits 0..7 : udpDstPort_dataTunnel[7:0]
                [25] = 0,                                               -- bits 0..7 : CAPWAP preamble should be 0
                [26] = 0,                                               -- bits 0..7 : CAPWAP don't care
                [27] = 0,                                               -- bits 0..7 : CAPWAP T bit should be 0
                [28] = 0,                                               -- bits 0..7 : CAPWAP F,K,M bits should be 0
                [29] = 0
            } }

            -- udbArray
            maskControlTunnelPtr = { udbArray  = {
                [0] = 0x1f,  -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk=0, bit 7 --> MAC2ME=0
                [1] = 0x1f,  -- bits 0..4 --> 5 MSB bits of pclid
                [2] = 0,                                        -- bits 0..7 --> eVLAN[7:0]
                [3] = 0,                                        -- bits 0..4 --> eVLAN[12:8]
                [4] = 0,                                        -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
                [5] = 0,                                        -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
                [6] = 0,                                        -- bits 4 : VLAN Tag0 Exists

                -- offsets in the packet section
                [7] = 0,                                        -- bits 0..7   : ac-ip[0:7]
                [8] = 0,                                        -- bits 0..7   : ac-ip[8:15]
                [9] = 0,                                        -- bits 0..7   : ac-ip[16:23]
                [10] = 0,                                       -- bits 0..7   : ac-ip[24:31]
                [11] = 0,                                       -- bits 0..7   : ac-ip[32:39]
                [12] = 0,                                       -- bits 0..7   : ac-ip[40:47]
                [13] = 0,                                       -- bits 0..7   : ac-ip[48:55]
                [14] = 0,                                       -- bits 0..7   : ac-ip[56:63]
                [15] = 0,                                       -- bits 0..7   : ac-ip[64:71]
                [16] = 0,                                       -- bits 0..7   : ac-ip[72:79]
                [17] = 0,                                       -- bits 0..7   : ac-ip[80:87]
                [18] = 0,                                       -- bits 0..7   : ac-ip[88:95]
                [19] = 0,                                       -- bits 0..7   : ac-ip[96:103]
                [20] = 0,                                       -- bits 0..7   : ac-ip[104:111]
                [21] = 0,                                       -- bits 0..7   : ac-ip[112:119]
                [22] = 0,                                       -- bits 0..7   : ac-ip[120:127]

                [23] = 0xff,                                    -- bits 0..7 : udpDstPort_dataTunnel[15:8]
                [24] = 0xff,                                    -- bits 0..7 : udpDstPort_dataTunnel[7:0]
                [25] = 0,                                       -- bits 0..7 : CAPWAP preamble should be 0
                [26] = 0,                                       -- bits 0..7 : CAPWAP don't care
                [27] = 0,                                       -- bits 0..7 : CAPWAP T bit should be 0
                [28] = 0,                                       -- bits 0..7 : CAPWAP F,K,M bits should be 0
                [29] = 0
            } }
        end


        if ipv6DefaultRuleTtiParams.ipv6DefaultTtiRuleDefined==false then
            -- Default Data Tunnel configuration
            -- define the UDBs that will be needed for the capwap classification
            --[[ we need next fields:
            pclid = 1000 --> see reserved_pclId

              0.  ***> metadata byte 22 bits 0..4 --> 5 LSB bits of pclid
              1.  ***> metadata byte 23 bits 0..4 --> 5 MSB bits of pclid

            MAC2ME = 1

                ***> metadata byte 22 bit 7 --> MAC2ME will be updated in udbindex=0

            <Capwap Header> --> check 32 most significant bits of the CAPWAP header: Preamble Type, T bit,
                                    F bit, K bit, M bit should be zero - L4 (4B)

                2.**> 'L4 offset' byte 8 bits 0..7 : CAPWAP preamble should be 0
                3.**> 'L4 offset' byte 9 bits 0..7 : CAPWAP don't care
                4.**> 'L4 offset' byte 10 bits 0..7 : CAPWAP T bit should be 0
                5.**> 'L4 offset' byte 11 bits 0..7 : CAPWAP F,K,M bits should be 0


            total: 6 udbs
            --]]

            -- udbArray
            patternDefaultTtiRulePtr = { udbArray  = {
                [0] = bit_or(bit_and(reserved_pclId,0x1f),0x80),        -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk=0, bit 7 --> MAC2ME=1
                [1] = bit_and(bit_shr(reserved_pclId,5),0x1f),  -- bits 0..4 --> 5 MSB bits of pclid
                [2] = 0,                                        -- bits 0..7 --> eVLAN[7:0]
                [3] = 0,                                        -- bits 0..4 --> eVLAN[12:8]
                [4] = 0,                                        -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
                [5] = 0,                                        -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
                [6] = 0,                                        -- bits 4 : VLAN Tag0 Exists

                -- offsets in the packet section
                [7] = 0,                                        -- bits 0..7   : ac-ip[0:7]
                [8] = 0,                                        -- bits 0..7   : ac-ip[8:15]
                [9] = 0,                                        -- bits 0..7   : ac-ip[16:23]
                [10] = 0,                                       -- bits 0..7   : ac-ip[24:31]
                [11] = 0,                                       -- bits 0..7   : ac-ip[32:39]
                [12] = 0,                                       -- bits 0..7   : ac-ip[40:47]
                [13] = 0,                                       -- bits 0..7   : ac-ip[48:55]
                [14] = 0,                                       -- bits 0..7   : ac-ip[56:63]
                [15] = 0,                                       -- bits 0..7   : ac-ip[64:71]
                [16] = 0,                                       -- bits 0..7   : ac-ip[72:79]
                [17] = 0,                                       -- bits 0..7   : ac-ip[80:87]
                [18] = 0,                                       -- bits 0..7   : ac-ip[88:95]
                [19] = 0,                                       -- bits 0..7   : ac-ip[96:103]
                [20] = 0,                                       -- bits 0..7   : ac-ip[104:111]
                [21] = 0,                                       -- bits 0..7   : ac-ip[112:119]
                [22] = 0,                                       -- bits 0..7   : ac-ip[120:127]

                [23] = 0,                                       -- bits 0..7 : udpDstPort_dataTunnel[15:8]
                [24] = 0,                                       -- bits 0..7 : udpDstPort_dataTunnel[7:0]
                [25] = 0,                                       -- bits 0..7 : CAPWAP preamble should be 0
                [26] = 0,                                       -- bits 0..7 : CAPWAP don't care
                [27] = 0,                                       -- bits 0..7 : CAPWAP T bit should be 0
                [28] = 0,                                       -- bits 0..7 : CAPWAP F,K,M bits should be 0
                [29] = 0
            } }

            -- udbArray
            maskDefaultTtiRulePtr = { udbArray  = {
                [0] = 0x9f,   -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk, bit 7 --> MAC2ME=1
                [1] = 0x1f,   -- bits 0..4 --> 5 MSB bits of pclid
                [2] = 0,                                        -- bits 0..7 --> eVLAN[7:0]
                [3] = 0,                                        -- bits 0..4 --> eVLAN[12:8]
                [4] = 0,                                        -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
                [5] = 0,                                        -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
                [6] = 0,                                        -- bits 4 : VLAN Tag0 Exists

                -- offsets in the packet section
                [7] = 0,                                        -- bits 0..7   : ac-ip[0:7]
                [8] = 0,                                        -- bits 0..7   : ac-ip[8:15]
                [9] = 0,                                        -- bits 0..7   : ac-ip[16:23]
                [10] = 0,                                       -- bits 0..7   : ac-ip[24:31]
                [11] = 0,                                       -- bits 0..7   : ac-ip[32:39]
                [12] = 0,                                       -- bits 0..7   : ac-ip[40:47]
                [13] = 0,                                       -- bits 0..7   : ac-ip[48:55]
                [14] = 0,                                       -- bits 0..7   : ac-ip[56:63]
                [15] = 0,                                       -- bits 0..7   : ac-ip[64:71]
                [16] = 0,                                       -- bits 0..7   : ac-ip[72:79]
                [17] = 0,                                       -- bits 0..7   : ac-ip[80:87]
                [18] = 0,                                       -- bits 0..7   : ac-ip[88:95]
                [19] = 0,                                       -- bits 0..7   : ac-ip[96:103]
                [20] = 0,                                       -- bits 0..7   : ac-ip[104:111]
                [21] = 0,                                       -- bits 0..7   : ac-ip[112:119]
                [22] = 0,                                       -- bits 0..7   : ac-ip[120:127]

                [23] = 0,                                       -- bits 0..7 : udpDstPort_dataTunnel[15:8]
                [24] = 0,                                       -- bits 0..7 : udpDstPort_dataTunnel[7:0]
                [25] = 0xff,                                    -- bits 0..7 : CAPWAP preamble should be 0
                [26] = 0,                                       -- bits 0..7 : CAPWAP don't care
                [27] = 0x1,                                     -- bits 0..7 : CAPWAP T bit should be 0
                [28] = 0x98,                                    -- bits 0..7 : CAPWAP F,K,M bits should be 0
                [29] = 0
            } }
        end

        tunnelType = "CPSS_TUNNEL_GENERIC_IPV6_E"
        tunnelStartStruct = "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_ipv6Cfg"
        configPtr = { ipv6Cfg = tsConfigStcArr }

        ------------------------------------------------------------
        -- PCL0 rule declaration

        -- PCL0 default rule

        ------------------------------------------------------------
    elseif params.protocol=="ipv4" then -- ipv4

        keyType = "CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E"

        actionPtr = { type2  = {
            tunnelTerminate       = true,
            ttPassengerPacketType = "CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E",
            command               = "CPSS_PACKET_CMD_FORWARD_E",
            bridgeBypass          = false,
            redirectCommand       = "CPSS_DXCH_TTI_NO_REDIRECT_E",
            keepPreviousQoS       = true,
            tunnelStart           = false,
            tunnelStartPtr        = 0,
            tag0VlanCmd           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
            tag1VlanCmd           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
            tag1UpCommand         = "CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E",
            pcl0OverrideConfigIndex = "CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E",
            iPclConfigIndex         = ipv4_reserved_pcl_config_index,
        } }

        udbArr =
        {   --byte#             offset type                                     offset
            -- metadata section
            { udbIndex = 0,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 22      }, -- bits 0..4 --> 5 LSB bits of pclid,
            -- bit 5 --> Local Device Source Is Trunk
            -- bit 7 --> MAC2ME
            { udbIndex = 1,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 23      }, -- bit 0..4 --> 5 MSB bits of pclid
            { udbIndex = 2,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 24      }, -- bits 0..7 --> eVLAN[7:0]
            { udbIndex = 3,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 25      }, -- bits 0..4 --> eVLAN[12:8]
            { udbIndex = 4,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 26      }, -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
            { udbIndex = 5,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 27      }, -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
            { udbIndex = 6,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 11      }, -- bits 4 : VLAN Tag0 Exists
            { udbIndex = 7,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 1       }, -- bits 0..7   : ac-ip[7:0]
            { udbIndex = 8,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 2       }, -- bits 8..15  : ac-ip[8:15]
            { udbIndex = 9,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 3       }, -- bits 16..23 : ac-ip[16:23]
            { udbIndex = 10,   offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 4       }, -- bits 24..31 : ac-ip[24:31]
            -- offsets in the packet section
            { udbIndex = 11,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 2      }, -- bits 0..7 : udpDstPort_dataTunnel[15:8]
            { udbIndex = 12,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 3     }, --  bits 0..7 : udpDstPort_dataTunnel[7:0]
            { udbIndex = 13,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 8     }, -- bits 0..7 : CAPWAP preamble should be 0
            { udbIndex = 14,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 9     }, -- bits 0..7 : CAPWAP don't care
            { udbIndex = 15,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 10     }, -- bits 0..7 : CAPWAP T bit should be 0
            { udbIndex = 16,   offsetType = "CPSS_DXCH_TTI_OFFSET_L4_E", offset = 11     }, -- bits 0..7 : CAPWAP F,K,M bits should be 0
        }


        -- udbArray
        patternPtr = { udbArray  = {
            [0] = bit_or(bit_and(reserved_pclId,0x1f),0x80),        -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk=0, bit 7 --> MAC2ME=1
            [1] = bit_and(bit_shr(reserved_pclId,5),0x1f),          -- bits 0..4 --> 5 MSB bits of pclid
            [2] = bit_and(params.vlan_id,0xff),                     -- bits 0..7 --> eVLAN[7:0]
            [3] =  bit_and(bit_shr(params.vlan_id,8),0x1f),         -- bits 0..4 --> eVLAN[12:8]
            [4] = bit_and(portNum,0xff),                            -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
            [5] = bit_and(bit_shr(portNum,8),0x1f),                 -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
            [6] = 0x10,                                             -- bit 4 : VLAN Tag0 Exists=1
            [7] = bit_and(params.ac_ip[4],0xff),                    -- bits 0..7   : ac-ip[7:0]
            [8] = bit_and(params.ac_ip[3],0xff),                    -- bits 8..15  : ac-ip[8:15]
            [9] = bit_and(params.ac_ip[2],0xff),                    -- bits 16..23 : ac-ip[16:23]
            [10] = bit_and(params.ac_ip[1],0xff),                   -- bits 24..31 : ac-ip[24:31]
            -- offsets in the packet section
            [11] = bit_and(bit_shr(udpDstPort_dataTunnel,8),0xff),  -- bits 0..7 : udpDstPort_dataTunnel[15:8]
            [12] = bit_and(udpDstPort_dataTunnel,0xff),             -- bits 0..7 : udpDstPort_dataTunnel[7:0]
            [13] = 0,                                               -- bits 0..7 : CAPWAP preamble should be 0
            [14] = 0,                                               -- bits 0..7 : CAPWAP don't care
            [15] = 0,                                               -- bits 0..7 : CAPWAP T bit should be 0
            [16] = 0,                                               -- bits 0..7 : CAPWAP F,K,M bits should be 0
            [17] = 0,
            [18] = 0,
            [19] = 0,
            [20] = 0,
            [21] = 0,
            [22] = 0,
            [23] = 0,
            [24] = 0,
            [25] = 0,
            [26] = 0,
            [27] = 0,
            [28] = 0,
            [29] = 0
        } }

        -- udbArray
        maskPtr = { udbArray  = {
            [0] = 0xbf,   -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk, bit 7 --> MAC2ME
            [1] = 0x1f,   -- bits 0..4 --> 5 MSB bits of pclid
            [2] = 0xff,   -- bits 0..7 --> eVLAN[7:0]
            [3] = 0x1f,   -- bits 0..4 --> eVLAN[12:8]
            [4] = 0xff,   -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
            [5] = 0x1f,   -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
            [6] = 0x10,   -- bit 4 : VLAN Tag0 Exists
            [7] = 0xff,   -- bits 0..7   : ac-ip[7:0]
            [8] = 0xff,   -- bits 8..15  : ac-ip[8:15]
            [9] = 0xff,   -- bits 16..23 : ac-ip[16:23]
            [10] = 0xff,  -- bits 24..31 : ac-ip[24:31]
            [11] = 0xff,  -- bits 0..7 : udpDstPort_dataTunnel[7:0]
            [12] = 0xff,  -- bits 0..7 : udpDstPort_dataTunnel[15:8]
            [13] = 0xff,   -- bits 0..7 : CAPWAP preamble should be 0
            [14] = 0,     -- bits 0..7 : CAPWAP don't care
            [15] = 0x1,   -- bits 0..7 : CAPWAP T bit should be 0
            [16] = 0x98,   -- bits 0..7 : CAPWAP F,K,M bits should be 0
            [17] = 0,
            [18] = 0,
            [19] = 0,
            [20] = 0,
            [21] = 0,
            [22] = 0,
            [23] = 0,
            [24] = 0,
            [25] = 0,
            [26] = 0,
            [27] = 0,
            [28] = 0,
            [29] = 0,
        } }

        ------------------------------------------------------------
        -- Control Tunnel configuration
        -- define the UDBs that will be needed for the capwap control tunnel classification
        --[[ we need next fields:
        pclid = 1000 --> see reserved_pclId

            0.**> metadata byte 22 bits 0..4 --> 5 LSB bits of pclid
            1.**> metadata byte 23 bits 0..4 --> 5 MSB bits of pclid

        UDPDstPort = 5246 --> see udpDstPort_controlTunnel L4(2B)

            2.**> 'L4 offset' byte 2 bits 0..7 : udpDstPort_dataTunnel[15:8]
            3.**> 'L4 offset' byte 3 bits 0..7 : udpDstPort_dataTunnel[7:0]

        total: 4 udbs
        --]]

        if ipv4ControlTtiParams.ipv4ControlTunnelDefined==false then
            -- udbArray
            patternControlTunnelPtr = { udbArray  = {
                [0] = bit_and(reserved_pclId,0x1f),             -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk=0, bit 7 --> MAC2ME=0
                [1] = bit_and(bit_shr(reserved_pclId,5),0x1f),  -- bits 0..4 --> 5 MSB bits of pclid
                [2] = 0,                                        -- bits 0..7 --> eVLAN[7:0]
                [3] = 0,                                        -- bits 0..4 --> eVLAN[12:8]
                [4] = 0,                                        -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
                [5] = 0,                                        -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
                [6] = 0,                                        -- bits 4 : VLAN Tag0 Exists
                [7] = 0,                                        -- bits 0..7   : ac-ip[7:0]
                [8] = 0,                                        -- bits 8..15  : ac-ip[8:15]
                [9] = 0,                                        -- bits 16..23 : ac-ip[16:23]
                [10] = 0,                                       -- bits 24..31 : ac-ip[24:31]
                -- offsets in the packet section
                [11] = bit_and(bit_shr(udpDstPort_controlTunnel,8),0xff),  -- bits 0..7 : udpDstPort_dataTunnel[15:8]
                [12] = bit_and(udpDstPort_controlTunnel,0xff),             -- bits 0..7 : udpDstPort_dataTunnel[7:0]
                [13] = 0,                                               -- bits 0..7 : CAPWAP preamble should be 0
                [14] = 0,                                               -- bits 0..7 : CAPWAP don't care
                [15] = 0,                                               -- bits 0..7 : CAPWAP T bit should be 0
                [16] = 0,                                               -- bits 0..7 : CAPWAP F,K,M bits should be 0
                [17] = 0,
                [18] = 0,
                [19] = 0,
                [20] = 0,
                [21] = 0,
                [22] = 0,
                [23] = 0,
                [24] = 0,
                [25] = 0,
                [26] = 0,
                [27] = 0,
                [28] = 0,
                [29] = 0
            } }

            -- udbArray
            maskControlTunnelPtr = { udbArray  = {
                [0] = 0x1f,     -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk=0, bit 7 --> MAC2ME=0
                [1] = 0x1f,     -- bits 0..4 --> 5 MSB bits of pclid[2] = 0xff,   -- bits 0..7 --> eVLAN[7:0]
                [3] = 0,        -- bits 0..4 --> eVLAN[12:8]
                [4] = 0,        -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
                [5] = 0,        -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
                [6] = 0,        -- bit 4 : VLAN Tag0 Exists
                [7] = 0,        -- bits 0..7   : ac-ip[7:0]
                [8] = 0,        -- bits 8..15  : ac-ip[8:15]
                [9] = 0,        -- bits 16..23 : ac-ip[16:23]
                [10] = 0,       -- bits 24..31 : ac-ip[24:31]
                [11] = 0xff,    -- bits 0..7 : udpDstPort_dataTunnel[7:0]
                [12] = 0xff,    -- bits 0..7 : udpDstPort_dataTunnel[15:8]
                [13] = 0,       -- bits 0..7 : CAPWAP preamble should be 0
                [14] = 0,       -- bits 0..7 : CAPWAP don't care
                [15] = 0,       -- bits 0..7 : CAPWAP T bit should be 0
                [16] = 0,       -- bits 0..7 : CAPWAP F,K,M bits should be 0
                [17] = 0,
                [18] = 0,
                [19] = 0,
                [20] = 0,
                [21] = 0,
                [22] = 0,
                [23] = 0,
                [24] = 0,
                [25] = 0,
                [26] = 0,
                [27] = 0,
                [28] = 0,
                [29] = 0
            } }
        end


        if ipv4DefaultRuleTtiParams.ipv4DefaultTtiRuleDefined==false then
            -- Default Data Tunnel configuration
            -- define the UDBs that will be needed for the capwap classification
            --[[ we need next fields:
            pclid = 1000 --> see reserved_pclId

              0.  ***> metadata byte 22 bits 0..4 --> 5 LSB bits of pclid
              1.  ***> metadata byte 23 bits 0..4 --> 5 MSB bits of pclid

            MAC2ME = 1

                ***> metadata byte 22 bit 7 --> MAC2ME will be updated in udbindex=0

            <Capwap Header> --> check 32 most significant bits of the CAPWAP header: Preamble Type, T bit,
                                    F bit, K bit, M bit should be zero - L4 (4B)

                2.**> 'L4 offset' byte 8 bits 0..7 : CAPWAP preamble should be 0
                3.**> 'L4 offset' byte 9 bits 0..7 : CAPWAP don't care
                4.**> 'L4 offset' byte 10 bits 0..7 : CAPWAP T bit should be 0
                5.**> 'L4 offset' byte 11 bits 0..7 : CAPWAP F,K,M bits should be 0


            total: 6 udbs
            --]]

            -- udbArray
            patternDefaultTtiRulePtr = { udbArray  = {
                [0] = bit_or(bit_and(reserved_pclId,0x1f),0x80),        -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk=0, bit 7 --> MAC2ME=1
                [1] = bit_and(bit_shr(reserved_pclId,5),0x1f),  -- bits 0..4 --> 5 MSB bits of pclid
                [2] = 0,                                        -- bits 0..7 --> eVLAN[7:0]
                [3] = 0,                                        -- bits 0..4 --> eVLAN[12:8]
                [4] = 0,                                        -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
                [5] = 0,                                        -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
                [6] = 0,                                        -- bits 4 : VLAN Tag0 Exists
                [7] = 0,                                        -- bits 0..7   : ac-ip[7:0]
                [8] = 0,                                        -- bits 8..15  : ac-ip[8:15]
                [9] = 0,                                        -- bits 16..23 : ac-ip[16:23]
                [10] = 0,                                       -- bits 24..31 : ac-ip[24:31]
                -- offsets in the packet section
                [11] = 0,                                       -- bits 0..7 : udpDstPort_dataTunnel[15:8]
                [12] = 0,                                       -- bits 0..7 : udpDstPort_dataTunnel[7:0]
                [13] = 0,                                       -- bits 0..7 : CAPWAP preamble should be 0
                [14] = 0,                                       -- bits 0..7 : CAPWAP don't care
                [15] = 0,                                       -- bits 0..7 : CAPWAP T bit should be 0
                [16] = 0,                                       -- bits 0..7 : CAPWAP F,K,M bits should be 0
                [17] = 0,
                [18] = 0,
                [19] = 0,
                [20] = 0,
                [21] = 0,
                [22] = 0,
                [23] = 0,
                [24] = 0,
                [25] = 0,
                [26] = 0,
                [27] = 0,
                [28] = 0,
                [29] = 0
            } }

            -- udbArray
            maskDefaultTtiRulePtr = { udbArray  = {
                [0] = 0x9f,   -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk, bit 7 --> MAC2ME
                [1] = 0x1f,   -- bits 0..4 --> 5 MSB bits of pclid
                [3] = 0,        -- bits 0..4 --> eVLAN[12:8]
                [4] = 0,        -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
                [5] = 0,        -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
                [6] = 0,        -- bit 4 : VLAN Tag0 Exists
                [7] = 0,        -- bits 0..7   : ac-ip[7:0]
                [8] = 0,        -- bits 8..15  : ac-ip[8:15]
                [9] = 0,        -- bits 16..23 : ac-ip[16:23]
                [10] = 0,       -- bits 24..31 : ac-ip[24:31]
                [11] = 0,       -- bits 0..7 : udpDstPort_dataTunnel[7:0]
                [12] = 0,       -- bits 0..7 : udpDstPort_dataTunnel[15:8]
                [13] = 0xff,    -- bits 0..7 : CAPWAP preamble should be 0
                [14] = 0,       -- bits 0..7 : CAPWAP don't care
                [15] = 0x1,     -- bits 0..7 : CAPWAP T bit should be 0
                [16] = 0x98,    -- bits 0..7 : CAPWAP F,K,M bits should be 0
                [17] = 0,
                [18] = 0,
                [19] = 0,
                [20] = 0,
                [21] = 0,
                [22] = 0,
                [23] = 0,
                [24] = 0,
                [25] = 0,
                [26] = 0,
                [27] = 0,
                [28] = 0,
                [29] = 0
            } }
        end

        tunnelType        = "CPSS_TUNNEL_GENERIC_IPV4_E"
        tunnelStartStruct = "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_ipv4Cfg"
        configPtr         = { ipv4Cfg = tsConfigStcArr }
        ------------------------------------------------------------
    else --should never happen
        command_data:handleCpssErrorDevPort(4, "wrong ip type given ")
    end

end

---------------------------------------------------------------------------------------------------
local function capwap_func_do_port(command_data,devNum,portNum, params)

    _debug("Hello capwap_func_do_port, here is params \n "..to_string(params))

    -- this function do all capwap needed configuration per Interface(debNum/portNum),
    -- according to the params given in the capwap command

    local result, val
    local table_capwap_system_info = {}
    local num_capwap_channel_db={}
    local myTsSize
    local myRuleSize
    local myIndexName

    pcl_manager_range_set(getPclRange(devNum)) -- getPclRange return two values

    -- check that the channel was not already defined
    -- if the channel defined in a nother ipType need to do "no capwap assign-ePort ePort" to delete it and reconfigure
    -- if the channel defined in the same ipType can update it
    if params.protocol=="ipv6" then -- ipv6
        -- check that the name was not defined for ipv4
        myTsSize = 2
        table_capwap_system_info = table_capwap_ipv6_system_info
        num_capwap_channel_db = num_ipv6_capwap_channel_db
        if table_capwap_ipv4_system_info[params.assign_eport]~=nil then
            print("invalid ePort already used for ipv4 channel")
            print(" ")
            return
        end
    elseif params.protocol=="ipv4" then -- ipv4
        -- check that the name was not defined for ipv6
        myTsSize = 1
        table_capwap_system_info = table_capwap_ipv4_system_info
        num_capwap_channel_db = num_ipv4_capwap_channel_db
        if table_capwap_ipv6_system_info[params.assign_eport]~=nil then
            print("invalid ePort already used for ipv6 channel")
            print(" ")
            return
        end
    else --should never happen
        command_data:handleCpssErrorDevPort(4, "wrong ip type given ")
    end

    protocol_specific(devNum, portNum, params)
    protocol_specific_pcl(devNum, portNum, params)

   ------------------------------------------------------------------------
--  Set a TTI MacToMe entry with {ac-mac-addr}{vlan_id}{assign_eport}
--  save values for restore in DB

    apiName = "device_to_hardware_format_convert"
    result, hwDevNum = device_to_hardware_format_convert(devNum)
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)


    myIndexName = "<capwap:> assign-eport ".. params.assign_eport
    _debug("myIndexName="..myIndexName)
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    macToMeEntryIndex = allocMac2MeIndexFromPool(myIndexName,false)
    _debug("macToMeEntryIndex="..macToMeEntryIndex)

    local macToMeValue  = {}
    macToMeValue.mac    = params.ac_mac_addr
    macToMeValue.vlanId = tonumber(params.vlan_id)

    local macToMeMask  = {}
    macToMeMask. mac   = "FF:FF:FF:FF:FF:FF"
    macToMeMask.vlanId = 0xfff

    local macToMeSrcInterfaceInfo = {}
    macToMeSrcInterfaceInfo.includeSrcInterface = "CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E"
    macToMeSrcInterfaceInfo.srcDevice  = hwDevNum
    macToMeSrcInterfaceInfo.srcIsTrunk = false
    macToMeSrcInterfaceInfo.srcPortTrunk =portNum

    apiName = "cpssDxChTtiMacToMeSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                            "devNum",         devNum },
        { "IN",     "GT_U32",                                           "entryIndex",     macToMeEntryIndex},
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",                       "valuePtr",       macToMeValue},
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",                       "maskPtr",        macToMeMask},
        { "IN",     "CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC",   "interfaceInfoPtr",macToMeSrcInterfaceInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------
    --if this is a first call to capwap configuration then need to keep global values for restore
    if params.protocol=="ipv6" then -- ipv6
        if ipv6firstCapwapCall==true then
            apiName = "cpssDxChTtiPortLookupEnableGet"
            result, val  = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                        "devNum",   devNum },
                { "IN",     "GT_PORT_NUM",                  "portNum",  portNum},
                { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",   "keyType",  keyType},
                { "OUT",     "GT_BOOL",                      "enable"},
            })
            ipv6PortLookupEnableGet  = val["enable"]
            command_data:handleCpssErrorDevPort(result, apiName)
            _debug(apiName .. "   result = " .. result)

            apiName = "cpssDxChTtiPacketTypeKeySizeGet"
            result, val  = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                        "devNum",   devNum },
                { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",   "keyType",  keyType},
                { "OUT",    "CPSS_DXCH_TTI_KEY_SIZE_ENT",   "keySize"},
            })
            ipv6TtiPacketTypeKeySizeGet =  val["keySize"]
            command_data:handleCpssErrorDevPort(result, apiName)
            _debug(apiName .. "   result = " .. result)

            apiName = "cpssDxChTtiPclIdGet"
            result, val  = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                         "devNum",     devNum },
                { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    keyType},
                { "IN",     "GT_U32",                        "pclId" },
            })
            ipv6TtiPclIdGet =  val["pclId"]
            command_data:handleCpssErrorDevPort(result, apiName)
            _debug(apiName .. "   result = " .. result)
        end
    elseif params.protocol=="ipv4" then -- ipv4
        if ipv4firstCapwapCall==true then
            apiName = "cpssDxChTtiPortLookupEnableGet"
            result, val  = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                        "devNum",   devNum },
                { "IN",     "GT_PORT_NUM",                  "portNum",  portNum},
                { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",   "keyType",  keyType},
                { "OUT",     "GT_BOOL",                      "enable"},
            })
            ipv4PortLookupEnableGet  = val["enable"]
            command_data:handleCpssErrorDevPort(result, apiName)
            _debug(apiName .. "   result = " .. result)

            apiName = "cpssDxChTtiPacketTypeKeySizeGet"
            result, val  = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                        "devNum",   devNum },
                { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",   "keyType",  keyType},
                { "OUT",     "CPSS_DXCH_TTI_KEY_SIZE_ENT",  "keySize"},
            })
            ipv4TtiPacketTypeKeySizeGet =  val["keySize"]
            command_data:handleCpssErrorDevPort(result, apiName)
            _debug(apiName .. "   result = " .. result)

            apiName = "cpssDxChTtiPclIdGet"
            result, val  = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                         "devNum",     devNum },
                { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    keyType},
                { "IN",     "GT_U32",                        "pclId" },
            })
            ipv4TtiPclIdGet =  val["pclId"]
            command_data:handleCpssErrorDevPort(result, apiName)
            _debug(apiName .. "   result = " .. result)
        end
    else --should never happen
        command_data:handleCpssErrorDevPort(4, "wrong ip type given ")
    end

    if ipv4firstCapwapCall==true and ipv6firstCapwapCall==true then
        apiName = "cpssDxChTunnelStartGenProfileTableEntryGet"
        result,val = myGenWrapper(apiName, {
            { "IN",     "GT_U8",                                     "devNum",         devNum },
            { "IN",     "GT_U32",                                    "profileIndex",   templateProfileIndex },
            { "OUT",     "CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC", "profileDataPtr"},
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        profileDataGet = val["profileDataPtr"]
    end

    apiName = "cpssDxChTtiPortLookupEnableSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",   devNum },
        { "IN",     "GT_PORT_NUM",                  "portNum",  portNum},
        { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",   "keyType",  keyType},
        { "IN",     "GT_BOOL",                      "enable",   true   },
                                                   })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------
    apiName = "cpssDxChTtiPacketTypeKeySizeSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",   devNum },
        { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",   "keyType",  keyType},
        { "IN",     "CPSS_DXCH_TTI_KEY_SIZE_ENT",   "keySize",  keySize},
                                                   })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------
    apiName = "cpssDxChTtiPclIdSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                         "devNum",     devNum },
        { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    keyType},
        { "IN",     "GT_U32",                        "pclId",      reserved_pclId},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------
    -- configure the UDBs needed for the match
    apiName = "cpssDxChTtiUserDefinedByteSet"
    for dummy,udbEntry in pairs(udbArr) do
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                         "devNum",     devNum },
            { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    keyType},
            { "IN",     "GT_U32",                        "udbIndex",   udbEntry.udbIndex },
            { "IN",     "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", udbEntry.offsetType },
            { "IN",     "GT_U8",                         "offset",     udbEntry.offset }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    -- if this is the first call to capwap command need to define a Default TTI Rule
    if (params.protocol=="ipv6" and ipv6DefaultRuleTtiParams.ipv6DefaultTtiRuleDefined==false) or (params.protocol=="ipv4" and ipv4DefaultRuleTtiParams.ipv4DefaultTtiRuleDefined==false) then
        if patternDefaultTtiRulePtr~=nil and maskDefaultTtiRulePtr~=nil then

            local actionDefaultTtiRulePtr = { type2  = {
                command               = "CPSS_PACKET_CMD_DROP_HARD_E",
                tag1VlanCmd           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
            } }

            myRuleSize = 3
            myIndexName = "<capwap default tunnel:>  assign-eport ".. params.assign_eport
            _debug("myIndexName="..myIndexName)
            -- if myIndexName not exists in DB it will give 'new index' and saved into DB
            -- if myIndexName exists in DB it will retrieved from DB
           local defaultTtiRuleIndex = allocTtiIndexFromPool_fromEnd(myRuleSize,myIndexName,"tti0")
            _debug("defaultTtiRuleIndex="..defaultTtiRuleIndex)

            apiName = "cpssDxChTtiRuleSet"
            result = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                           "devNum",     devNum },
                { "IN",     "GT_U32",                          "index",      defaultTtiRuleIndex},
                { "IN",     "CPSS_DXCH_TTI_RULE_TYPE_ENT",     "ruleType",   ruleType},
                { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "patternPtr", patternDefaultTtiRulePtr},
                { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "maskPtr",    maskDefaultTtiRulePtr},
                { "IN",     "CPSS_DXCH_TTI_ACTION_TYPE_ENT",   "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"},
                { "IN",     "CPSS_DXCH_TTI_ACTION_UNT_type2",  "actionPtr",  actionDefaultTtiRulePtr},
            })

            command_data:handleCpssErrorDevPort(result, apiName)
            _debug(apiName .. "   result = " .. result)

            if params.protocol=="ipv6" then
                ipv6DefaultRuleTtiParams.ipv6DefaultTtiRuleIndex = defaultTtiRuleIndex
                ipv6DefaultRuleTtiParams.ipv6DefaultTtiRuleDefined=true
            elseif params.protocol=="ipv4" then
                ipv4DefaultRuleTtiParams.ipv4DefaultTtiRuleIndex = defaultTtiRuleIndex
                ipv4DefaultRuleTtiParams.ipv4DefaultTtiRuleDefined=true
            else --should never happen
                command_data:handleCpssErrorDevPort(4, "wrong ip type given ")
            end
        end
    end

    -- if this is the first call to capwap command need to define a Control Tunnel Rule
    if (params.protocol=="ipv6" and ipv6ControlTtiParams.ipv6ControlTunnelDefined==false) or (params.protocol=="ipv4" and ipv4ControlTtiParams.ipv4ControlTunnelDefined==false) then
        if patternControlTunnelPtr~=nil and maskControlTunnelPtr~=nil then
            local actionControlTunnelPtr = { type2  = {
                command               = "CPSS_PACKET_CMD_TRAP_TO_CPU_E",
                tag1VlanCmd           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
                userDefinedCpuCode    = CPSS_NET_FIRST_USER_DEFINED_E+1
            } }

            myRuleSize = 3
            myIndexName = "<capwap control tunnel:> assign-eport ".. params.assign_eport
            _debug("myIndexName="..myIndexName)
            -- if myIndexName not exists in DB it will give 'new index' and saved into DB
            -- if myIndexName exists in DB it will retrieved from DB
            local controlTunnelTtiIndex = allocTtiIndexFromPool_fromEnd(myRuleSize,myIndexName,"tti0")
            _debug("controlTunnelTtiIndex="..controlTunnelTtiIndex)

            apiName = "cpssDxChTtiRuleSet"
            result = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                           "devNum",     devNum },
                { "IN",     "GT_U32",                          "index",      controlTunnelTtiIndex},
                { "IN",     "CPSS_DXCH_TTI_RULE_TYPE_ENT",     "ruleType",   ruleType},
                { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "patternPtr", patternControlTunnelPtr},
                { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "maskPtr",    maskControlTunnelPtr},
                { "IN",     "CPSS_DXCH_TTI_ACTION_TYPE_ENT",   "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"},
                { "IN",     "CPSS_DXCH_TTI_ACTION_UNT_type2",  "actionPtr",  actionControlTunnelPtr},
            })

            command_data:handleCpssErrorDevPort(result, apiName)
            _debug(apiName .. "   result = " .. result)

            if params.protocol=="ipv6" then
                ipv6ControlTtiParams.ipv6ControlTunnelTtiIndex = controlTunnelTtiIndex
                ipv6ControlTtiParams.ipv6ControlTunnelDefined=true
            elseif params.protocol=="ipv4" then
                ipv4ControlTtiParams.ipv4ControlTunnelTtiIndex = controlTunnelTtiIndex
                ipv4ControlTtiParams.ipv4ControlTunnelDefined=true
            else --should never happen
                command_data:handleCpssErrorDevPort(4, "wrong ip type given ")
            end
        end
    end

    ------------------------------------------------------------------------
    myRuleSize = 3
    myIndexName = "<capwap:> assign-eport ".. params.assign_eport
    _debug("myIndexName="..myIndexName)
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti0")
    _debug("ttiIndex="..ttiIndex)

    apiName = "cpssDxChTtiRuleSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                           "devNum",     devNum },
        { "IN",     "GT_U32",                          "index",      ttiIndex  },
        { "IN",     "CPSS_DXCH_TTI_RULE_TYPE_ENT",     "ruleType",   ruleType},
        { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "patternPtr", patternPtr},
        { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "maskPtr",    maskPtr},
        { "IN",     "CPSS_DXCH_TTI_ACTION_TYPE_ENT",   "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"},
        { "IN",     "CPSS_DXCH_TTI_ACTION_UNT_type2",  "actionPtr",  actionPtr},
                                        })
    command_data:handleCpssErrorDevPort(result, apiName)

    ------------------------------------------------------------------------
    -- PCL configuration
    ------------------------------------------------------------------------
    apiName = "cpssDxChPclPortIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_PORT_NUM",   "portNum",       portNum},
        { "IN",     "GT_BOOL",       "enable",        true },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    for index,entry in pairs(pclPacketType) do
        -- configure the UDBs needed for the match
        apiName = "cpssDxChPclUserDefinedByteSet"
        for dummy,udbEntry in pairs(pclUdbArr) do
            result = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",                                "devNum",     devNum },
                { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
                { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", entry},
                { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",    "CPSS_PCL_DIRECTION_INGRESS_E"},
                { "IN",     "GT_U32",                       "udbIndex",   udbEntry.udbIndex },
                { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT","offsetType", udbEntry.offsetType },
                { "IN",     "GT_U8",                        "offset",     udbEntry.offset }
            })
            command_data:handleCpssErrorDevPort(result, apiName.."()")
            _debug(apiName .. "   result = " .. result)
        end
    end

    -- if this is the first call to capwap command need to define a Default TTI Rule
    if defaultIpclRuleParams.defaultIpclRuleDefined==false then
        if patternDefaultIpclRulePtr~=nil and maskDefaultIpclRulePtr~=nil then

            local mirror_struct = {
                cpuCode = "CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E"
            }

            local actionDefaultIpclRulePtr = {
                pktCmd       = "CPSS_PACKET_CMD_DROP_HARD_E",
                mirror       = mirror_struct
            }

            myRuleSize = 3
            myIndexName = "<capwap default ingress Pcl0 rule:>  assign-eport ".. params.assign_eport
            _debug("myIndexName="..myIndexName)
            -- if myIndexName not exists in DB it will give 'new index' and saved into DB
            -- if myIndexName exists in DB it will retrieved from DB
            local defaultIpclRuleIndex = allocPclIndexFromPool_fromEnd(myRuleSize,myIndexName)
            _debug("defaultIpclRuleIndex="..defaultIpclRuleIndex)

            apiName = "cpssDxChPclRuleSet"
            result = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                                "devNum",           devNum },
                { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat",       pclRuleType},
                { "IN",     "GT_U32",                               "ruleIndex",        defaultIpclRuleIndex },
                { "IN",     "CPSS_DXCH_PCL_RULE_OPTION_ENT",        "ruleOptionsBmp",   0},
                { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb","patternPtr",patternDefaultIpclRulePtr},
                { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb","maskPtr",   maskDefaultIpclRulePtr},
                { "IN",     "CPSS_DXCH_PCL_ACTION_STC",             "actionPtr",        actionDefaultIpclRulePtr}})
            command_data:handleCpssErrorDevPort(result, apiName)
            _debug(apiName .. "   result = " .. result)
            defaultIpclRuleParams.defaultIpclRuleIndex = defaultIpclRuleIndex
            defaultIpclRuleParams.defaultIpclRuleDefined=true
        end
    end

    myRuleSize = 3
    myIndexName = "<capwap Ingress PCL0 Rule:> assign-eport ".. params.assign_eport
    _debug("myIndexName="..myIndexName)
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    ipclIndex = allocPclIndexFromPool(myRuleSize,myIndexName,false)

    _debug("ipclIndex="..ipclIndex)

    apiName = "cpssDxChPclRuleSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                "devNum",            devNum },
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat",        pclRuleType},
        { "IN",     "GT_U32",                               "ruleIndex",         ipclIndex },
        { "IN",     "CPSS_DXCH_PCL_RULE_OPTION_ENT",        "ruleOptionsBmp",    0},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb", "patternPtr",pclPatternPtr},
        { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_UNT_ruleIngrStdUdb", "maskPtr",   pclMaskPtr},
        { "IN",     "CPSS_DXCH_PCL_ACTION_STC",             "actionPtr",         pclActionPtr},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    local interfaceInfo = {}
    interfaceInfo.type = "CPSS_INTERFACE_INDEX_E"
    if params.protocol=="ipv6"then
        interfaceInfo.index = ipv6_reserved_pcl_config_index
    else
        interfaceInfo.index = ipv4_reserved_pcl_config_index
    end

    local groupKeyTypes={}
    groupKeyTypes.nonIpKey = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"
    groupKeyTypes.ipv4Key = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"
    groupKeyTypes.ipv6Key = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"

    local lookupCfg={}
    lookupCfg.enableLookup=true
    lookupCfg.pclId = reserved_pclId
    lookupCfg.groupKeyTypes=groupKeyTypes


    apiName = "cpssDxChPclCfgTblGet"
    result,val = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",               devNum },
        { "IN",     "CPSS_INTERFACE_INFO_STC",      "interfaceInfoPtr",     interfaceInfo},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",            "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",            "CPSS_PCL_LOOKUP_0_E"},
        { "OUT",     "CPSS_DXCH_PCL_LOOKUP_CFG_STC", "lookupCfgPtr"},
    })
    if params.protocol=="ipv6"then
        ipv6LookupCfgGet=val["lookupCfgPtr"]
    else
        ipv4LookupCfgGet=val["lookupCfgPtr"]
    end

    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    apiName = "cpssDxChPclCfgTblSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",               devNum },
        { "IN",     "CPSS_INTERFACE_INFO_STC",      "interfaceInfoPtr",     interfaceInfo},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",            "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",            "CPSS_PCL_LOOKUP_0_E"},
        { "IN",     "CPSS_DXCH_PCL_LOOKUP_CFG_STC", "lookupCfgPtr",         lookupCfg},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    apiName = "cpssDxChPclIngressPolicyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",        "devNum",     devNum},
        { "IN",     "GT_BOOL",      "enable",     true},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    apiName = "cpssDxChPclPortLookupCfgTabAccessModeGet"
    result,val = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",       devNum},
        { "IN",     "GT_PORT_NUM",                  "portNum",      portNum},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",    "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",    "CPSS_PCL_LOOKUP_0_E"},
        { "IN",     "GT_U32",                       "subLookupNum", 0},
        { "OUT",     "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT",  "mode"},
    })
    lookupCfgTabAccessModeGet = val["mode"]
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    apiName = "cpssDxChPclPortLookupCfgTabAccessModeSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                        "devNum",       devNum},
        { "IN",     "GT_PORT_NUM",                  "portNum",      portNum},
        { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",    "CPSS_PCL_DIRECTION_INGRESS_E"},
        { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",    "CPSS_PCL_LOOKUP_0_E"},
        { "IN",     "GT_U32",                       "subLookupNum", 0},
        { "IN",     "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT",  "mode", "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_BY_PORT_E"},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    apiName = "cpssDxCh3PclTunnelTermForceVlanModeEnableSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                    "devNum",      devNum},
        { "IN",     "GT_BOOL",                  "enable",      false},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    apiName = "appDemoDxChNewPclTcamSupportSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_BOOL",                  "enableIndexConversion",      false},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------

    myIndexName = "<capwap:> assign-eport ".. params.assign_eport
    _debug("myIndexName="..myIndexName)
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    tunnelStartIndex = allocTsIndexFromPool(myTsSize,myIndexName,false)

    _debug("tunnelStartIndex="..tunnelStartIndex)

    apiName = "cpssDxChTunnelStartEntrySet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                 "devNum",                        devNum },
        { "IN",     "GT_U32",                "routerArpTunnelStartLineIndex", tunnelStartIndex},
        { "IN",     "CPSS_TUNNEL_TYPE_ENT",  "tunnelType",                    tunnelType},
        { "IN",     tunnelStartStruct,       "configPtr",                     configPtr},
                                                        })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)
    ------------------------------------------------------------------------
    local profileDataPtr = {
        templateDataSize =  "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E",
        udpSrcPortMode =  "CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E",
    }

    apiName = "cpssDxChTunnelStartGenProfileTableEntrySet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                                     "devNum",         devNum },
        { "IN",     "GT_U32",                                    "profileIndex",   templateProfileIndex },
        { "IN",     "CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC", "profileDataPtr", profileDataPtr},
                                        })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    --------------------------
    assignEport = tonumber(params.assign_eport)

    local egressInfo = {}
    egressInfo.tunnelStart = true
    egressInfo.tunnelStartPtr = tunnelStartIndex
    egressInfo.tsPassengerPacketType  = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
    egressInfo.arpPtr = 0
    egressInfo.modifyMacSa = false
    egressInfo.modifyMacDa = false

    apiName = "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum      },
        { "IN",     "GT_PORT_NUM",             "portNum",         assignEport }, -- ePort
        { "IN",     "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", egressInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    local physicalInfo = {}
    physicalInfo.type = "CPSS_INTERFACE_PORT_E"
    physicalInfo.devPort = {}
    physicalInfo.devPort.devNum  = hwDevNum
    physicalInfo.devPort.portNum = portNum

    _debug(to_string(physicalInfo))

    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
    result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",                   "devNum",          devNum      },
            { "IN",     "GT_PORT_NUM",             "portNum",         assignEport }, -- ePort
            { "IN",     "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfo},
                                        })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- Enable Bridge Control Learning on the ingress port
    apiName = "cpssDxChBrgFdbNaToCpuPerPortSet"
    result = cpssPerPortParamSet(
        apiName,
        devNum, portNum, true, "enable",
        "GT_BOOL")
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    apiName = "cpssDxChBrgFdbPortLearnStatusSet"
    result = myGenWrapper(
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", portNum},
        { "IN", "GT_BOOL", "status", false}, -- control learning
        { "IN", "CPSS_PORT_LOCK_CMD_ENT", "cmd", "CPSS_LOCK_FRWRD_E"}
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------
    apiName = "cpssDxChIpRouterMacSaModifyEnable"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                "devNum",      devNum          },
        { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",     portNum         }, -- phyPort as index
        { "IN",     "GT_BOOL",              "enable",      true},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    apiName = "cpssDxChIpPortRouterMacSaLsbModeSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                     "devNum",      devNum          },
        { "IN",     "GT_PHYSICAL_PORT_NUM",      "portNum",     portNum         }, -- phyPort as index
        { "IN",     "CPSS_MAC_SA_LSB_MODE_ENT",  "saLsbMode",   "CPSS_SA_LSB_FULL_48_BIT_GLOBAL"},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    local macSaIndex = portNum

    lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,macSaIndex,params.ac_mac_addr)

    ------------------------------------------------------------------------

    lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,assignEport,macSaIndex)

    ------------------------------------------------------------------------

    apiName = "cpssDxChBrgVlanEgressPortTagStateModeSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",           "devNum",           devNum     },
        { "IN",     "GT_PORT_NUM",     "portNum",          assignEport}, -- ePort
        { "IN",     "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT", "stateMode", "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E"},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------

    apiName = "cpssDxChBrgVlanEgressPortTagStateSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",           "devNum",           devNum     },
        { "IN",     "GT_PORT_NUM",     "portNum",          assignEport}, -- ePort
        { "IN",     "CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT", "tagCmd", "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E"},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------

    --save orig info
    apiName = "cpssDxChBrgEportToPhysicalPortEgressPortInfoGet"
    result, val = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                              "devNum",        devNum            },
        { "IN",     "GT_PORT_NUM",                        "portNum",       assignEport       }, --ePort
        { "OUT",    "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr" },
                                                })
    egressInfoPtrOrig= val["egressInfoPtr"]
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    local egressInfoPtr = {
                        tunnelStart           = true,
                        tunnelStartPtr        = tunnelStartIndex,
                        tsPassengerPacketType = "CPSS_DXCH_TTI_PASSENGER_ETHERNET_E",
                    }

    apiName = "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                               "devNum",        devNum     },
        { "IN",     "GT_PORT_NUM",                         "portNum",       assignEport}, -- ePort
        { "IN",     "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC",  "egressInfoPtr", egressInfoPtr },
                                                })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- operation succed save the info into DB
    local entryInfo = {}
    entryInfo["protocol"] = params.protocol
    entryInfo["devNum"] = devNum
    entryInfo["portNum"] = portNum
    entryInfo["assign_eport"] = assignEport
    entryInfo["egressInfoPtrOrig"] =  egressInfoPtrOrig -- keept for restore
    entryInfo["vlan"] =  params.vlan_id
    entryInfo["ap_mac_addr"] =params.ap_mac_addr
    entryInfo["ac_mac_addr"] = params.ac_mac_addr
    entryInfo["ap_ip"] =  params.ap_ip
    entryInfo["ac_ip"] =  params.ac_ip
    entryInfo["ttiIndex"] =  ttiIndex
    entryInfo["tsIndex"] =  tunnelStartIndex
    entryInfo["macToMeEntryIndex"] =  macToMeEntryIndex
    entryInfo["ipclIndex"] =  ipclIndex

    -- use the assign_eport value as a key to the table
    table_capwap_system_info[params.assign_eport] = entryInfo

    if params.protocol=="ipv6"then
        global_capwap_channels.ipv6 = global_capwap_channels.ipv6 + 1
    else
        global_capwap_channels.ipv4 = global_capwap_channels.ipv4 + 1
    end

    if num_capwap_channel_db[portNum] == nil then
        num_capwap_channel_db[portNum] = 1
    else
        num_capwap_channel_db[portNum] = num_capwap_channel_db[portNum] + 1
    end
    return true
end

local function clean_global_configuration(devNum,portNum,params,ttiKeyType,pclPcktType)

    _debug("Hello, clean_global_configuration, here is params \n "..to_string(params))

    -- this function should clean all global configuration done for a specific portNum and protocol
    -- in case there is no channel defined on a given protocol we can clean all configuration related to the given protocol.
    -- if there are no channels on a the given physical port delete configuration specific to this portNum

    local command_data = Command_Data()

    local cleanGlobalCfg=false
    local portLookupEnableGet
    local ttiPacketTypeKeySizeGet
    local controlTunnelTtiIndex
    local defaultTtiRuleIndex
    local defaultIpclRuleIndex
    local defaultTtiRuleDefined
    local defaultIpclRuleDefined
    local ttiPclIdGet
    local num_capwap_channel_db={}
    local result

    if  ttiKeyType == "CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E" and global_capwap_channels.ipv4==0 then
        portLookupEnableGet=ipv4PortLookupEnableGet
        ttiPacketTypeKeySizeGet=ipv4TtiPacketTypeKeySizeGet
        controlTunnelTtiIndex=ipv4ControlTtiParams.ipv4ControlTunnelTtiIndex
        defaultTtiRuleIndex=ipv4DefaultRuleTtiParams.ipv4DefaultTtiRuleIndex
        defaultIpclRuleIndex=defaultIpclRuleParams.defaultIpclRuleIndex
        ttiPclIdGet=ipv4TtiPclIdGet
        ipv4DefaultRuleTtiParams.ipv4DefaultTtiRuleDefined = false
        ipv4ControlTtiParams.ipv4ControlTunnelDefined = false
        defaultIpclRuleParams.defaultIpclRuleDefined = false
        ipv4FirstCapwapCall = true
        num_capwap_channel_db = num_ipv4_capwap_channel_db
        cleanGlobalCfg=true
    elseif ttiKeyType == "CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E" and global_capwap_channels.ipv6==0 then
        portLookupEnableGet=ipv6PortLookupEnableGet
        ttiPacketTypeKeySizeGet=ipv6TtiPacketTypeKeySizeGet
        controlTunnelTtiIndex=ipv6ControlTtiParams.ipv6ControlTunnelTtiIndex
        defaultTtiRuleIndex=ipv6DefaultRuleTtiParams.ipv6DefaultTtiRuleIndex
        defaultIpclRuleIndex=defaultIpclRuleParams.defaultIpclRuleIndex
        ttiPclIdGet=ipv6TtiPclIdGet
        ipv6DefaultRuleTtiParams.ipv6DefaultTtiRuleDefined = false
        ipv6ControlTtiParams.ipv6ControlTunnelDefined = false
        defaultIpclRuleParams.defaultIpclRuleDefined = false
        ipv6FirstCapwapCall = true
        num_capwap_channel_db = num_ipv6_capwap_channel_db
        cleanGlobalCfg=true
    else
        _debug("nothing to clean")
        return true
    end

    if  cleanGlobalCfg==true then
        -- a global configuration should be restore after all configuration is deleted

        -- if last capwap channel per physical port delete configuration
        if num_capwap_channel_db[portNum] == 1 then
            apiName = "cpssDxChTtiPortLookupEnableSet"
            result = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                        "devNum",   devNum },
                { "IN",     "GT_PORT_NUM",                  "portNum",  portNum},
                { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",   "keyType",  ttiKeyType},
                { "IN",     "GT_BOOL",                      "enable",   portLookupEnableGet }, -- default value
            })
            command_data:handleCpssErrorDevPort(result, apiName)
            _debug(apiName .. "   result = " .. result)
        end
        ------------------------------------------------------------------------
        apiName = "cpssDxChTtiPacketTypeKeySizeSet"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",                        "devNum",   devNum },
            { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",   "keyType",  ttiKeyType},
            { "IN",     "CPSS_DXCH_TTI_KEY_SIZE_ENT",   "keySize",  ttiPacketTypeKeySizeGet},
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        ------------------------------------------------------------------------
        local i
        apiName = "cpssDxChTtiUserDefinedByteSet"
        for i=0,29  do
            result = myGenWrapper(apiName, {
                { "IN",     "GT_U8",                         "devNum",     devNum },
                { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    ttiKeyType},
                { "IN",     "GT_U32",                        "udbIndex",   i },
                { "IN",     "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", "CPSS_DXCH_TTI_OFFSET_INVALID_E"},  --default value
                { "IN",     "GT_U8",                         "offset",     0 }  --default value
            })
            command_data:handleCpssErrorDevPort(result, apiName..i)
            _debug(apiName .. "   result = " .. result)
        end

        apiName = "cpssDxChTtiRuleValidStatusSet"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",    "devNum",     devNum },
            { "IN",     "GT_U32",   "index",      controlTunnelTtiIndex},
            { "IN",     "GT_BOOL",  "valid",      false}, --default value
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        freeTtiIndexToPool(controlTunnelTtiIndex)

        apiName = "cpssDxChTtiRuleValidStatusSet"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",    "devNum",     devNum },
            { "IN",     "GT_U32",   "index",      defaultTtiRuleIndex},
            { "IN",     "GT_BOOL",  "valid",      false}, --default value
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        freeTtiIndexToPool(defaultTtiRuleIndex)

        -----------------------------------------------------------------------
        for index,entry in pairs(pclPacketType) do
            apiName = "cpssDxChPclUserDefinedByteSet"
            for i=0,15  do
                result = myGenWrapper(
                    apiName, {
                    { "IN",     "GT_U8",                                "devNum",     devNum },
                    { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pclRuleType},
                    { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", entry},
                    { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
                    { "IN",     "GT_U32",                       "udbIndex" ,  i },
                    { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT","offsetType", "CPSS_DXCH_PCL_OFFSET_INVALID_E" },
                    { "IN",     "GT_U8",                        "offset",     0 }
                })
                command_data:handleCpssErrorDevPort(result, apiName..i)
            end
        end

        apiName = "cpssDxChPclRuleInvalidate"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",                    "devNum",       devNum },
            { "IN",     "CPSS_PCL_RULE_SIZE_ENT",   "ruleSize",     pclKeySize},
            { "IN",     "GT_U32",                   "ruleIndex",    defaultIpclRuleIndex},
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        freePclIndexToPool(defaultIpclRuleIndex)

        apiName = "cpssDxChTtiPclIdSet"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",                         "devNum",     devNum },
            { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    ttiKeyType},
            { "IN",     "GT_U32",                        "pclId",      ttiPclIdGet},
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)
    end

    return true
end
---------------------------------------------------------------------------------------------------
local function no_capwap_eport_func(params)

    local command_data = Command_Data()

    -- this function return default configuration for a specific given channel (undo configuration done in capwap_func_do_port)

    -- Common variables declaration
    local devNum,portNum
    local result
    local table_capwap_system_info={}
    local channelData ={}
    local keyType
    local assignEport
    local egressInfoPtrOrig
    local num_capwap_channel_db={}

    local mac2meDefaultValuePtr = {
        mac     = 0,
        vlanId  = 4095
    }

    local mac2meDefaultMaskPtr = {
        mac     = convertMacAddrStringTo_GT_ETHERADDR("ff:ff:ff:ff:ff:ff"),
        vlanId  = 4095
    }

    -- use the assign_eport value as a key to the table

    channelData = table_capwap_ipv4_system_info[params.assign_eport]
    if channelData~=nil then -- the channel is ipv4 delete it
        table_capwap_system_info = table_capwap_ipv4_system_info
        num_capwap_channel_db = num_ipv4_capwap_channel_db
        params.protocol="ipv4"
        keyType = "CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E"
    else
        channelData = table_capwap_ipv6_system_info[params.assign_eport]
        if channelData~=nil then -- the channel is ipv6 delete it
            table_capwap_system_info = table_capwap_ipv6_system_info
            num_capwap_channel_db = num_ipv6_capwap_channel_db
            params.protocol="ipv6"
            keyType = "CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E"
        else
            print("capwap assign_eport= "..params.assign_eport.." is not valid - can not delete it")
            return true
        end
    end

    -- Common variables initialization.
    local save_current_devNum_1=nil
    local save_current_portNum_1=nil
    local numOfIterations=0

    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()
    -- check that only single interface will be used
    for dummy_, tempDevNum, tempPortNum in command_data:getPortIterator() do
        numOfIterations = numOfIterations + 1
        if numOfIterations > 1 then
            print("ERROR : The feature supports single port or trunk but not multi-ports")
            return nil
        end

        save_current_devNum_1 = tempDevNum
        save_current_portNum_1 = tempPortNum
    end

    devNum = channelData["devNum"]
    portNum = channelData["portNum"]

    -- check that current physical port and device are the same as in the DB
    if portNum ~= save_current_portNum_1 or devNum ~= save_current_devNum_1 then
        print("trying to delete eport "..params.assign_eport.." not from the physical port it was defined")
        return nil
    end

    assignEport = channelData["assign_eport"]
    egressInfoPtrOrig = channelData["egressInfoPtrOrig"]
    ttiIndex =channelData["ttiIndex"]
    tunnelStartIndex  = channelData["tsIndex"]
    macToMeEntryIndex = channelData["macToMeEntryIndex"]
    ipclIndex = channelData["ipclIndex"]

    _debug("no capwap assignEport="..assignEport.. "ttiIndex="..ttiIndex.." tsIndex= "..tunnelStartIndex)

    -- ttiIndex and tunnelStartIndex is deleted from DB in the end of this function
    -- channelData is deleted from DB in the end of this function

    _debug ("no Hello, \n\n ")

    local macToMeSrcInterfaceInfo = {}
    macToMeSrcInterfaceInfo.includeSrcInterface = "CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E"
    macToMeSrcInterfaceInfo.srcDevice = 0
    macToMeSrcInterfaceInfo.srcIsTrunk = GT_FALSE
    macToMeSrcInterfaceInfo.srcPortTrunk = 0

    -- unset a TTI MacToMe entry
    apiName = "cpssDxChTtiMacToMeSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                        "devNum",       devNum },
        { "IN",     "GT_U32",                       "entryIndex",   macToMeEntryIndex },
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",   "valuePtr",     mac2meDefaultValuePtr },
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",   "maskPtr",      mac2meDefaultMaskPtr },
        { "IN",     "CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC", "interfaceInfoPtr", macToMeSrcInterfaceInfo}
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)


    ------------------------------------------------------------------------
    -- unset a TTI Rule entry
    apiName = "cpssDxChTtiRuleValidStatusSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",    "devNum",     devNum },
        { "IN",     "GT_U32",   "index",      ttiIndex  },
        { "IN",     "GT_BOOL",  "valid",      false}, --default value
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------
    -- set IPv4 TS entry in place of IPv6 one  to avoid influence on other tests
    -- IPv6 TS takes place as two IPv4 entries. The odd entries type is defined
    -- as IPv6 if even part was IPv6.
    -- E.g. CAPWAP test use IPv6 TS entry 0. Following nvgre test use IPv4 TS entry 1.
    --      nvgre will NOT work if TS entry 0 still have leftover from IPv6 entry.
    --      Need to write IPv4 TS entry 0 to make entry#1 working.
    tunnelType        = "CPSS_TUNNEL_GENERIC_IPV4_E"
    tunnelStartStruct = "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_ipv4Cfg"
    configPtr         = { ipv4Cfg = tsConfigStcArr }

    apiName = "cpssDxChTunnelStartEntrySet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                 "devNum",                        devNum },
        { "IN",     "GT_U32",                "routerArpTunnelStartLineIndex", tunnelStartIndex},
        { "IN",     "CPSS_TUNNEL_TYPE_ENT",  "tunnelType",                    tunnelType},
        { "IN",     tunnelStartStruct,       "configPtr",                     configPtr},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)
    ------------------------------------------------------------------------
    -- unset a PCL Rule entry
    apiName = "cpssDxChPclRuleInvalidate"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                    "devNum",       devNum },
        { "IN",     "CPSS_PCL_RULE_SIZE_ENT",   "ruleSize",     pclKeySize},
        { "IN",     "GT_U32",                   "ruleIndex",    ipclIndex},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

        ------------------------------------------------------------------------
    -- unset a Eport to Physical port mapping
    apiName = "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                               "devNum",        devNum },
        { "IN",     "GT_PORT_NUM",                         "portNum",       assignEport},
        { "IN",     "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC",  "egressInfoPtr", egressInfoPtrOrig },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------
    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum      },
        { "IN",     "GT_PORT_NUM",             "portNum",         assignEport }, -- ePort
        { "IN",     "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", nil         }, --default, zero values
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    apiName = "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum      },
        { "IN",     "GT_PORT_NUM",             "portNum",         assignEport }, -- ePort
        { "IN",     "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", nil}, --default, zero values
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    ------------------------------------------------------------------------

    if global_capwap_channels.ipv4==0 and global_capwap_channels.ipv6==0 then
        apiName = "cpssDxChTunnelStartGenProfileTableEntrySet"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",                                     "devNum",         devNum },
            { "IN",     "GT_U32",                                    "profileIndex",   templateProfileIndex },
            { "IN",     "CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC", "profileDataPtr", profileDataGet},
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        ------------------------------------------------------------------------
        -- Disable Bridge Control Learning on the ingress port

        apiName = "cpssDxChBrgFdbNaToCpuPerPortSet"
        result = cpssPerPortParamSet(
            apiName,
            devNum, portNum, false, "enable",
            "GT_BOOL")
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)

        apiName = "cpssDxChBrgFdbPortLearnStatusSet"
        result = myGenWrapper(
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", portNum},
            { "IN", "GT_BOOL", "status", true}, -- disable control learning
            { "IN", "CPSS_PORT_LOCK_CMD_ENT", "cmd", "CPSS_LOCK_FRWRD_E"}
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)

        ------------------------------------------------------------------------

        -- unset a PCL Configuration entry
        local interfaceInfo = {}
        interfaceInfo.type = "CPSS_INTERFACE_INDEX_E"
        local lookupCfgGet

        if params.protocol=="ipv6"then
            interfaceInfo.index = ipv6_reserved_pcl_config_index
            lookupCfgGet = ipv6LookupCfgGet
        else
            interfaceInfo.index = ipv4_reserved_pcl_config_index
            lookupCfgGet = ipv4LookupCfgGet
        end

        apiName = "cpssDxChPclCfgTblSet"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",                        "devNum",               devNum },
            { "IN",     "CPSS_INTERFACE_INFO_STC",      "interfaceInfoPtr",     interfaceInfo},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",            "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",            "CPSS_PCL_LOOKUP_0_E"},
            { "IN",     "CPSS_DXCH_PCL_LOOKUP_CFG_STC", "lookupCfgPtr",         lookupCfgGet},
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        apiName = "cpssDxChPclIngressPolicyEnable"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",        "devNum",     devNum},
            { "IN",     "GT_BOOL",      "enable",     false},
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        apiName = "cpssDxChPclPortLookupCfgTabAccessModeSet"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",                        "devNum",       devNum},
            { "IN",     "GT_PORT_NUM",                  "portNum",      portNum},
            { "IN",     "CPSS_PCL_DIRECTION_ENT",       "direction",    "CPSS_PCL_DIRECTION_INGRESS_E"},
            { "IN",     "CPSS_PCL_LOOKUP_NUMBER_ENT",   "lookupNum",    "CPSS_PCL_LOOKUP_0_E"},
            { "IN",     "GT_U32",                       "subLookupNum", 0},
            { "IN",     "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT",  "mode", lookupCfgTabAccessModeGet},
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        apiName = "cpssDxCh3PclTunnelTermForceVlanModeEnableSet"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",                    "devNum",      devNum},
            { "IN",     "GT_BOOL",                  "enable",      true},
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)

        apiName = "appDemoDxChNewPclTcamSupportSet"
        myGenWrapper(apiName, {
            { "IN",     "GT_BOOL",                  "enableIndexConversion",      true},
        })

    end

    -- operation succed remove the info from DB
    -- ttiIndex and tunnelStartIndex is deleted from DB in the end of this function
    freeTtiIndexToPool(ttiIndex)
    freeTsIndexToPool(tunnelStartIndex)
    freeMac2MeIndexToPool(macToMeEntryIndex)
    freePclIndexToPool(ipclIndex)

    -- channelData is deleted from DB in the end of this function
    table_capwap_system_info[params.assign_eport] = nil

    if keyType == "CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E" then
        global_capwap_channels.ipv6  = global_capwap_channels.ipv6  - 1
        clean_global_configuration(devNum,portNum,params,"CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E","CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E")
    else
        global_capwap_channels.ipv4 = global_capwap_channels.ipv4 - 1
        clean_global_configuration(devNum,portNum,params,"CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E","CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E")
    end

    num_capwap_channel_db[portNum] = num_capwap_channel_db[portNum] - 1

    if(((num_ipv4_capwap_channel_db[portNum]==0) or (num_ipv4_capwap_channel_db[portNum]==nil)) and
       ((num_ipv6_capwap_channel_db[portNum]==0) or (num_ipv6_capwap_channel_db[portNum]==nil)))then
        apiName = "cpssDxChPclPortIngressPolicyEnable"
        result = myGenWrapper(apiName, {
            { "IN",     "GT_U8",         "devNum",        devNum },
            { "IN",     "GT_PORT_NUM",   "portNum",       portNum},
            { "IN",     "GT_BOOL",       "enable",        false },
        })
        command_data:handleCpssErrorDevPort(result, apiName)
        _debug(apiName .. "   result = " .. result)
    end

    return true

end


-- ************************************************************************
--  no_capwap_all_func
--        @description  Unset capwap configurations on all phyPorts, restore configuration
--
--        @param params   - none
--
--        @return       true on success, otherwise false and error message
--
local function no_capwap_all_func()

    -- this function return default configuration for all channels (undo all configuration done in capwap_func_do_port)

    local command_data = Command_Data()
    -- Common variables declaration
    local current_index
    local current_entry = {}
    local params = {}
    local result

    -- print input params
    _debug("no capwap all command no params \n")

    -- get the first element in the ipv4 table
    current_index = next(table_capwap_ipv4_system_info,nil)
    current_entry = table_capwap_ipv4_system_info[current_index]
    while current_entry do
        if current_entry.assign_eport ~= nil then
            params.assign_eport = tostring(current_entry.assign_eport)
            params.protocol = "ipv4"
            no_capwap_eport_func(params)
        end

        current_index = next(table_capwap_ipv4_system_info,current_index)
        current_entry = table_capwap_ipv4_system_info[current_index]

        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        command_data:getCommandExecutionResults()
    end

    -- get the first element in the ipv6 table
    current_index = next(table_capwap_ipv6_system_info,nil)
    current_entry = table_capwap_ipv6_system_info[current_index]
    while current_entry do
        if current_entry.assign_eport ~= nil then
            params.assign_eport = tostring(current_entry.assign_eport)
            params.protocol = "ipv6"
            no_capwap_eport_func(params)
        end

        current_index = next(table_capwap_ipv6_system_info,current_index)
        current_entry = table_capwap_ipv6_system_info[current_index]

        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        command_data:getCommandExecutionResults()
    end
    return true
end

---------------------------------------------------------------------------------------------------
local function capwap_func(params)

    -- define a capwap channel for a specific interface

    local command_data = Command_Data()

    command_data:initInterfaceDevPortRange()
     -- now iterate
    command_data:iterateOverPorts(capwap_func_do_port,params)
    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

--------------------------------------------------------
-- command registration: capwap
--------------------------------------------------------
CLI_addHelp("interface", "capwap", "Configure capwap")
CLI_addHelp("interface", "capwap protocol", "Configure capwap ip type (ipv4 or ipv6)")

CLI_addParamDict("capwap_common_params1", {
        { format="assign-eport %ePort", name="assign_eport",       help="source ePort" },

        { format="vlan %vid",           name="vlan_id",            help="vlan id" },

        { format="ap-mac-addr %mac-address", name="ap_mac_addr",   help="ap mac address" },
        { format="ac-mac-addr %mac-address", name="ac_mac_addr",   help="ap mac address" },
})

CLI_addCommand("interface", "capwap protocol ipv4", {
  func   = function(params)
    params.protocol="ipv4"
    return capwap_func(params)
  end,
  help   = "Configure capwap ipv4",

-- capwap type ipv4
--         ap-ip ip ac-ip ip
--         ap-src-udp-port  udp-port
--         ap-mac-addr  mac ac-mac-add mac
--         vlan vlan-id
--         assign-eport eport
--example:
--capwap protocol ipv4 assign-eport 18 vlan 200 ap-src-udp-port 64212 ap-mac-addr 98:32:21:19:18:06 ac-mac-add 88:77:11:11:55:66 ap-ip 2.2.2.2 ac-ip 1.1.1.3

  params = {
      { type="named",
          "#capwap_common_params1",
          { format="ap-ip %ipv4", name="ap_ip", help = "IP version 4 source address"},
          { format="ac-ip %ipv4", name="ac_ip", help = "IP version 4 destination address"},

          mandatory = {   "assign_eport",
                          "vlan_id",
                          "ap_mac_addr",
                          "ac_mac_addr",
                          "ap_ip",
                          "ac_ip"},
      },
  }
})

CLI_addCommand("interface", "capwap protocol ipv6", {
  func   = function(params)
    params.protocol="ipv6"
    return capwap_func(params)
  end,
  help   = "Configure capwap ipv6",

-- capwap type ipv6
--         ap-ip ip ac-ip ip
--         ap-src-udp-port  udp-port
--         ap-mac-addr  mac ac-mac-add mac
--         vlan vlan-id
--         assign-eport eport
--example:
--capwap protocol ipv6 assign-eport 18 vlan 200 ap-src-udp-port 64212 ap-mac-addr 98:32:21:19:18:06 ac-mac-add 88:77:11:11:55:66 ap-ip 6545::3212 ac-ip 1122::CCDD:EEFF

  params = {
      { type="named",
          "#capwap_common_params1",
          { format="ap-ip %ipv6", name="ap_ip", help = "IP version 6 source address"},
          { format="ac-ip %ipv6", name="ac_ip", help = "IP version 6 destination address"},

          mandatory ={"assign_eport",
                      "vlan_id",
                      "ap_mac_addr",
                      "ac_mac_addr",
                      "ap_ip",
                      "ac_ip"},
      },
  }
})

-- ************************************************************************
---
--  capwap_show
--        @description  show capwap info for all interfaces
--
--        @param params         none
--
--        @return       true on success, otherwise false and error message
--
local function capwap_show(params)
    -- show all capwap channels in the system

    --  get the first element in the table
    local current_index , current_entry
    local current_channel_name_data
    local temp
    local ap_ip_string
    local ac_ip_string
    local ap_mac_addr_string
    local ac_mac_addr_string
    local iterator
    local protocol
    local table_capwap_system_info={}
    local num_capwap_channel_db ={}

    for i = 0,1,1 do

        iterator = 1
        if(i==0) then
            protocol="ipv4"
            table_capwap_system_info = table_capwap_ipv4_system_info
            num_capwap_channel_db = num_ipv4_capwap_channel_db
        elseif(i==1) then
            protocol="ipv6"
            table_capwap_system_info = table_capwap_ipv6_system_info
            num_capwap_channel_db = num_ipv6_capwap_channel_db
        else --should never happen
            command_data:handleCpssErrorDevPort("GT_FAIL", "wrong protocol")
        end
        current_index , current_entry = next(table_capwap_system_info,nil)

        print("index# " .. "|" .. "  protocol  " .. "|" .. "  devNum  " .. " | " .. "  portNum  ".. " | " .. "  assign-eport ".. " | " .. "  vlan   ".. " | " .. "  ap-mac-addr             ".. " | " .. "  ac-mac-addr                ".. " | " .. "ac-ip                    ".. " | " .. "ap-ip ")
        print("-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")

        while current_entry do
            current_channel_name_data = current_entry["assign_eport"]
            if current_channel_name_data ~= nil then
                temp =current_entry["ap_mac_addr"]
                ap_mac_addr_string = tostring(temp["string"])

                temp =current_entry["ac_mac_addr"]
                ac_mac_addr_string = tostring(temp["string"])

                temp =current_entry["ap_ip"]
                ap_ip_string = tostring(temp["string"])

                temp =current_entry["ac_ip"]
                ac_ip_string = tostring(temp["string"])

                --ap_mac_addr_string = current_entry["ap_mac_addr"]
                --ac_mac_addr_string = current_entry["ac_mac_addr"].string
                print("    " .. iterator .. "       | " ..
                        current_entry["protocol"] .. "        | " ..
                        current_entry["devNum"] .. "            | " ..
                        current_entry["portNum"] .. "           | " ..
                        current_entry["assign_eport"] .. "              | " ..
                        current_entry["vlan"]  .. "      | " ..
                        ap_mac_addr_string .. "    |      " ..
                        ac_mac_addr_string .. "   |     " ..
                        ap_ip_string  .. "     |     " ..
                        ac_ip_string )
                print ("")
                iterator = iterator + 1
            end

            current_index , current_entry = next(table_capwap_system_info,current_index)

        end

        current_index , current_entry = next(num_capwap_channel_db,nil)
        while current_entry do
            print("number of channels defined for protocol "..protocol .." on portNum["..current_index.."] = "..current_entry)
            current_index , current_entry = next(num_capwap_channel_db,current_index)
        end

        print ("End of " .. (iterator - 1) .." " .. protocol .. " capwap configuration")
        print ("")

    end
end


-- the command looks like :
--Console# show capwap
--------------------------------------------------------------------------------
-- command registration: show capwap
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show capwap",
    {
        func   = capwap_show,
        help   = "Show capwap info configuration",
        params = {}
    }
)

--------------------------------------------------------
-- command registration: no capwap
--------------------------------------------------------

-- the command looks like :
--Console(config)# interface ethernet 0/2
--Console(config-if)# no capwap all
CLI_addCommand("interface", "no capwap all", {
    func   = no_capwap_all_func,
    help   = "Disable capwap configuration on all ports, restore configurations",
    params = {}
})

-- the command looks like :
--Console(config)# interface ethernet 0/2
--Console(config-if)# no capwap assign-eport {eport}
CLI_addCommand("interface", "no capwap", {
    func   = no_capwap_eport_func,
    help   = "Disable capwap configuration on eport, restore configuration",
    params = {
        { type = "named",
            { format = "assign-eport %ePort", name = "assign_eport", help = "ePort number to delete"},
            mandatory = { "assign_eport" }
        }
    }
})
