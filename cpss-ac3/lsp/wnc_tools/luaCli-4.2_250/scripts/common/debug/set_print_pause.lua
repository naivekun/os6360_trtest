--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* printing_with_pause.lua
--*
--* DESCRIPTION:
--*       configuring interrupting message periodical showing at table printing
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
myCPSSGenWrapperPausedPrinting  = true


-- ************************************************************************
---
--  speed_func
--        @description  configures interrupting message periodical showing 
--                      at table printing
--
--        @param params         - params["enable_disable"]: paused printing 
--                                enabling property
--
--        @usage myCPSSGenWrapperPausedPrinting       
--                              - global paused printing option
--
--        @return       true on success, otherwise false and error message
--
function set_print_pause_enable_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local paused_printing_enabling
    
    -- Command specific variables initialization.
    paused_printing_enabling    = isEquivalent(params["enable_disable"], 
                                               "enable")
    
    -- Setting of paused printing property.
    if true == command_data["status"] then
        myCPSSGenWrapperPausedPrinting  = paused_printing_enabling
    end    
    
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()      
end


--------------------------------------------------------------------------------
-- command registration: set print pause
--------------------------------------------------------------------------------
CLI_addHelp("debug",    "set",              "Setting of lua-wraper options")
CLI_addHelp("debug",    "set print",        "Configuring of printing functions")
CLI_addCommand("debug", "set print pause", {
  func   = set_print_pause_enable_func,
  help   = "Configuring of interrupting message periodical showing at table " ..
           "printing",
  params = {
      { type = "named",
          { format = "enable", 
                        help = "Enabling of interrupting message " ..
                               "periodical showing"                           },
          { format = "disable", 
                        help = "Disabling of interrupting message " ..
                               "periodical showing"                           },
        alt       = { enable_disable    = { "enable", "disable" }             }, 
        mandatory = { "enable_disable" }
      }
  }           
})
