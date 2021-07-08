--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* delete.lua
--*
--* DESCRIPTION:
--*       deleting of file from RAMFS
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes
require("common/running_config/config_file_support")

--constants


-- ************************************************************************
---
--  delete_func
--        @description  deletes file from RAMFS
--
--        @param params         - params["filename"]: file name
--
--        @return       true on success, otherwise false and error message
--
local function delete_func(params)
    -- Common variables declaration.
    local result, values
    local devNum, ports
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local removed_file_name
    
    -- Command specific variables initialization.  
    removed_file_name   = params["filename"]
    
    -- RAMFS file deleting.
    if true == command_data["status"] then    
        result  = fs.unlink(removed_file_name)
        if 0 ~= result  then
            command_data:setFailStatus()
            command_data:addError("Error at deleting of file %s from RAMFS.",
                                  removed_file_name)
        end
    end
    
    -- Setting of global variables which containts running config data.
    if true == command_data["status"] then    
        CLI_number_of_first_line_in_running_Cfg_Tbl, 
            CLI_first_line_in_running_Cfg_Tbl = get_last_line_of_Cfg_Tbl()
    end    
    
    -- Execution of garbage collection.
    if true == command_data["status"] then  
        collectgarbage("collect")
    end
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()      
end


--------------------------------------------------------------------------------
-- command registration: delete
--------------------------------------------------------------------------------
CLI_addCommand("running-config", "delete", {
  func   = delete_func,
  help   = "Deleting of file from RAMFS",
  params = {
      { type = "values", 
          "%filename"
      }
  }
})
