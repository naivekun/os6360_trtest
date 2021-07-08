--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cider.lua
--*
--* DESCRIPTION:
--*
--* FILE REVISION NUMBER:
--*       $Revision: 25 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- cider_unitname      - cider unit name
-- cider_subname       - sub name
--
cmdLuaCLI_callCfunction("prvLuaCider_register")

local cider_UT_mode = false
do

local cider_initialized = false
local register_cider_commands
local cider_print_details = true
local cider_port_group_id = 0 -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
local cider_xml = "cider.xml"

local function lookup_cider_xml()
    if fs.stat(cider_xml) ~= nil then
        return true
    end

    return false
end

local function cider_initialize(params)
    cider_xml = "cider.xml"
    if nil~=params and nil~=params["xmlFile"] then
        cider_xml=params["xmlFile"]
    end
    
    if not lookup_cider_xml() then
        print("Please upload "..cider_xml.." first")
        return false
    end
    if not cider_load(cider_xml) then
        print("Failed to load register definitions from "..cider_xml)
        return false
    end
    if false == cider_initialized then
        register_cider_commands()
    end
end

local function xml_dequote(s)
    s = string.gsub(s, "&lt;", "<")
    s = string.gsub(s, "&gt;", ">")
    s = string.gsub(s, "&amp;", "&")
    return s
end

local function prefix_match_i(prefix,str)
    if string.lower(string.sub(str, 1, string.len(prefix))) == string.lower(prefix) then
        return true
    end
    return false
end
local function xprefix(prefix,str)
    return prefix .. string.sub(str, string.len(prefix)+1)
end
local function string_eq_i(s1,s2)
    if string.lower(s1) == string.lower(s2) then
        return true
    end
    return false
end

-- ************************************************************************
---
--  cider_unitname
--        @description  Get unit name(swName)
--
--        @param unit           - unit (xml node)
--
--        @return unitname      - string
--        @return nil           - if not found
--
--+local function cider_unitname(unit)
--+    local name = xml_lookup(unit,"swName")
--+    if name ~= nil then
--+        name = xml_inlinetext(name)
--+    else
--+        name = xml_get_attr(unit, "name")
--+    end
--+    if name == nil then
--+        return nil
--+    end
--+    --name = string.gsub(name, "[\r\n\t]", " ")
--+    name = string.gsub(name, "  ", " ")
--+    -- strip
--+    if string.sub(name, 1, 1) == " " then
--+        name = string.sub(name,2)
--+    end
--+    if string.sub(name,-1) == " " then
--+        name = string.sub(name,1,-2)
--+    end
--+    name = string.gsub(name, " ", "_")
--+    return name
--+end
-- Use C implementation (optimized: no regex, single pass)
local cider_unitname = cider_getname

-- ************************************************************************
---
--  cider_GetSubUnit
--        @description  Get sub unit of given unit by name(swName)
--
--        @param unit           - unit to search in (xml node)
--        @param subname        - unit to search
--
--        @return subunit       - found unit (xml node)
--        @return nil           - if not found
--
local function cider_GetSubUnit(unit, subname)
    if unit == nil then
        unit = cider_RootNode()
    end
    local unit_childs = xml_childs(unit)
    local iterator = nil
    repeat 
        iterator = xml_getchild(unit_childs, iterator)
        if iterator ~= nil then
            local t = xml_tagname(iterator)
            if t == "UNIT" or t == "REGISTER" or t == "MEM" then
                t = cider_unitname(iterator)
                if string_eq_i(t, subname) then
                    return iterator
                end
            end
        end
    until iterator == nil
    return nil
end
local cider_unittags = {}
local cider_unitpath = {}
local cider_indices = {}
local cider_field = nil
local cider_field_to_check = nil
local function cider_rst()
    cider_unitpath = {}
    cider_unittags = {}
    cider_indices = {}
    cider_field = nil
    cider_field_to_check = nil
end
-- cider_u
--     return top node or selected node
local function cider_u()
    if #cider_unittags > 0 then
        return cider_unittags[#cider_unittags]
    end
    return cider_RootNode()
end
-- ************************************************************************
---
--  check_param_cider_unitname
--        @description  Check parameter that it is in the correct form,
--                      Cider unit name
--
--        @param param          - Parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--        @param varray         - previous values array
--
--        @return       boolean check status
--        @return       true:string or false:error_string if failed
--        @return       true if no more subunits
--
local function check_param_cider_unitname(param, name, desc, varray)
    if varray == nil or (type(varray) == "table" and #varray == 0) then
        cider_rst()
    end
    local parent, unit
    parent = nil
    if #cider_unitpath > 0 then
        parent = cider_unittags[#cider_unitpath]
        if xml_tagname(parent) ~= "UNIT" then
            return false, "no subunits in "..cider_unitpath[#cider_unitpath]
        end
    end
    unit = cider_GetSubUnit(parent, param)
    if unit ~= nil then
        local last = false
        local nm = cider_unitname(unit)
        if xml_tagname(unit) ~= "UNIT" then
            last = true
            if desc.tablesonly and xml_tagname(unit) ~= "MEM" then
                return false, "Wrong table name"
            end
            if desc.countersonly then
                if string.match(string.lower(nm),"counter") == nil then
                    return false, "Wrong counter name"
                end
            end
        end
        table.insert(cider_unitpath, nm)
        cider_unittags[#cider_unitpath] = unit
        return true, nm, last
    end
    return false, "Wrong cider unit name"
end

-- ************************************************************************
---
--  complete_value_cider_unitname
--        @description  autocompletes cider unit name
--
--        @param param              - parameter string
--        @param name               - parameter name string
--        @param desc               - type description reference
--        @param varray             - previous values array
--
--        @return        cider unit name autocomplete
--
local function complete_value_cider_unitname(param,name,desc,varray)
    if varray == nil or (type(varray) == "table" and #varray == 0) then
        cider_rst()
    end
    local parent
    parent = nil
    if #cider_unitpath > 0 then
        parent = cider_unittags[#cider_unitpath]
        if xml_tagname(parent) ~= "UNIT" then
            return {}, {}
        end
    end
    local compl = {}
    local help = {}
    if parent == nil then
        parent = cider_RootNode()
    end
    local parent_childs = xml_childs(parent)
    local iterator = nil
    repeat 
        iterator = xml_getchild(parent_childs, iterator)
        if iterator == nil then
            break
        end
        local t = xml_tagname(iterator)
        if t == "UNIT" or t == "REGISTER" or t == "MEM" then
            local unitname = cider_unitname(iterator)
            if unitname ~= nil then
                if prefix_match_i(param,unitname) then
                    local add = true
                    -- tablesonly and countersonly
                    if desc.tablesonly or desc.countersonly then
                        if t ~= "UNIT" then
                            if desc.tablesonly and t ~= "MEM" then
                                add = false
                            end
                            if desc.countersonly then
                                local nm = string.lower(unitname)
                                if string.match(nm,"counter") == nil then
                                    add = false
                                end
                            end
                        end
                    end
                    if add then
                        table.insert(compl, xprefix(param,unitname))
                        -- add help
                        -- help[#compl] = value
                    end
                end

            end
        end
    until iterator == nil
    return compl,help
end


CLI_type_dict["cider_unitname"] = {
    checker = check_param_cider_unitname,
    complete = complete_value_cider_unitname,
    help = "Cider unit name"
}

CLI_type_dict["cider_tablepath"] = {
    checker = check_param_cider_unitname,
    complete = complete_value_cider_unitname,
    tablesonly = true,
    help = "Cider table path"
}

CLI_type_dict["cider_counterpath"] = {
    checker = check_param_cider_unitname,
    complete = complete_value_cider_unitname,
    countersonly = true,
    help = "Cider counter path"
}


local function paths_to_entrynames(paths,names)
    -- add suffixes when required
    local i, j, p, r
    for i = 1, #paths do
        p = paths[i]
        if names[i] == nil then
            names[i] = p[#p]
            for j = i+1,#paths do
                r = paths[j]
                if string_eq_i(r[#r], p[#p]) then
                    names[i] = p[#p] .. "_(" .. table.concat(p, "_", 1, #p-1) .. ")"
                    names[j] = r[#r] .. "_(" .. table.concat(r, "_", 1, #r-1) .. ")"
                end
            end
        end
    end
end


-- ************************************************************************
---
--  cider_lookup_register
--        @description  Lookup register by name through whole cider.xml
--
--        @param ret.str        - string to search
--        @param ret.complete   - return results for complete/help
--
--        @param path           - parent path (swNames)
--        @param upath          - parent path (xml tags)
--        @param level          - length of parent path
--        @param unit           - currenl unit (xml tag)
--
--        @return       ret.found    - number of found items
--        @return       ret.unitpath
--        @return       ret.unittags
--        @return       ret.compl
--        @return       ret.help
--
local function cider_lookup_register(ret, path, upath, level, unit)
    if level == 0 and ret.complete then
        ret.paths = {}
    end
    local tagname = xml_tagname(unit)
    if tagname == "REGISTER" or tagname == "MEM" then
        local regname = cider_unitname(unit)
        local longname = regname .. "_(" .. table.concat(path, "_", 2, level) .. ")"
        if ret.complete then
            if not prefix_match_i(ret.str, regname) and not prefix_match_i(ret.str, longname) then
                return
            end
            local p = {}
            local i
            for i = 2, level do
                table.insert(p,path[i])
            end
            table.insert(p, regname)
            table.insert(ret.paths, p)
            if not prefix_match_i(ret.str, regname) then
                ret.compl[#ret.paths] = xprefix(ret.str,longname)
            end
            --TODO
            -- ret.help[#ret.paths] = help(unit)
            return
        end
        if not string_eq_i(regname,ret.str) and not string_eq_i(longname,ret.str) then
            return
        end
        ret.found = ret.found + 1
        path[level+1] = cider_unitname(unit)
        upath[level+1] = unit
        ret.unitpath = {}
        ret.unittags = {}
        local i
        for i = 1,level+1 do
            ret.unitpath[i] = path[i]
            ret.unittags[i] = upath[i]
        end
        return
    end
    if tagname ~= "UNIT" then
        return
    end
    path[level+1] = cider_unitname(unit)
    upath[level+1] = unit
    local unit_childs = xml_childs(unit)
    local iterator = nil
    repeat 
        iterator = xml_getchild(unit_childs, iterator)
        if iterator ~= nil then
            tagname = xml_tagname(iterator)
            if tagname == "UNIT" or tagname == "REGISTER" or tagname == "MEM" then
                cider_lookup_register(ret, path, upath, level+1, iterator)
            end
        end
    until iterator == nil
    -- add suffixes when required
    if level == 0 and ret.complete then
        paths_to_entrynames(ret.paths,ret.compl)
        local i
        for i = 1, #ret.paths do
            ret.compl[i] = xprefix(ret.str,ret.compl[i])
        end
    end
end
-- ************************************************************************
---
--  check_param_cider_register_name
--        @description  Check parameter that it is in the correct form, 
--                      Register name
--
--        @param param          - Parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--        @param varray         - previous values array
--
--        @return       boolean check status
--        @return       true:string or false:error_string if failed
--        @return       true if no more subunits
--
local function check_param_cider_register_name(param, name, desc, varray)
    cider_rst()
    --TODO: special case for 0x12345
    if prefix_match("0x", param) then
        local addr = tonumber(param)
        if addr == nil then
            return false, "Address must be a hex number"
        end
        local p = { address = addr }
        if do_search_unit_by_address(nil,p,{},0,cider_RootNode(),{}) then
            cider_unitpath = p.path
            cider_unittags = p.upath
            local i, idx
            if type(p.i_indices) == "table" then
                for i = 1,#p.i_indices do
                    idx = p.i_indices[i]
                    if i==1 then
                        cider_indices[string.format("index")] = idx.current_value
                    else
                        cider_indices[string.format("index%d",i)] = idx.current_value
                    end
                end
            end
            return true, param
        else
            return false, "No register with given address"
        end
    end
    local r = { found = 0, str = param, complete=false }
    cider_lookup_register(r, {}, {}, 0, cider_RootNode())
    if r.found == 0 then
        return false, "No register with given name"
    end
    if r.found > 1 then
        return false, "There is more than one register with name \""..param.."\"\n"..
             "Use \"show unit ...\" or \"set unit ...\" commands instead"
    end
    cider_unitpath = r.unitpath
    cider_unittags = r.unittags
    return true, param
end
-- ************************************************************************
---
--  complete_value_cider_register_name
--        @description  autocompletes field name of cider register name
--
--        @param param              - parameter string
--        @param name               - parameter name string
--        @param desc               - type description reference
--
--        @return        cider field name autocomplete
--
local function complete_value_cider_register_name(param,name,desc)
    cider_rst()
    if param == "" then
        return {}, {}
    end
    --TODO: special case for 0x12345
    local r = { found = 0, compl={}, help={}, str = param, complete=true }
    cider_lookup_register(r, {}, {}, 0, cider_RootNode())
    return r.compl, r.help
end

CLI_type_dict["cider_register_name"] = {
    checker = check_param_cider_register_name,
    complete = complete_value_cider_register_name,
    help = "Cider register name"
}


-- ************************************************************************
---
--  check_param_cider_field
--        @description  Check parameter that it is in the correct form,
--                      field name of cider unit
--
--        @param param          - Parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       boolean check status
--        @return       true:string or false:error_string if failed
--
local function check_param_cider_fieldname(param, name, desc)
    if #cider_unittags < 1 then
        return false, "No unit specified"
    end
    local v
    local u = cider_unittags[#cider_unittags]
    local unitname = cider_unitname(u)
    local function check_fields(u, param)
        u = xml_childs(u)
        local field = nil
        repeat
            field = xml_getchild(u, field)
            if xml_tagname(field) == "FIELD" then
                if string_eq_i(cider_getname(field), param) then
                    cider_field = field
                    return true
                end
            end
        until field == nil
        return false
    end
    if xml_tagname(u) == "REGISTER" then
        u = xml_lookup(u, "FIELDS")
        v = check_fields(u, param)
        if v then
            return true, param
        end
    elseif xml_tagname(u) == "MEM" then
        u = xml_childs(xml_lookup(u, "TABLE"))
        local ds = nil
        repeat
            ds = xml_getchild(u, ds)
            if xml_tagname(ds) == "DS" then
                v = check_fields(xml_lookup(ds, "FIELDS"), param)
                if v then
                    return true, param
                end
            end
        until ds == nil
    else
        return false, "Applicable for REGISTER/MEM only"
    end
    return false, "Field "..param.." not found in "..unitname
end

local function table_has(tbl, val)
    local i
    if type(tbl) ~= "table" then
        return false
    end
    for i=1,#tbl do
        if tbl[i] == val then
            return true
        end
    end
    return false
end

-- ************************************************************************
---
--  complete_value_cider_fieldname
--        @description  autocompletes field name of cider unit
--
--        @param param              - parameter string
--        @param name               - parameter name string
--        @param desc               - type description reference
--
--        @return        cider field name autocomplete
--
local function complete_value_cider_fieldname(param,name,desc)
    local compl = {}
    local help = {}
    if #cider_unittags < 1 then
        -- No unit specified
        return compl, help
    end
    local function compl_fields(u,param, compl, help)
        local field = nil
        u = xml_childs(u)
        repeat
            field = xml_getchild(u, field)
            if xml_tagname(field) == "FIELD" then
                local nm = cider_getname(field)
                if prefix_match_i(param,nm) and not table_has(compl,nm) then
                    table.insert(compl, xprefix(param,nm))
                    -- add help
                    -- local desc = xml_inlinetext(xml_lookup(field, "FIELD.DESCRIPTION"))
                    -- if desc ~= "" then
                    --    help[#compl] = xml_dequote(desc)
                    -- end
                end
            end
        until field == nil
    end
    local u = cider_unittags[#cider_unittags]
    if xml_tagname(u) == "REGISTER" then
        u = xml_lookup(u, "FIELDS")
        compl_fields(u, param, compl, help)
    elseif xml_tagname(u) == "MEM" then
        u = xml_childs(xml_lookup(u, "TABLE"))
        local ds = nil
        repeat
            ds = xml_getchild(u, ds)
            if xml_tagname(ds) == "DS" then
                compl_fields(xml_lookup(ds, "FIELDS"), param, compl, help)
            end
        until ds == nil
    else
        -- Applicable for REGISTER/MEM only
    end
    return compl,help
end

CLI_type_dict["cider_fieldname"] = {
    checker = check_param_cider_fieldname,
    complete = complete_value_cider_fieldname,
    help = "Cider subunit field name"
}

local function get_int_attr(tag,attrname,hex)
    local attr = xml_get_attr(tag,attrname)
    if attr == nil then
        return 0
    end
    if attr == "" then
        return 0
    end
    if hex and not prefix_match(attr,"0x") and not prefix_match(attr,"0X") then
        attr = "0x" .. attr
    end
    return tonumber(attr)
end

-- ************************************************************************
---
--  check_param_cider_field_value
--        @description  Check parameter that it is in the correct form,
--                      field value of cider unit
--
--        @param param          - Parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       boolean check status
--        @return       true:string or false:error_string if failed
--
local function check_param_cider_field_value(param, name, desc)
    local status, value
    local d
    if cider_field == nil then
        -- special for tables
        if xml_tagname(cider_unittags[#cider_unitpath]) == "MEM" then
            return true, param
        end
        -- set register (raw)
        d = CLI_type_dict["GT_U32_hex"]
        return d.checker(param,name,d)
    end
    local len = get_int_attr(cider_field, "len")
    if len > 32 then
        if prefix_match_i("0x", param) and string.len(param) > 10 then
            -- special case: hexstring for field longer than 32bit
            status = CLI_check_param_hexstring(string.sub(param, 3),name,desc)
            if status then
                return true, param
            end
        end
    end
    status, value = CLI_check_param_number(param,name,desc)
    local enums = xml_childs(xml_lookup(cider_field,"ENUMS"))
    if status or enums == nil then
        -- check range
        if value < 0 then
            status = false
        end
        local max_val = 0xffffffff
        if len < 31 then
            max_val = bit_shl(1,len) - 1
        elseif len == 31 then
            max_val = 0x7fffffff
        end
        if value > max_val then
            status = false
        end
        if status == true then
            return status, value
        end
        return false, string.format("Out of range (0..0x%x)",max_val)
    end

    -- check enum
    local enum = nil
    repeat
        enum = xml_getchild(enums, enum)
        if xml_tagname(enum) == "ENUM" then
            local ename = cider_getname(enum)
            ename = string.gsub(ename," ","_")
            if string_eq_i(ename,param) then
                return true, get_int_attr(enum,"val")
            end
        end
    until enum == nil
    return false, "wrong value"
end

-- ************************************************************************
---
--  complete_value_cider_field_value
--        @description  autocompletes field value of cider unit
--
--        @param param              - parameter string
--        @param name               - parameter name string
--        @param desc               - type description reference
--
--        @return        cider field value autocomplete
--
local function complete_value_cider_field_value(param,name,desc)
    local compl = {}
    local help = {}
    if cider_field == nil then
        return compl, help
    end
    local enums = xml_childs(xml_lookup(cider_field,"ENUMS"))
    if enums == nil then
        return compl, help
    end
    local enum = nil
    repeat
        enum = xml_getchild(enums, enum)
        if xml_tagname(enum) == "ENUM" then
            local ename = cider_getname(enum)
            ename = string.gsub(ename," ","_")
            if prefix_match_i(param,ename) and not table_has(compl,ename) then
                table.insert(compl, xprefix(param,ename))
            end
        end
    until enum == nil
    return compl,help
end

CLI_type_dict["cider_fieldvalue"] = {
    checker = check_param_cider_field_value,
    complete = complete_value_cider_field_value,
    help = "Cider subunit field value"
}

-- ************************************************************************
---
--  check_param_cider_field_to_check
--        @description  Check parameter that it is in the correct form,
--                      field name of cider unit
--
--        @param param          - Parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       boolean check status
--        @return       true:string or false:error_string if failed
--
--
-- show unit @cider_unitname [check %cider_field_to_check is %cider_fieldvalue] \
--        [field %cider_fieldname] [index %number] [index2 %number]
--
local function check_param_cider_field_to_check(param, name, desc)
    local valid, value = check_param_cider_fieldname(param,name,desc)
    if valid then
        cider_field_to_check = cider_field
    end
    return valid, value
end

CLI_type_dict["cider_field_to_check"] = {
    checker = check_param_cider_field_to_check,
    complete = complete_value_cider_fieldname,
    help = "Cider subunit field name to check with value"
}

local function indent(level)
    return string.rep("  ",level)
end

local function c_iterator_subst_indices(params,str)
    local i, idx
    if type(params.i_indices) ~= "table" then
        return str
    end
    for i = 1,#params.i_indices do
        idx = params.i_indices[i]
        str = string.gsub(str,"%%"..idx.name,tostring(idx.current_value),1)
        str = string.gsub(str,"_"..idx.name.."_","_"..tostring(idx.current_value).."_",1)
    end
    return str
end

-- ************************************************************************
---
--  cider_check_access
--        @description  Check fields access type (read/write)
--                      at least one field must be readable (writable)
--
--        @param fields         - fields xml node
--        @param access         - access type: "read", "write"
--
--        @return       boolean check status, warning about WO/RO
--
local function cider_check_access(fields,access)
    if fields == nil then
        return true, false
    end
    -- walk through all fields
    local field = nil
    local ret = false
    local warn = false
    repeat
        field = xml_getchild(fields, field)
        if xml_tagname(field) == "FIELD" then
            local f_access = xml_get_attr(field, "access")
            if f_access == "RO" then
                if access == "read" then
                    ret = true
                end
                if access == "write" then
                    warn = true
                end
            elseif f_access == "WO" then
                if access == "write" then
                    ret = true
                end
                if access == "read" then
                    warn = true
                end
            elseif f_access == "RW" then
                ret = true
            end
        end
    until field == nil
    return ret, warn
end


local function do_show_fields(params,level,parent,value)
    local fields
    fields = xml_childs(xml_lookup(parent, "FIELDS"))
    if fields == nil then
        if type(value) == "number" then
            fields = string.format("0x%x", value)
        else
            fields = cider_DumpUserdata(value)
        end
        print(indent(level)..cider_unitname(parent).."="..fields)
        return true
    end
    -- walk through all fields
    local field = nil
    repeat
        field = xml_getchild(fields, field)
        if xml_tagname(field) == "FIELD" then
            local v = 0
            local msb = get_int_attr(field, "msb")
            local lsb = get_int_attr(field, "lsb")
            local len = get_int_attr(field, "len")
            v = cider_GetBits(value,msb,lsb,len)
            local enums = xml_childs(xml_lookup(field,"ENUMS"))
            local displ
            if type(v) == "number" then
                displ = string.format("0x%x", v)
            else
                displ = v
            end
            if enums ~= nil then
                local enum = nil
                repeat
                    enum = xml_getchild(enums, enum)
                    if xml_tagname(enum) == "ENUM" then
                        if v == get_int_attr(enum,"val") then
                            displ = displ .. " ("..cider_getname(enum)..")"
                            break
                        end
                    end
                until enum == nil
            end
            -- display field
            local name = cider_getname(field)
            if params.field ~= nil then
                if string_eq_i(name,params.field) then
                    -- single field, display description if single entry displayed
                    print(indent(level)..c_iterator_subst_indices(params,name).."="..displ)
                    if params.i_single_entry == true then
                        local desc = xml_inlinetext(xml_lookup(field,"DESCRIPTION"))
                        if type(desc) == "string" and desc ~= "" then
                            print(indent(level).."Description: "..xml_dequote(desc))
                        end
                    end 
                end
            else
                print(indent(level)..c_iterator_subst_indices(params,name).."="..displ)
            end
        end
    until field == nil
end

local function xml_get_index_info(unit)
    if xml_tagname(unit) == "REGISTER" then
        return xml_lookup(unit, "ADDR")
    end
    if xml_tagname(unit) == "MEM" then
        local addr
        addr = xml_lookup(unit, "TABLE.DS.ADDR")
        if addr == nil then
            addr = xml_lookup(unit, "TABLE.ADDR")
        end
        if addr == nil then
            addr = xml_lookup(unit, "ADDR")
        end
        return addr
    end
end

local function c_iterator_chk_extra_params(params)
    local i
    for i = 1, 3 do
        local s
        if i > 1 then
            s = string.format("%d",i)
        else
            s = ""
        end
        local v = false
        if params.i_indices ~= nil then
            if params.i_indices[i] ~= nil then
                v = true
            end
        end
        if params["index"..s] ~= nil and v == false then
            print("Warning: option 'index"..s.."' not applicable for this entry")
        end
        if params["range"..s] ~= nil and v == false then
            print("Warning: option 'range"..s.."' not applicable for this entry")
        end
    end
end

local function c_iterator_init(params,base_address,addr)
    params.i_base_addr=base_address
    params.i_valid=true
    params.i_indices=nil
    params.i_single_entry=true
    -- initialize check
    if params.check ~= nil then
        if cider_field_to_check ~= nil then
            params.check_msb = get_int_attr(cider_field_to_check, "msb")
            params.check_lsb = get_int_attr(cider_field_to_check, "lsb")
            params.check_len = get_int_attr(cider_field_to_check, "len")
        else
            params.check = nil
        end
    end
    if addr == nil then
        c_iterator_chk_extra_params(params)
        return true
    end
    params.i_base_addr=get_int_attr(addr,"baseAddr")
    params.i_indices={}
    local aparams = xml_childs(xml_lookup(addr, "PARAMS"))
    local param = nil
    repeat
        param = xml_getchild(aparams, param)
        if xml_tagname(param) == "PARAM" then
            local idx = {}
            local ordinal
            idx.name=xml_get_attr(param,"name")
            idx.label=xml_get_attr(param,"lable")
            idx.startVal=get_int_attr(param,"startVal")
            idx.endVal=get_int_attr(param,"endVal")
            idx.incr=get_int_attr(param,"incr")
            idx.do_print = true
            ordinal = get_int_attr(param,"ordinal")
            idx.value = idx.startVal
            local index_string = "index"
            if ordinal ~= 0 then
                index_string = string.format("index%d",ordinal+1)
            end 
            local value = params[index_string]
            if string.lower(idx.label) == "port" then
                if params.port ~= nil then
                    if value ~= nil then
                        print(string.format("'%s' conflicts with 'port'",index_string))
                        return false
                    end
                    value = params.port
                    params.port = nil
                end
            end

            if value ~= nil then
                if value < idx.startVal or value > idx.endVal then
                    print(string.format("%s is out of range (%d .. %d)",
                            index_string,
                            idx.startVal,
                            idx.endVal))
                    return false
                end
                idx.value = value
                idx.startVal = value
                idx.endVal = value
                idx.incr = 1
                idx.do_print = false
            end
            local range_string = "range"
            if ordinal ~= 0 then
                range_string = string.format("range%d",ordinal+1)
            end
            value = params[range_string]
            if type(value) == "table" then
                if value[1] < idx.startVal or value[1] > idx.endVal then
                    print(string.format("%s start is out of range (%d .. %d)",
                            range_string,
                            idx.startVal,
                            idx.endVal))
                    return false
                end
                if value[2] < idx.startVal or value[2] > idx.endVal then
                    print(string.format("%s end is out of range (%d .. %d)",
                            range_string,
                            idx.startVal,
                            idx.endVal))
                    return false
                end
                if value[1] > value[2] then
                    print(string.format("%s start > end", range_string))
                    return false
                end
                idx.value = value[1]
                idx.startVal = value[1]
                idx.endVal = value[2]
                idx.do_print = true
            end
            if idx.value ~= idx.endVal then
                params.i_single_entry = false
            end
            params.i_indices[ordinal+1] = idx
        end
    until param == nil
    if params.port ~= nil then
        print("option 'port' not applicable for given entry")
        return false
    end
    c_iterator_chk_extra_params(params)
    local f = "function cider_formula("
    local i
    for i=1,#params.i_indices do
        if i > 1 then
            f = f .. ","
        end
        f = f .. params.i_indices[i].name
    end
    f = f .. ")\nreturn "..xml_get_attr(addr,"formula").."\nend"
    loadstring(f)()
    return true
end

local function c_iterator_check_expr(params, value)
    if params.check == nil or cider_field_to_check == nil then
        return true
    end
    local v = cider_GetBits(value, params.check_msb,
            params.check_lsb, params.check_len)
    if params.check[2] == "eq" and v ~= params.check[3] then
        return false
    end
    if params.check[2] == "ne" and v == params.check[3] then
        return false
    end
    if params.check[2] == "lt" and v >= params.check[3] then
        return false
    end
    if params.check[2] == "gt" and v <= params.check[3] then
        return false
    end
    if params.check[2] == "le" and v > params.check[3] then
        return false
    end
    if params.check[2] == "ge" and v < params.check[3] then
        return false
    end
    return true
end

local function c_iterator_print(params,level)
    local i, idx
    if type(params.i_indices) ~= "table" then
        return
    end
    for i = 1,#params.i_indices do
        idx = params.i_indices[i]
        if idx.do_print then
            print(indent(level)..idx.label.."="..tostring(idx.current_value))
        end
    end
end
local function c_iterator_next(params)
    if not params.i_valid then
        return nil
    end
    if type(params.i_indices) ~= "table" then
        params.i_valid = false
        return 0
    end
    local i, idx
    idx = {}
    for i = 1, #params.i_indices do
        params.i_indices[i].current_value = params.i_indices[i].value
        idx[i] = params.i_indices[i].value
    end
    -- increment
    i = #params.i_indices
    while i > 0 do
        local cidx = params.i_indices[i]
        cidx.value = cidx.value+cidx.incr
        if cidx.value <= cidx.endVal then
            break
        end
        cidx.value = cidx.startVal
        i = i - 1
    end
    if i == 0 then
        params.i_valid = false
    end

    return cider_formula(unpack(idx))
end

local function do_optional_params(params)
    if params.devID == nil then
        params.devID = 0
    end
    if params.port_group_id == nil then
        params.port_group_id = cider_port_group_id
    end
end

local function do_show_unit_register(params,level,unit)
    if not c_iterator_init(
                params,
                get_int_attr(unit, "addr", true),
                xml_get_index_info(unit)
            )
    then
        return false
    end
    print(indent(level).."Register "..cider_unitname(unit)..":")

    do_optional_params(params)

    while params.i_valid do
        local regoffset = c_iterator_next(params)
        local regaddr = params.i_base_addr+regoffset
        local result, values
        local do_entry = true
        if params.check ~= nil then
            result, values = myGenWrapper(
                "cpssDrvPpHwRegisterRead", {
                    { "IN", "GT_U8",  "devNum", params.devID },
                    { "IN", "GT_U32", "portGroupId", params.port_group_id },
                    { "IN", "GT_U32", "regAddr", regaddr },
                    { "OUT","GT_U32", "regValue"}
                }
            )
            if result == 0 then
                do_entry = c_iterator_check_expr(params, values.regValue)
            else
                do_entry = false
            end
        end
        if do_entry then
            c_iterator_print(params, level)
            if not params.no_print_addr_raw_value then
                print(indent(level+1)
                    ..string.format("address 0x%.8x + 0x%.8x",
                            params.i_base_addr,
                            regoffset
                    ))
            end
            if cider_UT_mode then
                print(string.format("###ADDR=0x%x", regaddr))
                return true
            end
            if params.check == nil then
                result, values = myGenWrapper(
                    "cpssDrvPpHwRegisterRead", {
                        { "IN", "GT_U8",  "devNum", params.devID },
                        { "IN", "GT_U32", "portGroupId", params.port_group_id },
                        { "IN", "GT_U32", "regAddr", regaddr },
                        { "OUT","GT_U32", "regValue"}
                    }
                )
            end
            if result == 0 then
                if not params.no_print_addr_raw_value then
                    print(indent(level+1)..string.format("value=0x%.8x",values.regValue))
                end
                if cider_print_details then
                    do_show_fields(params,level+1,unit,values.regValue)
                end
            else
                print(indent(level+1)
                        .."failed to read register from "
                        ..tostring(regaddr)
                        ..", err="..tostring(result))
            end
        end
    end
    return true
end

local function do_show_unit_mem(params,level,unit)
    do_optional_params(params)

    if not c_iterator_init(
                params,
                get_int_attr(unit, "mem_base", true),
                xml_get_index_info(unit)
            )
    then
        return false
    end
    local mem_entry_width = get_int_attr(unit, "entry_width")
    print(indent(level).."Table "..cider_unitname(unit)..":")
    local u = xml_childs(xml_lookup(unit, "TABLE"))
    while params.i_valid do
        local memoffset = c_iterator_next(params)
        local memaddr = params.i_base_addr+memoffset
        local memvalue
        local do_entry = true
        if params.check ~= nil then
            memvalue = cider_ReadPpMemory(
                    params.devID, -- deviceId
                    params.port_group_id, -- portGroupId
                    memaddr,
                    mem_entry_width)
            if type(memvalue) ~= "number" then
                do_entry = c_iterator_check_expr(params, memvalue)
            else
                do_entry = false
            end
        end
        if do_entry then
            c_iterator_print(params, level)
            if not params.no_print_addr_raw_value then
                print(indent(level+1)
                        ..string.format("read mem addr=(0x%x + 0x%x)",
                                params.i_base_addr,
                                memoffset
                        ))
            end
            if cider_UT_mode then
                print(string.format("###ADDR=0x%x", memaddr))
                return true
            end
            if params.check == nil then
                memvalue = cider_ReadPpMemory(
                        params.devID, -- deviceId
                        params.portGroupId, -- portGroupId
                        memaddr,
                        mem_entry_width)
            end
            if type(memvalue) == "number" then
                print(indent(level+1)
                        .."failed to read PP memory from "
                        ..string.format(" %x, err=%d", memaddr, memvalue))
            else
                if not params.no_print_addr_raw_value then
                    print(indent(level+1).."value="..cider_DumpUserdata(memvalue))
                end
                if cider_print_details then
                    u = xml_childs(xml_lookup(unit, "TABLE"))
                    local ds = nil
                    repeat
                        ds = xml_getchild(u, ds)
                        if xml_tagname(ds) == "DS" then
                            print(indent(level+1).."Section "..cider_getname(ds)..":")
                            do_show_fields(params,level+2,ds,memvalue)
                        end
                    until ds == nil
                end
            end
        end
    end
    return true
end

local function do_show_unit(params,level,unit)
    if xml_tagname(unit) == "REGISTER" then
        return do_show_unit_register(params, level, unit)
    end
    if xml_tagname(unit) == "MEM" then
        return do_show_unit_mem(params, level, unit)
    end
    if xml_tagname(unit) ~= "UNIT" then
        return false
    end
    --
    print(indent(level+1).."Doing for unit name=", cider_unitname(unit))
    local unit_inline = xml_childs(unit)
    local iterator = nil
    repeat
        iterator = xml_getchild(unit_inline, iterator)
        if iterator ~= nil then
            local tagname = xml_tagname(iterator)
            if tagname == "UNIT" or tagname == "REGISTER" or tagname == "MEM" then
                do_show_unit(params,level+1,iterator)
            end
        end
    until iterator == nil
end

local print_nice_enabled = true
local print_nice_saved_print
local print_nice_lines = 0
local print_nice_interrupted = false

local function print_nice_enable()
    print_nice_enabled = true
end
local function print_nice_disable()
    print_nice_enabled = false
end
local function print_nice(...)
    print_nice_lines = print_nice_lines + 1
    if print_nice_lines > 20 then
        local reply = cmdLuaCLI_readLine("Type <CR> to continue, Q<CR> to stop:")
        if reply == nil or reply == "Q" or reply == "q" then
            print_nice_interrupted = true
            error("break command")
        end
        print_nice_lines = 0
    end
    return print_nice_saved_print(...)
end

local function do_command_with_print_nice(cmd,...)
    if print_nice_enabled then
        print_nice_saved_print = print
        print_nice_lines = 0
        print_nice_interrupted = false
        print = print_nice
    end

    local success, rc

    success, rc = pcall(...)

    if print_nice_enabled then
        print = print_nice_saved_print
    end

    if success then
        return rc
    end
    if print_nice_enabled then
        if print_nice_interrupted then
            return true
        end
    end

    print("Failed in "..cmd..": "..rc)
    return false
end


local function show_unit(params)
    return do_command_with_print_nice(
                "do_show_unit()", do_show_unit,
                params,
                0,
                cider_u()
            )
end

local function do_set_fields(params,parent,value)
    local fields
    fields = xml_childs(xml_lookup(parent, "FIELDS"))
    if fields == nil then
        return params.value
    end
    if params.field == nil then
        print("field parameter required")
        return nil
    end
    -- walk through all fields
    local field = nil
    repeat
        field = xml_getchild(fields, field)
        if xml_tagname(field) == "FIELD" then
            local msb = get_int_attr(field, "msb")
            local lsb = get_int_attr(field, "lsb")
            local len = get_int_attr(field, "len")
            local name = cider_getname(field)
            if string_eq_i(name,params.field) then
                --print("cider_SetBits",value,msb,lsb,len,params.value)
                return cider_SetBits(value,msb,lsb,len,params.value)
            end
        end
    until field == nil
end

local function do_set_check_indices(params,unit)
    if type(params.i_indices) ~= "table" then
        return true
    end
    local i
    for i = 1, #params.i_indices do
        local index_string = "index"
        if i ~= 1 then
            index_string = string.format("index%d", i)
        end
        if params[index_string] == nil then
            print(string.format("%s (%d..%d) parameter required",
                    index_string,
                    params.i_indices[i].startVal,
                    params.i_indices[i].endVal))
            return false
        end
        params.i_indices[i].do_print = true
    end
    return true
end

local function do_set_unit_register(params,unit)
    if not c_iterator_init(
                params,
                get_int_attr(unit, "addr", true),
                xml_get_index_info(unit)
            )
    then
        return false
    end

    -- Don't check, allowed to set range/all
    -- -- check for required indices
    -- if not do_set_check_indices(params,unit) then
    --     return false
    -- end

    do_optional_params(params)

    while params.i_valid do
        local regoffset = c_iterator_next(params)
        local regaddr = params.i_base_addr+regoffset
        local result, values
        local do_entry = true
        if params.check ~= nil then
            result, values = myGenWrapper(
                    "cpssDrvPpHwRegisterRead", {
                        { "IN", "GT_U8",  "devNum", params.devID },
                        { "IN", "GT_U32", "portGroupId", params.port_group_id },
                        { "IN", "GT_U32", "regAddr", regaddr },
                        { "OUT","GT_U32", "regValue"}
                    }
                )
            if result == 0 then
                do_entry = c_iterator_check_expr(params, values.regValue)
            else
                do_entry = false
            end
        end
        if do_entry then
            c_iterator_print(params, 0)
            if params.check == nil then
                result, values = myGenWrapper(
                        "cpssDrvPpHwRegisterRead", {
                            { "IN", "GT_U8",  "devNum", params.devID },
                            { "IN", "GT_U32", "portGroupId", params.port_group_id },
                            { "IN", "GT_U32", "regAddr", regaddr },
                            { "OUT","GT_U32", "regValue"}
                        }
                    )
            end
            if result ~= 0 then
                print("failed to read register from "
                        ..string.format("%x",regaddr)
                        ..", err="..tostring(result))
                return false
            end
            --TODO: rm trace
            print(string.format("value    =0x%.8x",values.regValue))
            local newValue
            -- allow to set register value when field description present
            if params.field == nil then
                newValue = params.value
            else
                newValue = do_set_fields(params,unit,values.regValue)
            end
            if newValue == nil then
                return false
            end
            --TODO: rm trace
            print(string.format("new value=0x%.8x",newValue))
            -- TODO: special write register if %customAction != ""
            result = myGenWrapper(
                    "cpssDrvPpHwRegisterWrite", {
                        { "IN", "GT_U8",  "devNum", params.devID },
                        { "IN", "GT_U32", "portGroupId", params.port_group_id },
                        { "IN", "GT_U32", "regAddr", regaddr },
                        { "IN","GT_U32", "regValue", newValue }
                    }
                )
            if result ~= 0 then
                print(string.format("failed to write register addr=%x, err=%d", regaddr, result))
                return false
            end
        end
    end
    return true
end

local function do_set_unit_mem(params,unit)
    do_optional_params(params)

    if not c_iterator_init(
                params,
                get_int_attr(unit, "mem_base", true),
                xml_get_index_info(unit)
            )
    then
        return false
    end
    -- Don't check, allowed to set range/all
    -- -- check for required indices
    -- if not do_set_check_indices(params,unit) then
    --     return false
    -- end

    local mem_entry_width = get_int_attr(unit, "entry_width")
    local u = xml_childs(xml_lookup(unit, "TABLE"))
    while params.i_valid do
        local memoffset = c_iterator_next(params)
        local memaddr = params.i_base_addr+memoffset
        local memvalue
        local do_entry = true
        if params.check ~= nil then
            memvalue = cider_ReadPpMemory(
                    params.devID, -- deviceId
                    params.port_group_id, -- portGroupId
                    memaddr,
                    mem_entry_width)
            if type(memvalue) ~= "number" then
                do_entry = c_iterator_check_expr(params, memvalue)
            else
                do_entry = false
            end
        end
        if do_entry then
            c_iterator_print(params, 0)
            if params.check == nil then
                memvalue = cider_ReadPpMemory(
                        params.devID, -- deviceId
                        params.port_group_id, -- portGroupId
                        memaddr,
                        mem_entry_width)
            end
            if type(memvalue) == "number" then
                print(string.format("failed to read PP memory from %x, err=%d", memaddr, memvalue))
                return false
            end
            print("value    =0x"..cider_DumpUserdata(memvalue))
            u = xml_childs(xml_lookup(unit, "TABLE"))
            local newValue = nil
            -- allow to set register value when field description present
            if params.field == nil then
                newValue = cider_ToUserdata(params.value)
                if newValue == nil or string.len(cider_DumpUserdata(memvalue)) ~= string.len(cider_DumpUserdata(newValue)) then
                    print("The value must be a hex string of the same length as old value")
                    return false
                end
            else
                local ds = nil
                repeat
                    ds = xml_getchild(u, ds)
                    if xml_tagname(ds) == "DS" then
                          newValue = do_set_fields(params,ds,memvalue)
                          if newValue ~= nil then
                              break
                          end
                    end
                until ds == nil
            end
            if newValue == nil then
                return false
            end
            print("new value=0x"..cider_DumpUserdata(newValue))
            local result = cider_WritePpMemory(
                    params.devID, -- deviceId
                    params.port_group_id, -- portGroupId
                    memaddr,
                    mem_entry_width,
                    newValue)
            if result ~= 0 then
                print(string.format("failed to write mem addr=%x, err=%d", memaddr, result))
                return false
            end
        end
    end

    return true
end

local function set_unit(params)
    local unit = cider_u()
    if xml_tagname(unit) == "REGISTER" then
        return do_set_unit_register(params, unit)
    end
    if xml_tagname(unit) == "MEM" then
        return do_set_unit_mem(params, unit)
    end
    print(unitname(unit).." it not REGISTER nor MEM")
    return false
end

local function print_unit_address(unit)
    local p_params={}
    if xml_tagname(unit) == "REGISTER" then
        if not c_iterator_init(
                    p_params,
                    get_int_attr(unit, "addr", true),
                    xml_get_index_info(unit)
                )
        then
            return false
        end
    elseif xml_tagname(unit) == "MEM" then
        if not c_iterator_init(
                    p_params,
                    get_int_attr(unit, "mem_base", true),
                    xml_get_index_info(unit)
                )
        then
            return false
        end
    else
        return
    end
    local addr_offset = c_iterator_next(p_params)
    print(string.format(" address 0x%.8x + 0x%.8x",
                        p_params.i_base_addr,
                        addr_offset))
end

local function unit_path(path,level,unit)
    path[level+1] = cider_unitname(unit)
    return table.concat(path," ",2,level+1)
end
local function do_lookup(func,params,path,level,unit)
    if params.start_level == nil then
        params.start_level = level
    end
    if level == params.start_level then
        params.found_paths = {}
        params.found_types = {}
        params.found_units = {}
    end
    local ret = false
    if level > 0 then
        ret = func(params,path,level,unit)
        if type(ret) == "string" then
            local i
            local p = {}
            for i = 2, level do
                table.insert(p, path[i])
            end
            table.insert(p, cider_unitname(unit))
            table.insert(params.found_paths, p)
            params.found_types[#params.found_paths] = ret
            params.found_units[#params.found_paths] = unit
        end
    end
    if xml_tagname(unit) ~= "UNIT" then
        return ret
    end
    -- Skip PEX_IP_from_MSI and Device_SMI trees
    local unitname = cider_unitname(unit)
    if unitname == "PEX_IP_from_MSI" or unitname == "Device_SMI" then
        return ret
    end
    path[level+1] = unitname
    local unit_inline = xml_childs(unit)
    local iterator = nil
    repeat
        iterator = xml_getchild(unit_inline, iterator)
        if iterator ~= nil then
            local tagname = xml_tagname(iterator)
            if tagname == "UNIT" or tagname == "REGISTER" or tagname == "MEM" then
                do_lookup(func,params,path,level+1,iterator)
            end
        end
    until iterator == nil
    if level == params.start_level then
        -- print result
        if params.no_print_list then
            return
        end
        local i
        if params.print_paths then
            for i = 1, #params.found_paths do
                if params.found_types[i] ~= "" then
                    print(params.found_types[i]..": "..table.concat(params.found_paths[i]," "))
                else
                    print(table.concat(params.found_paths[i]," "))
                end
                print_unit_address(params.found_units[i])
            end
        else
            local regnames = {}
            paths_to_entrynames(params.found_paths,regnames)
            for i = 1, #params.found_paths do
                if params.found_types[i] ~= "" then
                    print(params.found_types[i]..": "..regnames[i])
                    print_unit_address(params.found_units[i])
                end
            end
        end
        if #params.found_paths == 0 then
            print("Nothing found")
        end
    end
end
local function lookup_params(param_path)
    local path = {}
    local level = 0
    if type(param_path) == "table" and #param_path > 0 then
        local i
        path[1] = "ROOT"
        for level = 1, #param_path do
            path[level+1] = param_path[level]
        end
        level = #param_path + 1
    else
        cider_rst()
    end
    return path, level, cider_u()
end


local function lookup_table(params,path,level,unit)
    if xml_tagname(unit) == "MEM" then
        --TODO:print description
        return "Table"
    end
    return nil
end
local function list_tables(params)
    return do_command_with_print_nice(
                "do_lookup()", do_lookup,
                lookup_table,
                params,
                lookup_params(params.cider_tablepath))
end

local function is_counter(unit)
    local nm = xml_tagname(unit)
    if nm == "MEM" or nm == "REGISTER" then
        nm = string.lower(cider_unitname(unit))
        if string.match(nm,"counter") ~= nil then
            return true
        end
    end
    return false
end
local function lookup_counters(params,path,level,unit)
    local nm = xml_tagname(unit)
    if nm == "MEM" or nm == "REGISTER" then
        nm = string.lower(cider_unitname(unit))
        if string.match(nm,"counter") ~= nil then
            --TODO:print description
            return "Counter"
        end
    end
    return nil
end
local function list_counters(params)
    return do_command_with_print_nice(
                "do_lookup()", do_lookup,
                lookup_counters,
                params,
                lookup_params(params.cider_counterpath))
end

local function lookup_register_by_string(params,path,level,unit)
    if xml_tagname(unit) == "REGISTER" then
        local nm = string.lower(cider_unitname(unit))
        if string.find(nm,params.string) ~= nil then
            return "Register"
        end
    end
    return nil
end
local function search_register_by_string(params)
    params.string = string.lower(params.string)
    return do_command_with_print_nice(
                "do_lookup()", do_lookup,
                lookup_register_by_string,
                params,
                {},
                0,
                cider_RootNode()
            )
end

local function lookup_table_by_string(params,path,level,unit)
    if xml_tagname(unit) == "MEM" then
        local nm = string.lower(cider_unitname(unit))
        if string.find(nm,params.string) ~= nil then
            return "Table"
        end
    end
    return nil
end
local function search_table_by_string(params)
    params.string = string.lower(params.string)
    return do_command_with_print_nice(
                "do_lookup()", do_lookup,
                lookup_table_by_string,
                params,
                {},
                0,
                cider_RootNode()
            )
end

local function lookup_counter_by_string(params,path,level,unit)
    local nm = xml_tagname(unit)
    if nm == "MEM" or nm == "REGISTER" then
        local nm = string.lower(cider_unitname(unit))
        if string.match(nm,"counter") ~= nil and string.find(nm,params.string) ~= nil then
            return "Counter"
        end
    end
    return nil
end
local function search_counter_by_string(params)
    params.string = string.lower(params.string)
    return do_command_with_print_nice(
                "do_lookup()", do_lookup,
                lookup_counter_by_string,
                params,
                {},
                0,
                cider_RootNode()
            )
end

local function lookup_entry_by_string(params,path,level,unit)
    local nm = xml_tagname(unit)
    local t = ""
    if nm == "MEM" then
        t = "Table"
    end
    if nm == "REGISTER" then
        t = "Register"
    end
    nm = string.lower(cider_unitname(unit))
    if string.find(nm,params.string) ~= nil then
        return t
    end
    return nil
end
local function search_entry_by_string(params)
    params.string = string.lower(params.string)
    return do_command_with_print_nice(
                "do_lookup()", do_lookup,
                lookup_entry_by_string,
                params,
                {},
                0,
                cider_RootNode()
            )
end

local function cider_name_count(glbl,path,level,unit)
    if level == nil then
        glbl = { name = glbl, count = 0 }
        path = {}
        level = 0
        unit = cider_RootNode()
    end
    local nm = xml_tagname(unit)
    if level > 0 then
        if nm == "REGISTER" or nm == "MEM" then
            nm = cider_unitname(unit)
            if nm == glbl.name then
                glbl.count = glbl.count + 1
            end
            return
        end
    end
    if nm ~= "UNIT" then
        return
    end
    --
    path[level+1] = cider_unitname(unit)
    local unit_childs = xml_childs(unit)
    local iterator = nil
    repeat
        iterator = xml_getchild(unit_childs, iterator)
        if iterator ~= nil then
            local tagname = xml_tagname(iterator)
            if tagname == "UNIT" or tagname == "REGISTER" or tagname == "MEM" then
                cider_name_count(glbl,path,level+1,iterator)
            end
        end
    until iterator == nil
    if level == 0 then
        return glbl.count
    end
end
local function do_search_unit_register_by_address(params,path,upath,level,unit)
    if not c_iterator_init(
                params,
                get_int_attr(unit, "addr", true),
                xml_get_index_info(unit)
            )
    then
        return false
    end

    while params.i_valid do
        local regoffset = c_iterator_next(params)
        local regaddr = params.i_base_addr+regoffset
        if regaddr == params.address then
            params.foundtype = "Register"
            params.foundpath = unit_path(path,level,unit)
            local i
            params.path = {}
            params.upath = {}
            for i = 1,level do
                params.path[i] = path[i]
                params.upath[i] = upath[i]
            end
            params.path[level+1] = cider_unitname(unit)
            params.upath[level+1] = unit
            return true -- found
        end
    end
    return false
end

local function do_search_unit_mem_by_address(params,path,upath,level,unit)
    local mem_base = get_int_attr(unit, "mem_base", true)
    local mem_end = get_int_attr(unit, "mem_end", false)
    if params.address < mem_base or params.address >= mem_base + mem_end then
        return false
    end
    if not c_iterator_init(
                params,
                mem_base,
                xml_get_index_info(unit)
            )
    then
        return false
    end
    local mem_entry_width = get_int_attr(unit, "entry_width")
    local mem_entry_byte_width = 0
    while mem_entry_width > 0 do
        mem_entry_width = mem_entry_width - 32
        mem_entry_byte_width = mem_entry_byte_width + 4;
    end
    -- print(indent(level).."Mem "..cider_unitname(unit)..":")
    local u = xml_childs(xml_lookup(unit, "TABLE"))
    while params.i_valid do
        local memoffset = c_iterator_next(params)
        local memaddr = params.i_base_addr+memoffset
        if params.address >= memaddr and params.address < memaddr + mem_entry_byte_width then
            params.foundtype = "Table"
            params.foundpath = unit_path(path,level,unit)
            local i
            params.path = {}
            params.upath = {}
            for i = 1,level do
                params.path[i] = path[i]
                params.upath[i] = upath[i]
            end
            params.path[level+1] = cider_unitname(unit)
            params.upath[level+1] = unit
            return true -- found
        end
    end
    return false
end

function do_search_unit_by_address(func,params,path,level,unit,upath,multiple)
    if multiple and params.results == nil then
        params.results = {}
    end
    if upath == nil then
        upath = {}
    end
    local tagname = xml_tagname(unit)
    if tagname == "REGISTER" or tagname == "MEM" then
        if func ~= nil and not func(params,path,level,unit) then
            return false
        end
        local v
        if tagname == "REGISTER" then
            v = do_search_unit_register_by_address(params, path, upath, level, unit)
        end
        if tagname == "MEM" then
            v = do_search_unit_mem_by_address(params, path, upath, level, unit)
        end
        if not v or not multiple then
            return v
        end
        local result = {}
        result.foundtype = params.foundtype
        result.foundpath = params.foundpath
        result.path = duplicate(params.path)
        result.upath = duplicate(params.path)
        result.i_indices = duplicate(params.i_indices)
        table.insert(params.results, result)
        return v
    end
    if tagname ~= "UNIT" then
        return false
    end
    --
    --print(indent(level+1).."Doing for unit name=", cider_unitname(unit))
    path[level+1] = cider_unitname(unit)
    upath[level+1] = unit
    local unit_childs = xml_childs(unit)
    local iterator = nil
    repeat
        iterator = xml_getchild(unit_childs, iterator)
        if iterator ~= nil then
            tagname = xml_tagname(iterator)
            if tagname == "UNIT" or tagname == "REGISTER" or tagname == "MEM" then
                if do_search_unit_by_address(func, params,path,level+1,iterator,upath,multiple) then
                    if not multiple then
                        return true
                    end
                end
            end
        end
    until iterator == nil
    if multiple then
        if #params.results > 0 then
            return true
        end
    end
    return false
end

local function do_search_unit_by_address_p(func,params)
    if do_search_unit_by_address(func,params,{},0,cider_RootNode(),{}, true) then
        local no
        for no = 1,#params.results do
            local r = params.results[no]
            local nm
            if params.print_paths then
                nm = r.foundpath
            else
                nm = r.path[#r.path]
                if cider_name_count(nm) > 1 then
                    nm = nm .. "_(" .. table.concat(r.path, "_", 2, #r.path-1)..")"
                end
            end
            if not cider_UT_mode then
                print(r.foundtype..": "..nm)
            end
            -- print index if present
            local idx_string = ""
            local i, idx
            if type(r.i_indices) == "table" then
                for i = 1,#r.i_indices do
                    idx = r.i_indices[i]
                    if idx.do_print then
                        -- also available: idx.label
                        local ii
                        if i==1 then
                            ii = "index "..tostring(idx.current_value)
                        else
                            ii = string.format("index%d ",i)..tostring(idx.current_value)
                        end
                        if not cider_UT_mode then
                            print(ii)
                        else
                            idx_string = idx_string .. " " .. ii
                        end
                    end
                end
            end
            if cider_UT_mode then
                print("###FOUND="..nm..idx_string)
            end
        end

    else
        print("address not found")
    end
end

local function lookup_check_register(params,path,level,unit)
    if xml_tagname(unit) == "REGISTER" then
        return true
    end
    return false
end
local function search_register_by_address(params)
    return do_search_unit_by_address_p(lookup_check_register, params)
end

local function lookup_check_table(params,path,level,unit)
    if xml_tagname(unit) == "MEM" then
        return true
    end
    return false
end
local function search_table_by_address(params)
    return do_search_unit_by_address_p(lookup_check_table, params)
end

local function lookup_check_counter(params,path,level,unit)
    local nm = xml_tagname(unit)
    if nm == "MEM" or nm == "REGISTER" then
        nm = string.lower(cider_unitname(unit))
        if string.match(nm,"counter") ~= nil then
            return true
        end
    end
    return false
end
local function search_counter_by_address(params)
    return do_search_unit_by_address_p(lookup_check_counter, params)
end

local function search_entry_by_address(params)
    return do_search_unit_by_address_p(nil, params)
end

local function do_lookup_field(params,path,parent)
    local fields
    local found = false
    fields = xml_childs(xml_lookup(parent, "FIELDS"))
    if fields == nil then
        return found
    end
    -- walk through all fields
    local field = nil
    repeat
        field = xml_getchild(fields, field)
        if xml_tagname(field) == "FIELD" then
            local v = 0
            local name = cider_getname(field)
            if string.find(string.lower(name),params.string) ~= nil then
                found = true
                print(path.." field "..name)
            end
        end
    until field == nil
    return found
end

local function lookup_field_by_string(params,path,level,unit)
    local nm = xml_tagname(unit)
    if nm == "REGISTER" then
        if do_lookup_field(params,"Register: " .. unit_path(path,level,unit),unit) then
            return "Register"
        end
    end
    if nm == "MEM" then
        local u = xml_childs(xml_lookup(unit, "TABLE"))
        local ds = nil
        local found = false
        repeat
            ds = xml_getchild(u, ds)
            if xml_tagname(ds) == "DS" then
                if do_lookup_field(params,"Table: " .. unit_path(path,level,unit),ds) then
                    found = true
                end
            end
        until ds == nil
        if found then
            return "Table"
        end
    end
    return false
end
local function search_entry_by_field(params)
    params.string = string.lower(params.string)
    params.no_print_list = true
    local ret = do_command_with_print_nice(
                "do_lookup()", do_lookup,
                lookup_field_by_string,
                params,
                {},
                0,
                cider_RootNode()
            )
    if #params.found_paths < 1 then
        print("Nothing found")
    end
    return ret
end

local function show_register(params)
    local k, v
    for k, v in pairs(cider_indices) do
        params[k] = v
    end
    return do_command_with_print_nice(
                "do_show_unit()", do_show_unit,
                params,
                0,
                cider_unittags[#cider_unittags]
            )
end
local function set_register(params)
    local k, v
    for k, v in pairs(cider_indices) do
        params[k] = v
    end
    local unit = cider_unittags[#cider_unittags]
    if xml_tagname(unit) == "REGISTER" then
        return do_set_unit_register(params, unit)
    end
    if xml_tagname(unit) == "MEM" then
        return do_set_unit_mem(params, unit)
    end
    --print(unitname(unit).." it not REGISTER nor MEM")
    return false
end

local function find_register(params)
    if prefix_match("0x", params.string) then
        params.address = tonumber(params.string)
        if params.address == nil then
            return false, "Address must be a hex number"
        end
        return search_register_by_address(params)
    end
    return search_register_by_string(params)
end

local function find_table(params)
    if prefix_match("0x", params.string) then
        params.address = tonumber(params.string)
        if params.address == nil then
            return false, "Address must be a hex number"
        end
        return search_table_by_address(params)
    end
    return search_table_by_string(params)
end

local function find_counter(params)
    if prefix_match("0x", params.string) then
        params.address = tonumber(params.string)
        if params.address == nil then
            return false, "Address must be a hex number"
        end
        return search_counter_by_address(params)
    end
    return search_counter_by_string(params)
end

local function find_all(params)
    if prefix_match("0x", params.string) then
        params.address = tonumber(params.string)
        if params.address == nil then
            return false, "Address must be a hex number"
        end
        return search_entry_by_address(params)
    end
    return search_entry_by_string(params)
end

local function lookup_port_counter(params,path,level,unit)
    local nm = xml_tagname(unit)
    if nm ~= "MEM" and nm ~= "REGISTER" then
        return
    end

    nm = string.lower(cider_unitname(unit))
    if string.match(nm,"counter") == nil then
        return
    end
    local addr = xml_get_index_info(unit)
    if addr == nil then
        return
    end
    local aparams = xml_childs(xml_lookup(addr, "PARAMS"))
    local param = nil
    local found = false
    repeat
        param = xml_getchild(aparams, param)
        if xml_tagname(param) == "PARAM" then
            local label=xml_get_attr(param,"lable")
            if string.lower(label) == "port" then
                local startVal=get_int_attr(param,"startVal")
                local endVal=get_int_attr(param,"endVal")
                if params.port >= startVal and params.port <= endVal then
                    found = true
                end
                break
            end
        end
    until param == nil
    if not found then
        return
    end
    -- save port number: do_show_unit will reset it
    params.counters_found = true
    local p = params.port
    do_show_unit(params,0,unit)
    -- restore port number
    params.port = p
end
local function show_port_counters(params)
    params.no_print_list = true
    params.no_print_addr_raw_value = true
    local ret = do_command_with_print_nice(
                "do_lookup()", do_lookup,
                lookup_port_counter,
                params,
                lookup_params(nil))
    if not params.counters_found then
        print("Wrong port "..tostring(params.port))
    end
    return ret
end

local function do_compare_register_defaults(params,path,level,unit)
    local nm = xml_tagname(unit)
    if nm ~= "REGISTER" then
        return
    end
    -- check if default value present
    local defvalue = xml_get_attr(unit, "default")
    if defvalue == nil then
        return
    end
    if defvalue == "" then
        return
    end
    defvalue = tonumber(defvalue)
    local addr = xml_get_index_info(unit)
    if addr ~= nil then
        -- skip register sets
        return
    end
    local access, warn = cider_check_access(xml_lookup(unit, "FIELDS"), "read")
    if not access then
        return
    end
    if warn then
        print("WARNING: Register "..cider_unitname(unit).." has write only fields")
    end
    -- register readable
    local regaddr = get_int_attr(unit, "addr", true)
    local result, values
    result, values = myGenWrapper(
                "cpssDrvPpHwRegisterRead", {
                    { "IN", "GT_U8",  "devNum", params.devID },
                    { "IN", "GT_U32", "portGroupId", params.port_group_id },
                    { "IN", "GT_U32", "regAddr", regaddr },
                    { "OUT","GT_U32", "regValue"}
                }
            )
    if result ~= 0 then
        --TODO: read failed, print???
        return
    end
    if values.regValue == defvalue then
        -- matched, do nothing
        return
    end
    print("Register "..cider_unitname(unit)..":")
    print(string.format("  address 0x%.8x + 0x%.8x", regaddr, 0))
    print(string.format("  value  =0x%.8x",values.regValue))
    print(string.format("  default=0x%.8x",defvalue))
end
local function compare_registers_defaults(params)
    do_optional_params(params)
    params.no_print_list = true
    params.no_print_addr_raw_value = true
    return do_command_with_print_nice(
                "do_lookup()", do_lookup,
                do_compare_register_defaults,
                params,
                lookup_params(nil))
end

local function do_help_fields(level,parent)
    local fields
    fields = xml_childs(xml_lookup(parent, "FIELDS"))
    if fields == nil then
        return
    end
    -- walk through all fields
    local field = nil
    repeat
        field = xml_getchild(fields, field)
        if xml_tagname(field) == "FIELD" then
            -- display field
            local name = cider_getname(field)
            print(indent(level)..name..":")
            local desc = xml_inlinetext(xml_lookup(field,"DESCRIPTION"))
            if type(desc) == "string" and desc ~= "" then
                local d = xml_dequote(desc)
                d = string.gsub(d, "\n", "\n"..indent(level+2))
                print(indent(level+1).."Description: "..d)
            end
        end
    until field == nil
end

local function do_help_cider(params)
    local unit = cider_unittags[#cider_unittags]
    if xml_tagname(unit) == "REGISTER" then
        print("Register "..cider_unitname(unit)..":")
    end
    if xml_tagname(unit) == "MEM" then
        print("Table "..cider_unitname(unit)..":")
    end
    local desc = xml_inlinetext(xml_lookup(unit, "DESCRIPTION"))
    if type(desc) == "string" and desc ~= "" then
        desc = string.gsub(xml_dequote(desc), "\n", "\n"..indent(3))
        print(indent(2).."Description: "..desc)
    end

    if xml_tagname(unit) == "REGISTER" then
        do_help_fields(1,unit)
        return true
    end
    if xml_tagname(unit) == "MEM" then
        local u = xml_childs(xml_lookup(unit, "TABLE"))
        local ds = nil
        repeat
            ds = xml_getchild(u, ds)
            if xml_tagname(ds) == "DS" then
                print(indent(1).."Section "..cider_getname(ds)..":")
                do_help_fields(2,ds)
            end
        until ds == nil
    end
end

local function help_cider(params)
    return do_command_with_print_nice(
                "do_help_cider()", do_help_cider,
                params)
end

local function help_compare(params)
    print("Please enter full command")
end

CLI_type_dict["cider_expr_op"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="comparison operator",
    enum = {
        ["eq"] = { value="eq", help="Equal to" },
        ["ne"] = { value="ne", help="Not equal to" },
        ["lt"] = { value="lt", help="Less than" },
        ["gt"] = { value="gt", help="Greater than" },
        ["le"] = { value="le", help="Less or equal than" },
        ["ge"] = { value="ge", help="Greater or equal than" }
    }
}



function register_cider_commands()


    if    true == cider_initialized then
        return
    end
        
    cider_initialized = true
    


    CLI_addParamDict("cider_index", {
        { format="port %number", help="port index (if applicable)" },
        { format="index %number", name="_index", help="table index (1st)" },
        { format="range %number %number", name="_index", help="index range (1st)",
            multiplevalues = true},
        { format="index2 %number", name="_index2",  help="table index (2nd)" },
        { format="range2 %number %number", name="_index2", help="index range (2nd)",
            multiplevalues = true},
        { format="check %cider_field_to_check %cider_expr_op %cider_fieldvalue",
            multiplevalues = true, help="check that field match/doesn't match value" },
        { format="field %cider_fieldname", help="cider field name" },
    })
    -- show unit @cider_unitname [field %cider_fieldname] [index %number] [index2 %number] \
    --    [check %cider_field_to_check %cider_expr_op %cider_fieldvalue ]
    CLI_addCommand("cider", "show unit", {
        func=show_unit,
        params={
            { type="values",
              { format="@cider_unitname", allowduplicate=true }
            },
            { type="named",
              "#cider_index",
            }
        }
    })
    -- set unit @cider_unitname (field %cider_fieldname) (value %number) [index %number] [index2 %number]
    CLI_addCommand("cider", "set unit", {
        func=set_unit,
        params={
            { type="values",
              { format="@cider_unitname", allowduplicate=true }
            },
            { type="named",
              "#cider_index",
              { format="value %cider_fieldvalue", help="field value" },
              mandatory = { "value" }
            }
        }
    })
    CLI_addHelp("cider", "set", "Set registers/tables described in cider XML")
    -- list tables [@cider_unitpath]
    CLI_addCommand("cider", "list tables", {
        help="List (all) tables",
        func=list_tables,
        params={{type="values",
            { format="@cider_tablepath", allowduplicate="true", allowempty=true, help="path to table" }
        }}
    })
    -- show table @cider_tablepath [field %cider_fieldname] [index %number] [index2 %number]
    CLI_addCommand("cider", "show table", {
        func=function(params)
            local unit = cider_u()
            if xml_tagname(unit) ~= "MEM" then
                print("not a table")
                return false
            end
            return show_unit(params)
        end,
        params={
            { type="values",
              { format="@cider_tablepath", name="cider_unitname", allowduplicate=true }
            },
            { type="named",
              "#cider_index",
            }
        }
    })
    -- show counter @cider_counterpath [field %cider_fieldname] [index %number] [index2 %number]
    CLI_addCommand("cider", "show counter", {
        func=function(params)
            local unit = cider_u()
            if not is_counter(unit) then
                print("not a counter")
                return false
            end
            return show_unit(params)
        end,
        params={
            { type="values",
              { format="@cider_counterpath", name="cider_unitname", allowduplicate=true }
            },
            { type="named",
              { format="field %cider_fieldname", help="cider field name" }
            }
        }
    })
    -- list counters [@cider_counterpath]
    CLI_addHelp("cider", "list", "List entries in cider XML")
    CLI_addCommand("cider", "list counters", {
        help="List (all) counters",
        func=list_counters,
        params={{type="values",
            { format="@cider_counterpath", allowduplicate="true", allowempty=true, help="path to counter" }
        }}
    })
    CLI_addHelp("cider", "find", "Search in cider XML")
    -- find register %string
    CLI_addCommand("cider", "find register", {
        func=find_register,
        help="Search register by string/address",
        params={{type="values",
            { format="%string", name="string", help="the register address or name substring to match" }
          },
          {type="named",
            { format="path", name="print_paths", help="Display the full path (unit,sub units...)" }
        }}
    })
    -- find table %string
    CLI_addCommand("cider", "find table", {
        func=find_table,
        help="Search table by string/address",
        params={{type="values",
            { format="%string", name="string", help="the table address or name substring to match" }
          },
          {type="named",
            { format="path", name="print_paths", help="Display the full path (unit,sub units...)" }
        }}
    })
    -- find counter %string
    CLI_addCommand("cider", "find counter", {
        func=find_counter,
        help="Search counter by string/address",
        params={{type="values",
            { format="%string", name="string", help="the counter address or name substring to match" }
          },
          {type="named",
            { format="path", name="print_paths", help="Display the full path (unit,sub units...)" }
        }}
    })
    -- find all %string
    CLI_addCommand("cider", "find all", {
        func=find_all,
        help="Search register/table/counter by string/address",
        params={{type="values",
            { format="%string", name="string", help="the entry address or name substring to match" }
          },
          {type="named",
            { format="path", name="print_paths", help="Display the full path (unit,sub units...)" }
        }}
    })
    -- find %string - the same as "find all %string"
    CLI_addCommand("cider", "find", {
        func=find_all,
        help="Search register/table/counter by string/address",
        params={{type="values",
            { format="%string", name="string", help="The entry address or name substring to match" }
          },
          {type="named",
            { format="path", name="print_paths", help="Display the full path (unit,sub units...)" }
        }}
    })  
    -- find field %string
    CLI_addCommand("cider", "find field", {
        func=search_entry_by_field,
        help="Search register/table/counter by field",
        params={{type="values",
            { format="%string", name="string", help="the substring to match" }
        }}
    })
    -- help-cider %cider_register_name
    CLI_addCommand("cider", "help-cider", {
        func=help_cider,
        help="Show register descriptions",
        params={{type="values",
              "%cider_register_name"
        }}
    })
    -- read %cider_register_name [field %cider_fieldname] [index %number] [index2 %number]
    CLI_addCommand("cider", "read", {
        func=show_register,
        help="Read registers/tables",
        params={
            { type="values",
              "%cider_register_name"
            },
            { type="named",
              "#cider_index",
            }
        }
    })
    -- read device %devID %cider_register_name [field %cider_fieldname] [index %number] [index2 %number]
    CLI_addCommand("cider", "read device", {
        func=show_register,
        params={
            { type="values",
              "%devID",
              "%cider_register_name"
            },
            { type="named",
              "#cider_index",
            }
        }
    })
    -- read portGroup %number %cider_register_name [field %cider_fieldname] [index %number] [index2 %number]
    CLI_addCommand("cider", "read portGroup", {
        func=show_register,
        params={
            { type="values",
              { format="%number", name="port_group_id", help="port group id" },
              "%cider_register_name"
            },
            { type="named",
              "#cider_index",
            }
        }
    })
    -- write %cider_register_name (field %cider_fieldname) (value %number) [index %number] [index2 %number]
    CLI_addCommand("cider", "write", {
        func=set_register,
        help="Write registers/tables",
        params={
            { type="values",
              "%cider_register_name"
            },
            { type="named",
              "#cider_index",
              { format="value %cider_fieldvalue", help="field value" },
              mandatory = { "value" }
            }
        }
    })
    -- write device %devID %cider_register_name (field %cider_fieldname) (value %number) [index %number] [index2 %number]
    CLI_addCommand("cider", "write device", {
        func=set_register,
        params={
            { type="values",
              "%devID",
              "%cider_register_name"
            },
            { type="named",
              "#cider_index",
              { format="value %cider_fieldvalue", help="field value" },
              mandatory = { "value" }
            }
        }
    })
    -- write portGroup %number %cider_register_name (field %cider_fieldname) (value %number) [index %number] [index2 %number]
    CLI_addCommand("cider", "write portGroup", {
        func=set_register,
        params={
            { type="values",
              { format="%number", name="port_group_id", help="port group id" },
              "%cider_register_name"
            },
            { type="named",
              "#cider_index",
              { format="value %cider_fieldvalue", help="field value" },
              mandatory = { "value" }
            }
        }
    })
    -- show port counters [device %devID] port %number
    CLI_addCommand("cider", "show port counters", {
        func=show_port_counters,
        params={
            { type="named",
              { format="device %devID", name="devID", help="device ID" },
              { format="portGroup %number", name="port_group_id", help="port group id" },
              { format="port %number", help="port number" },
              mandatory={"port"}
            }
        }
    })
    -- compare registers defaults [device %devID]
    CLI_addCommand("cider", "compare", {
        func=help_compare,  
        help = "Compare registers values with defaults",
    })  
    CLI_addCommand("cider", "compare registers defaults", {
        func=compare_registers_defaults,
        params={
            { type="named",
              { format="device %devID", name="devID", help="device ID" },
              { format="portGroup %number", name="port_group_id", help="port group id" }
            }
        }
    })
    -- set cider portGroup %number
    CLI_addCommand("cider", "set cider portGroup", {
        func = function(params)
            cider_port_group_id = params.port_group_id
        end,
        help = "set port group id for multicore devices",
        params={
            { type="values",
              { format="%number", name="port_group_id", help="port group ID" }
            }
        }
    })
    -- show cider portGroup
    CLI_addCommand("cider", "show cider portGroup", {
        func = function(params)
            print("Current portGroup="..tostring(cider_port_group_id))
        end,
        help = "show current port group id for multicore devices"
    })
    CLI_addCommand("cider", "more", {
        func = print_nice_enable,
        help = "Enable output pausing after 20 lines"
    })
    CLI_addCommand("cider", "no more", {
        func = print_nice_disable,
        help = "Disable output pausing"
    })
    CLI_addCommand("cider", "details", {
        func = function(params)
            cider_print_details = true
        end,
        help = "Print detailed field values"
    })
    CLI_addCommand("cider", "no details", {
        func = function(params)
            cider_print_details = false
        end,
        help = "Don't print detailed field values"
    })
end




-- ************************************************************************
---
--  check_xmlFile
--        @description  ckecks xml file name
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--
--        @return       true and portNum on success, otherwise false and
--                      error message
--


local function check_xmlFile(param, name)
    if ".xml" == string.sub(param, -4) and param:len()>4 then
        return true, param
    else
        return false, "Please enter valid xml file name"
    end  
end





-------------------------------------------------------
-- type registration: xmlFile
-------------------------------------------------------
CLI_type_dict["xmlFile"] =
{
    checker  = check_xmlFile,
    help     = "Enter xml file name"
}



if cider_is_loaded() then
    register_cider_commands()
    
-- load at startup if cider.xml present
elseif lookup_cider_xml() then
    cider_initialize()
end

    CLI_prompts["cider"] = "(debug-cider)#"    
    CLI_addCommand("debug", "cider", {
        func=CLI_change_mode_push,
        constFuncArgs={"cider"},
        help="Use cider commands"
    })

    CLI_addCommand("cider", "load", {
        help = "Load cider.xml, initialize 'show unit ...'",
        func = cider_initialize,
          params={
    {type="named",
      { format="%xmlFile", name="xmlFile" },

    }
  }
        })

        

end
