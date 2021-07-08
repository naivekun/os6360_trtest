--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* interface_pip.lua
--*
--* DESCRIPTION:
--*       set interface (per port) configuration related to PIP.
--*       Pre-Ingress Prioritization (PIP)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--[[ this file commands:
    CLI_addCommand("interface", "pip trust", {
    CLI_addCommand("interface", "no pip trust", {
    CLI_addCommand("interface", "pip profile", {
    CLI_addCommand("interface", "pip vid-ethertype-classification", {
    CLI_addCommand("interface", "no pip vid-ethertype-classification", {
    CLI_addCommand("interface", "pip priority-default", {
    CLI_addCommand("interface", "pip priority-vid-ethertype-classification", {
S]]--
--includes

--constants

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

-- implement per port : 'interface' --> pip trust command
local function per_port__interface_pip_trust(command_data, devNum, portNum, params) 
    local enable = not (params.flagNo == true)
    
    local apiName = "cpssDxChPortPipTrustEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "IN",     "GT_BOOL",    "enable",   enable }
    })

end
-- implement : 'interface' --> pip trust command
local function interface_pip_trust(params)
    --print(to_string(params))
    
    return generic_port_range_func(per_port__interface_pip_trust,params,pip_supported_interfaces)
end

local help_string_pip_trust = "set Ethernet port as 'trusted' for pip"
local help_string_no_pip_trust = "set Ethernet port as 'untrusted' for pip"

CLI_addHelp("interface", "pip", "Support for Pre-Ingress Prioritization (PIP)")
CLI_addHelp("interface", "pip trust", help_string_pip_trust)
CLI_addHelp("interface", "no pip trust", help_string_no_pip_trust)

--[[
Command to set for Ethernet port as 'trusted' for pip.

Context – ethernet

Format: 
pip trust

Parameters:
none
]]--
CLI_addCommand("interface", "pip trust", {
    func = interface_pip_trust,
    help = help_string_pip_trust,
    params = {}
})

CLI_addCommand("interface", "no pip trust", {
    func = function(params)
            params.flagNo = true
            return interface_pip_trust(params)
        end,
    help = help_string_no_pip_trust,
    params = {}
    
})

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################


-- implement per port : 'interface' --> pip profile command
local function per_port__interface_pip_profile(command_data, devNum, portNum, params) 
    local pipProfileList = params.pip_profile
    local pipProfileNumber = tonumber(pipProfileList[1])
    
    --print("params",to_string(params))
    
    if pipProfileNumber == nil or pipProfileList[2] ~= nil then -- more than single value
        -- error. this parameter not supports 'range'
        command_data:addError("ERROR : 'profile' must be single numeric (not range) and not :"..to_string(pipProfileList))
        command_data:setFailStatus()
        command_data:setFailLocalStatus()
        return false
    end
    
    local apiName = "cpssDxChPortPipProfileSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "IN",     "GT_U32",    "pipProfile",   pipProfileNumber }
    })

end

-- implement : 'interface' --> pip profile command
local function interface_pip_profile(params)
    --print(to_string(params))
    
    return generic_port_range_func(per_port__interface_pip_profile,params,pip_supported_interfaces)
end


local help_string_pip_profile = "assign for Ethernet port the pip profile"
CLI_addHelp("interface", "pip profile", help_string_pip_profile)
--[[
Command to assign for Ethernet port the pip profile.

Context – ethernet

Format: 
pip profile <profile_value>

Parameters:
profile_value	- one of 4 pip-profiles (0..3) 
]]--

CLI_addCommand("interface", "pip profile", {
    func = interface_pip_profile,
    help = help_string_pip_profile,
    params =
    {
        { type="values",
           { format = "%pip_profile", name = "pip_profile", help = pip_help_pip_profile }
        }
    }
})

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

-- implement per port : 'interface' --> pip vid-ethertype-classification command
local function per_port__interface_pip_vid_ethertype_classification(command_data, devNum, portNum, params) 
    local enable = not (params.flagNo == true)
    
    local apiName = "cpssDxChPortPipVidClassificationEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "IN",     "GT_BOOL",    "enable",   enable }
    })

end
-- implement : 'interface' --> pip vid-ethertype-classification command
local function interface_pip_vid_ethertype_classification(params)
    --print(to_string(params))
    
    return generic_port_range_func(per_port__interface_pip_vid_ethertype_classification,params,pip_supported_interfaces)
end

local help_string_pip_vid_ethertype_classification = "allow vid and ethertype pairs pip-classification."
local help_string_no_pip_vid_ethertype_classification = "not "..help_string_pip_vid_ethertype_classification

--[[
Command to allow vid and ethertype pairs pip-classification.


Context – ethernet

Format: 
pip vid-ethertype-classification
	
Parameters:
none
]]--
CLI_addCommand("interface", "pip vid-ethertype-classification", {
    func = interface_pip_vid_ethertype_classification,
    help = help_string_pip_vid_ethertype_classification,
    params = {}
})

CLI_addCommand("interface", "no pip vid-ethertype-classification", {
    func = function(params)
            params.flagNo = true
            return interface_pip_vid_ethertype_classification(params)
        end,
    help = help_string_no_pip_vid_ethertype_classification,
    params = {}
    
})
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

-- implement per port : 'interface' --> pip priority-default command
local function per_port__interface_pip_priority_default(command_data, devNum, portNum, params) 
    local priority = params.priority
    local classification_type = "CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_DEFAULT_E"
    
    local apiName = "cpssDxChPortPipPrioritySet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "IN",     "CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT",    "type",    classification_type },
        { "IN",     "GT_U32",    "priority",   priority }
    })

end

-- implement : 'interface' --> pip priority-default command
local function interface_pip_priority_default(params)
    --print(to_string(params))
    
    return generic_port_range_func(per_port__interface_pip_priority_default,params,pip_supported_interfaces)
end

local help_string_pip_priority_default = "set default pip-priority for the port"
CLI_addHelp("interface", "pip priority-default", help_string_pip_priority_default)

--[[
Command to set default pip-priority for the port.

Context – ethernet

Format: 
pip priority-default <priority_value>

Parameters:
priority_value	- the pip-priority (0..2 , 0-high , 1-medium , 2-low)
]]--

CLI_addCommand("interface", "pip priority-default", {
    func = interface_pip_priority_default,
    help = help_string_pip_priority_default,
    params =
    {
        { type="values",
           { format = "%pip_priority_enum", name="priority", help= pip_help_pip_priority }
        }
    }
})

-- implement per port : 'interface' --> pip priority-vid-ethertype-classification command
local function per_port__interface_pip_priority_vid_ethertype_classification(command_data, devNum, portNum, params) 
    local priority = params.priority
    local classification_type = "CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ETHERTYPE_AND_VID_E"
    
    local apiName = "cpssDxChPortPipPrioritySet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "IN",     "CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT",    "type",    classification_type },
        { "IN",     "GT_U32",    "priority",   priority }
    })

end
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

-- implement : 'interface' --> pip priority-vid-ethertype-classification command
local function interface_pip_priority_vid_ethertype_classification(params)
    --print(to_string(params))
    
    return generic_port_range_func(per_port__interface_pip_priority_vid_ethertype_classification,params,pip_supported_interfaces)
end

local help_string_pip_priority_vid_ethertype_classification = "set pip-priority for the vid-ethertype-classification"
CLI_addHelp("interface", "pip priority-vid-ethertype-classification", help_string_pip_priority_vid_ethertype_classification)
--[[
Command to set pip-priority for the vid-ethertype-classification.

Context – ethernet

Format: 
pip priority-vid-ethertype-classification <priority_value>

Parameters:
priority_value	- the pip-priority (0..2 , 0-high , 1-medium , 2-low)
]]--

CLI_addCommand("interface", "pip priority-vid-ethertype-classification", {
    func = interface_pip_priority_vid_ethertype_classification,
    help = help_string_pip_priority_vid_ethertype_classification,
    params =
    {
        { type="values",
           { format = "%pip_priority_enum", name="priority", help= pip_help_pip_priority }
        }
    }
})

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################
