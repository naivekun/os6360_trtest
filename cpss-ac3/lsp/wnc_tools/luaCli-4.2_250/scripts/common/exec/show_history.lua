--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_history.lua
--*
--* DESCRIPTION:
--*       showing of the commands that were entered during the current session
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants 


-- ************************************************************************
---
--  show_history_func
--        @description  show's of the commands that were entered during the 
--                      current session
--
--        @param params         - not used
--
--        @return       true on success, otherwise false and error message
--
local function show_history_func(params)
    -- Common variables declaration
    local result, values
    local stringItem
    local command_data = Command_Data()
    -- Command specific variables declaration
    local header_string, footer_string
    
    -- Common variables initialization    
    command_data:clearResultArray()
    command_data:initStringItemsRange(params, cmdLuaCLI_history)    
    
    -- Command specific variables initialization  
    header_string   = "\n"
    footer_string   = "\n"      
    
    -- Main string item handling cycle
    if true == command_data["status"]                               then
        local iterator
        for iterator, stringItem in command_data:getStringItemsIterator() do
            command_data:clearItemStatus()

            command_data:clearLocalStatus()
            
            command_data:addToResultArray(stringItem)
       
            command_data:updateStatus()
       
            command_data:updateItems()
        end
    end            

    -- Deleting of last command from history.
    if (true == command_data["status"])                             and
       (true == isNotEmptyTable(command_data["result_array"]))      then
        command_data["result_array"][command_data:getLastResultArrayItemIndex()] = 
            nil
        command_data["items_count"] = command_data["items_count"] - 1
    end
   
    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnItemsCount(header_string, command_data["result"], 
                                          footer_string,
                                          "There is no history to show\n")    
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()     
end    


--------------------------------------------
-- command registration: show history
--------------------------------------------  
CLI_addCommand("exec", "show history", {
  func   = show_history_func,
  help   = "Display the session command history"
})