--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_priority_flow_control_global_queue.lua
--*
--* DESCRIPTION:
--*       performing of global PFC settings for a given queue (traffic class)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  dce_priority_flow_control_global_queue_func
--        @description  perform's global PFC settings for a given queue 
--                      (traffic class)
--
--        @param params         - params["devID"]: device number;
--                                params["priority"]: cprofile priority;
--                                params["xon"]: xon threshold;
--                                params["xoff"]: xoff threshold;
--
--        @return       true on success, otherwise false and error message
--
local function dce_priority_flow_control_global_queue_func(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum
    local command_data = Command_Data()
    -- Command specific variables declaration     
    local priority, xoff, drop_threshold
    
    -- Common variables initialization    
    devNum          = params["devID"]
    devFamily       = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
    
    -- Command specific variables initialization.     
    priority        = params["priority"]
    xoff            = params["xoff"]
    drop_threshold  = params["drop_threshold"]
    
    if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
       ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
        -- Setting of PFC profile configurations for given tc queue.
        if true  == command_data["status"]  then
            result, values =                    
                myGenWrapper("cpssDxChPortPfcGlobalQueueConfigSet",
                             {{"IN", "GT_U8",   "devNum",        devNum     },
                              {"IN", "GT_U8",   "tcQueue",       priority   }, 
                              {"IN", "GT_U32",  "xoffThreshold", xoff       },
                              {"IN", "GT_U32",  "dropThreshold", 
                                                             drop_threshold }}) 
            if       0x10 == result   then
                command_data:setFailStatus() 
                command_data:addWarning("It is not allowed to set PFC " ..
                                        "profile configurations for tc " ..
                                        "queue %d on device %d.", priority, 
                                        devNum) 
            elseif    0 ~= result   then
                command_data:setFailStatus()
                command_data:addError("Error at setting of PFC profile " .. 
                                      "configurations for tc queue %d " ..
                                      "on device %d: %s.", priority, devNum, 
                                      returnCodes[result])
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
-- command registration: dce priority-flow-control global queue
--------------------------------------------------------------------------------
CLI_addHelp("debug",    "dce priority-flow-control global", 
                                            "Performing of global PFC settings")
CLI_addCommand("debug", "dce priority-flow-control global queue", {
    func   = dce_priority_flow_control_global_queue_func,
    help   = "Performing of global PFC settings for a given queue (traffic " ..
             "class)",
    params = {
        { type = "values",
            "%devID"
        },
        { type = "named",
            "#priority",
            "#xoff_bit11",
            "#drop_threshold_bit11",
          mandatory = { "priority", "xoff", "drop_threshold" }
        }
  }
})

