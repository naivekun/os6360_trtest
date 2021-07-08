--********************************************************************************
--*              (c), Copyright 2013, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* execute_genwrapper_JSON.lua
--*
--* DESCRIPTION:
--*       run cpssGenWrapper with JSON interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

-- externs
if cli_C_functions_registered(
        "wraplReadJSON", -- debug
        "wraplXmodemReceive",
        "wraplXmodemSend",
        "wraplZlibCompress",
        "wraplZlibDecompress",
        "wraplMacIncrement",
        "wraplIpv4Increment",
        "wraplIpv6Increment")
then

JSON = (loadfile "common/lib/JSON.lua")()

-- do_sleep
-- Sleep function in seconds scale. 
-- input value can be floating point for mili/nano Sec resolution
-- for example:
--              do_sleep(0.002) - wait 2mSec
--
-- INPUT:
--          s      -    time to wait in seconds
--
-- OUTPUT:
--          retval -    GT_OK   - on success
--                      GT_FAIL - on error
local function do_sleep(s)
  return cpssGenWrapper("osTimerWkAfter",{{"IN","GT_U32","mils", s*1000}})
end

-- get_time
-- Get the current time in seconds scale. 
-- output will be with floating point for mili/nano Sec resolution
--
-- INPUT:
--          NONE
--
-- OUTPUT:
--          retval -    GT_OK   - on success
--                      GT_FAIL - on error
--          output -    elapsed time within a second
local function get_time()
    local retval, output
    retval, output = cpssGenWrapper("osTimeRT",{
            {"OUT","GT_U32","seconds"},
            {"OUT","GT_U32","nanoSeconds"}})
    output = output.seconds + (output.nanoSeconds / 1000000000)
    return retval, output
end

-- doJsonCompressedFunc
-- decode JSON compressed data sent from Xmodem and handle errors
--
-- INPUT:
--          func - function parameters table
--
-- OUTPUT:
--          retval - decoded JSON params received or string in case of an error
local function doJsonCompressedFunc(func)
    local params, retval, status
    params = wraplXmodemReceive(true)
    retval = {}
    repeat
        if type(params) ~= "string" then
            retval = { ctrl_error_ret = { retval=1, error="failed in wraplXmodemReceive(): "..to_string(params) } }
            break
        end
        params = wraplZlibDecompress(params)
        if type(params) ~= "string" then
            retval = { ctrl_error_ret = { retval=1, error="failed in wraplZlibDecompress(): "..to_string(params) } }
            break
        end
        status, params = pcall(function(params)
                return JSON:decode(params)
        end, params)
        if status == false then
            retval = { ctrl_error_ret = { retval=1, error="JSON decode failed: "..params } }
            break
        end
        status, retval = pcall(func, params)
        if status == false then
            retval = { ctrl_error_ret = { retval=1, error="exec failed: "..retval } }
        end
    until true
    wraplXmodemSend(wraplZlibCompress(JSON:encode(retval)))
end

-- table_get_value
-- search value reference in table by '.' separated path
--     PATH := MEMBERNAME [ '.' MEMBERNAME ]
--     MEMBERNAME := token [ '.' MEMBERNAME ]
--
-- For example:
--          vlanId
--          vlanInfo.vrfId
--
-- INPUT:
--          tbl   - table
--          field - field to be incremented
--
-- OUTPUT:
--          tbl,field  or nil
local function tbl_get_value_ref(tbl, field)
    if type(tbl) ~= "table" then
        return nil
    end
    repeat
        local f = string.find(field, ".",1,true)
        if f == nil then
            return tbl, field
        end
        local of = string.sub(field,1,f-1)
        field = string.sub(field,f+1)
        if type(tbl[of]) == nil then
            tbl[of] = {}
        end
        if type(tbl[of]) ~= "table" then
            return
        end
        tbl = tbl[of]
    until false
end


function tbl_get_value(tbl, field)
    tbl, field = tbl_get_value_ref(tbl, field)
    if tbl ~= nil then
        return tbl[field]
    end
end

function tbl_set_value(tbl, field, value)
    tbl, field = tbl_get_value_ref(tbl, field)
    if tbl ~= nil then
        tbl[field] = value
    end
end

-- get_input_value_ref
-- search value reference in genwrapper params by path
--     PATH := name [ '.' MEMBERNAME ]
--     MEMBERNAME := token [ '.' MEMBERNAME ]
--
-- For example:
--          vlanId
--          vlanInfo.vrfId
--
-- INPUT:
--          params - function params table
--          field - field to be incremented
--
-- OUTPUT:
--          tbl,field  or nil
--          use tbl[field] to R/W
local function get_input_value_ref(params, field)
    local name, k
    local f = string.find(field, ".",1,true)
    if f == nil then
        name = field
        field = nil
    else
        name = string.sub(field,1,f-1)
        field = string.sub(field,f+1)
    end
    for k = 1, #params do
        if params[k][3] == name then
            if field == nil then
                return params[k], 4
            end
            local struct = params[k][4]
            repeat
                f = string.find(field, ".",1,true)
                if f == nil then
                    return struct, field
                end
                local of = string.sub(field,1,f-1)
                field = string.sub(field,f+1)
                if type(struct[of]) ~= "table" then
                    return
                end
                struct = struct[of]
            until false
            return
        end
    end
end

local function var_set (name, initval)
  rawset(_G, name, initval)
end

local function var_get(name)
    local value = rawget(_G, name)
    if value == nil then
      return 12, "variable not declared"
    end
    return 0, value
end

-- defineVarJsonControl
-- define variable JSON contol function handler
--
-- INPUT:
--          outerTable - sequence of functions/controls to be executed
--
-- OUTPUT:
--          retval -    GT_OK   - on success
--                      GT_FAIL - on error
--          output -    output values in case of read operation 
local function defineVarJsonControl(outerTable)
    local retval, output
    local list = outerTable.list
    if type(list) == "table" then
    local i
        for i = 1, #list do
            item = list[i] -- item == single entry of list array
            var_set(item.name, item.value)
        end
    end
    return 0, nil
end

-- funcVarJsonControl
-- function call with pre defined variable JSON contol function handler

-- INPUT:
--          outerTable - function table including parameters
--
-- For example:
--              "ctrl_var_inc":{
--                "list": [
--                  {"name":"devNum_variable","value":{ "int":1}, 
--                  {"name":"ipv4_variable","value":{ "ipv4": "1.1.1.1"}
--                ]
--              }
--
--  value step could be: int, mac, ipv4, ipv6
--
-- OUTPUT:
--          retval -    GT_OK   - on success
--                      GT_FAIL - on error
--          output -    output values in case of read operation 
local function funcVarJsonControl(outerTable)
    local retval, output, var_value
    local list = outerTable.list
    local funcTable = outerTable.func_call
    if type(list) == "table" then
        local i
        for i = 1, #list do
            name = list[i].name -- single entry of list array
            var = list[i].var -- single entry of list array
            tbl, fld = get_input_value_ref(funcTable.ctrl_func.params, name)
            if type(tbl) == "table" and tbl ~= nil then
                retval, var_value = var_get(var)
                if retval ~= 0 then
                    return retval, var_value
                end
                tbl[fld] = var_value
                retval, output = recursiveJsonControlFunc(funcTable)
                if retval ~= 0 and retval ~= 27 then
                    return retval, { retval = retval, error="func var error - func call failed" } -- return error
                end
            else
                return 13, { retval = 13, error="func var error - name not found" } -- return error
            end
        end
    end
    return 0, output
end

-- varIncJsonControl
-- variable Increment JSON contol function handler

-- INPUT:
--          outerTable - function table including parameters
--
-- For example:
--              "ctrl_var_inc":{
--                "list": [
--                  {"name":"devNum_variable","value":{ "int":1}, 
--                  {"name":"ipv4_variable","value":{ "ipv4": "1.1.1.1"}
--                ]
--              }
--
--  step could be: int, mac, ipv4, ipv6
--
-- OUTPUT:
--          retval -    GT_OK   - on success
--                      GT_FAIL - on error
--          output -    output values in case of read operation 
local function varIncJsonControl(outerTable)
    local retval, ver_value, inc_value, name, value
    local list = outerTable.list
    if type(list) == "table" then
        local i
        for i = 1, #list do
            name = list[i].name
            value = list[i].value
            retval, var_value = var_get(name)
            if retval ~= 0 then
                return retval, var_value
            end
            if value["int"] ~= nil then
                inc_value = var_value + value["int"]
            elseif value["mac"] ~= nil then
                inc_value = wraplMacIncrement(var_value, value["mac"])
            elseif value["ipv4"] ~= nil then
                inc_value = wraplIpv4Increment(var_value, value["ipv4"])
            elseif value["ipv6"] ~= nil then
                inc_value = wraplIpv6Increment(var_value, value["ipv6"])
            else
                return 13, { retval = 13, error="increment error - type not found" } -- return error
            end
            var_set(name, inc_value)
        end
    end
    return 0, nil
end

-- repeatJsonControlFunc
-- Repeat JSON contol function handler
--
-- INPUT:
--          outerTable - parameters to be repeated (sequence of functions/controls
--
-- For example:
--              {
--                "ctrl_repeat": {
--                  	  "times": 5,
--                        "errHndl": "continue",
--                  	  "list"": [
--                                      "ctrl_func":
--                                      {
--                                      funcname= "...",
--                                      params= [
--                                          [ "IN", "GT_U8", "devNum", 0 ],
--                                          ...
--                                          ]
--                                      }, 
--                                      ...
--                      ]
--                  }
--              }
--
-- OUTPUT:
--          retval -    GT_OK   - on success
--                      GT_FAIL - on error
--          output -    output values in case of read operation 
local function repeatJsonControlFunc(outerTable)
    local i, j, retval, output
    local jsonOutput = {}
    local outputList={}
    local cnt = 0
    local act_times = 0
    local errHndl = outerTable.errHndl

    if type(outerTable) ~= "table" or outerTable.times == nil or outerTable.list == nil or outerTable.errHndl == nil then
        return 1, { retval = 1, error= "bad repeat call params" } -- return error
    end

    for i=0, ( outerTable.times - 1 ) do
        for j=1,#outerTable.list do 
            local list = outerTable.list[j]
            retval, output = recursiveJsonControlFunc(list)
            if (retval ~= 0 and retval ~= 27) then
                if errHndl == "stop" then
                    return retval, { retval = retval, error="repeat failed on iteration #" .. i  } -- return error
                elseif errHndl == "ignore" then
                else --errHndl == "continue" - default
                    outputList[cnt]=output
                    cnt = cnt + 1
                end
            elseif output ~= nil then
                outputList[cnt]=output
                cnt = cnt + 1
            end
        end
        act_times = act_times + 1
    end

    jsonOutput = { {ctrl_start_ret = { ctrl = "ctrl_repeat_ret" }}, {outputList},
                 {ctrl_complete_ret = { ctrl_repeat_ret = { retval = 0, act_times = act_times } } } }
    return 0, jsonOutput
end

-- timeJsonControl
-- Time JSON contol function handler
--
-- INPUT:
--          outerTable - sequence of functions/controls to be executed
--
-- OUTPUT:
--          retval -    GT_OK   - on success
--                      GT_FAIL - on error
--          output -    time measured
local function timeJsonControl(outerTable)
    local i, status, retval, start_time, stop_time, diff_time
    local jsonOutput = {}

    if type(outerTable) ~= "table" or outerTable.list == nil then
        return 1, { retval = 1, error="bad time control params"  } -- return error
    end
    retval, start_time = get_time()
    for i=1,#outerTable.list do 
        local item = outerTable.list[i]
        status, retval, output = pcall(function(item)
            return recursiveJsonControlFunc(item)
        end, item)
        if status == false or retval ~= 0 then
            return retval, { retval = retval, error= "JSON returned faile on iteration #" .. i .. " output: " .. output  } -- return error
        end
    end

    retval, stop_time = get_time()
    if ( start_time > stop_time ) or (start_time == nil) then
        return 11, { retval = 11, error="time control error"  } -- return error
    end
    diff_time = stop_time - start_time

    jsonOutput = { {ctrl_start_ret = { ctrl = "ctrl_time_ret" }}, {output},
                 {ctrl_complete_ret = { ctrl_time_ret = { time = diff_time } } } }
    return 0, jsonOutput
end

-- funcMatchJsonControl
-- function match JSON contol function handler
--
-- INPUT:
--          outerTable - sequence of functions/controls to be executed
--
-- OUTPUT:
--          retval -    GT_OK   - on success
--                      GT_FAIL - on error
--          output -    output values in case of read operation 
local function funcMatchJsonControl(outerTable)
    local retval, output, match = false
    local list = outerTable.list
    local funcTable = outerTable.func_call

    if type(outerTable) ~= "table" or outerTable.list == nil or outerTable.func_call == nil then
        return 1, { retval = 1, error="bad match function call params"  } -- return error
    end

    retval, output = recursiveJsonControlFunc(funcTable)
    if retval ~= 0 and retval ~= 27 then
        return retval, { retval = retval, error="ctrl function match error - func call failed"  } -- return error
    end

    if type(list) == "table" then
    local i, matchValue
        for i = 1, #list do
            item = list[i] -- item == single entry of list array
            matchValue = tbl_get_value(output.ctrl_func_ret.values,item.name)
            if matchValue ~= item.value then
                break
            end
            if i == #list then
               match = true
            end
        end
    end

    if( match == true ) then
        return retval, output
    else
        return 0, nil
    end
end

-- tableJsonControl
-- Table JSON contol function handler
--
-- INPUT:
--          outerTable - sequence of functions/controls to be executed
--
-- OUTPUT:
--          retval -    GT_OK   - on success
--                      GT_FAIL - on error
--          output -    output values in case of read operation 
local function tableJsonControl(outerTable)
    local retval, output, nextIdxStatus
    local list = outerTable.list
    local index = outerTable.index
    local bulkSize = outerTable.bulksize
    local table = outerTable.table
    local funcTable = outerTable.funcname
    local devNum = outerTable.devNum
    local numofmatches = 0
    local jsonOutput = {}
    local outputList={}

    if type(outerTable) ~= "table" or outerTable.list == nil or outerTable.table == nil or outerTable.funcname == nil 
        or outerTable.index == nil or outerTable.devNum == nil or outerTable.countonly == nil then
        return 1, { retval = 1, error="bad table control params"  } -- return error
    end
    
    repeat 
    nextIdxStatus, output = cpssGenWrapper("cpssDxChTableValidIndexGetNext",{
                                            {"IN","GT_U8", "devNum", devNum}, 
                                            { "IN", "CPSS_DXCH_CFG_TABLES_ENT", "table", table },  
                                            {"INOUT","GT_U32","entryIndexPtr", index}})
    if ( nextIdxStatus == 12 ) then
        break
    elseif ( nextIdxStatus ~= 0 ) then
        return retval, { retval = nextIdxStatus, error="ctrl function error - cpssDxChTableValidIndexGetNext failed" } -- return error
    end
    if table == "CPSS_DXCH_CFG_TABLE_VLAN_E" then
        funcTable.ctrl_func.params.vlanId = index
    else
        funcTable.ctrl_func.params.index = index
    end
    index = output.entryIndexPtr + 1
    retval, output = recursiveJsonControlFunc(funcTable)
    if retval ~= 0 and retval ~= 27 then
        return retval, { retval = retval, error="ctrl function error - func call failed" } -- return error
    end

    if type(list) == "table" then
    local i, matchValue
        for i = 1, #list do
            item = list[i] -- item == single entry of list array
            matchValue = tbl_get_value(output.ctrl_func_ret.values,item.name)
            if matchValue ~= item.value then
                break
            end
            if i == #list then
                if outerTable.countonly == false then
                    outputList[numofmatches]=output
                end -- else: countonly == true
                numofmatches = numofmatches + 1
            end
        end
    end

    until nextIdxStatus == 12

    jsonOutput = { {ctrl_start_ret = { ctrl = "ctrl_table_ret" }}, {outputList},
            {ctrl_complete_ret = { ctrl_table_ret = { retval = 0, table = table, numofmatches = numofmatches } } } }
    return 0, jsonOutput
end

-- funcCallJsonControl
-- function call contol handler
--
-- INPUT:
--          outerTable - "funcname" & "params" - required 
--                       "output" - optional [default = "outonly", "all", "erroronly", "never"]
--
-- OUTPUT:  ctrl_func_ret{}
--          retval      -    GT_OK   - on success
--                           GT_FAIL - on error
--          invalues    -    input values supplied to function      (optional - depends on "output" value)
--          values      -    output values returned from function
--          funcname    -    function name
--          
local function funcCallJsonControl(outerTable)
    local retval, values
    local jsonOutput = {}

    if type(outerTable) ~= "table" or outerTable.funcname == nil or outerTable.params == nil then
        return 1, { retval = 1, error="bad input function call params"  } -- return error
    end

    retval, values = cpssGenWrapper(outerTable.funcname, outerTable.params)

    if outerTable.output == "all" then
        jsonOutput = { funcname = outerTable.funcname, retval = retval, invalues=outerTable.params, values = values }
    elseif outerTable.output == "erroronly" then
        jsonOutput = { retval = retval }
    elseif outerTable.output == "never" then
        return retval,{}
    else -- outerTable.output == "outonly" - default
        jsonOutput = { funcname = outerTable.funcname, retval = retval, values = values }
    end
    return retval, { ctrl_func_ret = jsonOutput }
end



-- recursiveJsonControlFunc
-- JSON contol recursive function handler
--
-- INPUT:
--          control - JSON control to be execute (repeat, time, sleep, etc...)
--
-- OUTPUT:
--          retval -    GT_OK   - on success
--                      GT_FAIL - on error
--          values -    output values in case of read operation 
function recursiveJsonControlFunc(control)
    if control["ctrl_func"] ~= nil then
        return funcCallJsonControl(control["ctrl_func"])
    elseif control["ctrl_repeat"] ~= nil then
        return repeatJsonControlFunc(control["ctrl_repeat"])
    elseif control["ctrl_var"] ~= nil then
        return defineVarJsonControl(control["ctrl_var"])
    elseif control["ctrl_func_var"] ~= nil then
        return funcVarJsonControl(control["ctrl_func_var"])
    elseif control["ctrl_var_inc"] ~= nil then
        return varIncJsonControl(control["ctrl_var_inc"])
    elseif control["ctrl_func_match"] ~= nil then
        return funcMatchJsonControl(control["ctrl_func_match"])
    elseif control["ctrl_time"] ~= nil then
        return timeJsonControl(control["ctrl_time"])
    elseif control["ctrl_sleep"] ~= nil then
        return do_sleep(control["ctrl_sleep"].time)
    elseif control["ctrl_table"] ~= nil then
        return tableJsonControl(control["ctrl_table"])
    else
        return 1, "Error: control not supplied"
    end
end

local function execGenWrapperFunc(params)
    local retval, output
    local func=params.ctrl_func
    if type(params) ~= "table" or params == nil or  params.ctrl_func == nil 
                                or func.funcname == nil or func.params == nil then
       return { retval="bad params" ..to_string(params) }
    end
    retval, output = recursiveJsonControlFunc(params)
    return output
end

local function execGenWrapperUncFunc()
    local params, retval
    params=JSON:decode(wraplReadJSON())
    retval = execGenWrapperFunc(params)
    print(JSON:encode_pretty(retval))
end

-- execGenWrapper
-- Read JSON encoded structure { funcname=cpssAPIname, params=cpssGenWrapperParams }
-- Send return values encoded with JSON
-- This function can be called from luaCLI console
CLI_addCommand("exec", "execGenWrapper", {
    help = "run cpssGenWrapper with JSON interface",
    func = execGenWrapperUncFunc
})

-- execGenWrapperC
-- Read JSON encoded structure { funcname=cpssAPIname, params=cpssGenWrapperParams }
-- Send return values encoded with JSON
-- This function is working with JSON interface, compressed in/out over Zmodem
CLI_addCommand("exec", "execGenWrapperC", {
    help = "run cpssGenWrapper with JSON interface, compressed in/out over Zmodem",
    func = doJsonCompressedFunc,
    constFuncArgs = { execGenWrapperFunc }

})

CLI_addCommand("exec", "ptype", {
    help = "run cpssGenWrapper with JSON interface",
    func = function(params)
        local s = cpssGenWrapperCheckParam(params.typeName)
        print(JSON:encode_pretty(s))
    end,
    params = {{ type="values",
        { format="%s", name="typeName", help="type name" }
    }}
})

local function execJsonControlFunc()
    local jsonInput, retval, output
    jsonInput=JSON:decode(wraplReadJSON())
    if type(jsonInput) ~= "table" then
        output={ retval=1, error="bad array syntax" }
    elseif jsonInput == nil then
        output={ retval=1, error="bad params: "..to_string(jsonInput) }
    else
        retval, output = recursiveJsonControlFunc(jsonInput)
    end
    if retval ~= 0 and retval ~= 4 and retval ~= 27 then
        output = { ctrl_error_ret = output }
    end
    print(JSON:encode_pretty(output))
end

-- execJsonControl
-- Read JSON encoded structure { funcname=cpssAPIname, params=cpssGenWrapperParams }
-- Send return values encoded with JSON
-- This function can be called from luaCLI console
CLI_addCommand("exec", "execJsonControl", {
    help = "run execJsonControl with JSON interface",
    func = execJsonControlFunc
})

local function execJsonControlCFunc(jsonInput)
    local retval, output
    if type(jsonInput) ~= "table" then
        output={ retval=1, error="bad array syntax" }
    end
    retval, output = recursiveJsonControlFunc(jsonInput)
    if retval ~= 0 and retval ~= 4 and retval ~= 27 then
        output = { ctrl_error_ret = output }
    end
    return output
end

-- execJsonControl
-- Read JSON encoded structure { funcname=cpssAPIname, params=cpssGenWrapperParams }
-- Send return values encoded with JSON
-- This function is working with JSON interface, compressed in/out over Zmodem
CLI_addCommand("exec", "execJsonControlC", {
    help = "run execJsonControlCFunc with JSON interface, compressed in/out over Zmodem",
    func = doJsonCompressedFunc,
    constFuncArgs = { execJsonControlCFunc }
})


if cli_C_functions_registered("wrapl_cmdEventRun") then
local function execGaltisWrapperFunc(params)
    local retval, output
    if type(params) ~= "table" or type(params.i) ~= "string" then
        return { retval="bad params" }
    end
    CLI_execution_unlock()
    retval, output = wrapl_cmdEventRun(params.i, params.f);
    CLI_execution_lock()
    return {retval=retval,output=output}
end
CLI_addCommand("exec", "execGaltisWrapperC", {
    help = "run cmdEventRun, JSON, compressed input:={i=\"cpssInitSystem 29,1,0\",f=nil}",
    func = doJsonCompressedFunc,
    constFuncArgs = { execGaltisWrapperFunc }
})
end


-- Json custom functions
--
-- execJsonAddCustomFunc
--      { "name": "funcName"
--        "src": "function source" }
--
-- execJsonCallCustomFunc
--      { "name": "funcName",
--        "params": ..... }
--
--
-- example:
--    execJsonAddCustomFunc
--      { "name": "helloWorld",
--        "src":  "function(params)\n  return { output=\"Hello world!\", params=params}\nend" }
--
--    execJsonCallCustomFunc
--      { "name": "helloWorld",
--        "params": "one" }
--    will return:
--      { "output": "Hello world!",
--        "param": "one" }
local execJsonCustomFunctions = { }

local function execJson_addCustomFunc(params)
    if type(params) ~= "table" or type(params.name) ~= "string" or type(params.src) ~= "string" then
        return { retval = 4, output="Bad param" }
    end
    if execJsonCustomFunctions[params.name] ~= nil then
        return { retval = 27, output="Already defined" }
    end
    local f = loadstring("return "..params.src, params.name)
    local status, func = pcall(f)
    if status == false then
        return { retval = 1, output=func }
    end
    if type(func) ~= "function" then
        return { retval = 4, output="input is not a function source" }
    end
    execJsonCustomFunctions[params.name] = func
    return { retval = 0 }
end
local function execJson_callCustomFunc(params)
    if type(params) ~= "table" or type(params.name) ~= "string" then
        return { retval = 4, output="Bad param" }
    end
    if execJsonCustomFunctions[params.name] == nil then
        return { retval = 4, output="Bad function name" }
    end
    local status, retval = pcall(execJsonCustomFunctions[params.name], params.params)
    if status == false then
        return { retval = 1, output=retval }
    end
    return retval
end
CLI_addCommand("exec", "execJsonAddCustomFunc", {
    help = "Add custom function, JSON, compressed input:={name=\"funcName\", src=\"function(param)...end\"}",
    func = doJsonCompressedFunc,
    constFuncArgs = { execJson_addCustomFunc }
})
CLI_addCommand("exec", "execJsonCallCustomFunc", {
    help = "call custom function, JSON, compressed input:={name=\"funcName\" [, params=... ]}",
    func = doJsonCompressedFunc,
    constFuncArgs = { execJson_callCustomFunc }
})

end
