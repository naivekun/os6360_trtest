--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_priority_flow_control_enable.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of PFC for an interface per a list of queue
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants 


-- ************************************************************************
---
--  dce_priority_flow_control_enable_func
--        @description  enables/disables of PFC for an interface per a list 
--                      of queue
--
--        @param params         - params["priority"]: 802.1Q Priority;
--                                params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function dce_priority_flow_control_enable_func(params)
    -- Common variables declaration.
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local pfc_enabling, pfc, mode_fc_hol
    local profileIndex, priority, pfcProfileCfg
    
    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()    
    command_data:initInterfaceDeviceRange()    
 
    -- Command specific variables initialization.
    pfc_enabling    = command_data:getTrueIfFlagNoIsNil(params)
    pfc             = command_data:getValueIfFlagNoIsNil(
                          params, 
                          "CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E", 
                          "CPSS_DXCH_PORT_PFC_DISABLE_ALL_E")
    mode_fc_hol     = command_data:getValueIfFlagNoIsNil(
                          params, 
                          "CPSS_DXCH_PORT_HOL_E", 
                          "CPSS_DXCH_PORT_FC_E")
    profileIndex    = 1
    priority        = params["priority"]
    pfcProfileCfg   = command_data:getValueIfFlagNoIsNil(
                          params, 
                          { xonThreshold    = 20,   xoffThreshold = 200  }, 
                          { xonThreshold    = 2000, xoffThreshold = 2000 })
                
    -- Main port handling cycle.
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum, portNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()  
                    
            command_data:clearLocalStatus() 
            
            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
            
            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
                -- Priority flow control setting.
                if true == command_data["local_status"]     then   
                    result, values = 
                        cpssPerDeviceParamSet("cpssDxChPortPfcEnableSet",
                                              devNum, pfc, "pfcEnable", 
                                              "CPSS_DXCH_PORT_PFC_ENABLE_ENT") 
                    if     0x10 == result   then
                        command_data:setFailEntryStatus() 
                        command_data:addWarning("It is not allowed to %s " ..
                                                "priority flow control on " ..
                                                "device %d.", 
                                                boolEnableLowerStrGet(pfc_enabling),
                                                devNum) 
                    elseif    0 ~= result   then
                        command_data:setFailEntryAndLocalStatus()
                        command_data:addError("Error at %s of priority flow " ..
                                              "control on device %d: %s.", 
                                              boolEnablingLowerStrGet(pfc_enabling),
                                              devNum, returnCodes[result])
                    end               
                end    
        
                -- HOL system mode setting.
                if true == command_data["local_status"]     then   
                    result, values = 
                        cpssPerDeviceParamSet("cpssDxChPortFcHolSysModeSet",
                                              devNum, mode_fc_hol, "modeFcHol", 
                                              "CPSS_DXCH_PORT_HOL_FC_ENT") 
                    if     0x10 == result   then
                        command_data:setFailEntryStatus() 
                        command_data:addWarning("It is not allowed to set " ..
                                                "HOL system mode on device %d.", 
                                                devNum) 
                    elseif    0 ~= result   then
                        command_data:setFailEntryAndLocalStatus()
                        command_data:addError("Error at setting of HOL system " ..
                                              "mode on device %d: %s.", devNum, 
                                              returnCodes[result])
                    end               
                end       
        
                -- Setting of FC profile configurations for given tc queue.
                if (true == command_data["local_status"])   and
                   (true == pfc_enabling)                   then   
                    result, values = 
                        myGenWrapper("cpssDxChPortPfcProfileQueueConfigSet", {
                                     { "IN", "GT_U8",  "devNum",  devNum       },
                                     { "IN", "GT_U32", "profileIndex", 
                                                                  profileIndex },
                                     { "IN", "GT_U8",  "tcQueue", priority     },
                                     { "IN", "CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC", 
                                                "pfcProfileCfg", pfcProfileCfg }})
                        
                    if     0x10 == result   then
                        command_data:setFailEntryStatus() 
                        command_data:addWarning("It is not allowed to set " ..
                                                "priority flow control %d on " ..
                                                "device %d.", priority, devNum) 
                    elseif    0 ~= result   then
                        command_data:setFailEntryAndLocalStatus()
                        command_data:addError("Error at setting of priority " ..
                                              "flow control %d on device %d: %s.", 
                                              priority, devNum, returnCodes[result])
                    end               
                end       
            else
                command_data:addWarningOnceOnKey(
                    devNum, "Family of device %d does not supported.", devNum)
            end
                
            command_data:updateStatus()
                        
            command_data:updateDevices()                
        end
        
        command_data:addWarningIfNoSuccessDevices(
            "Can not " .. boolEnableLowerStrGet(pfc_enabling) .. 
            " priority flow control for all processed devices.")            
    end       
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()  
end


--------------------------------------------------------------------------------
-- command registration: dce priority-flow-control enable
--------------------------------------------------------------------------------
CLI_addHelp("interface",    "dce priority-flow-control", 
                                            "Priority flow control commands")
CLI_addCommand("interface", "dce priority-flow-control enable", {
  func   = dce_priority_flow_control_enable_func,
  help   = "Enabling of PFC for an interface per a list of queues",
  params = {
      { type = "named",
          "#priority",
        mandatory = { "priority" }          
      } 
  }
})

--------------------------------------------------------------------------------
-- command registration: no dce priority-flow-control enable
--------------------------------------------------------------------------------
CLI_addHelp("interface",    "no dce priority-flow-control", 
                                            "Priority flow control commands")
CLI_addCommand("interface", "no dce priority-flow-control enable", {
  func   = function(params) 
               params.flagNo = true
               dce_priority_flow_control_enable_func(params)
           end,
  help   = "Disabling of PFC for an interface per a list of queues"
})
