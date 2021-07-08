--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* handleMultiLine.lua
--*
--* DESCRIPTION:
--*
--* FILE REVISION NUMBER:
--*       $Revision: 26 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
--

local cpssAPI_name = nil
local cpssAPI_tag = nil
local cpssAPI_params = {}
local cpssAPI_callP = {}
local cpssAPI_current_param = 0
local cpssAPI_current_param_s = 0
local cpssAPI_ignorecase = true
local cpssAPI_numberOfStructs = 0
local originalFD, toLoadFD
local function multiLine_cpssAPI_rst()
    cpssAPI_name = nil
    cpssAPI_tag = nil
    cpssAPI_params = {}
    cpssAPI_callP = {}
    cpssAPI_current_param = 0
    cpssAPI_current_param_s = 0
end


local function xml_dequote(s)
    s = string.gsub(s, "&lt;", "<")
    s = string.gsub(s, "&gt;", ">")
    s = string.gsub(s, "&amp;", "&")
    return s
end

local function toboolean(v)
    return (type(v) == "string" and v == "true") or (type(v) == "number" and v ~= 0) or (type(v) == "boolean" and v)
end


local function get_member(tbl, path)
    if tbl == nil or path == nil then
        return nil
    end
    local p = string.find(path, ".",1,true)
    if p == nil then
        return tbl[path]
    end
    local t = tbl[string.sub(path,1,p-1)]
    if type(t) ~= "table" then
        return nil
    end
    return get_member(t, string.sub(path,p+1))
end


local function set_member(tbl, path,val)
    local p = string.find(path, ".",1,true)
    if p == nil then
        tbl[path] = val
        return
    end
    local t = tbl[string.sub(path,1,p-1)]
    if type(t) ~= "table" then
        return
    end
    return set_member(t, string.sub(path,p+1),val)
end

-- ************************************************************************
---
--  getNeededArgumentsForAPI
--        @description  search for the wanted API and gets its arguments
--            on success, cpssAPI_params contains the needed arguments
--
--        @param        currAPI - API name
--
--

local function getNeededArgumentsForAPI(currAPI)
	local p, s, v
	cpssAPI_tag = cpssAPI_lookup(currAPI, cpssAPI_ignorecase)
	if cpssAPI_tag == nil then
		return "No description found for function "..currAPI
	end
	cpssAPI_name = xml_get_attr(cpssAPI_tag, "name")
	-- parse params section
	local params = xml_lookup(cpssAPI_tag, "Params")
	if params == nil then
		-- No params section, assume cpssAPI_name(void)
		return true, currAPI, true
	end
	local iterator = nil
	repeat
		iterator = xml_getchild(params, iterator)
		if iterator == nil then
			break
		end
		if xml_tagname(iterator) == "Param" then
			p = {}
			p.name = xml_get_attr(iterator, "name")
			p.dir  = xml_get_attr(iterator, "dir")
			p.type = xml_get_attr(iterator, "type")
			p.cls  = xml_get_attr(iterator, "class")
			p.special = xml_get_attr(iterator, "special")
			-- add description
			p.desc = xml_inlinetext(xml_lookup(iterator,"Description"))
			if type(p.desc) == "string" then
				p.desc = xml_dequote(p.desc)
			end
			table.insert(cpssAPI_params, p)
		end
	until iterator == nil
	return
end

local function createStringToGenericWrapper(currArg)
	local s,v
	if currArg.cls == "struct" then
		table.insert(cpssAPI_callP, { "s"..currArg.dir, currArg.type, currArg.name})
	else
		table.insert(cpssAPI_callP, { currArg.dir, currArg.type, currArg.name })
	end
end

local function removeNewLineInString(str)
		local loc = string.find(str, "\r\n")
		if loc == nil then
			loc = string.find(str, "\n")
		end
		if loc ~= nil then
			str = string.sub(str, 1 , loc-1)
		end
		return str
end



-- ************************************************************************
---
--  multiLine_readStructMember
--        @description  read structure member
--
--        @param pname              - parameter name
--        @param mname              - member name
--        @param mtype              - member type
--        @param mclass             - member calss (int|bool|enum|struct|string)
--        @param val                - previous value
--
--        @return        new value
--
local function multiLine_readStructMember(pname,mname,mtype,mclass,val)
    local s
    local prompt = pname .. "." .. mname
    if mclass == "struct" then
        return multiLine_readStructParam(mtype,prompt,val)
    elseif mclass == "string" then
        s = fs.gets(originalFD)
        if s ~= "" then
            return s
        end
        return val
    else -- mclass == "int", "bool", "enum"
        local m_compl_name = mname
        local m_compl_type = mtype
        local m_compl_class = mclass
        s = fs.gets(originalFD)
        if s == nil then
                return nil
        end

        s = removeNewLineInString(s)

        if s ~= "" then
            if mclass == "bool" then
                local d = CLI_type_dict["bool"]
                local st,v = d.checker(s, mname, d)
                if st then
                    return v
                end
            elseif mclass == "int" then
                if type(tonumber(s)) == "number" then
                    return tonumber(s)
                end
                if prefix_match("CPSS", s) then
                    local st, v
                    st, v = CLI_check_param_enum(s, mname, cpssConst)
                    if st then
                        return v
                    end
                end
            end
            return s
        end
        return val
    end
end

-- ************************************************************************
---
--  multiLine_readStructParam
--        @description  read parameter which is a structure
--
--        @param ptype              - parameter type(structure name)
--        @param pname              - parameter name
--        @param val                - previous value
--
--        @return        new value
--
function multiLine_readStructParam(ptype,pname,val)
    local prompt, special, s
    if val == nil then
        val = cpssGenWrapperCheckParam(ptype, nil)
    end
    local struct = cpssAPI_getstruct(ptype)
    if struct == nil then
        print("pname: no description for type "..ptype)
        return val
    end
    -- special structures
    local special = xml_get_attr(struct, "special")
    if special == "unused" then
        return val
    end
    if special == "string" then
        local prompt = pname.."("..val..")>"
        local s = fs.gets(originalFD)
        if s == "" then
            return val
        end
        return s
    end
    local member = nil
    repeat
        member = xml_getchild(struct, member)
        if member == nil then
            break
        end
        local mname = xml_get_attr(member,"name")
        local mtype = xml_get_attr(member,"type")
        local mclass = xml_get_attr(member,"class")
        local marr= tonumber(xml_get_attr(member,"array"))
        local v = get_member(val, mname)
        if v ~= nil then
            -- applicable
            if type(marr) == "number" then
                -- array
                local i
                if type(v) ~= "table" then
                    v = {}
                end
                for i = 0, marr-1 do
                    v[i] = multiLine_readStructMember(pname,string.format("%s[%d]",mname,i),mtype,mclass,v[i])
                    if v[i] == nil then
                        return nil
                    end
                    set_member(val, mname, v[i])
                end
            else
                v = multiLine_readStructMember(pname, mname,mtype,mclass,v)
				if v == nil then
					return nil
				end

            end
            set_member(val, mname, v)
        end
        if special == "union" then
            local nval = cpssGenWrapperCheckParam(ptype, val)
            val = nval
        end

    until member == nil

    return val
end


local function fillStructs()
    -- read structure parameters
    local has_struct_param = false
    local i, p, info
    for i=1,#cpssAPI_callP do

        p = cpssAPI_callP[i]
        info = cpssAPI_params[i]
        local key = nil
        local union_mname = nil
        local union_mtype = nil
        if info.special == "UNT" then
            -- Special case: UNT type, depend on other params
            local stc = cpssAPI_getstruct(info.type)
            if stc ~= nil then
                local kt = xml_get_attr(stc, "keyType")
                local t
                for t=1,#cpssAPI_callP do
                    if cpssAPI_callP[t][2] == kt and cpssAPI_callP[t][1] ~= "OUT" then
                        key = cpssAPI_callP[t][4]
                    end
                end
                if key ~= nil then
                    -- key found
                    local member = nil
                    repeat
                        member = xml_getchild(stc, member)
                        if member == nil then
                            break
                        end
                        local mkey = xml_get_attr(member,"key")
                        if mkey == key then
                            union_mname = xml_get_attr(member,"name")
                            union_mtype = xml_get_attr(member,"type")
                            break
                        end
                    until member == nil
                end
            end
        end

        if p[1] == "sIN" or p[1] == "sINOUT" then
            p[1] = string.sub(p[1],2)
            if union_mname == nil then
                p[4] = multiLine_readStructParam(p[2], p[3])
                if p[4] == nil then
                    return nil
                end
            else
                local v = multiLine_readStructParam(union_mtype, p[3].."."..union_mname)
                if v == nil then
                    return nil
                end
                p[4] = {}
                p[4][union_mname] = v
            end
            has_struct_param = true
        end

        if union_mname ~= nil then
            p[2] = p[2].."_"..union_mname
        end

    end
    return true
end

local function renameFile(from, to)
    local line, e
    local fromFD, toFD
    fromFD, e = fs.open(from, "r")
    if e ~= nil then
        return e
    end
    toFD, e = fs.open(to, "w+")
    if e ~= nil then
        fs.close(fromFD)
        return e
    end
    while true do
        line = fs.gets(fromFD)
        if line == nil then
            break
        end
        fs.write(toFD,line)
    end
    fs.unlink(fromFD)
    fs.close(fromFD)
    fs.close(toFD)
end

-- ************************************************************************
---
--  handle_multi_line
--        @description  search for cpss-api call in the configuration file
--						and replace it to a loadable format
--
--        @return       nil on success
--						error message on fail
--

function handle_multi_line(param)
    local splitedWords
    local line, e
    local strReadyToGenericWrapper
    originalFD, e = fs.open(param, "r")
    if e ~= nil then
        return e
    end
    toLoadFD, e = fs.open("configToLoad.txt", "w+")
    if e ~= nil then
        fs.close(originalFD)
        return e
    end

    -- runs over the file (config.txt) and search for cpss-api command to replace
    while true do
        line = fs.gets(originalFD)
        if line == nil then
            break
        end


        splitedWords = splitline(line)
        -- support one command in few lines, using the character "\"
        while (splitedWords[#splitedWords] == "\\") do
            local nextLine = fs.gets(originalFD)

            local lineWithoutBackSlash = ""
            for i = 1, #splitedWords-1 do
                lineWithoutBackSlash = lineWithoutBackSlash .. splitedWords[i] .. " "
            end
            line = lineWithoutBackSlash .. nextLine
            splitedWords = splitline(line)
        end

        if (#splitedWords>2 and splitedWords[1] == "cpss-api" and splitedWords[2] == "call") then

            -- found cpss-api call command
            e = getNeededArgumentsForAPI(splitedWords[3])
            if e ~= nil then
                -- API does not exist
                fs.close(originalFD)
                fs.close(toLoadFD)
                return e
            end

            for i = 1, #cpssAPI_params do
                local p = cpssAPI_params[i]
                createStringToGenericWrapper(p)
            end

            local cpssAPI_paramsIndex = 1
            -- runs over all the needed parameters and fill up the input

            for i = 4, #splitedWords, 2 do
                while (cpssAPI_callP[cpssAPI_paramsIndex][1] == "sIN" or cpssAPI_callP[cpssAPI_paramsIndex][1] == "sINOUT") do
                    -- in case of a struct, will fill it later
                    cpssAPI_numberOfStructs = cpssAPI_numberOfStructs + 1
                    cpssAPI_paramsIndex = cpssAPI_paramsIndex + 1
                end
                if (splitedWords[i] == cpssAPI_callP[cpssAPI_paramsIndex][3]) then
                    local value
                    if (splitedWords[i+1] == "false" or splitedWords[i+1] == "true") then
                        value = toboolean(splitedWords[i+1])
                    else
                        value = tonumber(splitedWords[i+1])
                    end
                    if value == nil then
                        value = splitedWords[i+1]
                    end
                    cpssAPI_callP[cpssAPI_paramsIndex][4] = value
                    cpssAPI_paramsIndex = cpssAPI_paramsIndex + 1
                else
                    fs.close(originalFD)
                    fs.close(toLoadFD)
                    return "wrong input, arguments mismatch the API \n"
                end
            end
            -- fill the input for the structs, based on the next lines in the file

            local filled = fillStructs()
            if filled == nil then
                fs.close(originalFD)
                fs.close(toLoadFD)
                return "wrong number of arguments \n"
            end
            fs.write(toLoadFD, "callCpssAPI "..cpssAPI_name.."\n<<<PARAMS\n"..to_string(cpssAPI_callP).."\n>>>\n")
            multiLine_cpssAPI_rst()
        else
            fs.write(toLoadFD, line)
        end
    end
    fs.close(originalFD)
    fs.close(toLoadFD)
    renameFile("configToLoad.txt",param)
    return true
end























