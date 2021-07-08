--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* config_file_support.lua
--*
--* DESCRIPTION:
--*       config file support functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("luaTaskDelay")  --in millisec

--constants
do
local CLI_filters_Cfg_Tbl = {"show ", "save ", "load ", "delete ", "search ", "list ", "find "}


-- ************************************************************************
---
--  config_file_load
--        @description  Load and execute commands from file
--
--        @param filename       - file name
--        @param params         - params[add]: if true adds the contents of
--                                the file to the history;
--                                params[silent]: if true only outputs
--                                error messages
--
--        @return       bool status
--
function config_file_load(filename, params)
    local fd, e,delayStart,delayEnd,delayTime
    fd, e = fs.open(filename)
    if fd == nil then
        print("failed to open file " .. filename .. ": "..e)
        return false
    end
    while true do
        local str, where
        str, e = fs.gets(fd)
        if str == nil then
            if e == "<EOF>" then
                break
            end
            fs.close(fd)
            print("error reading file: " .. e)
            return false
        end
        -- remove heading and trailing whitespaces
        str = string.gsub(str, "^%s*(.-)%s*$", "%1")
        -- handle contignous lines
        while string.sub(str,-1) == "\\" do
            str = string.gsub(string.sub(str,1,-2),"^%s*(.-)%s*$", "%1")
            local str1
            str1, e = fs.gets(fd)
            if str1 == nil then
                break
            end
            str = str .. " " .. string.gsub(str1, "^%s*(.-)%s*$", "%1")
        end

        -- remove comments
    where = string.find(str, "//")
    if where ~= nil then 
      str = string.sub(str, 0,where - 1)
            -- remove trailing whitespaces
            str = string.gsub(str, "^%s*(.-)%s*$", "%1")
    end
    
        -- special handling for CLIexit
        if str == "CLIexit" then
            CLIexit_state = true
            str = ""
        end
        -- skip empty strings and comments
        if str ~= "" and string.sub(str, 1, 2) ~= "--" then
            -- execute line:

            if (params["addToHistory"]~=nil) then
                table.insert(cmdLuaCLI_history,str)
            end

            --Find delay command
            delayStart,delayEnd,delayTime = string.find(string.lower(str),"delay%s+(%d+)")
            if ((delayStart==1) and (delayEnd==string.len(str)) and (delayTime~=nil)) then
                luaTaskDelay(delayTime)  --in milliseconds
            else
                if prefix_match("callCpssAPI ", str) then
                    local cpss_API_name = prefix_del("callCpssAPI ", str);
                    --local cpss_API_params
                    local params_str = ""
                    local state = 0
                    local str1
                    while true do
                        str1, e = fs.gets(fd)
                        if str1 == nil then
                            if e == "<EOF>" then
                                break
                            end
                            fs.close(fd)
                            print("error reading file: " .. e)
                            return false
                        end
                        if state == 0 then
                            if prefix_match("<<<PARAMS", str1) then
                                state = 1
                            end
                        else
                            if prefix_match(">>>", str1) then
                                break
                            end
                            params_str = params_str .. str1
                        end
                    end
                    e, str1 = pcall(loadstring("_cpss_API_params="..params_str))
                    if e then
                        if (params["silent"] == nil) then
                            print("Call "..cpss_API_name.."() with the following parameters:")
                            print(to_string(_cpss_API_params))
                        end
                        local result, values = myGenWrapper(cpss_API_name, _cpss_API_params)
                        if (params["silent"] == nil) then
                            print("result="..returnCodes[result])
                            print("values="..to_string(values))
                        end
                    else
                        print("Wrong parameters for "..str..": "..str1)
                    end
                    _cpss_API_params = nil
                elseif (params["silent"]==nil) then
                    print("Performing: "..str)
                    cmdLuaCLI_execute(str)

                elseif (string.match(str,"show")==nil) then  --if not silent mode and the command is not a show command - execute
                    cmdLuaCLI_execute(str)
                end
            end
        end
    end
    fs.close(fd)
    return true
end


-- ************************************************************************
---
--  copy_file
--        @description  copy file
--
--        @param srcname        - source
--        @param dstname        - destination
--
--        @return       bool status
--
function copy_file(srcname, dstname)
    if not fs.fileexists(srcname) then
        return false
    end
    fs.unlink(dstname)
    local fsrc, fdst, e
    fsrc, e = fs.open(srcname, "r")
    if fsrc == nil then
        print("failed to open file " .. srcname .. ": "..e)
        return false
    end
    fdst, e = fs.open(dstname, "w")
    if fdst == nil then
        print("failed to create file " .. dstname .. ": "..e)
        fs.close(fsrc)
        return false
    end
    while true do
        local data
        data = fs.read(fsrc, 1024);
        if data == nil or data == "" then
            break
        end
        fs.write(fdst, data)
    end
    fs.close(fsrc)
    fs.close(fdst)
    return true
end


-- ************************************************************************
---
--  config_file_clear
--        @description  clear a config file
--
--        @param filename       - file name
--
--        @return       bool status
--
local function config_file_clear(filename)
    fs.unlink(filename)
end


-- ************************************************************************
---
--  config_file_add_command
--        @description  add a command to config file
--
--        @param filename       - file name
--        @param command        - command
--
--        @return       bool status
--
local function config_file_add_command(filename, command)
    local fd, e
    fd, e = fs.open(filename, "a")
    if fd == nil then
        print("failed to open file " .. filename .. ": "..e)
        return false
    end
    fs.write(fd, command .. "\n")
    fs.close(fd)
    return true
end


-- ************************************************************************
---
--  config_file_remove_lines
--        @description  add a command to config file
--
--        @param filename       - file name
--        @param pattern        - pattern
--
--        @return       bool status
--
local function config_file_remove_lines(filename, pattern)
    if not fs.fileexists(filename) then
        return true
    end
    -- first create a backup file (copy)
    if not copy_file(filename, filename .. ".bak") then
        return false
    end
    -- now copy lines from bak file except matched
    local fd, e, fdbak
    fs.unlink(filename)
    fd, e = fs.open(filename, "w")
    if fd == nil then
        print("failed to create file " .. filename .. ": "..e)
        return false
    end
    fdbak, e = fs.open(filename .. ".bak", "r")
    if fdbak == nil then
        print("failed to open file " .. filename .. ".bak: "..e)
        fs.close(fd)
        return false
    end
    while true do
        local str
        str, e = fs.gets(fdbak)
        if str == nil then
            if e == "<EOF>" then
                break
            end
            fs.close(fd)
            fs.close(fdbak)
            print("error reading file: " .. e)
            return false
        end
        if string.match(str, pattern) == nil then
            fs.write(fd, str)
        end
    end
    fs.close(fd)
    fs.close(fdbak)


    return true
end


-- ************************************************************************
---
--  get_last_line_of_Cfg_Tbl
--        @description  gets number of last line of running config tanble
--                      and this line
--
--        @param filename       - name of file
--
--        @return       last line number and this line
--
function get_last_line_of_Cfg_Tbl()
    local last_line_position = 0
    local k, j, filter

    for k = #CLI_running_Cfg_Tbl, 1, -1  do
        filter = false
        for j = 1, #CLI_filters_Cfg_Tbl do
            if nil ~= string.match(CLI_running_Cfg_Tbl[k],
                                   CLI_filters_Cfg_Tbl[j])  then
                filter = true
                break
            end
        end

        if filter == false then
            last_line_position  = k;
            break;
        end
    end

    return last_line_position, CLI_running_Cfg_Tbl[last_line_position]
end


-- ************************************************************************
---
--  create_config_from_cfgTbl
--        @description  create config file from history
--
--        @param filename       - name of file
--
--        @return       bool status
--
function create_config_from_cfgTbl(filename)
    local number_of_first_showed_line   = 1

    if fs.fileexists(filename) then
        copy_file(filename, filename .. ".bak")
    end
    fs.unlink(filename)
    local fd, e
    fd, e = fs.open(filename, "w")
    if fd == nil then
        print("failed to open file " .. filename .. ": "..e)
        return false
    end

    -- Starting line number correction.
    if CLI_number_of_first_line_in_running_Cfg_Tbl <= #CLI_running_Cfg_Tbl  then
        if CLI_first_line_in_running_Cfg_Tbl ==
           CLI_running_Cfg_Tbl[CLI_number_of_first_line_in_running_Cfg_Tbl] then
            number_of_first_showed_line =
                CLI_number_of_first_line_in_running_Cfg_Tbl + 1
        end
    end

    local k, j, filter
    for k = number_of_first_showed_line, #CLI_running_Cfg_Tbl  do
        filter = false
        for j = 1, #CLI_filters_Cfg_Tbl do
            if nil ~= string.match(CLI_running_Cfg_Tbl[k],
                                   CLI_filters_Cfg_Tbl[j])  then
                filter = true
                break
            end
        end
        if filter==false then
            fs.write(fd, CLI_running_Cfg_Tbl[k] .. "\n")
        end
    end
    fs.close(fd)
    return true
end


--
-- config module registry table
--
-- here key is module name string, value is function
--
-- For example to register function save_config_fdb():
--     save_config_tbl["FDB configuration"] = save_config_fdb
--
local save_config_tbl = {}


-- ************************************************************************
---
--  save_config
--        @description  Save config to file (call all registered functions)
--
--        @param filename       - name of file
--
--        @return       bool status
--
local function save_config(filename)
    -- first create a backup file (copy)
    if fs.fileexists(filename) then
        copy_file(filename, filename .. ".bak")
    end
    fs.unlink(filename)
    local fd, e
    fd, e = fs.open(filename, "w")
    if fd == nil then
        print("failed to open file " .. filename .. ": "..e)
        return false
    end
    local k, v, status
    status = true
    for k,v in pairs(save_config_tbl) do
        if type(v) ~= "function" then
            print("wrong value in save_config_tbl[" .. string.format("%q",k) .. "]")
        else
            fs.write(fd,"-- " .. k .. "\n")
            if not v(fd) then
                status = false
            end
        end
    end
    fs.close(fd)
    return status
end

end
