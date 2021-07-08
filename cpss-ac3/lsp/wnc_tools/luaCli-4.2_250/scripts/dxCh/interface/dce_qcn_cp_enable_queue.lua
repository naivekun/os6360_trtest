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
--*       enabling/disabling of CP creation for an interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants 


-- ************************************************************************
---
--  dce_qcn_cp_enable_queue_func
--        @description  enables/disables of CP creation for an interface 
--
--        @param params         - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function dce_qcn_cp_enable_queue_func(params)
    -- Common variables declaration.
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local fc_enabling
    
    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()   

    -- Command specific variables initialization.
    fc_enabling = command_data:getTrueIfFlagNoIsNil(params)
    
    -- Main port handling cycle.
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()               
                      
            command_data:clearLocalStatus()              
                
            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
    
            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
                -- Flow control packets generation enabling.
                if true == command_data["local_status"]     then   
                    result, values = 
                        cpssPerPortParamSet("cpssDxChPortCnFcEnableSet",
                                            devNum, portNum, fc_enabling, 
                                            "enable", "GT_BOOL") 
                    if     0x10 == result   then
                        command_data:setFailPortStatus() 
                        command_data:addWarning("It is not allowed to %s " ..
                                                "flow control packets " ..
                                                "generation on device %d " ..
                                                "port %d.", 
                                                boolEnableLowerStrGet(fc_enabling), 
                                                devNum, portNum) 
                    elseif    0 ~= result   then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at %s of flow control " ..
                                              "packets generation on " ..
                                              "device %d port %d: %s.", 
                                              boolEnablingLowerStrGet(fc_enabling), 
                                              devNum, portNum,
                                              returnCodes[result])
                    end               
                end                  
               
                -- Enabling of congestion notification messages termination.
                if true == command_data["local_status"]     then   
                    result, values = 
                        cpssPerPortParamSet("cpssDxChPortCnTerminationEnableSet",
                                            devNum, portNum, fc_enabling, 
                                            "enable", "GT_BOOL") 
                    if     0x10 == result   then
                        command_data:setFailPortStatus() 
                        command_data:addWarning("It is not allowed to %s " ..
                                                "congestion notification " ..
                                                "messages termination on " ..
                                                "device %d port %d.", 
                                                boolEnableLowerStrGet(fc_enabling), 
                                                devNum, portNum) 
                    elseif    0 ~= result   then
                        command_data:setFailEntryAndLocalStatus()
                        command_data:addError("Error at %s of congestion " ..
                                              "notification messages " ..
                                              "termination on device %d " ..
                                              "port %d: %s.", 
                                              boolEnablingLowerStrGet(fc_enabling), 
                                              devNum, portNum,
                                              returnCodes[result])
                    end               
                end                 
            else
                command_data:addWarningOnceOnKey(
                    devNum, "Family of device %d does not supported.", devNum)
            end               
    
            command_data:updateStatus()
                
            command_data:updatePorts()               
        end
        
        command_data:addWarningIfNoSuccessPorts(
            "Can not set speed and/or interface mode for all processed ports.") 
    end       
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()      
end


--------------------------------------------------------------------------------
-- command registration: dce qcn cp enable queue
--------------------------------------------------------------------------------
CLI_addHelp("interface",    "dce",          "DCE subcommands")
CLI_addHelp("interface",    "dce qcn",      "Enabling of dce qcn congestion point")
CLI_addHelp("interface",    "dce qcn cp",   "Enabling dce qcn congestion point")
CLI_addHelp("interface",    "dce qcn cp enable",
                                            "Enabling of CP creation for an interface")
CLI_addCommand("interface", "dce qcn cp enable queue", {
  func   = dce_qcn_cp_enable_queue_func,
  help   = "Enabling of CP creation for an interface"
})

--------------------------------------------------------------------------------
-- command registration: no dce qcn cp enable
--------------------------------------------------------------------------------
CLI_addHelp("interface",    "no dce",       "DCE subcommands")
CLI_addHelp("interface",    "no dce qcn",   "Disabling of dce qcn congestion point")
CLI_addHelp("interface",    "no dce qcn cp",
                                            "Disabling dce qcn congestion point")
CLI_addCommand("interface", "no dce qcn cp enable", {
  func   = function(params) 
               params.flagNo = true,
               dce_qcn_cp_enable_queue_func(params)
           end,
  help   = "Disabling of CP creation for an interface"
})
