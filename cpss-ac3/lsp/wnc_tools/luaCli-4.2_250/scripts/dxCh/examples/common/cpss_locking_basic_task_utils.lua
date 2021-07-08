--********************************************************************************
--*              (C), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpss_locking_basic_task_utils.lua
--*
--* DESCRIPTION:
--*       Creating task for cpss_locking_basic test
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

require("common/generic/common")
require("CLI")

cmdLuaCLI_registerCfunction("luaMsgQSend")

function log(msg)
    if msg ~= nil then
        if log_console ~= nil then
            log_console(msg.."\r")
        else
            print(msg)
        end
    end
end
function logf(...)
    log(string.format(...))
end

function delay(mSec)
    local ret
    ret = cpssGenWrapper("osTimerWkAfter",{{"IN","GT_U32","mils",mSec}}, true)
end

local msgQ = task_Param[1]
local task_args = task_Param[2]

local errorId = 0
for i,args in pairs(task_args) do
    for n=1, args.count do
        result, values = cpssGenWrapper(args.api1, args.param1, true)
        delay(args.delay)
        result, values = cpssGenWrapper(args.api2, args.param2, true)

        if task_Name == "test_task1" then
            if values.enablePtr ~= args.checkValue then
                logf('%d: %s %s = %s %s', n, task_Name, args.api1, to_string(result), to_string(values))
                logf("ERROR: written and read values are different in %s", args.api1)
                errorId = 1
            end
        else
            if values.lookupTypePtr ~= args.checkValue then
                logf('%d: %s %s = %s %s', n, task_Name, args.api2, to_string(result), to_string(values))
                logf("ERROR: written and read values are different in %s", args.api1)
                errorId = 1
            end
        end
        result, values = cpssGenWrapper(args.api1, args.param3, true)
    end
end

if msgQ ~= nil then
   luaMsgQSend(msgQ, tonumber(errorId), 2000)
end


