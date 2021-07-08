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
--*       functionality of paused message stream printing
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

do

--------------------------------------------------------------------------------
-- variables for paused message stream printing
--------------------------------------------------------------------------------
local print_nice_saved_print            = print
local print_nice_saved_stack            = { }
local print_nice_lines                  = 0
local print_nice_interrupted            = false
local print_nice_activating             = false
local breaking_function                 = nil
local breaking_function_calling_data    = nil
local ignore_all_nice_printings_mode    = false

-- ************************************************************************
---
--  print_nice
--        @description  print's messages with periodical showing of
--                      interrupting message
--                      "Type <CR> to continue, Q<CR> to stop:"
--
--        @param ...            - printed message
--
--        @return       saved printing function execution result or nil if
--                      it is not called
--
function print_nice(...)
    if ignore_all_nice_printings_mode == true then
        -- ignore any option to break the print ... as we are running test !
        return print_nice_saved_print(...)
    end

    if true ~= print_nice_interrupted   then
        print_nice_lines = print_nice_lines + 1
        if print_nice_lines > 20 then
            local reply = cmdLuaCLI_readLine("Type <CR> to continue, " ..
                                             "Q<CR> to stop:")
            if reply == "Q" or reply == "q" then
                print_nice_interrupted = true

                if "function" ~= type(breaking_function) then
                    return nil
                else
                    breaking_function(breaking_function_calling_data)
                    return nil
                end
            end
            print_nice_lines = 0
        end

        return print_nice_saved_print(...)
    end
end


-- ************************************************************************
---
--  is_paused_printing_activated
--        @description  get's paused printing activation property
--
--
function is_paused_printing_activated()
    return print_nice_activating
end


-- ************************************************************************
---
--  is_paused_printing_interrupted
--        @description  get's paused printing interruption property
--
--
function is_paused_printing_interrupted()
    return print_nice_interrupted
end

-- ************************************************************************
---
--  ignore_all_nice_printings
--        @description  force the engine to not 'pause' printings with :
--              Type <CR> to continue, Q<CR> to stop:exit
--          because we are during 'lua test'
--         @ param enable       - enable/disable the 'ignore all'
--
function ignore_all_nice_printings(enable)
    if enable == nil then enable = true end

    ignore_all_nice_printings_mode    = enable
end

-- ************************************************************************
---
--  reset_paused_printing
--        @description  set's paused printing counters to default
--
--
function reset_paused_printing()
    print_nice_lines        = 0
    print_nice_interrupted  = false
    print_nice_activating   = false
    breaking_function       = nil
end


-- ************************************************************************
---
--  push_print_function
--        @description  pushing of current function to printing functions
--                      stack
--
--        @param func           - saved function; if irrelevant, as
--                                printing function handles "print"
--
function push_print_function(func)
    if nil == func  then
        func    = print
    end

    if "function" == type(func) then
        tableAppend(print_nice_saved_stack, func)
    end
end


-- ************************************************************************
---
--  push_and_replace_print_function
--        @description  pushing of current function to printing functions
--                      stack and replace it with paused printing  function
--
--        @param func           - saved function; if irrelevant, as
--                                printing function handles "print"
--
--        @return       replaced print function
--
function push_and_replace_print_function(func)
    if true == myCPSSGenWrapperPausedPrinting   then
    push_print_function(func)

    if nil == func  then
        print   = print_nice
    end
    func    = print_nice

    print_nice_activating  = true

    return func
    elseif nil == func                          then
        return print
    else
        return func
    end
end


-- ************************************************************************
---
--  restore_and_pop_print_function
--        @description  restoring of previus printing function and poping
--                      it from printing functions stack
--
--        @param non_default_printing
--                              - property of non-default printing function
--                                restoring
--
--        @return       restored print function
--
function restore_and_pop_print_function(non_default_printing)
    if true == myCPSSGenWrapperPausedPrinting   then
    local func

    if true == isNotEmptyTable(print_nice_saved_stack)  then
        func    = getLastTableItem(print_nice_saved_stack)
        popLastTableItem(print_nice_saved_stack)

        if true ~= non_default_printing then
            print   = func
        end
    end

    print_nice_activating  = false

    return func
    elseif true ~= non_default_printing         then
        return print
    else
        return nil
    end
end


-- ************************************************************************
---
--  set_breaking_function
--        @description  set's function pointer which calls on printing
--                      break
--
--        @param func           - breaking function
--        @param func_data      - breaking function calling data
--
function set_breaking_function(func, func_data)
    if "function" == type(func) then
        breaking_function               = func
        breaking_function_calling_data  = func_data
    end
end


-- ************************************************************************
---
--  do_command_with_print_nice
--        @description  executes command with paused printing
--
--        @param cmd            - printed message
--        @param ...            - printed message
--
function do_command_with_print_nice(cmd, ...)
    print_nice_saved_print = print
    print_nice_lines = 0
    print_nice_interrupted = false
    print = print_nice

    local success, rc

    success, rc = pcall(...)

    print = print_nice_saved_print

    if success then
        return rc
    end
    if print_nice_interrupted then
        print_nice_interrupted = false
        return true
    end

    print("Failed in "..cmd..": "..rc)
    return false
end

CLI_addCommand("exec", "set output paused", {
    help="Enable paused output",
    func=function()
        ignore_all_nice_printings(false)
        return true
    end
})

CLI_addCommand("exec", "set output nopause", {
    help="Disable paused output",
    func=function()
        ignore_all_nice_printings(true)
        return true
    end
})


end
