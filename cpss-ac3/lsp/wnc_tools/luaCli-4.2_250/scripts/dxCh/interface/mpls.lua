--********************************************************************************
--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mpls.lua
--*
--* DESCRIPTION:
--*       manage mpls configurations on phyPort
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
--require
--constants

local ruleType = "CPSS_DXCH_TTI_RULE_UDB_30_E"
local keyType = "CPSS_DXCH_TTI_KEY_UDB_MPLS_E"
local keySize = "CPSS_DXCH_TTI_KEY_SIZE_30_B_E"
local tunnelType = "CPSS_TUNNEL_X_OVER_MPLS_E"

local ingLabelMaxNum = 4
local ingLabelMaxNum_BobK = 6
local egrLabelMaxNum = 3
local entropyLabelMaxNum = 1
local entropyLocationMaxVal = 1
local entropyLabelMaxNum_BobK = 2
local entropyLocationMaxVal_BobK = 3

local function getMaxIngLabelNum()
    if is_sip_5_15(devEnv.dev) then
        return ingLabelMaxNum_BobK
    else
        return ingLabelMaxNum
    end
end

local function getMaxEntropyLabelNum()
    if is_sip_5_15(devEnv.dev) then
        return entropyLabelMaxNum_BobK
    else
        return entropyLabelMaxNum
    end
end

local function getMaxEntropyLabelLocationVal()
    if is_sip_5_15(devEnv.dev) then
        return entropyLocationMaxVal_BobK
    else
        return entropyLocationMaxVal
    end
end
-- reserved pclid for the lookup
local reserved_pclId = 0x124

local zero = 0

local debug_on = false
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
    end
end

CLI_type_dict["label"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1048575, --0xFFFFF
    complete = CLI_complete_param_number,
    help = "Value in range 0..0xFFFFF"
}


local function CLI_check_param_ingress_label_map(param, data)
    local i,s,e,ret,val

    s=1
    e=1
    i=0
    ret={}
    param=param..","

    while (e<string.len(param)) do
        e= string.find(param,",",s)
        if e==nil then break end

        i=i+1
        if (i>getMaxIngLabelNum()) then
            return false, "Maximum number of ingress labels is " .. getMaxIngLabelNum()
        end
        val=string.sub(param,s,e-1)
        s=e+1
        val=tonumber(val)
        if (val~=nil) and (val<=1048575) and (val>=0) then
            table.insert(ret,val)
        else
            return false, "The list must only contain numbers in the range 0..0xFFFFF"
        end
    end

    return true,ret
end


local function CLI_check_param_egress_label_map(param, data)
    local i,s,e,ret,val

    s=1
    e=1
    i=0
    ret={}
    param=param..","

    while (e<string.len(param)) do
        e= string.find(param,",",s)
        if e==nil then break end

        i=i+1
        if (i>egrLabelMaxNum) then
            return false, "Maximum number of egress labels is " .. egrLabelMaxNum
        end
        val=string.sub(param,s,e-1)
        s=e+1
        val=tonumber(val)
        if (val~=nil) and (val<=1048575) and (val>=0) then
            table.insert(ret,val)
        else
            return false, "The list must only contain numbers in the range 0..0xFFFFF"
        end
    end

    return true,ret
end


local function CLI_check_param_entropy_label_map(param, data)
    local i,s,e,ret,val
    s=1
    e=1
    i=0
    ret={}
    local numerOfEntropyLabelsInCommand=0;

    local entropylocation={}
    entropylocation[0]=nil
    entropylocation[1]=nil

    param=param..","
    -- we need to check according to the number of labels added in the ingress_values_list how many entropy label can be in the packet.
    -- in total in bobK we can have up to 6 labels, otherwise up to 4 labels. each entropy label is built from 2 labels so the counting should be as follows:
    -- for non bobk: up to one entropy label that can be after label0 or after label1
    -- for bobk: up to 2 entropy labels that can be after label0, label1, label2, label3
    -- (if we have 2 entropy labels so only 2 regular labels will be in the packet)
    while (e<string.len(param)) do
        e= string.find(param,",",s)
        if e==nil then break end

        i=i+1

        val=string.sub(param,s,e-1)
        s=e+1
        val=tonumber(val)
        if (val~=nil)then
            numerOfEntropyLabelsInCommand = numerOfEntropyLabelsInCommand + 1
        end
    end

    if is_sip_5_15(devEnv.dev) then
        --for BobK
        if((numberOfRegularLabelsInCommand==1 or numberOfRegularLabelsInCommand==2) and (numerOfEntropyLabelsInCommand>2))then
            return false, "Maximum number of entropy labels is up to 2 in case number of regular_labels is 1 or 2"
        else
            if((numberOfRegularLabelsInCommand==3 or numberOfRegularLabelsInCommand==4) and (numerOfEntropyLabelsInCommand>1))then
                 return false, "Maximum number of entropy labels is up to 1 in case number of regular_labels is 3 or 4"
            else
                if((numberOfRegularLabelsInCommand==0 or numberOfRegularLabelsInCommand==5) and (numerOfEntropyLabelsInCommand~=0))then
                    return false, "Maximum number of entropy labels is 0 in case number of regular_labels is 0 or 5"
                end
            end
        end
    else 
        return false, "Current device doesn't support entropy labels in MPLS TS entry"
    end

    s=1
    e=1
    i=0

    while (e<string.len(param)) do
        e= string.find(param,",",s)
        if e==nil then break end

        i=i+1
        if (i>getMaxEntropyLabelNum()) then
            return false, "Maximum number of entropy labels is " .. getMaxEntropyLabelNum()
        end
        val=string.sub(param,s,e-1)
        s=e+1
        val=tonumber(val)
        if (val~=nil) and (val<=getMaxEntropyLabelLocationVal()) and (val>=0) then
            if(((entropylocation[0]~=nil) and (entropylocation[0]==val)) or
               ((entropylocation[1]~=nil) and (entropylocation[1]==val))) then
                   return false, "entropy label can not be added twice in the same location"
            else
                table.insert(ret,val)
                entropylocation[i-1]=val
            end
        else
            return false, "The list must only contain numbers in the range 0.."..getMaxEntropyLabelLocationVal()
        end
    end

    return true,ret
end

CLI_type_dict["ingress_label_map"] = {
    checker = CLI_check_param_ingress_label_map,
    help = "Value in range 0..0xFFFFF"
}

CLI_type_dict["egress_label_map"] = {
    checker = CLI_check_param_egress_label_map,
    help = "Value in range 0..0xFFFFF"
}

CLI_type_dict["entropy_label_map"] = {
    checker = CLI_check_param_entropy_label_map,
    help = "Value in range 0..2"
}

-- define the UDBs that will be needed for the MPLS classification
--[[ we need next fields:
1. pclid = 0x124 --> see reserved_pclId

    ***> metadata byte 22 bits 0..4 --> 5 LSB bits of pclid
    ***> metadata byte 23 bits 0..4 --> 5 MSB bits of pclid

2. <Physical port>

    ***> metadata byte 26 bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
    ***> metadata byte 27 bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
    ***> metadata byte 22 bit  5 --> Local Device Source Is Trunk

3.<vlan-id>

    ***> metadata byte 24 bits 0..7 --> eVLAN[7:0]
    ***> metadata byte 25 bits 0..4 --> eVLAN[12:8]

4. MAC2ME = 1

    ***> metadata byte 22 bit 7 --> MAC2ME

5. <ingress-label0>

    **> 'MPLS offset' byte 2 bits 0..7 : ingress-label0[31:24]
    **> 'MPLS offset' byte 3 bits 8..15 : ingress-label0[23:16]
    **> 'MPLS offset' byte 4 bits 16..19 : ingress-label0[15:12]

6. <ingress-label1>

    **> 'MPLS offset' byte 6 bits 0..7 : ingress-label1[31:24]
    **> 'MPLS offset' byte 7 bits 8..15 : ingress-label1[23:16]
    **> 'MPLS offset' byte 8 bits 16..19 : ingress-label1[15:12]

7. <ingress-label2>

    **> 'MPLS offset' byte 10 bits 0..7 : ingress-label2[31:24]
    **> 'MPLS offset' byte 11 bits 8..15 : ingress-label2[23:16]
    **> 'MPLS offset' byte 12 bits 16..19 : ingress-label2[15:12]

8. <ingress-label3>

    **> 'MPLS offset' byte 14 bits 0..7 : ingress-label3[31:24]
    **> 'MPLS offset' byte 15 bits 8..15 : ingress-label3[23:16]
    **> 'MPLS offset' byte 16 bits 16..19 : ingress-label3[15:12]

9. <ingress-label4>

    **> 'MPLS offset' byte 17 bits 0..7 : ingress-label4[31:24]
    **> 'MPLS offset' byte 18 bits 8..15 : ingress-label4[23:16]
    **> 'MPLS offset' byte 19 bits 16..19 : ingress-label4[15:12]

10. <ingress-label5>

    **> 'MPLS offset' byte 20 bits 0..7 : ingress-label5[31:24]
    **> 'MPLS offset' byte 21 bits 8..15 : ingress-label5[23:16]
    **> 'MPLS offset' byte 22 bits 16..19 : ingress-label5[15:12]

total: 24 udbs
--]]

local udbArr =
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
    -- offsets in the packet section
    { udbIndex = 6,    offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 2      }, -- bits 0..7 : ingress-label0[31:24]
    { udbIndex = 7,    offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 3      }, -- bits 8..15 : ingress-label0[23:16]
    { udbIndex = 8,    offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 4      }, -- bits 16..19 : ingress-label0[15:12]
    { udbIndex = 9,    offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 6      }, -- bits 0..7 : ingress-label1[31:24]
    { udbIndex = 10,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 7      }, -- bits 8..15 : ingress-label1[23:16]
    { udbIndex = 11,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 8      }, -- bits 16..19 : ingress-label1[15:12]
    { udbIndex = 12,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 10     }, -- bits 0..7 : ingress-label2[31:24]
    { udbIndex = 13,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 11     }, -- bits 8..15 : ingress-label2[23:16]
    { udbIndex = 14,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 12     }, -- bits 16..19 : ingress-label1[15:12]
    { udbIndex = 15,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 14     }, -- bits 0..7 : ingress-label3[31:24]
    { udbIndex = 16,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 15     }, -- bits 8..15 : ingress-label3[23:16]
    { udbIndex = 17,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 16     }, -- bits 16..19 : ingress-label3[15:12]
    { udbIndex = 18,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 17     }, -- bits 0..7 : ingress-label4[31:24]
    { udbIndex = 19,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 18     }, -- bits 8..15 : ingress-label4[23:16]
    { udbIndex = 20,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 19     }, -- bits 16..19 : ingress-label4[15:12]
    { udbIndex = 21,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 20     }, -- bits 0..7 : ingress-label5[31:24]
    { udbIndex = 22,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 21     }, -- bits 8..15 : ingress-label5[23:16]
    { udbIndex = 23,   offsetType = "CPSS_DXCH_TTI_OFFSET_MPLS_MINUS_2_E", offset = 22     }  -- bits 16..19 : ingress-label5[15:12]
}

-- udbArray
local patternPtr = { udbArray  = {
    [0] = bit_or(bit_and(reserved_pclId,0x1f),0x80),        -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk, bit 7 --> MAC2ME
    [1] = bit_and(bit_shr(reserved_pclId,5),0x1f),          -- bits 0..4 --> 5 MSB bits of pclid
    [2] = FILLED_IN_RUN_TIME,                               -- bits 0..7 --> eVLAN[7:0]
    [3] = FILLED_IN_RUN_TIME,                               -- bits 0..4 --> eVLAN[12:8]
    [4] = FILLED_IN_RUN_TIME,                               -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
    [5] = FILLED_IN_RUN_TIME,                               -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
    [6] = FILLED_IN_RUN_TIME,                               -- bits 0..7 --> ingress-label0[31:24]
    [7] = FILLED_IN_RUN_TIME,                               -- bits 8..15 --> ingress-label0[23:16]
    [8] = FILLED_IN_RUN_TIME,                               -- bits 16..19 --> ingress-label0[15:12]
    [9] = FILLED_IN_RUN_TIME,                               -- bits 0..7 --> ingress-label1[31:24]
    [10] = FILLED_IN_RUN_TIME,                              -- bits 8..15 --> ingress-label1[23:16]
    [11] = FILLED_IN_RUN_TIME,                              -- bits 16..19 --> ingress-label1[15:12]
    [12] = FILLED_IN_RUN_TIME,                              -- bits 0..7 --> ingress-label2[31:24]
    [13] = FILLED_IN_RUN_TIME,                              -- bits 8..15 --> ingress-label2[23:16]
    [14] = FILLED_IN_RUN_TIME,                              -- bits 16..19 --> ingress-label2[15:12]
    [15] = FILLED_IN_RUN_TIME,                              -- bits 0..7 --> ingress-label3[31:24]
    [16] = FILLED_IN_RUN_TIME,                              -- bits 8..15 --> ingress-label3[23:16]
    [17] = FILLED_IN_RUN_TIME,                              -- bits 16..19 --> ingress-label3[15:12]
    [18] = FILLED_IN_RUN_TIME,                              -- bits 0..7 --> ingress-label4[31:24]
    [19] = FILLED_IN_RUN_TIME,                              -- bits 8..15 --> ingress-label4[23:16]
    [20] = FILLED_IN_RUN_TIME,                              -- bits 16..19 --> ingress-label4[15:12]
    [21] = FILLED_IN_RUN_TIME,                              -- bits 0..7 --> ingress-label5[31:24]
    [22] = FILLED_IN_RUN_TIME,                              -- bits 8..15 --> ingress-label5[23:16]
    [23] = FILLED_IN_RUN_TIME,                              -- bits 16..19 --> ingress-label5[15:12]
    [24] = 0,
    [25] = 0,
    [26] = 0,
    [27] = 0,
    [28] = 0,
    [29] = 0
} }

-- udbArray
local maskPtr = { udbArray  = {
    [0] = 0xbf,   -- bits 0..4 --> 5 LSB bits of pclid, bit 5 --> Local Device Source Is Trunk, bit 7 --> MAC2ME
    [1] = 0x1f,   -- bits 0..4 --> 5 MSB bits of pclid
    [2] = 0xff,   -- bits 0..7 --> eVLAN[7:0]
    [3] = 0x1f,   -- bits 0..4 --> eVLAN[12:8]
    [4] = 0xff,   -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
    [5] = 0x1f,   -- bits 0..4 --> Local Device Source ePort/TrunkID[12:8]
    [6] = 0xff,   -- bits 0..7 --> ingress-label0[31:24]
    [7] = 0xff,   -- bits 8..15 --> ingress-label0[23:16]
    [8] = 0xf0,    -- bits 16..19 --> ingress-label0[15:12]
    [9] = 0xff,   -- bits 0..7 --> ingress-label1[31:24]
    [10] = 0xff,  -- bits 8..15 --> ingress-label1[23:16]
    [11] = 0xf0,   -- bits 16..19 --> ingress-label1[15:12]
    [12] = 0xff,  -- bits 0..7 --> ingress-label2[31:24]
    [13] = 0xff,  -- bits 8..15 --> ingress-label2[23:16]
    [14] = 0xf0,   -- bits 16..19 --> ingress-label2[15:12]
    [15] = 0xff,  -- bits 0..7 --> ingress-label3[31:24]
    [16] = 0xff,  -- bits 8..15 --> ingress-label3[23:16]
    [17] = 0xf0,   -- bits 16..19 --> ingress-label3[15:12]
    [18] = 0xff,  -- bits 0..7 --> ingress-label4[31:24]
    [19] = 0xff,  -- bits 8..15 --> ingress-label4[23:16]
    [20] = 0xf0,   -- bits 16..19 --> ingress-label4[15:12]
    [21] = 0xff,  -- bits 0..7 --> ingress-label5[31:24]
    [22] = 0xff,  -- bits 8..15 --> ingress-label5[23:16]
    [23] = 0xf0,   -- bits 16..19 --> ingress-label5[15:12]
    [24] = 0,
    [25] = 0,
    [26] = 0,
    [27] = 0,
    [28] = 0,
    [29] = 0
}}

-- action2 struct
local actionPtr = { type2  = {
    tunnelTerminate       = GT_TRUE,
    ttPassengerPacketType = "CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E",
    command               = "CPSS_PACKET_CMD_FORWARD_E",
    bridgeBypass          = GT_FALSE,
    redirectCommand       = "CPSS_DXCH_TTI_NO_REDIRECT_E",
    keepPreviousQoS       = GT_TRUE,
    tunnelStart           = GT_FALSE,
    tunnelStartPtr        = 0,
    tag0VlanCmd           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
    tag1VlanCmd           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
    tag1UpCommand         = "CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E",
    copyTtlExpFromTunnelHeader = GT_TRUE,
    sourceEPortAssignmentEnable = GT_TRUE,
    sourceEPort           = FILLED_IN_RUN_TIME
} }

local configPtr = { mplsCfg = {
    tagEnable               = GT_TRUE,
    vlanId                  = FILLED_IN_RUN_TIME,
    macDa                   = FILLED_IN_RUN_TIME,
    label1                  = FILLED_IN_RUN_TIME,
    controlWordEnable       = FILLED_IN_RUN_TIME,
    controlWordIndex        = FILLED_IN_RUN_TIME,
    pushEliAndElAfterLabel1 = FILLED_IN_RUN_TIME,
    ttl                     = FILLED_IN_RUN_TIME,
    ttlMode                 = "CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E"
} }

local mac2meValuePtr =
{
    mac     = FILLED_IN_RUN_TIME,
    vlanId  = FILLED_IN_RUN_TIME
}

local mac2meMaskPtr =
{
    mac     = FILLED_IN_RUN_TIME,
    vlanId  = 0xfff
}

-- global number of MPLS channels in the system
local global_num_mpls_channels = 0

-- number of MPLS channel per physical port
local num_mpls_channel_db = {}

--[[ define the table that will hold the 'MPLS configuration done in the system'
     each line in the table hold info about different interface (device/port)
     each interface (device/port) hold next info :
                  1.assign_eport - a unique name
                  2.devNum - device number of the interface
                  3.portNum
                  4.vlan_id
                  5.my_mac_addr
                  6.nexthop_mac_addr
                  7.ingress_label0
                  8.ingress_label1 - optional param
                  9.ingress_label2 - optional param
                  10.ingress_label3 - optional param
                  11.ingress_label4 - optional param
                  12.ingress_label5 - optional param
                  13.egress_label0
                  14.egress_label1 - optional param
                  15.egress_label2 - optional param
                  16.cw_value - optional param
                  17.ttiIndex - calculated internally
                  18.tsIndex - calculated internally
                  19.cwIndex - calculated internally
                  20. mac2meIndex - calculated internally
--]]
local table_mpls_system_info = {}
function getTableMpls()
    return table_mpls_system_info
end

local function mpls_command_func(command_data,devNum,portNum, params)

    -- Common variables declaration
    local numberOfLabels = 1 -- one label
    local cwEnable = false
    local hwDevNum
    local apiName,result
    local ttiIndex,tsIndex,mac2meIndex
    local cwIndex = false
    local physicalInfo = {}
    local interfaceInfo = {}
    local egressInterface = {}
    local egressInfo = {}
    local i,j

    local myIndexName

    if false == is_supported_feature(devNum, "MPLS_CMD_SUPPORT") then
        print("Command not supported for current device")
        return
    end

    -- print input params
    _debug("MPLS command params \n "..to_string(params))

    numberOfLabels = table.getn(params["egress_values_list"])

    if params.cw_value ~= nil then
        actionPtr.type2.cwBasedPw = GT_TRUE
        cwEnable = true
    end

    -- change type for input params
    params.vlan_id = tonumber(params.vlan_id)
    params.eport = tonumber(params.eport)

    ------------- ingress configuration ---------------------

    myIndexName = "<MPLS: ingress> assign-eport ".. params.eport
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB

    mac2meIndex = sharedResourceNamedMac2meMemoAlloc(
        myIndexName, 1 --[[size--]], false --[[fromEnd--]]);

    apiName = "cpssDxChCfgHwDevNumGet"
    result, hwDevNum = device_to_hardware_format_convert(devNum)
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    interfaceInfo.includeSrcInterface = "CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_E"
    interfaceInfo.srcDevice = hwDevNum
    interfaceInfo.srcIsTrunk = GT_FALSE
    interfaceInfo.srcPortTrunk = portNum

    -- set 'runtime' mac2me table
    mac2meValuePtr.mac = params.my_mac_addr
    mac2meValuePtr.vlanId = params.vlan_id
    mac2meMaskPtr.mac = params.my_mac_addr

    -- set a TTI MacToMe entry
    apiName = "cpssDxChTtiMacToMeSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                        "devNum",       devNum },
        { "IN",     "GT_U32",                       "entryIndex",   mac2meIndex },
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",   "valuePtr",     mac2meValuePtr },
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",   "maskPtr",      mac2meMaskPtr },
        { "IN",     "CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC", "interfaceInfoPtr", interfaceInfo }
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    if(cwEnable == true) then
        myIndexName = "<MPLS: ingress> assign-eport ".. params.eport
        cwIndex = allocCwIndexFromPool(myIndexName,false)

        -- set a Pseudo Wire control word
        apiName = "cpssDxChTunnelStartMplsPwControlWordSet"
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",        "devNum",       devNum },
            { "IN",     "GT_U32",       "index",        cwIndex },
            { "IN",     "GT_U32",       "value",        params.cw_value }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)
    end

    -- set key type size
    apiName = "cpssDxChTtiPacketTypeKeySizeSet"
    result = myGenWrapper(
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", keyType},
        { "IN", "CPSS_DXCH_TTI_KEY_SIZE_ENT", "keySize", keySize}
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

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

    -- enable the TTI lookup for port
    apiName = "cpssDxChTtiPortLookupEnableSet"
    result = myGenWrapper(
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", portNum},
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", keyType},
        { "IN", "GT_BOOL", "enable", true}
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- set the PCL ID for the specified key type
    apiName = "cpssDxChTtiPclIdSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                         "devNum",     devNum },
        { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    keyType},
        { "IN",     "GT_U32",                        "pclId",      reserved_pclId  },
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    myIndexName = "<MPLS: ingress> assign-eport ".. params.eport
    ttiIndex = allocTtiIndexFromPool(3,myIndexName,"tti0")

    -- set 'runtime' pattern params
    patternPtr.udbArray[2] = bit_and(params.vlan_id,0xff)
    patternPtr.udbArray[3] = bit_and(bit_shr(params.vlan_id,8),0x1f)
    patternPtr.udbArray[4] = bit_and(portNum,0xff)
    patternPtr.udbArray[5] = bit_and(bit_shr(portNum,8),0x1f)
    j=6
    local ingress_label
    for i=1,table.getn(params["ingress_values_list"]) do
        ingress_label = bit_shl(params["ingress_values_list"][i],4)
        patternPtr.udbArray[j] = bit_and(bit_shr(ingress_label,16),0xff)
        patternPtr.udbArray[j+1] = bit_and(bit_shr(ingress_label,8),0xff)
        patternPtr.udbArray[j+2] = bit_and(ingress_label,0xf0)
        maskPtr.udbArray[j]   = 0xff
        maskPtr.udbArray[j+1] = 0xff
        maskPtr.udbArray[j+2] = 0xf0
        j=j+3
    end
    for i=j,table.getn(maskPtr.udbArray) do
        maskPtr.udbArray[i] = 0
    end
    -- set 'runtime' action params
    actionPtr.type2.sourceEPort = params.eport

    -- set the TTI Rule Pattern, Mask and Action
    apiName = "cpssDxChTtiRuleSet"
    result = myGenWrapper(
        apiName ,{
        { "IN",     "GT_U8",                           "devNum",     devNum },
        { "IN",     "GT_U32",                          "index",      ttiIndex },
        { "IN",     "CPSS_DXCH_TTI_RULE_TYPE_ENT",     "ruleType",   ruleType},
        { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "patternPtr", patternPtr},
        { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "maskPtr",    maskPtr},
        { "IN",     "CPSS_DXCH_TTI_ACTION_TYPE_ENT",   "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"},
        { "IN",     "CPSS_DXCH_TTI_ACTION_UNT_type2",  "actionPtr",  actionPtr},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    ------------- egress configuration ---------------------

    -- set 'runtime' ts params
    configPtr.mplsCfg.numLabels = numberOfLabels
    configPtr.mplsCfg.vlanId    = params.vlan_id
    configPtr.mplsCfg.macDa     = params.nexthop_mac_addr
    configPtr.mplsCfg.ttl       = 0x00
    configPtr.mplsCfg.label1    = params["egress_values_list"][1]
    if params["egress_values_list"][2] ~= nil then
        configPtr.mplsCfg.label1    = params["egress_values_list"][2]
        configPtr.mplsCfg.label2    = params["egress_values_list"][1]
    end
    if params["egress_values_list"][3] ~= nil then
        configPtr.mplsCfg.label1    = params["egress_values_list"][3]
        configPtr.mplsCfg.label2    = params["egress_values_list"][2]
        configPtr.mplsCfg.label3    = params["egress_values_list"][1]
    end 
    
    local pushEntropyLabel1 = false
    local pushEntropyLabel2 = false
    local pushEntropyLabel3 = false
    
    if (params["entropy_labels_locations_list"] ~= nil) then
        for i=1,table.getn(params["entropy_labels_locations_list"]) do
            if (params["entropy_labels_locations_list"][i] == 0) then 
                pushEntropyLabel1 = true
            end
            if (params["entropy_labels_locations_list"][i] == 1) then 
                pushEntropyLabel2 = true
            end
            if (params["entropy_labels_locations_list"][i] == 2) then 
                pushEntropyLabel3 = true
            end
        end
        configPtr.mplsCfg.ttl = 0x40
    end

    configPtr.mplsCfg.pushEliAndElAfterLabel1 = pushEntropyLabel1
    if (numberOfLabels > 1) then
        configPtr.mplsCfg.pushEliAndElAfterLabel2 = pushEntropyLabel2
    end
    if (numberOfLabels > 2) then
        configPtr.mplsCfg.pushEliAndElAfterLabel3 = pushEntropyLabel3
    end
    
    if (cwEnable == true) then
        configPtr.mplsCfg.controlWordEnable = GT_TRUE
        configPtr.mplsCfg.controlWordIndex = cwIndex -- was already allocated
    end

    myIndexName = "<MPLS: egress> assign-eport ".. params.eport
    tsIndex = sharedResourceNamedTsMemoAlloc(myIndexName,1)

    -- set a tunnel start entry
    apiName = "cpssDxChTunnelStartEntrySet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                 "devNum",                        devNum},
        { "IN",     "GT_U32",                "routerArpTunnelStartLineIndex", tsIndex},
        { "IN",     "CPSS_TUNNEL_TYPE_ENT",  "tunnelType",                    tunnelType},
        { "IN",     "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_mplsCfg", "configPtr", configPtr},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    physicalInfo.type = "CPSS_INTERFACE_PORT_E"
    physicalInfo.devPort = {}
    physicalInfo.devPort.devNum  = hwDevNum
    physicalInfo.devPort.portNum = portNum

    -- set physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum},
        { "IN",     "GT_PORT_NUM",             "portNum",         params.eport},
        { "IN",     "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    egressInfo.tunnelStart = true
    egressInfo.tunnelStartPtr = tsIndex
    egressInfo.tsPassengerPacketType  = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
    egressInfo.arpPtr = 0
    egressInfo.modifyMacSa = false
    egressInfo.modifyMacDa = false

    -- set physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum      },
        { "IN",     "GT_PORT_NUM",             "portNum",         params.eport },
        { "IN",     "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", egressInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- per Egress port bit Enable Routed packets MAC SA Modification
    apiName = "cpssDxChIpRouterMacSaModifyEnable"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                "devNum",   devNum},
        { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  portNum},
        { "IN",     "GT_BOOL",              "enable",   true},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- Set the mode, per port, in which the device sets the packet's MAC SA least significant bytes
    apiName = "cpssDxChIpPortRouterMacSaLsbModeSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                "devNum",   devNum},
        { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  portNum},
        { "IN",     "CPSS_MAC_SA_LSB_MODE_ENT","saLsbMode", "CPSS_SA_LSB_FULL_48_BIT_GLOBAL"},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- Sets full 48-bit Router MAC SA in Global MAC SA table
    lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,portNum,params.my_mac_addr)

    -- Set router mac sa index refered to global MAC SA table
    lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,params.eport,portNum)

    -- operation succed save the info into DB
    local entryInfo = {}

    entryInfo.devNum = devNum
    entryInfo.portNum = portNum
    entryInfo.assign_eport = params.eport
    entryInfo.vlan_id = params.vlan_id
    entryInfo.my_mac_addr = params.my_mac_addr
    entryInfo.nexthop_mac_addr = params.nexthop_mac_addr

    entryInfo.ingress_label={}
    for i=1,table.getn(params["ingress_values_list"]) do
       entryInfo.ingress_label[i-1] = params["ingress_values_list"][i]
    end
    
    entryInfo.egress_label={}
    for i=1,table.getn(params["egress_values_list"]) do
       entryInfo.egress_label[i-1] = params["egress_values_list"][i]
    end
    
    entryInfo.entropy_label0 = pushEntropyLabel1;
    entryInfo.entropy_label1 = pushEntropyLabel2;
    entryInfo.entropy_label2 = pushEntropyLabel3;

    entryInfo.cw_value  = params.cw_value
    entryInfo.ttiIndex  = ttiIndex
    entryInfo.tsIndex  = tsIndex
    entryInfo.cwIndex  = cwIndex
    entryInfo.mac2meIndex  = mac2meIndex

    -- use the eport value as a key to the table
    table_mpls_system_info[params.eport] = entryInfo

    -- another MPLS channel was added
    global_num_mpls_channels = global_num_mpls_channels + 1
    -- another MPLS channel on port was added
    if num_mpls_channel_db[portNum] == nil then
        num_mpls_channel_db[portNum] = 1
    else
        num_mpls_channel_db[portNum] = num_mpls_channel_db[portNum] + 1
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end




-- ************************************************************************
--
--  mpls_port_func
--        @description  Set MPLS configurations on phyPort
--
--        @param params             - params["eport"]: ePort number, a unique name
--                                  - params["vlan_id"]: vlan id
--                                  - params["my_mac_address"]: my mac address
--                                  - params["nexthop_mac_address"]: nexthop mac address
--                                  - params["ingress_label0"]: ingress label0
--                                  - params["ingress_label1"]: ingress label1, optional param
--                                  - params["ingress_label2"]: ingress label2, optional param
--                                  - params["ingress_label3"]: ingress label3, optional param
--                                  - params["ingress_label4"]: ingress label4, optional param
--                                  - params["ingress_label5"]: ingress label5, optional param
--                                  - params["egress_label0"]: egress label0
--                                  - params["egress_label1"]: egress label1, optional param
--                                  - params["egress_label2"]: egress label2, optional param
--                                  - params["cw_value"]: control word, optional param
--
--        @return       true on success, otherwise false and error message
--
local function mpls_func(params)

    local command_data = Command_Data()

    command_data:initInterfaceDevPortRange()
    -- now iterate
    command_data:iterateOverPorts(mpls_command_func,params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

-- ************************************************************************
--
--  no_mpls_eport_command_func
--        @description  Unset MPLS configurations on phyPort for eport, restore configuration
--
--        @param params   - params["eport"]: ePort number, a unique name
--
--        @return       true on success, otherwise false and error message
--
local function no_mpls_eport_command_func(command_data,devNum,portNum, params)

    -- Common variables declaration
    local apiName,result
    local ttiIndex,tsIndex,mac2meIndex,cwIndex
    local entryInfo = {}
    local interfaceInfo = {}
    local physicalInfo = {}
    local egressInfo = {}

    local mac2meDefaultValuePtr = {
        mac     = 0,
        vlanId  = 4095
    }

    local mac2meDefaultMaskPtr = {
        mac     = convertMacAddrStringTo_GT_ETHERADDR("ff:ff:ff:ff:ff:ff"),
        vlanId  = 4095
    }

    local configZeroPtr = { mplsCfg = {
        tagEnable               = GT_FALSE,
        vlanId                  = 0,
        macDa                   = 0,
        label1                  = 0,
        controlWordEnable       = GT_FALSE,
        controlWordIndex        = 0,
        ttlMode                 = "CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E",
        ttl                     = 0,
        numLabels               = 1,
        pushEliAndElAfterLabel1 = GT_FALSE
    } }

    -- print input params
    _debug("no MPLS eport command params \n "..to_string(params))

    -- change type for input params
    params.eport = tonumber(params.eport)

    -- read params from DB
    entryInfo = table_mpls_system_info[params.eport]
    if entryInfo == nil then
        print("eport "..params.eport.." was not defined in MPLS_db")
        return nil
    end

    -- check that current physical port and device are the same as in the DB
    if entryInfo.portNum ~= portNum or entryInfo.devNum ~= devNum then
        print("trying to delete eport "..params.eport.." not from the physical port it was defined")
        return nil
    end

    devNum = entryInfo.devNum
    portNum = entryInfo.portNum
    mac2meIndex = entryInfo.mac2meIndex
    cwIndex = entryInfo.cwIndex
    ttiIndex = entryInfo.ttiIndex
    tsIndex = entryInfo.tsIndex

    ------------- disbale ingress configuration ---------------------

    interfaceInfo.includeSrcInterface = "CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E"
    interfaceInfo.srcHwDevice = 0
    interfaceInfo.srcIsTrunk = GT_FALSE
    interfaceInfo.srcPortTrunk = portNum

    -- unset a TTI MacToMe entry
    apiName = "cpssDxChTtiMacToMeSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                        "devNum",       devNum },
        { "IN",     "GT_U32",                       "entryIndex",   mac2meIndex },
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",   "valuePtr",     mac2meDefaultValuePtr },
        { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",   "maskPtr",      mac2meDefaultMaskPtr },
        { "IN",     "CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC", "interfaceInfoPtr", interfaceInfo }
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    sharedResourceNamedMac2meMemoFree(mac2meIndex);

    if (cwIndex ~= false) then
        -- unset a Pseudo Wire control word
        apiName = "cpssDxChTunnelStartMplsPwControlWordSet"
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",        "devNum",       devNum },
            { "IN",     "GT_U32",       "index",        cwIndex },
            { "IN",     "GT_U32",       "value",        zero }
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)

        freeCwIndexToPool(cwIndex)
    end

    -- unset the TTI Rule Pattern, Mask and Action
    apiName = "cpssDxChTtiRuleValidStatusSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_U32", "index", ttiIndex},
            { "IN", "GT_BOOL", "valid", GT_FALSE}
        })

    freeTtiIndexToPool(ttiIndex)

    -- if last MPLS channel in the system delete configuration
    if global_num_mpls_channels == 1 then

        -- unset key type size
        apiName = "cpssDxChTtiPacketTypeKeySizeSet"
        result = myGenWrapper(
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", keyType},
            { "IN", "CPSS_DXCH_TTI_KEY_SIZE_ENT", "keySize", "CPSS_DXCH_TTI_KEY_SIZE_10_B_E"}
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)

        -- configure the UDBs needed for the match back to diffault
        apiName = "cpssDxChTtiUserDefinedByteSet"
        for dummy,udbEntry in pairs(udbArr) do
            result = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",                         "devNum",     devNum },
                { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    keyType},
                { "IN",     "GT_U32",                        "udbIndex",   udbEntry.udbIndex },
                { "IN",     "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", "CPSS_DXCH_TTI_OFFSET_INVALID_E"},
                { "IN",     "GT_U8",                         "offset",     zero }
            })
        end

        -- unset the PCL ID for the specified key type
        apiName = "cpssDxChTtiPclIdSet"
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                         "devNum",     devNum },
            { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    keyType},
            { "IN",     "GT_U32",                        "pclId",      zero  },
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)

    end

    -- if last MPLS channel per physical port delete configuration
    if num_mpls_channel_db[portNum] == 1 then

        -- disable the TTI lookup for port
        apiName = "cpssDxChTtiPortLookupEnableSet"
        result = myGenWrapper(
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", portNum},
            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", keyType},
            { "IN", "GT_BOOL", "enable", false}
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)

    end

    ------------- egress configuration ---------------------

    -- unset a tunnel start entry
    apiName = "cpssDxChTunnelStartEntrySet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                 "devNum",                        devNum},
        { "IN",     "GT_U32",                "routerArpTunnelStartLineIndex", tsIndex},
        { "IN",     "CPSS_TUNNEL_TYPE_ENT",  "tunnelType",                    tunnelType},
        { "IN",     "CPSS_DXCH_TUNNEL_START_CONFIG_UNT_mplsCfg", "configPtr", configZeroPtr},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    local myIndexName = "<MPLS: egress> assign-eport ".. params.eport
    sharedResourceNamedTsMemoFree(myIndexName);

    physicalInfo.type = "CPSS_INTERFACE_PORT_E"
    physicalInfo.devPort = {}
    physicalInfo.devPort.devNum  = 0
    physicalInfo.devPort.portNum = 0

    -- unset physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum},
        { "IN",     "GT_PORT_NUM",             "portNum",         params.eport},
        { "IN",     "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    egressInfo.tunnelStart = false
    egressInfo.tunnelStartPtr = 0
    egressInfo.tsPassengerPacketType  = "CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
    egressInfo.arpPtr = 0
    egressInfo.modifyMacSa = false
    egressInfo.modifyMacDa = false

    -- unset physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum      },
        { "IN",     "GT_PORT_NUM",             "portNum",         params.eport },
        { "IN",     "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", egressInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)

    -- per Egress port bit disable Routed packets MAC SA Modification
    -- the default bit state is enable, disabling below commented out
    --[[
    apiName = "cpssDxChIpRouterMacSaModifyEnable"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                "devNum",   devNum},
        { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  portNum},
        { "IN",     "GT_BOOL",              "enable",   GT_FALSE},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)
    --]]

    -- unSets full 48-bit Router MAC SA in Global MAC SA table
    lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,portNum,zero)

    -- unSet router mac sa index refered to global MAC SA table
    lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,params.eport,zero)

    -- operation succed remove the info from DB
    table_mpls_system_info[params.eport] = nil
    num_mpls_channel_db[portNum] = num_mpls_channel_db[portNum] - 1
    global_num_mpls_channels = global_num_mpls_channels - 1

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

local function no_mpls_eport_func(params)

    local command_data = Command_Data()

    command_data:initInterfaceDevPortRange()
    -- now iterate
    command_data:iterateOverPorts(no_mpls_eport_command_func, params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

-- ************************************************************************
--  no_mpls_all_func
--        @description  Unset MPLS configurations on all phyPorts, restore configuration
--
--        @param params   - none
--
--        @return       true on success, otherwise false and error message
--
local function no_mpls_all_func()

    local command_data = Command_Data()
    -- Common variables declaration
    local current_index
    local current_entry = {}
    local params = {}
    local apiName,result

    -- print input params
    _debug("no MPLS all command no params \n")

    -- get the first element in the table
    current_index , current_entry = next(table_mpls_system_info,nil)

    while current_entry do

        if current_entry.assign_eport ~= nil then
            params.eport = current_entry.assign_eport
            no_mpls_eport_func(params)
        end

        current_index , current_entry = next(table_mpls_system_info,current_index)

    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- ************************************************************************
---
--  mpls_show
--        @description  show MPLS info for all interfaces
--
--        @param params         none
--
--        @return       true on success, otherwise false and error message
--
local function mpls_show(params)

    -- show all MPLS channels in the system

    --  get the first element in the table
    local current_index , current_entry
    local current_channel_name_data
    local temp
    local ingress_label1,ingress_label2,ingress_label3,ingress_label4,ingress_label5
    local egress_label1,egress_label2,cw_value
    local entropy_label0,entropy_label1,entropy_label2
    local my_mac_addr
    local nexthop_mac_addr
    local iterator,port
    local bobK_labels,ingress_bobK_labels

    iterator = 1

    current_index , current_entry = next(table_mpls_system_info,nil)

    if is_sip_5_15(devEnv.dev) then
        bobK_labels = " i-label4 ".." i-label5 "
    else
        bobK_labels = ""
    end
    
    print("\nindex  ".." devNum ".." portNum ".." eport ".." vlan ".."    my-mac-addr   ".."  nexthop-mac-addr "..
           "  i-label0 ".." i-label1 ".." i-label2 ".." i-label3 "..bobK_labels.." e-label0 ".." entropy0 ".." e-label1 ".." entropy1 ".." e-label2 ".." entropy2 ".." cw-value ")
    print("------  ------  -------  -----  ----  -----------------  -----------------  --------  --------  --------  --------  --------  --------  --------  --------  --------  --------  --------  --------  --------")

    while current_entry do
        current_channel_name_data = current_entry.assign_eport
        if current_channel_name_data ~= nil then

            if current_entry.ingress_label[1] ~= nil then
                ingress_label1 = tostring(current_entry.ingress_label[1])
            else
                ingress_label1 = "    "
            end

            if current_entry.ingress_label[2] ~= nil then
                ingress_label2 = tostring(current_entry.ingress_label[2])
            else
                ingress_label2 = "    "
            end

            if current_entry.ingress_label[3] ~= nil then
                ingress_label3 = tostring(current_entry.ingress_label[3])
            else
                ingress_label3 = "    "
            end

            if current_entry.ingress_label[4] ~= nil then
                ingress_label4 = tostring(current_entry.ingress_label[4])
            else
                ingress_label4 = "    "
            end

            if current_entry.ingress_label[5] ~= nil then
                ingress_label5 = tostring(current_entry.ingress_label[5])
            else
                ingress_label5 = "    "
            end

            if current_entry.egress_label[1] ~= nil then
                egress_label1 = tostring(current_entry.egress_label[1])
            else
                egress_label1 = "    "
            end

            if current_entry.egress_label[2] ~= nil then
                egress_label2 = tostring(current_entry.egress_label[2])
            else
                egress_label2 = "    "
            end

            if current_entry.entropy_label0 ~= nil then
                entropy_label0 = tostring(current_entry.entropy_label0)
            else
                entropy_label0 = "    " 
            end

            if current_entry.entropy_label1 ~= nil then
                entropy_label1 = tostring(current_entry.entropy_label1)
            else
                entropy_label1 = "    " 
            end

            if current_entry.entropy_label2 ~= nil then
                entropy_label2 = tostring(current_entry.entropy_label2)
            else
                entropy_label2 = "    " 
            end

            if current_entry.cw_value ~= nil then
                cw_value = tostring(current_entry.cw_value)
            else
                cw_value = "   "
            end

            -- convert params to string
            temp =current_entry.my_mac_addr
            my_mac_addr = tostring(temp.string)

            temp =current_entry.nexthop_mac_addr
            nexthop_mac_addr = tostring(temp.string)

            if is_sip_5_15(devEnv.dev) then
                ingress_bobK_labels = ingress_label4.."      "..ingress_label5.."      "
            else
                ingress_bobK_labels = ""
            end

            print("   " .. iterator .. "       " ..
                    current_entry.devNum .. "       " ..
                    current_entry.portNum .. "     " ..
                    current_entry.assign_eport .. "    " ..
                    current_entry.vlan_id  .. "   " ..
                    my_mac_addr .. "  " ..
                    nexthop_mac_addr .. "     " ..
                    current_entry.ingress_label[0].. "       "..
                    ingress_label1.."      "..
                    ingress_label2.."      "..
                    ingress_label3.."      "..
                    ingress_bobK_labels..
                    current_entry.egress_label[0].."      "..
                    entropy_label0.."      "..
                    egress_label1.."      "..
                    entropy_label1.."      "..
                    egress_label2.."      "..
                    entropy_label2.."      "..
                    cw_value)

            print ("")
            iterator = iterator + 1
        end

        current_index , current_entry = next(table_mpls_system_info,current_index)

    end

    print ("End of " .. (iterator - 1) .." MPLS configuration")
    print ("")

    print("global number of MPLS channels in the system "..global_num_mpls_channels)

    current_index , current_entry = next(num_mpls_channel_db,nil)
    while current_entry do
        print("number of MPLS channels defined per physical port ["..current_index.."] = "..current_entry)
        current_index , current_entry = next(num_mpls_channel_db,current_index)
    end

end


-- the command looks like :
--Console(config)# interface ethernet 0/2
--Console(config-if)# mpls assign-eport {eport} vid {vlan-id} my-mac-addr {my-mac-addr} nexthop-mac-addr {nexthop-mac-addr} 
--                         ingress-labels-values-list {ingress-values-list} egress-labels-values-list {egress-values-list} 
--                         [cw-value {cw-value}] [add-egress-entropy-label-after-labels {entropy-labels-locations-list}]
CLI_addCommand("interface", "mpls", {
    func   = mpls_func,
    help   = "Set MPLS configuration on port",
    params = {
        { type = "named",
            { format = "assign-eport %ePort", name = "eport", help = "ePort number"},
            { format="vid %vid", name="vlan_id", help="vlan id" },
            { format="my-mac-addr %mac-address", name="my_mac_addr", help="my mac address"},
            { format="nexthop-mac-addr %mac-address", name="nexthop_mac_addr", help="nexthop mac address"},
            { format = "ingress-labels-values-list %ingress_label_map", name = "ingress_values_list", help = "ingress labels values list. Configures the label value only without EXP and TTL fields"},
            { format = "egress-labels-values-list %egress_label_map",  name = "egress_values_list",  help = "egress labels values list. Configures the label value only without EXP and TTL fields"},
            { format = "cw-value %label", name = "cw_value", help = "control word, optional parameter (defualt is 'do not add MPSL control word')"},
            { format = "add-egress-entropy-label-after-labels %entropy_label_map", name = "entropy_labels_locations_list", help = "index 0 refers to the outermost label"},
            mandatory = {"eport","vlan_id","my_mac_addr","nexthop_mac_addr","ingress_values_list","egress_values_list"}
        }
    }
})

-- the command looks like :
--Console(config)# interface ethernet 0/2
--Console(config-if)# no mpls all
CLI_addCommand("interface", "no mpls all", {
    func   = no_mpls_all_func,
    help   = "Disable MPLS configuration on all ports, restore configurations",
    params = {}
})

-- the command looks like :
--Console(config)# interface ethernet 0/2
--Console(config-if)# no mpls assign-eport {eport}
CLI_addCommand("interface", "no mpls", {
    func   = no_mpls_eport_func,
    help   = "Disable MPLS configuration on eport, restore configuration",
    params = {
        { type = "named",
            { format = "assign-eport %ePort", name = "eport", help = "ePort number to delete"},
            mandatory = { "eport" }
        }
    }
})

-- the command looks like :
--Console# show mpls
--------------------------------------------------------------------------------
-- command registration: show capwap
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show mpls",
    {
        func   = mpls_show,
        help   = "Show mpls info configuration",
        params = {}
    }
)