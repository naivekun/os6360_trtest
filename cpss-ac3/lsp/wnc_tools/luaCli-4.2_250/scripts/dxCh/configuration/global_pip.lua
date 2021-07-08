--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* global_pip.lua
--*
--* DESCRIPTION:
--*       set global (not per port) configuration related to PIP.
--*       Pre-Ingress Prioritization (PIP)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--[[ this file commands:
	CLI_addCommand("config", "pip enable",
	CLI_addCommand("config", "no pip enable",
	CLI_addCommand("config", "expect pip enable",
	CLI_addCommand("config", "expect no pip enable",
    
	CLI_addCommand("config", "pip vid-ethertype",
	CLI_addCommand("config", "no pip vid-ethertype",
	CLI_addCommand("config", "expect pip vid-ethertype",
    CLI_addCommand("config", "expect no pip vid-ethertype",
    
	CLI_addCommand("config", "pip user-defined-ethertype",
	CLI_addCommand("config", "no pip user-defined-ethertype",
	CLI_addCommand("config", "expect pip user-defined-ethertype",
	CLI_addCommand("config", "expect no pip user-defined-ethertype",
    
	CLI_addCommand("config", "pip mac-da",
	CLI_addCommand("config", "no pip mac-da",
    CLI_addCommand("config", "expect pip mac-da",
	CLI_addCommand("config", "expect no pip mac-da",
	    
	CLI_addCommand("config", "pip profile",
    CLI_addCommand("config", "expect pip profile",
    
	CLI_addCommand("config", "pip threshold",
    CLI_addCommand("config", "expect pip threshold",
]]--
-- build 'expect' help string , from the 'help' string of the relevant 'config' string
local function help_build_expect_string(configString)
    return "check with expected value the '" .. configString .."'" 
end

--includes


--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

 
--constants

-- implement 'per device' : 'config' --> pip enable command
local function per_device__config_pip_enable(command_data,devNum,params)
    local enable = not (params.flagNo == true)
    
    local apiName = "cpssDxChPortPipGlobalEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_BOOL",    "enable",     enable }
    })

end

-- implement : 'config' --> pip enable command
local function config_pip_enable(params)
    -- init pip DB manager
    pip_db_manager_init()

    return generic_all_device_func(per_device__config_pip_enable,params)
end

-- implement 'per device' : 'config' --> pip enable command
local function per_device__config_expect_pip_enable(command_data,devNum,params)
    local enable = not (params.flagNo == true)
    
    local apiName = "cpssDxChPortPipGlobalEnableGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "OUT",     "GT_BOOL",    "enable",     enable}-->we compare the value !
    })

end

local function config_expect_pip_enable(params)
    -- init pip DB manager
    pip_db_manager_init()

    return generic_all_device_func(per_device__config_expect_pip_enable,params)
end



local help_string_pip_enable  = "enable pip on specific device or on all devices"
local help_string_no_pip_enable  = "disable pip on specific device or on all devices"
local help_string_expect_pip_enable = help_build_expect_string(help_string_pip_enable)
local help_string_expect_no_pip_enable = help_build_expect_string(help_string_no_pip_enable)

CLI_addHelp("config", "pip", "Support for Pre-Ingress Prioritization (PIP)")
CLI_addHelp("config", "pip enable", help_string_pip_enable)
CLI_addHelp("config", "no pip enable", help_string_no_pip_enable)
CLI_addHelp("config", "expect pip enable", help_string_expect_pip_enable)
CLI_addHelp("config", "expect no pip enable", help_string_expect_no_pip_enable)

--[[
Context – config

Format:
pip enable device <all / <devNum>>
]]--
CLI_addCommand("config", "pip enable",
    { func = config_pip_enable,
      help = help_string_pip_enable,
      params =
      {
            { type="named",
            "#all_device",-- specific device or on all devices
               mandatory = {"all_device"}
            }
      }
    });
 
CLI_addCommand("config", "no pip enable",
    { func = function(params)
            params.flagNo = true
            return config_pip_enable(params)
        end,
      help = help_string_no_pip_enable,
      params =
      {
            { type="named",
            "#all_device",-- specific device or on all devices
               mandatory = {"all_device"}
            }
      }
    });
    
CLI_addCommand("config", "expect pip enable",
    { func = config_expect_pip_enable,
      help = help_string_expect_pip_enable,
      params =
      {
            { type="named",
            "#all_device",-- specific device or on all devices
               mandatory = {"all_device"}
            }
      }
    });
    
CLI_addCommand("config", "expect no pip enable",
    { func = function(params)
            params.flagNo = true
            return config_expect_pip_enable(params)
        end,
      help = help_string_expect_no_pip_enable,
      params =
      {
            { type="named",
            "#all_device",-- specific device or on all devices
               mandatory = {"all_device"}
            }
      }
    });
    
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

local help_ude_name    = "A unique name (string) for one of the 4 user defined ethertypes , for example : \"pip-ude1\" "
local help_mac_da_name = "A unique name (string) for one of the 4 mac-da {pattern,mask} , for example : \"pip-mac-da1\" "


-- implement 'per device' : 'config' --> pip vid-ethertype command
local function per_device__config_pip_vid_ethertype_classification(command_data,devNum,params)

    --print("params ", to_string(params))
    
    local unset = (params.flagNo == true)
    local ethertypeString = "0x"..params.ethertype
    local ethertype = tonumber(ethertypeString)
    local vid = params.vid --(optional) parameter named - "vid" (0..4095),
    
    if(ethertype == nil) then
        command_data:addError("ERROR : ethertype ".. params.ethertype .." is not in HEX format (need to be without 0x)")
        command_data:setFailStatus()
        command_data:setFailLocalStatus()
        return false
    end
    
    if vid == nil then 
        vid = pip_vid_hw_default 
    end

    if (unset == true) then
        -- check that entry in DB 
        local isAlreadyExists = pip_db_vid_index_check(ethertypeString,vid)
        if isAlreadyExists == false then
            command_data:addError("ERROR : can't find : {ethertype ".. params.ethertype .." , vid " .. vid .."} in table")
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
            return false
        end
    end

    local index , isAlreadyExists = pip_db_vid_index_get(ethertypeString,vid)
    if index == nil then
        -- we will not get here when (unset == true) because we already checked that 'entry' exists
        command_data:addError("ERROR : table is FULL : can't add : {ethertype ".. params.ethertype .." , vid " .. vid .."}")
        command_data:setFailStatus()
        command_data:setFailLocalStatus()
        return false
    end

    if (unset == true) then
        -- remove the entry from the DB
        pip_db_vid_index_reset(ethertypeString,vid)
    
        -- we got index of the 'entry'
        -- so we need to set 'default' of 'entry'
        ethertype = tonumber(pip_tpid_hw_default[index])
        vid = pip_vid_hw_default
    end
    
    
    local protocol = "CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E"
    
    local apiName = "cpssDxChPortPipGlobalEtherTypeProtocolSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",     protocol },
        { "IN",     "GT_U32",    "index",       index },
        { "IN",     "GT_U16",    "etherType",   ethertype }
    })

    local apiName = "cpssDxChPortPipGlobalVidClassificationSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U32",    "index",       index },
        { "IN",     "GT_U16",    "vid",         vid }
    })
    
    return true
end

-- implement : 'config' --> pip vid-ethertype command
local function config_pip_vid_ethertype_classification(params)
    -- init pip DB manager
    pip_db_manager_init()
    --print("params ", to_string(params))

    return generic_all_device_func(per_device__config_pip_vid_ethertype_classification,params)
end

-- implement 'per device' : 'config' --> pip vid-ethertype command
local function per_device__config_expect_pip_vid_ethertype_classification(command_data,devNum,params)
    --print("params ", to_string(params))
    local unset = (params.flagNo == true)

    local ethertypeString = "0x"..params.ethertype
    local ethertype = tonumber(ethertypeString)
    local vid = params.vid --(optional) parameter named - "vid" (0..4095),
    
    if(ethertype == nil) then
        command_data:addError("ERROR : ethertype ".. params.ethertype .." is not in HEX format (need to be without 0x)")
        command_data:setFailStatus()
        command_data:setFailLocalStatus()
        return false
    end
    
    if vid == nil then 
        vid = pip_vid_hw_default 
    end

    local isAlreadyExists = pip_db_vid_index_check(ethertypeString,vid)
    if isAlreadyExists == false and unset == false then
        command_data:addError("ERROR : can't find in DB : {ethertype ".. params.ethertype .." , vid " .. vid .."}")
        command_data:setFailStatus()
        command_data:setFailLocalStatus()
        return false
    elseif isAlreadyExists == true and unset == true then
        command_data:addError("ERROR : (was not removed) sill in DB : {ethertype ".. params.ethertype .." , vid " .. vid .."}")
        command_data:setFailStatus()
        command_data:setFailLocalStatus()
        return false
    end

    if (unset == true) then
        -- we not have index to check the HW
        -- we any checked that DB not hold it any more
        return true
    end
    
    local index , isAlreadyExists = pip_db_vid_index_get(ethertypeString,vid)

    local protocol = "CPSS_DXCH_PORT_PIP_PROTOCOL_VLAN_TAG_E"
    
    local apiName = "cpssDxChPortPipGlobalEtherTypeProtocolGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",     protocol },
        { "IN",     "GT_U32",    "index",       index },
        { "OUT",     "GT_U16",    "etherType",   ethertype}-->we compare the value !
    })

    local apiName = "cpssDxChPortPipGlobalVidClassificationGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U32",    "index",       index },
        { "OUT",     "GT_U16",    "vid",        vid}-->we compare the value !
    })
    
    return true
end

local function config_expect_pip_vid_ethertype_classification(params)
    -- init pip DB manager
    pip_db_manager_init()
    --print("params ", to_string(params))

    return generic_all_device_func(per_device__config_expect_pip_vid_ethertype_classification,params)
end

local help_string_pip_vid_ethertype_classification = "set the ethertype and vid for packets classification as vlan-tag"
local help_string_no_pip_vid_ethertype_classification = "un"..help_string_pip_vid_ethertype_classification
local help_string_expect_pip_vid_ethertype_classification = help_build_expect_string(help_string_pip_vid_ethertype_classification)
local help_string_expect_no_pip_vid_ethertype_classification = help_build_expect_string(help_string_no_pip_vid_ethertype_classification)

CLI_addHelp("config", "pip vid-ethertype", help_string_pip_vid_ethertype_classification)
CLI_addHelp("config", "no pip vid-ethertype", help_string_no_pip_vid_ethertype_classification)
CLI_addHelp("config", "expect pip vid-ethertype", help_string_expect_pip_vid_ethertype_classification)
CLI_addHelp("config", "expect no pip vid-ethertype", help_string_expect_no_pip_vid_ethertype_classification)

--[[
Command to set {vid, ethertype} for classification as 'vlan tagged'.

Context – config

Format: 
pip vid-ethertype device <all / <devNum>> ethertype < ethertype_value > [vid <vid_value>]

Parameters:
device 		– 'all' or specific device number
ethertype	- the ethertype value as 'HEX' number without '0x' (8100,88a8,9100...) 
vid		- (optional  - default is 0) Specific IEEE 802.1Q VLAN ID (0..4095)
priority 	- the pip-priority (0..2 , 0-high , 1-medium , 2-low)

]]--

CLI_addCommand("config", "pip vid-ethertype",
    { func = config_pip_vid_ethertype_classification ,
      help = help_string_pip_vid_ethertype_classification,
      params =
      {
            { type="named",
                "#all_device",-- specific device or on all devices
               { format = "etherType %pip_vlan_tag_ethertype", name = "ethertype", help = pip_help_vlan_tag_ethertype }, 
                "#pip-vid-value",--(optional) parameter named - "vid" (0..4095),
               
               mandatory = {"all_device" , "ethertype"}
            }
      }
    });

CLI_addCommand("config", "no pip vid-ethertype",
    { func = function(params)
            params.flagNo = true
            return config_pip_vid_ethertype_classification(params) 
        end,
      help = help_string_no_pip_vid_ethertype_classification,
      params =
      {
            { type="named",
                "#all_device",-- specific device or on all devices
               { format = "etherType %pip_vlan_tag_ethertype", name = "ethertype", help = pip_help_vlan_tag_ethertype }, 
                "#pip-vid-value",--(optional) parameter named - "vid" (0..4095),
               
               mandatory = {"all_device" , "ethertype"}
            }
      }
    });

CLI_addCommand("config", "expect pip vid-ethertype",
    { func = config_expect_pip_vid_ethertype_classification ,
      help = help_string_expect_pip_vid_ethertype_classification,
      params =
      {
            { type="named",
                "#all_device",-- specific device or on all devices
               { format = "etherType %pip_vlan_tag_ethertype", name = "ethertype", help = pip_help_vlan_tag_ethertype }, 
                "#pip-vid-value",--(optional) parameter named - "vid" (0..4095),
               
               mandatory = {"all_device" , "ethertype"}
            }
      }
    });

CLI_addCommand("config", "expect no pip vid-ethertype",
    { func = function(params)
            params.flagNo = true
            return config_expect_pip_vid_ethertype_classification(params) 
        end,
      help = help_string_expect_no_pip_vid_ethertype_classification,
      params =
      {
            { type="named",
                "#all_device",-- specific device or on all devices
               { format = "etherType %pip_vlan_tag_ethertype", name = "ethertype", help = pip_help_vlan_tag_ethertype }, 
                "#pip-vid-value",--(optional) parameter named - "vid" (0..4095),
               
               mandatory = {"all_device" , "ethertype"}
            }
      }
    });
    
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

-- implement 'per device' : 'config' --> user-defined-ethertype
local function per_device__config_pip_user_defined_ethertype(command_data,devNum,params)

    --print("params ", to_string(params))
    
    local unset = (params.flagNo == true)
    local ethertype = params.ethertype
    local index = params.pip_index
    
    if (unset == false) then
        local name = params.name
        -- add the device to the DB 
        pip_db_ude_name_device_add(name,devNum)
    end
    
    local protocol = "CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E"
    
    local apiName = "cpssDxChPortPipGlobalEtherTypeProtocolSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",     protocol },
        { "IN",     "GT_U32",    "index",       index },
        { "IN",     "GT_U16",    "etherType",   ethertype }
    })
end
    
-- implement : 'config' --> pip user-defined-ethertype command
local function config_pip_user_defined_ethertype(params)
    -- init pip DB manager
    pip_db_manager_init()

    local command_data = Command_Data()
    local unset = (params.flagNo == true)
    local name = params.name
    local index
    local didError = false

    if(unset == false) then
        local ethertypeString = "0x"..params.ethertype
        local ethertype = tonumber(ethertypeString)

        if(ethertype == nil) then
            command_data:addError("ERROR : ethertype ".. params.ethertype .." is not in HEX format (need to be without 0x)")
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
            didError = true
        end

        if (didError == false) then
            local isAlreadyExists
            index , isAlreadyExists = pip_db_ude_index_get(ethertypeString)
            if index == nil then
                command_data:addError("ERROR : can't add new ethertype to classify 'UDE' : ".. params.ethertype)
                command_data:setFailStatus()
                command_data:setFailLocalStatus()
                didError = true
            end

            if (didError == false) then
                if isAlreadyExists == false then
                    -- check that the 'name' is not bound to other ethertype
                    local check_index = pip_db_ude_name_index_get(name)
                    if check_index ~= nil then
                        local previous_ethertype = pip_db_ude_ethertype_get(check_index)
                        command_data:addError("ERROR : the name : ".. name .. " already associated with ethertype" .. previous_ethertype )
                        
                        -- remove the 'ethertype' from the DB
                        pip_db_ude_ethertype_index_reset(index)
                        
                        command_data:setFailStatus()
                        command_data:setFailLocalStatus()
                        didError = true
                    end
                else -- ethertype already exists
                    command_data:addError("ERROR : the ethertype already used : ".. params.ethertype)
                    command_data:setFailStatus()
                    command_data:setFailLocalStatus()
                    didError = true
                end
            end
                
            if (didError == false) then
                -- bind 'name' to 'index'
                if isAlreadyExists == false then
                    pip_db_ude_name_index_set(name,index)
                end

                local ethertypeString = "0x"..params.ethertype

                -- let internal function be aware to 'index' , ethertype
                params.pip_index = index
                params.ethertype = tonumber(ethertypeString)

                return generic_all_device_func(per_device__config_pip_user_defined_ethertype,params)
            end
        end
    else
        -- check that entry in DB 
        index = pip_db_ude_name_index_get(name)
        if index == nil then
            command_data:addError("ERROR : the name : " .. name .. " was not found")
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
            didError = true
        end

        if (didError == false) then
            -- let internal function be aware to 'index' , ethertype
            params.pip_index = index
            params.ethertype = pip_ude_default

            -- the 'device' parameter not comes from the command ... we need to retrieve it from the DB
            local numDevs = pip_db_ude_name_device_num_get(name)
            local devNum
            for dev_idx = 1,numDevs do
                devNum = pip_db_ude_name_device_id_get(name,dev_idx)
                per_device__config_pip_user_defined_ethertype(command_data,devNum,params)
            end
            -- remove the name from the DB
            pip_db_ude_name_reset(name)
            -- remove the index from db
            pip_db_ude_ethertype_index_reset(index)
        end

    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- implement 'per device' : 'config' --> user-defined-ethertype
local function per_device__config_expect_pip_user_defined_ethertype(command_data,devNum,params)

    --print("params ", to_string(params))
    
    local unset = (params.flagNo == true)
    local ethertype = params.ethertype
    local index = params.pip_index
    
    if (unset == false) then
        local name = params.name
        -- check that the device is in the DB 
        local devIndex = pip_db_ude_name_device_index_get(name,devNum)
        if devIndex == nil then
            command_data:addError("ERROR : ".. name .." not hold devNum = " .. devNum .. "in DB")
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
        end
    end
    
    local protocol = "CPSS_DXCH_PORT_PIP_PROTOCOL_UDE_E"
    
    local apiName = "cpssDxChPortPipGlobalEtherTypeProtocolGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "CPSS_DXCH_PORT_PIP_PROTOCOL_ENT",    "protocol",     protocol },
        { "IN",     "GT_U32",    "index",       index },
        { "OUT",     "GT_U16",    "etherType",   ethertype}-->we compare the value !
    })
end

local function config_expect_pip_user_defined_ethertype(params)
    -- init pip DB manager
    pip_db_manager_init()

    local command_data = Command_Data()
    local unset = (params.flagNo == true)
    local name = params.name
    local index
    local didError = false

    if(unset == false) then
        local ethertypeString = "0x"..params.ethertype
        local ethertype = tonumber(ethertypeString)

        if(ethertype == nil) then
            command_data:addError("ERROR : ethertype ".. params.ethertype .." is not in HEX format (need to be without 0x)")
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
            didError = true
        end

        if (didError == false) then
            local isAlreadyExists
            
            isAlreadyExists = pip_db_ude_index_check(ethertypeString)
            if isAlreadyExists == false then
                command_data:addError("ERROR : not found in DB : ethertype : ".. params.ethertype)
                command_data:setFailStatus()
                command_data:setFailLocalStatus()
                didError = true
            else
                -- get the index
                index , isAlreadyExists = pip_db_ude_index_get(ethertypeString)
            end
                
            if (didError == false) then
                -- check 'name' to 'index'
                local nameIndex = pip_db_ude_name_index_get(name)
                if nameIndex == nil then nameIndex = "not found" end
                
                if nameIndex ~= index then
                    command_data:addError("ERROR : Error in DB : expected index : " .. index .. " got: " .. nameIndex)
                    command_data:setFailStatus()
                    command_data:setFailLocalStatus()
                    didError = true
                end

                local ethertypeString = "0x"..params.ethertype

                -- let internal function be aware to 'index' , ethertype
                params.pip_index = index
                params.ethertype = tonumber(ethertypeString)

                return generic_all_device_func(per_device__config_expect_pip_user_defined_ethertype,params)
            end
        end
    else
        -- check that entry in DB 
        index = pip_db_ude_name_index_get(name)
        if index ~= nil then
            command_data:addError("ERROR : " .. name .. " still exists in DB at index " .. index)
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
            didError = true
        end

        if (didError == false) then
            -- let internal function be aware to 'index' , ethertype
            params.pip_index = index
            params.ethertype = pip_ude_default

            -- the 'device' parameter not comes from the command ... we need to retrieve it from the DB
            local numDevs = pip_db_ude_name_device_num_get(name)
            if numDevs ~= 0 then
                command_data:addError("ERROR : " .. name .. " still hold " .. numDevs .." devices in DB ")
                command_data:setFailStatus()
                command_data:setFailLocalStatus()
                didError = true
            end
        end

    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end
    
local help_string_pip_user_defined_ethertype = "'name' an ethertype for classification as 'user defined ethertype'"
local help_string_pip_no_user_defined_ethertype = "un-" .. help_string_pip_user_defined_ethertype
local help_string_expect_pip_user_defined_ethertype = help_build_expect_string(help_string_pip_user_defined_ethertype)
local help_string_expect_no_pip_user_defined_ethertype = help_build_expect_string(help_string_pip_no_user_defined_ethertype)


CLI_addHelp("config", "pip user-defined-ethertype", help_string_pip_user_defined_ethertype)
CLI_addHelp("config", "no pip user-defined-ethertype", help_string_pip_no_user_defined_ethertype)
CLI_addHelp("config", "expect pip user-defined-ethertype", help_string_expect_pip_user_defined_ethertype)
CLI_addHelp("config", "expect no pip user-defined-ethertype", help_string_expect_no_pip_user_defined_ethertype)

--[[
Command to 'name' an ethertype for classification as 'user defined ethertype'.

Context – config

Format: 
pip user-defined-ethertype <name> device <all / <devNum>> ethertype < ethertype_value >  

Parameters:
name		- a unique name for one of the 4 user defined ethertypes.
device 		– 'all' or specific device number
ethertype	- the ethertype value as 'HEX' number without '0x' (6666,abcd,789a...) 

]]-- 
CLI_addCommand("config", "pip user-defined-ethertype",
    { func = config_pip_user_defined_ethertype ,
      help = help_string_pip_user_defined_ethertype,
      params =
      {
            { type="values",
               { format = "%string", name = "name", help = help_ude_name }
            },
            { type="named",
                "#all_device",-- specific device or on all devices
               { format = "etherType %pip_generic_ethertype", name = "ethertype", help = pip_help_generic_ethertype },
               
               mandatory = {"all_device" , "ethertype"}
            }
      }
    });
CLI_addCommand("config", "no pip user-defined-ethertype",
    { func = function(params)
            params.flagNo = true
            return config_pip_user_defined_ethertype(params) 
        end,
      help = help_string_pip_no_user_defined_ethertype,
      params =
      {
            { type="values",
               { format = "%string", name = "name", help = help_ude_name }, 
            }
      }
    });
CLI_addCommand("config", "expect pip user-defined-ethertype",
    { func = config_expect_pip_user_defined_ethertype ,
      help = help_string_expect_pip_user_defined_ethertype,
      params =
      {
            { type="values",
               { format = "%string", name = "name", help = help_ude_name }
            },
            { type="named",
                "#all_device",-- specific device or on all devices
               { format = "etherType %pip_generic_ethertype", name = "ethertype", help = pip_help_generic_ethertype },
               
               mandatory = {"all_device" , "ethertype"}
            }
      }
    });
CLI_addCommand("config", "expect no pip user-defined-ethertype",
    { func = function(params)
            params.flagNo = true
            return config_expect_pip_user_defined_ethertype(params) 
        end,
      help = help_string_expect_no_pip_user_defined_ethertype,
      params =
      {
            { type="values",
               { format = "%string", name = "name", help = help_ude_name }, 
            }
      }
    });

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

-- implement 'per device' : 'config' --> 'pip mac-da'
local function per_device__config_pip_mac_da(command_data,devNum,params)

    --print("params ", to_string(params))
    
    local unset = (params.flagNo == true)
    local macAddr       = params.macAddr
    local macAddrMask   = params.macAddrMask
    local index = params.pip_index
    
    if (unset == false) then
        local name = params.name
        -- add the device to the DB 
        pip_db_mac_da_name_device_add(name,devNum)
    end
    
    local entry = {macAddrValue = macAddr, macAddrMask = macAddrMask}
    
    local apiName = "cpssDxChPortPipGlobalMacDaClassificationEntrySet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U32",    "index",       index },
        { "IN",     "CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC",    "entryPtr",   entry }
    })
end


-- implement : 'config' --> pip mac-da command
local function config_pip_mac_da(params)
    -- init pip DB manager
    pip_db_manager_init()

    --print(to_string(params))
    
    local command_data = Command_Data()
    local unset = (params.flagNo == true)
    local name = params.name
    local index
    local didError = false

    if(unset == false) then
        local macAddr       = params["mac-address"]
        local macAddrMask   = params["mac_mask"]
    
        local isAlreadyExists
        index , isAlreadyExists = pip_db_mac_da_index_get(macAddr.string,macAddrMask.string)
        if index == nil then
            command_data:addError("ERROR : Table FULL : can't add new mac ".. macAddr.string .. " mask : " .. macAddrMask.string)
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
            didError = true
        end

        if (didError == false) then
            if isAlreadyExists == false then
                -- check that the 'name' is not bound to other mac-da {pattern,mask}
                local check_index = pip_db_mac_da_name_index_get(name)
                if check_index ~= nil then
                    local previous_mac_pattern , previous_mac_mask = pip_db_mac_da_entry_get(check_index)
                    command_data:addError("ERROR : the name : ".. name .. " already associated with mac-da:" .. previous_mac_pattern .. " mask : " .. previous_mac_mask )
                    command_data:setFailStatus()
                    command_data:setFailLocalStatus()
                    didError = true
                end
            else -- mac-da already exists
                command_data:addError("ERROR : the mac-da already used : ".. macAddr.string .. " mask : " .. macAddrMask.string)
                command_data:setFailStatus()
                command_data:setFailLocalStatus()
                didError = true
            end
        end
            
        if (didError == false) then
            -- bind 'name' to 'index'
            if isAlreadyExists == false then
                pip_db_mac_da_name_index_set(name,index)
            end

            -- let internal function be aware to 'index' , mac-da{pattern,mask}
            params.pip_index = index
            params.macAddr = macAddr.string
            params.macAddrMask = macAddrMask.string

            return generic_all_device_func(per_device__config_pip_mac_da,params)
        end
    else
        -- check that entry in DB 
        index = pip_db_mac_da_name_index_get(name)
        if index == nil then
            command_data:addError("ERROR : the name : " .. name .. " was not found")
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
            didError = true
        end

        if (didError == false) then
            -- let internal function be aware to 'index' , mac-da{pattern,mask}
            params.pip_index = index
            params.macAddr = pip_mac_da_default
            params.macAddrMask = pip_mac_da_default

            -- the 'device' parameter not comes from the command ... we need to retrieve it from the DB
            local numDevs = pip_db_mac_da_name_device_num_get(name)
            local devNum
            for dev_idx = 1,numDevs do
                devNum = pip_db_mac_da_name_device_id_get(name,dev_idx)
                per_device__config_pip_mac_da(command_data,devNum,params)
            end
            -- remove the name from the DB
            pip_db_mac_da_name_reset(name)
            -- remove the index from db
            pip_db_mac_da_index_reset(index)
        end

    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- implement 'per device' : 'config' --> 'pip mac-da'
local function per_device__config_expect_pip_mac_da(command_data,devNum,params)

    --print("params ", to_string(params))
    
    local unset = (params.flagNo == true)
    local macAddr       = params.macAddr
    local macAddrMask   = params.macAddrMask
    local index = params.pip_index
    
    if (unset == false) then
        local name = params.name
        -- check that the device is in the DB 
        local devIndex = pip_db_mac_da_name_device_index_get(name,devNum)
        if devIndex == nil then
            command_data:addError("ERROR : ".. name .." not hold devNum = " .. devNum .. "in DB")
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
        end
    end
    
    local entry = {macAddrValue = macAddr, macAddrMask = macAddrMask}
    
    local apiName = "cpssDxChPortPipGlobalMacDaClassificationEntryGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U32",    "index",       index },
        { "OUT",     "CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC",    "entryPtr",   entry}-->we compare the value !
    })
end


-- implement : 'config' --> pip mac-da command
local function config_expect_pip_mac_da(params)
    -- init pip DB manager
    pip_db_manager_init()

    --print(to_string(params))
    
    local command_data = Command_Data()
    local unset = (params.flagNo == true)
    local name = params.name
    local index
    local didError = false

    if(unset == false) then
        local macAddr       = params["mac-address"]
        local macAddrMask   = params["mac_mask"]
    
        local isAlreadyExists
        
        isAlreadyExists = pip_db_mac_da_index_check(macAddr.string,macAddrMask.string)
        if isAlreadyExists == false then
            command_data:addError("ERROR : not found in DB : the mac ".. macAddr.string .. " mask : " .. macAddrMask.string)
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
            didError = true
        else
            index , isAlreadyExists = pip_db_mac_da_index_get(macAddr.string,macAddrMask.string)
        end
           

        if (didError == false) then
            -- check that the 'name' is not bound to other mac-da {pattern,mask}
            local check_index = pip_db_mac_da_name_index_get(name)
            if check_index == nil then
                command_data:addError("ERROR : the name : " .. name .. " was not found")
                command_data:setFailStatus()
                command_data:setFailLocalStatus()
                didError = true
            elseif check_index ~= index then
                local previous_mac_pattern , previous_mac_mask = pip_db_mac_da_entry_get(check_index)
                command_data:addError("ERROR : the name : ".. name .. " already associated with mac-da:" .. previous_mac_pattern .. " mask : " .. previous_mac_mask )
                command_data:setFailStatus()
                command_data:setFailLocalStatus()
                didError = true
            end
        end
            
        if (didError == false) then
            -- let internal function be aware to 'index' , mac-da{pattern,mask}
            params.pip_index = index
            params.macAddr = macAddr.string
            params.macAddrMask = macAddrMask.string

            return generic_all_device_func(per_device__config_expect_pip_mac_da,params)
        end
    else
        -- check that entry in DB 
        index = pip_db_mac_da_name_index_get(name)
        if index ~= nil then
            command_data:addError("ERROR : the name : " .. name .. " is still in DB")
            command_data:setFailStatus()
            command_data:setFailLocalStatus()
            didError = true
        end

        if (didError == false) then
            -- let internal function be aware to 'index' , mac-da{pattern,mask}
            params.pip_index = index
            params.macAddr = pip_mac_da_default
            params.macAddrMask = pip_mac_da_default

            -- the 'device' parameter not comes from the command ... we need to retrieve it from the DB
            local numDevs = pip_db_mac_da_name_device_num_get(name)
            if numDevs ~= 0 then
                command_data:addError("ERROR : " .. name .. " still hold " .. numDevs .." devices in DB ")
                command_data:setFailStatus()
                command_data:setFailLocalStatus()
                didError = true
            end
        end

    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


local help_string_pip_mac_da = "'name' a mac-da {pattern,mask} for classification as 'mac-da'"
local help_string_pip_no_mac_da = "un-" .. help_string_pip_mac_da
local help_string_expect_pip_mac_da = help_build_expect_string(help_string_pip_mac_da)
local help_string_expect_no_pip_mac_da = help_build_expect_string(help_string_pip_no_mac_da)


CLI_addHelp("config", "pip mac-da", help_string_pip_mac_da)
CLI_addHelp("config", "no pip mac-da", help_string_pip_no_mac_da)
CLI_addHelp("config", "expect pip mac-da", help_string_expect_pip_mac_da)
CLI_addHelp("config", "expect no pip mac-da", help_string_expect_no_pip_mac_da)
   
--[[
Command to 'name' a mac-da {pattern,mask} for classification as 'mac-da'.

Context – config

Format: 
pip mac-da <name> device <all / <devNum>> mac <mac_addr_value> mask < mac_addr_mask > 

Parameters:
name		- a unique name for one of the 4 mac-da {pattern,mask}.
device 		– 'all' or specific device number
mac		- the 6 bytes of mac address value
mask		- the 6 bytes of mac address mask 

]]
CLI_addCommand("config", "pip mac-da",
    { func = config_pip_mac_da ,
      help = help_string_pip_mac_da,
      params =
      {
            { type="values",
               { format = "%string", name = "name", help = help_mac_da_name }
            },
            { type="named",
                "#all_device",-- specific device or on all devices
                "#mac-address",-- parameter name is 'mac-address'
                { format="mask %macMask", name="mac_mask", help= "The mac-da mask" },  
               
               mandatory = {"all_device" , "mac-address" , "mac_mask" }
            }
      }
    });

CLI_addCommand("config", "no pip mac-da",
    { func = function(params)
            params.flagNo = true
            return config_pip_mac_da(params) 
        end,
      help = help_string_pip_no_mac_da,
      params =
      {
            { type="values",
               { format = "%string", name = "name", help = help_mac_da_name }
            }
      }
    });

    
CLI_addCommand("config", "expect pip mac-da",
    { func = config_expect_pip_mac_da ,
      help = help_string_expect_pip_mac_da,
      params =
      {
            { type="values",
               { format = "%string", name = "name", help = help_mac_da_name }
            },
            { type="named",
                "#all_device",-- specific device or on all devices
                "#mac-address",-- parameter name is 'mac-address'
                { format="mask %macMask", name="mac_mask", help= "The mac-da mask" },  
               
               mandatory = {"all_device" , "mac-address" , "mac_mask" }
            }
      }
    });
    
CLI_addCommand("config", "expect no pip mac-da",
    { func = function(params)
            params.flagNo = true
            return config_expect_pip_mac_da(params) 
        end,
      help = help_string_expect_no_pip_mac_da,
      params =
      {
            { type="values",
               { format = "%string", name = "name", help = help_mac_da_name }
            }
      }
    });
    
    
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

--[[
{
  all_device="all",
  pip_type="CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV4_TOS_E",
  pip_profile=2,
  pip_field="565",
  priority=2
}
]]--

-- implement 'per device': 'config' --> pip profile command
local function per_device__config_pip_profile(command_data,devNum,params)
    local index_list = params.pip_field -- list of fields or {"all"} or 'ude name' or 'mac-da name'
    local pip_profile = params.pip_profile
    local pip_type = params.pip_type -- specific type or "all"
    local priority = params.priority
    local pip_type_list
    local ipv6_erratum = pip_is_device_ipv6_erratum(devNum)
    local ipv6_erratum_impact = false
    
    --print("pip_type",to_string(pip_type))

    if pip_type == "all" then
        pip_type_list = { DSA_QOS_PROFILE , DSA_UP , VLAN_TAG_UP , MPLS_EXP , 
                          IPV4_TOS , IPV6_TC , UDE_INDEX , MAC_DA_INDEX }
    else
        pip_type_list = {pip_type}
    end
    
    
    local apiName = "cpssDxChPortPipGlobalProfilePrioritySet"
    
    for dummy_pip_type , pip_type in pairs(pip_type_list) do 
        --print("dummy_pip_type,pip_type",to_string(dummy_pip_type),to_string(pip_type))
    
        if type(params.pip_field) == "string" then
            local name = params.pip_field
            local index
            
            if name ~= "all" then
                -- check if mac-da name or ude name
                if pip_type == UDE_INDEX then
                    index = pip_db_ude_name_index_get(name)
                elseif pip_type == MAC_DA_INDEX then
                    index = pip_db_mac_da_name_index_get(name)
                end

                if index == nil then
                    command_data:addError("ERROR : " .. name .. " not exists in DB")
                    command_data:setFailStatus()
                    command_data:setFailLocalStatus()
                    return false
                end
                
                --print ("index = " , to_string(index))

                index_list = {}
                table.insert(index_list,index)
            else
                index_list = {0,1,2,3} -- all 4 indexes
            end
            
        elseif type(params.pip_field) == "table" and 
               params.pip_field[1] == "all" then
            -- build index_list that proper for current 'pip_type'
            index_list = {}
            local max_num = pip_type_max_value[pip_type]
            --print("max_num",to_string(max_num))
            
            for ii = 0, max_num do
                table.insert(index_list,ii)
            end
        else
            index_list = params.pip_field
        end
    
    
        for dummy_index , fieldIndex in pairs(index_list) do
            
            local numeric_fieldIndex = tonumber(fieldIndex)
            
            if ipv6_erratum and pip_type == IPV6_TC then
                -- only 6 bits are used
                numeric_fieldIndex = numeric_fieldIndex % (pip_type_max_value_ipv6_erratum + 1)
                ipv6_erratum_impact = true
            end
        
            for dummy_ii, profile in pairs(pip_profile) do
                --print("profile , pip_type , fieldIndex , priority",profile , pip_type , numeric_fieldIndex , priority)
            
                genericCpssApiWithErrorHandler(command_data,
                    apiName, {
                    { "IN",     "GT_U8",      "devNum",     devNum },
                    { "IN",     "GT_U32",      "pipProfile",     profile },
                    { "IN",     "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT",      "type",     pip_type },
                    { "IN",     "GT_U32",      "fieldIndex",     numeric_fieldIndex },
                    { "IN",     "GT_U32",      "priority",     priority }
                })
            end
        end
    end
    
    if ipv6_erratum_impact == true then
        print("IMPORTANT NOTE : for ipv6 :" .. pip_ipv6_erratum_string .. 
                " So we get the same priority for TC 0,64,128,192 (and TC 1,65,129,193 ...)")
    end
    
end
-- implement : 'config' --> pip profile command
local function config_pip_profile(params)
    -- init pip DB manager
    pip_db_manager_init()

    --print(to_string(params))
    
    return generic_all_device_func(per_device__config_pip_profile,params)
end

-- implement 'per device': 'config' --> pip profile command
local function per_device__config_expect_pip_profile(command_data,devNum,params)
    local index_list = params.pip_field -- list of fields or {"all"} or 'ude name' or 'mac-da name'
    local pip_profile = params.pip_profile
    local pip_type = params.pip_type -- specific type or "all"
    local priority = params.priority
    local pip_type_list
    local ipv6_erratum = pip_is_device_ipv6_erratum(devNum)

    if pip_type == "all" then
        pip_type_list = { DSA_QOS_PROFILE , DSA_UP , VLAN_TAG_UP , MPLS_EXP , 
                          IPV4_TOS , IPV6_TC , UDE_INDEX , MAC_DA_INDEX }
    else
        pip_type_list = {pip_type}
    end
    
    --print("params.pip_field",to_string(params.pip_field))
    
    local apiName = "cpssDxChPortPipGlobalProfilePriorityGet"
    
    --print ("type(pip_type_list)" , to_string(type(pip_type_list)))

    for dummy_pip_type , pip_type in pairs(pip_type_list) do 
        --print("dummy_pip_type,pip_type",to_string(dummy_pip_type),to_string(pip_type))
    
        if type(params.pip_field) == "string" then
            local name = params.pip_field
            local index
            -- check if mac-da name or ude name
            if name ~= "all" then
                if pip_type == UDE_INDEX then
                    index = pip_db_ude_name_index_get(name)
                elseif pip_type == MAC_DA_INDEX then
                    index = pip_db_mac_da_name_index_get(name)
                end

                if index == nil then
                    command_data:addError("ERROR : " .. name .. " not exists in DB")
                    command_data:setFailStatus()
                    command_data:setFailLocalStatus()
                    return false
                end
                
                --print ("index = " , to_string(index))

                index_list = {index}
            else
                index_list = {0,1,2,3} -- all 4 indexes
            end
            
        elseif type(params.pip_field) == "table" and 
               params.pip_field[1] == "all" then
            -- build index_list that proper for current 'pip_type'
            index_list = {}
            local max_num = pip_type_max_value[pip_type]
            --print("max_num",to_string(max_num))
            
            for ii = 0, max_num do
                table.insert(index_list,ii)
            end
        else
            index_list = params.pip_field
        end
    
    
        for dummy_index , fieldIndex in pairs(index_list) do
            
            local numeric_fieldIndex = tonumber(fieldIndex)
            if ipv6_erratum and pip_type == IPV6_TC then
                -- only 6 bits are used
                numeric_fieldIndex = numeric_fieldIndex % (pip_type_max_value_ipv6_erratum + 1)
            end
        
            for dummy_ii, profile in pairs(pip_profile) do
                --print("profile , pip_type , fieldIndex , priority",profile , pip_type , numeric_fieldIndex , priority)
            
                local isError , result, values = 
                genericCpssApiWithErrorHandler(command_data,
                    apiName, {
                    { "IN",     "GT_U8",      "devNum",     devNum },
                    { "IN",     "GT_U32",      "pipProfile",     profile },
                    { "IN",     "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT",      "type",     pip_type },
                    { "IN",     "GT_U32",      "fieldIndex",     numeric_fieldIndex },
                    { "OUT",     "GT_U32",      "priority",     priority}-->we compare the value !
                })
            end
        end
    end
    
end

local function config_expect_pip_profile(params)
    -- init pip DB manager
    pip_db_manager_init()

    --print(to_string(params))
    
    return generic_all_device_func(per_device__config_expect_pip_profile,params)
end


local help_string_pip_profile = "set pip-priority 'per profile' 'per packet type' 'per field'"
local help_string_expect_pip_profile = help_build_expect_string(help_string_pip_profile)
CLI_addHelp("config", "pip profile", help_string_pip_profile)
CLI_addHelp("config", "expect pip profile", help_string_expect_pip_profile)
--[[
Command to set pip-priority 'per profile' 'per packet type' 'per field'.

Context – config

Format: 
pip profile <index> device <all / <devNum>> type < type_name > field <field_value> priority <priority_value>

Parameters:
index		- one of 4 pip-profiles (0..3) 
device 		– 'all' or specific device number
type		- one of next types : dsa-qos , dsa-up , vlan-tag-up, mpls-exp , ipv4-tos , ipv6-tc  , user-defined-ethertype , mac-da
field		- (support range : all or 5 or 1-3,6 or 1,2-4,6)
the field value : 
			for dsa-qos , value = 0..127
			for dsa-up , value = 0..7
			for vlan-tag-up, value = 0.. 7
			for mpls-exp, value = 0..7
			for ipv4-tos, value = 0..255
			for ipv6-tc, value = 0..255
			for user-defined-ethertype : one of its 'names'
			for mac-da : one of its 'names'

priority 	- the pip-priority (0..2 , 0-high , 1-medium , 2-low)
]]--    
local function addFunctionPipProfilePriority()
    if is_sip_5_20(devEnv.dev) then
        pip_help_pip_priority = "pip-priority options : very-high/high/medium/low "
        local pip_pririty_enum = {
            ["very-high"]   = { value=0, help="Very high pip-priority" },
            ["high"]       = { value=1, help="High pip-priority" },
            ["medium"]     = { value=2, help="Medium pip-priority" },
            ["low"]        = { value=3, help="Low pip-priority" }
        }
        CLI_addParamDictAndType_enum("pip_priority_enum","priority", pip_help_pip_priority, pip_pririty_enum)
    else
        pip_help_pip_priority = "pip-priority options : high/medium/low "
        local pip_pririty_enum = {
            ["high"]       = { value=0, help="High pip-priority" },
            ["medium"]     = { value=1, help="Medium pip-priority" },
            ["low"]        = { value=2, help="Low pip-priority" }
        }
        CLI_addParamDictAndType_enum("pip_priority_enum","priority", pip_help_pip_priority, pip_pririty_enum)
    end

    CLI_addCommand("config", "pip profile",
        { func = config_pip_profile ,
          help = help_string_pip_profile,
          params =
          {
              { type="values",
                 { format = "%pip_profile", name = "pip_profile", help = pip_help_pip_profile }
              },
              { type="named",
                  "#all_device",-- specific device or on all devices
                { format = "type %pip_type_enum", name = "pip_type", help = pip_help_pip_type },
                { format = "field %pip_field", name = "pip_field", help = help_pip_field },
                "#pip_priority_enum",-- name is "priority"

                requirements = {
                    pip_field = {"pip_type"},
                    priority = {"pip_field"}
                },

                mandatory = {"all_device" , "pip_type" , "pip_field" , "priority" }
              }
          }
        });

    CLI_addCommand("config", "expect pip profile",
        { func = config_expect_pip_profile ,
          help = help_string_expect_pip_profile,
          params =
          {
              { type="values",
                 { format = "%pip_profile", name = "pip_profile", help = pip_help_pip_profile }
              },
              { type="named",
                  "#all_device",-- specific device or on all devices
                 { format = "type %pip_type_enum", name = "pip_type", help = pip_help_pip_type },
                 { format = "field %pip_field", name = "pip_field", help = help_pip_field },
                 "#pip_priority_enum",-- name is "priority"

                 requirements = {
                     pip_field = {"pip_type"},
                     priority = {"pip_field"}
                 },

                 mandatory = {"all_device" , "pip_type" , "pip_field" , "priority" }
              }
          }
        });
end

table.insert(typeCallBacksList, addFunctionPipProfilePriority)

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################
    
-- implement 'per device' : 'config' --> pip threshold command
local function per_device__config_pip_threshold(command_data,devNum,params)
    local priority = params.priority
    local threshold = params.threshold_value
    local apiName = "cpssDxChPortPipGlobalBurstFifoThresholdsSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_DATA_PATH_BMP", "dataPathBmp", 0xFFFFFFFF},
        { "IN",     "GT_U32",    "priority",    priority },
        { "IN",     "GT_U32",    "threshold",   threshold }
    })

end
    
-- implement : 'config' --> pip threshold command
local function config_pip_threshold(params)
    --print(to_string(params))
    
    -- init pip DB manager
    pip_db_manager_init()

    return generic_all_device_func(per_device__config_pip_threshold,params)
end

-- implement 'per device' : 'config' --> pip threshold command
local function per_device__config_expect_pip_threshold(command_data,devNum,params)
    local priority = params.priority
    local threshold = params.threshold_value
    local apiName = "cpssDxChPortPipGlobalBurstFifoThresholdsGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_DATA_PATH_BMP", "dataPathBmp", 0xFFFFFFFF},
        { "IN",     "GT_U32",    "priority",    priority },
        { "OUT",     "GT_U32",    "threshold",   threshold}-->we compare the value !
    })

end

-- implement : 'config' --> pip threshold command
local function config_expect_pip_threshold(params)
    --print(to_string(params))
    
    -- init pip DB manager
    pip_db_manager_init()

    return generic_all_device_func(per_device__config_expect_pip_threshold,params)
end

    
local help_string_pip_threshold = "set number of packets as threshold for specific pip-priority"
local help_string_expect_pip_threshold = help_build_expect_string(help_string_pip_threshold)

CLI_addHelp("config", "pip threshold", help_string_pip_threshold)
CLI_addHelp("config", "expect pip threshold", help_string_expect_pip_threshold)
    
--[[
Command to set number of packets as threshold for specific pip-priority.

Context – config

Format: 
pip threshold < threshold_value > device <all / <devNum>> priority <priority_value>

Parameters:
threshold_value – the threshold value. (number of packets) (0..1023)
                                                           (0..65k) SIP 5.20
device 		– 'all' or specific device number
priority 	- the pip-priority (0..2 , 0-high , 1-medium , 2-low)
priority 	- the pip-priority (0..3 , 0-very-high 1-high , 2-medium , 3-low) (SIP 5.20)
]]--    

local function addFunctionSetPipThreshold()
    if is_sip_5_20(devEnv.dev) then
        pip_help_pip_threshold_value = "number of packets as threshold (0..(65k-1)) for specific pip priority"
        CLI_type_dict["pip_threshold_value"] = {
            checker = CLI_check_param_number,
            min=0,
            max=65535,
            complete = CLI_complete_param_number,
            help=pip_help_pip_threshold_value
        }

    else
        pip_help_pip_threshold_value = "number of packets as threshold (0..(1k-1)) for specific pip priority"
        CLI_type_dict["pip_threshold_value"] = {
            checker = CLI_check_param_number,
            min=0,
            max=1023,
            complete = CLI_complete_param_number,
            help=pip_help_pip_threshold_value
        }
    end

    CLI_addCommand("config", "pip threshold",
        { func = config_pip_threshold ,
          help = help_string_pip_threshold,
          params =
          {
                { type="values",
                   { format = "%pip_threshold_value", name = "threshold_value", help = pip_help_pip_threshold_value }
                },
                { type="named",
                    "#all_device",-- specific device or on all devices
                    "#pip_priority_enum",-- name is "priority"

                    mandatory = {"all_device" , "priority" }
                }
          }
        });

    CLI_addCommand("config", "expect pip threshold",
        { func = config_expect_pip_threshold ,
          help = help_string_expect_pip_threshold,
          params =
          {
                { type="values",
                   { format = "%pip_threshold_value", name = "threshold_value", help = pip_help_pip_threshold_value }
                },
                { type="named",
                    "#all_device",-- specific device or on all devices
                    "#pip_priority_enum",-- name is "priority"

                    mandatory = {"all_device" , "priority" }
                }
          }
        });
end

table.insert(typeCallBacksList, addFunctionSetPipThreshold)

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################
