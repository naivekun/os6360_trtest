--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* common.lua
--*
--* DESCRIPTION:
--*       common lua functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 22 $
--*
--********************************************************************************

--includes


__common_global = {}

-- ************************************************************************
---
--  setGlobal
--        @description  Stores a parameter in a global table 
--
--        @param name           - The name of the parameter
--        @param value          - The value of the parameter to store
--
--        @return       true
--
function setGlobal(name, value)
    __common_global[name] = value
    return true
end


-- ************************************************************************
---
--  getGlobal
--        @description  Retrieves a parameter from the global table
--
--        @param  name          - The name of the parameter
--
--        @return       It's value
--
function getGlobal(name)
    return __common_global[name]
end

-- ************************************************************************
---
--  logicalNot
--        @description  logical inversion of boolean value
--
--        @param v                  - checked variable
--
--        @return       true if v ~= true, otherwise false
-- 
function logicalNot(v)
    return true ~= v 
end


-- ************************************************************************
---
--  isNil
--        @description  checking if parameter is nil
--
--        @param param              - entry parameter
--
--        @return       true if param is nil
-- 
function isNil(param)
    return nil == param
end


-- ************************************************************************
---
--  isNotNil
--        @description  checking if parameter is not nil
--
--        @param param              - entry parameter
--
--        @return       true if param is not nil
-- 
function isNotNil(param)
    return nil ~= param
end


-- ************************************************************************
---
--  isEquivalent
--        @description  comparision of values
--
--        @param first              - first value
--        @param second             - second value
--
--        @return       true if values are equal, otherwise false
-- 
function isEquivalent(first, second)
    return first == second
end


-- ************************************************************************
---
--  isNotEquivalent
--        @description  comparision of values
--
--        @param first              - first value
--        @param second             - second value
--
--        @return       true if values are not equal, otherwise false
-- 
function isNotEquivalent(first, second)
    return first ~= second
end


-- ************************************************************************
---
--  getResultErrorInitPair
--        @description  getting result/error message initialization pair 
--                      value
--
--        @return       0, nil
-- 
function getResultErrorInitPair()
    return o, nil
end


-- ************************************************************************
---
--  getTrueIfNil
--        @description  getting of boolean true if variable is undefined
--
--        @param v                  - checked variable
--
--        @return       true if nil, otherwise false
-- 
function getTrueIfNil(v)
    return nil == v 
end


-- ************************************************************************
---
--  getSign
--        @description  getting of number sign
--
--        @param x                  - checked number variable
--
--        @return       1   if number is positive;
--                      0   if number is equal to zero;
--                      -1  if number is negative;
--                      nil if variable is not a number
-- 
function getSign(x)
    if "number" == type(x) then
        return (x < 0 and -1) or (x > 0 and 1) or 0
    else
        return nil
    end
end


-- ************************************************************************
---
--  getFirstDefinedValue
--        @description  getting of firts defined value from entry params
--
--        @param alt1               - firts alterbative param
--        @param alt2               - second alterbative param
--        @param alt3               - third alterbative param
--
--        @return       value from entry params not equal to nil
-- 
function getFirstDefinedValue(alt1, alt2, alt3, ...)
    local index, value
    
    if     nil ~= alt1 then
        return alt1
    elseif nil ~= alt2 then
        return alt2
    elseif nil ~= alt3 then
        return alt3
    else
        for index, value in pairs(...) do
            if nil ~= value then
                return value
            end
        end
    end
    
    return nil    
end


-- ************************************************************************
---
--  getFalseIfNotNil
--        @description  getting of boolean false if variable is defined
--
--        @param v                  - checked variable
--
--        @return       false if not nil, otherwise true
-- 
function getFalseIfNotNil(v)
    return nil == v 
end


-- ************************************************************************
---
--  getTrueIfNotNil
--        @description  getting of boolean true if variable is defined
--
--        @param v                  - checked variable
--
--        @return       true if not nil, otherwise false
-- 
function getTrueIfNotNil(v)
    return nil ~= v 
end


-- ************************************************************************
---
--  getTrueIfValueInLits
--        @description  getting of boolean true if variable is in given 
--                      list
--
--        @param v                  - checked variable
--        @param ...                - given list
--
--        @return       true if value is in list, otherwise false
-- 
function getTrueIfValueInLits(v, ...)   
    for index, value in pairs(...) do
        if value == v   then
            return true
        end
    end
    
    return false
end


-- ************************************************************************
---
--  getTrueIfOneValueIsDefined
--        @description  getting of boolean true if at least one value is 
--                      not nil
--
--        @param ...                - values list
--
--        @return       true if at least one value is not nil, otherwise 
--                      false
-- 
function getTrueIfOneValueIsDefined(...)   
    for index, value in pairs(...) do
        if nil ~= v   then
            return true
        end
    end
    
    return false
end


-- ************************************************************************
---
--  valueIfConditionNotNil
--        @description  getting of value or its default according to 
--                      condition determination
--
--        @param cond               - condition
--        @param value              - given value
--        @param default            - default value
--
--        @return       value if condition is not nil, otherwise default
-- 
function valueIfConditionNotNil(cond, value, default)
    if nil ~= cond then
        return value
    else
        return default    
    end
end


-- ************************************************************************
---
--  valueIfConditionNil
--        @description  getting of value or its default according to 
--                      condition determination
--
--        @param cond               - condition
--        @param value              - given value
--        @param default            - default value
--
--        @return       value if condition is nil, otherwise default
-- 
function valueIfConditionNil(cond, value, default)
    if nil == cond then
        return value
    else
        return default    
    end
end


-- ************************************************************************
---
--  valueIfConditionFalse
--        @description  getting of value or its default according to 
--                      boolean condition
--
--        @param cond               - condition
--        @param value              - given value
--        @param default            - default value
--
--        @return       value if condition false, otherwise default
-- 
function valueIfConditionFalse(cond, value, default)
    if true ~= cond then
        return value
    else
        return default    
    end
end


-- ************************************************************************
---
--  valueIfConditionTrue
--        @description  getting of value or its default according to 
--                      boolean condition 
--
--        @param cond               - condition
--        @param value              - given value
--        @param default            - default value
--
--        @return       value if condition is true, otherwise default
-- 
function valueIfConditionTrue(cond, value, default)
    if true == cond then
        return value
    else
        return default    
    end
end


-- ************************************************************************
---
--  valueIfConditionNotTrue
--        @description  getting of value or its default according to 
--                      boolean condition
--
--        @param cond               - condition
--        @param value              - given value
--        @param default            - default value
--
--        @return       value if condition not true, otherwise default
-- 
function valueIfConditionNotTrue(cond, value, default)
    if true ~= cond then
        return value
    else
        return default    
    end
end


-- ************************************************************************
---
--  getDecimalOrZeroFromHex
--        @description  converts of hex value to devimal; if entry 
--                      parameter not hex string then returns 0
--
--        @param hex                - hex value
--
--        @return       converted hex value or 0, if entry parameter not 
--                      hex string
-- 
function getDecimalOrZeroFromHex(hex)
    if "string" == type(hex) then
        local returned = tonumber(hex, 16)
        
        if "number" == type(returned) then
            return returned
        else
            return 0    
        end
    else
        return 0
    end
end


-- ************************************************************************
---
--  isEmptyTable
--        @description  check's if table is empty
--
--        @param tbl            - The table
--
--        @return       true if table is empty otherwise false
--
function isEmptyTable(tbl)
    if nil ~= tbl then
        for key, value in pairs(tbl) do
            return false
        end  
    else
        return false
    end
    
    return true
end


-- ************************************************************************
---
--  isNotEmptyTable
--        @description  check's if table is not empty
--
--        @param tbl            - The table
--
--        @return       true if table is not empty otherwise false
--
function isNotEmptyTable(tbl)
    if nil ~= tbl then
        for key, value in pairs(tbl) do
            return true
        end  
    else
        return false
    end
    
    return false
end


-- ************************************************************************
---
--  isItemInTable
--        @description  checking if item is present in table
--
--        @param item               - checked item
--        @param tbl                - table
--
--        @return       true, if item is present, otherwise false
--
function isItemInTable(item, tbl)
    local index, value
    
    if "table" ~= type(tbl) then
        if nil ~= tbl   then
        tbl = { [1] = tbl }
        else
            return false
        end
    end
    
    for index, value in pairs(tbl) do
        if true == compareWithEtalonTable(item, value) then
            return true
        end
    end
    
    return false
end


-- ************************************************************************
---
--  isItemNotInTable
--        @description  checking if item is absent in table
--
--        @param item               - checked item
--        @param tbl                - table
--
--        @return       true, if item is absent, otherwise false
--
function isItemNotInTable(item, tbl)
    return false == isItemInTable(item, tbl)
end


--  
-- ************************************************************************
---
--  getItemTableKey
--        @description  get's item table key when is present in table
--
--        @param item               - item
--        @param tbl                - table
--
--        @return       table key, if item is present, otherwise nil
--
function getItemTableKey(item, tbl)
    local index, value
    
    if "table" ~= type(tbl) then
        return nil
    end
    
    for index, value in pairs(tbl) do
        if true == compareWithEtalonTable(item, value) then
            return index
        end
    end
    
    return nil
end


-- ************************************************************************
---
--  getLastTableItem
--        @description  getting of last table item
--
--        @param tbl            - The table
--
--        @return       last table item or nil, it it is empty table
--
function getLastTableItem(tbl)
    local index, item
    local ret_item = nil

    for index, item in pairs(tbl) do
        ret_item = item
    end
    
    return ret_item
end


-- ************************************************************************
---
--  getLastTableItemIndex
--        @description  getting of last table item index
--
--        @param tbl            - The table
--
--        @return       last table item index or nil, it it is empty table
--
function getLastTableItemIndex(tbl)
    local index, item
    local ret_index = nil

    for index, item in pairs(tbl) do
        ret_index = index
    end
    
    return ret_index
end


-- ************************************************************************
---
--  getTableSize
--        @description  getting of table size 
--
--        @param tbl            - The table
--
--        @return       table size or 0 is tbl is not table 
--
function getTableSize(tbl)
    if nil ~= tbl then
        local size = 0
    
        for key, value in pairs(tbl) do
            size = size + 1
        end  
        
        return size
    else
        return 0
    end
end


-- ************************************************************************
---
--  getNestedTableSize
--        @description  getting of nested table size
--
--        @param tbl            - The table
--
--        @return       table size or 0 is tbl is not table
--
function getNestedTableSize(tbl)
    if nil ~= tbl then
        local size = 0
    
        for key, value in pairs(tbl) do
            if "table" == type(value) then
                size = size + getNestedTableSize(value)
            else
                size = size + 1
            end
        end  
        
        return size
    else
        return 0
    end
end


-- ************************************************************************
---
--  getTableItemKey
--        @description  getting of key of given item in table
--
--        @param item               - checked item
--        @param tbl                - table
--
--        @return       item index, if it is present, otherwise false
--
function getTableItemKey(item, tbl)
    local index, value
    
    if "table" ~= type(tbl) then
        tbl = { [1] = tbl }
    end
    
    for index, value in pairs(tbl) do
        if item == value then
            return index
        end
    end
    
    return nil
end


-- ************************************************************************
---
--  getTableKeys
--        @description  getting of kyes of given table
--
--        @param tbl                - table
--
--        @return       keys table
--
function getTableKeys(tbl)
    local index, value
    local ret = {}
    
    if "table" ~= type(tbl) then
        tbl = {}
    end
    
    for index, value in pairs(tbl) do
        table.insert(ret, index)
    end
    
    return ret
end


-- ************************************************************************
---
--  tableInsert
--        @description  insert's item in table
--
--        @param tbl            - The table
--        @param itm            - The item
--
--        @return       table with appened item
--
function tableInsert(tbl, itm)
    if nil ~= tbl then
        table.insert(tbl, itm)
        return tbl
    else
        return {[1] = itm}
    end
end


-- ************************************************************************
---
--  tablePrepend
--        @description  prepend item to the end of table
--
--        @param tbl            - The table
--        @param itm            - The item
--
--        @return       table with appened item
--
function tablePrepend(tbl, itm)
    if nil ~= tbl then
        table.insert(tbl, 1, itm)
        return tbl        
    else
        return {[1] = itm}
    end
end


-- ************************************************************************
---
--  tableAppend
--        @description  append item to the end of table
--
--        @param tbl            - The table
--        @param itm            - The item
--
--        @return       table with appened item
--
function tableAppend(tbl, itm)
    if nil ~= tbl then
        tbl[#tbl + 1] = itm
        return tbl        
    else
        return {[1] = itm}
    end
end


-- ************************************************************************
---
--  tableAppendToTable
--        @description  append second table items to the end of first table
--
--        @param tbl            - The first table
--        @param tbl2           - The second table
--
--        @return       merged tables
--
function tableAppendToTable(tbl, tbl2)
    if nil ~= tbl then
        local index, item
        
        for index, item in ipairs(tbl2) do
            tbl[#tbl + 1] = item
        end

        return tbl        
    else
        return tbl2
    end
end


-- ************************************************************************
---
--  tableAppendOnCondition
--        @description  append item to the end of table on condition
--
--        @param cond           - The condition
--        @param tbl            - The table
--        @param itm            - The item
--
--        @return       table with appened item     if condition is "true"
--                      entry table                 if condition is "false"
--
function tableAppendOnCondition(cond, tbl, itm)
    if true == cond then
        return tableAppend(tbl, itm)
    else
        return tbl
    end 
end


-- ************************************************************************
---
--  popLastTableItem
--        @description  poping of last table item
--
--        @param tbl            - The table
--
--        @return       updated table
--
function popLastTableItem(tbl)
    if "table" == type(tbl) then
        local index, item
        local last_index    = nil

        for index, item in pairs(tbl) do
            last_index  = index
        end
       
        if nil ~= last_index    then
            tbl[last_index] = nil
        end
    end    
    
    return tbl
end


-- ************************************************************************
---
--  deepcopy
--        @description  making deep copy of a given table
--
--        @param object         - given table
--
--        @return       deepcopy of the object
--
function deepcopy(object)
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local new_table = {}
        lookup_table[object] = new_table
        for index, value in pairs(object) do
            new_table[_copy(index)] = _copy(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return _copy(object)
end


-- ************************************************************************
---
--  compareWithEtalonTable
--        @description  comparing two tables; compared table should have 
--                      all same fields like in etalon table
--
--        @param etalon         - etalon table
--        @param compared       - compared table
--
--        @return       deepcopy of the object
--
function compareWithEtalonTable(etalon, compared)
    if "table" == type(etalon) then 
      local index, value, value_type
    
      for index, value in pairs(etalon) do
        value_type = type(value)
        if     "table" == value_type then
          if false == compareWithEtalonTable(value, compared[index]) then
            return false
          end
        elseif (value_type == "number") or (value_type == "string") or (value_type == "boolean") then
          if value ~= compared[index] then
            return false
          end
        else
          return false
        end
      end
      return true
      
    else
      return etalon == compared
    end
end


-- ************************************************************************
---
--  sortNestedTable
--        @description  sorting of nested tables
--
--        @param tbl            - entry table
--        @param desc           - descrnding property (could be 
--                                irrelevant), by default - ascending
--
--        @return       sorted table
--
function sortNestedTable(tbl, desc)
    if "table" == type(tbl) then
        local index, value
        
        function tableItemComparator(a, b)
            if ("number" == type(a)) and ("number" == type(b)) then
                if nil == desc then
                    return a < b
                else
                    return a > b
                end
            else
                return false
            end
        end
        
        for index, value in pairs(tbl) do
            if      "table" == type(value)  then
                tbl[index]  = sortNestedTable(value)
            end
        end

        table.sort(tbl, tableItemComparator)
        
        return tbl
    else
        return {[1] = tbl}
    end
end


-- ************************************************************************
---
--  mergeTablesRightJoin
--        @description  megres nested two unordered tables. if same index 
--                      passing second table value will be taken
--
--        @param tbl1           - The first (left) table 
--        @param tbl2           - The second (right) table
--
--        @return       merging table
--
function mergeTablesRightJoin(tbl1, tbl2)
    local k, v
    
    if nil == tbl1 then
        tbl1 = {}
    end
    
    if nil == tbl2 then
        tbl2 = {}
    end
    
    for k, v in pairs(tbl2) do
        if type(v) == "table" then
                if type(tbl1[k] or false) == "table" then
                        mergeTablesRightJoin(tbl1[k] or {}, tbl2[k] or {})
                else
                        tbl1[k] = v
                end
        else
                tbl1[k] = v
        end
    end
    
    return tbl1
end


-- ************************************************************************
---
--  mergeOrderedTables
--        @description  megres nested two ordered tables: {0, 2, 5} \/ 
--                      {2, 7, 8} = {0, 2, 5, 7, 8}
--
--        @param tbl1               - The first table
--        @param tbl2               - The second table
--
--        @return       merged table
-- 
function mergeOrderedTables(tbl1, tbl2)
    local k, v
    
    function addValueToTbl1(v)
        local key = getTableItemKey(v, tbl)
        
        if nil ~= key then
            tbl1[key] = v
        else
            tableAppend(tbl1, v)
        end
    end
    
    if nil == tbl1 then
        tbl1 = {}
    end
    
    if nil == tbl2 then
        tbl2 = {}
    end
    
    for k, v in pairs(tbl2) do
        if type(v) == "table" then
                if type(tbl1[k] or false) == "table" then
                        mergeOrderedTables(tbl1[k] or {}, tbl2[k] or {})
                else
                        addValueToTbl1(v)
                end
        else
                addValueToTbl1(v)
        end
    end
    
    return tbl1
end


-- ************************************************************************
---
--  makeNestedTable
--        @description  makes array from indexes from first table and 
--                      second table as elements (if it is nil then empty 
--                      tables)
--
--        @param first_level    - first level array 
--        @param second_level   - second level array 
--
--        @return       nested table
--
function makeNestedTable(first_level, second_level)
    local index, item
    local second_index, second_item
    local result_array = {}
    
    if     nil == second_level then
        second_level = {}
    elseif "table" ~= type(second_level) then
        second_level = {[1] = second_level}
    end
    
    for index, item in pairs(first_level) do
        result_array[item] = mergeTablesRightJoin(result_array[item], 
                                                  second_level)
    end
    
    return result_array
end


-- ************************************************************************
---
--  clearNestedTable
--        @description  getting of table with the same structure of input 
--                      table but with empty arrays insted of nested arrays
--                      input:  {["a"] = {1, 2}, ["b"] = {4, 7, 8}, 5}
--                      output: {["a"] = {}, ["b"] = {}, 5}
--
--        @param tbl                - input table
--
--        @return       table with the same structure of input 
--                      table but with empty arrays insted of nested arrays
--
function clearNestedTable(tbl)
    local index, value
    local ret = {}
    
    if "table" ~= type(tbl) then
        tbl = {}
    end
    
    for index, value in pairs(tbl) do
        if "table" == type(value) then
            ret[index] = {}
        else
            ret[index] = value
        end
    end
    
    return ret
end


-- ************************************************************************
---
--  insertInNumberLists
--        @description  inserts the item in number list (eliminating 
--                      duplicates)
--
--        @param item           - inserted item
--        @param tbl            - list
--
--        @return       updated list
--
function insertInNumberLists(item, tbl)
    local index, value, pos
    
    if "table" ~= type(tbl)     then
        tbl = { [1] = item }
    end
    
    if "number" == type(item)   then
        pos = 0
        for index, value in pairs(tbl) do
            if "number" == type(value)      then
                if     value < item     then
                    pos = index
                elseif value == item    then
                    return tbl
                end
            end
        end  

        table.insert(tbl, pos + 1, item)        
    else
        table.insert(tbl, item)
    end
    
    return tbl
end


-- ************************************************************************
---
--  mergeLists
--        @description  Merges the both lists according to their values 
--                      (makes sure they are unique)
--
--        @param tbl1           - The first table
--        @param tbl2           - The second table
--
--        @return       a list with the unique content of both
--
function mergeLists(tbl1, tbl2)
    local ret, tmp
    local key, val

    tmp = {}
    ret = {}

    if (type(tbl1) == "table") then
        for key, val in pairs(tbl1) do
            tmp[val] = 1
        end
    end

    if (type(tbl2) == "table") then
        for key, val in pairs(tbl2) do
            tmp[val] = 1
        end
    end

    for key, val in pairs(tmp) do
        table.insert(ret, key)
    end

    return ret
end


-- ************************************************************************
---
--  mergeAndSortLists
--        @description  Merges the both lists according to their values 
--                      (makes sure they are unique) and sort it
--
--        @param tbl1               - The first table
--        @param tbl2               - The second table
--
--        @return       a sorted list with the unique content of both
-- 
function mergeAndSortLists(tbl1, tbl2)
    local ret = mergeLists(tbl1, tbl2)
    
    table.sort(ret)
    
    return ret
end


-- ************************************************************************
---
--  mergeMapOfLists
--        @description  Merges the both indexed tables of list according to 
--                      their values (makes sure they are unique) 
--
--        @param tbl1           - The first table
--        @param tbl2           - The second table
--
--        @return       a indexed table with the unique content of both
--
function mergeMapOfLists(tbl1, tbl2)
    local ret, tmp
    local key, val

    tmp = {}
    ret = {}

    if (type(tbl1) == "table") then
        for key, val in pairs(tbl1) do
            tmp[key] = 1
        end
    end

    if (type(tbl2) == "table") then
        for key, val in pairs(tbl2) do
            tmp[key] = 1
        end
    end

    for key, val in pairs(tmp) do
        ret[key] = mergeLists(tbl1[key], tbl2[key])
    end

    return ret
end


-- ************************************************************************
---
--  excludeList
--        @description  Remove the items that are in tbl2 from tbl1 and 
--                      returns what remains from tbl1 
--
--        @param tbl1           - The first table from which to exclude
--        @param tbl2           - The second table that contains the items 
--                                to exlude
--
--        @return       A list with the unique content of both
--
function excludeList(tbl1, tbl2)
    local ret, tmp, keys
    local key, val

    tmp = {}
    ret = {}

    if (type(tbl1) == "table") then
        for key, val in pairs(tbl1) do
            tmp[val] = key
        end
    end

    if (type(tbl2) == "table") then
        for key, val in pairs(tbl2) do
            tmp[val] = nil
        end
    end

    for key, val in pairs(tmp) do
        if nil ~= val then
            --ret[val] = key
            table.insert(ret, key)
        end
    end
    table.sort(ret)

    return ret
end


-- ************************************************************************
---
--  printStructure
--        @description  Recursievly prints the table's structure, do not 
--                      run it on the global environment or it will loop 
--                      forever (since _G is a member of _G)
--
--        @param str            - A prefix to append to all strings
--        @param tbl            - The table to print the structure of
--
--        @return       A list with the unique content of both
--
function printStructure(str, tbl)
    local n, v;

    if (type(tbl) ~= "table") then
        print(type(tbl) .. " -> " .. tostring(tbl))
        return
    end
    n, v = next(tbl, nil)  -- get first var and its value
    while n do
        if (type(v) == "table") then
            if (str == nil) then
                print(type(v) .. ":  " .. n)
                printStructure (n, v)
            else
                print(type(v) .. ":  " .. str .."." .. n)
                printStructure (str .. "." .. n, v)
            end
        else
            if (str == "") then print(type(v) .. ":  " .. n .. " = " .. tostring(tbl[n]))
            else print(type(v) .. ":  " .. str .."." .. n .. " = " .. tostring(tbl[n]))
            end
        end
        n, v = next(tbl, n)        -- get next global var and its value
    end
end


-- ************************************************************************
---
--  split
--        @description  Splits the string str according to pattern and 
--                      returns a list
--
--        @param str            - The string to be split
--        @param pattern        - The pattern according to which to slip
--
--        @usage __global       - __global["ifRange"]: iterface range 
--
--        @return       A list
--
function split(str, pattern)
    local s, e, substrS, substrE
    local ret

    substrS = 1
    e = 0
    ret = {}
    while 1 do
        tempE = e
        s, e = string.find(str, pattern, e + 1)
        if (type(s) == "nil") then
            break
        end
        if ((s - 1) >= 1) then
            table.insert(ret, string.sub(str, substrS, s - 1))
            substrS = e + 1
        end
    end
    if (type(s) == "nil") then
        if (tempE ~= string.len(str)) then
            table.insert(ret, string.sub(str, tempE + 1))
        end
    else
        print("ERROR: shouldn't be here\nstr is: " .. str .. " pattern is: " .. pattern .. "\n")
    end

    return ret
end


-- ************************************************************************
---
--  compareInit
--        @description  Compares the prefix of str1 to see if it is str2
--
--        @param str1           - The string to check
--        @param str2           - The possible prefix of the string
--
--        @return       0 if str2 is a prefix of str1
--
function compareInit(str1, str2)
    local s, e

    if (str1 == str2) then
        return 0, true
    end
    s, e = string.find(str1, str2, 1, true)
    if ((type(s) ~= "nil") and (s == 1)) then
        return 0
    end
    return 1
end


-- ************************************************************************
---
--  guesstoi
--        @description  Attempts to convert the string str to a number 
--
--        @param str            - The string to convert to a number
--
--        @return       The converted number
--
function guesstoi(str)
    tmp = tonumber(str)
    if (type(tmp) ~= "nil") then
        return tmp
    end
    return 0
end


setGlobal("maxNumberOfPorts", 64)


-- ************************************************************************
---
--  get_interlaced_tbl
--        @description  get tbale which is interlaced toward entry table 
--
--        @param tbl            - The table
--        @param sp             - The separator
--        @param first          - First item array offset (optional)
--        @param step           - Items step in array (optional)
--
--        @return       merged string
--
function get_interlaced_tbl(tbl, first, step)
    local index, ret
    local first_index = first
    local index_step  = step
       
    if nil == first_index then 
        first_index = 1
    end
    
    if nil == index_step then
        index_step = 1
    end
    
    if "table" == type(tbl) then
        if 0 < #tbl then     
            ret = { tbl[first_index] }
            
            for index = first_index + index_step, #tbl, index_step do
                tableAppend(ret, tbl[index])
            end
        else
            ret = { }
        end
    else
        ret = { }
    end
    
    return ret
end


-- ************************************************************************
---
--  checkSubStrings
--        @description  This function checks for sub strings in a a string, 
--                      str is the string , strMatches is of the form:
--                      checkParams={ {offset= (the offset) , 
--                                     subStr= (the string) }, 
--                                    {offset= (the offset) , 
--                                     subStr= (the string) }, etc...   }
--
--        @param str            - entry string
--        @param strMatches     - pattern
--
--        @return       operation succed
--
function checkSubStrings(str, strMatches)
    local i,v
    for i,v in pairs(strMatches) do
        if (string.sub(str,v.offset,v.offset+str.len(v.subStr)-1)==v.subStr)==false then return false end
    end
    return true
end


-- ************************************************************************
---
--  tbltostr
--        @description  merges all table items to entire string 
--
--        @param tbl            - The table
--        @param sp             - The separator
--        @param first          - First item array offset (optional)
--        @param step           - Items step in array (optional)
--
--        @return       merged string
--
function tbltostr(tbl, sp, first, step)
    local index, ret
    local first_index = first
    local index_step  = step
    
    if nil == sp            then
        sp = ', '
    end
    
    if nil == first_index then 
        first_index = 1
    end
    
    if nil == index_step then
        index_step = 1
    end
    
    if "table" == type(tbl) then
        if 0 < #tbl then
            ret = tbl[first_index]
            
            for index = first_index + index_step, #tbl, index_step do
                ret = ret .. sp .. tostring(tbl[index])
            end
        else
            ret = ""
        end
    else
        ret = ""
    end
    
    return ret
end


-- ************************************************************************
---
--  tblToKeyValueStr
--        @description  merges all table key/value pairs to entire string
--
--        @param tbl            - The table
--        @param sp             - The separator
--
--        @return       operation succed
--
function tblToKeyValueStr(tbl, sp)
    local ret = ""
    local key1, value1
    local key2, value2
    local first = true
    
    function addStrSeparator()
        if false == first then
            ret = ret .. sp
        else
            first = false
        end    
    end
    
    if "table" == type(tbl) then
        for key1, value1 in pairs(tbl) do
            if "table" == type(value1) then
                table.sort(value1)
                for key2, value2 in pairs(value1) do
                    addStrSeparator()
                    ret = ret .. tostring(key1) .. "/" .. tostring(value2) 
                end
            else
                addStrSeparator()
                ret = ret .. tostring(key1) .. "/" .. tostring(value1)             
            end
        end
    else
        ret = ""
    end
    
    if "" == ret then
        ret = "-"
    end
    
    return ret
end


-- ************************************************************************
---
--  tblToSortedKeyValueStr
--        @description  merges all table key/value pairs to entire string 
--                      after sorting of this nested table
--
--        @param tbl            - The table
--        @param sp             - The separator
--        @param desc           - descrnding property (could be 
--                                irrelevant), by default - ascending
--
--        @return       operation succed
--
function tblToSortedKeyValueStr(tbl, sp, desc)
    local sorted_tbl
    
    sorted_tbl = sortNestedTable(tbl, desc)
    
    return tblToKeyValueStr(sorted_tbl, sp)
end


-- ************************************************************************
---
--  tblToKeyValueStrRow
--        @description  merges all table key/value pairs to entire string 
--                      and format it 
--
--        @param tbl            - The table
--        @param sp             - The separator
--
--        @return       operation succed
--
function tblToKeyValueStrRow(tbl, sp, row_with, row_offset)
    return tblToKeyValueStr(tbl, sp)
end

-- ************************************************************************
---
--  printCallStack
--        @description  print call stack info , using debug.getinfo(level)
--
--        @param maxDepth       - (optional) to state depth of call stack
--                                  default is '5' 
--
--        @return       nil
--
function printCallStack(maxDepth)
    -- get the 'caller function info' !!!
    local stackIndex
    if maxDepth == nil then maxDepth = 5 end
    
    maxDepth = maxDepth + 2
    
    -- start from level 2 , as level 2 is 'my caller' (caller of function printCallStack(...))
    for stackIndex = 2 ,maxDepth do
        local debug_info = debug.getinfo(stackIndex)
--        print("call stack level : " .. tostring(stackIndex) , to_string(debug_info))
        printMsg("call stack level : " .. tostring(stackIndex) , to_string(debug_info))
        if (debug_info == nil) then
            break
        end
    end
    
    return nil
end

--------------------------------------------
-- CPSS type registration: GT_TRUNK_ID
--------------------------------------------
mgmType_to_c_GT_PORT_GROUPS_BMP     = mgmType_to_c_int
mgmType_to_lua_GT_PORT_GROUPS_BMP   = mgmType_to_lua_int
mgmType_to_c_GT_TRUNK_ID   = mgmType_to_c_int
mgmType_to_lua_GT_TRUNK_ID = mgmType_to_lua_int
mgmType_to_c_GT_DATA_PATH_BMP     = mgmType_to_c_int
mgmType_to_lua_GT_DATA_PATH_BMP   = mgmType_to_lua_int

