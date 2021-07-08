--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* CLI.lua
--*
--* DESCRIPTION:
--*       lua CLI parser implementation
--*
--* FILE REVISION NUMBER:
--*       $Revision: 22 $
--*
--********************************************************************************


--
-- The following interface functions are defined here
--
--    CLI_type_dict             - value type descriptions
--    CLI_addParamDict()        - add item to parameter dictionary
--    CLI_addCommand()          - add command description
--    CLI_addHelp()             - add help for incomplete node
--    CLI_setmode()             - change current mode
--    CLI_change_mode_push()
--    CLI_change_mode_pop()
--    CLI_change_mode_pop_all()
--    CLI_mode_get()
--	  CLI_current_mode_get()
--    CLI_exec()
--    CLI_show_desc()
--    CLI_dont_add_to_running_config
--                              - command function should raise this
--                                flag to true to don't add exactly this
--                                command to CLI_running_Cfg_Tbl
--
--
--    add registration of type and parameter by the same name .
--    CLI_addParamDictAndType()         - generic.
--    CLI_addParamDictAndType_enum()    - for enum.
--    CLI_addParamDictAndType_number()  - for numbers
--    CLI_addParamDictAndType_numberRange() - for number range
--
--
--    CLI_check_param_number()  - common checker for numeric parameters
--    CLI_check_param_enum()
--    CLI_complete_param_enum()
--    CLI_check_param_C_enum()
--    CLI_complete_param_C_enum()
--    CLI_check_param_number_range() - range of numbers
--    CLI_complete_param_number_range()
--
--    duplicate()
--    prefix_match()
--    prefix_del()
--    to_string()
--    get_item_by_path()
--
-- cmdLua externs:
--    cmdLuaCLI_execute()
--    cmdLuaCLI_complete()
--    cmdLuaCLI_readLine_compl()
--    cmdLuaCLI_help()
--    cmdLuaCLI_prompt
--
--
-- global tables defined:
--    CLI_type_dict
--    CLI_prompts
--

do
CLI_type_dict = {}
CLI_prompts = {}
CLI_dont_add_to_running_config = false
-- locals
-- desc_tbl = {
--    [state] = { ["key"] = { next=new_state,
--                          node={
--                          },
--                          help="node help"
--                        }
local desc_tbl={ [1] = {}, [2] = { 
    -- allow to do: 'do command' from 'exec' context , for example:
    --(exec)# do clear bridge interface 0 all
    ["do"] = { next=2, help="Allows to execute command from the USER mode" }}--[[end of '2']]
}
-- mode_list = { ["key"] = state }
local mode_list = { all = 1, exec = 2 }
local desc_tbl_state_enum = 3
local param_dict = {}
local current_mode = ""
local current_state = 1
local test_mode = false
local default_help_offset = 19
local cr_string             = "<CR>"
CLI_mode_stack = {}

-- support of multiline commands
local savedPrompt = nil
local continiousString = nil

-- ************************************************************************
--  CLI_type_dict description
-- ************************************************************************
--  NOTE: CLI_type_dict[typename].checker can be 'table':
--      CLI_type_dict[typename].checker = { [1] = checker_a , [2] = checker_b ...}
--      if checker in the table state 'fail check' .. we continue to next checker...
--
--  CLI_type_dict[typename].checker(param,name,desc,varray,params)
--          A function which check parameter value and converts it to
--          application
--
--              param   - Parameter string
--              name    - parameter name (typename or .name)
--              desc    - == CLI_type_dict[typename]
--              varray  - previous values array
--              params  - partially filled params array
--
--              return       (true,value) or (false,error_string) if failed
--              can return third value: true if no more words in
--                      mutiword parameter
--
-- **********
--  NOTE: CLI_type_dict[typename].complete can be 'table':
--      CLI_type_dict[typename].complete = { [1] = complete_a , [2] = complete_b ...}
--      if complete in the table state 'fail complete' .. we continue to next complete...
--
--  CLI_type_dict[typename].complete(param,name,desc,varray,params)
--          A function which returns autocomplete alternatives and help
--
--              param   - Parameter string
--              name    - parameter name (typename or .name)
--              desc    - == CLI_type_dict[typename]
--              varray  - previous values array
--              params  - partially filled params array
--          
--              returns compl,help
--                  Where compl - array of complete alternatives
--                        help  - array of help (help[i] for compl[i])
--                                Also help.def for no values
--
-- ************************************************************************

--  CLI_type_dict_checker(entry_in_dictionary,typename, word, varray, params)
--          A function which check parameter value and converts it to
--          application
--
--        @param typename       - parameter type name
--        @param word           - command line word to check
--        @param desc    - == CLI_type_dict[typename]
--        @param varray  - previous values array
--        @param params  - partially filled params array
--
--        return       (true,value) or (false,error_string) if failed
--        can return third value: true if no more words in
--                mutiword parameter
local function CLI_type_dict_checker(entry_in_dictionary, typename, word, varray, params)
    if entry_in_dictionary == nil then
        -- no type defined, skip parsing
        return true, word
    end
    
    local d = entry_in_dictionary
    local checkerFunc = d.checker
    local name
    if d.name ~= nil then
        name = d.name
    else
        name = typename
    end
    
    if type(checkerFunc) == "table" then
        local myTable = checkerFunc
        local total_error_info = ""
        
        -- call for each of the checkers
        for ii,specific_checkerFunc in pairs(myTable) do
            if type(specific_checkerFunc) == "function" then
                local is_Ok,retValue2,retValue3,retValue4 = 
                    specific_checkerFunc(word, name, d, varray, params)
                    
                if is_Ok == true then
                    -- the current checker is OK with the parameter ... so check completed !
                    return is_Ok,retValue2,retValue3,retValue4
                end
                -- accumulate the 'errors' from the checkers , because if ALL fail.
                -- we will give the info to this function caller.
                total_error_info = total_error_info .. tostring(retValue2) .. "\n"
            else
                total_error_info = total_error_info .. "ERROR : expected 'checker function' at index [" .. to_string (ii) .. "]got:" .. to_string(myTable)
            end
        end
        
        -- we failed on all checker in the table
        return false , total_error_info
    end
    
    if type(checkerFunc) ~= "function" then
        -- no checker, skip parsing
        return true, word
    end
    return checkerFunc(word, name, d, varray, params)
end

--  CLI_type_dict_complete(entry_in_dictionary,typename, word, varray, params)
--          A function which returns autocomplete alternatives and help
--
--        @param typename       - parameter type name
--        @param word           - command line word to check
--        @param desc    - == CLI_type_dict[typename]
--        @param varray  - previous values array
--        @param params  - partially filled params array
--
--        returns compl,help
--            Where compl - array of complete alternatives
--                  help  - array of help (help[i] for compl[i])
--                          Also help.def for no values
local function CLI_type_dict_complete(entry_in_dictionary, typename, word, varray, params)
    if entry_in_dictionary == nil then
        -- no type defined, skip parsing
        return {}, {}
    end
    
    local d = entry_in_dictionary
    local completeFunc = d.complete
    local name
    if d.name ~= nil then
        name = d.name
    else
        name = typename
    end

    local compl, help
    
    local function mkhelp(str)
        if str == nil then
            return "("..typename..")"
        end
        return str
    end 

    if type(completeFunc) == "table" then
        local myTable = completeFunc
        local total_compl = {}
        local total_help = {}
        
        -- call for each of the checkers
        for ii,specific_completeFunc in pairs(myTable) do
            if type(specific_completeFunc) == "function" then
                compl, help = 
                    specific_completeFunc(word, name, d, varray, params)
                
                --print("compl" , to_string(compl))
                --print("help" , to_string(help))
                
                if "table" ~= type(compl)    then
                    tableAppend(total_compl, compl)
                else 
                    tableAppendToTable(total_compl,compl)
                end
                
                if "table" ~= type(compl)    then
                    tableAppend(total_help,help)
                else 
                    tableAppendToTable(total_help,help)
                end

                --print("compl" , to_string(total_compl))
                --print("help" , to_string(total_help))

            else
                --print("ERROR : expected 'complete function' at index [" .. to_string (ii) .. "]got:" .. to_string(myTable))
            end
        end
        
        
        compl, help = total_compl , total_help
        
    elseif type(completeFunc) ~= "function" then
        -- no checker, skip parsing
        return {}, { def=mkhelp(d.help) }
    else
        compl, help = completeFunc(word, name, d, varray, params)
    end
    
    --print("compl" , to_string(compl))
    --print("help" , to_string(help))

    if help.def == nil then
        help.def = mkhelp(d.help)
    end
    local i
    for i = 1, #compl do
        help[i] = mkhelp(help[i])
    end
    return compl, help
end




-- ************************************************************************
---
--  duplicate
--        @description  duplicate value (table)
--
--        @param val            - value to duplicate
--
--        @return       value copy
--
function duplicate(val)
    if type(val) ~= "table" then
        return val
    end
    local k, v, t
    t = {}
    for k,v in pairs(val) do
        t[k] = duplicate(v)
    end
    return t
end


-- ************************************************************************
---
--  prefix_match
--        @description  prefix matching checking
--
--        @param prefix         - prefix
--        @param str            - checked string
--        @param ignorecase     - case insensetive when true
--
--        @return       true, if string contains given prefix, otherwise
--                      false
--
function prefix_match(prefix, str, ignorecase)
    if ignorecase ~= true then
        if string.sub(str, 1, string.len(prefix)) == prefix then
            return true
        end
        return false
    end
    if string.lower(string.sub(str, 1, string.len(prefix))) == string.lower(prefix) then
        return true
    end
    return false
end


-- ************************************************************************
---
--  prefix_del
--        @description  deleting of prefix from string (if given string
--                      contains prefix)
--
--        @param prefix         - prefix
--        @param str            - processed string
--
--        @return       resulting string
--
function prefix_del(prefix,str)
    if prefix_match(prefix, str) then
        return string.sub(str, string.len(prefix)+1)
    end
    return str
end


-- ************************************************************************
---
--  to_string
--        @description  Convert value to string. This function can be used
--                      to print complex tables.
--
--        @param val            - converted value
--        @param level          - value numeric level
--        @param idx            - reserved
--
--        @return       string or nil, if could not to convert
--
function to_string(val, level, idx)
    if level == nil then
        level = 0
    end
    if val == nil then
        return "nil"
    elseif type(val) == "boolean" then
        if val then
            return "true"
        end
        return "false"
    elseif type(val) == "number" then
        return tostring(val)
    elseif type(val) == "string" then
        return string.format("%q",val)
    elseif type(val) == "table" then
        local indent = string.rep("  ",level+1)
        local s,k,v,not_first
        local short,is_short
        s = "{"
        short = "{"
        is_short = true
        not_first = false

        -- exception for GT_U64 -> print as hex number instead of table structure
        if val["l"] ~= nil and type(val["l"]) == "table" and val["l"][0] ~= nil then
          return string.format("0x%X", wrlCpssGtU64StrGet(val))
        end

        if val[0] ~= nil then
            s = s .. "\n" .. indent .. "[0]=" .. to_string(val[0],level+1,idx)
            if type(val[0]) ~= "table" then
                short = short .. " [0]=" .. to_string(val[0])
            else
                is_short = false
            end
            not_first = true
        end
        for k = 1, #val do
            if not_first then
                s = s .. ","
                short = short .. ","
            end
            not_first = true
            s = s .. "\n" .. indent
            short = short .. " "
            if idx then
                v = "["..tostring(k).."]="
                s = s .. v
                short = short .. v
            end
            s = s .. to_string(val[k],level+1,idx)
            if type(val[k]) ~= "table" then
                short = short .. to_string(val[k])
            else
                is_short = false
            end
        end
        for k,v in pairs(val) do
            if type(k) == "number" and k >= 0 and k <= #val then
            else
                is_short = false
                if not_first then s = s .. "," end
                not_first = true
                s = s .. "\n" .. indent
                if type(k) == "string" and string.match(k,"%a[%a%d_]*") == k then
                    s = s .. k .. "="
                else
                    s = s .. "[" .. to_string(k,level+1) .. "]="
                end
                s = s .. to_string(v,level+1,idx)
            end
        end
        if is_short and string.len(short) < 78 then
            return short .. " }"
        end
        s = s .. "\n" .. string.rep("  ",level) .. "}"
        return s
    elseif type(val) == "function" then
        return "<function>"
    elseif type(val) == "userdata" then
        return "<userdata>"
    elseif type(val) == "thread" then
        return "<thread>"
    else
        return "nil"
    end
end

-- ************************************************************************
---
--  get_item_by_path
--        @description  return table value by path where
--                      path is key or key_of_subtable.item_key
--
--        @param tbl    - table to get item from
--        @param path   - item path
--
--        @return       operation succed
--
function get_item_by_path(tbl,path)
    if type(tbl) ~= "table" then
        return nil
    end
    local f = string.find(path, ".",1,true)
    if f == nil then
        return tbl[path]
    end
    return get_item_by_path(
        tbl[string.sub(path,1,f-1)],
        string.sub(path,f+1))
end

local function in_table(tbl, key)
    local i
    for i = 1, #tbl do
        if tbl[i] == key then
            return true
        end
    end
end

-- ************************************************************************
---
--  modename_to_state
--        @description  return state, create new state if required
--
--        @param mode           - current mode
--        @param autocreate     - add new record if doesn't exists yet
--
--        @return       state
--
function modename_to_state(mode, autocreate)
    if mode == nil or mode == "" then
        return 1
    end
    if mode_list[mode] == nil and autocreate then
        mode_list[mode] = desc_tbl_state_enum
        desc_tbl_state_enum = desc_tbl_state_enum+1
    end
    if mode_list[mode] == nil then
        return 1
    end
    if type(desc_tbl[mode_list[mode]]) ~= "table" then
        desc_tbl[mode_list[mode]] = { }
    end
    return mode_list[mode]
end

desc_tbl[1]["do"] = { next=2, help="Allows to execute command from the USER mode" }


-- ************************************************************************
---
--  CLI_addParamDict
--        @description  add record to param_dict dictionaty
--
--        @param key            - key for references
--        @param param          - record to be added
--
--        @return       none
--
function CLI_addParamDict(key, param)
    if type(param) ~= "table" then
        error("param must be a table", 2)
    end
    if type(param[1]) == "table" then
        param_dict[key] = duplicate(param)
    else
        param_dict[key] = { duplicate(param) }
    end
end


-- ************************************************************************
---
--  CLI_show_desc
--        @description  Print desc_tbl table (debug)
--
function CLI_show_desc(what)
    local sep = "->"
    print("Showing ",what)
    if what == "desc_tbl" then
        print("desc_tbl="..to_string(desc_tbl,nil,true))
        return
    end
    local function print_desc(prefix, state)
        if type(desc_tbl[state]) ~= "table" then
            return
        end
        local k, v, c
        for k, v in pairs(desc_tbl[state]) do
            if type(v.node) == "table" then
                print(prefix .. k)
            end
            if v.next ~= nil and v.next ~= state then
                print_desc(prefix .. k .. sep,v.next)
            end
            if v.next == state then
                c = k
            end
        end
        if c ~= nil then
            for k, v in pairs(desc_tbl[state]) do
                if k ~= c then
                    if type(v.node) == "table" then
                        print(prefix .. c .. sep .. k)
                    end
                    if v.next ~= nil then
                        print_desc(prefix .. c .. sep .. k .. sep,v.next)
                    end
                end
            end
        end
    end
    if what == "all" then
        local k, v
        for k, v in pairs(mode_list) do
            if v ~= 1 then
                print(k..":")
                print_desc("    ",v)
            end
        end
    else
        if #CLI_mode_stack < 1 then
            print("desc_tbl="..to_string(desc_tbl,nil,true))
        else
            print_desc("", modename_to_state(CLI_mode_stack[#CLI_mode_stack]))
        end
    end
end


-- ************************************************************************
---
--  add_node
--        @description  add node to desc_tbl (local)
--
--        @param cmd            - command line splitted into words
--        @param node           - node to add
--        @param state          - add for this state (state == 1 means nil
--                                mode)
--
--        @return       added node reference
--
local function add_node(cmd, node, state)
    if state == nil then
        state = 1
    end
    local idx
    local ret
    for idx = 1,#cmd do
        if desc_tbl[state] == nil then
            desc_tbl[state] = {}
        end
        if desc_tbl[state][cmd[idx]] == nil then
            desc_tbl[state][cmd[idx]] = {}
        end
        if idx == #cmd then
            desc_tbl[state][cmd[idx]].node = duplicate(node)
            ret = desc_tbl[state][cmd[idx]].node
        else
            if desc_tbl[state][cmd[idx]].next == nil then
                desc_tbl[state][cmd[idx]].next = desc_tbl_state_enum
                desc_tbl_state_enum = desc_tbl_state_enum+1
            end
            state = desc_tbl[state][cmd[idx]].next
        end
    end
    return ret
end


-- ************************************************************************
---
--  check_params
--        @description  check params and return parsed params table (local)
--
--        @param node_params    - params to parse
--
--        @return       table to store parsed params to (n_params)
--
local function check_params(node_params, n_params)
    -- check params (if defined):
    -- 1.   n.params={}
    --
    -- 2.   params[].type == "values"
    -- 2.1. convert short form "%TYPENAME"
    -- 2.2. convert short form "#DICT_KEY"
    -- 2.3. otherwise assume a const string
    -- 2.4. if params[][].name==nil, set it to typename
    --
    -- 3.   params[].type == "named"
    -- 3.1. copy params[] attributes (mandatory, alt, order, etc)
    -- 3.2. convert short form "#DICT_KEY"
    -- 3.3. if params[][].name==nil set it
    -- 3.4. split .format into words
    -- 3.5. add ["name"] = { "word", "word2", ...} to alt   (auto alternatives)
    local i, j, jj, p
    for i = 1, #(node_params) do
        if node_params[i].type == "values" then
            -- 2.
            n_params[i] = { type="values" }
            jj = 1
            for j = 1,#(node_params[i]) do
                p = node_params[i][j]
                if type(p) == "string" then
                    if prefix_match("%", p) then
                        -- 2.1.
                        n_params[i][jj] = { format=p, name=prefix_del("%", p) }
                        jj = jj + 1
                    elseif prefix_match("@", p) then
                        -- 2.1.
                        n_params[i][jj] = { format=p, name=prefix_del("@", p) }
                        jj = jj + 1
                    elseif prefix_match("#", p) then
                        -- 2.2.
                        --p = prefix_sub("#", p)
                        p = string.sub(p, 2)--remove first char '#'
                        if param_dict[p] == nil then
                            error("Wrong dict reference: #"..p, 3)
                        end
                        p = param_dict[p]
                        for k = 1, #p do
                            n_params[i][jj]=duplicate(p[k])
                            jj = jj + 1
                        end
                    else
                        -- 2.3 const string
                        n_params[i][jj] = {format=p, name="const_"..p }
                        jj = jj + 1
                    end
                elseif type(p) == "table" then
                    n_params[i][jj] = duplicate(p)
                    jj = jj + 1
                else
                    n_params={}
                    error(string.format("wrong parameter #%d, #%d",i,j), 3)
                end
            end
            for j = 1, #(n_params[i]) do
                -- 2.4
                local fmt = n_params[i][j].format
                if fmt == nil then
                    n_params={}
                    error(string.format("No format defined for parameter #%d, #%d",i,j), 3)
                end
                --if not prefix_match("%", fmt) then
                --    n_params = {}
                --    error(string.format("Wrong format defined for parameter #%d, #%d",i,j), 3)
                --end
                if n_params[i][j].name == nil then
                    n_params[i][j].name = string.sub(fmt,2)
                end

            end
        elseif node_params[i].type == "named" then
            -- 3.
            n_params[i] = { type="named" }
            -- 3.1. copy p type="named"arams[] attributes (mandatory, alt, order, etc)
            for j, p in pairs(node_params[i]) do
                if type(j) ~= "number" then
                    n_params[i][j] = duplicate(p)
                end
            end
            -- 3.2. convert short form "#DICT_KEY"
            jj = 1
            for j =1,#(node_params[i]) do
                p = node_params[i][j]
                if type(p) == "string" then
                    if prefix_match("#",p) then
                        -- 3.2.
                        p = prefix_del("#", p)
                        if param_dict[p] == nil then
                            error("Wrong dict reference: #"..p, 3)
                        end
                        p = param_dict[p]
                        for k = 1, #p do
                            n_params[i][jj]=duplicate(p[k])
                            if k < #p then
                                if type(n_params[i][jj].format) ~= "string" then
                                    n_params={}
                                    error(string.format("wrong parameter #%d, #%d: format required",i,j), 3)
                                end
                                -- 3.4.
                                n_params[i][jj].format = splitline(n_params[i][jj].format)
                                if #(n_params[i][jj].format) < 1 then
                                    n_params={}
                                    error(string.format("empty format for parameter #%d, #%d",i,j), 3)
                                end
                            end
                            jj = jj + 1
                        end
                        if type(p.alt) == "table" then
                            local aa
                            if n_params[i].alt == nil then
                                n_params[i].alt = {}
                            end
                            for k, aa in pairs(p.alt) do
                                if n_params[i].alt[k] == nil then
                                    n_params[i].alt[k] = aa
                                end
                            end
                        end
                        if type(p.requirements) == "table" then
                            local aa
                            if n_params[i].requirements == nil then
                                n_params[i].requirements = {}
                            end
                            for k, aa in pairs(p.requirements) do
                                if n_params[i].requirements[k] == nil then
                                    n_params[i].requirements[k] = aa
                                end
                            end
                        end
                    else
                        n_params={}
                        error("wrong parameter: '"..p.."'", 3)
                    end
                elseif type(p) == "table" then
                    n_params[i][jj] = duplicate(p)
                    jj = jj + 1
                else
                    n_params={}
                    error(string.format("wrong parameter #%d, #%d",i,j), 3)
                end
                if type(n_params[i][jj-1].format) ~= "string" then
                    n_params={}
                    error(string.format("wrong parameter #%d, #%d: format required",i,j), 3)
                end
                -- 3.4.
                n_params[i][jj-1].format = splitline(n_params[i][jj-1].format)
                if #(n_params[i][jj-1].format) < 1 then
                    n_params={}
                    error(string.format("empty format for parameter #%d, #%d",i,j), 3)
                end
            end
            local auto_alt, k, v
            auto_alt = {}
            for j = 1, #(n_params[i]) do
                p = n_params[i][j]
                -- 3.3. if params[][].name==nil set it
                if p.name == nil then
                    p.name = p.format[1]
                end
                if auto_alt[p.name] == nil then
                    auto_alt[p.name] = { p.format[1] }
                else
                    table.insert(auto_alt[p.name], p.format[1])
                end
            end
            -- 3.5. add ["name"] = { "word", "word2", ...} to alt   (auto alternatives)
            for k,v in pairs(auto_alt) do
                if #v > 1 then
                    if n_params[i].alt == nil then
                        n_params[i].alt = {}
                    end
                    if n_params[i].alt[k] == nil then
                        n_params[i].alt[k] = v
                    end
                end
            end
        else
            error("wrong param section #"..tostring(i), 3)
        end
    end
end


-- ************************************************************************
---
--  CLI_addCommandSingleMode
--        @description  add new command description with single mode, do as
--                      many checks as possible
--
--        @param mode           - mode
--        @param cmd            - command name
--        @param node           - command description (parameters,
--                                function, etc)
--
--        @return       none or raise an error
--
function CLI_addCommandSingleMode(mode, cmd, node)
    local state, n
    -- let us see exact info to find problem quickly !!!
    local error_string = "\n" .. "mode = " .. to_string(mode) .. ", cmd = " .. to_string(cmd) .. ",node = " .. to_string(node) .. "\n"

    if type(mode) ~= "string" and mode ~= nil then
        error(error_string .. "Wrong 'mode' parameter, must be string", 2)
    end
    if type(cmd) ~= "string" then
        error(error_string .. "Wrong 'cmd' parameter, must be string", 2)
    end
    if type(node) ~= "table" then
        error(error_string .. "Wrong node description, must be a table", 2)
    end
    if type(node.func) ~= "function" then
        error(error_string .. "Wrong node description, 'func' not defined",2)
    end
    state = modename_to_state(mode, true)
    n = add_node(splitline(cmd), node, state)
    if type(node.params) == "table" then
        n.params={}
        check_params(node.params, n.params)
    end
end


-- ************************************************************************
---
--  CLI_addCommand
--        @description  add new command description, do as many checks as
--                      possible
--
--        @param modes          - modes
--        @param cmd            - command name
--        @param node           - command description (parameters,
--                                function, etc)
--
--        @return       none or raise an error
--
function CLI_addCommand(modes, cmd, node)
    if (nil == modes) or ("table" ~= type(modes)) then
        CLI_addCommandSingleMode(modes, cmd, node)
    else
        local index, mode

        for index, mode in pairs(modes) do
            CLI_addCommandSingleMode(mode, cmd, node)
        end
    end
end


-- ************************************************************************
---
--  CLI_addHelpSingleMode
--        @description  add help for incomplete command with single mode
--
--        @param mode         - mode
--        @param str          - incomlete command string
--        @param help         - help string
--
--        @return       none or raise an error
--
function CLI_addHelpSingleMode(mode, str, help)
    local state

    if type(mode) ~= "string" and mode ~= nil then
        error("Wrong 'mode' parameter, must be string", 2)
    end
    if type(help) ~= "string" then
        error("The help parameter must be a string " .. "\n mode=" .. to_string(mode) .. ",str=" .. to_string(str), 2)
    end
    state = modename_to_state(mode, true)
    local cmd = splitline(str)
    local idx
    for idx = 1,#cmd do
        if desc_tbl[state] == nil then
            desc_tbl[state] = {}
        end
        if desc_tbl[state][cmd[idx]] == nil then
            desc_tbl[state][cmd[idx]] = {}
        end
        if idx == #cmd then
            desc_tbl[state][cmd[idx]].help = help
        else
            if desc_tbl[state][cmd[idx]].next == nil then
                desc_tbl[state][cmd[idx]].next = desc_tbl_state_enum
                desc_tbl_state_enum = desc_tbl_state_enum+1
            end
            state = desc_tbl[state][cmd[idx]].next
        end
    end
end


-- ************************************************************************
---
--  CLI_addHelp
--        @description  add help for incomplete command
--
--        @param modes        - modes
--        @param str          - incomlete command string
--        @param help         - help string
--
--        @return       none or raise an error
--
function CLI_addHelp(modes, str, help)
    if (nil == modes) or ("table" ~= type(modes)) then
        CLI_addHelpSingleMode(modes, str, help)
    else
        local index, mode

        for index, mode in pairs(modes) do
            CLI_addHelpSingleMode(mode, str, help)
        end
    end
end


-- ************************************************************************
---
--  CLI_setCmdPrompt
--        @description  set's command prompt
--
--        @param prompt_prefix  - command prompt prefix, by default
--                                "Console"
--
function CLI_setCmdPrompt(prompt_prefix)
    if (nil == prompt_prefix) then
        prompt_prefix = getGlobal("hostname")

        if (nil == prompt_prefix) or (true == isEmptyStr(prompt_prefix)) then
            prompt_prefix = "Console"
        end
    end

    cmdLuaCLI_prompt = prompt_prefix .. CLI_prompts[current_mode] .. " "
end


-- ************************************************************************
---
--  CLI_setmode
--        @description  Change current mode, perform
--                      CLI_mode_<current_mode>_exit and
--                      CLI_mode_<mode>_enter functions
--
--        @param mode           - new mode
--
function CLI_setmode(mode)
    if mode == nil or mode == "" or mode == "all" then
        print("current mode cannot be set to nil")
        return false
    end
    if type(mode) ~= "string" then
        print("the CLI_setmode() param must be a string")
        return false
    end
    modename_to_state(mode, true)
    local f
    f = _G["CLI_mode_" .. current_mode .. "_exit"]
    if type(f) == "function" then
        pcall(f)
    end
    current_mode=mode
    current_state=modename_to_state(mode)
    if type(CLI_prompts[mode]) == "string" then
        CLI_setCmdPrompt()
    end
    if test_mode then cmdLuaCLI_prompt = "CLI_PROMPT# " end
    f = _G["CLI_mode_" .. current_mode .. "_enter"]
    if type(f) == "function" then
        pcall(f)
    end
    return true
end

-- ************************************************************************
---
--  CLI_change_mode_push
--        @description changing of cli mode
--
--        @param mode           - new mode
--
function CLI_change_mode_push(mode)
    local c = current_mode
    if CLI_setmode(mode) then
        table.insert(CLI_mode_stack, c)
    end
end


-- ************************************************************************
---
--  CLI_change_mode_push
--        @description returning to previus cli mode
--
function CLI_change_mode_pop()
    if #CLI_mode_stack < 1 then
        return
    end
    CLI_setmode(CLI_mode_stack[#CLI_mode_stack])
    table.remove(CLI_mode_stack, #CLI_mode_stack)
end


-- ************************************************************************
---
--  CLI_change_mode_pop_all
--        @description  clearing of cli mode stack
--
function CLI_change_mode_pop_all()
    if #CLI_mode_stack > 0 then
        CLI_mode_stack = { CLI_mode_stack[1] }
        CLI_setmode(CLI_mode_stack[1])
    else
        CLI_setmode("exec")
    end

end


-- ************************************************************************
---
--  CLI_mode_get
--        @description  getting of current CLI mode
--
--        @return       mode string
--
function CLI_mode_get()
    if #CLI_mode_stack > 0 then
        return CLI_mode_stack[#CLI_mode_stack]
    else
        return "exec"
    end
end

-- ************************************************************************
---
--  CLI_current_mode_get
--        @description  getting of (really) current CLI mode
--
--        @return       mode string
--
function CLI_current_mode_get()
    if #CLI_mode_stack > 0 then
        return current_mode
    else
        return "exec"
    end
end

-- ************************************************************************
---
--  parse_value
--        @description  check parameter value (local)
--
--        @param typename       - parameter type name
--        @param word           - command line word to check
--
--        @return       true, value
--                      false, error_string
--
local function parse_value(typename, word, varray, params)
    if not prefix_match("%", typename) then
        if typename == word then
            return true, true
        else
            return false, "const string \""..typename.."\" expected here"
        end
    end
    typename = prefix_del("%", typename)
    local d = CLI_type_dict[typename]
    
    return CLI_type_dict_checker(d, typename, word, varray, params)
--[[ --> old code before 'table' support for '.checker'
    if d == nil then
        -- no type defined, skip parsing
        return true, word
    end
    if type(d.checker) ~= "function" then
        -- no checker, skip parsing
        return true, word
    end
    local name = typename
    if d.name ~= nil then
        name = d.name
    end
    return d.checker(word, name, d, varray, params)
]]--    
    
end


-- ************************************************************************
---
--  complete_value
--        @description  do autocomplete for parameter value
--
--        @param typename       - parameter type name
--        @param word           - command line word to complete
--
--        @return       completion_table, help_table
--
local function complete_value(typename, word, varray, params)
    if not prefix_match("%", typename) then
        return { typename }, {}
    end
    typename = prefix_del("%", typename)
    local d = CLI_type_dict[typename]

    return CLI_type_dict_complete(d,typename, word, varray, params)
    
--[[    
    local d = CLI_type_dict[typename]
    if d == nil then
        -- no type defined, skip parsing
        return {}, {}
    end

    if type(d.complete) ~= "function" then
        -- no checker, skip parsing
        return {}, { def=mkhelp(d.help) }
    end
    local name = typename
    if d.name ~= nil then
        name = d.name
    end
    local compl, help = d.complete(word, name, d, varray, params)
    if help.def == nil then
        help.def = mkhelp(d.help)
    end
    local i
    for i = 1, #compl do
        help[i] = mkhelp(help[i])
    end
    return compl, help
]]--
end


-- Previus param positions.
local last_j, last_idx

-- ************************************************************************
---
--  clear_param_position
--        @description  clears parameter positions for using it in its
--                      check
--
--        @usage last_j         - previus param description table index
--        @usage last_idx       - previus command line parameters index
--
--        @return       operations succed
--
local function clear_param_position()
    last_j      = 0
    last_idx    = 0
end


-- ************************************************************************
---
--  check_param_position
--        @description  check's parameter position;
--                      asser's error, when param has wrong position
--
--        @param j              - param description table index
--        @param idx            - command line parameters index
--        @param no_saving      - positions saving flag
--
--        @usage last_j         - previus param description table index
--        @usage last_idx       - previus command line parameters index
--
--        @return       false   if param has right position;
--                      true    if param has wrong position
--
local function check_param_position(j, idx, no_saving)
    if nil == no_saving then
        no_saving = false
    end

    if getSign(j - last_j) == getSign(idx - last_idx) then
        if false == no_saving then
            last_j      = j
            last_idx    = idx
        end

        return false
    else
        return true
    end
end

local function parse_params_varray_complete(cmd, idx, n, typefmt, p, params)
    -- special case
    local compl = {}
    local help = {}
    local varray
    typefmt = "%"..prefix_del("@", typefmt)
    if params[n] == nil and p.allowempty then
        params[n] = {}
    end
    -- last word, complete
    local co, he, ii, jj
    varray = params[n]
    if varray == nil then
        varray = params[n.."_not_full"]
    end
    co, he = complete_value(typefmt, cmd[idx],varray,params)
    if p.help ~= nil then
        help.def = p.help
    end
    if he.def ~= nil then
        help.def = he.def
    end
    -- add if not in list yet
    for ii = 1, #co do
        local add = true
        if not p.allowduplicate then
            varray = params[n]
            valid, value = parse_value(typefmt, co[ii], varray, params)
            if type(varray) == "table" then
                for jj = 1, #varray do
                    if varray[jj] == value then
                        add = false
                    end
                end
            end
        end
        if add then
            table.insert(compl, co[ii])
            if he[ii] ~= nil then
                help[#compl] = he[ii]
            end
        end
    end
    if #compl == 0 and help.def == nil then
        help.def = table.concat(p.format, " ")
    end
    return idx-1, params, compl, help, idx
end


local function parse_params_varray(cmd, idx, n, typefmt, p, params)
    local varray
    local typefmt = "%"..prefix_del("@", typefmt)
    if params[n] == nil and p.allowempty then
        params[n] = {}
    end
    varray = params[n]
    if varray == nil then
        varray = params[n.."_not_full"]
    end
    local last
    valid, value, last = parse_value(typefmt, cmd[idx], varray, params)
    if valid then
        if varray == nil then
            varray = { value }
        else
            local ii
            if not p.allowduplicate then
                for ii = 1, #varray do
                    if varray[ii] == value then
                        error("The value "..cmd[idx].." already present in command line", 2)
                    end
                end
            end
            table.insert(varray, value)
        end
        if last == false then
            params[n] = nil
            params[n.."_not_full"] = varray
        else
            params[n] = varray
            params[n.."_not_full"] = nil
        end
        if last then
            params[n] = varray
            idx = idx + 1
            return idx
        end
    else
        if type(params[n]) == "table" then
            return idx
        end
        local hlp = ""
        if p.help ~= nil then
            hlp = ": "..p.help
        end
        error("Wrong parameter "..p.name.." of type "..p.format..": "..value..hlp, 2)
    end
    idx = idx + 1
    return idx
end

-- ************************************************************************
---
--  parse_params
--        @description  parse command parameters
--
--        @param cmd            - command line splitted into words
--        @param idx            - command line parameters index
--        @param node_params    - params description table
--        @param complete       - complete/help flag
--
--        @return       last_parsed_word_index, params_tbl, compl_tbl,
--                      help_tbl, last_parsed_parameter_index
--
local function parse_params(cmd, idx, node_params, complete)
    local compl = {}
    local help = {}
    local params = {}
    -- parse parameters
    if node_params == nil then
        return idx, {}, compl, help, idx
    end
    clear_param_position()
    for i = 1, #node_params do
        local n = node_params[i]
        if n.type == "values" then
            local j, p, valid, value
            for j = 1, #n do
                p = n[j]
                if prefix_match("@", p.format) then
                    while true do
                        if idx == #cmd and complete then
                            return parse_params_varray_complete(cmd, idx, n[j].name, p.format, p, params)
                        end
                        if idx > #cmd then
                            return idx, params, compl, help, idx
                        end
                        local nidx
                        nidx = parse_params_varray(cmd, idx, n[j].name, p.format, p, params)
                        if nidx == idx then
                            break
                        end
                        idx = nidx
                    end
                else
                    -- check params
                    -- just assing value
                    if idx == #cmd and complete then
                        -- last word, complete
                        compl, help = complete_value(p.format, cmd[idx],nil,params)
                        if help.def == nil and p.help ~= nil then
                            help.def = p.help
                        end
                        if #compl == 0 and help.def == nil then
                            help.def = table.concat(p.format, " ")
                        end
                        return idx-1, params, compl, help, idx
                    end
                    if idx > #cmd then
                        return idx-1, params, compl, help, idx
                    end
                    valid, value = parse_value(p.format, cmd[idx], nil, params)
                    if valid then
                        params[n[j].name]=value
                    else
                        local hlp = ""
                        if p.help ~= nil then
                            hlp = ": "..p.help
                        end
                        error("Wrong parameter "..p.name.." of type "..p.format..": "..value..hlp, 2)
                    end
                    idx = idx + 1
                end
            end
        end
        if n.type == "named" then
            -- do named parameters
            local in_section = true
            while in_section do
                local j, p, w
                in_section = false
                -- Search matched sentence
                for j = 1, #n do
                    p = n[j]
                    local allowed_now = true
                    -- if parameter already set don't add to alternatives
                    if params[p.name] ~= nil then
                        allowed_now = false
                    end
                    -- if altername parameter already set don't add to alternatives
                    if type(n.alt) == "table" then
                        local a, t, v
                        for a,t in pairs(n.alt) do
                            for v = 1, #t do
                                if t[v] == p.name then
                                    if params[a] ~= nil then
                                        allowed_now=false
                                    end
                                end
                            end
                        end
                    end
                    -- check requirements
                    if type(p.requirements) == "table" then
                        local t
                        for t = 1, #p.requirements do
                            if params[p.requirements[t]] == nil then
                                allowed_now = false
                            end
                        end
                    end
                    if type(n.requirements) == "table" then
                        if n.requirements[p.name] ~= nil then
                            local t
                            for t = 1, #n.requirements[p.name] do
                                if params[n.requirements[p.name][t]] == nil then
                                    allowed_now = false
                                end
                            end
                        end
                    end
                    if allowed_now or not complete then
                        local was_value=false
                        w = 1
                        while w <= #p.format do
                            if idx + w - 1 == #cmd and complete then
                                -- last word and complete mode
                                if prefix_match("%", p.format[w]) then
                                    compl, help = complete_value(p.format[w], cmd[idx+w-1],nil,params)
                                    if #compl == 0 and cmd[idx+w-1] ~= "" then
                                        local valid, value =
                                            parse_value(p.format[w], cmd[idx+w-1], nil, params)
                                        if false == valid then
                                            local hlp = ""
                                            if p.help ~= nil then
                                                hlp = ": "..p.help
                                            end
                                            error("Wrong parameter " .. p.name ..
                                                  " of type "..p.format[w]..": "..value..hlp, 2)
                                        end
                                    end
                                    if help.def == nil and p.help ~= nil then
                                        help.def = p.help
                                    end
                                    if #compl == 0 and help.def == nil then
                                        help.def = table.concat(p.format, " ")
                                    end
                                    return idx+w-1, params, compl, help, idx
                                end
                                if prefix_match("@", p.format[w]) then
                                    return parse_params_varray_complete(cmd, idx+w-1, p.name, p.format[w], p, params)
                                end
                                if false == check_param_position(j, idx + 1, true) then
                                local compl_key = getItemTableKey(p.format[w],
                                                                  compl)
                                if nil == compl_key then
                                table.insert(compl, p.format[w])
                                else
                                    local j1, p1, w1
                                    for j1 = 1, j - 1 do
                                        p1  = n[j1]
                                        if p1.format[w] == compl[compl_key] then
                                            compl[compl_key] =
                                                p1.format[w] .. " " ..
                                                p1.format[w + 1]
                                        end
                                    end
                                    table.insert(compl, p.format[w] .. " " ..
                                                        p.format[w + 1])
                                end
                                if p.help ~= nil then
                                    help[#compl] = p.help
                                end
                                    if isItemInTable(p.name, n.mandatory) then
                                        return idx, params, compl, help, idx
                                    end
                                else
                                    if isItemInTable(p.name, n.mandatory) then
                                        error("Mandatory parameter " .. p.name ..
                                              " is missing", 2)
                                    end
                                end
                                break
                            end
                            if idx + w - 1 > #cmd then
                                break
                            end
                            local param_completed
                            if prefix_match("%", p.format[w]) then
                                local valid, value = parse_value(p.format[w],cmd[idx+w-1],nil,params)
                                local nm
                                nm = p.name
                                if type(n.alt) == "table" and n.alt[nm] ~= nil then
                                    nm = p.format[1]
                                    if params[p.name] ~= nil and params[p.name] ~= nm then
                                        error("Parameter " .. nm .. " conflicts " ..
                                              " with " .. params[p.name], 2)
                                    end
                                end
                                if valid then
                                    was_value = true
                                    if params[nm] ~= nil and p.multiplevalues then
                                        if type(params[nm]) ~= "table" then
                                            if check_param_position(j, idx) then
                                                error("The parameter ".. nm ..
                                                      " of type " .. p.format[w] ..
                                                      " = ".. params[nm] ..
                                                      " violates the parameter " ..
                                                      "order", 2)
                                            end
                                            params[nm] = { params[nm] }
                                        end
                                        table.insert(params[nm],value)
                                    else
                                        if check_param_position(j, idx) then
                                            error("The parameter ".. nm ..
                                                  " of type " .. p.format[w] ..
                                                  " violates the parameter " ..
                                                  "order", 2)
                                        end
                                        params[nm]=value
                                        if p.multiplevalues then
                                            params[nm.."_"..prefix_del("%",p.format[w])] = value
                                        end
                                        if (idx + w == #cmd)    and
                                           (2 < #p.format)      then
                                            local format_param =
                                                tostring(p.format[w + 1])
                                            table.insert(compl, format_param)
                                            if p["help_" .. format_param]
                                                                    ~= nil then
                                                help[#compl] =
                                                    p["help_" .. format_param]
                                            elseif p.help ~= nil then
                                                help[#compl] =
                                                    p.help .. ' (' ..
                                                    format_param .. ')'
                                            else
                                                help[#compl] = format_param
                                            end
                                        end
                                    end
                                else
                                    local hlp = ""
                                    if p.help ~= nil then
                                        hlp = ": "..p.help
                                    end
                                    error("Wrong parameter "..nm.." of type "..p.format[w]..": "..value..hlp, 2)
                                end
                                param_completed = (w == #p.format)
                            elseif prefix_match("@", p.format[w]) then
                                local nidx
                                nidx = parse_params_varray(cmd, idx+w-1, p.name, p.format[w], p, params)
                                if nidx == idx+w-1 then
                                    idx = idx + w
                                else
                                    if nidx > #cmd then
                                        idx = nidx
                                    else
                                        w = w - 1
                                        idx = idx + 1
                                    end
                                end
                            else
                                if cmd[idx+w-1] ~= p.format[w] then
                                    break
                                end
                                param_completed = (w == #p.format)
                            end
                            if param_completed then
                                if type(n.alt) == "table" then
                                    local a, t, v
                                    for a,t in pairs(n.alt) do
                                        if a == p.name and params[a] ~= nil then
                                            error("Parameter "..p.name.." conflicts with "..params[a], 2)
                                        end
                                        for v = 1, #t do
                                            if t[v] == p.name then
                                                if params[a] ~= nil then
                                                    error("Parameter "..p.name.." conflicts with "..params[a], 2)
                                                end
                                                if check_param_position(j, idx) then
                                                    error("5. The parameter ".. a ..
                                                          " violates the " ..
                                                          "parameter order", 2)
                                                end
                                                params[a] = p.name
                                            end
                                        end
                                    end
                                end
                                if was_value and type(n.alt) == "table" then
                                    if n.alt[p.name] ~= nil then
                                        if check_param_position(j, idx) then
                                            error("6. The parameter ".. p.name ..
                                                  " violates the " ..
                                                  "parameter order", 2)
                                        end
                                        params[p.name] = p.format[1]
                                    end
                                elseif not was_value then
                                    if check_param_position(j, idx) then
                                        error("7. The parameter ".. p.name ..
                                              " violates the " ..
                                              "parameter order", 2)
                                    end
                                    params[p.name] = p.format[1]
                                end
                                idx = idx + w
                                if not n.ordered then
                                    in_section = true
                                end
                            end
                            w = w + 1
                        end -- search sentence cycle
                        if in_section then
                            -- sentence matched,
                            -- start from begin for other sentences from this section
                            break
                        end
                    end
                end
            end
        end
    end
    return idx, params, compl, help, idx
end


-- ************************************************************************
---
--  CLI_mandatory_and_complete_param_check
--        @description  check's mandatory parameters conflicts and
--                      parameters correctness and completness
--
--        @param cmd            - command line splitted into words
--        @param idx            - command line parameters index
--        @param params         - command parameters
--        @param node           - params description table
--        @param suppress_output
--                              - output suppressing option
--
--        @return       true, if success, otherwise false
--
function CLI_mandatory_and_complete_param_check(cmd, idx, params, node, suppress_output)

    local cmd_len   = #cmd
    if "" == cmd[#cmd]  then
        cmd_len = cmd_len - 1
    end
    if idx <= cmd_len and 
        not node.extraParametersAllowed and 
        not node.first_optional -- relevant only to type == "values"
    then
        if true ~= suppress_output then
            print("Wrong or incomplete option: "..table.concat(cmd, " ",idx))
            if test_mode then print "###ERR#SYNTAX" end
        end
        return false
    end

    local node_params
    node_params = node.params
    if type(node_params) == "table" then
        local i, j
        for i = 1, #node_params do
            if node_params[i].type == "values" then
                local parameter_is_optional = false
                for j = 1, #node_params[i] do
                    local name = node_params[i][j].name
                    if node.first_optional == name then -- relevant only to type == "values"
                        -- if we got here meaning that from this parameter all 'values' are 'optional' 
                        -- including this one !
                        parameter_is_optional = true
                    end
                    if params[name] == nil and 
                        (parameter_is_optional == false) -- do not fail command if 'parameter is optional'
                    then
                        if true ~= suppress_output then
                            print("Mandatory parameter " .. name ..
                                  " is missing")
                        if test_mode then print "###ERR#SYNTAX" end
                        end
                        return false
                    end
                end
            elseif node_params[i].type == "named" then
                -- check mandatory
                local m = node_params[i].mandatory
                if type(m) == "table" then
                    for j = 1, #m do
                        if params[m[j]] == nil then
                            if true ~= suppress_output then
                                print("Mandatory parameter " .. m[j] ..
                                      " is missing")
                            if type(node_params[i].alt) == "table" then
                                local a = node_params[i].alt[m[j]]
                                if type(a) == "table" then
                                        print("Should be _one_ of " ..
                                              table.concat(a," "))
                                end
                            end
                            if test_mode then print "###ERR#SYNTAX" end
                            end
                            return false
                        end
                    end
                end
                -- check mandatoryIf
                m = node_params[i].mandatoryIf
                if type(m) == "table" then
                    local k, v
                    for k,v in pairs(m) do
                        if params[k] ~= nil then
                            for j = 1, #v do
                                if params[v[j]] == nil then
                                    if true ~= suppress_output then
                                        print("Mandatory parameter " .. v[j] ..
                                              " is missing")
                                        if test_mode    then
                                            print "###ERR#SYNTAX"
                                        end
                                    end
                                    return false
                                end
                            end
                        end
                    end
                end
                -- check requirements
                m = node_params[i].requirements
                if type(m) == "table" then
                    local k, v
                    for k,v in pairs(m) do
                        if params[k] ~= nil then
                            for j = 1, #v do
                                if params[v[j]] == nil then
                                    if true ~= suppress_output  then
                                        print("Parameter " .. k ..
                                              " requires " .. v[j])
                                        if test_mode    then
                                            print "###ERR#SYNTAX"
                                        end
                                    end
                                    return false
                                end
                            end
                        end
                    end
                end
                -- check conflict
                m = node_params[i].conflict
                if type(m) == "table" then
                    local k, v
                    for k,v in pairs(m) do
                        if params[k] ~= nil then
                            for j = 1, #v do
                                if params[v[j]] ~= nil then
                                    if true ~= suppress_output  then
                                        print("Parameter " .. k ..
                                              " conflicts with " .. v[j])
                                        if test_mode    then
                                            print "###ERR#SYNTAX"
                                        end
                                    end
                                    return false
                                end
                            end
                        end
                    end
                end
            end
        end
    end
    return true
end

-- ************************************************************************
---
--  CLI_execution_lock
--        @description  Locking critical section
--                      This function should redefined in cmdLuaCLIDefs.lua
--
function CLI_execution_lock()
end

-- ************************************************************************
---
--  CLI_execution_unlock
--        @description  Locking critical section
--                      This function should redefined in cmdLuaCLIDefs.lua
--
function CLI_execution_unlock()
end

-- ************************************************************************
---
--  CLI_command_starts
--        @description  function called every time the CLI starts
--                      generic place to set ALL global configurations needed 
--                      for possible left-overs from previous command
--
--      NOTE:           This function is redefined in system_capabilities.lua
--
function CLI_command_starts()
--      NOTE:           This function is redefined in system_capabilities.lua
end


-- ************************************************************************
---
--  flushTraceBuf
--      NOTE:           This function is redefined myGenWrapper.lua
--
function flushTraceBuf()
--      NOTE:           This function is redefined in myGenWrapper.lua
end

-- ************************************************************************
---
--  CLI_exec
--        @description  execete cmdline command
--
--        @param cmdline        - command line
--
--        @return       true, if success, otherwise false
--
function CLI_exec(cmdline)

--  flush trace buffer before new command
    flushTraceBuf()

    if cmdline ~= "end " and 
       cmdline ~= "examples " and 
       cmdline ~= "debug-mode " and
       cmdline ~= "configure " and
       cmdline ~= "traffic " and
       cmdline ~= "running-config " and
       cmdline ~= "micro-init " and
       cmdline ~= "exit " then
        
  --    printTrace("_______________________________________________________________")
  --    printTrace("_______________________________________________________________")
      printTrace("Trace of the command <<" .. cmdline .. ">>  in progress ")
      printTrace(" ")
    end

    local hasMultilineMark =  string.sub(cmdline,-1) == "\\"
    -- support of multiline commands
    if hasMultilineMark then
      if savedPrompt == nil then
        continiousString = string.sub(cmdline,1,-2)
        savedPrompt = cmdLuaCLI_prompt
        cmdLuaCLI_prompt = "+"
      else
        continiousString = continiousString .. " " .. string.sub(cmdline,1,-2)
      end
      return true
    else
      if continiousString ~= nil then
        cmdline = continiousString .. " ".. cmdline
        cmdLuaCLI_prompt = savedPrompt
        continiousString = nil
        savedPrompt = nil
      end
    end

    -- Enable/disable UT
    if cmdline == "CLI_enable_test_mode" then
        test_mode=true
        cmdLuaCLI_prompt = "CLI_PROMPT# "
        return true
    end
    if cmdline == "CLI_disable_test_mode" then
        test_mode=false
        if type(CLI_prompts[mode]) == "string" then
            cmdLuaCLI_prompt = "Console" .. CLI_prompts[mode] .. " "
        end
        return true
    end
    local syntax_check_only = false
    if test_mode and prefix_match("#chk_syntax ", cmdline) then
        syntax_check_only = true
        cmdline = prefix_del("#chk_syntax ", cmdline)
    end
    local cmd = splitline(cmdline)
    local node, node_idx
    -- search node
    local si
    local stk = { current_state, 1 }
    node_idx = 0
    for si = 1, #stk  do
        -- cycle through state to fing node
        --
        local n, i, st, c
        st = stk[si]
        for i = 1, #cmd do
            if desc_tbl[st] == nil then
                break
            end
            c = desc_tbl[st][cmd[i]]
            if st == 1 and cmd[i] == "do" and current_state == 2 then
                c = nil
            end
            if c == nil then
                break
            end
            if c.node ~= nil and i > node_idx then
                node = c.node
                node_idx = i
            end
            if c.next == nil then
                break
            end
            st = c.next
        end
    end
    if node == nil then
        if node_idx == 0 then
            node_idx = 1
        end
        if (cmd[node_idx]~=nil) then print("Error in command line: "..cmd[node_idx] .. ". Command : ",to_string(cmdline)) end
        if test_mode then print "###ERR#SYNTAX" end
        return false
    end
    -- do parse_params
    -- node ~= nil continue from node_idx+1
    
    -- let the system get ready for new command :
    -- maybe need to clean up left-overs from previous command
    CLI_command_starts()

    if node.preFunc then
        -- call pre-function , that should be shared by similar CLI_commands
        -- no parameters are given as we are before check/validation/filling the 'params'
        pcall(node.preFunc, "no parameters")
    end
    
    local parsed, idx, params
        parsed, idx, params = pcall(parse_params, cmd, node_idx+1, node.params, false)
    if not parsed then
        print("Failed to parse parameters: "..idx)
        if test_mode then print "###ERR#SYNTAX" end
        return false
    end

    -- Checking of mandatory, conflict and wrong or incomplete options.
    if false == CLI_mandatory_and_complete_param_check(cmd, idx, params,
                                                       node)                then
        return false
    end

    local args={}
    -- apply node.constFuncArgs
    if type(node.constFuncArgs) == "table" then
        args = duplicate(node.constFuncArgs)
    end
    params["_cmdline"] = cmdline
    table.insert(args,params)

    if idx <= #cmd then
        local j
        for j=idx,#cmd do
            table.insert(args,cmd[j])
        end
    end

    if test_mode then print "###SYNTAXOK" end
    if test_mode and syntax_check_only then
        print("###EXECOK#0#")
        return true
    end

    CLI_execution_lock()
    -- call the 'main' command function
    state, m = pcall(node.func, unpack(args))
    
    if node.postFunc then
        -- call post function , that should be shared by similar CLI_commands
        pcall(node.postFunc, unpack(args))
    end
    
    CLI_execution_unlock()

    if not state then
        print("error while executing: "..m)
        if test_mode then print "###ERR#EXEC" end
    else
        if test_mode then
            if type(m) == "number" then
                print("###EXECOK#"..tostring(m).."#")
            elseif type(m) == "bool" then
                if m then
                    print("###EXECOK#0#")
                else
                    print("###EXECOK#1#")
                end
            else
                print("###EXECOK#0#")
            end
        end
        if type(m) == "number" then state = (m == 0) end
        if type(m) == "boolean" then state = m end
        if not CLI_dont_add_to_running_config then
            table.insert(CLI_running_Cfg_Tbl,cmdline)
        end
        CLI_dont_add_to_running_config = false
        if (#CLI_running_Cfg_Tbl>1000) then table.remove(CLI_running_Cfg_Tbl,1) end  --Max running config saved length
    end
    
    
    --  flush trace buffer after the command
    flushTraceBuf()
    -- print("flushEnd")
    
    return state
end


-- ************************************************************************
---
--  cmdLuaCLI_execute
--        @description  execute command
--
--        @param str            - line to complete
--
--        @return       true, if success, otherwise false
--
function cmdLuaCLI_execute(str)
   local ret = true

   if str == "" then
     return ret
   end

   -- if no space after last word, try complete it
   if string.sub(str, -1) ~= " " then
      local strcompl, compl = cmdLuaCLI_complete(str)
      if #compl == 1 then
         -- single alternative
         str = str .. strcompl
      end
   end
   ret = CLI_exec(str)
   collectgarbage()
   return ret
end


-- ************************************************************************
---
--  cmdLuaCLI_readLine_compl
--        @description  call cmdLuaCLI_readLine with special autocompletion
--
--        @param sparam - parameter description as for type="values".
--                        Fx: {
--                              format = "%cpssAPI_struct_member",
--                              name = "structMember",
--                              help = ""
--                            }
--        @param prompt - prompt string
--        @param history - history enable flag, bool
--
--        @return       string
--
local read_special_node = nil
function cmdLuaCLI_readLine_compl(sparam, prompt, history)
    read_special_node = { params={ { type="values", sparam }}}

    local success, s = pcall(cmdLuaCLI_readLine, prompt, history, true)

    read_special_node = nil
    if not success then
        return ""
    end
    -- remove trailing whitespaces
    while string.sub(s,-1) == " " do
        s = string.sub(s,1,-2)
    end
    return s
end

-- ************************************************************************
---
--  CLI_c
--        @description  autocomplete/help common actions
--
--        @param cmdline        - command line
--
--        @return       true, false
--                      last_cmdline_word, compl_tbl, help_tbl
--
local function CLI_c(cmdline)
    local cmd
    if cmdline == "" then
        cmd = {""}
    elseif string.sub(cmdline, -1) == " " then
        cmd = splitline(cmdline.."x")
        cmd[#cmd] = ""
    else
        cmd = splitline(cmdline)
    end
    local compl = {}
    local help = {}
    local node
    local node_idx = 0
    -- search node
    local si
    local stk = { current_state, 1 }
    local k, v
    local node_help, cr_help
    if read_special_node == nil then
        for si = 1,#stk do
            -- cycle through state
            --
            local n, i, j, st, c
            st = stk[si]
            for i = 1, #cmd do
                if desc_tbl[st] == nil then
                    break
                end
                if (i == #cmd) then
                    -- last word, complete here
                    for k, v in pairs(desc_tbl[st]) do
                        if k == "do" and st == 1 and current_state == 2 then
                            -- skip
                        elseif prefix_match(cmd[i], k) then
                            if not in_table(compl,k) then
                                table.insert(compl, k)
                                if v.node ~= nil then
                                    if v.node.help ~= nil then
                                        help[#compl] = v.node.help
                                        node_help       = v.node.help
                                    end
                                else
                                    if v.help ~= nil then
                                        help[#compl] = v.help
                                        node_help       = v.help
                                    end
                                end
                            end
                        end
                    end
                    break
                end
                c = desc_tbl[st][cmd[i]]
                if st == 1 and cmd[i] == "do" and current_state == 2 then
                    c = nil
                end
                if c == nil then
                    break
                end
                if c.node ~= nil and i > node_idx then
                    node = c.node
                    node_idx = i
                end
                if c.next == nil then
                    break
                end
                st = c.next
            end

            st = stk[si]
            if (nil == node_help) and (nil ~= desc_tbl[st]) then
                for k, v in pairs(desc_tbl[st]) do
                    if prefix_match(cmd[1], k)  then
                        if v.node ~= nil    then
                            node_help   = v.node.help
                        end
                    end
                end
            end
        end
    else -- read_special_node ~= nil
        node = read_special_node
        node_idx = 0
    end
    local ncompl, nhelp, i
    if node ~= nil then
        -- do parse_params
        -- node ~= nil continue from node_idx+1
        local success, idx, params, recovered_idx
        success, idx, params, ncompl, nhelp, recovered_idx =
            pcall(parse_params, cmd, node_idx+1, node.params, true)
        if success then
            for i=1,#ncompl do
                table.insert(compl, ncompl[i])
                if nhelp[i] ~= nil then
                    help[#compl] = nhelp[i]
                end
            end
            if nhelp.def ~= nil then
                help.def = nhelp.def
            end

            -- Checking of mandatory, conflict and wrong or incomplete options.
            if true == CLI_mandatory_and_complete_param_check(
                           cmd, recovered_idx, params, node, true)  then
                if node_help ~= nil then
                    cr_help = node_help
                else
                    cr_help = node.help
                end
            end
        else
            compl       = {}
            help        = {}
            help.def    = "Failed to parse parameters: "..idx
        end
        if nil ~= cr_help    then
            help.cr = cr_help
        end
    end

    ncompl = {}
    nhelp = {}
        nhelp.def = help.def
    nhelp.cr    = help.cr
    -- keep matching only
    for i = 1, #compl do
        if prefix_match(cmd[#cmd], compl[i]) then
            table.insert(ncompl, compl[i])
            if help[i] ~= nil then
                nhelp[#ncompl] = help[i]
            end
        end
    end

    return cmd[#cmd], ncompl, nhelp
end


-- ************************************************************************
---
--  cmdLuaCLI_complete
--        @description  autocomplete
--
--        @param str           - line to complete
--
--        @return       string  - string for completion
--                      array   - array of altenatives
--
function cmdLuaCLI_complete(str)
    local lastword, compl = CLI_c(str)
    if #compl == 0 then
        return "", {}
    end
    local c = compl[1]
    if #compl == 1 then
        return prefix_del(lastword,c) .. " ", compl
    end
    local i
    -- TODO handle case with too many alternatives
    for i = 2, #compl do
        if string.len(compl[i]) < string.len(c) then
            c = string.sub(c, 1, string.len(compl[i]))
        end
        while c ~= "" do
            if not prefix_match(c, compl[i]) then
                c = string.sub(c, 1, string.len(c)-1)
            else
                break
            end
        end
    end
    return prefix_del(lastword, c), compl
end

local function num_max(a,b)
    if a > b then
        return a
    end
    return b
end

-- ************************************************************************
---
--  cmdLuaCLI_help
--        @description  get help for partialy completed command
--
--        @param str            - line to complete
--
function cmdLuaCLI_help(str)
    local lastword, compl, help = CLI_c(str)
    local ret, i,maxNameLen,fmt
    maxNameLen=0
    ret = ""
    if help.def ~= nil then
        ret = help.def
    end
    local special_keys = { "do", "exit", "end", "CLIexit", "CLImemstat" }
    if current_state ~= 1 then
        for i = 1,#special_keys do
            if desc_tbl[current_state][special_keys[i]] ~= nil then
                special_keys[i] = " "..special_keys[i]
            end
        end
    end
    local s_compl = {}
    local s_help = {}
    if str == "" then
        local r_compl = {}
        local r_help = {}
        -- don't sort special keywords
        for i = 1, #compl do
            if in_table(special_keys,compl[i]) then
                s_compl[compl[i]] = compl[i]
                s_help[compl[i]] = help[i]
            else
                r_compl[#r_compl+1] = compl[i]
                r_help[#r_compl] = help[i]
            end
            r_help.def = help.def
        end
        compl = r_compl
        help = r_help
    end


    -- TODO handle case with too many alternatives

    -- sort
    for i = 2, #compl do
        local j = i
        while j > 1 and compl[j] < compl[j-1] do
            local tmp = compl[j-1]
            compl[j-1] = compl[j]
            compl[j] = tmp
            tmp = help[j-1]
            help[j-1] = help[j]
            help[j] = tmp
            j = j - 1
        end
    end

    if str == "" then
        -- now insert special keys
        table.insert(compl, "")
        for i = 1, #special_keys do
            if s_compl[special_keys[i]] ~= nil then
                table.insert(compl, s_compl[special_keys[i]])
                help[#compl] = s_help[special_keys[i]]
            end
        end
    end

    for i = 1, #compl do
        if (compl[i]~=nil) then maxNameLen=num_max(maxNameLen,string.len(compl[i])) end
    end

    maxNameLen = num_max(maxNameLen, default_help_offset)

    fmt = "  %-" .. tostring(maxNameLen + 1) .. "s %s"
    if maxNameLen > 80 then
        fmt = "  %s %s"
    end

    for i = 1, #compl do
        if ret ~= "" then
            ret = ret .. "\n"
        end
        if help[i] ~= nil then
            ret=ret .. string.format(fmt, compl[i], help[i])
        else
            ret=ret .. string.format(fmt, compl[i], "")
        end
        if i > 100 then
            ret = ret .. "\n[too many options - please refine]"
            break
        end
   end

    if nil ~= help.cr then
        if ret ~= "" then
            ret = ret .. "\n"
        end
        ret = ret .. string.format(fmt, cr_string, help.cr)
    end

   return ret
end


-- ************************************************************************
---
--  CLI_check_param_number
--        @description  A common function to check numeric parameters
--                      min/max rules applied id set
--
--        @param param          - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       true, value
--                      false, error_string
--
function CLI_check_param_number(param, name, desc)
    param = tonumber(param)
    if param == nil then
        return false, name .. " not a number"
    end
    if ( desc.min ~= nil and desc.max ~= nil ) and ( param < desc.min or param > desc.max ) then
        return false, name.." is out of range "..tostring(desc.min)..".."..tostring(desc.max)
    end
    if desc.min ~= nil and param < desc.min then
        return false, name.." less than a minimal value "..tostring(desc.min)
    end
    if desc.max ~= nil and param > desc.max then
        return false, name.." greater than a maximum value "..tostring(desc.max)
    end
    return true, param
end

-- ************************************************************************
---
--  CLI_complete_param_number
--        @description  A common function to autocomplete numeric parameters
--                      It just creates help
--
--        @param str            - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       compl == {}, help == {def="help"}
--                      false, error_string
--
function CLI_complete_param_number(str, name, desc)
    if desc.min == nil and desc.max == nil then
        return {}, {}
    end

    help = "<"
    if    (nil ~= desc.min) and (nil ~= desc.max)   then
        help = help .. tostring(desc.min) .. "-" .. tostring(desc.max)
    elseif nil ~= desc.min                          then
        help = help .. tostring(desc.min) .. " and above"
    elseif nil ~= desc.max                          then
        help = help .. tostring(desc.max) .. " and below"
    end
    help = help .. ">"

    if type(desc.help) == "string" then
        help = help .. "   " .. desc.help
    end
    return {}, {def=help}
end
CLI_type_dict["number"] = {
    checker = CLI_check_param_number
}


-- ************************************************************************
---
--  CLI_check_param_enum
--        @description  A common function to check enum parameters
--                      enum in desc is table:
--                          {[key] = {value=xxx,help=yyy}, ... }
--
--        @param str            - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       true, value
--                      false, error_string
--
function CLI_check_param_enum(str, name, desc, varray, params)
    if desc.enum[str] == nil then
        return false, "Invalid value for enum "..name..": "..str
    end
    if type(desc.enum[str]) ~= "table" then
        return true, desc.enum[str]
    end
    
    local v = desc.enum[str]
    local res = true -- is device applicable for enum
    
    -- check if list of applicable devices exists
    if get_item_by_path(params,"devFamily") ~= nil and  type(v) == "table" and v.appl ~= nil then
    
      -- is enum applicable
      res = false
      local l = v.appl
      if type(l) == "table" then
        local i
        for i = 1, #l do
          if params.devFamily == "CPSS_PP_FAMILY_DXCH_" .. string.upper(l[i]) .. "_E" then res=true end
        end
      end
    end
    
    if res then 
      return true, desc.enum[str].value
    else
      return false, "Not applicable value for enum " ..name .. " : " .. str
    end
    
end


-- ************************************************************************
---
--  CLI_complete_param_enum
--        @description  A common function to autocomplete enum parameters
--                      enum in desc is table:
--                          {[key] = {value=xxx,help=yyy}, ... }
--
--        @param str            - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       true, value
--                      false, error_string
--
function CLI_complete_param_enum(str, name, desc, varray, params)
    local k,v
    local compl = {}
    local help = {}
    for k, v in pairs(desc.enum) do
        if prefix_match(str, k) then
            
            local res = true -- is device applicable for enum
            
            -- check if list of applicable devices exists
            if get_item_by_path(params,"devFamily") ~= nil and  type(v) == "table" and v.appl ~= nil then
              
              -- is enum applicable
              res = false
              local l = v.appl
              if type(l) == "table" then
                local i
                for i = 1, #l do
                  if params.devFamily == "CPSS_PP_FAMILY_DXCH_" .. string.upper(l[i]) .. "_E" then res=true end
                end
              end
            end
            
            -- add to list only applicable to devices enums
            if res then
              table.insert(compl, k)
              if type(v) == "table" and v.help ~= nil then
                help[#compl] = v.help
              end
            end
        end
    end
    return compl,help
end

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

-- help for 'number-range' parameters
local help_CLI_check_param_number_range = "range format. examples: all or 5 or 1-3,6 or 1,2-4,6"
-- ************************************************************************
---
--  CLI_check_param_number_range
--        @description  A common function to check numeric-range parameters
--                      checked for min/max rules with 'range format':
--                      all or 5 or 1-3,6 or 1,2-4,6
--
--        @param param          - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       true, value
--                      false, error_string
--
-- NOTE: based on code from check_param_tpid_range(...)
--
function CLI_check_param_number_range(param, name, desc) 

    local s,i,start,stop
    local result = {}

    if param == "all" then
        start = desc.min
        stop = desc.max
        
        if start == nil then 
            -- state default is '0'
            start = 0 
        end
        
        if stop == nil then
            -- state default is start = stop
            stop = start
        end
        
        for i = start, stop do
            table.insert(result,i)
        end

        -- we are done with 'all' case
        return true,result
    end

    --print("param " , to_string(param))
    --printCallStack(5)

    local str = param .. ","
    local errMsg = "Wrong " .. help_CLI_check_param_number_range
    while str ~= "" do
        i = string.find(str,",",1,true)
        s = string.sub(str, 1, i-1)
        str = string.sub(str,i+1)
        -- s is "number" or "number-number"
        i = string.find(s,"-",1,true)
        if i == nil then
            start = tonumber(s)
            stop = start
        else
            start = tonumber(string.sub(s,1,i-1))
            stop = tonumber(string.sub(s,i+1))
        end
        
        if start == nil or stop == nil then
            return false, errMsg
        end

        -- check start and stop values
        -- check start
        local isOk,param2,param3 = CLI_check_param_number(start, name, desc) 
        if (stop ~= start and isOk) then
            -- check stop
            isOk,param2,param3 = CLI_check_param_number(stop, name, desc) 
        end
        
        if not isOk then
            -- error in one of the numbers
            return isOk,param2,param3
        end
        
        for i = start, stop do
            table.insert(result,i)
        end
    end

    return true,result
end

-- ************************************************************************
---
--  CLI_complete_param_number_range
--        @description  A common function to autocomplete numeric parameters
--                      It just creates help
--
--        @param param          - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       compl == {}, help == {def="help"}
--  
-- NOTE: using CLI_check_param_number(...)
--
function CLI_complete_param_number_range(param, name, desc)
    -- let the 'numeric auto complete' to have it's 'help'
    local compl , help = CLI_complete_param_number(param, name, desc)

    if help and help.def then
        -- add our 'range format info'
        help.def = help.def .. " " .. help_CLI_check_param_number_range
    end
    -- add the 'all' to the 'auto-complete'
    if compl and help then
        compl[#compl+1] = "all"
        compl[#compl+1] = ""-- needed so 'all' will not do auto complete when field is 'empty' 
        help[#help+1] = "full range: <" .. desc.min .. ".." .. desc.max .. ">"
        help[#help+1] = ""-- needed so 'all' will not do auto complete when field is 'empty' 
    end
    
    --print("compl",to_string(compl),"help",to_string(compl))

    return compl , help
end  
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

-- add registration of type and parameter by the same name .
function CLI_addParamDictAndType(dictionary_name_string,param_and_prefix_string,help_string, checker_func , complete_func ,min,max , enum_tbl)
    --print("CLI_addParamDictAndType:",to_string(dictionary_name_string), to_string(param_and_prefix_string) , to_string(help_string))
    
    CLI_type_dict[dictionary_name_string] = {
        checker  = checker_func,
        complete = complete_func,
        help = help_string,
        
        min=min,
        max=max,         
        enum = enum_tbl
    }
    CLI_addParamDict(dictionary_name_string, {
        { format = param_and_prefix_string .. " %" .. dictionary_name_string,
                                name = param_and_prefix_string, 
                                help = help_string}
    })  
end
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

-- add registration of ENUM type and parameter by the same name .
function CLI_addParamDictAndType_enum(dictionary_name_string,param_and_prefix_string,help_string, enum_tbl)
    local checker_func  = CLI_check_param_enum
    local complete_func = CLI_complete_param_enum
    CLI_addParamDictAndType(dictionary_name_string,param_and_prefix_string,help_string, checker_func , complete_func ,nil,nil , enum_tbl)
end
-- add registration of number type and parameter by the same name .
function CLI_addParamDictAndType_number(dictionary_name_string,param_and_prefix_string,help_string, min,max)
    local checker_func  = CLI_check_param_number
    local complete_func = CLI_complete_param_number
    CLI_addParamDictAndType(dictionary_name_string,param_and_prefix_string,help_string, checker_func , complete_func ,min,max , nil)
end
-- add registration of number-range type and parameter by the same name .
function CLI_addParamDictAndType_numberRange(dictionary_name_string,param_and_prefix_string,help_string, min,max)
    local checker_func  = CLI_check_param_number_range
    local complete_func = CLI_complete_param_number_range
    CLI_addParamDictAndType(dictionary_name_string,param_and_prefix_string,help_string, checker_func , complete_func ,min,max , nil)
end

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

CLI_type_dict["bool"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    enum = {
        ["true"] = { value=true, help="BOOLEAN True value" },
        ["false"] = { value=false, help="BOOLEAN False value" }
    }
}


-- ************************************************************************
---
--  CLI_check_param_C_enum
--        @description  A common function to check C enum parameters
--                      enum in desc is string:
--                          enum="CPSS_PORT_SPEED_ENT"
--
--        @param str            - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       true, value
--                      false, error_string
--
function CLI_check_param_C_enum(str, name, desc)
    if not desc.casesensitive then
        local c
        c = cmdLuaCLI_callCfunction("mgmType_complete_"..desc.enum, "")
        if type(c) == "table" then
            local i, l
            l = string.lower(str)
            for i = 1, #c do
                if l == string.lower(c[i]) then
                    str = c[i]
                    break
                end
            end
        end
    end
    if type(desc.enum) ~= "string" then
        return false, "Invalid value for enum "..name..": "..str
    end
    local v = cmdLuaCLI_callCfunction("mgmType_check_"..desc.enum, str)
    if v then
        return true, str
    end
    return false, "Invalid value for enum "..name..": "..str
end


-- ************************************************************************
---
--  CLI_complete_param_C_enum
--        @description  A common function to autocomplete C enum parameters
--                      enum in desc is string: enum="CPSS_PORT_SPEED_ENT"
--
--        @param str            - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       true, value
--                      false, error_string
--

function CLI_complete_param_C_enum(str, name, desc)
    local compl
    if not desc.casesensitive then
        local c, i
        c = cmdLuaCLI_callCfunction("mgmType_complete_"..desc.enum, "")
        if type(c) == "table" then
            local i
            compl = {}
            for i = 1, #c do
                if prefix_match(str, c[i], true) then
                    table.insert(compl, str .. string.sub(c[i],string.len(str)+1))
                end
            end
            return compl, {}
        end
        return {}, {}
    end
    compl = cmdLuaCLI_callCfunction("mgmType_complete_"..desc.enum, str)
    if type(compl) == "table" then
        return compl, {}
    end
    return {}, {}
end


-- ************************************************************************
---
--  CLI_check_param_compound
--        @description  A common function to check compound parameters
--                      which are one of selected type compound in desc is
--                      table: { "type1", "type2", ... }
--
--        @param str            - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       true, value
--                      false, error_string
--
function CLI_check_param_compound(str, name, desc, varray, params)
    local i
    for i=1,#desc.compound do

        local n = desc.compound[i]
        local d = CLI_type_dict[n]
        if d == nil then
            -- no type defined, skip parsing
            return true, { compound=n, value=str }
        end
        if type(d.checker) ~= "function" then
            -- no checker, skip parsing
            return true, { compound=n, value=str }
        end
        local status, ret = d.checker(str, n, d, varray, params)
        if status then
            if     "table" == type(ret) then
                ret["compound"] = n
                return true, ret
            elseif "string" == type(ret) then
                return true, { ["compound"] = n, ["string"] = ret }
            else
                return true, { ["compound"] = n, ["value"]  = ret }
            end
        end
    end
    return false, "Value doesn't match any of "..table.concat(desc.compound, ",")
end


-- ************************************************************************
---
--  CLI_complete_param_compound
--        @description  A common function to check compound parameters
--                      which are one of selected type compound in desc is
--                      table: { "type1", "type2", ... }
--
--        @param str            - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       true, value
--                      false, error_string
--
function CLI_complete_param_compound(str, name, desc, varray, params)
    local compl = {}
    local help = {}

    local i
    for i=1,#desc.compound do
        local n = desc.compound[i]
        local d = CLI_type_dict[n]
        if d ~= nil then
            if type(d.complete) == "function" then
                local c, h = d.complete(str, n, d, varray, params)
                local ci
                for ci = 1, #h do
                    table.insert(compl, c[ci])
                    if h[ci] ~= nil then
                        help[#compl] = h[ci]
                    end
                end
            end
        end
    end
    return compl,help
end


-- ************************************************************************
function cmdLuaCLI_exe_print(str)
local ret = true

   if str == "" then
     return ret
   end

   -- if no space after last word, try complete it
   if string.sub(str, -1) ~= " " then
      local strcompl, compl = cmdLuaCLI_complete(str)
      if #compl == 1 then
         -- single alternative
         str = str .. strcompl
      end
   end

   printMsgLvlLog("debug", string.format("%s",str))

   collectgarbage()
   return ret
end

-- ************************************************************************
---
--  cmdLuaCLI_atexit
--        @description  executes at exiting from lua-CLI dialog mode
--
function cmdLuaCLI_atexit()
    -- Saving of first displayed line of running config file.
    luaGlobalStore("CLI_number_of_first_line_in_running_Cfg_Tbl",
                   CLI_number_of_first_line_in_running_Cfg_Tbl)
    luaGlobalStore("CLI_first_line_in_running_Cfg_Tbl",
                   CLI_first_line_in_running_Cfg_Tbl)

   if type(globalStore) == "function" then
        cmdLuaCLI_history[#cmdLuaCLI_history] = "end"
        globalStore("cmdLuaCLI_history", cmdLuaCLI_history)
        CLI_running_Cfg_Tbl[#CLI_running_Cfg_Tbl] = "end"
        globalStore("CLI_running_Cfg_Tbl", CLI_running_Cfg_Tbl)
    end
end


-- Create default CLI mode:
CLI_prompts["exec"] = "#"
CLI_setmode("exec")

-- CLIexit
CLI_addCommand(nil, "CLIexit", {
    help   = "Exit CLI",
    func   = function()
             end
})

-- setModeDX
CLI_addCommand(nil, "setModeDX", {
    help   = "switch to DX dev (if exists)",
    func   = function()
             end
})

-- setModeDX
CLI_addCommand(nil, "setModePX", {
    help   = "switch to PX dev (if exists)",
    func   = function()
             end
})

-- CLImemstat
CLI_addCommand("debug", "CLImemstat", {
    help   = "Show CLI memory use",
    func   = function()
             end
})
end
