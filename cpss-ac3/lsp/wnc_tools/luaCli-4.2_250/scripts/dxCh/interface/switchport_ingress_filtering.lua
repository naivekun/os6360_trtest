--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* switchport_ingress_filtering.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of ingress filtering of n interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants 


-- ************************************************************************
---
--  switchport_ingress_filtering_func
--        @description  enables/disables ingress filtering of n interface
--
--        @param params             - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
-- 
local function switchport_ingress_filtering_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local ingress_filtering_enable
    
    -- Common variables initialization    
    command_data:initInterfaceDevPortRange()
    
    -- Common variables initialization
    ingress_filtering_enable = valueIfConditionNil(params["flagNo"], true, 
                                                   false)
  
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
    
            -- Ingress filtering enabling setting.
            command_data:clearLocalStatus()
    
            if true == command_data["local_status"]     then
                result, values = 
                    cpssPerPortParamSet("cpssDxChBrgVlanPortIngFltEnable", 
                                        devNum, portNum, 
                                        ingress_filtering_enable, "enable", 
                                        "GT_BOOL")  
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to set ingress " ..
                                            "filtering on device %d port %d.", 
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at ingress setting on " ..
                                          "device %d port %d: %s.", devNum, 
                                          portNum, returnCodes[result])
                end               
            end    
    
            command_data:updateStatus()
            
            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not configure ingress filtering on all processed ports.")
    end
    
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()  
end


--------------------------------------------------------
-- command registration: switchport ingress-filtering
--------------------------------------------------------
CLI_addCommand({"interface", "interface_eport"}, "switchport ingress-filtering", {
  func   = switchport_ingress_filtering_func,
  help   = "ingress filtering"
})


--------------------------------------------------------
-- command registration: no switchport ingress-filtering
--------------------------------------------------------
CLI_addCommand({"interface", "interface_eport"}, "no switchport ingress-filtering", {
  func   = function(params) 
               params.flagNo=true
               switchport_ingress_filtering_func(params)
           end,
  help   = "ingress filtering"
})
