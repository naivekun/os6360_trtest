--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* myGenWrapper.lua
--*
--* DESCRIPTION:
--*       default CPSS wrappers
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants

-- local latestWrapperCmd = ""
local wrapperCmdCounter = 0
-- local myGenWrapperInFlag = false



-- traced API's data
local tracedApiCStyleName     = ""
local tracedApiCStyleVarDecl  = {}
local tracedApiCStyleVarAssgn = {}
local tracedApiCStyleVarList  = {}
local tracedApiCStyleVarXMLDecl  = {}
local tracedApiCStyleVarXMLList  = {}


local tracedApiCStyleNum = 0

local tempVarCounter = 0

-- trace buffer
local wrapperBufTable = {}


-- wrCallerName = ""
-- wrCallerLineNum = 0


-- ************************************************************************
---
--  createNewRecord
--    @description  create new empty record's in trace buffer 
--        
--
local function createNewRecord(callerLineNum)
  -- create new record
  wrapperBufTable[callerLineNum] = {}
  wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] = 0
  wrapperBufTable[callerLineNum]["tracedApiCStyleName"] = ""
  wrapperBufTable[callerLineNum]["isMainWrapper"] = false
  wrapperBufTable[callerLineNum]["tracedApiCStyleVarDecl"]  = {}
  wrapperBufTable[callerLineNum]["tracedApiCStyleVarAssgn"] = {}
  wrapperBufTable[callerLineNum]["tracedApiCStyleVarList"]  = {}
  wrapperBufTable[callerLineNum]["tracedApiCStyleVarValues"]  = {}
  wrapperBufTable[callerLineNum]["tracedApiCStyleVarXMLDecl"]  = {}
  wrapperBufTable[callerLineNum]["tracedApiCStyleVarXMLList"]  = {}
  wrapperBufTable[callerLineNum]["msg"] = {}
  wrapperBufTable[callerLineNum]["res"] = nil
  wrapperBufTable[callerLineNum]["resstr"] = nil
end


-- ************************************************************************
---
--  printModuleChangeMsg
--    @description  put message with new and old module's names in trace buffer
--        
--
local function printModuleChangeMsg(oldModuleName, newModuleName)
      printTrace("______________________________________________________________")
--      if string.len(wrCallerName)>1 and not endsWith(wrCallerName, "system_capabilities") then
      if string.len(oldModuleName)>1 then
        printTrace("Trace leave the module <<" .. oldModuleName .. ">>")
      end
--      if string.len(callerName)>1 and not endsWith(callerName, "system_capabilities") then
      if string.len(newModuleName)>1 then
        printTrace("Trace enter in the module <<" .. newModuleName  .. ">>")
      end
      printTrace("______________________________________________________________")
end

-- ************************************************************************
---
--  startAPILog
--    @description  put startAPI msg in trace buffer and update trace margins
--        
--
function startAPILog(...)
  local i, s
  local len
  
  local debuginfo = debug.getinfo(2)
  local callerName = debuginfo.source
  local callerLineNum = debuginfo.currentline

  -- save module name of previous call 
  local wrCallerName = wrapperBufTable["callerName"]
  if wrCallerName == nil then wrCallerName = "" end
  
  -- check module name in traceBuf
  if wrCallerName ~= callerName then
    flushTraceBuf()
    printModuleChangeMsg(wrCallerName, callerName)
    wrapperBufTable["callerName"] = callerName
    wrCallerName = callerName
  end

  -- check if tail_call 
  if callerLineNum < 1 then callerLineNum = #wrapperBufTable+1 end
  
  -- check if records already exists
  if wrapperBufTable[callerLineNum] ~= nil then
    return
  end

  createNewRecord(callerLineNum)

  table.insert(wrapperBufTable[callerLineNum]["msg"], "=============================================================================================")
  table.insert(wrapperBufTable[callerLineNum]["msg"],  tostring(callerName) .. ":" .. tonumber(callerLineNum))
  len = select("#", ...)
  for i= 1, len do
    s= select(i, ...)
    table.insert(wrapperBufTable[callerLineNum]["msg"], tostring(s))
  end
  table.insert(wrapperBufTable[callerLineNum]["msg"], "{")
end


-- ************************************************************************
---
--  leaveAPILog
--    @description  put leaveAPI msg in trace buffer and update trace margins
--        
--
function leaveAPILog(...)
  local i, s
  local len
  
  local debuginfo = debug.getinfo(2)
  local callerName = debuginfo.source
  local callerLineNum = debuginfo.currentline

  -- save module name of previous call 
  local wrCallerName = wrapperBufTable["callerName"]
  if wrCallerName == nil then wrCallerName = "" end
  
  -- check module name in traceBuf
  if wrCallerName ~= callerName then
    return
  end

  -- check if tail_call 
  if callerLineNum < 1 then callerLineNum = #wrapperBufTable+1 end
  
  -- check if records already exists
  if wrapperBufTable[callerLineNum] ~= nil then
    return
  end

  createNewRecord(callerLineNum)

  table.insert(wrapperBufTable[callerLineNum]["msg"], "return")
  table.insert(wrapperBufTable[callerLineNum]["msg"], "}")

  len = select("#", ...)
  for i= 1, len do
    s= select(i, ...)
    table.insert(wrapperBufTable[callerLineNum]["msg"], tostring(s))
  end
  table.insert(wrapperBufTable[callerLineNum]["msg"],  tostring(callerName) .. ":" .. tonumber(callerLineNum))
  table.insert(wrapperBufTable[callerLineNum]["msg"], "=============================================================================================")

end


-- ************************************************************************
---
--  startLoopLog
--    @description  put startLoop msg in trace buffer and update trace margins
--        
--
function startLoopLog(...)
  local i, s
  local len
  
  local debuginfo = debug.getinfo(2)
  local callerName = debuginfo.source
  local callerLineNum = debuginfo.currentline

  -- save module name of previous call 
  local wrCallerName = wrapperBufTable["callerName"]
  if wrCallerName == nil then wrCallerName = "" end
  
  -- check module name in traceBuf
  if wrCallerName ~= callerName then
    return
  end

  -- check if tail_call 
  if callerLineNum < 1 then callerLineNum = #wrapperBufTable+1 end
  
  -- check if records already exists
  if wrapperBufTable[callerLineNum] ~= nil then
    return
  end

  createNewRecord(callerLineNum)

  table.insert(wrapperBufTable[callerLineNum]["msg"], "=================================")
  table.insert(wrapperBufTable[callerLineNum]["msg"],  tostring(callerName) .. ":" .. tonumber(callerLineNum))

  len = select("#", ...)
  for i= 1, len do
    s= select(i, ...)
    table.insert(wrapperBufTable[callerLineNum]["msg"], tostring(s))
  end
  table.insert(wrapperBufTable[callerLineNum]["msg"], "{")
end


-- ************************************************************************
---
--  stopLoopLog
--    @description  put stopLoop msg in trace buffer and update trace margins
--        
--
function stopLoopLog(...)
  local i, s
  local len
  
  local debuginfo = debug.getinfo(2)
  local callerName = debuginfo.source
  local callerLineNum = debuginfo.currentline

  -- save module name of previous call 
  local wrCallerName = wrapperBufTable["callerName"]
  if wrCallerName == nil then wrCallerName = "" end
  
  -- check module name in traceBuf
  if wrCallerName ~= callerName then
    return
  end

  -- check if tail_call 
  if callerLineNum < 1 then callerLineNum = #wrapperBufTable+1 end
  
  -- check if records already exists
  if wrapperBufTable[callerLineNum] ~= nil then
    return
  end

  createNewRecord(callerLineNum)

  len = select("#", ...)
  for i= 1, len do
    s= select(i, ...)
    table.insert(wrapperBufTable[callerLineNum]["msg"], tostring(s))
  end
  table.insert(wrapperBufTable[callerLineNum]["msg"], "}")
  table.insert(wrapperBufTable[callerLineNum]["msg"],  tostring(callerName) .. ":" .. tonumber(callerLineNum))
  table.insert(wrapperBufTable[callerLineNum]["msg"], "=================================")
end


-- ************************************************************************
---
--  prepareValuesString
--        @description  parse and analyze string with values separated by vertical bar (|)
--        return prepared string and true if value iterated multiply times
--
local function prepareValuesString(inS)
  local w, i, s
  local t = {}
  
    -- split string and store values list in t
    for w in string.gmatch(inS, "[%w_]+") do
      if w~="" then
        table.insert(t,w) 
      end
    end

    -- if only one value in list
    if table.maxn(t)<2 then
      s = tostring(t[1]) .. ";"
      return s, false
    end

    -- compare values in list 
    local a = true
    for i = 2, #t do
      if t[1] ~= t[i] then a=false end
    end
    
    -- if all values in list the same
    if a then
      s = tostring(t[1]) .. ";"
      return s, false
    end

    s = tostring(t[1])
    for i = 2, #t do
      s = s .. "; " .. tostring(t[i])
    end

  return s, true
end

-- ************************************************************************
---
--  flushTraceBuf
--        @description  flush trace buffer
--        
--
function flushTraceBuf()

  local key, value
  local listOfEntry = {}
  local x = 0
  local callerName
  local callerLineNum

  if wrapperBufTable == nil then 
    return 
  end

  for key, value in pairs(wrapperBufTable) do
    -- printTrace("key " .. to_string(key))
    if type(key) == "number" then 
      table.insert(listOfEntry, key)
    end
  end


  local c=0
  for _ in pairs(listOfEntry) do c=c+1 end

  if c<1 then 
    wrapperBufTable = {}
    return 
  end

  table.sort(listOfEntry)


  callerName = wrapperBufTable["callerName"]
  
  -- iterate thru all logEntry in wrapperBufTable
  for key, value in pairs(listOfEntry) do

    callerLineNum = value

  
    if wrapperBufTable[callerLineNum]["isMainWrapper"] == true then
      -- print trace buf created by wrLogWrapper
  
      -- if this regular API's trace message
      printTrace("______________________________________________________________")
      printTrace(tostring(callerName) .. ":" .. tonumber(callerLineNum))
      printTrace(" ")
      printTrace(" Lua main wrapper " .. wrapperBufTable[callerLineNum]["tracedApiCStyleName"]    .. "     called " .. wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] .. " time(s)")
      printTrace(" ")
      printTrace(" ")

      -- store input data
      if #wrapperBufTable[callerLineNum]["tracedApiCStyleVarList"] > 0 then
        printTrace("Input :")
        for i=1, #wrapperBufTable[callerLineNum]["tracedApiCStyleVarList"] do
          printTrace( "    " .. wrapperBufTable[callerLineNum]["tracedApiCStyleVarList"][i] .. " = " .. to_string(wrapperBufTable[callerLineNum]["tracedApiCStyleVarValues"][i]))
        end
      else
        printTrace("Input = {}")
      end
  
      if wrapperBufTable[callerLineNum]["res"] ~= nil then
        printTrace("Output = " .. wrapperBufTable[callerLineNum]["res"])
      else
        printTrace("Output = {}")
      end
      printTrace(" ")
      printTrace("")
    else
      -- print trace buf created by myGenWrapper
    if #wrapperBufTable[callerLineNum]["msg"]>0 and wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] == 0 then
      -- if this message only
      for key, value in pairs(wrapperBufTable[callerLineNum]["msg"]) do
        printTrace(tostring(value))
      end
    else
      -- if this regular API's trace message
      printTrace("______________________________________________________________")
      printTrace(tostring(callerName) .. ":" .. tonumber(callerLineNum))
      printTrace(wrapperBufTable[callerLineNum]["tracedApiCStyleName"]    .. "     called " .. wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] .. " times")
      printTrace("result = " .. wrapperBufTable[callerLineNum]["res"] .. " (" .. wrapperBufTable[callerLineNum]["resstr"] .. ")")
  
      if #wrapperBufTable[callerLineNum]["msg"]>0 then
        printTrace("______________________________________________________________")
      end
      
      for key, value in pairs(wrapperBufTable[callerLineNum]["msg"]) do
        printTrace(tostring(value))
      end
  
  
      printTrace("______________________________________________________________")
    
      -- disable comments
    --  myCPSS_API_log_prefix = ""
    --  myCPSS_API_log_suffix = ""
    
      printTrace("static GT_STATUS cpssCallWrap" .. string.format("0x%05X", wrapperCmdCounter) )
      wrapperCmdCounter = wrapperCmdCounter+1
      printTrace("(")
      printTrace("    GT_VOID")
      printTrace(")")
      printTrace("{")
      printTrace("    GT_STATUS  rc;")
    
    
      local i
      -- print var declaration
      for i = 1, #wrapperBufTable[callerLineNum]["tracedApiCStyleVarXMLDecl"] do
          printTrace(wrapperBufTable[callerLineNum]["tracedApiCStyleVarXMLDecl"][i])
      end
    
      printTrace("")
      printTrace("")

     
      -- print var assignments
      for i = 1, #wrapperBufTable[callerLineNum]["tracedApiCStyleVarAssgn"] do
      
          local s, m = prepareValuesString(wrapperBufTable[callerLineNum]["tracedApiCStyleVarValues"][i])
      
          if (m and wrapperBufTable[callerLineNum]["tracedApiCStyleNum"]>1) then
            printTrace(wrapperBufTable[callerLineNum]["tracedApiCStyleVarAssgn"][i] .. s .. "  /* iterated ".. wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] .. " times */")
          else
            printTrace(wrapperBufTable[callerLineNum]["tracedApiCStyleVarAssgn"][i] .. s)
          end

          -- wrapperBufTable[callerLineNum]["tracedApiCStyleVarValues"][i]
      end
  
      printTrace("")
      printTrace("")
    
      -- print call with parameters
      printTrace("    rc = " .. wrapperBufTable[callerLineNum]["tracedApiCStyleName"] .. "(".. table.concat(wrapperBufTable[callerLineNum]["tracedApiCStyleVarXMLList"],", ") .. ");")
      printTrace("    return  rc;")
      printTrace("}")
      printTrace("")
      printTrace("")
    end
    end -- else if wrapperBufTable[callerLineNum]["isMainWrapper"] == true
  end  -- for key, value in pairs(listOfEntry) do

  wrapperBufTable = {}
end


-- ************************************************************************
---
--  getTraceBufStrings
--       @description  return postponed log about traces (if exists) 
--
--
function getTraceBufStrings()
   return false, nil
end


-- ************************************************************************
---
--  parseStructure
--        @description  Recursievly parsed the table's structure, do not
--                      run it on the global environment or it will loop
--                      forever (since _G is a member of _G)
--
--        @param str            - A prefix to append to all strings
--        @param tbl            - The table to print the structure of a
--                                list with the unique content of both
--
--        @return       A list with the unique content of both
--
local function parseStructure(str, tbl, idx)
    local n, v, tempStr;


--[[
    if (type(tbl) ~= "table") then
        if (tonumber(tbl)~=nil) then
            printTrace(string.format("0x%X",tbl))
            printTrace("111111111")
        else
            printTrace(tostring(tbl))
            printTrace("222222222")
        end
        return
    end
]]--

    n, v = next(tbl, nil)  -- get first var and its value
    while n do
        if (type(v) == "table") then
            if (str == nil) then
--                printTrace(n)
                parseStructure (n, v, idx)
            else
--                printTrace(str .."." .. n)
                parseStructure (str .. "." .. n, v, idx)
            end
        else
            if (tonumber(tbl[n])~=nil) then
                tempStr = string.format("0x%X",tbl[n])
            else
                tempStr = tostring(tbl[n])
            end

            if (str == "") then 
--              printTrace(n .. " = " .. tempStr)
            else 
--              printTrace(str .."." .. n .. " = " .. tempStr)
              -- add variable assignments to list
              if (tonumber(n) ~= nil) then
                table.insert(wrapperBufTable[idx]["tracedApiCStyleVarAssgn"], "    " .. str .."[" .. n .. "]".. " = " ) -- .. tempStr .. "; //")
                table.insert(wrapperBufTable[idx]["tracedApiCStyleVarValues"], tempStr)
              else
                table.insert(wrapperBufTable[idx]["tracedApiCStyleVarAssgn"], "    " .. str .."." .. n .. " = " ) -- .. tempStr .. ";  //")
                table.insert(wrapperBufTable[idx]["tracedApiCStyleVarValues"], tempStr)
              end
            end
        end
        n, v = next(tbl, n)        -- get next global var and its value
    end
end


-- ************************************************************************
---
--  parseParams
--        @description  fill traced API's data tables:
--              tracedApiCStyleVarDecl  = {}
--              tracedApiCStyleVarAssgn = {}
--              tracedApiCStyleVarList  = {}
--
--        @param params         - The parameters of the command
--
local function parseParams(params, idx)
    local k, param

--[[
    tracedApiCStyleVarDecl  = {}
    tracedApiCStyleVarAssgn = {}
    tracedApiCStyleVarList  = {}
]]--

    for k, param in pairs(params) do
      if (type(param[4]) ~= "nil" ) then

        if (type(param[4]) ~= "table") then
          
          -- not a table
          if (tonumber(param[4])~=nil) then
            -- print numeric value
--            printTrace("(" .. param[2] .. ") " .. param[3] .. " = " .. string.format("0x%X",param[4]))

            -- add variable name to varlist
            table.insert(wrapperBufTable[idx]["tracedApiCStyleVarList"], param[3])
            -- add variable declaration
            table.insert(wrapperBufTable[idx]["tracedApiCStyleVarDecl"], "    " .. param[2] .. "    " .. param[3] .. ";")
            -- add variable assignments
            table.insert(wrapperBufTable[idx]["tracedApiCStyleVarAssgn"], "    " .. param[3] .. " = " ) -- .. string.format("0x%X",param[4]) .. "; //")
            table.insert(wrapperBufTable[idx]["tracedApiCStyleVarValues"], string.format("0x%X", param[4]))
          else
            -- print symbolic value
--            printTrace("(" .. param[2] .. ") " .. param[3] .. " = " .. tostring(param[4]))
            
            -- add variable name to varlist
            table.insert(wrapperBufTable[idx]["tracedApiCStyleVarList"], param[3])
            -- add variable declaration
            table.insert(wrapperBufTable[idx]["tracedApiCStyleVarDecl"], "    " .. param[2] .. "    " .. param[3] .. ";")
            -- add variable assignments
            table.insert(wrapperBufTable[idx]["tracedApiCStyleVarAssgn"], "    " .. param[3] .. " = ") -- .. tostring(param[4]) .. "; //")
            table.insert(wrapperBufTable[idx]["tracedApiCStyleVarValues"], tostring(param[4]))
          end
          
        else
          -- parser for a table
--          printTrace("(" .. param[2] .. ") " .. param[3] .. " = ")

          -- add variable name to varlist
          table.insert(wrapperBufTable[idx]["tracedApiCStyleVarList"], param[3])
          -- add variable declaration
          table.insert(wrapperBufTable[idx]["tracedApiCStyleVarDecl"], "    " .. param[2] .. "    " .. param[3] .. ";")
          
          -- call table parser
          parseStructure (param[3], param[4], idx)

        end
        
      end
    end
--[[
        if "table"==type(param[4]) then	--if parameter is struct we should "fix" him (remove extra fields and add default fields)
            param[4]=cpssGenWrapperCheckParam(param[2], param[4])
        end
]]--
end

-- ************************************************************************
---
--  updateStructure
--        @description  Recursievly updateed the table's structure
--
--        @param str            - A prefix to append to all strings
--        @param tbl            - The table to print the structure of a
--                                list with the unique content of both
--                  Note:  used tempVarCounter globals
--        @return       A list with the unique content of both
--
local function updateStructure(str, tbl, idx)
    local n, v, tempStr;

    n, v = next(tbl, nil)  -- get first var and its value
    while n do
        if (type(v) == "table") then
            if (str == nil) then
--                printTrace(n)
                updateStructure (n, v, idx)
            else
--                printTrace(str .."." .. n)
                updateStructure (str .. "." .. n, v, idx)
            end
        else
            if (tonumber(tbl[n])~=nil) then
                tempStr = string.format("0x%X",tbl[n])
            else
                tempStr = tostring(tbl[n])
            end

            if (str == "") then 
--              printTrace(n .. " = " .. tempStr)
            else 
--              printTrace(str .."." .. n .. " = " .. tempStr)
              -- add variable assignments to list
              if (tonumber(n) ~= nil) then
                -- table.insert(wrapperBufTable[idx]["tracedApiCStyleVarAssgn"], "    " .. str .."[" .. n .. "]".. " = " .. tempStr .. ";")
                wrapperBufTable[idx]["tracedApiCStyleVarValues"][tempVarCounter] = wrapperBufTable[idx]["tracedApiCStyleVarValues"][tempVarCounter] .. "|" ..  tempStr
                tempVarCounter = tempVarCounter +1
              else
                -- table.insert(wrapperBufTable[idx]["tracedApiCStyleVarAssgn"], "    " .. str .."." .. n .. " = " .. tempStr .. ";")
                wrapperBufTable[idx]["tracedApiCStyleVarValues"][tempVarCounter] = wrapperBufTable[idx]["tracedApiCStyleVarValues"][tempVarCounter] .. "|" ..  tempStr 
                tempVarCounter = tempVarCounter +1
              end
            end
        end
        n, v = next(tbl, n)        -- get next global var and its value
    end
end

-- ************************************************************************
---
--  updateParsedParameters
--        @description          - add to tracedApiCStyleVarAssgn actual parameters
--        @param params         - The parameters of the command
--                                Note:  used tempVarCounter globals
--
local function updateParsedParameters(params, idx)
    local k, param
    for k, param in pairs(params) do
      if (type(param[4]) ~= "nil" ) then

        if (type(param[4]) ~= "table") then
          
          -- not a table
          if (tonumber(param[4])~=nil) then
            -- add variable assignments
            wrapperBufTable[idx]["tracedApiCStyleVarValues"][tempVarCounter] = wrapperBufTable[idx]["tracedApiCStyleVarValues"][tempVarCounter] .. "|" ..  string.format("0x%X",param[4])
            tempVarCounter = tempVarCounter +1
          else
            -- print symbolic value
            -- add variable assignments
            wrapperBufTable[idx]["tracedApiCStyleVarValues"][tempVarCounter] = wrapperBufTable[idx]["tracedApiCStyleVarValues"][tempVarCounter] .. "|" ..  tostring(param[4])
            tempVarCounter = tempVarCounter +1
          end
          
        else
          -- parser for a table
          -- call table updater
          updateStructure (param[3], param[4], idx)
        end
      end
    end
end


-- ************************************************************************
---
--  wrapperDebugInformationPrint
--        @description  parse API's name and input parameters information
--
--        @param cmd            - The executed command
--        @param params         - The parameters of executed command
--
--        @return       operation
--
--
local function wrapperDebugInformationPrePrint(cmd, params, callerName, callerLineNum)


  -- skip parsing for non-trace mode
  if not myCPSS_API_Debug then
    return
  end

  -- skip getters in "set-only" logging mode
  if loggingSetOnly and endsWith(cmd, "Get") then
    return
  end
  
  
  -- check if tail_call 
  if callerLineNum < 1 then callerLineNum = #wrapperBufTable+1 end
  
  
  -- check if records already exists
  if wrapperBufTable[callerLineNum] ~= nil then
  
    -- check for collisions
    if  wrapperBufTable[callerLineNum]["tracedApiCStyleName"] ~= cmd then
      table.insert(wrapperBufTable[callerLineNum]["msg"], "/* Trace collision detected in " .. to_string(callerName) .. ":" .. to_string(callerLineNum) .. " */")
      table.insert(wrapperBufTable[callerLineNum]["msg"],  "/* " .. to_string(wrapperBufTable[callerLineNum]["tracedApiCStyleName"]) .. " vs " .. to_string(cmd) .. " */")
      return
    end
    
    -- update call's counter
    wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] = wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] + 1
    
    if wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] < trace.getCounter() then
      tempVarCounter = 1
      updateParsedParameters(params, callerLineNum)
    end
    
    return
  end
  
  createNewRecord(callerLineNum)
  wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] = 1
  wrapperBufTable[callerLineNum]["tracedApiCStyleName"] = cmd

--      printTrace(">>>>>>>>")
  parseParams(params, callerLineNum)
--      printTrace(to_string(params))
--      printTrace("<<<<<<<<")


  -- scan cpss API by name over XML tree
  local cpssAPI_tag = cpssAPI_lookup(wrapperBufTable[callerLineNum]["tracedApiCStyleName"], false)
  if cpssAPI_tag == nil then
    table.insert(wrapperBufTable[callerLineNum]["msg"], "/* The " .. wrapperBufTable[callerLineNum]["tracedApiCStyleName"] .. " API's XML PARSER ERROR */")
    return nil 
  end


  local aparams = xml_lookup(cpssAPI_tag, "Params")
  
  -- parameters node not found
  if aparams == nil then
    return nil
  end

  local iterator = nil
  local index = 0
  repeat
    iterator = xml_getchild(aparams, iterator)
    if iterator == nil then
      break
    end
    if xml_tagname(iterator) == "Param" then
      
      -- add variable declaration
      table.insert(wrapperBufTable[callerLineNum]["tracedApiCStyleVarXMLDecl"], "    " .. 
                   string.format("%-35s", xml_get_attr(iterator, "type")) .. "    " .. 
                   string.format("%-30s", xml_get_attr(iterator, "name")) .. "; /* " .. 
                   xml_get_attr(iterator, "dir") .." */")
                   
      -- add parameter to varlist
      if (xml_get_attr(iterator, "dir")=="IN" and xml_get_attr(iterator, "class")=="struct") or 
         xml_get_attr(iterator, "dir")=="OUT" then
        
        table.insert(wrapperBufTable[callerLineNum]["tracedApiCStyleVarXMLList"], "&"..xml_get_attr(iterator, "name"))
      else
        table.insert(wrapperBufTable[callerLineNum]["tracedApiCStyleVarXMLList"], xml_get_attr(iterator, "name"))
      end
  end
  until iterator == nil

end


-- ************************************************************************
---
--  wrapperDebugInformationPostPrint
--        @description  print API's results
--
--        @param params         - The parameters of executed command
--        @param result         - Operation execution result
--
--        @return       operation
--
--
local function wrapperDebugInformationPostPrint(cmd, params, result, callerName, callerLineNum)
  
  -- skip printing for non-debug modes
  if not myCPSS_API_Debug then
    return
  end
  
  -- skip getters in "set-only" logging mode
  if loggingSetOnly and endsWith(cmd, "Get") then
    return
  end

  -- print only call's log in C- style 
  if myCPSS_API_log_in_C_Style ~= true then
    return
  end

  -- check if tail_call 
  if callerLineNum < 1 then callerLineNum = #wrapperBufTable end


  local b = returnCodes[result]
  if b == nil then b = "" end
  
  wrapperBufTable[callerLineNum]["res"] = result
  wrapperBufTable[callerLineNum]["resstr"] = b

end


-- ************************************************************************
---
local function do_cpssGenWrapper_checkAPI(cmd,params)
    if cpssAPI_lookup == nil then return nil end
    local cpssAPI_tag = cpssAPI_lookup(cmd, false)
    if cpssAPI_tag == nil then return nil end
    local aparams = xml_lookup(cpssAPI_tag, "Params")
    
    if aparams == nil then
        if #params > 0 then
            return "extra parameters passed"
        end
        return nil
    end
    local iterator = nil
    local index = 0
    repeat
        iterator = xml_getchild(aparams, iterator)
        if iterator == nil then
            break
        end
        if xml_tagname(iterator) == "Param" then
            index = index + 1
            local pp = params[index]
            if pp == nil then
                return "param #"..tostring(index).." missing"
            end
            if xml_get_attr(iterator, "dir") ~= pp[1] then
                return "param #"..tostring(index)..": wrong direction, "..pp[1].." expected"
            end
            local t = pp[2]
            local et = xml_get_attr(iterator, "type")
            if et ~= t then
                local c = xml_get_attr(iterator, "class")
                if c == "enum" or c == "bool" or c == "int" then
                    if t ~= "GT_U32" and t ~= "GT_U16" and t ~= "GT_U8" then
                        return "param #"..tostring(index)..": type doesn't match, "..et.." expected"
                    end
                else
                    if c == "struct" and xml_get_attr(iterator, "special") == "UNT" then
                        if string.find(t,"_UNT_",1,true) == nil then
                            return "param #"..tostring(index)..": type doesn't match, "..et.." (union) expected"
                        end
                    else
                        return "param #"..tostring(index)..": type doesn't match, "..et.." expected"
                    end
                end
            end
        end
    until iterator == nil
    if index ~= #params then
        return "extra parameter passed: #"..tostring(index+1)
    end
    return nil
end

-- ************************************************************************
---
local function cpssGenWrapper_checkAPI(cmd,params)
    local m = do_cpssGenWrapper_checkAPI(cmd,params)
    if m == nil then return end
    if type(m) ~= "string" then m = to_string(m) end
    print("*** checkAPI: bad params for "..cmd.."(): "..to_string(params)..": "..m)
end

check_genWrapper_API=false

-- ************************************************************************
---
--  myGenWrapper
--        @description  Wrapper for the generic wrapper designed to allow
---                     to add the CPSS debug printing capability
--
--        @param cmd            - The command to execute
--        @param params         - The parameters of the command
--        @param wrapLevel      - temporary wrapper adjustment level for logger
--            in case of call myGenWrapper not directly(over additional wrapper)
--
--        @return       result, values (the result of the command and the
--                      values that were returned)
--
function myGenWrapper(cmd, params, wrapLevel)
    local result, values

    if check_genWrapper_API then
        cpssGenWrapper_checkAPI(cmd,params)
    end

    local debuginfo
    if (wrapLevel == nil) then debuginfo = debug.getinfo(2)
    else debuginfo = debug.getinfo(2+wrapLevel)
    end
    
    local callerName = debuginfo.source
    local callerLineNum = debuginfo.currentline

--    if callerName == nil then callerName = "" end
    if callerLineNum == nil then callerLineNum=0 end
    
    -- save module name of previous call 
    local wrCallerName = wrapperBufTable["callerName"]
    if wrCallerName == nil then wrCallerName = "" end
    
    -- check module name in traceBuf
    if wrCallerName ~= callerName then
    
      flushTraceBuf()
      
      printModuleChangeMsg(wrCallerName, callerName)
      
      wrapperBufTable["callerName"] = callerName
      wrCallerName = callerName
      
    end

    -- set flag
    -- myGenWrapperInFlag = true

--    if not endsWith(callerName, "system_capabilities") --[[and (wrCallerLineNum > 0) ]]-- 
--    then
      wrapperDebugInformationPrePrint(cmd, params, callerName, callerLineNum)
      
--    end

    result, values = cpssGenWrapper(cmd, params)

    if type(result) == "string" then
        print("cpssGenWrapper(\"" .. cmd .. "\", ".. to_string(params) ..
              "): " .. result)
    end

--    if not endsWith(callerName, "system_capabilities") --[[and (wrCallerLineNum > 0) ]]--
--    then
      wrapperDebugInformationPostPrint(cmd, params, result, callerName, callerLineNum)
--    end

    -- reset flag
    -- myGenWrapperInFlag = false
    
    return result, values
end


-- ************************************************************************
---
--  wrLogWrapper
--        @description  Callback for main wrappers designed to logging lua calls
--                        from cpss
--        @param wrFunName         - string with name of main wrapper
--        @param wrParamList       - parameters string
--        ...                      - parameters list
--



function wrLogWrapper(wrFunName, wrParamList, ... )
  local len, i, d, tmpS

  local wrName, wrCommand
  local inValues={}
  local inNames={}
  local outValues


  -- if not in trace mode --> fast call
  if not myCPSS_API_Debug then
    return _G[wrFunName](...)
  end

  if wrParamList == nil then
      wrParamList = "()"
  end
  wrCommand = wrFunName .. wrParamList

  local wrlResult = {}

  local debuginfo, callerName, callerLineNum, wrCallerName


  -- call mainWrapper___________________________________________________________
  -- assert(loadstring("wrlResult={" .. wrCommand .. "}"))()
  local f = function (funcName, ...) wrlResult = { _G[funcName](...) } end
  pcall(f, wrFunName, ...)

  -- parse input values (if exists)
  inValues = { ... }

  -- parse wrapperName and names of values
  i=1
  for tmpS in string.gmatch(wrCommand, "[%w_]+") do
    if i ~= 1 then
      -- name of values
      table.insert(inNames, tmpS)
    else
      -- name of wrapper
      wrName = tmpS
    end
    i = i+1
  end

--[[ -- FOR DEBUG PURPOSES:
  print(">>>>______________________________________________")
  -- output values
  outValues = to_string(wrlResult)

  print("Call of " ..  wrName)

  if #inValues > 0 then
    print("Input:")
    for i=1, #inValues do
      print("   " .. inNames[i] .. " = " .. to_string(inValues[i]))
    end
  end


  if wrlResult ~= nil then
    print("Output:")
    for i=1, #wrlResult do
      print("[" .. tostring(i) .. "] = " .. to_string(wrlResult[i]))
    end
  end

  print("<<<<______________________________________________")
]]--


  --____________________________________________________________________________

  -- save callers parameters
  debuginfo = debug.getinfo(2)
  callerName = debuginfo.source
  callerLineNum = debuginfo.currentline

  -- save module name of previous call
  wrCallerName = wrapperBufTable["callerName"]
  if wrCallerName == nil then wrCallerName = "" end

  -- check module name in traceBuf
  if wrCallerName ~= callerName then

    flushTraceBuf()

    printModuleChangeMsg(wrCallerName, callerName)

    wrapperBufTable["callerName"] = callerName
    wrCallerName = callerName
  end

  -- check if tail_call
  if callerLineNum < 1 then callerLineNum = #wrapperBufTable+1 end

  -- check if records already exists
  if wrapperBufTable[callerLineNum] ~= nil then
    -- the record already exists
    -- check for collisions
    if  wrapperBufTable[callerLineNum]["tracedApiCStyleName"] ~= wrName then
      table.insert(wrapperBufTable[callerLineNum]["msg"], "/* Trace collision detected in " .. to_string(callerName) .. ":" .. to_string(callerLineNum) .. " */")
      table.insert(wrapperBufTable[callerLineNum]["msg"],  "/* " .. to_string(wrapperBufTable[callerLineNum]["tracedApiCStyleName"]) .. " vs " .. wrName .. " */")
    else
      -- update call's counter
      wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] = wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] + 1
      if wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] < trace.getCounter() then
        tempVarCounter = 1

        -- updateParsedParameters(params, callerLineNum)
      end
    end
  else
    -- create new record
    createNewRecord(callerLineNum)
    wrapperBufTable[callerLineNum]["tracedApiCStyleNum"] = 1
    wrapperBufTable[callerLineNum]["tracedApiCStyleName"] = wrName
    wrapperBufTable[callerLineNum]["isMainWrapper"] = true


    -- store input data
    if #inValues > 0 then
      for i=1, #inValues do
        table.insert(wrapperBufTable[callerLineNum]["tracedApiCStyleVarList"], inNames[i])
        table.insert(wrapperBufTable[callerLineNum]["tracedApiCStyleVarValues"], inValues[i])
      end
    end

    -- store output data
    if wrlResult == nil then
      wrapperBufTable[callerLineNum]["res"] = nil
    else
      wrapperBufTable[callerLineNum]["res"] = to_string(wrlResult)
    end
  end


  -- return results to caller
  if wrlResult == nil then
    return
  else
    return unpack(wrlResult)
  end
end
