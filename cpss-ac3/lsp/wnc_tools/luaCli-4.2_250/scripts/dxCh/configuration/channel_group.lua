--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_channel.lua
--*
--* DESCRIPTION:
--*       adding/removing of an interface to/from a channel group 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 9 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlDxChTrunkMemberAddingConditionChecking")

--constants
local default_num_members_per_trunk = 8
local sip4_fixed_num_members_per_trunk = 8

-- ************************************************************************
---
--  channel_group_func
--        @description  add's an interface to a channel group 
--
--        @param params         - params["trunkID"]: trunk
--
--        @return       true on success, otherwise false and error message
--
local function channel_group_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local trunkId, hwDevNum, hwPortNum
    local trunk_member_adding_condition, trunk_member_in_trunk_presence
    local trunk_member
    local trunk_member_adding_warning_string

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    
    -- Command specific variables initialization.
    local interface_port_channel  = getGlobal("ifPortChannel")
        trunkId = params["trunkID"]
    
    -- Interface mode checking.
    if true == command_data["status"] then    
        command_data:setFailStatusOnCondition(isNotNil(interface_port_channel))
        command_data:addConditionalError(
            isNotNil(interface_port_channel), 
            "This command is not supported on port-channel interface")
    end    
    
    -- Trunk id initialization checking.
    if true == command_data["status"] then    
        command_data:setFailStatusOnCondition(isNil(params["trunkID"]))
        command_data:addConditionalError(
            isNil(params["trunkID"]), "Port-channel index is not defined")    
    end
   
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
             
            command_data:clearLocalStatus()
            
            -- Converting of trunk member to hardware format.
            if true == command_data["local_status"]         then

                result, values, hwPortNum = 
                    device_port_to_hardware_format_convert(devNum, portNum)
                if        0 == result                   then
                    hwDevNum =  values
                elseif 0x10 == result                   then
                    command_data:setFailPortAndLocalStatus() 
                    command_data:addError("Device id %d and port number %d " ..
                                          "converting is not allowed.", devNum, 
                                          portNum)
                elseif    0 ~= result                   then
                    command_data:setFailPortAndLocalStatus()                 
                    command_data:addError("Error at device id %d and port " ..
                                          "number %d converting: %s.", devNum, 
                                          portNum, values)                    
                end
            end
            
            -- Trunk member forming.
            if true == command_data["local_status"]         then           
                trunk_member = { device = hwDevNum, port = hwPortNum }
            end

            -- Trunk member adding condition checking.
            if true == command_data["local_status"]         then            
                result, values, trunk_member_in_trunk_presence, 
                trunk_member_adding_warning_string = 
                    wrLogWrapper("wrlDxChTrunkMemberAddingConditionChecking", 
                                 "(devNum, trunkId, hwDevNum, hwPortNum)", 
                                   devNum, trunkId, hwDevNum, hwPortNum)
                if        0 == result                   then
                    trunk_member_adding_condition   = values
                elseif 0x10 == result                   then
                    command_data:setFailPortAndLocalStatus() 
                    command_data:addWarning("It is not allowed to check " ..
                                            "member adding condition to " ..
                                            "trunk %d on device %d port %d.", 
                                            trunkId, devNum, portNum)                  
                elseif    0 ~= result                   then
                    command_data:setFailPortAndLocalStatus()    
                    command_data:addError("Error at checking of member " ..
                                          "adding condition to trunk %d " ..
                                          "on device %d port %d: %s.", trunkId,
                                          devNum, portNum, returnCodes[result])                        
                end
            end

            -- Trunk member adding.
            if (true == command_data["local_status"])       and
               (true == trunk_member_adding_condition)      then
                result, values = 
                    cpssPerTrunkParamSet("cpssDxChTrunkMemberAdd", devNum, 
                                         trunkId, trunk_member, "member", 
                                         "CPSS_TRUNK_MEMBER_STC")   
                if     0x10 == result                   then
                    command_data:setFailPortStatus()                   
                    command_data:addWarning("It is not allowed to add new " ..
                                            "port %d to trunk %d on " ..
                                            "device %d. ", portNum, devNum, 
                                            trunkId)             
                elseif    0 ~= result                   then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at adding of new port %d " ..
                                          "to trunk %d on device %d: %s.", 
                                          portNum, devNum, trunkId, 
                                          returnCodes[result])
                end
            end

            -- Trunk member adding condition checkong.
            if (true == command_data["local_status"])       and   
               (false == trunk_member_adding_condition)     and
               (false == trunk_member_in_trunk_presence)    then
                command_data:setFailPortStatus()              
                command_data:addWarning("It is not allowed to add new " ..
                                        "port %d to trunk %d on device %d: %s.", 
                                        portNum, trunkId, devNum, 
                                        trunk_member_adding_warning_string)
            elseif (true == command_data["local_status"]) then
                -- state to the DB that this trunkId is used
                general_resource_manager_use_specific_index(general_resource_manager_trunk_table,trunkId)
            end 
            
            command_data:updateStatus() 
      
            command_data:updatePorts()            
        end     

        command_data:addWarningIfNoSuccessPorts("Can not add to trunk all " ..
                                                "processed ports.")
    end
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
--  no_channel_group_func
--
--  DESCRIPTION:
--        removes an interface from a channel group
--
--  INPUTS:
--        params["trunkID"]                 - trunk 
--
--  RETURNS:
--        true on success, otherwise false and error message
--
-- ************************************************************************
local function no_channel_group_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local trunkId, hwDevNum, hwPortNum
    local trunk_member_deleting_condition, trunk_member

    -- Common variables initialization
    command_data:initInterfaceRangeIterator()
    
    -- Command specific variables initialization.
    if nil ~= params["trunkID"] then
        trunk_member_deleting_condition = true    
        trunkId = params["trunkID"]
    else
        command_data:setFailStatus()
        command_data:addError("Port-channel index is not defined")    
    end
        
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()
            
            -- Converting of trunk member to hardware format.
            if (true == command_data["local_status"])   and
               (true == trunk_member_deleting_condition)then
                result, values, hwPortNum =
                    device_port_to_hardware_format_convert(devNum, portNum)
                if        0 == result                   then
                    hwDevNum =  values
                elseif 0x10 == result                   then
                    command_data:setFailPortAndLocalStatus() 
                    command_data:addError("Device id %d and port number %d " ..
                                          "converting is not allowed.", devNum, 
                                          portNum)
                elseif    0 ~= result                   then
                    command_data:setFailPortAndLocalStatus()                 
                    command_data:addError("Error at device id %d and port " ..
                                          "number %d converting: %s.", devNum, 
                                          portNum, values)                    
                end
            end
            
            -- Trunk member forming.
            if (true == command_data["local_status"])   and
               (true == trunk_member_deleting_condition)then           
                trunk_member = { device = hwDevNum, port = hwPortNum }
            end
            
            -- Trunk member adding.
            if (true == command_data["local_status"])   and
               (true == trunk_member_deleting_condition)then 
                result, values = cpssPerTrunkParamSet("cpssDxChTrunkMemberRemove",
                                                      devNum, trunkId, 
                                                      trunk_member, "member", 
                                                      "CPSS_TRUNK_MEMBER_STC")   
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to delete port %d " ..
                                            "in device %d trunk %d.", 
                                            portNum, devNum, trunkId)                                                   
                elseif LGT_NOT_INITIALIZED == result    then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("Trunk %d does not exists on " ..
                                            "device %d port %d.", trunkId, 
                                            portNum, devNum)  
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at deleting of port %d " ..
                                          "in device %d trunk %d: %s.", 
                                          portNum, devNum, trunkId, 
                                          returnCodes[result])
                                          
                else
                    --***************
                    --***************
                    local apiName = "cpssDxChTrunkDbTrunkTypeGet"
                    local is_faild , rc , OUT_params = 
                        genericCpssApiWithErrorHandler(command_data,
                            apiName ,{
                            { "IN",     "GT_U8",                    "devNum",               devNum },
                            { "IN",     "GT_TRUNK_ID",              "trunkId",              trunkId },
                            { "OUT",     "CPSS_TRUNK_TYPE_ENT",                   "type"}
                        })
                    if is_faild == false and OUT_params.type == "CPSS_TRUNK_TYPE_FREE_E" then
                        -- free the trunk in the DB
                        general_resource_manager_free_specific_index(general_resource_manager_trunk_table,trunkId)
                    end
                end 
            end    
        
            command_data:updateStatus() 
      
            command_data:updatePorts()            
        end     

        command_data:addWarningIfNoSuccessPorts("Can not delete from trunk all processed ports.")
    end
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()
end


-------------------------------------------------------
-- command registration: channel-group
-------------------------------------------------------
CLI_addCommand("interface", "channel-group", {
  func=channel_group_func,
  help="Configure channel-group",
  params={
    { type = "values",
      "%trunkID"
    }
  }
})

-------------------------------------------------------
-- command registration: no channel-group
-------------------------------------------------------
CLI_addCommand("interface", "no channel-group", {
  func=no_channel_group_func,
  help="Configure channel-group",
  params={
    { type = "values",
      "%trunkID"
    }
  }
})
-- ************************************************************************
--  per_dev__fixed_num_members_func
--
--  DESCRIPTION:
--        per devoice : 
--          re-init the trunk LIB to new number of trunks and members
--
--  INPUTS:
--
--  RETURNS:
--
-- ************************************************************************
local function per_dev__fixed_num_members_func(command_data,devNum,params)
    local maxNumberOfTrunks = params.numTrunks

    if not is_sip_5(devNum) and params.maxMembers ~= sip4_fixed_num_members_per_trunk then
        command_data:handleCpssErrorDevice(0x1e --[[GT_NOT_APPLICABLE_DEVICE ]], 
            "changing max number of trunk members" , devNum) 
        return false
    end

    local apiName = "cpssDxChTrunkDbInitInfoGet"
    local is_faild , rc , outValues = 
        genericCpssApiWithErrorHandler(command_data,
            apiName ,{
            { "IN", "GT_U8",    "devNum",               devNum },
            { "OUT", "GT_U32",   "maxNumberOfTrunks"},
            { "OUT", "CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT", "trunkMembersMode"}
        })
    if is_faild or not outValues then
        return false
    end
    
    local orig_maxNumberOfTrunks = outValues.maxNumberOfTrunks
    local orig_trunkMembersMode  = outValues.trunkMembersMode
    
    local function restoreOrig(command_data,devNum)
        local apiName = "cpssDxChTrunkInit"
        local is_faild , rc = 
            genericCpssApiWithErrorHandler(command_data,
                apiName ,{
                { "IN", "GT_U8",    "devNum",               devNum },
                { "IN", "GT_U32",   "maxNumberOfTrunks",    orig_maxNumberOfTrunks },
                { "IN", "CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT", "trunkMembersMode",        orig_trunkMembersMode}
            })
    end

    local trunkMembersMode = "CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E"

    if not is_sip_5(devNum) then
        -- only Need to re-init the number of trunks
        trunkMembersMode = "CPSS_DXCH_TRUNK_MEMBERS_MODE_NATIVE_E"
    else
        trunkMembersMode = "CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E"
    end
    
    local apiName = "cpssDxChTrunkDestroy"
    
    local is_faild , rc = 
        genericCpssApiWithErrorHandler(command_data,
            apiName ,{
            { "IN", "GT_U8",    "devNum",               devNum }
        })
    if is_faild then
        restoreOrig(command_data,devNum)
        return false
    end

    local apiName = "cpssDxChTrunkInit"
    
    local is_faild , rc = 
        genericCpssApiWithErrorHandler(command_data,
            apiName ,{
            { "IN", "GT_U8",    "devNum",               devNum },
            { "IN", "GT_U32",   "maxNumberOfTrunks",    maxNumberOfTrunks },
            { "IN", "CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT", "trunkMembersMode",        trunkMembersMode}
        })
    if is_faild then
        restoreOrig(command_data,devNum)
        return false
    end
    
    
    if is_sip_5(devNum) then
        -- need to set the new max number of trunks to all trunks
        local apiName = "cpssDxChTrunkFlexInfoSet"
        local maxNumOfMembers = params.maxMembers
        local ALL_TRUNKS_SAME_VALUE = 0xFFFF
        local is_faild , rc = 
            genericCpssApiWithErrorHandler(command_data,
                apiName ,{
                { "IN", "GT_U8",    "devNum",          devNum },
                { "IN", "GT_TRUNK_ID",   "trunkId",    ALL_TRUNKS_SAME_VALUE },
                { "IN", "GT_U32", "l2EcmpStartIndex",  0},
                { "IN", "GT_U32", "maxNumOfMembers",   maxNumOfMembers}
            })
        if is_faild then
            restoreOrig(command_data,devNum)
            return false
        end
    end

    -- need to reload the MAXIMUM_TRUNK_ID
    reloadTrunkIdDictionary()
    
    
    return true -- passed
end
-- ************************************************************************
--  fixed_num_members_func
--
--  DESCRIPTION:
--        re-init the trunk LIB to new number of trunks and members
--
--  INPUTS:
--        
--
--  RETURNS:
--        
--
-- ************************************************************************
local function fixed_num_members_func(params)
    if not params.maxMembers then 
        params.maxMembers = default_num_members_per_trunk 
    end

    if params.numTrunks == 0 then 
        params.numTrunks = 1 
    end

    -- function to support iteration on all devices that we got from parameter "#all_device"
    generic_all_device_func(per_dev__fixed_num_members_func,params)     
  
end

local help_fixed_num_members = "Re-init the trunk LIB to new number of trunks and members."..
                                "\t\t Set the max number of trunks  ,each with max number of members"
-------------------------------------------------------
-- command registration: port-channel fixed-num-members
-------------------------------------------------------
CLI_addCommand("config", "port-channel fixed-num-members", {
  func=fixed_num_members_func,
  help=help_fixed_num_members,
  params={
    {  type="named",
      "#all_device",
      { format="num-of-trunks %number", name="numTrunks" ,help="max number of trunks"},
      { format="max-members %number", name="maxMembers" ,help="max number of members (default is 8)"},
        mandatory={"all_device","numTrunks",}
    }
  }
})


-------------------------------------------------------
-- NOTE: for 'show' see command : (exec) show interfaces port-channel
-------------------------------------------------------

