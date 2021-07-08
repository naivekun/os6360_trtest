--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_priority_flow_control_bind.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of PFC global drop
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  dce_priority_flow_control_global_drop_func
--        @description  enables/disables PFC global drop
--
--        @param params         - params["devID"]: device number;
--                                params["flagNo"]: no command property;
--
--        @return       true on success, otherwise false and error message
--
local function dce_priority_flow_control_global_drop_func(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local port_pfc_global_drop_enable
    
    -- Common variables initialization    
    devNum                      = params["devID"]    
    devFamily                   = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
    
    -- Command specific variables initialization.     
    port_pfc_global_drop_enable = command_data:getTrueIfFlagNoIsNil(params)
    
    if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
       ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
        -- Enabling/disabling of PFC global drop.
        if true  == command_data["status"]  then
            result, values = 
                cpssPerDeviceParamSet("cpssDxChPortPfcGlobalDropEnableSet",
                                      devNum, port_pfc_global_drop_enable, 
                                      "enable", "GT_BOOL")
            if       0x10 == result   then
                command_data:setFailStatus() 
                command_data:addWarning("It is not allowed to %s PFC global " ..
                                        "drop on device %d.",  
                                        boolEnableLowerStrGet(
                                            port_pfc_global_drop_enable), 
                                        devNum) 
            elseif    0 ~= result   then
                command_data:setFailStatus()
                command_data:addError("Error at %s of PFC global drop " ..
                                      "on device %d: %s.",  
                                      boolEnablingLowerStrGet(
                                          port_pfc_global_drop_enable), 
                                      devNum, returnCodes[result])
            end     
        end
    else
        command_data:setFailStatus()    
        command_data:addWarning("Family of device %d does not supported.", 
                                devNum)    
    end
    
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
  return command_data:getCommandExecutionResults() 
end


--------------------------------------------------------------------------------
-- command registration: dce priority-flow-control global drop
--------------------------------------------------------------------------------
CLI_addCommand("debug", "dce priority-flow-control global drop", {
    func   = dce_priority_flow_control_global_drop_func,
    help   = "Enabling of PFC global drop",
    params = {
        { type = "values",
            "%devID"
        }
  }
})

--------------------------------------------------------------------------------
-- command registration: no dce priority-flow-control global drop
--------------------------------------------------------------------------------
CLI_addCommand("debug", "no dce priority-flow-control global drop", {
    func   = function(params) 
               params.flagNo = true
               dce_priority_flow_control_global_drop_func(params)
           end,
    help   = "Disabling of PFC global drop",
    params = {
        { type = "values",
            "%devID"
        }
  }
})
