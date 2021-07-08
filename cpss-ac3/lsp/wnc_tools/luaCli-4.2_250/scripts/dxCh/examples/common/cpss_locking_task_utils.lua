--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpss_locking_task_utils.lua
--*
--* DESCRIPTION:
--*       Task code, which operates in separate thread. Will run cpss apis
--*       with cpssGenWrapper() according to parameter passed in task_Param table
--*       from main thread.
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



task_args = task_Param
msgQ = task_Param[1]

if task_Param[2].api ~= nil then
    task_args = { task_Param[2] }
else
    task_args = task_Param[2]
end

if task_Param[3] ~= nil then
    count = task_Param[3]
else
    count = 1
end

for j=1, count do
    for i,args in pairs(task_args) do
        delay(args.delay)
        for i=1, args.count do
            result, values = cpssGenWrapper(args.api, args.param, true)
            logf('%s %s %s = %s %s', task_Name, args.api, to_string(args.param), to_string(result), to_string(values))
        end
    end
end

if msgQ ~= nil then
    result, values = cpssGenWrapper("osTaskGetSelf",{{"OUT","GT_U32","tid"}}, true)
    luaMsgQSend(msgQ, values.tid)
end


