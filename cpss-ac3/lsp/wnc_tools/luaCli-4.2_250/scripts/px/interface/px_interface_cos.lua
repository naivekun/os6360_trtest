--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_interface_cos.lua
--*
--* DESCRIPTION:
--*       Set CoS attributes per Port
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


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
--  setCosPortAttributes
--
--  @description Set CoS Attributes per Port
--
--  @params params["dei"]        - Drop Eligibility Indicator
--          params["dp"]         - Drop Precedence
--          params["up"]         - User Priority
--          params["tc"]         - Traffic Class
--
--  @return true on success. Otherwise false and error message
--
--******************************************************************************
local function setCosPortAttributes(command_data, devNum, portNum, params)
    local cosAttr
    local ret, val

    ret, val = myGenWrapper("cpssPxCosPortAttributesGet",
        {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
            { "OUT", "CPSS_PX_COS_ATTRIBUTES_STC", "cosAttributesPtr" }
        })

    if (ret ~= LGT_OK) then
        print("cpssPxCosPortAttributesGet failed: " .. returnCodes[ret])
        return false, "cpssPxCosPortAttributesGet failed: rc[" .. ret .."]"
    end

    cosAttr = setCosAttributes(val["cosAttributesPtr"], params)

    ret, val = myGenWrapper("cpssPxCosPortAttributesSet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
            { "IN", "CPSS_PX_COS_ATTRIBUTES_STC", "cosAttributesPtr", cosAttr }
        })

    if (ret ~= LGT_OK) then
        print("cpssPxCosPortAttributesSet failed: " .. returnCodes[ret])
        return false, "cpssPxCosPortAttributesSet failed: rc[" .. ret .."]"
    end
end


--******************************************************************************
-- add help: cos
--******************************************************************************
CLI_addHelp("interface", "cos", "Set CoS attributes")

--******************************************************************************
-- command registration: cos port-attributes
--******************************************************************************
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

CLI_addCommand("interface", "cos port-attributes", {
    func   = function(params)
                 return generic_port_range_func(setCosPortAttributes, params)
             end,
    help   = "Set CoS attributes per Port",
    params = {
        {
            type="named",
            cosParamsDei,
            cosParamsDp,
            cosParamsTc,
            cosParamsUp
        }
    }
})


--******************************************************************************
--  setCosMapTypeL2
--
--  @description Set CoS Attributes per Packet for Port L2 CoS Mapping
--
--  @params params["entryIndex"] - entry index
--          params["dei"]        - Drop Eligibility Indicator
--          params["dp"]         - Drop Precedence
--          params["up"]         - User Priority
--          params["tc"]         - Traffic Class
--
--  @return true on success. Otherwise false and error message
--
--******************************************************************************
local function setCosMapTypeL2(command_data, devNum, portNum, params)
    local cosAttr
    local entryIndex = params["entryIndex"]
    local ret, val

    ret, val = myGenWrapper("cpssPxCosPortL2MappingGet",
        {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
            { "IN",  "GT_U32", "l2Index", entryIndex },
            { "OUT", "CPSS_PX_COS_ATTRIBUTES_STC", "cosAttributesPtr" }
        })

    if (ret ~= LGT_OK) then
        print("cpssPxCosPortL2MappingGet failed: " .. returnCodes[ret])
        return false, "cpssPxCosPortL2MappingGet failed: rc[" .. ret .."]"
    end

    cosAttr = setCosAttributes(val["cosAttributesPtr"], params)

    ret, val = myGenWrapper("cpssPxCosPortL2MappingSet",
        {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
            { "IN", "GT_U32", "l2Index", entryIndex },
            { "IN", "CPSS_PX_COS_ATTRIBUTES_STC", "cosAttributesPtr", cosAttr }
        })

    if (ret ~= LGT_OK) then
        print("cpssPxCosPortL2MappingSet failed: " .. returnCodes[ret])
        return false, "cpssPxCosPortL2MappingSet failed: rc[" .. ret .."]"
    end
end


--******************************************************************************
-- add help: cos packet-attributes
--******************************************************************************
CLI_addHelp("interface", "cos map", "Set CoS attributes per Packet")

--******************************************************************************
-- command registration: cos map type l2
--******************************************************************************
CLI_addCommand("interface", "cos map type l2", {
    func   = function(params)
                 return generic_port_range_func(setCosMapTypeL2, params)
             end,
    help   = "Set CoS attributes per Packet for Port L2 CoS Mapping",
    params = {
        {
            type="named",
            {
                format = "entry %cos_map_l2_entry_index",
                name = "entryIndex",
                help = "Port L2 CoS Mapping entry"
            },
            cosParamsDei,
            cosParamsDp,
            cosParamsTc,
            cosParamsUp,
            mandatory = {
                "entryIndex"
            }
        }
    }
})
