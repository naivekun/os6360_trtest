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
--*       binding of a PFC profile to an interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  dce_priority_flow_control_bind_func
--        @description  bind's a PFC profile to an interface
--
--        @param params         - params["pfc_profile"]: The PFC profile 
--                                number;
--                                params["ethernet"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil;
--
--        @return       true on success, otherwise false and error message
--
local function dce_priority_flow_control_bind_func(params)
     -- Common variables declaration
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration 
    local pfc_profile
    
    -- Common variables initialization
    command_data:initAllInterfacesPortIterator(params)

    -- Command specific variables initialization.     
    pfc_profile     = params["pfc_profile"]
 
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()    
            
            command_data:clearLocalStatus()

            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
                -- Binding of a source port to a PFC profile.
                if true == command_data["local_status"]         then
                    result, values = 
                        cpssPerPortParamSet("cpssDxChPortPfcProfileIndexSet", 
                                            devNum, portNum, pfc_profile,
                                            "profileIndex", "GT_U32")
                    if  0x10 == result then
                        command_data:setFailPortAndLocalStatus() 
                        command_data:addWarning("It is not allowed to bind " ..
                                                "a source port to a PFC " ..
                                                "profile %d on device %d " ..
                                                "port %d.", pfc_profile, devNum, 
                                                portNum)
                    elseif 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at binding of a source " ..
                                              "port to a PFC profile %d on " .. 
                                              "device %d port %d: %s.", 
                                              pfc_profile, devNum, portNum, 
                                              returnCodes[result])
                    end
                end    
            else
                command_data:setFailStatus()             
                command_data:addWarningOnceOnKey(
                    devNum, "Family of device %d does not supported.", devNum)
            end
            
            command_data:updateStatus()            
            
            command_data:updatePorts()            
        end
    end            
          
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
  return command_data:getCommandExecutionResults()              
end


--------------------------------------------------------------------------------
-- command registration: dce priority-flow-control bind
--------------------------------------------------------------------------------
CLI_addCommand("debug", "dce priority-flow-control bind", {
    func   = dce_priority_flow_control_bind_func,
    help   = "Binding of a PFC profile to an interface",
    params = {
        { type = "named",
            "#pfc_profile",
            "#single_interface",
          mandatory = { "pfc_profile", "ethernet" }
        }
  }
})

