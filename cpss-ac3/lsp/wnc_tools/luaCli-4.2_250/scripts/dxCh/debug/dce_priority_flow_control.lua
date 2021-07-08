--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_priority_flow_control.lua
--*
--* DESCRIPTION:
--*       setting of a PFC profile per priority
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  dce_priority_flow_control_func
--        @description  set's of a PFC profile per priority
--
--        @param params         - params["pfc_profile"]: The PFC profile 
--                                number;
--                                params["dev"]: device number;
--                                params["priority"]: cprofile priority;
--                                params["xon"]: xon threshold;
--                                params["xoff"]: xoff threshold;
--
--        @return       true on success, otherwise false and error message
--
local function dce_priority_flow_control_func(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum
    local command_data = Command_Data()
    -- Command specific variables declaration     
    local pfc_profile, priority, xon, xoff, profile_config
    
    -- Common variables initialization    
    devNum          = params["dev"]
    devFamily       = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
    
    -- Command specific variables initialization.     
    pfc_profile     = params["pfc_profile"]
    priority        = params["priority"]
    xon             = params["xon"]
    xoff            = params["xoff"]
    profile_config  = { ["xonThreshold"]    = xon,
                        ["xoffThreshold"]   = xoff  }

    if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
       ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
        -- Port PFC profile configuring.
        if true  == command_data["status"]  then
            result, values =                    
                myGenWrapper("cpssDxChPortPfcProfileQueueConfigSet",
                             {{"IN", "GT_U8",    "devNum",       devNum      },
                              {"IN", "GT_U32",   "profileIndex", pfc_profile }, 
                              {"IN", "GT_U8",    "tcQueue",      priority    },
                              {"IN", "CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC",  
                                             "pfcProfileCfg", profile_config }}) 
            if       0x10 == result   then
                command_data:setFailStatus() 
                command_data:addWarning("It is not allowed to configure " .. 
                                        "port pfc profice %d on device %d.",  
                                        pfc_profile, devNum) 
            elseif    0 ~= result   then
                command_data:setFailStatus()
                command_data:addError("Error at configuring of port pfc " ..
                                      "profice %d on device %d: %s.",  
                                      pfc_profile, devNum, returnCodes[result])
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
-- command registration: dce priority-flow-control
--------------------------------------------------------------------------------
CLI_addHelp("debug",    "dce",      "DCE subcommands")
CLI_addCommand("debug", "dce priority-flow-control", {
    func   = dce_priority_flow_control_func,
    help   = "Setting of a PFC profile per priority",
    params = {
        { type = "named",
          "#pfc_profile",
          "#dev",
          "#priority",
          "#xon_bit11",
          "#xoff_bit11",
      mandatory = { "pfc_profile", "dev", "priority", "xon", "xoff" }
        }
  }
})
