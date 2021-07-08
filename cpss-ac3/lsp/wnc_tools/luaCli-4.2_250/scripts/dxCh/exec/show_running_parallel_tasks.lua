--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_running_parallel_tasks.lua
--*
--* DESCRIPTION:
--*       Show and delete parallel running tasks with running C functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  delete_running_parallel_tasks
--        @description  delete task running in parallel with main process
--
--        @return       none
--
local function delete_running_parallel_task(params)
    local rc
    local taskId = params["taskId"]
    rc = new_instance_shell_cmd_kill(taskId)
    if rc == false then
        print("Task: " .. to_string(taskId) .." was not found")
    end
end

-- ************************************************************************
---
--  show_running_parallel_tasks
--        @description  shows tasks running in parallel with main process
--
--        @return       true
--
local function show_running_parallel_tasks(params)
    local command_data = Command_Data()
    local header_string, footer_string
    local rc, instance

    -- Common variables initialization
    command_data:clearResultArray()

    -- Command specific variables initialization
    header_string       =
        "\n" ..
        "           Command                  Task ID   \n" ..
        "-------------------------------  ------------ \n"
    footer_string       = "\n"

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    if #threads ~= 0 then
        -- Loop all tasks in database 
        for instance=1,#threads do
            -- the task found
            rc = new_instance_shell_cmd_wait(threads[instance].taskId, 0)
            if rc == false then
                -- the task still running - show it's status
                print("Running command " .. threads[instance].command .. ",Task ID: " .. to_string(threads[instance].taskId))
                command_data["result"] =
                    string.format("%-32s %-12s", 
                                  threads[instance].command,
                                  threads[instance].taskId)

                -- command_data:addResultToResultArrayOnCondition(
                command_data:addResultToResultArray()
                command_data:updateStatus()
                command_data:updateItems()
            else
               -- the task is finished - remove from database
                print("Task: " .. threads[instance].command .. ",Task ID: " .. to_string(threads[instance].taskId) .. " removed")
                table.remove(threads, instance)
            end
        end
    end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnItemsCount(header_string, command_data["result"], 
                                          footer_string,
                                          "There is no task to show\n")
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()      
end

-- Parallel running task show
CLI_addCommand("exec", "show running-parallel-tasks", {
    func=show_running_parallel_tasks,
    help="Parallel running tasks"
})

-- Parallel running task delete
CLI_addCommand("exec", "delete running-parallel-task", {
    func=delete_running_parallel_task,
    help="Delete parallel running task",
    params={
        { type="values",
            { format = "%GT_U32", name = "taskId"},
            mandatory = {"taskId"}
        }
    }
})

