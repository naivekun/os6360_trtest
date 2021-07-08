--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* iterators.lua
--*
--* DESCRIPTION:
--*  implementation of the iterators with log.trace support:
--*   - iterators basic class, 
--*   - function iterate() to create simple cycles and
--*   - function iterator() to use it in 'for x in iterator(i) do ...'
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

require("common/generic/class")

--  Iterator base class 
Iterator = class(
    function(object)
        object.key = nil
        object.name = "none" -- iterator name
    end)

function Iterator:restart() -- return key
end

function Iterator:next()
end

function Iterator:nextDev()
end
 

-- ************************************************************************
---
--  iterate
--    @description  run iteration 
--    for each iteration function is called as func(key, ...)
--
--
function iterate(iterator,func,...)
    log.trace("START ITERATION "..iterator.name)
    local key,rc,ret
    key = iterator:restart()
    while key ~= nil do
--        log_set_key(key)
        log.trace("ITEM BEGIN")
        local action = "next"
        rc,ret = pcall(func,key,...)
        if rc then
            if ret == 1 then
                action = "next"
            end
            if ret == 2 then
                action = "nextdev"
            end
        else
            -- error
            if string.sub(ret,-13) == "::ERRNEXTITEM" then
                action = "next"
            elseif string.sub(ret,-15) == "::ERRNEXTDEVICE" then
                action = "nextdev"
            else
                action = "break"
            end
        end
        log.trace("ITEM END")
        if action == "next" then
            key = iterator:next()
        elseif action == "nextdev" then
            log.trace("SKIP_TO_NEXT_DEV")
            key = iterator:nextDev()
        else
            break
        end
    end
    -- log_set_key()
    log.trace("END ITERATION "..iterator.name)
end

-- ************************************************************************
---
--  iterator
--    @description  
--      Create iterator from 'Iterator' class
--       usage:
--          x=Iterator()
--          for k in iterator(x) do
--            print(to_string(k))
--          end
--
function iterator(i)
    local first = true
    return function()
        if not first then
            return i:next()
        end
        first = false
        return i:restart()
    end
end

