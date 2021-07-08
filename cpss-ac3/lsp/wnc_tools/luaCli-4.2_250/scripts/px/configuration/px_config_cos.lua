--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_config_cos.lua
--*
--* DESCRIPTION:
--*       Set CoS attributes per Packet and per PacketType
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


-- Information about applicable ranges of <Num Cos Bits> field of
-- <PCP CoS Format Table Entry <n> Register>
local MSG_APPLICABLE_RANGES_OF_NUM_COS_BITS =
    "Applicable ranges of number of CoS bits for Packet CoS Mode:\n" ..
    "  for DSA CoS:       [1..7]\n" ..
    "  for MPLS CoS:      [1..3]\n" ..
    "  for Port L2 CoS:   [1..4]\n" ..
    "  for L3 CoS:        [1..6]\n"


--******************************************************************************
--  setCosFormatEntry
--
--  @description Set CoS Attributes to variable
--
--  @params cosAttr       - CoS Attributes that we will change
--          params["dei"] - Drop Eligibility Indicator
--          params["dp"]  - Drop Precedence
--          params["up"]  - User Priority
--          params["tc"]  - Traffic Class
--
--  @return Changed cosAttr
--
--******************************************************************************
local function setCosAttributes(cosAttr, params)
    if (params["tc"] ~= nil) then
        cosAttr["trafficClass"] = params["tc"]
    end

    if (params["up"] ~= nil) then
        cosAttr["userPriority"] = params["up"]
    end

    if (params["dei"] ~= nil) then
        cosAttr["dropEligibilityIndication"] = params["dei"]
    end

    if (params["dp"] ~= nil) then
        cosAttr["dropPrecedence"] = params["dp"]
    end

    return cosAttr
end


--******************************************************************************
--  isCosNumBitsNotValid
--
--  @description Verify <Num CoS Bits> field of
--               <PCP CoS Format Table Entry <n> Register>
--
--  @params cosType    - CoS Map Type: "l2", "l3", "mpls", "dsa"
--          cosNumBits - Number of CoS Bits
--
--  @return true if cosNumBits NOT VALID. Otherwise return false
--
--******************************************************************************
local function isCosNumBitsNotValid(cosType, cosNumBits)
    if (((cosType == "dsa") and ((cosNumBits < 1) or (cosNumBits > 7))) or
        ((cosType == "mpls") and ((cosNumBits < 1) or (cosNumBits > 3))) or
        ((cosType == "l2") and ((cosNumBits < 1) or (cosNumBits > 4))) or
        ((cosType == "l3") and ((cosNumBits < 1) or (cosNumBits > 6)))) then
        return true
    else
        return false
    end
end


--******************************************************************************
--  setCosFormatEntry
--
--  @description Set CoS Attributes per Packet Type
--
--  @params cosMode              - CoS Mode
--          params["device"]     - device number
--          params["entryIndex"] - entry index (packet type)
--          params["cosType"]    - CoS Map Type
--          params["byteOffset"] - CoS Byte Offset
--          params["bitOffset"]  - CoS Bit Offset
--          params["numCosBits"] - Number of CoS bits
--          params["dei"]        - Drop Eligibility Indicator
--          params["dp"]         - Drop Precedence
--          params["up"]         - User Priority
--          params["tc"]         - Traffic Class
--
--  @return true on success. Otherwise false and error message
--
--******************************************************************************
local function setCosFormatEntry(cosMode, params)
    local devNum = params["device"]
    local entryIndex = params["entryIndex"]
    local cosType, cosNumBits
    local cosFormatEntry
    local ret, val


    ret, val = myGenWrapper("cpssPxCosFormatEntryGet",
        {
            { "IN",  "GT_SW_DEV_NUM", "device", devNum },
            { "IN",  "CPSS_PX_PACKET_TYPE", "packetType", entryIndex },
            { "OUT", "CPSS_PX_COS_FORMAT_ENTRY_STC", "cosFormatEntryPtr" }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxCosFormatEntryGet failed: " .. returnCodes[ret])
        return false, "cpssPxCosFormatEntryGet failed: rc[" .. ret .."]"
    end

    cosFormatEntry = val["cosFormatEntryPtr"]

    -- CoS Mode: Entry CoS Attributes
    if (cosMode == "entry") then

        cosFormatEntry["cosMode"] = "CPSS_PX_COS_MODE_FORMAT_ENTRY_E"

        cosFormatEntry["cosAttributes"] =
                        setCosAttributes(cosFormatEntry["cosAttributes"], params)

    -- CoS Mode: Port CoS Attributes
    elseif (cosMode == "port") then

        cosFormatEntry["cosMode"] = "CPSS_PX_COS_MODE_PORT_E"

    -- CoS Mode: Map Packet CoS
    elseif (cosMode == "packet") then

        cosType = params["cosType"]

        if (cosType == "dsa") then
            cosFormatEntry["cosMode"] = "CPSS_PX_COS_MODE_PACKET_DSA_E"
        elseif (cosType == "mpls") then
            cosFormatEntry["cosMode"] = "CPSS_PX_COS_MODE_PACKET_MPLS_E"
        elseif (cosType == "l2") then
            cosFormatEntry["cosMode"] = "CPSS_PX_COS_MODE_PACKET_L2_E"
        elseif (cosType == "l3") then
            cosFormatEntry["cosMode"] = "CPSS_PX_COS_MODE_PACKET_L3_E"
        else
            print("Wrong CoS Type: " .. cosType)
            return false, "Wrong CoS Type: " .. cosType
        end

        if (params["byteOffset"] ~= nil) then
            cosFormatEntry["cosByteOffset"] = params["byteOffset"]
        end

        if (params["bitOffset"] ~= nil) then
            cosFormatEntry["cosBitOffset"] = params["bitOffset"]
        end

        if (params["numCosBits"] ~= nil) then
            cosNumBits = params["numCosBits"]

            if (isCosNumBitsNotValid(cosType, cosNumBits)) then
                print("Incorrect number of CoS bits: " .. cosNumBits .. "\n" ..
                      MSG_APPLICABLE_RANGES_OF_NUM_COS_BITS)
                return false, "Incorrect number of CoS bits: " .. cosNumBits
            end

            cosFormatEntry["cosNumOfBits"] = cosNumBits
        end

    -- wrong CoS Mode
    else
        print("Wrong CoS mode: " .. cosMode)
        return false, "Wrong CoS mode: " .. cosMode
    end

    ret, val = myGenWrapper("cpssPxCosFormatEntrySet",
        {
            { "IN", "GT_SW_DEV_NUM", "device", devNum },
            { "IN", "CPSS_PX_PACKET_TYPE", "packetType", entryIndex },
            { "IN", "CPSS_PX_COS_FORMAT_ENTRY_STC", "cosFormatEntryPtr",
              cosFormatEntry }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxCosFormatEntrySet failed: " .. returnCodes[ret])

        if ((ret == LGT_BAD_PARAM) and (cosType ~= nil)) then
            cosNumBits = cosFormatEntry["cosNumOfBits"]

            if (isCosNumBitsNotValid(cosType, cosNumBits)) then
                print("\n" ..
                      "Possible problem: incorrect number of CoS bits: " .. cosNumBits .. "\n" ..
                      MSG_APPLICABLE_RANGES_OF_NUM_COS_BITS ..
                      "If you did not specify number of CoS bits it was set from previous value. When\n" ..
                      "you change CoS Type you can have a collision of number of CoS bits.\n" ..
                      "Use \"do show cos format-entry\" command to get information about current values.\n")
            end
        end
        return false, "cpssPxCosFormatEntryGet failed: rc[" .. ret .."]"
    end

    return true
end


--******************************************************************************
-- add help: cos format-entry
--******************************************************************************
CLI_addHelp("config", "cos", "Set CoS attributes")
CLI_addHelp("config", "cos format-entry", "Set CoS attributes per Packet Type")

--******************************************************************************
-- command registration: cos format-entry
--******************************************************************************
local cosParamsDevNum = {
    format = "device %devID",
    name = "device",
    help = "The device ID"
}
local setCosFormatEntryParamPacketType = {
    format = "packet-type %cos_format_entry_packet_type",
    name = "entryIndex",
    help = "Packet Type"
}
local cosParamsDei = {
    format = "dei %cos_drop_eligibility_indicator",
    name = "dei",
    help = "Drop Eligibility Indication"
}
local cosParamsDp = {
    format = "dp %cos_drop_precedence",
    name = "dp",
    help = "Drop Precedence"
}
local cosParamsTc = {
    format = "tc %cos_traffic_class",
    name = "tc",
    help = "Traffic Class"
}
local cosParamsUp = {
    format = "up %cos_user_priority",
    name = "up",
    help = "User Priority"
}

-- cos format-entry entry-mode port
CLI_addCommand("config", "cos format-entry entry-mode port", {
    func = function(params)
               return setCosFormatEntry("port", params)
           end,
    help = "Port CoS Attributes",
    params = {
        {
            type= "named",
            cosParamsDevNum,
            setCosFormatEntryParamPacketType,
            mandatory = {
                "device", "entryIndex"
            }
        }
    }
})

-- cos format-entry entry-mode packet
CLI_addCommand("config", "cos format-entry entry-mode packet", {
    func = function(params)
               return setCosFormatEntry("packet", params)
           end,
    help = "Map Packet CoS",
    params = {
        {
            type= "named",
            {
                format = "type %cos_map_cos_type",
                name = "cosType",
                help = "CoS type"
            },
            cosParamsDevNum,
            setCosFormatEntryParamPacketType,
            {
                format = "bit-offset %cos_format_entry_bit_offset",
                name = "bitOffset",
                help = "CoS Bit Offset relative to CoS byte offset"
            },
            {
                format = "byte-offset %cos_format_entry_byte_offset",
                name = "byteOffset",
                help = "CoS Byte Offset"
            },
            {
                format = "num-bits %cos_format_entry_num_of_bits",
                name = "numCosBits",
                help = "Number of CoS bits to extract from the packet"
            },
            mandatory = {
                "device", "entryIndex", "cosType"
            }
        }
    }
})

-- cos format-entry entry-mode format-entry
CLI_addCommand("config", "cos format-entry entry-mode entry", {
    func = function(params)
               return setCosFormatEntry("entry", params)
           end,
    help = "Entry CoS Attributes",
    params = {
        {
            type= "named",
            cosParamsDevNum,
            setCosFormatEntryParamPacketType,
            cosParamsDei,
            cosParamsDp,
            cosParamsTc,
            cosParamsUp,
            mandatory = {
                "device", "entryIndex"
            }
        }
    }
})


--******************************************************************************
--  setCosMap
--
--  @description Set CoS Attributes per Packet
--
--  @params params["device"]     - device number
--          params["cosType"]    - CoS Mapping Type: "l3", "dsa", "mpls"
--          params["entryIndex"] - entry index
--          params["dei"]        - Drop Eligibility Indicator
--          params["dp"]         - Drop Precedence
--          params["up"]         - User Priority
--          params["tc"]         - Traffic Class
--
--  @return true on success. Otherwise false and error message
--
--******************************************************************************
local function setCosMap(cosType, params)
    local devNum = params["device"]
    local cosAttr
    local entryIndex = params["entryIndex"]
    local apiGet, apiSet, apiIndexName
    local ret, val

    if (cosType == "l3") then
        apiGet = "cpssPxCosL3MappingGet"
        apiSet = "cpssPxCosL3MappingSet"
        apiIndexName = "l3Index"
    elseif (cosType == "mpls") then
        apiGet = "cpssPxCosMplsMappingGet"
        apiSet = "cpssPxCosMplsMappingSet"
        apiIndexName = "mplsIndex"
    elseif (cosType == "dsa") then
        apiGet = "cpssPxCosDsaMappingGet"
        apiSet = "cpssPxCosDsaMappingSet"
        apiIndexName = "dsaIndex"
    else
        print("Wrong CoS Mapping Type: " .. cosType)
        return false, "Wrong CoS Mapping Type: " .. cosType
    end


    ret, val = myGenWrapper(apiGet,
        {
            { "IN",  "GT_SW_DEV_NUM", "device", devNum },
            { "IN",  "GT_U32", apiIndexName, entryIndex },
            { "OUT", "CPSS_PX_COS_ATTRIBUTES_STC", "cosAttributesPtr" }
        })

    if (ret ~= LGT_OK) then
        print(apiGet .. " failed: " .. returnCodes[ret])
        return false, apiGet .. " failed: rc[" .. ret .."]"
    end

    cosAttr = setCosAttributes(val["cosAttributesPtr"], params)

    ret, val = myGenWrapper(apiSet,
        {
            { "IN", "GT_SW_DEV_NUM", "device", devNum },
            { "IN", "GT_U32", apiIndexName, entryIndex },
            { "IN", "CPSS_PX_COS_ATTRIBUTES_STC", "cosAttributesPtr", cosAttr }
        })

    if (ret ~= LGT_OK) then
        print(apiSet .. " failed: " .. returnCodes[ret])
        return false, apiSet .." failed: rc[" .. ret .."]"
    end
end


--******************************************************************************
-- add help: cos map
--******************************************************************************
CLI_addHelp("config", "cos map", "Set CoS attributes per Packet")

--******************************************************************************
-- command registration: cos map
--******************************************************************************

-- cos map type l3
CLI_addCommand("config", "cos map type l3", {
    func   = function(params)
                 return setCosMap("l3", params)
             end,
    help   = "Set CoS attributes per Packet for L3 CoS Mapping",
    params = {
        {
            type="named",
            cosParamsDevNum,
            {
                format = "entry %cos_map_l3_entry_index",
                name = "entryIndex",
                help = "L3 CoS Mapping entry"
            },
            cosParamsDei,
            cosParamsDp,
            cosParamsTc,
            cosParamsUp,
            mandatory = {
                "device", "entryIndex"
            }
        }
    }
})

-- cos map type mpls
CLI_addCommand("config", "cos map type mpls", {
    func   = function(params)
                 return setCosMap("mpls", params)
             end,
    help   = "Set CoS attributes per Packet for MPLS CoS Mapping",
    params = {
        {
            type="named",
            cosParamsDevNum,
            {
                format = "entry %cos_map_mpls_entry_index",
                name = "entryIndex",
                help = "MPLS CoS Mapping entry"
            },
            cosParamsDei,
            cosParamsDp,
            cosParamsTc,
            cosParamsUp,
            mandatory = {
                "device", "entryIndex"
            }
        }
    }
})

-- cos map type dsa
CLI_addCommand("config", "cos map type dsa", {
    func   = function(params)
                 return setCosMap("dsa", params)
             end,
    help   = "Set CoS attributes per Packet for DSA CoS Mapping",
    params = {
        {
            type="named",
            cosParamsDevNum,
            {
                format = "entry %cos_map_dsa_entry_index",
                name = "entryIndex",
                help = "DSA CoS Mapping entry"
            },
            cosParamsDei,
            cosParamsDp,
            cosParamsTc,
            cosParamsUp,
            mandatory = {
                "device", "entryIndex"
            }
        }
    }
})
