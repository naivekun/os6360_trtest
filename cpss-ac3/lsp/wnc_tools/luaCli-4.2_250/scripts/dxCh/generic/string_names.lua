--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* string_names.lua
--*
--* DESCRIPTION:
--*       command processing subroutines
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes

--constants 


--------------------------------------------------------------------------------
--        construct's command execution data class
--------------------------------------------------------------------------------
Index_String_Names = class(
    function(object)
        store   = {}
    end)    

    
-- ************************************************************************
---
--  Command_Data:clear
--        @description  clear's all name and its indexes
--
--        @return       operation succed
--
function Command_Data:clear()
    self["v"] = {}
    
    return 0
end    


-- ************************************************************************
---
--  Index_String_Names:checkName
--        @description  check's of string name 
--
--        @param result         - last operation result
--        @param name           - entry name (string value) 
--
--        @return       operation status and error message, if detected
-- 
function Index_String_Names:checkName(result, name)
    local error_message = nil
    
    if (0 == result) and ("string" ~= type(name))               then
        result          = 3
        error_message   = "Name is not string value"
    end  
    
    if (0 == result) and (false == isParameterName(name))       then
        result          = 3
        error_message   = "Enty name \'" .. name .. "\' is not parameter name"    
    end
    
    return result, error_message
end
    

-- ************************************************************************
---
--  Index_String_Names:checkIdex
--        @description  check's of number index 
--
--        @param result         - last operation result
--        @param index          - entry index (number value) 
--
--        @return       operation status and error message, if detected
-- 
function Index_String_Names:checkIdex(result, index)
    local error_message = nil
    
    if (0 == result) and ("number" ~= type(index))              then
        result          = 3
        error_message   = "Index is not number value"
    end     
    
    return result, error_message    
end


-- ************************************************************************
---
--  Index_String_Names:getIndex
--        @description  get's index according to entry name
--
--        @param name           - added name (string value)
--
--        @return       searched index or -1 and error message if error
--                      occures
-- 
function Index_String_Names:getIndex(name)   
    local result, error_message = getResultErrorInitPair()
    local index
       
    result, error_message   = checkIdex(result, index)
    
    index                   = self:store[name]
    
    if nil == index                                              then
        result          = 3
        error_message   = "Given name was not added"
    end

    if 0 == result                                              then
        return index
    else
        return -1, error_message
    end
end


-- ************************************************************************
---
--  Index_String_Names:getName
--        @description  get's name according to its index
--
--        @param index          - added index (number value)
--
--        @return       searched name or empty string and error message if 
--                      error occures
-- 
function Index_String_Names:getName(index)   
    local result, error_message = getResultErrorInitPair()
    local name
    
    result, error_message   = checkName(result, name) 
    
    name                    = self:store[index]
    
    if nil == name                                              then
        result          = 3
        error_message   = "Given index was not added"
    end

    if 0 == result                                              then
        return name
    else
        return "", error_message
    end
end


-- ************************************************************************
---
--  Index_String_Names:get
--        @description  get's name or index according to entry data type
--
--        @param param          - added index (number value)
--
--        @return       in case of string param:    searched name or empty 
--                      string and error message if error occures
--                      in case of number param:    searched index or -1 
--                      and error message if error
--                      in other cases:             nil and error message
-- 
function Index_String_Names:get(param)   
    if     "string" == type(param)                              then
        return getName(param)
    elseif "number" == type(param)                              then
        return getIndex(name) 
    else
        return nil, "Entry parameter could be only number or string"
    end
end


-- ************************************************************************
---
--  Index_String_Names:add
--        @description  add's new name of index 
--
--        @param name           - added name (string value) 
--        @param index          - added index (number value)
--
--        @return       operation status and error message, if occures
-- 
function Index_String_Names:add(name, index)   
    local result, error_message = getResultErrorInitPair()
    
    result, error_message   = checkName(result, name) 
    result, error_message   = checkIdex(result, index)
    
    self:store[name]        = index
    self:store[index]       = name
    
    return result, error_message
end


-- ************************************************************************
---
--  Index_String_Names:erase
--        @description  deletes name and its index
--
--        @param name           - deleted name (string value) 
--
--        @return       operation status and error message, if occures
-- 
function Index_String_Names:erase(name)   
    local result, error_message = getResultErrorInitPair()
    local index
    
    result, error_message   = checkName(result, name) 
    
    index                   = self:store[name]
    self:store[name]        = nil
    self:store[index]       = nil
    
    return result, error_message
end
    