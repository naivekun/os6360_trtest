--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vsi.lua
--*
--* DESCRIPTION:
--*       creates UNI and NNI interfaces
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
local tcam_max_num_rules_per_floor = 256 * 12

local VPN_EARCH_DB_TYPE_ENT = {
    -- DB TYPE structure
    ["VPN_EARCH_DB_TYPE_EVID_E"] = 0,
    ["VPN_EARCH_DB_TYPE_EVIDX_E"] = 1,
    ["VPN_EARCH_DB_TYPE_EPORT_E"] = 2,
    ["VPN_EARCH_DB_TYPE_L2MLL_E"] = 3,
    ["VPN_EARCH_DB_TYPE_TS_E"] = 4,
    ["VPN_EARCH_DB_TYPE_TTI_E"] = 5,
}

local VPN_DB_OPERATION_ENT = {
    -- operation type structure
    ["VPN_DB_OPERATION_ALLOC_E"] = 0,
    ["VPN_DB_OPERATION_FREE_E"] = 1,
    ["VPN_DB_OPERATION_CREATE_E"] = 2,
    ["VPN_DB_OPERATION_DESTROY_E"] = 3,
    ["VPN_DB_OPERATION_ADD_E"] = 4,
    ["VPN_DB_OPERATION_REMOVE_E"] = 5,
    ["VPN_DB_OPERATION_SET_E"] = 6,
    ["VPN_DB_OPERATION_GET_E"] = 7,
}

local TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_0 = 0
local TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_1 = 1
local TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_DEFAULT = 0xFF

local TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_0 = 0
local TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_1 = 1
local TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_2 = 2
local TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_3 = 3

local TPID_TABLE_INDEX_EGRESS_1 = 1

local bit_index_in_sourceId_represent_meshId = 10 -- use 10 to support BC3 in 512 ports mode (instead of 10)
local sourceIdIndication = bit_shl(1,bit_index_in_sourceId_represent_meshId) --0x400 instead of 0x800 (for BC3 512 ports mode)

local MESH_ID_0 = 0
local MESH_ID_2 = 2

-- dataBases

local isInitDB = false
local numberOfFreeEvid = 20
local numberOfFreeEvidx = 20
local numberOfFreeMll = 20
local numberOfL2Mll = 40
local numberOfTs = 20
local numberOfFreeTtiRules = 20
local freeTti = {}
local freeTtiBmp = {}
local freeTs = {}
local freeEvid = {}
local freeEvidx = {}
local freeMll = {}
local vsiDb = {}
local configuredPortsUni = {}
local configuredPortsNni = {}
local eVidStored = {}
local portToMll = {}

local l2MllLookupMaxVidxIndex = 0x1000 

local l2MllLookupBaseVidxIndex = 0 

local vxlanNvgreGenericInitDone = false

local mllIndexStored = 0

-- ************************************************************************
--
--  getLogicalDevNum
--        @description  get logical device number from eport value
--
local function getLogicalDevNum(eport)
    return bit_shr(eport,6)
end

-- ************************************************************************
--
--  getLogicalPortNum
--        @description  get logical port number from eport value
--
local function getLogicalPortNum(eport)
    return bit_and(eport,0x3f)
end

-- ************************************************************************
--
--  getIndexInTCAM
--        @description  return the actual index of the rule in TCAM
--

local function getTTIIndexInTCAM(deviceNum, startOrEnd)
    local offsetTTI0, v
    offsetTTI0, v = myGenWrapper("appDemoDxChTcamTtiBaseIndexGet", {
        { "IN", "GT_U8", "devNum", deviceNum },
        { "IN", "GT_U32", "hitNum", startOrEnd }
    })
    return offsetTTI0
end



-- ************************************************************************
--
--  dec2hex
--        @description  convert a decimal number to hexa
--
local function dec2hex(IN)
    local B, K, OUT, I, D = 16, "0123456789ABCDEF", "", 0
    while IN > 0 do
        I = I + 1
        IN, D = math.floor(IN / B), math.mod(IN, B) + 1
        OUT = string.sub(K, D, D) .. OUT
    end
    return OUT
end

-- ************************************************************************
--
--  Hex2Bin
--        @description  convert a hexa number to binary number
--
local function Hex2Bin(s)
    local hex2bin = {
        ["0"] = "0000",
        ["1"] = "0001",
        ["2"] = "0010",
        ["3"] = "0011",
        ["4"] = "0100",
        ["5"] = "0101",
        ["6"] = "0110",
        ["7"] = "0111",
        ["8"] = "1000",
        ["9"] = "1001",
        ["a"] = "1010",
        ["b"] = "1011",
        ["c"] = "1100",
        ["d"] = "1101",
        ["e"] = "1110",
        ["f"] = "1111"
    }

    local ret = ""
    local i = 0


    for i in string.gfind(s, ".") do
        i = string.lower(i)

        ret = ret .. hex2bin[i]
    end

    return ret
end

-- ************************************************************************
--
--  getProfileDataPtr
--        @description  getter function, get the profile data
--                      for entering an entry in the tunnel start
--
local function getProfileDataPtr(profileIndex)
    local profileDataPtr = {}
    local sidBaseBit;

    profileDataPtr.templateDataBitsCfg = {}
    local indexOfTunnelStartTemplateData
    if (profileIndex == 0) then -- if VxLAN
        indexOfTunnelStartTemplateData = 3;
        sidBaseBit = 32;
        profileDataPtr.templateDataSize = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E"
    else -- if NVGRE
        -- the first 4 bytes of GRE Header built using Tunnel Start Entry fields - not profile
        -- this profile is only for the second 4 bytes containing SID (24 bits) and Reserved
        indexOfTunnelStartTemplateData = 2;
        sidBaseBit = 0;
        profileDataPtr.templateDataSize = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E"
    end
    profileDataPtr.templateDataBitsCfg[indexOfTunnelStartTemplateData] = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E"

    for i = sidBaseBit, (sidBaseBit + 7) do -- set bits 32 - 39 (VXLAN) or bits 0 - 7 (NVGRE)
        profileDataPtr.templateDataBitsCfg[i] = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_2_E";
    end
    for i = (sidBaseBit + 8), (sidBaseBit + 15) do -- set bits 40 - 47 (VXLAN) or bits 8 - 15 (NVGRE)
        profileDataPtr.templateDataBitsCfg[i] = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_1_E";
    end
    for i = (sidBaseBit + 16), (sidBaseBit + 23) do -- set bits 48 - 55 (VXLAN) or bits 16 - 23 (NVGRE)
        profileDataPtr.templateDataBitsCfg[i] = "CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_0_E";
    end

    profileDataPtr.udpSrcPortMode = "CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E";
    return profileDataPtr
end

-- ************************************************************************
--
--  createTpid
--        @description  creates TPid for tag0 and tag1
--
--
local function createTpid(devNum)
    local ret, val

    -- set ingress tpid entry
    ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DIRECTION_ENT", "direction", "CPSS_DIRECTION_INGRESS_E" },
        { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_0 },
        { "IN", "GT_U16", "etherType", 0x8100 }
    })
    if ret ~= 0 then
        print("Error while executing cpssDxChBrgVlanTpidEntrySet")
        return false
    end

    ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DIRECTION_ENT", "direction", "CPSS_DIRECTION_INGRESS_E" },
        { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_1 },
        { "IN", "GT_U16", "etherType", 0x88A8 }
    })
    if ret ~= 0 then
        print("Error while executing cpssDxChBrgVlanTpidEntrySet")
        return false
    end

    if (is_sip_5(devNum)) then
        -- set egress tpid entry
        ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DIRECTION_ENT", "direction", "CPSS_DIRECTION_EGRESS_E" },
            { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_0 },
            { "IN", "GT_U16", "etherType", 0x8100 }
        })
        if ret ~= 0 then
            print("Error while executing cpssDxChBrgVlanTpidEntrySet")
            return false
        end

        ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DIRECTION_ENT", "direction", "CPSS_DIRECTION_EGRESS_E" },
            { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_1 },
            { "IN", "GT_U16", "etherType", 0x88A8 }
        })
        if ret ~= 0 then
            print("Error while executing cpssDxChBrgVlanTpidEntrySet")
            return false
        end

        -- set ingress profile for vlan tag0 with bitmap 0
        ret, val = myGenWrapper("cpssDxChBrgVlanIngressTpidProfileSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "profile", 7 },
            { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE0_E" },
            { "IN", "GT_U32", "tpidBmp", bit_shl(1,TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_1) }
        })
        if ret ~= 0 then
            print("Error while executing cpssDxChBrgVlanIngressTpidProfileSet")
            return false
        end

        -- set ingress profile for vlan tag1 with bitmap 1
        ret, val = myGenWrapper("cpssDxChBrgVlanIngressTpidProfileSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "profile", 7 },
            { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE1_E" },
            { "IN", "GT_U32", "tpidBmp", bit_shl(1,TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_0) }
        })
        if ret ~= 0 then
            print("Error while executing cpssDxChBrgVlanIngressTpidProfileSet")
            return false
        end
    elseif (isVplsModeEnabled(devNum)) then
        -- set egress tpid entry
        ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DXCH_ETHERTYPE_TABLE_ENT", "direction", "CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E" },
            { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_2 },
            { "IN", "GT_U16", "etherType", 0x88a8 }
        })
        if ret ~= 0 then
            print("Error while executing cpssDxChBrgVlanTpidEntrySet")
            return false
        end

        ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DXCH_ETHERTYPE_TABLE_ENT", "direction", "CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E" },
            { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_3 },
            { "IN", "GT_U16", "etherType", 0x8100 }
        })
        if ret ~= 0 then
            print("Error while executing cpssDxChBrgVlanTpidEntrySet")
            return false
        end
    end
    return true
end

-- ************************************************************************
--   resetTpid
--
--        @description  resets TPid for tag0 and tag1
--
--
local function resetTpid(devNum)
    local ret, val

    -- set ingress tpid entry
    ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DIRECTION_ENT", "direction", "CPSS_DIRECTION_INGRESS_E" },
        { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_0 },
        { "IN", "GT_U16", "etherType", 0x8100 }
    })
    if ret ~= 0 then
        print("Error while executing cpssDxChBrgVlanTpidEntrySet")
        return false
    end

    ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DIRECTION_ENT", "direction", "CPSS_DIRECTION_INGRESS_E" },
        { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_1 },
        { "IN", "GT_U16", "etherType", 0x8100 }
    })
    if ret ~= 0 then
        print("Error while executing cpssDxChBrgVlanTpidEntrySet")
        return false
    end

    -- set egress tpid entry
    ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DIRECTION_ENT", "direction", "CPSS_DIRECTION_EGRESS_E" },
        { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_0 },
        { "IN", "GT_U16", "etherType", 0x8100 }
    })
    if ret ~= 0 then
        print("Error while executing cpssDxChBrgVlanTpidEntrySet")
        return false
    end

    ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DIRECTION_ENT", "direction", "CPSS_DIRECTION_EGRESS_E" },
        { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_1 },
        { "IN", "GT_U16", "etherType", 0x8100 }
    })
    if ret ~= 0 then
        print("Error while executing cpssDxChBrgVlanTpidEntrySet")
        return false
    end
    if (is_sip_5(devNum)) then
        -- set ingress profile for vlan tag0 with bitmap 0
        ret, val = myGenWrapper("cpssDxChBrgVlanIngressTpidProfileSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "profile", 7 },
            { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE0_E" },
            { "IN", "GT_U32", "tpidBmp", TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_DEFAULT }
        })
        if ret ~= 0 then
            print("Error while executing cpssDxChBrgVlanIngressTpidProfileSet")
            return false
        end

        -- set ingress profile for vlan tag1 with bitmap 1
        ret, val = myGenWrapper("cpssDxChBrgVlanIngressTpidProfileSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "profile", 7 },
            { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE1_E" },
            { "IN", "GT_U32", "tpidBmp", TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_DEFAULT }
        })
        if ret ~= 0 then
            print("Error while executing cpssDxChBrgVlanIngressTpidProfileSet")
            return false
        end
    elseif (isVplsModeEnabled(devNum)) then
        -- set egress tpid entry
        ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DXCH_ETHERTYPE_TABLE_ENT", "direction", "CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E" },
            { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_2 },
            { "IN", "GT_U16", "etherType", 0x8100 }
        })
        if ret ~= 0 then
            print("Error while executing cpssDxChBrgVlanTpidEntrySet")
            return false
        end

        ret, val = myGenWrapper("cpssDxChBrgVlanTpidEntrySet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DXCH_ETHERTYPE_TABLE_ENT", "direction", "CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E" },
            { "IN", "GT_U32", "entryIndex", TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_3 },
            { "IN", "GT_U16", "etherType", 0x8100 }
        })
        if ret ~= 0 then
            print("Error while executing cpssDxChBrgVlanTpidEntrySet")
            return false
        end
    end
    return true
end

-- ************************************************************************
---
-- splitToBytes
-- @description splits a number to a bytes array (table)
-- the table is from msb to lsb
--
local function splitToBytes(param, numberOfBytes)
    local udpPortAsString = string.format("%0" .. numberOfBytes * 2 .. "x", param)
    local bytesTbl = {}
    for i = 1, numberOfBytes * 2, 2 do -- the table is from msb to lsb
        table.insert(bytesTbl, tonumber("0x" .. string.sub(udpPortAsString, i, i + 1)))
    end
    return bytesTbl
end

-- ************************************************************************
---
-- userDefineBytesSet
-- @description set the UDB (user define bytes) for CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E
--
local function userDefineBytesSet_util(devNum, isVxlan, pclId)
    local ret, val, packetType;

    if isVxlan == true then
        packetType = "CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E";
    else
        packetType = "CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E";
    end

    -- set key size to 30 bytes
    ret, val = myGenWrapper("cpssDxChTtiPacketTypeKeySizeSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetType },
        { "IN", "CPSS_DXCH_TTI_KEY_SIZE_ENT", "size", "CPSS_DXCH_TTI_KEY_SIZE_30_B_E" }
    })
    if (ret ~= 0) then
        print("Error while executing cpssDxChTtiPacketTypeKeySizeSet")
        return false
    end

    --pcl id
    ret, val = myGenWrapper("cpssDxChTtiPclIdSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetType },
        { "IN", "GT_U32", "pclId", pclId }
    })
    if (ret ~= 0) then
        print("Error while executing cpssDxChTtiPclIdSet")
        return false
    end

    -- pclID - 2 bytes
    -- pclId[4:0] - bits[180:176]
    ret, val = myGenWrapper("cpssDxChTtiUserDefinedByteSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetType },
        { "IN", "GT_U32", "udbIndex", 0 },
        { "IN", "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", "CPSS_DXCH_TTI_OFFSET_METADATA_E" },
        { "IN", "GT_U8", "offset", 22 }
    })
    if (ret ~= 0) then
        print("Error while executing cpssDxChTtiUserDefinedByteSet")
        return false
    end

    -- pclId[9:5] - bits[188:184]
    ret, val = myGenWrapper("cpssDxChTtiUserDefinedByteSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetType },
        { "IN", "GT_U32", "udbIndex", 1 },
        { "IN", "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", "CPSS_DXCH_TTI_OFFSET_METADATA_E" },
        { "IN", "GT_U8", "offset", 23 }
    })
    if (ret ~= 0) then
        print("Error while executing cpssDxChTtiUserDefinedByteSet")
        return false
    end

    -- port - 2 bytes - bits[220:208] Local Device Source ePort/TrunkID
    ret, val = myGenWrapper("cpssDxChTtiUserDefinedByteSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetType },
        { "IN", "GT_U32", "udbIndex", 2 },
        { "IN", "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", "CPSS_DXCH_TTI_OFFSET_METADATA_E" },
        { "IN", "GT_U8", "offset", 26 }
    })
    if (ret ~= 0) then
        print("Error while executing cpssDxChTtiUserDefinedByteSet")
        return false
    end

    ret, val = myGenWrapper("cpssDxChTtiUserDefinedByteSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetType },
        { "IN", "GT_U32", "udbIndex", 3 },
        { "IN", "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", "CPSS_DXCH_TTI_OFFSET_METADATA_E" },
        { "IN", "GT_U8", "offset", 27 }
    })
    if (ret ~= 0) then
        print("Error while executing cpssDxChTtiUserDefinedByteSet")
        return false
    end

    -- src/dest IP
    -- bits[39:8]  IPv4/ARP DIP
    -- bits[71:40] IPv4/ARP SIP
    for i = 4, 11 do
        ret, val = myGenWrapper("cpssDxChTtiUserDefinedByteSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetType },
            { "IN", "GT_U32", "udbIndex", i },
            { "IN", "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", "CPSS_DXCH_TTI_OFFSET_METADATA_E" },
            { "IN", "GT_U8", "offset", i - 3 }
        })
        if (ret ~= 0) then
            print("Error while executing cpssDxChTtiUserDefinedByteSet")
            return false
        end
    end

    if (isVxlan == true) then
        --udp port, 2 bytes - destination UDP port[15:0] - UDP_HEADER[31:16]
        for i = 12, 13 do
            ret, val = myGenWrapper("cpssDxChTtiUserDefinedByteSet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetType },
                { "IN", "GT_U32", "udbIndex", i },
                { "IN", "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", "CPSS_DXCH_TTI_OFFSET_L4_E" },
                { "IN", "GT_U8", "offset", i - 10 }
            })
            if (ret ~= 0) then
                print("Error while executing cpssDxChTtiUserDefinedByteSet")
                return false
            end
        end

        --vsi, 3 bytes - bytes 8(UDP_HDR) + 6,5,4(VXLAN_HDR)
        local offset = 14
        for i = 14, 16 do
            ret, val = myGenWrapper("cpssDxChTtiUserDefinedByteSet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetType },
                { "IN", "GT_U32", "udbIndex", i },
                { "IN", "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", "CPSS_DXCH_TTI_OFFSET_L4_E" },
                { "IN", "GT_U8", "offset", offset }
            })
            if (ret ~= 0) then
                print("Error while executing cpssDxChTtiUserDefinedByteSet")
                return false
            end
            offset = offset - 1
        end
    else -- nvGre
        --vsi, 3 bytes - bytes 6,5,4(NVGRE_HDR)
        local offset = 6
        for i = 14, 16 do
            ret, val = myGenWrapper("cpssDxChTtiUserDefinedByteSet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetType },
                { "IN", "GT_U32", "udbIndex", i },
                { "IN", "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", "CPSS_DXCH_TTI_OFFSET_L4_E" },
                { "IN", "GT_U8", "offset", offset }
            })
            if (ret ~= 0) then
                print("Error while executing cpssDxChTtiUserDefinedByteSet")
                return false
            end
            offset = offset - 1
        end
    end
    return true
end

local function userDefineBytesSet(devNum)
    local ret;

    -- for vxLan
    ret = userDefineBytesSet_util(devNum, true, 1);
    if (ret ~= true) then return ret; end;

    -- for nvGre
    ret = userDefineBytesSet_util(devNum, false, 2);
    if (ret ~= true) then return ret; end;

    return true;
end

local function userDefineBytesReset(devNum)
    local ret;

    local ret, val;
    local packetTypes, pktTypeKey;
    local udbIndexes, usbIndexKey;

    packetTypes = {
        "CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E",
        "CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E"};

    udbIndexes = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    pktTypeKey = next(packetTypes, nil);
    while (pktTypeKey ~= nil) do

        -- set key size to 30 bytes
        ret, val = myGenWrapper("cpssDxChTtiPacketTypeKeySizeSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetTypes[pktTypeKey] },
            { "IN", "CPSS_DXCH_TTI_KEY_SIZE_ENT", "size", "CPSS_DXCH_TTI_KEY_SIZE_30_B_E" }
        })

        --pcl id
        ret, val = myGenWrapper("cpssDxChTtiPclIdSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetTypes[pktTypeKey] },
            { "IN", "GT_U32", "pclId", 1 }
        })

        usbIndexKey = next(udbIndexes, nil);
        while (usbIndexKey ~= nil) do
            ret, val = myGenWrapper("cpssDxChTtiUserDefinedByteSet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", packetTypes[pktTypeKey] },
                { "IN", "GT_U32", "udbIndex", udbIndexes[usbIndexKey] },
                { "IN", "CPSS_DXCH_TTI_OFFSET_TYPE_ENT",
                    "offsetType", "CPSS_DXCH_TTI_OFFSET_INVALID_E" },
                { "IN", "GT_U8", "offset", 0 }
            })

            usbIndexKey = next(udbIndexes, usbIndexKey);
        end

        pktTypeKey = next(packetTypes, pktTypeKey);
    end

    return true;
end

-- ************************************************************************
--
--  logicalTargetGenConfUNIInit
--        @description  initiate the logical target general configuration for UNI
--
--
local function logicalTargetGenConfUNIInit(devNum, hwDevNum, portNum, eport, tagState)
    local ret, val

    local logicalPortMappingTable = {}
    logicalPortMappingTable.egressVlanFilteringEnable = false
    logicalPortMappingTable.egressVlanTagStateEnable = true
    logicalPortMappingTable.egressVlanTagState = tagState
    logicalPortMappingTable.egressTagTpidIndex = TPID_TABLE_INDEX_EGRESS_1
    logicalPortMappingTable.outputInterface = {}
    logicalPortMappingTable.outputInterface.physicalInterface = {}
    logicalPortMappingTable.outputInterface.physicalInterface.type = "CPSS_INTERFACE_PORT_E"
    logicalPortMappingTable.outputInterface.physicalInterface.devPort = {}
    logicalPortMappingTable.outputInterface.physicalInterface.devPort.devNum = hwDevNum
    logicalPortMappingTable.outputInterface.physicalInterface.devPort.portNum = portNum
    ret, val = myGenWrapper("cpssDxChLogicalTargetMappingTableEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U8", "logicalDevNum", getLogicalDevNum(eport) },
        { "IN", "GT_U8", "logicalPortNum", getLogicalPortNum(eport) },
        { "IN", "CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC", "logicalPortMappingTablePtr", logicalPortMappingTable }
    })

    if (ret ~= 0) then
        print("Error: could not set a logical target mapping entry on logical target device " .. getLogicalDevNum(eport) .. " and port " .. getLogicalPortNum(eport) .. ":" .. returnCodes[ret])
        return false
    end

    return true
end

-- ************************************************************************
--
--  logicalTargetGenConfNNIInit
--        @description  initiate the logical target general configuration for NNI 
--
--
local function logicalTargetGenConfNNIInit(devNum, hwDevNum, portNum, eport, pwTagState, pwTagMode, ptagVid, tsIndex)
    local ret, val

    local logicalPortMappingTable = {}
    logicalPortMappingTable.egressVlanFilteringEnable = false
    logicalPortMappingTable.egressVlanTagStateEnable = true
    logicalPortMappingTable.egressVlanTagState = pwTagState
    if (pwTagMode == 'CPSS_DXCH_TTI_PW_TAG_TAGGED_PW_MODE_E') then
        logicalPortMappingTable.assignVid0Enable = true
    else
        logicalPortMappingTable.assignVid0Enable = false
    end
    logicalPortMappingTable.vid0 = ptagVid
    logicalPortMappingTable.egressTagTpidIndex = TPID_TABLE_INDEX_EGRESS_1
    logicalPortMappingTable.outputInterface = {}
    logicalPortMappingTable.outputInterface.physicalInterface = {}
    logicalPortMappingTable.outputInterface.physicalInterface.type = "CPSS_INTERFACE_PORT_E"
    logicalPortMappingTable.outputInterface.physicalInterface.devPort = {}
    logicalPortMappingTable.outputInterface.physicalInterface.devPort.devNum = hwDevNum
    logicalPortMappingTable.outputInterface.physicalInterface.devPort.portNum = portNum
    logicalPortMappingTable.outputInterface.isTunnelStart = true
    logicalPortMappingTable.outputInterface.tunnelStartInfo = {}
    logicalPortMappingTable.outputInterface.tunnelStartInfo.passengerPacketType = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
    logicalPortMappingTable.outputInterface.tunnelStartInfo.ptr = tsIndex

    ret, val = myGenWrapper("cpssDxChLogicalTargetMappingTableEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U8", "logicalDevNum", getLogicalDevNum(eport) },
        { "IN", "GT_U8", "logicalPortNum", getLogicalPortNum(eport) },
        { "IN", "CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC", "logicalPortMappingTablePtr", logicalPortMappingTable }
    })

    if (ret ~= 0) then
        print("Error: could not set a logical target mapping entry on logical target device " .. getLogicalDevNum(eport) .. " and port " .. getLogicalPortNum(eport) .. ":" .. returnCodes[ret])
        return false
    end

    return true
end

-- ************************************************************************
--
--  logicalTargetGenConfRestore
--        @description  restore the logical target general configuration 
--
--
local function logicalTargetGenConfRestore(devNum, eport)
    local ret, val
    
    local logicalPortMappingTable = {}
    logicalPortMappingTable.egressVlanFilteringEnable = false
    logicalPortMappingTable.egressVlanTagStateEnable = false
    logicalPortMappingTable.assignVid0Enable = false
    logicalPortMappingTable.outputInterface = {}
    logicalPortMappingTable.outputInterface.physicalInterface = {}
    logicalPortMappingTable.outputInterface.physicalInterface.type = "CPSS_INTERFACE_PORT_E"
    logicalPortMappingTable.outputInterface.physicalInterface.devPort = {}
    logicalPortMappingTable.outputInterface.physicalInterface.devPort.devNum = 0
    logicalPortMappingTable.outputInterface.physicalInterface.devPort.portNum = 0
    ret, val = myGenWrapper("cpssDxChLogicalTargetMappingTableEntrySet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U8", "logicalDevNum", getLogicalDevNum(eport) },
        { "IN", "GT_U8", "logicalPortNum", getLogicalPortNum(eport) },
        { "IN", "CPSS_DXCH_LOGICAL_TARGET_MAPPING_STC", "logicalPortMappingTablePtr", logicalPortMappingTable }
    })

    if (ret ~= 0) then
        print("Error: could not set a logical target mapping entry on logical target device " .. getLogicalDevNum(eport) .. " and port " .. getLogicalPortNum(eport) .. ":" .. returnCodes[ret])
        return false
    end

    return true
end

-- ************************************************************************
--
--  vxlanNvgreGenericInit
--        @description  initiate the generic configurations for VxLAN and NVGRE
--
--
local function vxlanNvgreGenericInit(devNum)
    local ret, val
    local profileIndex -- VxLan/NvGRE profile index
    local profileDataPtr

    if (vxlanNvgreGenericInitDone == true) then
        return true;
    end
    if (is_sip_5(devNum)) then
        -- set tunnel start profile for VxLan (profileIndex = 0) and for NVGRE (profileIndex = 1)
        for profileIndex = 0, 1 do
            profileDataPtr = getProfileDataPtr(profileIndex)

            ret, val = myGenWrapper("cpssDxChTunnelStartGenProfileTableEntrySet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "GT_U32", "profileIndex", profileIndex },
                { "IN", "CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC", "profileDataPtr", profileDataPtr }
            })

            if (ret ~= 0) then
                print("Error while executing cpssDxChTunnelStartGenProfileTableEntrySet")
                return false
            end
        end

        local lookup = 0
        -- get tti source id bits
        ret, val = myGenWrapper("cpssDxChTtiSourceIdBitsOverrideGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "lookup", lookup },
            { "OUT", "GT_U32", "overrideBitmapPtr" }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChTtiSourceIdBitsOverrideGet")
            return false
        end

        --  overrideBitmapPtr = overrideBitmapPtr | 0x800
        local bitMap = val.overrideBitmapPtr
        local bitMapAsString = Hex2Bin(dec2hex(bitMap))
        local horizonBitIndex = #bitMapAsString - 11
        local horizonBit = bitMapAsString:sub(horizonBitIndex, horizonBitIndex)
        if (horizonBit == "0") then bitMap = bitMap + sourceIdIndication end

        -- set tti source id bits after the OR operation
        ret, val = myGenWrapper("cpssDxChTtiSourceIdBitsOverrideSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "lookup", lookup },
            { "IN", "GT_U32", "overrideBitmapPtr", bitMap }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChTtiSourceIdBitsOverrideSet")
            return false
        end

        -- eVidx general configuration
        ret, val = myGenWrapper("cpssDxChL2MllLookupForAllEvidxEnableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_BOOL", "enable", false }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllLookupForAllEvidxEnableSet")
            return false
        end

        ret, val = myGenWrapper("cpssDxChL2MllLookupMaxVidxIndexSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "maxVidxIndex", (l2MllLookupMaxVidxIndex - 1) }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllLookupMaxVidxIndexSet")
            return false
        end

        -- set mesh ID size
        ret, val = myGenWrapper("cpssDxChBrgEgrMeshIdConfigurationSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_BOOL", "enable", true },
            { "IN", "GT_U32", "meshIdOffset", bit_index_in_sourceId_represent_meshId },
            { "IN", "GT_U32", "meshIdSize", 1 }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChBrgEgrMeshIdConfigurationSet")
            return false
        end

        -- Disable Exceptions caused by non-zero bit in 16 MS-bits og GRE header
        ret, val = myGenWrapper("cpssDxChTtiGreExtensionsEnableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_BOOL", "enable", false }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChTtiGreExtensionsEnableSet")
            return false
        end
    elseif (isVplsModeEnabled(devNum)) then
        ret, val = myGenWrapper("cpssDxChLogicalTargetMappingEnableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_BOOL", "enable", true }
            })
        if (ret ~= 0) then
            print("Error: could not enable logical port mapping: " .. returnCodes[ret])
            return false
        end
        ret, val = myGenWrapper("cpssDxChTtiPwCwExceptionCmdSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT", "exceptionType", "CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E" },
            { "IN", "CPSS_PACKET_CMD_ENT", "command", "CPSS_PACKET_CMD_DROP_HARD_E" }
            })
        if (ret ~= 0) then
            print("Error: could not set PW exception command: " .. returnCodes[ret])
            return false
        end

-- local macEntry = 
--    { key =          {macAddr = "000000240003",          vlanId  = 4001},
--      dstInterface = {type    = "CPSS_INTERFACE_PORT_E", devPort = { portNum=23, devNum=10}}
--    }
-- ret, val = myGenWrapper(
--    "cpssDxChBrgFdbMacEntrySet", {
--        { "IN", "GT_U8",                    "devNum",   devNum          },
--        { "IN", "CPSS_MAC_ENTRY_EXT_STC",   "macEntry", macEntry }})
-- if (ret ~= 0) then
--    print("cpssDxChBrgFdbMacEntrySet: could not set mac entry: " .. returnCodes[ret])
--    return false
-- end

    end
    -- Cause using MAC_SA in legacy TTI_KEY_ETH key
    ret, val = myGenWrapper("cpssDxChTtiMacModeSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", "CPSS_DXCH_TTI_KEY_ETH_E" },
        { "IN", "CPSS_DXCH_TTI_MAC_MODE_ENT", "macMode", "CPSS_DXCH_TTI_MAC_MODE_SA_E" }
    })

    if (ret ~= 0) then
        print("Error while executing cpssDxChTtiMacModeSet")
        return false
    end

    ret = createTpid(devNum)
    if (ret ~= true) then
        print("Error while executing createTpid ")
        return false
    end
    if (is_sip_5(devNum)) then
        ret = userDefineBytesSet(devNum)
        if (ret ~= true) then
            print("Error in setting udb")
            return false
        end
    end

    vxlanNvgreGenericInitDone = true -- raise initDone flag in order to do it only once.
    return true
end

-- ************************************************************************
--
--  vxlanNvgreGenericRestore
--        @description  restore the generic configurations for VxLAN and NVGRE
--
--
local function vxlanNvgreGenericRestore(devNum)
    local ret, val
    local profileIndex -- VxLan/NvGRE profile index
    local profileDataPtr

    print("Reseting Global Configuration");

    vxlanNvgreGenericInitDone = false -- drop initDone flag to cause reinitialization.
    if (is_sip_5(devNum)) then
        -- set tunnel start profile for VxLan (profileIndex = 0) and for NVGRE (profileIndex = 1)
        for profileIndex = 0, 1 do
            profileDataPtr = {}

            ret, val = myGenWrapper("cpssDxChTunnelStartGenProfileTableEntrySet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "GT_U32", "profileIndex", profileIndex },
                { "IN", "CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC", "profileDataPtr", profileDataPtr }
            })

            if (ret ~= 0) then
                print("Error while executing cpssDxChTunnelStartGenProfileTableEntrySet")
            end
        end

        -- set tti source id bits to default (0xFFF)
        ret, val = myGenWrapper("cpssDxChTtiSourceIdBitsOverrideSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "lookup", 0 },
            { "IN", "GT_U32", "overrideBitmapPtr", 0xFFF }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChTtiSourceIdBitsOverrideSet")
        end

        -- eVidx general configuration
        ret, val = myGenWrapper("cpssDxChL2MllLookupForAllEvidxEnableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_BOOL", "enable", false }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllLookupForAllEvidxEnableSet")
        end

        ret, val = myGenWrapper("cpssDxChL2MllLookupMaxVidxIndexSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "maxVidxIndex", 0xFFF }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllLookupMaxVidxIndexSet")
        end

        -- set mesh ID size - HW defaults offset = 1, size = 0
        ret, val = myGenWrapper("cpssDxChBrgEgrMeshIdConfigurationSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_BOOL", "enable", true },
            { "IN", "GT_U32", "meshIdOffset", 1 },
            { "IN", "GT_U32", "meshIdSize", 0 }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChBrgEgrMeshIdConfigurationSet")
        end

        -- Disable Exceptions caused by non-zero bit in 16 MS-bits of GRE header
        -- HW default - disable
        ret, val = myGenWrapper("cpssDxChTtiGreExtensionsEnableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_BOOL", "enable", false }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChTtiGreExtensionsEnableSet")
        end
    elseif (isVplsModeEnabled(devNum)) then
        ret, val = myGenWrapper("cpssDxChLogicalTargetMappingEnableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_BOOL", "enable", false }
            })
        if (ret ~= 0) then
            print("Error: could not enable logical port mapping: " .. returnCodes[ret])
            return false
        end
        ret, val = myGenWrapper("cpssDxChTtiPwCwExceptionCmdSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "CPSS_DXCH_TTI_PW_CW_EXCEPTION_ENT", "exceptionType", "CPSS_DXCH_TTI_PW_CW_EXCEPTION_PASSENGER_PW_TAG0_NOT_FOUND_ERROR_E" },
            { "IN", "CPSS_PACKET_CMD_ENT", "command", "CPSS_PACKET_CMD_FORWARD_E" }
            })
        if (ret ~= 0) then
            print("Error: could not set PW exception command: " .. returnCodes[ret])
            return false
        end
    end
    -- Cause using MAC_DA in legacy TTI_KEY_ETH key (Default mode)
    ret, val = myGenWrapper("cpssDxChTtiMacModeSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", "CPSS_DXCH_TTI_KEY_ETH_E" },
        { "IN", "CPSS_DXCH_TTI_MAC_MODE_ENT", "macMode", "CPSS_DXCH_TTI_MAC_MODE_DA_E" }
    })

    if (ret ~= 0) then
        print("Error while executing cpssDxChTtiMacModeSet")
    end

    ret = resetTpid(devNum)
    if (is_sip_5(devNum)) then
        ret = userDefineBytesReset(devNum)
        if (ret == false) then
            print("Error in resetting udb")
        end
    end

    return true
end

-- ************************************************************************
--
--  initDB
--        @description  initiate the data base for the used indices
--
--
local function initDB()
    local i
    local base = 4001; --[[0x1000;--]]
    for i = base, base + numberOfFreeEvid do
        table.insert(freeEvid, i)
    end
    if (is_sip_5(nil)) then
        base = l2MllLookupMaxVidxIndex;
    elseif (isVplsModeEnabled(devNum)) then
        base = l2MllLookupBaseVidxIndex;
    end
    for i = base, base + numberOfFreeEvidx do
        table.insert(freeEvidx, i)
    end

    -- for i = 0, numberOfFreeMll * 2, 2 do
    for i = 0, numberOfFreeMll do
        table.insert(freeMll, i)
    end

    for i = 1, numberOfTs do
        table.insert(freeTs, i)
    end
    local ttiStart = getTTIIndexInTCAM(devNum, 0)

    local newTtiIndex
    local minVal = ttiStart
    local maxVal = ttiStart + tcam_max_num_rules_per_floor
    local func=nil
    if (NumberOfEntriesIn_TTI_TCAM_table / tcam_max_num_rules_per_floor) <= 3 then
        func=function(index)
            return (index%12) < 6
        end
    end
    for i = 1, numberOfFreeTtiRules do
        newTtiIndex = getFreeIndexInTable(freeTtiBmp, minVal, maxVal, 3, 3, nil, func)
        table.insert(freeTti, newTtiIndex)
        freeTtiBmp[newTtiIndex] = true
    end

    isInitDB = true
    return true
end

-- ************************************************************************
--
--  dataBaseManagment
--        @description  manage the database according the DB type and the operation
--
--
local function dataBaseManagment(DBtype, DBoperation, index)
    if isInitDB == false then
        initDB()
    end
    local db
    local retIndex = nil

    -- choose database to operate on
    if (DBtype == VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_EVID_E"]) then db = freeEvid
    elseif (DBtype == VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_EVIDX_E"]) then db = freeEvidx
    elseif (DBtype == VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_L2MLL_E"]) then db = freeMll
    elseif (DBtype == VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_TS_E"]) then db = freeTs
    elseif (DBtype == VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_TTI_E"]) then db = freeTti
    else return false
    end

    if (DBoperation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ALLOC_E"]) then -- alloc new item from db
        retIndex = table.remove(db, 1)
        if (retIndex == nil) then
            print("can't allocate, database is full")
            return false
        end
        if db == freeTti then
            freeTtiBmp[retIndex] = nil
        end
    elseif (DBoperation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"]) then -- free item and return it to db
        table.insert(db,1, index)
        table.sort(db)
        if db == freeTti then
            freeTtiBmp[index] = true
        end
    end
    return true, retIndex
end

-- ************************************************************************
--
--  getAvailableRuleId
--        @description  gets the available TTI rule Id from the dataBase
--
--
local function getAvailableRuleId(devNum)

    local ret, index
    ret, index = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_TTI_E"], VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ALLOC_E"])
    if (ret == false) then
        return -1
    else
        return index
    end
end

-- ************************************************************************
--
--  getAvailableRuleId_fromEnd
--        @description  gets the available TTI rule Id from the end of dataBase
--
--
local function getAvailableRuleId_fromEnd(devNum)

    local ret, index
    index = table.remove(freeTti)
    if (index == nil) then
        print("can't allocate, database is full")
        return -1
    else
        return index
    end
end

-- ************************************************************************
--
--  getHwDevNum
--        @description  gets the hw device number from a given devNum
--
--
local function getHwDevNum(devNum)
    local ret, val
    ret, val = myGenWrapper("cpssDxChCfgHwDevNumGet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "OUT", "GT_HW_DEV_NUM", "hwDevNumPtr" }
    })

    if (ret ~= 0) then
        print("Error while executing cpssDxChCfgHwDevNumGet")
        return false
    end

    return true, tonumber(val.hwDevNumPtr)
end

-- ************************************************************************
--
--  vsiDomainCreate
--        @description  creates the domain for a given VSI
--
--
local function vsiDomainCreate(devNum, vsid, evid, floodingEvidx)
    local ret, val, mllEntry

    ret, val = myGenWrapper("cpssDxChBrgVlanMruProfileValueSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "mruIndex", 0 },
        { "IN", "GT_U32", "mruValue", 2000 }
    })

    if (ret ~= 0) then
        print("Error while executing cpssDxChBrgVlanMruProfileValueSet")
        return false
    end

    local vlanInfoPtr = {}
    local portsMembersPtr = {}
    local portsTaggingPtr = {}
    local portsTaggingCmdPtr = {}
    vlanInfoPtr.naMsgToCpuEn = true
    vlanInfoPtr.floodVidxMode = "CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E"
    vlanInfoPtr.floodVidx = floodingEvidx
    vlanInfoPtr.fidValue = evid
    vlanInfoPtr.ipv4UcastRouteEn = true;
    vlanInfoPtr.ipv4McastRouteEn = true;

    ret, val = myGenWrapper("cpssDxChBrgVlanEntryWrite", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U16", "vlanId", evid },
        { "IN", "CPSS_PORTS_BMP_STC", "portsMembersPtr", portsMembersPtr },
        { "IN", "CPSS_PORTS_BMP_STC", "portsTaggingPtr", portsTaggingPtr },
        { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfoPtr", vlanInfoPtr },
        { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmdPtr", portsTaggingCmdPtr }
    })

    if (ret ~= 0) then
        print("Error while executing cpssDxChBrgVlanEntryWrite")
        return false
    end

    -- configure flooding evidx and set mllPointer to an MLL which contains only the null port for a start
    ret, mllEntry = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_L2MLL_E"], VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ALLOC_E"])
    if (ret == false) then
        print("Error - can't create vsi domain")
        return false
    end

    local hwDevNum
    ret, hwDevNum = getHwDevNum(devNum)
    if (ret == false) then
        return false
    end
    local mllPairEntryPtr = {}
    mllPairEntryPtr.firstMllNode = {}
    mllPairEntryPtr.firstMllNode.egressInterface = {}
    mllPairEntryPtr.firstMllNode.egressInterface.type = "CPSS_INTERFACE_PORT_E"
    mllPairEntryPtr.firstMllNode.egressInterface.devPort = {}
    mllPairEntryPtr.firstMllNode.egressInterface.devPort.portNum = 62 -- null port
    mllPairEntryPtr.firstMllNode.egressInterface.devPort.devNum = hwDevNum
    mllPairEntryPtr.firstMllNode.last = true

    -- create MLL pair
    ret, val = myGenWrapper("cpssDxChL2MllPairWrite", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "mllPairEntryIndex", mllEntry },
        { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT", "mllPairWriteForm",
            "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
        { "IN", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr", mllPairEntryPtr }
    })

    if (ret ~= 0) then
        print("Error while executing cpssDxChL2MllPairWrite")
        return false
    end
    
    mllIndexStored = mllEntry
    
    if (is_sip_5(devNum)) then
        local lttEntryPtr = {}
        lttEntryPtr.mllPointer = mllEntry
        lttEntryPtr.entrySelector = "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E"

        ret, val = myGenWrapper("cpssDxChL2MllLttEntrySet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "index", (floodingEvidx - l2MllLookupMaxVidxIndex)},
            { "IN", "CPSS_DXCH_L2_MLL_LTT_ENTRY_STC", "lttEntryPtr", lttEntryPtr }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllLttEntrySet")
            return false
        end
        ret, val = myGenWrapper("cpssDxChTunnelStartEgessVlanTableServiceIdSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U16", "vlanId", evid },
            { "IN", "GT_U32", "vlanServiceId", vsid }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChTunnelStartEgessVlanTableServiceIdSet")
            return false
        end
    elseif (isVplsModeEnabled(devNum)) then
        ret, val = myGenWrapper("cpssDxChL2MllVidxEnableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U16", "vidx", floodingEvidx},
            { "IN", "GT_BOOL", "enable", true }
        })
        
        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllVidxEnableSet")
            return false
        end
    end
    print("eVid " .. to_string(evid) .. " created for VSI " .. to_string(vsid));
    return true
end

-- ************************************************************************
--
--  vsiDomainDestroy
--        @description  destroys the domain for a given VSI
--
--
--
local function vsiDomainDestroy(devNum, evid, floodingEvidx)
    local ret, val
    local mllEntry, mllIndex, mllIndex1
    local i
    local mllLastEntry = false;
    local mllPair
    local lttEntryPtr;

    ret, val = myGenWrapper("cpssDxChBrgVlanMruProfileValueSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "mruIndex", 0 },
        { "IN", "GT_U32", "mruValue", 0x5F2 } -- power on default
    })

    if (ret ~= 0) then
        print("Error while executing cpssDxChBrgVlanMruProfileValueSet")
        return false
    end
    if (is_sip_5(devNum)) then
        ret, val = myGenWrapper("cpssDxChTunnelStartEgessVlanTableServiceIdSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U16", "vlanId", evid },
            { "IN", "GT_U32", "vlanServiceId", 0 }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChTunnelStartEgessVlanTableServiceIdSet")
            return false
        end
    end

    ret, val = myGenWrapper("cpssDxChBrgVlanEntryInvalidate", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U16", "vlanId", evid }
    })

    if (ret ~= 0) then
        print("Error while executing cpssDxChBrgVlanEntryInvalidate")
        return false
    end
    if (is_sip_5(devNum)) then
        ret, val = myGenWrapper("cpssDxChL2MllLttEntryGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "index", (floodingEvidx - l2MllLookupMaxVidxIndex) },
            { "OUT", "CPSS_DXCH_L2_MLL_LTT_ENTRY_STC", "lttEntryPtr" }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllLttEntryGet")
            return false
        end

        lttEntryPtr = val.lttEntryPtr
        mllIndex = lttEntryPtr.mllPointer

        -- invalidate L2MllLttEntry
        lttEntryPtr = {};

        ret, val = myGenWrapper("cpssDxChL2MllLttEntrySet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "index", (floodingEvidx - l2MllLookupMaxVidxIndex) },
            { "IN", "CPSS_DXCH_L2_MLL_LTT_ENTRY_STC", "lttEntryPtr",  lttEntryPtr}
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllLttEntrySet")
            return false
        end
    elseif (isVplsModeEnabled(devNum)) then
        ret, val = myGenWrapper("cpssDxChL2MllVidxEnableSet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U16", "vidx", floodingEvidx},
            { "IN", "GT_BOOL", "enable", false }
        })
        
        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllVidxEnableSet")
            return false
        end
        
        mllIndex = mllIndexStored
    end
    -- erases used L2Mll pairs and frees them from DB
    for i = 0, numberOfL2Mll do

        ret, val = myGenWrapper("cpssDxChL2MllPairRead", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "mllPairEntryIndex", mllIndex },
            { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT",
                    "mllPairReadForm", "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
            { "OUT", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr" }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllPairRead")
            return false
        end

        mllPair = val.mllPairEntryPtr
        mllIndex1 = mllPair.nextPointer

        if ((mllPair.firstMllNode.last == true)
            or (mllPair.secondMllNode.last == true)) then -- indicates if the last is second
            mllLastEntry = true;
        end

        mllPair = {};
        ret, val = myGenWrapper("cpssDxChL2MllPairWrite", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "mllPairEntryIndex", mllIndex },
            { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT",
                    "mllPairReadForm", "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
            { "IN", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr", mllPair }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllPairWrite")
            return false
        end

        ret, mllEntry = dataBaseManagment(
            VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_L2MLL_E"],
            VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"],
            mllIndex)
        if (ret == false) then
            print("Error - can't free L2MLL entry")
            return false
        end

        mllIndex = mllIndex1

        if mllLastEntry == true then
            break
        end
    end

    return true
end
--
-- ************************************************************************
--
--  addRemoveEvidxEport
--        @description  handle the MLL
--                      add or remove eport to l2MLL
--
--
local function addRemoveEvidxEport(devNum, evidx, operation, eport, meshId)
    local ret, val, hwDevNum
    local mllEntry, mllIndex
    local i
    local mllIndexToDelete, mllIndexToUpdate, mllIndexLast, newMllIndex
    local firstOrSecond
    local startMllIndex, startFirstOrSecond
    local mllPair
    if (is_sip_5(devNum)) then
        -- start pair of <mllIndex, firstOrSecond> in the chain
        ret, val = myGenWrapper("cpssDxChL2MllLttEntryGet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "index", (evidx - l2MllLookupMaxVidxIndex) },
            { "OUT", "CPSS_DXCH_L2_MLL_LTT_ENTRY_STC", "lttEntryPtr" }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllLttEntryGet")
            return false
        end

        mllEntry = val.lttEntryPtr
        startMllIndex = mllEntry.mllPointer
        if mllEntry.entrySelector == "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E" then
            startFirstOrSecond = "first"
        else
            startFirstOrSecond = "second"
        end
    elseif (isVplsModeEnabled(devNum)) then
        startMllIndex = mllIndexStored
        startFirstOrSecond = "first"
    end

    ret, hwDevNum = getHwDevNum(devNum)
    if (ret == false) then
        return false
    end

    if (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ADD_E"]) then -- add operation
        -- finds L2Mll entry with given eport from start pair <startMllIndex, startFirstOrSecond>
        mllIndex = startMllIndex
        firstOrSecond = startFirstOrSecond
        for i = 0, numberOfL2Mll do

            ret, val = myGenWrapper("cpssDxChL2MllPairRead", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "GT_U32", "mllPairEntryIndex", mllIndex },
                { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT", "mllPairReadForm",
                    "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
                { "OUT", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr" }
            })

            if (ret ~= 0) then
                print("Error while executing cpssDxChL2MllPairRead")
                return false
            end

            mllPair = val.mllPairEntryPtr

            if (firstOrSecond == "first") then
                if (is_sip_5(devNum)) then
                    if (mllPair.firstMllNode.egressInterface.devPort.portNum == eport) then
    --                    print("Warning: eport already included in L2Mll chain")
                        return true
                    else
                        if (mllPair.firstMllNode.last == true) then
                            break
                        end
                        firstOrSecond = "second"
                    end
                elseif (isVplsModeEnabled(devNum)) then
                    if (mllPair.firstMllNode.egressInterface.devPort.portNum == getLogicalPortNum(eport)) and
                       (mllPair.firstMllNode.egressInterface.devPort.devNum == getLogicalDevNum(eport)) then
    --                    print("Warning: eport already included in L2Mll chain")
                        return true
                    else
                        if (mllPair.firstMllNode.last == true) then
                            break
                        end
                        firstOrSecond = "second"
                    end
                end
            end

            if (firstOrSecond == "second") then
                if (is_sip_5(devNum)) then
                    if (mllPair.secondMllNode.egressInterface.devPort.portNum == eport) then
    --                    print("Warning: eport already included in L2Mll chain")
                        return true
                    else
                        if (mllPair.secondMllNode.last == true) then
                            break
                        end
                    end
                elseif (isVplsModeEnabled(devNum)) then
                    if (mllPair.secondMllNode.egressInterface.devPort.portNum == getLogicalPortNum(eport)) and
                       (mllPair.secondMllNode.egressInterface.devPort.devNum == getLogicalDevNum(eport)) then
    --                    print("Warning: eport already included in L2Mll chain")
                        return true
                    else
                        if (mllPair.secondMllNode.last == true) then
                            break
                        end
                    end
                end
            end

            mllIndex = mllPair.nextPointer
            if (mllPair.entrySelector == "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E") then
                firstOrSecond = "first"
            else
                firstOrSecond = "second"
            end

            if (i == numberOfL2Mll) then -- if not found last link, return error
                print("Error: too long or wrong L2Mll chain (adding eport)")
                return false
            end
        end

        -- finds L2Mll entry with null port from start pair <startMllIndex, startFirstOrSecond>
        mllIndexToUpdate = nil
        mllIndex = startMllIndex
        firstOrSecond = startFirstOrSecond
        for i = 0, numberOfL2Mll do

            ret, val = myGenWrapper("cpssDxChL2MllPairRead", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "GT_U32", "mllPairEntryIndex", mllIndex },
                { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT", "mllPairReadForm",
                    "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
                { "OUT", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr" }
            })

            if (ret ~= 0) then
                print("Error while executing cpssDxChL2MllPairRead")
                return false
            end

            mllPair = val.mllPairEntryPtr

            if (firstOrSecond == "first") then
                if (mllPair.firstMllNode.egressInterface.devPort.portNum == 62) then
                    mllIndexToUpdate = mllIndex
                    break
                else
                    if (mllPair.firstMllNode.last == true) then
                        mllIndexLast = mllIndex
                        break
                    end
                    firstOrSecond = "second"
                end
            end

            if (firstOrSecond == "second") then
                if (mllPair.secondMllNode.egressInterface.devPort.portNum == 62) then
                    mllIndexToUpdate = mllIndex
                    break
                else
                    if (mllPair.secondMllNode.last == true) then
                        mllIndexLast = mllIndex
                        break
                    end
                end
            end

            mllIndex = mllPair.nextPointer
            if (mllPair.entrySelector == "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E") then
                firstOrSecond = "first"
            else
                firstOrSecond = "second"
            end

            if (i == numberOfL2Mll) then -- if not found last link, return error
                print("Error: too long or wrong L2Mll chain (adding eport)")
                return false
            end
        end

        if (mllIndexToUpdate == nil) then
            if (firstOrSecond == "first") then
                mllIndex = mllIndexLast
                mllPair.firstMllNode.last = false
                mllPair.secondMllNode.egressInterface.type = "CPSS_INTERFACE_PORT_E"
                if (is_sip_5(devNum)) then
                    mllPair.secondMllNode.egressInterface.devPort.portNum = eport
                    mllPair.secondMllNode.egressInterface.devPort.devNum = hwDevNum
                elseif (isVplsModeEnabled(devNum)) then
                    mllPair.secondMllNode.egressInterface.devPort.portNum = getLogicalPortNum(eport)
                    mllPair.secondMllNode.egressInterface.devPort.devNum = getLogicalDevNum(eport)
                    mllPair.secondMllNode.meshId = meshId
                end
                mllPair.secondMllNode.last = true
            else
                ret, newMllIndex = dataBaseManagment(
                    VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_L2MLL_E"],
                    VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ALLOC_E"])
                if (ret == false) then
                    return false
                end

                -- update the last entry to continue chain
                mllPair.secondMllNode.last = false
                mllPair.nextPointer = newMllIndex
                mllPair.entrySelector = "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E"

                ret, val = myGenWrapper("cpssDxChL2MllPairWrite", {
                    { "IN", "GT_U8", "devNum", devNum },
                    { "IN", "GT_U32", "mllPairEntryIndex", mllIndex },
                    { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT", "mllPairWriteForm",
                        "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
                    { "IN", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr", mllPair }
                })

                if (ret ~= 0) then
                    print("Error while executing cpssDxChL2MllPairWrite")
                    return false
                end

                -- new entry with added eport
                mllIndex = newMllIndex
                mllPair = {firstMllNode = {egressInterface = {devPort = {}}}}
                mllPair.firstMllNode.egressInterface.type = "CPSS_INTERFACE_PORT_E"
                if (is_sip_5(devNum)) then
                    mllPair.firstMllNode.egressInterface.devPort.portNum = eport
                    mllPair.firstMllNode.egressInterface.devPort.devNum = hwDevNum
                elseif (isVplsModeEnabled(devNum)) then
                    mllPair.firstMllNode.egressInterface.devPort.portNum = getLogicalPortNum(eport)
                    mllPair.firstMllNode.egressInterface.devPort.devNum = getLogicalDevNum(eport)
                    mllPair.firstMllNode.meshId = meshId
                end
                mllPair.firstMllNode.last = true
            end
        else
            if (is_sip_5(devNum)) then
                if (firstOrSecond == "first") then
                    mllPair.firstMllNode.egressInterface.devPort.portNum = eport
                else
                    mllPair.secondMllNode.egressInterface.devPort.portNum = eport
                end 
            elseif (isVplsModeEnabled(devNum)) then
                if (firstOrSecond == "first") then
                    mllPair.firstMllNode.egressInterface.devPort.portNum = getLogicalPortNum(eport)
                    mllPair.firstMllNode.egressInterface.devPort.devNum = getLogicalDevNum(eport)
                    mllPair.firstMllNode.meshId = meshId
                else
                    mllPair.secondMllNode.egressInterface.devPort.portNum = getLogicalPortNum(eport)
                    mllPair.secondMllNode.egressInterface.devPort.devNum = getLogicalDevNum(eport)
                    mllPair.secondMllNode.meshId = meshId
                end 
            end
            mllIndex = mllIndexToUpdate
        end

        ret, val = myGenWrapper("cpssDxChL2MllPairWrite", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "mllPairEntryIndex", mllIndex },
            { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT", "mllPairWriteForm",
                "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
            { "IN", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr", mllPair }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllPairWrite")
            return false
        end

    elseif (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_REMOVE_E"]) then

        -- finds L2Mll entry with given eport from start pair <startMllIndex, startFirstOrSecond>
        mllIndexToDelete = nil
        mllIndex = startMllIndex
        firstOrSecond = startFirstOrSecond
        for i = 0, numberOfL2Mll do

            ret, val = myGenWrapper("cpssDxChL2MllPairRead", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "GT_U32", "mllPairEntryIndex", mllIndex },
                { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT", "mllPairReadForm",
                    "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
                { "OUT", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr" }
            })

            if (ret ~= 0) then
                print("Error while executing cpssDxChL2MllPairRead")
                return false
            end

            mllPair = val.mllPairEntryPtr                  

            if (firstOrSecond == "first") then
                if (is_sip_5(devNum)) then
                    if (mllPair.firstMllNode.egressInterface.devPort.portNum == eport) then
                        mllIndexToDelete = mllIndex
                        break
                    else
                        if (mllPair.firstMllNode.last == true) then
                            print("Error: deleted eport not found in L2Mll chain: " .. tostring(eport))
                            return false
                        end
                        firstOrSecond = "second"
                    end
                elseif (isVplsModeEnabled(devNum)) then
                    if (mllPair.firstMllNode.egressInterface.devPort.portNum == getLogicalPortNum(eport)) and
                       (mllPair.firstMllNode.egressInterface.devPort.devNum == getLogicalDevNum(eport)) then
                        mllIndexToDelete = mllIndex
                        break
                    else
                        if (mllPair.firstMllNode.last == true) then
                            print("Error: deleted logical dev/port not found in L2Mll chain: " .. tostring(getLogicalDevNum(eport)) .. "/" .. tostring(getLogicalPortNum(eport)))
                            return false
                        end
                        firstOrSecond = "second"
                    end
                end  
            end

            if (firstOrSecond == "second") then
                if (is_sip_5(devNum)) then
                    if (mllPair.secondMllNode.egressInterface.devPort.portNum == eport) then
                        mllIndexToDelete = mllIndex
                        break
                    else
                        if (mllPair.secondMllNode.last == true) then
                            print("Error: deleted eport not found in L2Mll chain: " .. tostring(eport))
                            return false
                        end
                    end
                elseif (isVplsModeEnabled(devNum)) then
                    if (mllPair.secondMllNode.egressInterface.devPort.portNum == getLogicalPortNum(eport)) and
                       (mllPair.secondMllNode.egressInterface.devPort.devNum == getLogicalDevNum(eport)) then
                        mllIndexToDelete = mllIndex
                        break
                    else
                        if (mllPair.secondMllNode.last == true) then
                            print("Error: deleted logical dev/port not found in L2Mll chain: " .. tostring(getLogicalDevNum(eport)) .. "/" .. tostring(getLogicalPortNum(eport)))
                            return false
                        end
                    end
                end  
            end

            mllIndex = mllPair.nextPointer
            if (mllPair.entrySelector == "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E") then
                firstOrSecond = "first"
            else
                firstOrSecond = "second"
            end

            if (i == numberOfL2Mll) then -- if not found last link, return error
                print("Error: too long or wrong L2Mll chain (deleting eport)")
                return false
            end
        end

        if mllIndexToDelete == nil then
            print("Error: deleted eport not found in L2Mll Chain")
            return false
        end

        -- replace eport by null port
        if (firstOrSecond == "first") then
            mllPair.firstMllNode.egressInterface.devPort.portNum = 62
        else
            mllPair.secondMllNode.egressInterface.devPort.portNum = 62
        end

        ret, val = myGenWrapper("cpssDxChL2MllPairWrite", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "mllPairEntryIndex", mllIndexToDelete },
            { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT", "mllPairWriteForm",
                "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
            { "IN", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr", mllPair }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllPairWrite")
            return false
        end

        return true
    else
        print("Error, bad Param - operation unknown")
        return false
    end

    return true
end
--
-- ************************************************************************
--
--  addRemoveLogicalPortToMll
--        @description  handle the MLL
--                      add or remove eport to l2MLL
--
--
local function addRemoveLogicalPortToMll(devNum, operation, eport, meshId)
    local ret, val
    local mllEntry, mllIndex
    local i
    local mllIndexToDelete, mllIndexToUpdate, mllIndexLast, newMllIndex
    local firstOrSecond
    local startMllIndex, startFirstOrSecond
    local mllPair

    if (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ADD_E"]) then -- add operation

        -- configure flooding evidx and set mllPointer to an MLL which contains only the null port for a start
        ret, mllEntry = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_L2MLL_E"], VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ALLOC_E"])
        if (ret == false) then
            print("Error - can't create vsi domain")
            return false
        end

        portToMll[eport] = mllEntry
        
        local mllPairEntryPtr = {}
        mllPairEntryPtr.firstMllNode = {}
        mllPairEntryPtr.firstMllNode.egressInterface = {}
        mllPairEntryPtr.firstMllNode.egressInterface.type = "CPSS_INTERFACE_PORT_E"
        mllPairEntryPtr.firstMllNode.egressInterface.devPort = {}
        mllPairEntryPtr.firstMllNode.egressInterface.devPort.portNum = getLogicalPortNum(eport)
        mllPairEntryPtr.firstMllNode.egressInterface.devPort.devNum = getLogicalDevNum(eport)
        mllPairEntryPtr.firstMllNode.meshId = meshId
        mllPairEntryPtr.firstMllNode.last = true

        -- create MLL pair
        ret, val = myGenWrapper("cpssDxChL2MllPairWrite", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "mllPairEntryIndex", mllEntry },
            { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT", "mllPairWriteForm", "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
            { "IN", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr", mllPairEntryPtr }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllPairWrite")
            return false
        end

        ret, val = myGenWrapper("cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U8", "targetDevNum", getLogicalDevNum(eport) },
            { "IN", "GT_U8", "targetPortNum", getLogicalPortNum(eport) },
            { "IN", "GT_U32", "mllPairEntryIndex", mllEntry },
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet")
            return false
        end

    elseif (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_REMOVE_E"]) then
        
        local mllPairEntryPtr = {}
        mllPairEntryPtr.firstMllNode = {}
        mllPairEntryPtr.firstMllNode.egressInterface = {}
        mllPairEntryPtr.firstMllNode.egressInterface.type = "CPSS_INTERFACE_PORT_E"
        mllPairEntryPtr.firstMllNode.egressInterface.devPort = {}
        mllPairEntryPtr.firstMllNode.egressInterface.devPort.portNum = 62 --null port
        mllPairEntryPtr.firstMllNode.egressInterface.devPort.devNum = devNum
        mllPairEntryPtr.firstMllNode.last = true

        -- create MLL pair
        ret, val = myGenWrapper("cpssDxChL2MllPairWrite", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U32", "mllPairEntryIndex", portToMll[eport] },
            { "IN", "CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT", "mllPairWriteForm", "CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E" },
            { "IN", "CPSS_DXCH_L2_MLL_PAIR_STC", "mllPairEntryPtr", mllPairEntryPtr }
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MllPairWrite")
            return false
        end

        ret, val = myGenWrapper("cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet", {
            { "IN", "GT_U8", "devNum", devNum },
            { "IN", "GT_U8", "targetDevNum", getLogicalDevNum(eport) },
            { "IN", "GT_U8", "targetPortNum", getLogicalPortNum(eport) },
            { "IN", "GT_U32", "mllPairEntryIndex", portToMll[eport] },
        })

        if (ret ~= 0) then
            print("Error while executing cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet")
            return false
        end
        
        -- free L2MLL entries for Known DA
       dataBaseManagment( VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_L2MLL_E"], VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"],
            portToMll[eport])

    else
        print("Error, bad Param - operation unknown")
        return false
    end

    return true
end

local function vsiDbManage(vsiID, operation, evid, floodingEvidx)

    if (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_GET_E"]) then
        return true, vsiDb[vsiID]
    elseif (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_SET_E"]) then
        vsiDb[vsiID] = { evid = evid, floodingEvidx = floodingEvidx }
        return true
    elseif (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_REMOVE_E"]) then
        vsiDb[vsiID] = nil
        return true
    else
        print("Error, bad Param - operation unknown")
        return false
    end
end

-- ************************************************************************
--
--  vsiDomainManage
--        @description  manage the VSI domain with the following operations
--                      Create new vsi domain
--                      add eport to existing vsi
--                      remove eport from existing vsi
--
local function vsiDomainManage(devNum, vsiID, operation, eportNum, tagState, meshId)
    local ret, val, index
    local vsiEntry
    if (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_CREATE_E"]) then -- create new vsi domain
        ret, vsiEntry = vsiDbManage(vsiID, VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_GET_E"])
        if (ret == false) then
            return false
        end
        if (vsiEntry == nil) then
            -- allocating evid
            ret, index = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_EVID_E"],
                VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ALLOC_E"])
            if (ret == false) then
                return false
            end

            local evid = index

            -- allocating floodingEvidx
            ret, index = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_EVIDX_E"],
                VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ALLOC_E"])
            if (ret == false) then
                dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_EVID_E"],
                    VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], evid) -- free evid
                return false
            end
            local floodingEvidx = index

            ret = vsiDomainCreate(devNum, vsiID, evid, floodingEvidx)
            if (ret == false) then
                dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_EVID_E"],
                    VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], evid) -- free evid
                dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_EVIDX_E"],
                    VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], floodingEvidx) -- free floodingEvidx
                return false
            end
            
            eVidStored[vsiID] = evid
            
            ret = vsiDbManage(vsiID, VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_SET_E"], evid, floodingEvidx)
            if (ret == false) then
                return false
            end
        end

    elseif (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ADD_E"]) then -- add eport to VSI
        ret, vsiEntry = vsiDbManage(vsiID, VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_GET_E"])
        if (ret == false) then
            return false
        end
        if (vsiEntry == nil) then
            return false
        end

        local floodingEvidX = vsiEntry["floodingEvidx"]
        if (is_sip_5(devNum)) then
            -- set tagstate of eport to default
            ret, val = myGenWrapper("cpssDxChBrgVlanEgressPortTagStateSet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "GT_PORT_NUM", "portNum", eportNum },
                { "IN", "CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT", "tagCmd", tagState }
            })

            if (ret ~= 0) then
                print("Error while executing cpssDxChBrgVlanEgressPortTagStateSet")
                return false
            end
        end
        ret = addRemoveEvidxEport(devNum, floodingEvidX, VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ADD_E"], eportNum, meshId)
        if (ret == false) then
            return false
        end
        if (isVplsModeEnabled(devNum)) then
            ret = addRemoveLogicalPortToMll(devNum, VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ADD_E"], eportNum, meshId)
            if (ret == false) then
                return false
            end
        end
    elseif (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_REMOVE_E"]) then -- remove eport from VSI
        ret, vsiEntry = vsiDbManage(vsiID, VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_GET_E"])
        if (ret == false) then
            return false
        end
        if (vsiEntry == nil) then
            return false
        end

        -- free eVid
        if (eVidStored[vsiID] ~= nil) then
            dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_EVID_E"],
                VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], eVidStored[vsiID]) -- free evid
            eVidStored[vsiID] = nil
        end          
        
        if (is_sip_5(devNum)) then                    
            -- set tagstate of eport to default
            ret, val = myGenWrapper("cpssDxChBrgVlanEgressPortTagStateSet", {
                { "IN", "GT_U8", "devNum", devNum },
                { "IN", "GT_PORT_NUM", "portNum", eportNum },
                { "IN", "CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT", "tagCmd",
                    "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E" }
            })

            if (ret ~= 0) then
                print("Error while executing cpssDxChBrgVlanEgressPortTagStateSet")
                return false
            end
        end
        
        local floodingEvidX = vsiEntry["floodingEvidx"]
        ret = addRemoveEvidxEport(devNum, floodingEvidX, VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_REMOVE_E"], eportNum, meshId)
        if (ret == false) then
            return false
        end
        if (isVplsModeEnabled(devNum)) then
            ret = addRemoveLogicalPortToMll(devNum, VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_REMOVE_E"], eportNum, meshId)
            if (ret == false) then
                return false
            end
        end
    elseif (operation == VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_DESTROY_E"]) then -- destroy VSI
        ret, vsiEntry = vsiDbManage(vsiID, VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_GET_E"])
        if (ret == false) then
            print("Error in VSI entry searching");
            return false
        end
        if (vsiEntry == nil) then
            print("VSI entry not found: " .. tostring(vsiID));
            return true
        end
        print("VSI entry removing: " .. tostring(vsiID));
        vsiDomainDestroy(devNum, vsiEntry["evid"], vsiEntry["floodingEvidx"])
        dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_EVIDX_E"],
            VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], vsiEntry["floodingEvidx"]) -- free floodingEvidx
        vsiDbManage(vsiID, VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_REMOVE_E"])
        return true
    else
        print("Error, bad Param - operation unknown")
        return false
    end
    return true
end

-- ************************************************************************
local function isExistUniVsiId(vsiId)
    local key, val;

    for key, val in pairs(configuredPortsUni) do
        if (vsiId == val.vsi) then
            -- record found
            return true;
        end
    end

    return false;
end

local function isExistNniVsiId(vsiId)
    local key, val;

    for key, val in pairs(configuredPortsNni) do
        if (vsiId == val.vsi) then
            -- record found
            return true;
        end
    end

    return false;
end

-- ************************************************************************
-- conditionalCleanUp
--
--        @description  conditional Clean Up
--
local function conditionalCleanUp(devNum, vsiId)
    local ret;

    if (isExistUniVsiId(vsiId) == true) then
        return true;
    end
    if (isExistNniVsiId(vsiId) == true) then
        return true;
    end

    ret = vsiDomainManage(
        devNum, vsiId,
        VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_DESTROY_E"],
        nil --[[eport--]], nil --[[tagState--]], nil --[[meshId--]]);

    -- reset global configuration when there are no VSIs
    if (next(vsiDb) == nil) then
        vxlanNvgreGenericRestore(devNum);
    end

    return true;
end

-- ************************************************************************
--
--  createTTIRuleForUNI
--        @description  creates TTI rule for the "create uni" command
--
local function createTTIRuleForUNI(devNum, portNum, vid, srcMac, eport, vsi, index, pvid, isSecondRule)
    -- define full masks
    local fullMaskFor13 = 0x1FFF
    local fullMaskForBool = true
    local fullMaskForMac = {}
    for i = 0, 5 do
        fullMaskForMac[i] = 0xFF
    end

    local ret, val
    local patternTable, patternMaskTable, actionTable
    local evid = vsiDb[vsi]["evid"] -- get evid related to the given vsi


    patternTable = {}
    patternTable["eth"] = {}
    patternTable["eth"]["common"] = {}


    patternMaskTable = {}
    patternMaskTable["eth"] = {}
    patternMaskTable["eth"]["common"] = {}

    -- interface port match
    patternTable["eth"]["common"]["srcIsTrunk"] = false
    patternTable["eth"]["common"]["srcPortTrunk"] = portNum
    patternMaskTable["eth"]["common"]["srcIsTrunk"] = fullMaskForBool
    patternMaskTable["eth"]["common"]["srcPortTrunk"] = fullMaskFor13

    --vid match
    patternMaskTable["eth"]["isVlan1Exists"] = fullMaskForBool

    if(isSecondRule ~= true) then
        patternTable["eth"]["isVlan1Exists"] = true
        patternTable["eth"]["vid1"] = vid
        patternMaskTable["eth"]["vid1"] = 0xFFF
    else
        patternTable["eth"]["isVlan1Exists"] = false
        patternTable["eth"]["vid1"] = 0
        patternMaskTable["eth"]["vid1"] = 0
    end

    --[[ !!! to fix mask for srcMac --]]
    --src-mac match
    if (srcMac ~= nil) and (srcMac.isDummy == nil) then
        patternTable["eth"]["common"]["mac"] = srcMac
        patternMaskTable["eth"]["common"]["mac"] = srcMac
    end

    -- actions
    actionTable = {}
    actionTable["type2"] = {}

    actionTable["type2"]["command"] = "CPSS_PACKET_CMD_FORWARD_E"
    actionTable["type2"]["tag0VlanCmd"] = "CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E"
    actionTable["type2"]["tag0VlanId"] = evid
    actionTable["type2"]["tag1VlanCmd"] = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    actionTable["type2"]["tag1VlanId"] = pvid
    
    if (is_sip_5(devNum)) then
        actionTable["type2"]["sourceEPortAssignmentEnable"] = true
        actionTable["type2"]["sourceEPort"] = tonumber(eport)
        actionTable["type2"]["sourceIdSetEnable"] = true
        actionTable["type2"]["sourceId"] = 0
    elseif (isVplsModeEnabled(devNum)) then
        actionTable["type2"]["redirectCommand"] = "CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E"
        actionTable["type2"]["egressInterface"] = {}
        actionTable["type2"]["egressInterface"]["type"] = "CPSS_INTERFACE_PORT_E"
        actionTable["type2"]["egressInterface"]["devPort"] = {}
        actionTable["type2"]["egressInterface"]["devPort"]["devNum"] = getLogicalDevNum(eport)
        actionTable["type2"]["egressInterface"]["devPort"]["portNum"] = getLogicalPortNum(eport)
        actionTable["type2"]["sourceMeshIdSetEnable"] = true
        actionTable["type2"]["sourceMeshId"] = MESH_ID_0
    end

    ret, val = myGenWrapper("cpssDxChTtiRuleSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "index", index },
        { "IN", "CPSS_DXCH_TTI_RULE_TYPE_ENT", "ruleType", "CPSS_DXCH_TTI_RULE_ETH_E" },
        { "IN", "CPSS_DXCH_TTI_RULE_UNT_eth", "patternPtr", patternTable },
        { "IN", "CPSS_DXCH_TTI_RULE_UNT_eth", "maskPtr", patternMaskTable },
        { "IN", "CPSS_DXCH_TTI_ACTION_TYPE_ENT", "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT" },
        { "IN", "CPSS_DXCH_TTI_ACTION_UNT_type2", "actionPtr", actionTable }
    })
    if (ret ~= 0) then
        return false
    end

    return true
end


-- ************************************************************************
--
--  createDefaultTTIRuleForVpls
--        @description  creates TTI rule for drop not VPLS packets
--
local function createDefaultTTIRuleForVpls(devNum, portNum, index)
    local ret, val
    local patternTable, patternMaskTable, actionTable

    -- set pattern and mask for vsi
    patternTable = {}
    patternTable["mpls"] = {}
    patternTable["mpls"]["common"] = {}

    patternMaskTable = {}
    patternMaskTable["mpls"] = {}
    patternMaskTable["mpls"]["common"] = {}

    -- interface port match
    patternTable["mpls"]["common"]["srcIsTrunk"] = false
    patternTable["mpls"]["common"]["srcPortTrunk"] = portNum
    patternMaskTable["mpls"]["common"]["srcIsTrunk"] = true
    patternMaskTable["mpls"]["common"]["srcPortTrunk"] = 0x1FFF

    -- actions
    actionTable = {}
    actionTable["type2"] = {}
    actionTable["type2"]["command"] = "CPSS_PACKET_CMD_DROP_HARD_E"
    actionTable["type2"]["tag1VlanCmd"] = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

    ret, val = myGenWrapper("cpssDxChTtiRuleSet", {
        { "IN", "GT_U8",  "devNum", devNum },
        { "IN", "GT_U32", "index",  index },
        { "IN", "CPSS_DXCH_TTI_RULE_TYPE_ENT",    "ruleType",   "CPSS_DXCH_TTI_RULE_MPLS_E" },
        { "IN", "CPSS_DXCH_TTI_RULE_UNT_mpls",    "patternPtr", patternTable },
        { "IN", "CPSS_DXCH_TTI_RULE_UNT_mpls",    "maskPtr",    patternMaskTable },
        { "IN", "CPSS_DXCH_TTI_ACTION_TYPE_ENT",  "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT" },
        { "IN", "CPSS_DXCH_TTI_ACTION_UNT_type2", "actionPtr",  actionTable }
    })
    if (ret ~= 0) then
        print("createDefaultTTIRuleForVpls Error: cpssDxChTtiRuleSet")
        return false
    end

    return true
end


local function bindPortToTpidUtil(devNum, ports, isDefault, pwTagMode, profile)
    local ret, val

    for i, portNum in pairs(ports) do
        for j = 0, 1 do
            local tunnelEthType, ethType, tpidBmp
            if (j == 0) then
                if (is_sip_5(devNum)) then
                    ethType = "CPSS_VLAN_ETHERTYPE0_E"
                elseif (isVplsModeEnabled(devNum)) then
                    ethType = "CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E"
                    tunnelEthType = "CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE0_E"
                end
                if (pwTagMode == "CPSS_DXCH_TTI_PW_TAG_RAW_PW_MODE_E") then
                    tpidBmp = bit_shl(1,TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_0)
                else
                    tpidBmp = bit_shl(1,TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_1)
                end
            else
                if (is_sip_5(devNum)) then
                    ethType = "CPSS_VLAN_ETHERTYPE1_E"
                elseif (isVplsModeEnabled(devNum)) then
                    ethType = "CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E"
                    tunnelEthType = "CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_INGRESS_PASSENGER_ETHERTYPE1_E"
                end
                    tpidBmp = bit_shl(1,TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_0)
            end
            if (0 == profile) then
                tpidBmp = TPID_ENTRY_INDEX_INGRESS_ETHERTYPE_DEFAULT
            end
            if (is_sip_5(devNum)) then
                ret, val = myGenWrapper("cpssDxChBrgVlanPortIngressTpidProfileSet", {
                    { "IN", "GT_U8", "devNum", devNum },
                    { "IN", "GT_PORT_NUM", "portNum", portNum },
                    { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", ethType },
                    { "IN", "GT_BOOL", "isDefaultProfile", isDefault },
                    { "IN", "GT_U32", "profile", profile }
                })
                if ret ~= 0 then
                    print("Error while executing cpssDxChBrgVlanPortIngressTpidProfileSet")
                    return false
                end
            elseif (isVplsModeEnabled(devNum)) then
                ret, val = myGenWrapper("cpssDxChBrgVlanPortIngressTpidSet", {
                    { "IN", "GT_U8", "devNum", devNum },
                    { "IN", "GT_PORT_NUM", "portNum", portNum },
                    { "IN", "CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT", "ethMode", ethType },
                    { "IN", "GT_U32", "tpidBmp", tpidBmp }
                })
                if ret ~= 0 then
                    print("Error while executing cpssDxChBrgVlanPortIngressTpidSet")
                    return false
                end
                ret, val = myGenWrapper("cpssDxChBrgVlanPortIngressTpidSet", {
                    { "IN", "GT_U8", "devNum", devNum },
                    { "IN", "GT_PORT_NUM", "portNum", portNum },
                    { "IN", "CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT", "ethMode", tunnelEthType },
                    { "IN", "GT_U32", "tpidBmp", tpidBmp }
                })
                if ret ~= 0 then
                    print("Error while executing cpssDxChBrgVlanPortIngressTpidSet")
                    return false
                end
            
            end
        end
    end
    return true
end

local function bindPortToTpid(devNum, ports, pwTagMode, isDefault)
    return  bindPortToTpidUtil(devNum, ports, isDefault, pwTagMode, 7 --[[profile--]]);
end

local function unbindPortToTpid(devNum, ports, pwTagMode, isDefault)
    return  bindPortToTpidUtil(devNum, ports, isDefault, pwTagMode, 0 --[[profile--]]);
end

local function isExistUniPhyPort(portNum)
    local key, val;

    for key, val in pairs(configuredPortsUni) do
        if (portNum == val.portNum) then
            -- port found
            return true;
        end
    end

    -- not found in DB
    return nil;
end

local function isExistUniEPort(portNum, eport)
    local key, val;

    for key, val in pairs(configuredPortsUni) do
        if (portNum ~= val.portNum and eport == val.eport) then
            -- eport used for another physical port
            return false;
        end
    end

    for key, val in pairs(configuredPortsUni) do
        if (portNum == val.portNum and eport == val.eport) then
            -- correct ready eport
            return true;
        end
    end

    -- not found in DB
    return nil;
end


local function isExistUniConf(portNum, vsi, vid, eport, srcMac)
    local key, val;

    for key, val in pairs(configuredPortsUni) do
        if (vid == val.vid
            and srcMac.string == val.srcMac.string) then
            if (portNum ~= val.portNum
                or eport ~= val.eport
                or vsi ~= val.vsi) then
                    -- record found with another eport or Phy port or VSI
                    return false;
            end
        end
    end

    for key, val in pairs(configuredPortsUni) do
        if (portNum == val.portNum and vsi == val.vsi
                and vid == val.vid and eport == val.eport
                and srcMac.string == val.srcMac.string) then
            -- record found with correct parameters
            return true;
        end
    end

    -- record can be added
    return nil;
end

local function getUniConf(portNum, vid, srcMac)
    local key, val;

    for key, val in pairs(configuredPortsUni) do
        if (portNum == val.portNum and vid == val.vid
                and srcMac.string == val.srcMac.string) then
            return val;
        end
    end

    return nil;
end

local function countUniEPortVsiMembership(vsi, eport)
    local key, val;
    local num = 0;

    for key, val in pairs(configuredPortsUni) do
        if (vsi == val.vsi and eport == val.eport) then
            num = num + 1;
        end
    end

    return num;
end

local function countUniConfPhyPortUse(portNum)
    local key, val, num;

    num = 0;
    for key, val in pairs(configuredPortsUni) do
        if (portNum == val.portNum) then
            num = num + 1;
        end
    end

    return num;
end

local function countUniConfEPortUse(eport)
    local key, val, num;

    num = 0;
    for key, val in pairs(configuredPortsUni) do
        if (eport == val.eport) then
            num = num + 1;
        end
    end

    return num;
end

local function removeUniConf(portNum, vid, srcMac)
    local key, val;

    for key, val in pairs(configuredPortsUni) do
        if (portNum == val.portNum and vid == val.vid
                and srcMac.string == val.srcMac.string) then
            configuredPortsUni[key] = nil;
        end
    end
end

local function isExistNniPhyPort(portNum)
    local key, val;

    for key, val in pairs(configuredPortsNni) do
        if (portNum == val.portNum) then
            -- port found
            return true;
        end
    end

    -- not found in DB
    return nil;
end

local function isExistNniEPort(eport)
    local key, val;

    for key, val in pairs(configuredPortsNni) do
        if (eport == val.eport) then
            -- port found
            return true;
        end
    end

    -- not found in DB
    return nil;
end

local function getNniConf(portNum, vsi, localIP, remoteIP)
    local key, val
    for key, val in pairs(configuredPortsNni) do
        if (portNum == val.portNum and vsi == val.vsi) then
            if((localIp == nil or remoteIp == nil) or
               ( localIP.string == val.localIP.string and remoteIP.string == val.remoteIP.string) ) then
                return val;
            end
        end
    end

    return nil
end

local function removeNniConf(portNum, vsi, localIP, remoteIP)
    local key, val
    for key, val in pairs(configuredPortsNni) do
        if (portNum == val.portNum and vsi == val.vsi) then
            if((localIp == nil or remoteIp == nil) or
               ( localIP.string == val.localIP.string and remoteIP.string == val.remoteIP.string) ) then
                configuredPortsNni[key] = nil;
            end
        end
    end

    return true
end

-- ************************************************************************
---
-- createUNI
-- @description creates user network interface (UNI)
--
local function createUNI(params)
    local ret, val
    local devNum, portNum, portsForTpid, index, index1, index2
    local command_data = Command_Data()

    local vsi = tonumber(params["vsi-id"])
    local eport = tonumber(params["eport"])
    local srcMac = params["src-mac"]
    local tagState = params["tagState"]
    local eport_exist, phy_port_exist, eport_vsi_cnt;
    local already_configured;

    local pvid = 0 -- zero by default
    if (params["vid"] ~= nil) then
        pvid = tonumber(params["vid"])
    end

    local vid = 0 -- zero by default
    if (params["vlanId"] ~= nil) then
        vid = tonumber(params["vlanId"])
    end

    if (srcMac == nil) then
        srcMac = {string = "00:00:00:00:00:00", isDummy = true, "00", "00", "00", "00", "00", "00"};
    end
    if (tagState == nil) then
        tagState = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E";
    end

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()


    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus();

            if (true == isExistNniPhyPort(portNum)) then
                command_data:setFailDeviceAndLocalStatus();
                command_data:addError("physical port used for NNI ");
                return false;
            end

            if (true == isExistNniEPort(eport)) then
                command_data:setFailDeviceAndLocalStatus();
                command_data:addError("eport used for NNI ");
                return false;
            end

            -- found - true, can be added - nil, cannot be added - false
            eport_exist = isExistUniEPort(portNum, eport);
            if (eport_exist == false) then
                command_data:setFailDeviceAndLocalStatus();
                command_data:addError("eport used for other physical port");
                return false;
            end

            -- found - true, can be added - nil, cannot be added - false
            already_configured = isExistUniConf(portNum, vsi, vid, eport, srcMac);
            if (already_configured == false) then
                command_data:setFailDeviceAndLocalStatus();
                command_data:addError(
                    "other eport or Phy port or VSI used for the given vid/srcMac");
                return false;
            end

            phy_port_exist = isExistUniPhyPort(portNum);
            eport_vsi_cnt = countUniEPortVsiMembership(vsi, eport);

            if (already_configured ~= true) then
                if true == command_data["local_status"] then
                    vxlanNvgreGenericInit(devNum);
                    if (phy_port_exist ~= true) then
                        portsForTpid = { portNum }
                        ret = bindPortToTpid(devNum, portsForTpid, nil --[[pwTagMode--]], true)
                        if (ret == false) then
                            command_data:setFailDeviceAndLocalStatus()
                            break
                        end
                    end

                    if (phy_port_exist ~= true) then
                        -- enable tti per port
                        ret, val = myGenWrapper("cpssDxChTtiPortLookupEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", portNum },
                            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", "CPSS_DXCH_TTI_KEY_ETH_E" },
                            { "IN", "GT_BOOL", "enable", true }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not enable tti for port " .. portNum)
                        end
                    end

                    if (eport_vsi_cnt == 0) then
                        ret = vsiDomainManage(
                            devNum, vsi,
                            VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_CREATE_E"],
                            eport, tagState, nil --[[meshId--]]);
                        if (ret == false) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error managing the vsi")
                            break
                        end
                        -- add port to flooding evidx
                        ret = vsiDomainManage(
                            devNum, vsi,
                            VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ADD_E"],
                            eport, tagState, MESH_ID_0);
                        if (ret == false) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not add eport " .. eport .. " to vsi " .. vsi)
                        end

                        if (isVplsModeEnabled(devNum)) then
                            ret, val = myGenWrapper("cpssDxChLogicalTargetMappingDeviceEnableSet", {
                                { "IN", "GT_U8", "devNum", devNum },
                                { "IN", "GT_U8", "logicalDevNum", getLogicalDevNum(eport) },
                                { "IN", "GT_BOOL", "enable", true }
                                })
                            if (ret == false) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error: could not enable logical port mapping for this device: " .. returnCodes[ret])
                                break
                            end
                        end
                        
                        if (is_sip_5(devNum)) then
                            ret, val = myGenWrapper("cpssDxChBrgEgrPortMeshIdSet", {
                                { "IN", "GT_U8", "devNum", devNum },
                                { "IN", "GT_PORT_NUM", "portNum", eport },
                                { "IN", "GT_U32", "meshId", 0 }
                            })
                            if (ret ~= 0) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error: could not set meshId for port " .. eport)
                            end
                        end
                    end

                    -- TTI rule can be created only when VSI domain created
                    -- and flood VIDX allocated for it

                    -- create first rule
                    index = getAvailableRuleId()
                    if (index == -1) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("can't find available TTI index")
                        return false
                    end
                    ret = createTTIRuleForUNI(devNum, portNum, vid, srcMac, eport, vsi, index, pvid)

                    if (ret ~= true) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not create rule for port " .. portNum)
                    end
                    -- create second rule
                    index1 = getAvailableRuleId()
                    if (index1 == -1) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("can't find available TTI index1")
                        return false
                    end
                    ret = createTTIRuleForUNI(devNum, portNum, vid, srcMac, eport, vsi, index1, pvid, true --[[isSecondRule]])
                    if (ret ~= true) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not create rule1 for port " .. portNum)
                    end

                    if (phy_port_exist ~= true) then
                        -- create last rule
                        index2 = getAvailableRuleId_fromEnd()
                        if (index2 == -1) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("can't find available TTI index2")
                            return false
                        end
                        ret = createDefaultTTIRuleForVpls(devNum, portNum, index2)
                        if (ret ~= true) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not create rule2 for port " .. portNum)
                        end
                    end
                    
                    -- mapping between physical port and ePort
                    local ret, hwDevNum = getHwDevNum(devNum)
                    if (ret == false) then
                        return false
                    end

                    if (is_sip_5(devNum)) then
                        -- mapping between physical port and ePort
                        if (phy_port_exist ~= true) then
                            local physicalInfoPtr = {}
                            physicalInfoPtr.type = "CPSS_INTERFACE_PORT_E"
                            physicalInfoPtr.devPort = {}
                            physicalInfoPtr.devPort.devNum = hwDevNum
                            physicalInfoPtr.devPort.portNum = portNum

                            ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
                                { "IN", "GT_U8", "devNum", devNum },
                                { "IN", "GT_PORT_NUM", "portNum", eport },
                                { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfoPtr }
                            })
                            if (ret ~= 0) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error: could not map between eport " .. eport .. " and physical port " .. portNum)
                            end
                        end

                        if (eport_exist ~= true) then
                            -- tag state mode
                            ret, val = myGenWrapper("cpssDxChBrgVlanEgressPortTagStateModeSet", {
                                { "IN", "GT_U8", "devNum", devNum },
                                { "IN", "GT_PORT_NUM", "portNum", eport },
                                { "IN", "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT", "stateMode", "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E" }
                            })
                            if (ret ~= 0) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error: could not set tag state mode for eport " .. eport)
                            end
                            ret, val = myGenWrapper("cpssDxChBrgEgrFltVlanPortFilteringEnableSet", {
                                { "IN", "GT_U8", "devNum", devNum },
                                { "IN", "GT_PORT_NUM", "portNum", eport },
                                { "IN", "GT_BOOL", "enable", false }
                            })
                            if (ret ~= 0) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error: could not disable vlan filtering")
                            end
                        end
                    elseif (isVplsModeEnabled(devNum)) then       
                        ret, val = myGenWrapper("cpssDxChBrgVlanEgressFilteringEnable", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_BOOL", "enable", false }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not disable vlan filtering")
                        end
                        -- logical port mapping
                        logicalTargetGenConfUNIInit(devNum, hwDevNum, portNum, eport, tagState)
                    end
                    
                end

                table.insert(configuredPortsUni,
                    {portNum = portNum, vsi = vsi, vid = vid,
                    srcMac = srcMac, eport = eport,  tagState = tagState,
                    ttiIndex = index, ttiIndex1 = index1, ttiIndex2 = index2});
            end


            command_data:updateStatus()

            command_data:updatePorts()

        end
    end
    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
-- removeUNI
-- @description removes user network interface (UNI)
--
local function removeUNI(params)
    local ret, val
    local devNum, portNum, portsForTpid, index
    local command_data = Command_Data()

    local vid = tonumber(params["vlanId"]);
    local srcMac = params["src-mac"];
    local uni_cfg_entry, phy_port_use, eport_use;
    local vsi, eport, ttiIndex, ttiIndex1, ttiIndex2, physicalInfoPtr;
    local eport_exist, phy_port_exist, eport_vsi_cnt;

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()

    if (srcMac == nil) then
        srcMac = {string = "00:00:00:00:00:00", isDummy = true, "00", "00", "00", "00", "00", "00"};
    end

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus();

            uni_cfg_entry = getUniConf(portNum, vid, srcMac);
            if (uni_cfg_entry ~= nil) then
                vsi      = uni_cfg_entry["vsi"];
                eport    = uni_cfg_entry["eport"];
                ttiIndex = uni_cfg_entry["ttiIndex"];
                ttiIndex1 = uni_cfg_entry["ttiIndex1"];
                ttiIndex2 = uni_cfg_entry["ttiIndex2"];

                phy_port_use = countUniConfPhyPortUse(portNum);
                eport_use = countUniConfEPortUse(eport);
                eport_vsi_cnt = countUniEPortVsiMembership(vsi, eport);

                -- invalidate tti rule and free it's index
                ret, val = myGenWrapper("cpssDxChTtiRuleValidStatusSet", {
                    { "IN", "GT_U8", "devNum", devNum },
                    { "IN", "GT_U32", "index", ttiIndex },
                    { "IN", "GT_BOOL", "valid", false }
                })
                if (ret ~= 0) then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error: could not invalidate tti rule")
                end

                ret, val = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_TTI_E"],
                    VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], ttiIndex);
                if (ret == false) then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error: can't free index of tti")
                end

                -- invalidate tti rule 1 and free it's index
                ret, val = myGenWrapper("cpssDxChTtiRuleValidStatusSet", {
                    { "IN", "GT_U8", "devNum", devNum },
                    { "IN", "GT_U32", "index", ttiIndex1 },
                    { "IN", "GT_BOOL", "valid", false }
                })
                if (ret ~= 0) then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error: could not invalidate tti rule")
                end

                ret, val = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_TTI_E"],
                    VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], ttiIndex1);
                if (ret == false) then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error: can't free index of tti")
                end

                -- invalidate tti rule 2 and free it's index
                ret, val = myGenWrapper("cpssDxChTtiRuleValidStatusSet", {
                    { "IN", "GT_U8", "devNum", devNum },
                    { "IN", "GT_U32", "index", ttiIndex2 },
                    { "IN", "GT_BOOL", "valid", false }
                })
                if (ret ~= 0) then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error: could not invalidate tti rule")
                end

                ret, val = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_TTI_E"],
                    VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], ttiIndex2);
                if (ret == false) then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error: can't free index of tti")
                end

                if (isVplsModeEnabled(devNum)) then
                    ret, val = myGenWrapper("cpssDxChLogicalTargetMappingDeviceEnableSet", {
                        { "IN", "GT_U8", "devNum", devNum },
                        { "IN", "GT_U8", "logicalDevNum", getLogicalDevNum(eport) },
                        { "IN", "GT_BOOL", "enable", false }
                        })
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not enable logical port mapping for this device: " .. returnCodes[ret])
                        break
                    end
                end 

                if (phy_port_use <= 1) then
                    -- restore default TPID use
                    portsForTpid = { portNum }
                    ret = unbindPortToTpid(devNum, portsForTpid, nil --[[pwTagMode--]], true);
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                    end
                    -- disable tti per port
                    ret, val = myGenWrapper("cpssDxChTtiPortLookupEnableSet", {
                        { "IN", "GT_U8", "devNum", devNum },
                        { "IN", "GT_PORT_NUM", "portNum", portNum },
                        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", "CPSS_DXCH_TTI_KEY_ETH_E" },
                        { "IN", "GT_BOOL", "enable", false }
                    })
                    if (ret ~= 0) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not enable tti for port " .. portNum)
                    end
                end

                if (eport_use <= 1) then

                    if (is_sip_5(devNum)) then
                        ret, val = myGenWrapper("cpssDxChBrgEgrPortMeshIdSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "GT_U32", "meshId", 0 }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not set meshId for port " .. eport)
                        end
                        -- default mapping eport to 0/0
                        physicalInfoPtr = {}
                        physicalInfoPtr.type = "CPSS_INTERFACE_PORT_E"
                        physicalInfoPtr.devPort = {}
                        physicalInfoPtr.devPort.devNum = 0
                        physicalInfoPtr.devPort.portNum = 0

                        ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfoPtr }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not map between eport " .. eport .. " and physical port " .. portNum)
                        end
                        
                        -- tag state mode
                        ret, val = myGenWrapper("cpssDxChBrgVlanEgressPortTagStateModeSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT", "stateMode", "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E" }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not set tag state mode for eport " .. eport)
                        end
                        ret, val = myGenWrapper("cpssDxChBrgEgrFltVlanPortFilteringEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "GT_BOOL", "enable", false }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not disable vlan filtering")
                        end
                    elseif (isVplsModeEnabled(devNum)) then        
                        ret, val = myGenWrapper("cpssDxChBrgVlanEgressFilteringEnable", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_BOOL", "enable", true }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not enable vlan filtering")
                        end               
                        logicalTargetGenConfRestore(devNum, eport)
                    end
                end
                if (eport_vsi_cnt <= 1) then
                    ret = vsiDomainManage(
                        devNum, vsi,
                        VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_REMOVE_E"],
                        eport, nil --[[tagState--]], nil --[[meshId--]]);
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error managing the vsi")
                    end
                end
            end
        end
        -- clear records from DB
        for iterator, devNum, portNum in command_data:getPortIterator() do
            removeUniConf(portNum, vid, srcMac);
        end

        for iterator, devNum, portNum in command_data:getPortIterator() do
            conditionalCleanUp(devNum, vsi);
        end

    end
end

-- ************************************************************************
--
--  createTTIRuleForNNI
--        @description  creates TTI rule for the "create nni" command, vxVlan or nvgre case
--
local function createTTIRuleForNNI(devNum, portNum, index, srcIP, dstIP, udpPort, vsi, eport, isVxlan)
    -- define full masks
    local fullMaskFor13 = 0x1FFF
    local fullMaskForBool = true
    local ret, val, i
    local patternTable, patternMaskTable, actionTable
    local pclId
    local lsbOfUdpPort, msbOfUdpPort
    local evid = vsiDb[vsi]["evid"] -- gets evid related to the vsi

    if isVxlan == true then
        pclId = 1;
    else
        pclId = 2;
    end

    patternTable = {}
    patternTable["udbArray"] = {}

    patternMaskTable = {}
    patternMaskTable["udbArray"] = {}

    -- PCL ID + isTrunk
    local pclTbl = splitToBytes(pclId, 2)
    patternTable["udbArray"][0] = pclTbl[2]
    patternMaskTable["udbArray"][0] = 0x3F

    patternTable["udbArray"][1] = pclTbl[1]
    patternMaskTable["udbArray"][1] = 0x1F

    -- port
    local portTbl = splitToBytes(portNum, 2)
    patternTable["udbArray"][2] = portTbl[2]
    patternMaskTable["udbArray"][2] = 0xFF

    patternTable["udbArray"][3] = portTbl[1]
    patternMaskTable["udbArray"][3] = 0x1F

    local ipIdx = 4
    -- dest IP
    for i = 4, 7 do
        patternTable["udbArray"][i] = tonumber(dstIP[ipIdx]);
        patternMaskTable["udbArray"][i] = 0xFF
        ipIdx = ipIdx - 1
    end

    if (isVxlan == true) then
        -- src IP - for VXLAN only
        ipIdx = 4
        for i = 8, 11 do
                patternTable["udbArray"][i] = tonumber(srcIP[ipIdx]);
                patternMaskTable["udbArray"][i] = 0xFF
                ipIdx = ipIdx - 1
        end

        -- udp-port - for VXLAN only
        local portTbl = splitToBytes(udpPort, 2)

        patternTable["udbArray"][12] = portTbl[1]
        patternMaskTable["udbArray"][12] = 0xff

        patternTable["udbArray"][13] = portTbl[2]
        patternMaskTable["udbArray"][13] = 0xff
     end



    -- set pattern and mask for vsi
    local vsiTbl = splitToBytes(vsi, 3)
    for i = 1, 3 do
        patternTable["udbArray"][13 + i] = vsiTbl[4 - i]
        patternMaskTable["udbArray"][13 + i] = 0xff
    end

    -- actions
    actionTable = {}
    actionTable["type2"] = {}

    actionTable["type2"]["command"] = "CPSS_PACKET_CMD_FORWARD_E"
    actionTable["type2"]["tag0VlanCmd"] = "CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E"
    actionTable["type2"]["tag0VlanId"] = evid
    actionTable["type2"]["tag1VlanCmd"] = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    actionTable["type2"]["ttPassengerPacketType"] = "CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E"
    actionTable["type2"]["tunnelTerminate"] = true
    actionTable["type2"]["sourceEPortAssignmentEnable"] = true
    actionTable["type2"]["sourceEPort"] = tonumber(eport)
    actionTable["type2"]["sourceIdSetEnable"] = true
    actionTable["type2"]["sourceId"] = sourceIdIndication

    ret, val = myGenWrapper("cpssDxChTtiRuleSet", {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "index", index },
        { "IN", "CPSS_DXCH_TTI_RULE_TYPE_ENT", "ruleType", "CPSS_DXCH_TTI_RULE_UDB_30_E" },
        { "IN", "CPSS_DXCH_TTI_RULE_UNT_udbArray", "patternPtr", patternTable },
        { "IN", "CPSS_DXCH_TTI_RULE_UNT_udbArray", "maskPtr", patternMaskTable },
        { "IN", "CPSS_DXCH_TTI_ACTION_TYPE_ENT", "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT" },
        { "IN", "CPSS_DXCH_TTI_ACTION_UNT_type2", "actionPtr", actionTable }
    })
    if (ret ~= 0) then
        print("Error: cpssDxChTtiRuleSet")
        return false
    end

    return true
end

-- ************************************************************************
--
--  createVplsTtiRuleForNNI
--        @description  creates TTI rule for the "create nni" command, vpls case
--
local function createVplsTtiRuleForNNI(devNum, portNum, index, vsi, eport, pvid, params)
    -- define full masks
    local fullMaskFor13 = 0x1FFF
    local fullMaskForBool = true
    local ret, val, i
    local patternTable, patternMaskTable, actionTable
    local evid = vsiDb[vsi]["evid"] -- gets evid related to the vsi

    local pwTagMode     = params["pw-tag-mode"]
    local inTunnelLabel = params["in-tunnel-label"]
    local pwLabel       = params["pw-label"]
    local hwDevNum
    
    -- set pattern and mask for vsi
    patternTable = {}
    patternTable["mpls"] = {}
    patternTable["mpls"]["common"] = {}

    patternMaskTable = {}
    patternMaskTable["mpls"] = {}
    patternMaskTable["mpls"]["common"] = {}

    -- interface port match
    patternTable["mpls"]["common"]["srcIsTrunk"] = false
    patternTable["mpls"]["common"]["srcPortTrunk"] = portNum
    patternMaskTable["mpls"]["common"]["srcIsTrunk"] = fullMaskForBool
    patternMaskTable["mpls"]["common"]["srcPortTrunk"] = fullMaskFor13

    patternTable["mpls"]["label0"] = inTunnelLabel
    patternMaskTable["mpls"]["label0"] = 0x3FFF --14 bits

    patternTable["mpls"]["label1"] = pwLabel
    patternMaskTable["mpls"]["label1"] = 0x3FFF --14 bits

    -- actions
    actionTable = {}
    actionTable["type2"] = {}
    actionTable["type2"]["command"] = "CPSS_PACKET_CMD_FORWARD_E"
    actionTable["type2"]["tag0VlanCmd"] = "CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E"
    actionTable["type2"]["tag0VlanId"] = evid
    actionTable["type2"]["tag1VlanCmd"] = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    actionTable["type2"]["tag1VlanId"] = pvid -- eport pvid
    actionTable["type2"]["ttPassengerPacketType"] = "CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E"
    actionTable["type2"]["tunnelTerminate"] = true

    ret, hwDevNum = getHwDevNum(devNum)
    if (ret == false) then
        return false
    end
    
    if (is_sip_5(devNum)) then
        actionTable["type2"]["sourceEPortAssignmentEnable"] = true
        actionTable["type2"]["sourceEPort"] = tonumber(eport)
        actionTable["type2"]["sourceIdSetEnable"] = true
        actionTable["type2"]["sourceId"] = sourceIdIndication
    elseif (isVplsModeEnabled(devNum)) then
        actionTable["type2"]["pwTagMode"] = pwTagMode
        actionTable["type2"]["redirectCommand"] = "CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E"
        actionTable["type2"]["egressInterface"] = {}
        actionTable["type2"]["egressInterface"]["hwDevNum"] = hwDevNum
        actionTable["type2"]["egressInterface"]["type"] = "CPSS_INTERFACE_PORT_E"
        actionTable["type2"]["egressInterface"]["devPort"] = {}
        actionTable["type2"]["egressInterface"]["devPort"]["devNum"] = getLogicalDevNum(eport)
        actionTable["type2"]["egressInterface"]["devPort"]["portNum"] = getLogicalPortNum(eport)
        actionTable["type2"]["sourceMeshIdSetEnable"] = true
        actionTable["type2"]["sourceMeshId"] = MESH_ID_2
        actionTable["type2"]["unknownSaCommandEnable"] = false
        actionTable["type2"]["cwBasedPw"] = false    
    end

    ret, val = myGenWrapper("cpssDxChTtiRuleSet", {
        { "IN", "GT_U8",  "devNum", devNum },
        { "IN", "GT_U32", "index",  index },
        { "IN", "CPSS_DXCH_TTI_RULE_TYPE_ENT",    "ruleType",   "CPSS_DXCH_TTI_RULE_MPLS_E" },
        { "IN", "CPSS_DXCH_TTI_RULE_UNT_mpls",    "patternPtr", patternTable },
        { "IN", "CPSS_DXCH_TTI_RULE_UNT_mpls",    "maskPtr",    patternMaskTable },
        { "IN", "CPSS_DXCH_TTI_ACTION_TYPE_ENT",  "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT" },
        { "IN", "CPSS_DXCH_TTI_ACTION_UNT_type2", "actionPtr",  actionTable }
    })
    if (ret ~= 0) then
        print("createVplsTtiRuleForNNI Error: cpssDxChTtiRuleSet")
        return false
    end

    return true
end


-- ************************************************************************
--
--  createNNI
--        @description  creates network network interface (NNI)
--
local function createNNI(params)
    local ret, val
    local devNum, portNum, portsForTpid
    local command_data = Command_Data()
    local index, index2
    local vsi = tonumber(params["vsi-id"])
    local eport = tonumber(params["eport"])
    local localIP = params["localIP"]
    local remoteIP = params["remoteIP"]
    local nextHopMac = params["nextHopMac"]
    local tagState = params["tagState"];
    local pwLabel = params["pw-label"];
    local inTunnelLabel = params["in-tunnel-label"];
    local outTunnelLabel = params["out-tunnel-label"];
    local pwTagMode = params["pw-tag-mode"];
    local pTag = params["p-tag"];
    local udpPort = 4789
    local isVxlan = false
    local isVpls  = false
    local nniType;
    local tsIndex;
    local entry;
    local phy_port_exist;
    local pwTagState = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E"
    local apiName

    local pvid = 0 -- zero by default
    if (params["vid"] ~= nil) then
        pvid = tonumber(params["vid"])
    end

    if (params["nvgre"] == true) then
        if (tagState ~= nil) then
            print("Error: NVGRE tag state must not be specified, always untagged");
            return false
        end
    end

    if (tagState == nil) then
        tagState = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E";
    end

    if (params["vpls"] == true) then
        isVpls = true
        nniType = "vpls";
    elseif(params["vxlan"] == true) then
        isVxlan = true
        nniType = "vxlan";
    elseif (params["nvgre"] == true) then
        isVxlan = false
        nniType = "nvgre";
    else
        print("Error: bad params")
        return false
    end

    if (true == isVpls) and (false == is_supported_feature(devEnv.dev, "VPLS")) then
        print("Command is not supported for current device")
        return
    end 
              
    -- Common variables initialization
    command_data:initInterfaceDevPortRange()

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            if(isVpls ~= true) then
                ret = isExistNniEPort(eport);
                if (ret == true) then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error: eport already used for NNI")
                    print("Error: eport already used for NNI")
                    break
                end
            end

            ret = countUniConfEPortUse(eport);
            if (ret ~= 0) then
                command_data:setFailDeviceAndLocalStatus()
                command_data:addError("Error: eport already used for UNI")
                print("Error: eport already used for UNI")
                break
            end

            phy_port_exist = isExistNniPhyPort(portNum);
            
            if(isVpls ~= true) then
                entry = getNniConf(portNum, vsi, localIP, remoteIP);
                if (entry ~= nil) then
                    if (nniType ~= entry.nniType
                        or eport ~= entry.eport
                        or nextHopMac.string ~= entry.nextHopMac.string
                        or tagState ~= entry.tagState) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError(
                                "Error: already exists with other data");
                            print("Error: already exists with other data");
                            break;
                    end
                end
            end

            if (entry == nil) then
                if true == command_data["local_status"] then
                    vxlanNvgreGenericInit(devNum);

                    ret = vsiDomainManage(
                        devNum, vsi,
                        VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_CREATE_E"],
                        eport, tagState, nil --[[meshId--]]);
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error managing the vsi")
                        break
                    end
                    if (isVplsModeEnabled(devNum)) then
                        ret, val = myGenWrapper("cpssDxChLogicalTargetMappingDeviceEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_U8", "logicalDevNum", getLogicalDevNum(eport) },
                            { "IN", "GT_BOOL", "enable", true }
                            })
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not enable logical port mapping for this device: " .. returnCodes[ret])
                        break
                        end
                    end                    
                    if (is_sip_5(devNum)) then
                        portsForTpid = { eport }
                    elseif (isVplsModeEnabled(devNum)) then
                        portsForTpid = { portNum }
                    end
                    ret = bindPortToTpid(devNum, portsForTpid, pwTagMode, false)
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                        break
                    end

                    index = getAvailableRuleId()

                    if isVpls == true then
                        ret = createVplsTtiRuleForNNI(devNum, portNum, index, vsi, eport, pvid, params)
                    else
                        ret = createTTIRuleForNNI(devNum, portNum, index, remoteIP, localIP, udpPort, vsi, eport, isVxlan)
                    end
                    if (ret ~= true) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not create rule for port " .. portNum)
                    end

                    if isVpls == true then
                        if (phy_port_exist ~= true) then
                            -- create last rule
                            index2 = getAvailableRuleId_fromEnd()
                            if (index2 == -1) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("can't find available TTI index2")
                                return false
                            end
                            ret = createDefaultTTIRuleForVpls(devNum, portNum, index2)
                            if (ret ~= true) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error: could not create rule2 for port " .. portNum)
                            end
                        end
                    end

                    -- enable tti per port
                    if isVpls == true then
                        ret, val = myGenWrapper("cpssDxChTtiPortLookupEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", portNum },
                            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", "CPSS_DXCH_TTI_KEY_MPLS_E" },
                            { "IN", "GT_BOOL", "enable", true }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not enable tti for port " .. portNum)
                        end
                    else
                        ret, val = myGenWrapper("cpssDxChTtiPortLookupEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", portNum },
                            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", "CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E" },
                            { "IN", "GT_BOOL", "enable", true }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not enable tti for port " .. portNum)
                        end

                        ret, val = myGenWrapper("cpssDxChTtiPortLookupEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", portNum },
                            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", "CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E" },
                            { "IN", "GT_BOOL", "enable", true }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not enable tti for port " .. portNum)
                        end
                    end

                    -- set physical info for eport
                    ret, tsIndex = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_TS_E"],
                        VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ALLOC_E"])
                    if (ret == false) then
                        return false
                    end
                    if (is_sip_5(devNum)) then
                        local egressInfoPtr = {}
                        egressInfoPtr.tunnelStart = true
                        egressInfoPtr.tunnelStartPtr = tsIndex
                        egressInfoPtr.tsPassengerPacketType = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
                        egressInfoPtr.arpPtr = 0
                        egressInfoPtr.modifyMacSa = false
                        egressInfoPtr.modifyMacDa = false

                        ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", egressInfoPtr }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not Set physical info for eport " .. eport)
                        end
                    end

                    -- tunnel start entry set
                    if (isVpls ~= true) then
                        local ipHeaderProtocol, profileIndex, udpDstPort;
                        local greFlagsAndVersion, greProtocolForEthernet

                        if (isVxlan == true) then
                            ipHeaderProtocol = "CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E"
                            profileIndex = 0
                            udpDstPort = udpPort
                            greFlagsAndVersion = 0
                            greProtocolForEthernet = 0
                        else
                            ipHeaderProtocol = "CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E"
                            profileIndex = 1
                            udpDstPort = 0
                            greFlagsAndVersion = 0x2000
                            greProtocolForEthernet = 0x6558 --[[ NvGre stamp --]]
                        end

                        local configPtr = {}
                        configPtr.ipv4Cfg = {}
                        configPtr.ipv4Cfg.macDa = nextHopMac
                        configPtr.ipv4Cfg.dontFragmentFlag = false
                        configPtr.ipv4Cfg.ttl = 64
                        configPtr.ipv4Cfg.destIp = remoteIP
                        configPtr.ipv4Cfg.srcIp = localIP
                        configPtr.ipv4Cfg.profileIndex = profileIndex
                        configPtr.ipv4Cfg.udpDstPort = udpDstPort
                        configPtr.ipv4Cfg.ipHeaderProtocol = ipHeaderProtocol
                        configPtr.ipv4Cfg.greFlagsAndVersion = greFlagsAndVersion
                        configPtr.ipv4Cfg.greProtocolForEthernet = greProtocolForEthernet

                        ret, val = myGenWrapper("cpssDxChTunnelStartEntrySet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_U32", "routerArpTunnelStartLineIndex", tsIndex },
                            { "IN", "CPSS_TUNNEL_TYPE_ENT", "tunnelType", "CPSS_TUNNEL_GENERIC_IPV4_E" },
                            { "IN", "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_ipv4Cfg", "configPtr", configPtr }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not Set ts for index  " .. tsIndex)
                        end

                    else -- vpls case

                        local configPtr = {}
                        configPtr.mplsCfg = {}
                        configPtr.mplsCfg.macDa = nextHopMac
                        configPtr.mplsCfg.label1 = pwLabel
                        configPtr.mplsCfg.label2 = outTunnelLabel
                        configPtr.mplsCfg.setSBit = true
                        --configPtr.mplsCfg.tagEnable = true;
                        --configPtr.mplsCfg.label3 = params["in-tunnel-label"]
                        configPtr.mplsCfg.numLabels = 2

                        if ( params["cw-value"] ~= nil) then
                            configPtr.mplsCfg.controlWordEnable = true
                            configPtr.mplsCfg.controlWordIndex = tonumber(params["cw-value"])
                        end

                        ret, val = myGenWrapper("cpssDxChTunnelStartEntrySet", {
                            { "IN", "GT_U8",  "devNum", devNum },
                            { "IN", "GT_U32", "routerArpTunnelStartLineIndex", tsIndex },
                            { "IN", "CPSS_TUNNEL_TYPE_ENT", "tunnelType", "CPSS_TUNNEL_X_OVER_MPLS_E" },
                            { "IN", "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_mplsCfg", "configPtr", configPtr }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("cpssDxChTunnelStartEntrySet Error: could not Set ts for index  " .. tsIndex)
                        end
                    end
                    if (is_sip_5(devNum)) then
                        ret, val = myGenWrapper("cpssDxChBrgEgrPortMeshIdSet", {
                            { "IN", "GT_U8",       "devNum",  devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "GT_U32",      "meshId",  1 }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not set meshId for eport " .. eport)
                        end
                    end

                    -- add port to flooding evidx
                    ret = vsiDomainManage(
                        devNum, vsi,
                        VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_ADD_E"],
                        eport, tagState, MESH_ID_2 );
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not add eport " .. eport .. " to vsi " .. vsi)
                    end

                    if (is_sip_5(devNum)) then
                        -- tag state mode
                        ret, val = myGenWrapper("cpssDxChBrgVlanEgressPortTagStateModeSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT", "stateMode", "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E" }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not set tag state mode for eport " .. eport)
                        end
                    end
                    
                    -- set tagstate of eport to tag1 for RAW mode, or {tag0, tag1} for TAG mode
                    if (isVpls == true) then

                        -- set port tag state

                        if (pwTagMode == 'CPSS_DXCH_TTI_PW_TAG_RAW_PW_MODE_E') then
                            if (tagState == "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E") then
                                pwTagState = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E"
                            else
                                pwTagState = "CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E"
                            end
                            -- do not drop packets without pw-tag
                            if (is_sip_5(devNum)) then
                                ret, val = myGenWrapper("cpssDxChBrgVlanPortAccFrameTypeSet", {
                                    { "IN", "GT_U8", "devNum", devNum },
                                    { "IN", "GT_PORT_NUM", "portNum", eport },
                                    { "IN", "CPSS_PORT_ACCEPT_FRAME_TYPE_ENT", "keyType", "CPSS_PORT_ACCEPT_FRAME_ALL_E" }
                                })
                                if (ret ~= 0) then
                                    command_data:setFailDeviceAndLocalStatus()
                                    command_data:addError("Error: could not enable access frame type for eport " .. eport)
                                end
                            elseif (isVplsModeEnabled(devNum)) then
                                -- ret, val = myGenWrapper("cpssDxChBrgVlanPortAccFrameTypeSet", {
                                --    { "IN", "GT_U8", "devNum", devNum },
                                --    { "IN", "GT_PORT_NUM", "portNum", portNum },
                                --    { "IN", "CPSS_PORT_ACCEPT_FRAME_TYPE_ENT", "keyType", "CPSS_PORT_ACCEPT_FRAME_ALL_E" }
                                -- })
                                -- if (ret ~= 0) then
                                --    command_data:setFailDeviceAndLocalStatus()
                                --    command_data:addError("Error: could not enable access frame type for logical target device " .. getLogicalDevNum(eport) .. " and port " .. getLogicalPortNum(eport))
                                -- end                            
                            end
                        elseif (pwTagMode == 'CPSS_DXCH_TTI_PW_TAG_TAGGED_PW_MODE_E') then
                            if (tagState == "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E") then
                                pwTagState = "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E"
                            else
                                pwTagState = "CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E"
                            end
                            -- drop packets without pw-tag
                            if (is_sip_5(devNum)) then
                                ret, val = myGenWrapper("cpssDxChBrgVlanPortAccFrameTypeSet", {
                                    { "IN", "GT_U8", "devNum", devNum },
                                    { "IN", "GT_PORT_NUM", "portNum", eport },
                                    { "IN", "CPSS_PORT_ACCEPT_FRAME_TYPE_ENT", "keyType", "CPSS_PORT_ACCEPT_FRAME_TAGGED_E" }
                                })
                                if (ret ~= 0) then
                                    command_data:setFailDeviceAndLocalStatus()
                                    command_data:addError("Error: could not enable access frame type for eport " .. eport)
                                end
                            elseif (isVplsModeEnabled(devNum)) then
                               -- ret, val = myGenWrapper("cpssDxChBrgVlanPortAccFrameTypeSet", {
                               --     { "IN", "GT_U8", "devNum", devNum },
                               --     { "IN", "GT_PORT_NUM", "portNum", portNum },
                               --     { "IN", "CPSS_PORT_ACCEPT_FRAME_TYPE_ENT", "keyType", "CPSS_PORT_ACCEPT_FRAME_TAGGED_E" }
                               -- })
                               -- if (ret ~= 0) then
                               --     command_data:setFailDeviceAndLocalStatus()
                               --     command_data:addError("Error: could not enable access frame type for logical target device " .. getLogicalDevNum(eport) .. " and port " .. getLogicalPortNum(eport))
                               -- end                            
                            end
                        end

                        if (is_sip_5(devNum)) then
                            ret, val = myGenWrapper("cpssDxChBrgVlanEgressPortTagStateSet", {
                                { "IN", "GT_U8", "devNum", devNum },
                                { "IN", "GT_PORT_NUM", "portNum", eport},
                                { "IN", "CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT", "tagCmd", pwTagState }
                            })
                            if (ret ~= 0) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Vpls error: could not set tag state mode for eport " .. eport)
                            end

                            -- set egress tpid
                            ret, val = myGenWrapper("cpssDxChBrgVlanPortEgressTpidSet", {
                                { "IN", "GT_U8", "devNum", devNum},
                                { "IN", "GT_PORT_NUM", "portNum", eport},
                                { "IN", "CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT", 
                                            "ethMode", "CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E" },
                                { "IN", "GT_U32", "tpidEntryIndex", TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_1 }
                            })

                            if ret ~= 0 then
                                print("Error while executing cpssDxChBrgVlanPortEgressTpidSet")
                                return false
                            end

                            -- set egress tpid
                            ret, val = myGenWrapper("cpssDxChBrgVlanPortEgressTpidSet", {
                                { "IN", "GT_U8", "devNum", devNum},
                                { "IN", "GT_PORT_NUM", "portNum", eport},
                                { "IN", "CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT", 
                                            "ethMode", "CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E" },
                                { "IN", "GT_U32", "tpidEntryIndex", TPID_ENTRY_INDEX_EGRESS_ETHERTYPE_0 }
                            })

                            if ret ~= 0 then
                                print("Error while executing cpssDxChBrgVlanPortEgressTpidSet")
                                return false
                            end

                            -- set egress p-tag vid
                            apiName = "cpssDxChBrgVlanPortVid0CommandEnableSet"
                            ret = myGenWrapper( apiName, {
                                { "IN", "GT_U8",              "devNum",     devNum },
                                { "IN", "GT_PORT_NUM",        "portNum",    eport},
                                { "IN", "GT_BOOL",            "enable",     true }
                            })
                            if (ret ~= 0) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error: cpssDxChBrgVlanPortVid0CommandEnableSet")
                            end
                        
                            local ptagVid = tonumber(pTag)
                            apiName = "cpssDxChBrgVlanPortVidSet"
                            ret = myGenWrapper( apiName, {
                                { "IN", "GT_U8",              "devNum",     devNum },
                                { "IN", "GT_PORT_NUM",        "portNum",    eport},
                                { "IN", "CPSS_DIRECTION_ENT", "direction",  "CPSS_DIRECTION_EGRESS_E"},
                                { "IN", "GT_U16",             "vlanId",     ptagVid}
                            })
                            if (ret ~= 0) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error: cpssDxChBrgVlanPortVidSet")
                            end
                        end
                    end

                    local ret, hwDevNum = getHwDevNum(devNum)
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not get hw device number ")
                        return false
                    end

                    if (is_sip_5(devNum)) then
                        -- mapping between physical port and ePort
                        local physicalInfoPtr = {}
                        physicalInfoPtr.type = "CPSS_INTERFACE_PORT_E"
                        physicalInfoPtr.devPort = {}
                        physicalInfoPtr.devPort.devNum = hwDevNum
                        physicalInfoPtr.devPort.portNum = portNum
                        ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfoPtr }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not set tag-state for eport " .. eport .. ": " .. returnCodes[ret])
                        end

                        ret, val = myGenWrapper("cpssDxChBrgEgrFltVlanPortFilteringEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "GT_BOOL", "enable", false }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not disable vlan filtering")
                        end
                    elseif (isVplsModeEnabled(devNum)) then   
                        ret, val = myGenWrapper("cpssDxChBrgVlanEgressFilteringEnable", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_BOOL", "enable", false }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not disable vlan filtering")
                        end                
                        -- logical port mapping
                        logicalTargetGenConfNNIInit(devNum, hwDevNum, portNum, eport, pwTagState, pwTagMode, pTag, tsIndex)
                    end
                    
                    local irrevevantField 
                    local irrelevantField = "IRRELEVANT_FIELD"

                    table.insert(configuredPortsNni,
                        {portNum = portNum, nniType = nniType,
                        vsi = vsi, eport = eport,
                        pwLabel = pwLabel,
                        inTunnelLabel = inTunnelLabel,
                        outTunnelLabel = outTunnelLabel,
                        pwTagMode = pwTagMode,
                        pTag = pTag,
                        localIP = localIP, remoteIP = remoteIP,
                        nextHopMac = nextHopMac,
                        tagState = tagState,
                        ttiIndex = index, 
                        ttiIndex2 = index2, 
                        tsIndex = tsIndex,
                        irrelevantField = irrelevantField});
                end

            end

            command_data:updateStatus()

            command_data:updatePorts()
        end
    end
    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

-- ************************************************************************
-- removeNNI
--
--        @description  removes network network interface (NNI)
--
local function removeNNI(params)
    local ret, val, index
    local devNum, portNum, portsForTpid
    local command_data = Command_Data()
    local vsi = tonumber(params["vsi-id"])
    local localIP = params["localIP"]
    local remoteIP = params["remoteIP"]
    local eport;
    local nextHopMac;
    local tagState;
    local udpPort = 4789
    local nniType;
    local tsIndex, ttiIndex, ttiIndex2;
    local entry;
    local apiName

    local isVpls = false
    if (params["vpls"] == true) then
        isVpls = true
    end

    if (true == isVpls) and (false == is_supported_feature(devEnv.dev, "VPLS")) then
        print("Command is not supported for current device")
        return
    end

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            entry = getNniConf(portNum, vsi, localIP, remoteIP);
            if (entry == nil) then
                command_data:setFailDeviceAndLocalStatus()
                command_data:addError(
                    "Error: nni data not found");
                    break;
            end


            if (entry ~= nil) then
                if true == command_data["local_status"] then
                    -- from DB
                    nniType    = entry.nniType;
                    eport      = entry.eport;
                    nextHopMac = entry.nextHopMac;
                    tagState   = entry.tagState;
                    ttiIndex   = entry.ttiIndex;
                    ttiIndex2  = entry.ttiIndex2;
                    tsIndex    = entry.tsIndex;

                    if (isVplsModeEnabled(devNum)) then
                        ret, val = myGenWrapper("cpssDxChLogicalTargetMappingDeviceEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_U8", "logicalDevNum", getLogicalDevNum(eport) },
                            { "IN", "GT_BOOL", "enable", false }
                            })
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not enable logical port mapping for this device: " .. returnCodes[ret])
                        break
                        end
                    end  
                    if (is_sip_5(devNum)) then
                        portsForTpid = { eport }
                    elseif (isVplsModeEnabled(devNum)) then
                        portsForTpid = { portNum }
                    end
                    ret = unbindPortToTpid(devNum, portsForTpid, nil --[[pwTagMode--]], false)
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                    end

                    ret, val = myGenWrapper("cpssDxChTtiRuleValidStatusSet", {
                        { "IN", "GT_U8", "devNum", devNum },
                        { "IN", "GT_U32", "index", ttiIndex },
                        { "IN", "GT_BOOL", "valid", false }
                    })
                    if (ret ~= 0) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not invalidate tti rule")
                    end

                    ret, index = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_TTI_E"],
                        VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], ttiIndex);
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: can't delete index of tti")
                    end

                    -- disable tti per port
                    if (isVpls == true) then
                        ret, val = myGenWrapper("cpssDxChTtiRuleValidStatusSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_U32", "index", ttiIndex2 },
                            { "IN", "GT_BOOL", "valid", false }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not invalidate tti rule")
                        end

                        ret, index = dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_TTI_E"],
                            VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], ttiIndex2);
                        if (ret == false) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: can't delete index of tti")
                        end
                        
                        ret, val = myGenWrapper("cpssDxChTtiPortLookupEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", portNum },
                            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", "CPSS_DXCH_TTI_KEY_MPLS_E" },
                            { "IN", "GT_BOOL", "enable", false }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not disable tti for port " .. portNum)
                        end
                        
                        if (is_sip_5(devNum)) then
                            -- set egress p-tag vid
                            apiName = "cpssDxChBrgVlanPortVid0CommandEnableSet"
                            ret = myGenWrapper( apiName, {
                                { "IN", "GT_U8",              "devNum",     devNum },
                                { "IN", "GT_PORT_NUM",        "portNum",    eport},
                                { "IN", "GT_BOOL",            "enable",     false }
                            })
                            if (ret ~= 0) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error: cpssDxChBrgVlanPortVid0CommandEnableSet")
                            end
                        end

                    else
                        ret, val = myGenWrapper("cpssDxChTtiPortLookupEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", portNum },
                            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", "CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E" },
                            { "IN", "GT_BOOL", "enable", false }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not disable tti for port " .. portNum)
                        end

                        ret, val = myGenWrapper("cpssDxChTtiPortLookupEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", portNum },
                            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", "CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E" },
                            { "IN", "GT_BOOL", "enable", false }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not enable tti for port " .. portNum)
                        end
                    end
                    
                    if (is_sip_5(devNum)) then
                        -- set physical info for eport
                        local egressInfoPtr = {}
                        egressInfoPtr.tunnelStart = false
                        egressInfoPtr.tunnelStartPtr = tsIndex
                        egressInfoPtr.tsPassengerPacketType = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
                        egressInfoPtr.arpPtr = 0
                        egressInfoPtr.modifyMacSa = false
                        egressInfoPtr.modifyMacDa = false

                        ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortEgressPortInfoSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", egressInfoPtr }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not Set physical info for eport " .. eport)
                        end
                    end
                    
                    -- tunnel start entry reset
                    local configPtr = {}
                    local tunnelType
                    if (is_sip_5(devNum)) then
                        tunnelType = "CPSS_TUNNEL_GENERIC_IPV4_E"
                    else
                        tunnelType = "CPSS_TUNNEL_X_OVER_IPV4_E"
                    end
                    ret, val = myGenWrapper("cpssDxChTunnelStartEntrySet", {
                        { "IN", "GT_U8", "devNum", devNum },
                        { "IN", "GT_U32", "routerArpTunnelStartLineIndex", tsIndex },
                        { "IN", "CPSS_TUNNEL_TYPE_ENT", "tunnelType", tunnelType },
                        { "IN", "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_ipv4Cfg", "configPtr", configPtr }
                    })
                    if (ret ~= 0) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not Set ts for index  " .. tsIndex)
                    end

                    dataBaseManagment(VPN_EARCH_DB_TYPE_ENT["VPN_EARCH_DB_TYPE_TS_E"],
                        VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_FREE_E"], tsIndex)
                    if (is_sip_5(devNum)) then
                        ret, val = myGenWrapper("cpssDxChBrgEgrPortMeshIdSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "GT_U32", "meshId", 0 }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not set meshId for eport " .. eport)
                        end
                    end

                    -- remove port to flooding evidx
                    ret = vsiDomainManage(
                        devNum, vsi,
                        VPN_DB_OPERATION_ENT["VPN_DB_OPERATION_REMOVE_E"],
                        eport, tagState, nil --[[meshId--]]);
                    if (ret == false) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error: could not remove eport " .. eport .. " from vsi " .. vsi)
                    end

                    -- do not drop packets without pw-tag
                    if (is_sip_5(devNum)) then
                        ret, val = myGenWrapper("cpssDxChBrgVlanPortAccFrameTypeSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "CPSS_PORT_ACCEPT_FRAME_TYPE_ENT", "keyType", "CPSS_PORT_ACCEPT_FRAME_ALL_E" }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not enable access frame type for eport " .. eport)
                        end
                    elseif (isVplsModeEnabled(devNum)) then
                        -- ret, val = myGenWrapper("cpssDxChBrgVlanPortAccFrameTypeSet", {
                        --    { "IN", "GT_U8", "devNum", devNum },
                        --    { "IN", "GT_PORT_NUM", "portNum", portNum },
                        --    { "IN", "CPSS_PORT_ACCEPT_FRAME_TYPE_ENT", "keyType", "CPSS_PORT_ACCEPT_FRAME_ALL_E" }
                        -- })
                        -- if (ret ~= 0) then
                        --    command_data:setFailDeviceAndLocalStatus()
                        --    command_data:addError("Error: could not enable access frame type for logical target device " .. getLogicalDevNum(eport) .. " and port " .. getLogicalPortNum(eport))
                        -- end                            
                    end

                    if (is_sip_5(devNum)) then
                        -- tag state mode
                        ret, val = myGenWrapper("cpssDxChBrgVlanEgressPortTagStateModeSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT", "stateMode",
                                    "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E" }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not set tag state mode for eport " .. eport)
                        end

                        -- mapping between physical port and ePort
                        local physicalInfoPtr = {}
                        physicalInfoPtr.type = "CPSS_INTERFACE_PORT_E"
                        physicalInfoPtr.devPort = {}
                        physicalInfoPtr.devPort.devNum = 0
                        physicalInfoPtr.devPort.portNum = 0
                        ret, val = myGenWrapper("cpssDxChBrgEportToPhysicalPortTargetMappingTableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfoPtr }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not restore mapping between physical port and ePort " 
                                                        .. eport .. ": " .. returnCodes[ret])
                        end

                        ret, val = myGenWrapper("cpssDxChBrgEgrFltVlanPortFilteringEnableSet", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_PORT_NUM", "portNum", eport },
                            { "IN", "GT_BOOL", "enable", false }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not disable vlan filtering")
                        end
                    elseif (isVplsModeEnabled(devNum)) then   
                        ret, val = myGenWrapper("cpssDxChBrgVlanEgressFilteringEnable", {
                            { "IN", "GT_U8", "devNum", devNum },
                            { "IN", "GT_BOOL", "enable", true }
                        })
                        if (ret ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error: could not enable vlan filtering")
                        end                 
                        logicalTargetGenConfRestore(devNum, eport)
                    end

                    removeNniConf(portNum, vsi, localIP, remoteIP);
                end
            end

            command_data:updateStatus()
            command_data:updatePorts()
        end
    end

    for iterator, devNum, portNum in command_data:getPortIterator() do
        conditionalCleanUp(devNum, vsi);
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

local function show_vpn_func()
    local key;
    local vsi, entry;

    local tagStateConvert =
    {
        ["CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E"] = "untagged",
        ["CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E"] = "tagged"
    };

    local pwTagModeConvert =
    {
        ["CPSS_DXCH_TTI_PW_TAG_RAW_PW_MODE_E"] = "raw-mode",
        ["CPSS_DXCH_TTI_PW_TAG_TAGGED_PW_MODE_E"] = "tag-mode"
    };

    local irrelevantFieldConvert = 
    {
        ["IRRELEVANT_FIELD"] = "      -      "
    };

    local uniCol =
    {
        "portNum",
        {"vsi", "vsi-id...", "d"},
        {"vid", "vlan-id", "d"},
        {{"srcMac", "string"}, "srcMac...........", "string"},
        {"eport", "e-port", "hex"},
        {"tagState", "tag-state",
            {"string", convert = tagStateConvert}},
        "ttiIndex"
    };

    local nniCol =
    {
        "portNum" ,
        {"nniType", "nniType", "string"},
        {"vsi", "vsi-id...", "d"},
        {"eport", "e-port", "hex"},
        {"irrelevantField", "pw-label.....",
            {"string", convert = irrelevantFieldConvert}},
        {{"localIP", "string"}, "localIP......", "string"},
        {{"remoteIP", "string"}, "remoteIP.....", "string"},
        {{"nextHopMac", "string"}, "nextHopMac.......", "string"},
        {"irrelevantField", "out-tunnel-label",
            {"string", convert = irrelevantFieldConvert}},
        {"irrelevantField", "in-tunnel-label",
            {"string", convert = irrelevantFieldConvert}},
        {"irrelevantField", "pw-tag-mode..",
            {"string", convert = irrelevantFieldConvert}},
        {"irrelevantField", "p-tag........",
            {"string", convert = irrelevantFieldConvert}},
        {"tagState", "tag-state",
            {"string", convert = tagStateConvert}},
        "ttiIndex",
        "tsIndex"
    };

    local nniColVpls =
    {
        "portNum" ,
        {"nniType", "nniType", "string"},
        {"vsi", "vsi-id...", "d"},
        {"eport", "e-port", "hex"},
        {"pwLabel", "pw-label.....", "d"},
        {"irrelevantField", "localIP",
            {"string", convert = irrelevantFieldConvert}},
        {"irrelevantField", "remoteIP",
            {"string", convert = irrelevantFieldConvert}},
        {{"nextHopMac", "string"}, "nextHopMac.......", "string"},
        {"outTunnelLabel", "out-tunnel-label", "d"},
        {"inTunnelLabel", "in-tunnel-label", "d"},
        {"pwTagMode", "pw-tag-mode..",
            {"string", convert = pwTagModeConvert}},
        {"pTag", "p-tag........", "d"},
        {"tagState", "tag-state",
            {"string", convert = tagStateConvert}},
        "ttiIndex",
        "tsIndex"
    };


    vsi = next(vsiDb, nil);
    while (vsi ~= nil) do
        entry = vsiDb[vsi];
        print("VSI " .. to_string(vsi)
            .. " used eVid " .. to_string(entry["evid"])
            .. " used flood eVidx " .. to_string(entry["floodingEvidx"]));

        print("UNI ports: ");
        print(string_for_table_title(nil, uniCol));
        print(string_for_table_break(nil, uniCol));
        key = next(configuredPortsUni, nil);
        while (key ~= nil) do
            entry = configuredPortsUni[key];
            if (entry["vsi"] == vsi) then
                print(string_for_table_line(nil, uniCol, entry));
            end
            key = next(configuredPortsUni, key);
        end

        print("NNI ports: ");
        print(string_for_table_title(nil, nniCol));
        print(string_for_table_break(nil, nniCol));
        key = next(configuredPortsNni, nil);
        while (key ~= nil) do
            entry = configuredPortsNni[key];
            if (entry["vsi"] == vsi) then
                if(entry["nniType"] ~= "vpls") then
                    print(string_for_table_line(nil, nniCol, entry));
                else
                    print(string_for_table_line(nil, nniColVpls, entry));
                end
            end
            key = next(configuredPortsNni, key);
        end
        print("\n");

        vsi = next(vsiDb, vsi);
    end
end

--------------------------------------------------------------------------------
-- command registration: create uni
--------------------------------------------------------------------------------
CLI_addCommand("interface", "uni", {
    func = createUNI,
    help = "create UNI interface",
    params = {
        {
            type = "named",
            { format = "vsi %vsi-id", name = "vsi-id", help = "vsi id" },
            { format = "assign-eport %e", name = "eport", help = "eport number" },
            { format = "eport-pvid %vlanIdNoCheck", name = "vid", help = "eport pvid" },
            { format = "vid %vlanIdNoCheck", name = "vlanId", help = "vlan id" },
            { format = "src-mac %mac-address", name = "src-mac", help = "source mac address" },
            { format = "tag-state %vsi_tagged_untagged", name = "tagState", help = "tagged or untagged" },
            requirements = {
                ["eport"] = { "vsi-id" },
                ["vlanId"] = { "eport" },
                -- ["src-mac"] = { "vlanId" }, -- src-mac parameter is optional
                ["tagState"] = { "vlanId" }
            },
            mandatory = { "vlanId" }
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: delete uni
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no uni", {
    func = function(params)
        return removeUNI(params)
    end,
    help = "delete the UNI interface",
    params = {
        {
            type = "named",
            { format = "vid %vlanIdNoCheck", name = "vlanId", help = "vlan id" },
            { format = "src-mac %mac-address", name = "src-mac", help = "source mac address" },
            requirements = {
                ["src-mac"] = { "vlanId" }
            },
            mandatory = { "vlanId" }
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: create nni vxlan
--------------------------------------------------------------------------------
CLI_addHelp("interface", "nni", "create NNI interface")
CLI_addCommand("interface", "nni vxlan", {
    func = function(params)
        params["vxlan"] = true
        return createNNI(params)
    end,
    help = "create NNI interface",
    params = {
        {
            type = "named",
            { format = "vsi %vsi-id", name = "vsi-id", help = "vsi id" },
            { format = "assign-eport %e", name = "eport", help = "eport number" },
            { format = "local-ip-addr %ipv4", name = "localIP", help = "local ip address" },
            { format = "remote-ip-addr %ipv4", name = "remoteIP", help = "local ip address" },
            { format = "nexthop-mac-addr %mac-address", name = "nextHopMac", help = "next hop mac address" },
            { format = "tag-state %vsi_tagged_untagged", name = "tagState", help = "tagged or untagged" },
            requirements = {
                ["eport"] = { "vsi-id" },
                ["localIP"] = { "eport" },
                ["remoteIP"] = { "localIP" },
                ["nextHopMac"] = { "remoteIP" },
                ["tagState"] = { "nextHopMac" }
            },
            mandatory = { "nextHopMac" }
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: delete nni vxlan
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no nni vxlan", {
    func = function(params)
        params["vxlan"] = true
        return removeNNI(params)
    end,
    help = "delete NNI interface",
    params = {
        {
            type = "named",
            { format = "vsi %vsi-id", name = "vsi-id", help = "vsi id" },
            { format = "local-ip-addr %ipv4", name = "localIP", help = "local ip address" },
            { format = "remote-ip-addr %ipv4", name = "remoteIP", help = "local ip address" },
            requirements = {
                ["localIP"] = { "vsi-id" },
                ["remoteIP"] = { "localIP" }
            },
            mandatory = { "remoteIP" }
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: create nni nvgre
--------------------------------------------------------------------------------
CLI_addCommand("interface", "nni nvgre", {
    func = function(params)
        params["nvgre"] = true
        return createNNI(params)
    end,
    help = "create NNI interface",
    params = {
        {
            type = "named",
            { format = "vsi %vsi-id", name = "vsi-id", help = "vsi id" },
            { format = "assign-eport %e", name = "eport", help = "eport number" },
            { format = "local-ip-addr %ipv4", name = "localIP", help = "local ip address" },
            { format = "remote-ip-addr %ipv4", name = "remoteIP", help = "local ip address" },
            { format = "nexthop-mac-addr %mac-address", name = "nextHopMac", help = "next hop mac address" },
            requirements = {
                ["eport"] = { "vsi-id" },
                ["localIP"] = { "eport" },
                ["remoteIP"] = { "localIP" },
                ["nextHopMac"] = { "remoteIP" }
            },
            mandatory = { "nextHopMac" }
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: delete nni nvgre
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no nni nvgre", {
    func = function(params)
        params["vxlan"] = false
        return removeNNI(params)
    end,
    help = "delete NNI interface",
    params = {
        {
            type = "named",
            { format = "vsi %vsi-id", name = "vsi-id", help = "vsi id" },
            { format = "local-ip-addr %ipv4", name = "localIP", help = "local ip address" },
            { format = "remote-ip-addr %ipv4", name = "remoteIP", help = "local ip address" },
            requirements = {
                ["localIP"] = { "vsi-id" },
                ["remoteIP"] = { "localIP" }
            },
            mandatory = { "remoteIP" }
        }
    }
})


--------------------------------------------------------------------------------
-- command registration: create nni vpls
--------------------------------------------------------------------------------
CLI_addCommand("interface", "nni vpls", {
    func = function(params)
        params["vpls"] = true
        return createNNI(params)
    end,
    help = "create NNI interface",
    params = {
        {
            type = "named",
            { format = "vsi %vsi-id", name = "vsi-id", help = "vsi id" },
            { format = "assign-eport %e", name = "eport", help = "eport number" },
            { format = "eport-pvid %vlanIdNoCheck", name = "vid", help = "eport pvid" },

            { format = "pw-label %label", name = "pw-label", help = "pw label"},
            { format = "in-tunnel-label %label", name = "in-tunnel-label", help = "in tunnel label"},
            { format = "out-tunnel-label %label", name = "out-tunnel-label", help = "out tunnel label"},

            { format = "pw-tag-mode %pw_tag_mode_raw_mode_tag_mode", name = "pw-tag-mode", help = "raw-mode or tag-mode" },
            { format = "p-tag %vlanIdNoCheck", name = "p-tag", help = "p-tag value" },

            { format = "c-tag-state %vsi_tagged_untagged", name = "tagState", help = "tagged or untagged" },

            { format = "cw-value %label", name = "control-word", help = "control word, optional parameter (default is 'do not add MPSL control word')"},
            { format = "nexthop-mac-addr %mac-address", name = "nextHopMac", help = "next hop mac address" },
            requirements = {
                ["eport"] = { "vsi-id" },
                ["p-tag"] = { "pw-tag-mode" },
            },
            mandatory = { "vsi-id",  
                          "eport", 
                          "pw-label", 
                          "in-tunnel-label", 
                          "out-tunnel-label",
                          "pw-tag-mode",
                          "p-tag",
                          "tagState",
                          "nextHopMac" }
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: delete nni vpls
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no nni vpls", {
    func = function(params)
        params["vpls"] = true
        return removeNNI(params)
    end,
    help = "delete NNI interface",
    params = {
        {
            type = "named",
            { format = "vsi %vsi-id", name = "vsi-id", help = "vsi id" },
            { format = "assign-eport %e", name = "eport", help = "eport number" },
            requirements = {
                ["eport"] = { "vsi-id" },
            },
            mandatory = { "vsi-id", "eport" }
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: show vpn
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show vpn", "Show vpn info")
CLI_addCommand("exec", "show vpn info", {
    func   = show_vpn_func,
    help   = "Show VPNs UNI and NNI configuration",
    params = {}
})

