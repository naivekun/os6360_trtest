--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* wraplCpssCommon.lua
--*
--* DESCRIPTION:
--*       CPSS common functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


--
-- Import Lua C functions from wraplCpssCommon.c
--
cmdLuaCLI_registerCfunction("wrlIsDevIdValid")
cmdLuaCLI_registerCfunction("wrlDevList")
cmdLuaCLI_registerCfunction("wrlCpssDeviceFamilyGet")
--
-- Functions
--


-- ************************************************************************
---
--  list_to_string
--        @description  Convert list of numbers to comma separated string.
--                      print(list_to_string({1,4,6,7,8,9,11,20}))
--                          1,4,6-9,11,20
--
--        @param list           - parameter string
--
--        @return       target string
--
function list_to_string(list)
    local s
    local is_range
    is_range = false
    for k = 1, #list do
        if k == 1 then
            s = list[k]
        elseif list[k-1]+1 ~= list[k] then
            if is_range then
                s = s .. "-" .. list[k-1]
            end
            is_range = false
            s = s .. "," .. list[k]
        else
            is_range = true
        end
    end
    if is_range then
        s = s .. "-" .. list[#list]
    end
    return s
end


-- ************************************************************************
---
--  string_to_list
--        @description  Convert string to list of numbers 
--                      print(table.concat(
--                          string_to_list("1,4,6-9,11,20"), ","))
--                      1,4,6,7,8,9,11,20
--
--        @param s              - parameter string
--
--        @return       target lits
--
function string_to_list(s)
    local list
    local is,ie,ch,val,prevch,prev
    list = {}
    while true do
        is,ie,ch = string.find(s, "([-,])")
        if type(is) == "nil" then
            val = tonumber(s)
        else
            val = tonumber(string.sub(s,1,is-1))
        end

        if prevch == "-" then
            prev = prev + 1
            while prev < val do
                table.insert(list, prev)
                prev = prev + 1
            end
        end
        table.insert(list,val)

        if type(is) == "nil" then
            break
        else
            prev = val
            prevch = ch
            s = string.sub(s,ie+1)
        end
    end

    return list
end


