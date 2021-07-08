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
--* mpls_transit.lua
--*
--* DESCRIPTION:
--*       manage mpls transit configurations on phyPort
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
--require
--constants

CLI_type_dict["mpls_command"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "MPLS commands: NOP,POP,PUSH,SWAP,POP2,POP3,POP-SWAP",
    enum = {
        ["NOP"] = { value="CPSS_DXCH_TTI_MPLS_NOP_CMD_E", help="NOP, packet egress with no labels change" },
        ["SWAP"] = { value="CPSS_DXCH_TTI_MPLS_SWAP_CMD_E", help="SWAP the last label with a new one" },
        ["PUSH1"] =  { value="CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E", help="PUSH1, push a new label in the end of the label list" },
        ["POP1"] = { value="CPSS_DXCH_TTI_MPLS_POP1_CMD_E", help="POP1, pop one label from the packet" },
        ["POP2"] = { value="CPSS_DXCH_TTI_MPLS_POP2_CMD_E", help="POP2, pop 2 labels from the packet" },
        ["POP_AND_SWAP"] =  { value="CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E", help="POP_AND_SWAP, pop one label and swap the other one with a new label"},
        ["POP3"] =  { value="CPSS_DXCH_TTI_MPLS_POP3_CMD_E", help="POP3, pop 3 labels from the packet" }
   }
}

local ruleType = "CPSS_DXCH_TTI_RULE_UDB_30_E"
local keyType = "CPSS_DXCH_TTI_KEY_UDB_MPLS_E"
local keySize = "CPSS_DXCH_TTI_KEY_SIZE_30_B_E"
local tunnelType = "CPSS_TUNNEL_X_OVER_MPLS_E"

local ingLabelMaxNum = 4
local ingLabelMaxNum_BobK = 6
local entropyLabelMaxNum = 1
local entropyLocationMaxVal = 1
local entropyLabelMaxNum_BobK = 2
local entropyLocationMaxVal_BobK = 3

local numberOfRegularLabelsInCommand=0;
local numerOfEntropyLabelsInCommand=0;

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
    numberOfRegularLabelsInCommand=0

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
            numberOfRegularLabelsInCommand = numberOfRegularLabelsInCommand + 1
        else
            return false, "The list must only contain numbers in the range 0..0xFFFFF"
        end
    end

    return true,ret
end


local function CLI_check_param_entropy_labels_locations_map(param, data)
    local i,s,e,ret,val
    s=1
    e=1
    i=0
    ret={}
    numerOfEntropyLabelsInCommand=0;

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
        -- currently entropy label is not supported for non BobK devices
        return false, "Entropy Label is not supported for devices other the caelum"
        --[[
        if((numberOfRegularLabelsInCommand==1 or numberOfRegularLabelsInCommand==2)and(numerOfEntropyLabelsInCommand~=1))then
            return false, "Maximum number of entropy labels can be 1"
        else
            if((numberOfRegularLabelsInCommand==0)and(numerOfEntropyLabelsInCommand~=0))then
                return false, "We can not have entropy labels without regular labels"
            else
                if((numberOfRegularLabelsInCommand>2)and(numerOfEntropyLabelsInCommand~=0))then
                    return false, "We can not have entropy labels when we have more then 2 regular labels"
                end
            end
        end
        --]]
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

CLI_type_dict["entropy_labels_locations_map"] = {
    checker = CLI_check_param_entropy_labels_locations_map,
    help = "Value in range 0..4"
}

-- define the UDBs that will be needed for the MPLS transit classification
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
    tunnelTerminate       = GT_FALSE,
    ttPassengerPacketType = "CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E",
    tsPassengerPacketType = "CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E",
    mplsCommand           = FILLED_IN_RUN_TIME,
    command               = "CPSS_PACKET_CMD_FORWARD_E",
    bridgeBypass          = GT_TRUE,
    redirectCommand       = "CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E",
    egressInterface       = {},
    keepPreviousQoS       = GT_TRUE,
    tunnelStart           = GT_FALSE,
    tunnelStartPtr        = 0,
    arpPtr                = 0,
    tag0VlanCmd           = "CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E",
    tag1VlanCmd           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E",
    tag1UpCommand         = "CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E",
    copyTtlExpFromTunnelHeader = GT_TRUE,
    sourceEPortAssignmentEnable = GT_TRUE,
    sourceEPort           = FILLED_IN_RUN_TIME
} }

local configPtr = { mplsCfg = {
    tagEnable             = GT_TRUE,           -- in case we want the packet to egress untagged we need to change in to GT_FALSE
    vlanId                = FILLED_IN_RUN_TIME,
    macDa                 = FILLED_IN_RUN_TIME,
    label1                = FILLED_IN_RUN_TIME,
    controlWordEnable     = FILLED_IN_RUN_TIME,
    controlWordIndex      = FILLED_IN_RUN_TIME,
    ttlMode               = "CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E"
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
local global_num_mpls_transit_channels = 0

-- number of MPLS channel per physical port
local num_mpls_transit_channel_db = {}

--[[ define the table that will hold the 'MPLS configuration done in the system'
     each line in the table hold info about different interface (device/port)
     each interface (device/port) hold next info :
                  1.assign_eport - a unique name
                  2.devNum - device number of the interface
                  3.portNum
                  4.vlan_id
                  5.my_mac_addr
                  6.egress_mac_sa_addr
                  7.nexthop_mac_da_addr
                  8.ingress_label0
                  9.ingress_label1 - optional param
                  10.ingress_label2 - optional param
                  11.ingress_label3 - optional param
                  12.ingress_label4 - optional param
                  13.ingress_label5 - optional param
                  14.entropyLabel0Location - optional param
                  15.entropyLabel1Location - optional param
                  16.mpls_command
                  17.mpls_new_label
                  18.cw_value - optional param
                  19.egress_interface_port_channel (egress_eport/egress_ethernet/egress_port_channel)
                  20.ttiIndex - calculated internally
                  21.tsIndex -  calculated internally
                  22.arpIndex - calculated internally
                  23.cwIndex -  calculated internally
                  24.mac2meIndex - calculated internally
--]]
local table_mpls_transit_system_info = {}
function getTableMplsTransit()
    return table_mpls_transit_system_info
end

local function mpls_transit_command_func(command_data,devNum,portNum, params)

    -- Common variables declaration
    local numberOfEntropyLabels = 0
    local entropyLabelsLocations = {0,0}
    local numberOfLabels = 0
    local cwEnable=false
    local hwDevNum
    local apiName,result
    local ttiIndex,tsIndex,mac2meIndex,cwIndex,arpIndex
    local physicalInfo = {}
    local interfaceInfo = {}
    local egressInterface = {}
    local egress_Interface_to_action = {}
    local egressInfo = {}
    local i,j

    local myIndexName

    if false == is_supported_feature(devNum, "MPLS_CMD_SUPPORT") then
        print("Command not supported for current device")
        return
    end

    -- print input params
    _debug("MPLS command params \n "..to_string(params))

    if(params["entropy_labels_locations_list"]~=nil)then
        numberOfEntropyLabels = table.getn(params["entropy_labels_locations_list"])
        entropyLabelsLocations[0]= params["entropy_labels_locations_list"][1]
        entropyLabelsLocations[1]= params["entropy_labels_locations_list"][2]
    end

    actionPtr.type2.mplsCommand = params["mpls_command"]
    if((params["mpls_command"]=="CPSS_DXCH_TTI_MPLS_SWAP_CMD_E")or
       (params["mpls_command"]=="CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E")or
       (params["mpls_command"]=="CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E"))
        then  numberOfLabels = 1
    else
        numberOfLabels = 0 --POP1, POP2, POP3, NOP
    end

    if params.cw_value ~= nil then
        actionPtr.type2.cwBasedPw = GT_TRUE
        cwEnable = true
    end

    -- change type for input params
    params.vlan_id = tonumber(params.vlan_id)
    params.assign_eport = tonumber(params.assign_eport)

    ------------- ingress configuration ---------------------

    myIndexName = "<MPLS Transit: ingress> assign-eport ".. params.assign_eport
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB

    mac2meIndex = sharedResourceNamedMac2meMemoAlloc(myIndexName, 1 --[[size--]], false --[[fromEnd--]]);

    apiName = "cpssDxChCfgHwDevNumGet"
    result, hwDevNum = device_to_hardware_format_convert(devNum)
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- set Trunk Hash MPLS mode
    apiName = "cpssDxChTrunkHashMplsModeEnableSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_BOOL",       "enable",        true },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
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

    if(cwEnable == true)then
        myIndexName = "<MPLS Transit: ingress> assign-eport ".. params.assign_eport
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

    myIndexName = "<MPLS Transit: ingress> assign-eport ".. params.assign_eport
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

        -- according to the entropy_labels_locations_list parameter
        -- we will mask the entropy label in the TTI rule
        if((j<18) and -- can not add entropy label after udb[18] since it is not supported more then 6 labels.
           ((entropyLabelsLocations[0]~=nill)and(entropyLabelsLocations[0]==(i-1)))or
           ((entropyLabelsLocations[1]~=nill)and(entropyLabelsLocations[1]==(i-1))))then
                 -- before the entropy label we have a special label with value 7,
                 -- entropy label should be masked
                ingress_label = bit_shl(7,4)
                patternPtr.udbArray[j+3] = bit_and(bit_shr(ingress_label,16),0xff)
                patternPtr.udbArray[j+4] = bit_and(bit_shr(ingress_label,8),0xff)
                patternPtr.udbArray[j+5] = bit_and(ingress_label,0xf0)
                maskPtr.udbArray[j+3] = 0xff
                maskPtr.udbArray[j+4] = 0xff
                maskPtr.udbArray[j+5] = 0xf0
               --entropy label mask = 0
                maskPtr.udbArray[j+6] = 0
                maskPtr.udbArray[j+7] = 0
                maskPtr.udbArray[j+8] = 0
               j=j+9
        else
             j=j+3
        end
    end

    for i=j,table.getn(maskPtr.udbArray) do
        maskPtr.udbArray[i] = 0
    end

    ------------- egress configuration ---------------------
    -- set egress_interface
    if (nil ~= params["egress_ethernet"]) then
        local hwDevNum_param,hwPortNum_param

        hwDevNum_param, hwPortNum_param =
            Command_Data:getHWDevicePort(params["egress_ethernet"]["devId"],
                                 params["egress_ethernet"]["portNum"])

        apiName = "cpssDxChCfgHwDevNumGet"
        result, hwDevNum_param = device_to_hardware_format_convert(params["egress_ethernet"]["devId"])
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)

        egress_Interface_to_action.type = "CPSS_INTERFACE_PORT_E"
        egress_Interface_to_action.devPort = {}

        egress_Interface_to_action.devPort.devNum    = hwDevNum_param
        egress_Interface_to_action.devPort.portNum   = hwPortNum_param

        -- the egressInterface will be saved in the DB with the devId parameter and not with the HW devId
        egressInterface.type = "CPSS_INTERFACE_PORT_E"
        egressInterface.devPort = {}

        egressInterface.devPort.devNum    = params["egress_ethernet"]["devId"]
        egressInterface.devPort.portNum   = hwPortNum_param

        -- per Egress port bit Enable Routed packets MAC SA Modification
        apiName = "cpssDxChIpRouterMacSaModifyEnable"
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                "devNum",   devNum},
            { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  egressInterface.devPort.portNum},
            { "IN",     "GT_BOOL",              "enable",   true},
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)

        -- Set the mode, per port, in which the device sets the packet's MAC SA least significant bytes
        apiName = "cpssDxChIpPortRouterMacSaLsbModeSet"
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",                "devNum",   devNum},
            { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  egressInterface.devPort.portNum},
            { "IN",     "CPSS_MAC_SA_LSB_MODE_ENT","saLsbMode", "CPSS_SA_LSB_FULL_48_BIT_GLOBAL"},
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)


        -- Set router mac sa index refered to global MAC SA table
        lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,egressInterface.devPort.portNum,egressInterface.devPort.portNum)

        -- Sets full 48-bit Router MAC SA in Global MAC SA table
        lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,egressInterface.devPort.portNum,params.egress_mac_sa_addr)
    else
        if (nil ~= params["egress_eport"]) then
            local hwDevNum_param,hwPortNum_param

            hwDevNum_param, hwPortNum_param =
            Command_Data:getHWDevicePort(params["egress_eport"]["devId"],
                                 params["egress_eport"]["portNum"])

            apiName = "cpssDxChCfgHwDevNumGet"
            result, hwDevNum_param = device_to_hardware_format_convert(params["egress_eport"]["devId"])
            command_data:handleCpssErrorDevPort(result, apiName.."()")
            _debug(apiName .. "   result = " .. result)

            egress_Interface_to_action.type = "CPSS_INTERFACE_PORT_E"
            egress_Interface_to_action.devPort = {}

            egress_Interface_to_action.devPort.devNum    = hwDevNum_param
            egress_Interface_to_action.devPort.portNum   = hwPortNum_param

            -- the egressInterface will be saved in the DB with the devId parameter and not with the HW devId
            egressInterface.type = "CPSS_INTERFACE_PORT_E"
            egressInterface.devPort = {}

            egressInterface.devPort.devNum    = params["egress_eport"]["devId"]
            egressInterface.devPort.portNum   = hwPortNum_param

            -- per Egress port bit Enable Routed packets MAC SA Modification
            apiName = "cpssDxChIpRouterMacSaModifyEnable"
            result = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",                "devNum",   devNum},
                { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  egressInterface.devPort.portNum},
                { "IN",     "GT_BOOL",              "enable",   true},
            })
            command_data:handleCpssErrorDevPort(result, apiName.."()")
            _debug(apiName .. "   result = " .. result)

            -- Set the mode, per port, in which the device sets the packet's MAC SA least significant bytes
            apiName = "cpssDxChIpPortRouterMacSaLsbModeSet"
            result = myGenWrapper(
                apiName, {
                { "IN",     "GT_U8",                "devNum",   devNum},
                { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  egressInterface.devPort.portNum},
                { "IN",     "CPSS_MAC_SA_LSB_MODE_ENT","saLsbMode", "CPSS_SA_LSB_FULL_48_BIT_GLOBAL"},
            })
            command_data:handleCpssErrorDevPort(result, apiName.."()")
            _debug(apiName .. "   result = " .. result)

            -- Set router mac sa index refered to global MAC SA table
            lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,egressInterface.devPort.portNum,egressInterface.devPort.portNum)

            -- Sets full 48-bit Router MAC SA in Global MAC SA table
            lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,egressInterface.devPort.portNum,params.egress_mac_sa_addr)
        else
            if (nil ~= params["egress_port_channel"]) then
                egressInterface.type = "CPSS_INTERFACE_TRUNK_E"
                egressInterface.trunkId = params["egress_port_channel"]

                egress_Interface_to_action = egressInterface

                local i,v
                local res, vals = get_trunk_device_port_list(egressInterface.trunkId)
                if res == 0 and next(vals) ~= nil then
                    for i,v in pairs(vals[0]) do

                 -- per Egress port bit Enable Routed packets MAC SA Modification
                apiName = "cpssDxChIpRouterMacSaModifyEnable"
                result = myGenWrapper(
                    apiName, {
                    { "IN",     "GT_U8",                "devNum",   devNum},
                    { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  v},
                    { "IN",     "GT_BOOL",              "enable",   true},
                })
                command_data:handleCpssErrorDevPort(result, apiName.."()")
                _debug(apiName .. "   result = " .. result)

                -- Set the mode, per port, in which the device sets the packet's MAC SA least significant bytes
                apiName = "cpssDxChIpPortRouterMacSaLsbModeSet"
                result = myGenWrapper(
                    apiName, {
                    { "IN",     "GT_U8",                "devNum",   devNum},
                    { "IN",     "GT_PHYSICAL_PORT_NUM", "portNum",  v},
                    { "IN",     "CPSS_MAC_SA_LSB_MODE_ENT","saLsbMode", "CPSS_SA_LSB_FULL_48_BIT_GLOBAL"},
                })
                command_data:handleCpssErrorDevPort(result, apiName.."()")
                _debug(apiName .. "   result = " .. result)

                        -- Set router mac sa index refered to global MAC SA table
                        lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,v,v)

                         -- Sets full 48-bit Router MAC SA in Global MAC SA table
                        lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,v,params.egress_mac_sa_addr)
                    end
                end
            end
        end
    end

    actionPtr.type2.egressInterface = egress_Interface_to_action

    if(numberOfLabels==0)then
        -- configure in TTI action the ARP pointer
        egressInfo.tunnelStart = false
        actionPtr.type2.tunnelStart = GT_FALSE
        myIndexName = "<MPLS Transit: egress> assign-eport ".. params.assign_eport
        arpIndex = sharedResourceNamedArpMemoAlloc(myIndexName,1)

        egressInfo.arpPtr = arpIndex
        actionPtr.type2.arpPtr = arpIndex
        egressInfo.modifyMacSa = true
        egressInfo.modifyMacDa = true

        -- set a tunnel start entry
        apiName = "cpssDxChIpRouterArpAddrWrite"
        result = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",         "devNum",         devNum},
            { "IN",     "GT_U32",        "routerArpIndex", arpIndex},
            { "IN",     "GT_ETHERADDR",  "arpMacAddr",     params.nexthop_mac_da_addr},
        })
        command_data:handleCpssErrorDevPort(result, apiName.."()")
        _debug(apiName .. "   result = " .. result)

    else
        -- configure in TTI action the TS pointer
        -- set 'runtime' ts params
        configPtr.mplsCfg.numLabels = numberOfLabels
        configPtr.mplsCfg.vlanId    = params.vlan_id
        configPtr.mplsCfg.macDa     = params.nexthop_mac_da_addr
        configPtr.mplsCfg.label1    = params["mpls_new_label"]

        if (cwEnable == true) then
            configPtr.mplsCfg.controlWordEnable = GT_TRUE
            configPtr.mplsCfg.controlWordIndex = cwIndex -- was already allocated
        end

        myIndexName = "<MPLS Transit: egress> assign-eport ".. params.assign_eport
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

        egressInfo.tunnelStart = true
        egressInfo.tunnelStartPtr = tsIndex
        egressInfo.tsPassengerPacketType  = "CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E" --"CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E"
        actionPtr.type2.tunnelStart = GT_TRUE
        actionPtr.type2.tunnelStartPtr = tsIndex

    end

    -- set 'runtime' action params
    actionPtr.type2.sourceEPort = params.assign_eport
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

    physicalInfo.type = "CPSS_INTERFACE_PORT_E"
    physicalInfo.devPort = {}
    physicalInfo.devPort.devNum  = hwDevNum
    physicalInfo.devPort.portNum = portNum

    -- set physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum},
        { "IN",     "GT_PORT_NUM",             "portNum",         params.assign_eport},
        { "IN",     "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName.."()")
    _debug(apiName .. "   result = " .. result)

    -- set physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum      },
        { "IN",     "GT_PORT_NUM",             "portNum",         params.assign_eport },
        { "IN",     "CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC", "egressInfoPtr", egressInfo},
    })
    command_data:handleCpssErrorDevPort(result, apiName)
    _debug(apiName .. "   result = " .. result)


   -- operation succed save the info into DB
    local entryInfo = {}

    entryInfo.devNum = devNum
    entryInfo.portNum = portNum
    entryInfo.assign_eport = params.assign_eport
    entryInfo.vlan_id = params.vlan_id
    entryInfo.my_mac_addr = params.my_mac_addr
    entryInfo.egress_mac_sa_addr = params.egress_mac_sa_addr
    entryInfo.nexthop_mac_da_addr = params.nexthop_mac_da_addr

    entryInfo.ingress_label={}
    for i=1,table.getn(params["ingress_values_list"]) do
       entryInfo.ingress_label[i-1] = params["ingress_values_list"][i]
    end

    if(params["entropy_labels_locations_list"]~=nil)then
        entryInfo.entropy_labels_locations_list={}
        for i=1,table.getn(params["entropy_labels_locations_list"]) do
            entryInfo.entropy_labels_locations_list[i-1] = params["entropy_labels_locations_list"][i]
        end
    end

    entryInfo.mpls_command = params.mpls_command
    entryInfo.mpls_new_label = params.mpls_new_label
    entryInfo.cw_value  = params.cw_value
    entryInfo.egress_interface_port_channel = egressInterface

    entryInfo.ttiIndex  = ttiIndex
    entryInfo.tsIndex  = tsIndex
    entryInfo.arpIndex  = arpIndex
    entryInfo.cwIndex  = cwIndex
    entryInfo.mac2meIndex  = mac2meIndex

    -- use the assign_eport value as a key to the table
    table_mpls_transit_system_info[params.assign_eport] = entryInfo

    -- another MPLS channel was added
    global_num_mpls_transit_channels = global_num_mpls_transit_channels + 1
    -- another MPLS channel on port was added
    if num_mpls_transit_channel_db[portNum] == nil then
        num_mpls_transit_channel_db[portNum] = 1
    else
        num_mpls_transit_channel_db[portNum] = num_mpls_transit_channel_db[portNum] + 1
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end




-- ************************************************************************
--
--  mpls_transit_port_func
--        @description  Set MPLS transit configurations on phyPort
--
--        @param params             - params["assign_eport"]: ePort number, a unique name
--                                  - params["vlan_id"]: vlan id
--                                  - params["my_mac_address"]: my mac address
--                                  - params["nexthop_mac_da_address"]: nexthop mac address
--                                  - params["ingress_label0"]: ingress label0
--                                  - params["ingress_label1"]: ingress label1, optional param
--                                  - params["ingress_label2"]: ingress label2, optional param
--                                  - params["ingress_label3"]: ingress label3, optional param
--                                  - params["ingress_label4"]: ingress label4, optional param
--                                  - params["ingress_label5"]: ingress label5, optional param
--                                  - params["entropyLabel0Location"]: entropy first location, optional param
--                                  - params["entropyLabel1Location"]: entropy second location, optional param
--                                  - params["mpls_command"]: mpls command : NOP,POP1,POP2, POP3,SWAP,PUSH1,POP_AND_SWAP
--                                  - params["mpls_new_label"]: mpls new label : 'none' in case of NOP,POP1,POP2,POP3 else label value
--                                  - params["cw_value"]: control word, optional param
--                                  - params["egress_interface_port_channel"]: egress_eport/egress_ethernet/egress_port_channel
--
--        @return       true on success, otherwise false and error message
--
local function mpls_transit_func(params)

    local command_data = Command_Data()

    command_data:initInterfaceDevPortRange()
    -- now iterate
    command_data:iterateOverPorts(mpls_transit_command_func,params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

-- ************************************************************************
--
--  no_mpls_transit_eport_command_func
--        @description  Unset MPLS transit configurations on phyPort for assign_eport, restore configuration
--
--        @param params   - params["assign_eport"]: ePort number, a unique name
--
--        @return       true on success, otherwise false and error message
--
local function no_mpls_transit_eport_command_func(command_data,devNum,portNum, params)

    -- Common variables declaration
    local apiName,result
    local ttiIndex,tsIndex,mac2meIndex,cwIndex
    local entryInfo = {}
    local interfaceInfo = {}
    local physicalInfo = {}
    local egressInfo = {}
    local myIndexName
    local mac2meDefaultValuePtr = {
        mac     = 0,
        vlanId  = 4095
    }

    local mac2meDefaultMaskPtr = {
        mac     = convertMacAddrStringTo_GT_ETHERADDR("ff:ff:ff:ff:ff:ff"),
        vlanId  = 4095
    }

    local configZeroPtr = { mplsCfg = {
        tagEnable             = GT_FALSE,
        vlanId                = 0,
        macDa                 = 0,
        label1                = 0,
        controlWordEnable     = GT_FALSE,
        controlWordIndex      = 0,
        ttlMode               = "CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E",
        numLabels             = 1
    } }

    -- print input params
    _debug("no MPLS assign_eport command params \n "..to_string(params))

    -- change type for input params
    params.assign_eport = tonumber(params.assign_eport)

    -- read params from DB
    entryInfo = table_mpls_transit_system_info[params.assign_eport]
    if entryInfo == nil then
        print("assign_eport "..params.assign_eport.." was not defined in MPLS_db")
        return nil
    end

    -- check that current physical port and device are the same as in the DB
    if entryInfo.portNum ~= portNum or entryInfo.devNum ~= devNum then
        print("trying to delete assign_eport "..params.assign_eport.." not from the physical port it was defined")
        return nil
    end

    devNum = entryInfo.devNum
    portNum = entryInfo.portNum
    mac2meIndex = entryInfo.mac2meIndex
    cwIndex = entryInfo.cwIndex
    ttiIndex = entryInfo.ttiIndex
    tsIndex = entryInfo.tsIndex
    local arpIndex = entryInfo.arpIndex

    -- disable Trunk Hash MPLS mode
   apiName = "cpssDxChTrunkHashMplsModeEnableSet"
    result = myGenWrapper(apiName, {
        { "IN",     "GT_U8",         "devNum",        devNum },
        { "IN",     "GT_BOOL",       "enable",        false },
    })
    command_data:handleCpssErrorDevPort(result, apiName)
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

    if(cwIndex~=nil)then
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
    if global_num_mpls_transit_channels == 1 then

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
    if num_mpls_transit_channel_db[portNum] == 1 then

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

    if(tsIndex~=nil)then
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

         myIndexName = "<MPLS Transit: egress> assign-eport ".. params.assign_eport
         sharedResourceNamedTsMemoFree(myIndexName);
    end

    if(arpIndex~=nil)then
        myIndexName = "<MPLS Transit: egress> assign-eport ".. params.assign_eport
        sharedResourceNamedArpMemoFree(myIndexName);
    end

    physicalInfo.type = "CPSS_INTERFACE_PORT_E"
    physicalInfo.devPort = {}
    physicalInfo.devPort.devNum  = 0
    physicalInfo.devPort.portNum = 0

    -- unset physical info for the given ePort
    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",                   "devNum",          devNum},
        { "IN",     "GT_PORT_NUM",             "portNum",         params.assign_eport},
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
        { "IN",     "GT_PORT_NUM",             "portNum",         params.assign_eport },
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

    -- unSet router mac sa index refered to global MAC SA table
    if(entryInfo.egress_interface_port_channel~=nill)then
        if(entryInfo.egress_interface_port_channel.type == "CPSS_INTERFACE_PORT_E")then

            -- unSets full 48-bit Router MAC SA in Global MAC SA table
            lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,entryInfo.egress_interface_port_channel.devPort.portNum,zero)

            lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,entryInfo.egress_interface_port_channel.devPort.portNum,zero)
        else
            local i,v
            local res, vals = get_trunk_device_port_list(entryInfo.egress_interface_port_channel.trunkId)
            --print(to_string(vals))
            if res == 0 and next(vals) ~= nil then
                for i,v in pairs(vals[0]) do

                    -- unSets full 48-bit Router MAC SA in Global MAC SA table
                    lua_cpssDxChIpRouterGlobalMacSaSet(command_data,devNum,v,zero)

                    -- Set router mac sa index refered to global MAC SA table
                    lua_cpssDxChIpRouterPortGlobalMacSaIndexSet(command_data,devNum,v,zero)
                end
            end
        end
    end

    -- operation succed remove the info from DB
    table_mpls_transit_system_info[params.assign_eport] = nil
    num_mpls_transit_channel_db[portNum] = num_mpls_transit_channel_db[portNum] - 1
    global_num_mpls_transit_channels = global_num_mpls_transit_channels - 1

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

local function no_mpls_transit_eport_func(params)

    local command_data = Command_Data()

    command_data:initInterfaceDevPortRange()
    -- now iterate
    command_data:iterateOverPorts(no_mpls_transit_eport_command_func, params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

-- ************************************************************************
--  no_mpls_transit_all_func
--        @description  Unset MPLS transit configurations on all phyPorts, restore configuration
--
--        @param params   - none
--
--        @return           true on success, otherwise false and error message
--
local function no_mpls_transit_all_func()

    local command_data = Command_Data()
    -- Common variables declaration
    local current_index
    local current_entry = {}
    local params = {}
    local apiName,result

    -- print input params
    _debug("no MPLS transit all command no params \n")

    -- get the first element in the table
    current_index , current_entry = next(table_mpls_transit_system_info,nil)

    while current_entry do

        if current_entry.assign_eport ~= nil then
            params.assign_eport = current_entry.assign_eport
            no_mpls_transit_eport_func(params)
        end

        current_index , current_entry = next(table_mpls_transit_system_info,current_index)

    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function getStringWithSpaces(stringValue,totalSizeOfString)
    local originalStringSize
    local numberOfSpacesToAdd
    local stringWithSpaces
    local temp

    originalStringSize = string.len(stringValue)
    if originalStringSize > totalSizeOfString then
        return stringValue
    end

    numberOfSpacesToAdd = totalSizeOfString - originalStringSize
    temp = stringValue

    for i=0,numberOfSpacesToAdd  do
        stringWithSpaces = temp .." "
        temp = stringWithSpaces
    end
    return stringWithSpaces
end

-- ************************************************************************
---
--  mpls_transit_show
--        @description  show MPLS transit info for all interfaces
--
--        @param params     none
--
--        @return           true on success, otherwise false and error message
--
local function mpls_transit_show(params)

    -- show all MPLS transit channels in the system

    --  get the first element in the table
    local current_index , current_entry
    local current_channel_name_data
    local temp
    local ingress_label1,ingress_label2,ingress_label3,ingress_label4,ingress_label5
    local entropyLabel0Location,entropyLabel1Location, cw_value, egress_Interface_value
    local mpls_command, mpls_new_label
    local my_mac_addr
    local egress_mac_sa_addr
    local nexthop_mac_da_addr
    local iterator,port
    local bobK_labels,ingress_bobK_labels

    iterator = 1

    current_index , current_entry = next(table_mpls_transit_system_info,nil)

    if is_sip_5_15(devEnv.dev) then
        bobK_labels = getStringWithSpaces(" i-label4",13).."| "..getStringWithSpaces(" i-label5",13)
    else
        bobK_labels = " "
    end


    temp = getStringWithSpaces("index#",10).."| "..getStringWithSpaces("devNum",18).."| "..getStringWithSpaces("portNum",13).."| ".. getStringWithSpaces("assign_eport",13)
    print(temp)
    temp = getStringWithSpaces(" ",10).."| "..getStringWithSpaces("my-mac-addr",18).."| "..getStringWithSpaces("vlan",13).."| "
            ..getStringWithSpaces(" i-label0",13).."| "..getStringWithSpaces(" i-label1",13).."| " ..getStringWithSpaces(" i-label2",13).."| "..getStringWithSpaces(" i-label3",13)
    print(temp)
    temp =getStringWithSpaces(" ",10).."| "..getStringWithSpaces("egress-mac-sa-addr",18).."| "..getStringWithSpaces(" ",13).."| "..bobK_labels
    print(temp)
    temp =getStringWithSpaces(" ",10).."| "..getStringWithSpaces("nexthop-mac-da-addr",18).."| "..getStringWithSpaces("EL_location0",13).."| "..getStringWithSpaces("EL_location1",13)
    print(temp)
    temp =getStringWithSpaces(" ",10).."| "..getStringWithSpaces("cw-value",18).."| ".. getStringWithSpaces("mpls_command",13).."| ".. getStringWithSpaces("mpls_new_label",13).."|"..getStringWithSpaces("egress_interface port/eport/port_channel",13)
    print(temp)
    print("----------------------------------------------------------------------------------------------------------------------")
    print(" ")

    while current_entry do
        current_channel_name_data = current_entry.assign_eport
        if current_channel_name_data ~= nil then

            if current_entry.ingress_label[1] ~= nil then
                ingress_label1 = tostring(current_entry.ingress_label[1])
            else
                ingress_label1 = "   "
            end

            if current_entry.ingress_label[2] ~= nil then
                ingress_label2 = tostring(current_entry.ingress_label[2])
            else
                ingress_label2 = "   "
            end

            if current_entry.ingress_label[3] ~= nil then
                ingress_label3 = tostring(current_entry.ingress_label[3])
            else
                ingress_label3 = "   "
            end

            if current_entry.ingress_label[4] ~= nil then
                ingress_label4 = tostring(current_entry.ingress_label[4])
            else
                ingress_label4 = "   "
            end

            if current_entry.ingress_label[5] ~= nil then
                ingress_label5 = tostring(current_entry.ingress_label[5])
            else
                ingress_label5 = "   "
            end

            if(current_entry.entropy_labels_locations_list~=nil)then
                if current_entry.entropy_labels_locations_list[0] ~= nil then
                    entropyLabel0Location = tostring(current_entry.entropy_labels_locations_list[0])
                else
                    entropyLabel0Location = "   "
                end

                if current_entry.entropy_labels_locations_list[1] ~= nil then
                    entropyLabel1Location = tostring(current_entry.entropy_labels_locations_list[1])
                else
                    entropyLabel1Location = "   "
                end
            else
                entropyLabel0Location = "   "
                entropyLabel1Location = "   "
            end

            if current_entry.mpls_command ~= nil then
                -- cut CPSS_DXCH_TTI_MPLS_ and _CMD_E from the string
                mpls_command=string.sub(current_entry.mpls_command,20,-7)
            else
                mpls_command = "   "
            end

            if current_entry.mpls_new_label ~= nil then
                mpls_new_label = tostring(current_entry.mpls_new_label)
            else
                mpls_new_label = "   "
            end

            if current_entry.cw_value ~= nil then
                cw_value = tostring(current_entry.cw_value)
            else
                cw_value = "   "
            end

            if (nil ~= current_entry.egress_interface_port_channel) then
                if(current_entry.egress_interface_port_channel.type == "CPSS_INTERFACE_TRUNK_E")then
                    egress_Interface_value = current_entry.egress_interface_port_channel.trunkId
                else
                    egress_Interface_value = tostring(current_entry.egress_interface_port_channel.devPort.devNum).."/"..tostring(current_entry.egress_interface_port_channel.devPort.portNum)
                end
            end

            -- convert params to string
            temp =current_entry.my_mac_addr
            my_mac_addr = tostring(temp.string)

            temp =current_entry.egress_mac_sa_addr
            egress_mac_sa_addr = tostring(temp.string)

            temp =current_entry.nexthop_mac_da_addr
            nexthop_mac_da_addr = tostring(temp.string)

            if is_sip_5_15(devEnv.dev) then
                ingress_bobK_labels = getStringWithSpaces(ingress_label4,13).."| "..getStringWithSpaces(ingress_label5,13)
            else
                ingress_bobK_labels = " "
            end


            print(" ")

            print(getStringWithSpaces(tostring(iterator),10).."| "..
                    getStringWithSpaces(current_entry.devNum,18).."| "..
                    getStringWithSpaces(current_entry.portNum,13).."| "..
                    getStringWithSpaces(current_entry.assign_eport,13))

            print(getStringWithSpaces(" ",10).."| "..
                    getStringWithSpaces(my_mac_addr,18).."| "..
                    getStringWithSpaces(current_entry.vlan_id ,13).."| "..
                    getStringWithSpaces(current_entry.ingress_label[0],13).."| "..
                    getStringWithSpaces(ingress_label1,13).."| "..
                    getStringWithSpaces(ingress_label2,13).."| "..
                    getStringWithSpaces(ingress_label3,13))

            print(getStringWithSpaces(" ",10).."| "..
                   getStringWithSpaces(egress_mac_sa_addr,18).."| "..
                   getStringWithSpaces(" " ,13).."| "..ingress_bobK_labels)

            print(getStringWithSpaces(" ",10).."| "..
                   getStringWithSpaces(nexthop_mac_da_addr,18).."| "..
                   getStringWithSpaces(entropyLabel0Location ,13).."| "..
                   getStringWithSpaces(entropyLabel1Location ,13))

            print(getStringWithSpaces(" ",10).."| "..
                  getStringWithSpaces(cw_value,18).."| "..
                  getStringWithSpaces(mpls_command ,13).."| "..
                  getStringWithSpaces(mpls_new_label ,13).."| "..
                  getStringWithSpaces(egress_Interface_value,13))

            print ("")
            print ("")

            iterator = iterator + 1
        end

        current_index , current_entry = next(table_mpls_transit_system_info,current_index)

    end

    print ("End of " .. (iterator - 1) .." MPLS configuration")
    print ("")

    print("global number of MPLS channels in the system "..global_num_mpls_transit_channels)

    current_index , current_entry = next(num_mpls_transit_channel_db,nil)
    while current_entry do
        print("number of MPLS channels defined per physical port ["..current_index.."] = "..current_entry)
        current_index , current_entry = next(num_mpls_transit_channel_db,current_index)
    end

end


local function CLI_check_param_mpls_new_label(param, data)
    if(param==nil)then
         return false, "MPLS new label must be in the range 0..0xFFFFF in case of PUSH1, SWAP, POP_AND_SWAP mpls_commands, or 'none' for all other commands"
    else
        if(param=="none")then
            return false, "MPLS new label can be 'none' only for NOP,POP1,POP2,POP3 mpls_commands"
        else
            if((tonumber(param)~=nil) and (tonumber(param)<=1048575) and (tonumber(param)>=0)) then
                -- check param in range
                return true,tonumber(param)
            else
                return false, "MPLS new label must be in the range 0..0xFFFFF in case of PUSH1, SWAP, POP_AND_SWAP mpls_commands, or 'none' for all other commands"
            end
        end
    end
end

local function CLI_check_param_mpls_new_label_field_number(param,name,desc,varray,params)
    if (params.mpls_command == "CPSS_DXCH_TTI_MPLS_SWAP_CMD_E" or
        params.mpls_command == "CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E" or
        params.mpls_command=="CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E") then
        return CLI_check_param_mpls_new_label(param,params)
    else
        if param == "none" then
            return true,"none"
        else
            return false,"MPLS new label must be in the range 0..0xFFFFF in case of PUSH1, SWAP, POP_AND_SWAP mpls_commands, or 'none' for all other commands"
        end
    end
end

--------------------------------------------------------------------------------
-- parameter dictionary registration: mpls_new_label
--------------------------------------------------------------------------------
CLI_type_dict["mpls_new_label"] = {
    checker  = {CLI_check_param_mpls_new_label_field_number},
}

--------------------------------------------------------------------------------
-- parameter dictionary registration: egress_interface_port_channel
--------------------------------------------------------------------------------
CLI_addParamDict("egress_interface_port_channel", {
  { format = "egress_ethernet %dev_port", name="egress_ethernet", help = "Specific ethernet dev/port" },
  { format = "egress_port_channel %trunkID",name="egress_port_channel", help = "Valid Port-Channel interface"},
  { format = "egress_eport %dev_ePort",name="egress_eport", help = "Specific dev/ePort"},
  alt = { egress_interface_port_channel = { "egress_ethernet", "egress_port_channel", "egress_eport" }}
})

-- the command looks like :
--Console(config)# interface ethernet 0/2
--Console(config-if)# mpls-transit assign-eport {assign_eport} vid {vlan-id} my-mac-addr {my-mac-addr} nexthop-mac-sa-addr {nexthop-mac-sa-addr}
--                    nexthop-mac-da-addr {nexthop-mac-da-addr} ingress-labels-values-list {ingress-values-list}
--                    [ingress-entropy-label-after-labels {entropy-labels-locations-list}] mpls-command {mpls-command} mpls-new-label {mpls-new-label}
--                    [cw-value {cw-value}] egress-interface {egress_eport/egress_ethernet/egress_port_channel}
--
CLI_addCommand("interface", "mpls-transit", {
    func   = mpls_transit_func,
    help   = "Set MPLS transit configuration on port",
    params = {
        { type = "named",
            { format = "assign-eport %ePort", name = "assign_eport", help = "ePort number"},
            { format="vid %vid", name="vlan_id", help="vlan id" },
            { format="my-mac-addr %mac-address", name="my_mac_addr", help="my mac address"},
            { format="egress-mac-sa-addr %mac-address", name="egress_mac_sa_addr", help="the mac sa address of the ergress packet"},
            { format="nexthop-mac-da-addr %mac-address", name="nexthop_mac_da_addr", help="nexthop mac da address"},
            { format="ingress-labels-values-list %ingress_label_map", name="ingress_values_list", help = "ingress labels values list"},
            { format="entropy-labels-locations-list %entropy_labels_locations_map", name="entropy_labels_locations_list", help = "entropy labels locations list"},
            { format="mpls-command %mpls_command",  name="mpls_command",  help="mpls command applied for egress packet"},
            { format="mpls-new-label %mpls_new_label",  name="mpls_new_label",  help="mpls new label incase the command is SWAP,PUSH1,POP_AND_SWAP"},
            { format="cw-value %label", name = "cw_value", help="control word, optional parameter (defualt is 'do not add MPSL control word')"},
            "#egress_interface_port_channel",
            requirements = {
                    mpls_new_label = {"mpls_command"},
                },
            mandatory = {"assign_eport","vlan_id","my_mac_addr","egress_mac_sa_addr","nexthop_mac_da_addr","ingress_values_list","mpls_command","mpls_new_label","egress_interface_port_channel"}
        }
    }
})

-- the command looks like :
--Console(config)# interface ethernet 0/2
--Console(config-if)# no mpls_transit all
CLI_addCommand("interface", "no mpls-transit all", {
    func   = no_mpls_transit_all_func,
    help   = "Disable MPLS transit configuration on all ports, restore configurations",
    params = {}
})

-- the command looks like :
--Console(config)# interface ethernet 0/2
--Console(config-if)# no mpls_transit assign-eport {assign_eport}
CLI_addCommand("interface", "no mpls-transit", {
    func   = no_mpls_transit_eport_func,
    help   = "Disable MPLS transit configuration on assign_eport, restore configuration",
    params = {
        { type = "named",
            { format = "assign-eport %ePort", name = "assign_eport", help = "ePort number to delete"},
            mandatory = { "assign_eport" }
        }
    }
})

-- the command looks like :
--Console# show mpls_transit
--------------------------------------------------------------------------------
-- command registration: show mpls_transit
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show mpls-transit",
    {
        func   = mpls_transit_show,
        help   = "Show mpls transit info configuration",
        params = {}
    }
)