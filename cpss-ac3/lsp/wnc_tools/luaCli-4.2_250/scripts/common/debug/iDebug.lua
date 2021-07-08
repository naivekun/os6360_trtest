--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* iDebug.lua
--*
--* DESCRIPTION: updated to work without needing to understand how idebug itself works.
--* 		Uses interface Unique names rather than instances
--* 		Updated by Amir Mazor 8/30/17
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssIsAsicSimulation")
--constants

--
-- Types defined:
--
--
--
do

local DEVEL = false -- set to true for simulation (ignore register access)
local DEBUG = false

local iDebug_initialized = false
local register_iDebug_commands
local iDebug_list_file = "dxCh/iDebug_list.txt"
local iDebug_xml = "iDebug.xml"
local iDebug_ignorecase = false

local iDebug_root = nil
local iDebug_CPU = nil
local iDebug_dfxSelector_addr = 0x044f8000 -- puma3
local iDebug_Instances = nil
local iDebug_InterfacesList = nil
local iDebug_checkInstance = function()
    return true
end
local iDebug_devID = nil
local iDebug_Instance = nil
local iDebug_dfxPipe = nil
local iDebug_Interfaces = nil
local iDebug_Interface = nil
local iDebug_InterfaceDesc = nil
local iDebug_instanceOnDirectInterface = nil
local listOfAvailableInterfaces = {}
local	interfaceTable = {}

local uniqueInterfaceTable = {}
local instances = {}
local cpuTable = {}
local possibleInterfaceCount = 0
local hideEnabledInterfaces = false
local currentInterface = nil
local currentField = nil
local enabledInstances = {}
local checkTable = {}
local enabledEnum = {[-1] = "false", [0] = "false", [1] = "true"}

--------------------------------------------
-----------------Tools----------------------
--------------------------------------------




--------------------------------------------
-- Prefix Match
--
-- returns true if prefix exists at the beginning of string
--
-- input:
-- 	prefix:string 	- the prefix to be checked if it exists in str
-- 	str:string 	- string that is being checked if the prefix exists in it
-- 
-- output:
--	boolean	
--
--------------------------------------------
--------------------------------------------
local function prefix_matchi(prefix, str)
    local prf = string.sub(str, 1, string.len(prefix))
    if not iDebug_ignorecase then
        return prf == prefix
    end
    return string.lower(prf) == string.lower(prefix)
end

--------------------------------------------
-- Suffix Match
--
-- returns true if suffix exists at the end of string
--
-- input:
-- 	suffix:string 	- the suffix to be checked if it exists in str
-- 	str:string 	- string that is being checked if the suffix exists in it
-- 
-- output:
--	boolean	
--
--------------------------------------------
--------------------------------------------
local function suffix_matchi(suffix, str)
    local post = string.sub(str, #str - #suffix+1, #str)
    if not iDebug_ignorecase then
        return post == suffix
    end
    return string.lower(post) == string.lower(suffix)
end

--------------------------------------------
-- String Equal
--
-- returns true if suffix exists at the end of string
--
-- input:
-- 	suffix:string 	- the suffix to be checked if it exists in str
-- 	str:string 	- string that is being checked if the suffix exists in it
-- 
-- output:
--	boolean	
--
--------------------------------------------
--------------------------------------------
local function streqi(str1, str2)
    if not iDebug_ignorecase then
        return str1 == str2
    end
    return string.lower(str1) == string.lower(str2)
end


--------------------------------------------
-- Sort By Member
--
-- Sorts table by member name in alphabetical order
--
-- input:
-- 	Table:table			- The table to be sorted
-- 	MemberName:string 	- A direct member of Table that contains a value of type string
-- 	bAsc:boolean 			- if true then will order table in ascending A-Z order, otherwise in Z-A order by member value 
-- 
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function sort_by_member( Table, MemberName, bAsc )
	local TableMemberSort = function( a, b, MemberName, bReverse ) 
		if ( type( a[ MemberName ] ) == "string" ) then
			if ( bReverse ) then
				return a[ MemberName ]:lower() < b[ MemberName ]:lower()
			else
				return a[ MemberName ]:lower() > b[ MemberName ]:lower()
			end
		end
		if ( bReverse ) then
			return a[ MemberName ] < b[ MemberName ]
		else
			return a[ MemberName ] > b[ MemberName ]
		end
	end
	table.sort( Table, function( a, b ) return TableMemberSort( a, b, MemberName, bAsc or false ) end )
end


--------------------------------------------
-- Deep Copy
--
-- returns a deep copy of any lua value - should specifically be used for lua Tables
--
-- input:
-- 	orig:_ - the variable for a deep copy to be made

-- 
-- output:
--	a copy of the original value
--
--------------------------------------------
--------------------------------------------
local function deep_copy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[deep_copy(orig_key)] = deep_copy(orig_value)
        end
        setmetatable(copy, deep_copy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

--------------------------------------------
-- table.contains
--
-- checks if a table contains a certain element
--
-- input:
-- 	tble:table 	- The table to check if it contains an element
-- 	element:_	 - The element used to check if the table contains it
-- 
-- output:
--	boolean
--
--------------------------------------------
--------------------------------------------
function table.contains(tbl, element)
  local vals
  for i, vals in pairs(tbl) do
    if vals ~= nil then
      if vals == element then
        return true
      end
    end
  end
  return false
end

--------------------------------------------
-- Recursive Print
--
-- prints all the values in a table recursively. mainly used for debugging purposes.
--
-- input:
-- 	tb:table - the table to recursively print
-- 
-- output:
--	none (prints to console)
--
--------------------------------------------
--------------------------------------------
local function rec_print(tb)
  function inner(tb, depth)
    depth = depth + 1
    for k, v in pairs(tb) do
      if(type(v) == "table") then
        for i = 2, depth do 
          io.write('     ')
        end
        print("index: " ..  k)
        inner(v, depth)
      else 
        for i = 2, depth do
          io.write('     ')
        end
        if(type(v) == "userdata") then
        	print("index: " .. k,"value: USERDATA")
        else
        	print("index: " .. k,"value: "..  tostring(v))
	end
      end
    end
  end


  local depth = 1
  inner(tb, depth)


end


--------------------------------------------
-- Initialize Data
--
-- Initalizes string of given length of bits "size" to a proper
-- HEX string of that size (in increments of 32bits/4bytes) 
--
-- input:
-- 	size:number		- size of the string
--	set_all:boolean 	- sets all to true if flag is true
-- 
-- output:
--	initialized string
--
--------------------------------------------
--------------------------------------------
local function init_data(size, set_all)
    local w = "00000000"
    if set_all then w = "FFFFFFFF" end
    if size < 33 then
        return w
    end
    local s = ""
    while size > 31 do
        s = s .. w
        size = size - 32
    end
    if size > 0 then
        s = s .. w
    end
    return s
end


--------------------------------------------
-- handle completion long list
--
-- Used in lua dictionary completion function. Adds combines
-- options of the completion list that have similar prefixes with an
-- asterisk
--
-- input:
-- 	param:string	- the string prefix used to complete the 
--					possible parameters
--	compl:table 	- table of strings of which the prefix exists 
--	help:table 	- table that contains "def" index which contains a help string
-- 	defhelp:string	- string that will be put in help table
-- 
-- output:
--	table of list with asterisks used to combine names that are too long that are also similar
--
--------------------------------------------
--------------------------------------------
local function handle_completion_long_list(param, compl, help, defhelp)
    if #compl < 21 then
        return compl, help
    end
    local c = duplicate(compl)
    table.sort(c)
    local ncompl = {}
    local short, short_prev
    local o, i
    short_prev = nil
    for i = 1, #c do
        if i == 1 then
            o = c[i]
        else
            if string.len(c[i]) < string.len(o) then
                o = string.sub(o, 1, string.len(c[i]))
            end
            while o ~= "" do
                if not prefix_match(o, c[i]) then
                    o = string.sub(o, 1, string.len(o)-1)
                else
                    break
                end
            end
        end
        short = string.match(c[i], param .. "[%a%d]*")
        if short == nil then
            short = string.match(c[i], param .. "%d+")
        end
        if short == nil then
            return compl, help
        end
        if short ~= c[i] then
            short = short .. "*"
        end
        if short ~= short_prev then
            table.insert(ncompl, short)
            short_prev = short
        end
    end
    if o == param then
        return ncompl, { def = defhelp }
    end
end

--------------------------------------------
-- Pairs function by Keys
--
-- iterates through a table by alphabetical order of the key (index)
--
-- input:
-- 	t:table	- table to iterate
--	f:function- seperate function to iterate table by
-- 
-- output:
--	iterator
--
--------------------------------------------
--------------------------------------------
local function pairs_by_keys (t, f)
      local a = {}
      for n in pairs(t) do table.insert(a, n) end
      table.sort(a, f)
      local i = 0      -- iterator variable
      local iter = function ()   -- iterator function
        i = i + 1
        if a[i] == nil then
        	return nil
        else
        	return a[i], t[a[i]]
        end
        
      end
      return iter
end

--------------------------------------------
-- Pairs by Bit Start
--
-- iterates through a table by bitStart order (used in debugging)
-- should only be used with parsedValueTable
--
-- input:
-- 	tab:table			- table to iterate through
--	isReverse:boolean	- if true iterates through highest bitrate first
-- 
-- output:
--	initialized string
--
--------------------------------------------
--------------------------------------------
local function pairs_by_bit_start(tab, isReverse)
	local a = {}
	for n, v in pairs(tab) do table.insert(a, {name = n, bitStart = tonumber(v.bitStart)}) end
	sort_by_member(a,"bitStart", isReverse)
	
      local i = 0      -- iterator variable
      local iter = function ()   -- iterator function
			        i = i + 1
			        if a[i] == nil then
			        	return nil
			        else
			        	return a[i].name, tab[a[i].name]
			        end
			        
			   end
      return iter
end

--------------------------------------------
-- Dump Sample
--
-- prints sample in 32 bit increments followed by a newline.
-- used for debugging
--
-- input:
-- 	sample:string	- sample in hex string
-- 
-- output:
--	none(pritns to console)
--
--------------------------------------------
--------------------------------------------
local function dumpSample(sample)
	local data, i
	print("Sample Dump:")
	print()
	for  i = 0, #sample*4, 32 do 
		data = cider_GetBits(cider_ToUserdata(sample), i+31, i, 32)	
		print(string.format("0x%08x", data))
	end
end
------------------------------------------------
--------------------END Tools--------------------
------------------------------------------------

--------------------------------------------
-- Interface Mode
--
--changes mode from iDebug -> iDebug-<selected-interface>
--
-- input:
-- 	interfacename:string - the name of the interface to be changed to
-- 
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function interface_mode(interfaceName)
	currentInterface = interfaceName
	if (uniqueInterfaceTable[currentInterface].isSynced) then
		CLI_prompts["idebug-select-interface"] = "(idebug-interface-"..interfaceName ..")#"
	else 
		CLI_prompts["idebug-select-interface"] = "(idebug-interface-"..interfaceName .." - *UNSYNCED*)#"
	end
	CLI_change_mode_push("idebug-select-interface")
end

-- helper function used for the CLI
local function interface_mode_helper(params)
	local interfaceName = params.interfaceName
	interface_mode(interfaceName)
end


--------------------------------------------
-- Print Interface With Fields
--
-- prints interface information and max values of all fields
--
-- input:
-- 	uniqueName:string - the name of the interface to print
-- 
-- output:
--	none(prints to console)
--
--------------------------------------------
--------------------------------------------
local function print_Interface_w_fields(uniqueName)

	local id = uniqueInterfaceTable[uniqueName].id
	print()
	print("instance: "..uniqueInterfaceTable[uniqueName].instanceName)
	print("id: "..id)
	print("is enabled: ".. enabledEnum[uniqueInterfaceTable[uniqueName].enabled])
	print("size: " .. interfaceTable[id].size)
	print()
	local str = ""
	str = str.."fields: "	
	for k,v in pairs_by_keys(interfaceTable[id].elements) do
		str = str..k..", "..v.type
		if(v.type == "enum") then
			local enumsTable = {}
			for _, val in pairs(v.enums) do
				str = str.."\n           "
				str = str..(val)
			end
			str = str.."\n"
		elseif v.type == "raw" then
			str = str..(", "..(v.bitStop-v.bitStart +1).. "bits\n")
		else 
			local numBits = v.bitStop - v.bitStart +1
			local numFs = numBits/4 
			MSBits = numBits%4
			MShex = {[0] = "", [1] = "1",[2] = "3", [3] = "7" }
			str = str..(", 0x"..MShex[MSBits])
			for i = 1,numFs do 
				str = str..'F'
			end
			str = str.."\n"

		end
		str = str.."        "
	end
	str = str.."\n"
	print(str)
end

-- helper function used for the CLI
local function print_Interface_w_fields_helper(param)
	local uniqueName = param.uniqueName
	print_Interface_w_fields(uniqueName)
end

--------------------------------------------
-- Print Instance
--
-- print Instances and its instances name (should only be used for debugging)
--
-- input:
-- 	instanceName:string - the name of the Instance to print
-- 
-- output:
--	none(prints to console)
--
--------------------------------------------
--------------------------------------------
-- Prints interface name and other information (only used in print_instance)
local function print_Interface(uniqueName)
	local id = uniqueInterfaceTable[uniqueName].id
	print(interfaceTable[id].name)
	print('     '.. enabledEnum[uniqueInterfaceTable[uniqueName].enabled])
end

local function print_instance(instanceName)
	for k in pairs(instances[instanceName].interfaces) do
		print(k)
		print_Interface(k)
	end
end

-- helper function used for the CLI
local function print_instance_helper(param)
	local instanceName = param.instanceName
	print_instance(instanceName)
end


--------------------------------------------
-- Print Fields
--
-- prints all rules set and their values
-- NOTE: These rules are not necessarily enabled
--
-- input:
-- 	uniqueName:string - the name of the interface to print
-- 
-- output:
--	none(prints to console)
--
--------------------------------------------
--------------------------------------------
local function print_fields(uniqueName)
	local str2print = ""
	if uniqueInterfaceTable[uniqueName].sampleMode ~= nil then print("sample mode: " ..uniqueInterfaceTable[uniqueName].sampleMode) end
 	if uniqueInterfaceTable[uniqueName].isReverse then print("Inverse Key: Enabled\n")
 		elseif uniqueInterfaceTable[uniqueName].isReverse == false then print("Inverse Key: Disabled\n") end
	for k,v in pairs(uniqueInterfaceTable[uniqueName].fields) do
		local field = interfaceTable[uniqueInterfaceTable[uniqueName].id].elements[k]
		local fieldName = k
		rule = v.rule
		mask= v.mask
		
		if rule ~= "null" then
			str2print = str2print..(k..": ")

			if "ipv4_dip" == fieldName or "ipv4_sip" == fieldName then 
				for c in string.gmatch(string.format("%08x", tonumber(rule)), "%x%x") do
				    	str2print = str2print..(string.format("%d",tonumber(c, 16))) .. '.'
				end
				str2print = string.sub(str2print, 1, -2)
				if mask ~= nil then str2print = str2print .. " mask: " .. mask end
				str2print = str2print..'\n'
			elseif field.type == "enum" then
				str2print = str2print..(field.enums[rule])
				if mask ~= nil then str2print = str2print .. " mask: " .. mask end
				str2print = str2print .. "\n"
			elseif field.type == "number" then 
				str2print = str2print .. string.format("0x%x",rule) 
				if mask ~= nil then str2print = str2print .. " mask: " .. mask end
				str2print = str2print .. "\n"
			else
				str2print = str2print .. string.format("0x%s",rule) 
				if mask ~= nil then str2print = str2print .. " mask: " .. mask end
				str2print = str2print .. "\n"

			end
		end
	end
	if str2print == "" then str2print = "no field rules set (will sample all packets)" end
	print(str2print)
end
-- helper function used for the CLI
local function print_fields_helper(param)
	local uniqueName = param.uniqueName
	print_fields(uniqueName)
end

--------------------------------------------
-- Print Fields Configuration
--
-- prints all enabled rules and it's value in an interface
--
-- input:
-- 	uniqueInterface:string - the name of the interface to print
-- 
-- output:
--	string of configuration
--
--------------------------------------------
--------------------------------------------

local function print_fields_config(uniqueInterface)
	local strRules = ""
	local strDisplay = ""
	if enabledInstances[interfaceName].interface.sampleMode ~= nil then strRules = 
						strRules..("sample mode: " ..enabledInstances[interfaceName].interface.sampleMode .. "\n") end
	if(enabledInstances[interfaceName].interface.isReverse) then
		for k,v in pairs(uniqueInterface.fields) do
			rule = v.rule
			mask = v.mask
			display = v.isDisplay
			local field = interfaceTable[uniqueInterface.id].elements[k]
			if rule ~= "null" then
				if "ipv4_dip" == k or "ipv4_sip" == k then 
					strRules = strRules .. k.. ": not "
					for c in string.gmatch(string.format("%08x", tonumber(rule)), "%x%x") do
					    	strRules = strRules.. (string.format("%d",tonumber(c, 16))) .. '.'
					end
					strRules =string.sub(strRules, 1, -2)
					print(mask)
					if mask ~= nil then strRules = strRules .. " mask: " .. mask end
					strRules = strRules..'\n'


				elseif field.type == "enum" then
					strRules = strRules..k.. ": not "..field.enums[rule]
					if mask ~= nil then strRules = strRules .. " mask: " .. mask end
					strRules = strRules..'\n'
				elseif field.type == "number" then 
					strRules = strRules..k.. ": not "..string.format("0x%x",rule )
					if mask ~= nil then strRules = strRules .. " mask: " .. mask end
					strRules = strRules..'\n'
				else
					strRules = strRules..k.. ": not "..string.format("0x%s",rule )
					if mask ~= nil then strRules = strRules .. " mask: " .. mask end
					strRules = strRules..'\n'

				end
			end

			if display then 
				strDisplay = strDisplay ..k .. "\n"
			end
		end
	else
		for k,v in pairs(uniqueInterface.fields) do
			rule = v.rule
			mask = v.mask
			display = v.isDisplay
			local field = interfaceTable[uniqueInterface.id].elements[k]
			if rule ~= "null" then
				if "ipv4_dip" == k or "ipv4_sip" == k then 
					strRules = strRules .. k.. ": "
					for c in string.gmatch(string.format("%08x", tonumber(rule)), "%x%x") do
					    	strRules = strRules..(string.format("%d",tonumber(c, 16))) .. '.'
					end
					strRules =  string.sub(strRules, 1, -2)
					if mask ~= nil then strRules = strRules .. " mask: " .. mask end
					strRules = strRules..'\n'

				elseif field.type == "enum" then
					strRules = strRules..k.. ": "..field.enums[rule]
					if mask ~= nil then strRules = strRules .. " mask: " .. mask end
					strRules = strRules..'\n'

				elseif field.type == "number" then
					strRules = strRules..k.. ": "..string.format("0x%x",rule )
					if mask ~= nil then strRules = strRules .. " mask: " .. mask end
					strRules = strRules..'\n'

				else 
					strRules = strRules..k.. ": "..string.format("0x%x",rule )
					if mask ~= nil then strRules = strRules .. " mask: " .. mask end
					strRules = strRules..'\n'

				end
			end
			if display then 
				strDisplay = strDisplay ..k .. "\n"
			end
		end
	end
	
	if strRules == "" then strRules = "No Rule Configurations" end
	if strDisplay == "" then strDisplay = "No Display Configurations" end
	
	strRules = "Rules:\n".. strRules
	strDisplay = "Display:\n" .. strDisplay
	return strRules .. "\n\n" ..strDisplay
end

--------------------------------------------
-- Print Configuration
--
--prints all enabled interfaces and their enabled rules and display(if there are any)
--
-- input:
--	none
-- 
-- output:
--	none(prints to console)
--
--------------------------------------------
--------------------------------------------
local function print_config()
	local str = "Current Configuration: \n\n"
	for _,v in pairs(enabledInstances) do
		interfaceName = v.interface.name
		str = str..interfaceName..'\n'
		for i=0,#interfaceName do
			str = str..'-'
		end
		str = str.."\n"
		strFields = print_fields_config(v.interface)
		if strFields == "" then str = str..("no field rules set (will sample all packets)") else
		str = str.. strFields
		end
		str = str..'\n\n'

	end
	if str == "Current Configuration: \n\n" then
		print("There is no current Configuration")
	else 
	print(str)
	end

end


--------------------------------------------
-- XML To Table
--
-- parses XML into 4 LUA tables:
-- CPUTable: contains the sampling configurations, rules, and sampled data
-- 				is copied to all ENABLED interfaces
-- interfaceTable: contains information about an interface and all of its fields
-- uniqueInterfaceTable: contains names and rule values of all UNIQUE interfaces in iDebug
-- instances: contains names of all instances and which UNIQUE interfaces belong to them--
--
-- input:
--	none
-- 
-- output:
--	none
--
--------------------------------------------
--------------------------------------------

local function xml2table()
	
	local reg = xml_getchild(iDebug_CPU, nil)
	while (reg ~=nil) do

		local name = xml_get_attr(reg, "internalName")

		cpuTable[name] = {}
		cpuTable[name].size = xml_get_attr(reg,"size")
		cpuTable[name].relOffset = xml_get_attr(reg,"relOffset")
		if(xml_tagname(reg) == "Memory") then	
			cpuTable[name].rowsNo = xml_get_attr(reg, "rowsNo")
			cpuTable[name].lineAlignment = xml_get_attr(reg, "lineAlignment")
			cpuTable[name].byteSize = xml_get_attr(reg, "byteSize")
		else
			if name == "Match_Sample_%i" then
				cpuTable[name].multiplicity = {}
				local multPtr = xml_getchild(reg, nil)
				while xml_tagname(multPtr) ~= "multiplicity" do 

					multPtr = xml_getchild(reg, multPtr)
				end
				local baseAddressPtr = xml_getchild(multPtr, nil)
				while xml_tagname(baseAddressPtr, "baseAddress") ~= "baseAddress" do
					baseAddressPtr = xml_getchild(multPtr, baseAddressPtr)
				end

				cpuTable[name].multiplicity.baseAddress = xml_inlinetext(baseAddressPtr)

				local offsetFormulaPtr = xml_getchild(multPtr, nil)
				while xml_tagname(offsetFormulaPtr, "offsetFormula") ~= "offsetFormula" do
					
					offsetFormulaPtr = xml_getchild(multPtr, offsetFormulaPtr)
				end
				local iteratorPtr = xml_getchild(offsetFormulaPtr, nil)
				while iteratorPtr ~=nil do
					local tagname = xml_tagname(iteratorPtr)
					if tagname == "params" then 
						local paramPtr = xml_getchild(iteratorPtr, nil)
						cpuTable[name].multiplicity.params = {}
						cpuTable[name].multiplicity.params.startStr = xml_get_attr(paramPtr, "start")
						cpuTable[name].multiplicity.params.ordinal = xml_get_attr(paramPtr, "ordinal")
						cpuTable[name].multiplicity.params.name = xml_get_attr(paramPtr, "name")
						cpuTable[name].multiplicity.params.label = xml_get_attr(paramPtr, "label")
						cpuTable[name].multiplicity.params.inc = xml_get_attr(paramPtr, "inc")
						cpuTable[name].multiplicity.params.endStr = xml_get_attr(paramPtr, "end")
					elseif tagname == "formula" then 
						cpuTable[name].multiplicity.formula = xml_inlinetext(iteratorPtr)
					elseif tagname == "formulaDisplayStr" then
						cpuTable[name].multiplicity.displayString = xml_inlinetext(iteratorPtr)
					end

					iteratorPtr = xml_getchild(offsetFormulaPtr, iteratorPtr)
				end
			end
					
			cpuTable[name].fields = {}
			elemsptr = xml_getchild(reg,nil)
			while (xml_tagname(elemsptr) ~= "subElements") do
				elemsptr = xml_getchild(reg, elemsptr)
			end

			local field = xml_getchild(elemsptr, nil)
			while  field ~=nil do
				local fieldName = xml_get_attr(field, "internalName")
				local initValuePtr = xml_getchild(field, nil)
				while xml_tagname(initValuePtr) ~= "initValue" do 
					initValuePtr = xml_getchild(field, initValuePtr)
				end
				cpuTable[name].fields[fieldName] = {}
				cpuTable[name].fields[fieldName].bitStart = xml_get_attr(field, "bitStart")
				cpuTable[name].fields[fieldName].bitStop = xml_get_attr(field,"bitStop")
				cpuTable[name].fields[fieldName].initValue = xml_inlinetext(initValuePtr)
				
				field = xml_getchild(elemsptr,field)
			end
		end
		reg = xml_getchild(iDebug_CPU,reg)
	end
	
	local interf = xml_getchild(iDebug_InterfacesList, nil)
	while(interf ~= nil) do
		local id = xml_get_attr(interf, "id")
		lastname = id	

		interfaceTable[id] = {}
		interfaceTable[id].visible  = xml_get_attr(interf, "visible")
		interfaceTable[id].name  = xml_get_attr(interf, "internalName")
		interfaceTable[id].size  = xml_get_attr(interf, "size")
		interfaceTable[id].version = xml_get_attr(interf, "version")
		interfaceTable[id].id  = xml_get_attr(interf, "id")
		interfaceTable[id].disableDoubleMatch  = xml_get_attr(interf, "disableDoubleMatch")
		interfaceTable[id].controlPolarity  = xml_get_attr(interf, "ControlPolarity")
		interfaceTable[id].visible  = xml_get_attr(interf, "visible")
		interfaceTable[id].elements ={}


		elemsptr = xml_getchild(interf,nil)
		while (xml_tagname(elemsptr) ~= "subElements") do
			elemsptr = xml_getchild(interf, elemsptr)
		end
		local var = xml_getchild(elemsptr, nil)
		while(var ~= nil) do
			interfaceTable[id].elements[xml_get_attr(var, "internalName")] = {}		
			interfaceTable[id].elements[xml_get_attr(var, "internalName")].bitStart =  xml_get_attr(var, "bitStart")
			interfaceTable[id].elements[xml_get_attr(var, "internalName")].bitStop =  xml_get_attr(var, "bitStop")
			interfaceTable[id].elements[xml_get_attr(var, "internalName")].visible =  xml_get_attr(var, "visible")
			local types
			if tonumber(interfaceTable[id].elements[xml_get_attr(var, "internalName")].bitStop) - tonumber(interfaceTable[id].elements[xml_get_attr(var, "internalName")].bitStart)+1 >32 then
				types = "raw"
			else 				
				local enums = xml_getchild(var, nil)
				while(xml_tagname(enums) ~= "Enums") and enums ~= nil do 
					enums = xml_getchild(var,enums)
				end
				if enums ~= nil then 
					enum = xml_getchild(enums,nil)
					if enum ~= nil then types = "enum" end
				end
			end

			if(types == "raw") then
				interfaceTable[id].elements[xml_get_attr(var, "internalName")].type =  types
			else if (types == "enum") then 
				interfaceTable[id].elements[xml_get_attr(var, "internalName")].type =  types
				interfaceTable[id].elements[xml_get_attr(var, "internalName")].enums =  {}

				local enums = xml_getchild(var, nil)
				while(xml_tagname(enums) ~= "Enums") do 
					enums = xml_getchild(var,enums)					
				end

				local enum = xml_getchild(enums, nil)
				while (enum ~= nil) do 
					enumName = xml_get_attr(enum, "name")  
					enumName = string.gsub(enumName, "%s+", "_")
					interfaceTable[id].elements[xml_get_attr(var, "internalName")].enums[(xml_get_attr(enum, "value"))] = enumName
					enum = xml_getchild(enums, enum)
				end
			else
				interfaceTable[id].elements[xml_get_attr(var, "internalName")].type =  "number"				
			end
	
		end
		var = xml_getchild(elemsptr, var)
	end
			
		interf = xml_getchild(iDebug_InterfacesList, interf)
	end

	local i = 1
	instances.ptr = iDebug_Instances
	local instance = xml_getchild(instances.ptr, nil)
	while(instance ~=nil) do
		instanceName = xml_get_attr(instance, "name")
		instances[instanceName] = {}
		instances[instanceName].interfaces = {}
		instances[instanceName].instanceName = instanceName
		instances[instanceName].ptr= instance
		instances[instanceName].baseAddress = xml_get_attr(instance, "baseAddress")
		instances[instanceName].visible = xml_get_attr(instance, "visible")
		instances[instanceName].portGroup = xml_get_attr(instance, "portGroup")
		instances[instanceName].masterName = xml_get_attr(instance, "masterName")
		instances[instanceName].dfxPipe= xml_get_attr(instance, "dfxPipe")

		local interfaces = xml_getchild(instance,nil)
		while xml_tagname(interfaces) ~= "Interfaces" do 
			interfaces = xml_getchild(instance,interfaces)
		end
		local interf = xml_getchild(interfaces, nil)
		while(interf ~= nil) do
			local id = xml_get_attr(interf, "id")
			local index = xml_get_attr(interf, "index")
			local nameptr = xml_getchild(interf, nil) 
			local uniqueName = xml_inlinetext(nameptr)
			uniqueName = string.lower(uniqueName)
			uniqueName = string.gsub(uniqueName, "%s+", "_")

			if uniqueInterfaceTable[uniqueName] ~= nil then
			end
			uniqueInterfaceTable[uniqueName] = {}
			uniqueInterfaceTable[uniqueName].instanceName = instanceName
			uniqueInterfaceTable[uniqueName].id = id
			uniqueInterfaceTable[uniqueName].isSynced = true
			uniqueInterfaceTable[uniqueName].name = uniqueName
			uniqueInterfaceTable[uniqueName].enabled = 0
			uniqueInterfaceTable[uniqueName].index = index
			uniqueInterfaceTable[uniqueName].fields = {}
			for k in pairs(interfaceTable[id].elements) do
				uniqueInterfaceTable[uniqueName].fields[k] = {}
				uniqueInterfaceTable[uniqueName].fields[k].rule = "null"
				uniqueInterfaceTable[uniqueName].fields[k].isDisplay = false				
			end
			instances[instanceName].interfaces[uniqueName] = {}
			
			interf = xml_getchild(xml_getchild(instance, nil), interf)
		end
		instance = xml_getchild(instances.ptr, instance)
	end
end

--------------------------------------------
-- Lookup iDebug XML
--
--looks up correct iDebug XML file based on device ID using iDebug_list_file
--
-- input:
--	none
-- 
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function lookup_iDebug_xml()
    local fd, str, t, v
    if DeviceType == nil then
        return false
    end
    iDebug_xml = nil
    fd = fs.open(iDebug_list_file,"r")
    if fd == nil then
        return false
    end
    while true do
        str = fs.gets(fd)
        if str == nil then
            break
        end
        -- if not comment then
        if string.sub(str,1,1) ~= "#" then
            t = {}
            for v in string.gmatch(str,"(%S+)") do
                table.insert(t,v)
            end
            if #t > 1 then
                if DeviceType == tonumber(t[2]) then
                    local apply = false
                    for v = 3,#t do
                        if DeviceRevision == tonumber(t[v]) then
                            apply = true
                            break
                        end
                    end
                    if #t == 2 then
                        apply = true
                    end
                    if apply then
                        iDebug_xml = t[1]
                        break
                    end
                end
            end
        end
    end
    fs.close(fd)

    if iDebug_xml == nil then
        return false
    end
    if fs.stat("dxCh/"..iDebug_xml) ~= nil then
        return true
    end
    return false
end


--------------------------------------------
-- Lookup iDebug XML
--
-- loads or reloads iDebug XML
--
-- input:
--	reload:boolean - true if realoding XML
-- 
-- output:
--	boolean
--
--------------------------------------------
--------------------------------------------
local function iDebug_load_xml(reload)
    if not lookup_iDebug_xml() then
        print("No iDebug XML found for this device")
        return false
    end
    iDebug_root = xml_open("dxCh/"..iDebug_xml, reload)
    
    if iDebug_root == nil then
        print("Failed to load iDebug info from "..iDebug_xml)
        return false
    end
    iDebug_CPU = xml_lookup(iDebug_root, "IDebug.CPU")
    iDebug_Instances = xml_lookup(iDebug_root, "IDebug.Instances")
    iDebug_InterfacesList = xml_lookup(iDebug_root, "IDebug.InterfacesList")
    iDebug_checkInstance = function()
        return true
    end

    --resets all tables
	listOfAvailableInterfaces = {}
	interfaceTable = {}

	uniqueInterfaceTable = {}
	instances = {}
	cpuTable = {}
	possibleInterfaceCount = 0
	hideEnabledInterfaces = false
	currentInterface = nil
	currentField = nil
	enabledInstances = {}
   	xml2table()

   	-- sets checkTable  (table of interfaces that end in _desc)
	for k,v in pairs(uniqueInterfaceTable) do
		if suffix_matchi("_desc", k) then table.insert(checkTable, k) end	
	end

    
    if iDebug_xml == "iDebug_bobK.xml" then
        if bit_and(DeviceType, 0xff00ffff) ~= 0xbc0011ab then
            -- Device type != caelum, ignore dfxPipe 3
            iDebug_checkInstance = function(instance)
                local dfxPipe = tonumber(xml_get_attr(instance, "dfxPipe"))
                return not (dfxPipe == 3)
            end
        end
    end

    local id = xml_lookup(iDebug_root, "IDebug.DFXServerRoot")
    if id ~= nil then
        local a = tonumber(xml_get_attr(id, "baseAddress"))
        if a ~= nil then
            iDebug_dfxSelector_addr = a + 0xf8000
        end
    end
    return true
end

--------------------------------------------
-- iDebug Initialize
--
--initializes CLI commands and loads correct iDebug XML
--
-- input:
--	none
-- 
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function iDebug_initialize()
    if iDebug_initialized then
        -- already registered
        print("Already loaded")
        return true
    end
    if not iDebug_load_xml(reload) then
        return false
    end

    register_iDebug_commands()
    iDebug_initialized = true
end


-------------------- I/O--------------------
local function iDebug_hw_read_ni(address, defValue, message)
    error("iDebug_hw_read(): not initialized")
end
local function iDebug_hw_write_ni(address, val, message)
    error("iDebug_hw_write(): not initialized")
end
local iDebug_hw_read = iDebug_hw_read_ni
local iDebug_hw_write = iDebug_hw_write_ni



local function iDebug_hw_read_bc2(address, defValue, message)
    if DEVEL then
        if defValue == nil then
            defValue = 0
        end
        return 0, defValue
    end
    --print("reading")
    if wrLogWrapper("wrlCpssIsAsicSimulation") then 
        if defValue == nil then
            defValue = 0
        end
        return 0, defValue
    end
--    print(string.format("address: 0x%x",address ))
    local result, value = myGenWrapper("cpssDrvHwPpResetAndInitControllerReadReg", {
        { "IN", "GT_U8",  "devNum", iDebug_devID },
        { "IN", "GT_U32", "regAddr", address },
        { "OUT", "GT_U32", "data"}
    })
    if result == 0 then
        return result, value.data
    end
    if message == nil then
        message = "failed to read"
    end
    error(string.format(message .. " 0x%x: %d", address, result))
end

local function iDebug_hw_write_bc2(address, val, message)
    if DEVEL then
        return 0
    end
    if wrLogWrapper("wrlCpssIsAsicSimulation") then 
        return 0
    end
   -- print(string.format("writing to address: 0x%08x, value: %x ", address, val))
    local result = myGenWrapper("cpssDrvHwPpResetAndInitControllerWriteReg", {
        { "IN", "GT_U8",  "devNum", iDebug_devID },
        { "IN", "GT_U32", "regAddr", address },
        { "IN", "GT_U32", "data", val}
    })
    if result == 0 then
        return result
    end
    if message == nil then
        message = "failed to write"
    end
    error(string.format(message .. " 0x%x to register 0x%x: %d",
                val, address, result))
    return result
end

local iDebug_hw_stack = {}
local function iDebug_hw_clean()
    iDebug_hw_stack = {}
    iDebug_devID = nil
    iDebug_hw_read = iDebug_hw_read_ni
    iDebug_hw_write = iDebug_hw_write_ni
end
local function iDebug_hw_set(devID)
    table.insert(iDebug_hw_stack, {
        devID = iDebug_devId,
        read  = iDebug_hw_read,
        write = iDebug_hw_write
    })
    iDebug_devID = devID
    iDebug_hw_read = iDebug_hw_read_ni
    iDebug_hw_write = iDebug_hw_write_ni
--    if is_sip_5(devID) then
        iDebug_hw_read = iDebug_hw_read_bc2
        iDebug_hw_write = iDebug_hw_write_bc2
--    end
end
local function iDebug_hw_pop()
    if #iDebug_hw_stack == 0 then
        return
    end
    local t = iDebug_hw_stack[#iDebug_hw_stack]
    iDebug_devID = t.devID
    iDebug_hw_read = t.read
    iDebug_hw_write = t.write
    table.remove(iDebug_hw_stack)
end
-------------------- END I/O--------------------


--------------------------------------------
-- Write Field
--
-- writes CPU type fields to H/W
--
-- input:
--	elem:table			- a CPU register table
--	field_path:string 		- the name of the field in CPU register
--	value:string/number	- the data to be written
--	data:string 			- the previous data in the register
--	mask:string 			- the custom mask for the field, keep nil if no mask necessary
--	size:number			- the size of the field, keep nil to default to 32 bits
--	offset:number			- offset of the register, can be nil
-- 
-- output:
--	returns the changed data and mask of the register
--
--------------------------------------------
--------------------------------------------
local function write_field(elem, field_path, value, data, mask, size, offset)
    local bitStart, bitStop, bitLen, i, v
    if size == nil then size = 32 end
    if offset == nil then offset = 0 end
    if data == nil then data = init_data(size) end
    if mask == nil then mask = init_data(size, true) end
    local field = nil
    	fieldValue = elem.fields[field_path]
        local fname = field_path
        local ftype = "number"
        bitStart = tonumber(fieldValue.bitStart)
        bitStop = tonumber(fieldValue.bitStop)
        bitLen = bitStop - bitStart + 1

        local val = value
        local val_mask
        if custom_mask ~= nil then
            val_mask = bit_not(custom_mask)
        end

        if fname == field_path then
            if ftype == "number" then
                data = cider_SetBits(data,
                        bitStop + offset,
                        bitStart + offset,
                        bitLen, val)
            elseif ftype == "enum" then
                val = tonumber(value)
                if val ~= nil then
                    data = cider_SetBits(data,
                            bitStop + offset,
                            bitStart + offset,
                            bitLen, val)
                else
                    local enumDescr = xml_lookup(field, "Enums")
                    val = xml_lookup_by_attr(enumDescr,
                        "Value", "internalName", value,
                        iDebug_ignorecase)
                    if val == nil then
                        return data, mask
                    end
                    data = cider_SetBits(data,
                            bitStop + offset,
                            bitStart + offset,
                            bitLen,
                            get_int_attr(val, "value"))
                end
            elseif ftype == "raw" then
                local rawdata = cider_ToUserdata(value)
                i = 0
                while i + 15 < bitLen do
                    v = cider_GetBits(rawdata, i + 15, i, 16)
                    data = cider_SetBits(data,
                            offset + bitStart + i + 15,
                            offset + bitStart + i,
                            16,
                            v)
                    i = i + 16
                end
                if i < bitLen then
                    v = cider_GetBits(rawdata, bitLen - 1, i, bitLen - i)
                    data = cider_SetBits(data,
                            offset + bitStop,
                            offset + bitStart + i,
                            bitLen - i, v)
                end
            end
            if val_mask ~= nil and bitStop-bitStart+1 < 32 then
                mask = cider_SetBits(mask,
                        offset + bitStop,
                        offset + bitStart,
                        bitStop + 1 - bitStart,
                        val_mask)
            else
                i = bitStart
                while i + 31 < bitStop do
                    mask = cider_SetBits(mask,
                            offset + i + 31,
                            offset + i,
                            32,
                            0)
                    i = i + 32
                end
                if i < bitStop + 1 then
                    mask = cider_SetBits(mask,
                            offset + bitStop,
                            offset + i,
                            bitStop + 1 - i,
                            0)
                end
            end
        end
    return data, mask
end

--------------------------------------------
-- Write Field Elements
--
-- writes interface fields to H/W
--
-- input:
--	elem:table			- a unique Interface Table
--	field_path:string		- the name of the field in the interface register
--	value:string/number 	- the data to be written
--	data:string			- the previous data in the register
--	mask:string			- the custom mask for the field, keep nil if no mask necessary
--	size:number 			- the size of the field, keep nil to default to 32 bits
--	offset:number			- offset of the register, can be nil
-- 
-- output:
--	returns the changed data and mask of the register
--
--------------------------------------------
local function write_field_elems(elem, data, mask, size, offset, custom_mask)
	local fields = interfaceTable[elem.id].elements
	 if mask == nil then mask = init_data(size, true) end

	 mask = cider_ToUserdata(mask)

	for KEY,VAL in pairs(elem.fields) do
	    local field = fields[KEY]
	    local value = VAL.rule
	    local custom_mask = VAL.mask
	    local bitStart, bitStop, bitLen, i, v

	    if size == nil then size = 32 end
	    if offset == nil then offset = 0 end
	    if data == nil then data = init_data(size) end
	    	fieldValue = value
	        local fname = KEY
	        local ftype = field.type
	        
	        bitStart = tonumber(field.bitStart)
	        bitStop = tonumber(field.bitStop)
	        bitLen = bitStop - bitStart + 1

	        local val = value
	        local val_mask


	        if fieldValue ~= "null" then
	            if ftype == "number" then
	                data = cider_DumpUserdata(cider_SetBits(cider_ToUserdata(data),
	                        bitStop + offset,
	                        bitStart + offset,
	                        bitLen, val))
	            elseif ftype == "enum" then
	                val = tonumber(value)
	                if val ~= nil then
	                    data = cider_DumpUserdata(cider_SetBits(cider_ToUserdata(data),
	                            bitStop + offset,
	                            bitStart + offset,
	                            bitLen, val))
	                else
				error("enum value not given")
	                end
	            elseif ftype == "raw" then
	                local rawdata = value
	                i = 0
	                while i + 15 < bitLen do
	                    v = cider_GetBits(rawdata, i + 15, i, 16)
	                    data = cider_SetBits(data,
	                            offset + bitStart + i + 15,
	                            offset + bitStart + i,
	                            16,
	                            v)
	                    i = i + 16
	                end
	                if i < bitLen then
	                    v = cider_GetBits(rawdata, bitLen - 1, i, bitLen - i)
	                    data = cider_SetBits(data,
	                            offset + bitStop,
	                            offset + bitStart + i,
	                            bitLen - i, v)
	                end
	            end
	            if custom_mask ~= nil and bitStop-bitStart+1 <= 32 then
	                -- use custom mask
	                mask = cider_SetBits(mask,
	                        offset + bitStop,
	                        offset + bitStart,
	                        bitStop + 1 - bitStart,
	                        custom_mask)
	            else
	                i = bitStart
	                while i + 31 < bitStop do
	                    mask = cider_SetBits(mask,
	                            offset + i + 31,
	                            offset + i,
	                            32,
	                            0)
	                    i = i + 32
	                end
	                if i < bitStop + 1 then
	                

	                    mask = cider_SetBits(mask,
	                            offset + bitStop,
	                            offset + i,
	                            bitStop + 1 - i,
	                            0)
	                end
	            end
	        end
    end
    return data, cider_DumpUserdata(mask)
end

-------------------- DFX pipe selection--------------------


-- selects correct dfxPipe
local function iDebug_select_dfx_pipe(dfxPipe)
    local result
    local address = iDebug_dfxSelector_addr

    if dfxPipe == nil then dfxPipe = 0 end
    local val = bit_shl(1, dfxPipe)

    --DFX server is in portGroup 0
    if DEBUG then
        print(string.format("iDebug_select_dfx_pipe(): write 0x%x to 0x%x", val, address))
    end
--    print("new dfxVal: " .. val)
    result = iDebug_hw_write(address, val)
    return (result == 0)
end

-- saves previous dfxPipe
local function iDebug_save_dfx_pipe()
    --DFX server is in portGroup 0
    local address = iDebug_dfxSelector_addr
    local d
    if DEBUG then
        d = string.format("iDebug_save_dfx_pipe(): read 0x%x", address)
    end
    local result, value = iDebug_hw_read(address, 1)

    if result ~= 0 then
        return false
    end
    if DEBUG then
        print(d..string.format(", value=0x%x",value))
    end
    return value
end

--restores old dfxPipe
local function iDebug_restore_dfx_pipe(value)
    --DFX server is in portGroup 0
    local address = iDebug_dfxSelector_addr
    if DEBUG then
        print(string.format("iDebug_restore_dfx_pipe(): write 0x%x to 0x%x", value, address))
    end
    local result = iDebug_hw_write(address, value)
    return (result == 0)
end


--------------------DFX Pipe selection END--------------------



--------------------------------------------
-- iDebug Read Register
--
-- reads a register with given address and dfxPipe
--
-- input:
--	address:number 	- the adress of the register to read
--	dfxPipe:number	- the DFX pipe the register exists in
-- 
-- output:
--	the register value
--
--------------------------------------------
--------------------------------------------

local function iDebug_read_register(address, dfxPipe)
    local prevDfxPipe

    --Save he current DFX pipe, return the value later
    prevDfxPipe = iDebug_save_dfx_pipe()
    if prevDfxPipe == nil then
        return nil
    end
	
    --Change DFX pipe
    if (prevDfxPipe ~= bit_shl(1,dfxPipe)) then
        iDebug_select_dfx_pipe(dfxPipe)
    else
        prevDfxPipe = nil
    end
    local result, value = iDebug_hw_read(address, 0x12345678)

    if DEBUG then
        print(string.format("iDebug_read_register(): read 0x%x, return 0x%x", address, value))
    end


    --Return DFX pipe
    if prevDfxPipe ~= nil then
        iDebug_restore_dfx_pipe(prevDfxPipe)
    end

    return value
end

--------------------------------------------
-- iDebug Read Register
--
--writes to a register with a given address, value and dfxPipe
--
-- input:
--	address:number	- the adress of the register to write
--	value:number		- the value to write to the register
--	dfxPipe:number	- the DFX pipe the register exists in
-- 
-- output:
--	the new register value
--
--------------------------------------------
--------------------------------------------

local function iDebug_write_register(address, value, dfxPipe)
    local prevDfxPipe
	
    if dfxPipe == nil then dfxPipe = iDebug_dfxPipe end

    --Save he current DFX pipe, return the value later
    prevDfxPipe = iDebug_save_dfx_pipe()
    if prevDfxPipe == nil then
        return nil
    end
	
    --Change DFX pipe
    if prevDfxPipe ~= bit_shl(1,dfxPipe) then
        iDebug_select_dfx_pipe(dfxPipe)
    else
        prevDfxPipe = nil
    end
	
    local result = iDebug_hw_write(address, value)
    if DEBUG then
        print(string.format("iDebug_write_register(): write 0x%x to 0x%x, ret=%d", value, address, result))
    end

    --Return DFX pipe
    if prevDfxPipe ~= nil then
        iDebug_restore_dfx_pipe(prevDfxPipe)
    end

    if result ~= 0 then
        error(string.format("failed to write 0x%x to register 0x%x: %d", tonumber(value, 16), address, result))
        return false
    end

    return true
end

--------------------------------------------
-- iDebug Write Key
--
-- writes a an interface's key and mask
--
-- input:
--	cpuRegister:string	- the name of the CPU register to access (Match key or Match Mask)
--	data:number		- the value to write to the register 
-- 	size:number		- the size the key 
--	dfxPipe:number	- the DFX pipe the register exists in
-- 
-- output:
--	boolean - if succesful
--
--------------------------------------------
--------------------------------------------
local function iDebug_write_key(cpuRegister, data, size, dfxPipe)
    local data = cider_ToUserdata(data)
    local addr = tonumber(instances[enabledInstances[currentInterface].interface.instanceName].baseAddress) +
    			  tonumber(enabledInstances[currentInterface].cpu[cpuRegister].relOffset)    
    local prevDfxPipe
    if dfxPipe == nil then dfxPipe = iDebug_dfxPipe end

    --Save he current DFX pipe, return the value later
    prevDfxPipe = iDebug_save_dfx_pipe()
    if prevDfxPipe == nil then
        return nil
    end
	
    --Change DFX pipe
    if prevDfxPipe ~= bit_shl(1,dfxPipe) then
        iDebug_select_dfx_pipe(dfxPipe)
    else
        prevDfxPipe = nil
    end
	
    if DEBUG then
        print(string.format("iDebug_write_key(), address=0x%x, size=%d", addr, size))
    end
    local i = 0
    while i < size do
        local v = cider_GetBits(data, i + 31, i, 32)
    --    print(string.format("BITS WRITTEN: 0x%x", v))
        i = i + 32
        iDebug_hw_write(addr, v, "failed to write key/mask")
    end
    if DEBUG then
        print(string.format("%s has been written to 0x%x data=",cpuRegister,addr)..cider_DumpUserdata(data))
    end

    --Return DFX pipe
    if prevDfxPipe ~= nil then
        iDebug_restore_dfx_pipe(prevDfxPipe)
    end
    return true

end

--------------------------------------------
-- iDebug Read Key
--
--reads an interfaces key and mask (mainly used for debugging)
--
-- input:
--	cpuRegister:string	- the name of the CPU register to access (Match key or Match Mask)
-- 	size:number		- the size the key 
--	dfxPipe:number	- the DFX pipe the register exists in
-- 
-- output:
--	the register data dump
--
--------------------------------------------
--------------------------------------------
local function iDebug_read_key(cpuRegister, size, dfxPipe)
    local prevDfxPipe
    local addr = tonumber(instances[enabledInstances[currentInterface].interface.instanceName].baseAddress) +  tonumber(enabledInstances[currentInterface].cpu[cpuRegister].relOffset)    

    if dfxPipe == nil then dfxPipe = iDebug_dfxPipe end
	
    --Save he current DFX pipe, return the value later
    prevDfxPipe = iDebug_save_dfx_pipe()
    if prevDfxPipe == nil then
        return nil
    end
    --Change DFX pipe
    if prevDfxPipe ~= bit_shl(1,dfxPipe) then
        iDebug_select_dfx_pipe(dfxPipe)
    else
        prevDfxPipe = nil
    end
	
    if DEBUG then
        print(string.format("iDebug_read_key(), address=0x%x, size=%d", addr, size))
    end
    local data = cider_ToUserdata(init_data(size))
    local i = 0
    while i < size do
        local result, value = iDebug_hw_read(addr, 0, "failed to read key/mask from register")
        data = cider_SetBits(data, i + 31, i, 32, value)        
        i = i + 32
    end
    if DEBUG then
        print(string.format("%s has been read from 0x%x data=",cpuRegister,addr)..cider_DumpUserdata(data))
    end
	
    --Return DFX pipe
    if prevDfxPipe ~= nil then
        iDebug_restore_dfx_pipe(prevDfxPipe)
    end
	
    return cider_DumpUserdata(data)
end


--------------------------------------------
-- Reset Access Pointer
--
--resets access pointer (must be used whenever reading or writing to key/mask)
--
-- input:
--	reg:table			- a CPU register table
-- 	addr:number		- address of the register
--	regval:number	- value to write to H/W
--	dfxPipe:number	- the DFX pipe the register exists in
-- 
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function  reset_access_pointer(reg, addr, regval, dfxPipe)

        local function set_val(fieldname, value)
       	 regval = write_field(reg, fieldname, value, regval)
	  end

    set_val("ResetAccessPointer", 1)
    iDebug_write_register(addr, regval, dfxPipe)
    set_val("ResetAccessPointer", 0)
    iDebug_write_register(addr, regval,dfxPipe)

end


--------------------------------------------
-- Get CPU Register
--
--gets information on a CPU register of a given interface
--
-- input:
-- 	interfaceName:string	- name of the interface being accessed
--	regName:string		- name of the CPU register being accessed
--
-- output:
-- 	reg:table			 - table of register information
-- 	addr:number 		 - address of register
--	regvalue:number	 - the value inside the register
-- 	dfxPipe:number 	 - which dfxPipe the interface is
--
--------------------------------------------
--------------------------------------------

local function get_cpu_register(interfaceName, regName)
    local instanceName = enabledInstances[interfaceName].interface.instanceName
    local dfxPipe = tonumber(instances[instanceName].dfxPipe)
    local reg =enabledInstances[interfaceName].cpu[regName]
    if reg == nil then
        return nil
    end
    local addr = tonumber(instances[instanceName].baseAddress) + tonumber(reg.relOffset)
    local regvalue = iDebug_read_register(addr, dfxPipe)
    return reg, addr, regvalue, dfxPipe
end


--------------------------------------------
-- Disable Apply
--
-- disables interface sampling in H/W
--
-- input:
-- 	interfaceName:string	- name of the interface to disable sampling
--
-- output:
--	none (disables sampling in H/W)
--
--------------------------------------------
--------------------------------------------
local function disable_apply(interfaceName)
        local reg, addr, regval, dfxPipe= get_cpu_register(interfaceName, "General_Configurations")

        local function set_val(fieldname, value)
       	 regval = write_field(reg, fieldname, value, regval)
	 end
	set_val("MatchSampleEnable", 0)
	iDebug_write_register(addr, regval,dfxPipe)

end

--------------------------------------------
-- Disable Interface 
--
-- disables an interface in S/W then disables sampling in H/W
--
-- input:
-- 	interfaceName:string	- name of the interface to disable sampling
--
-- output:
--	none (disables sampling in H/W)
--
--------------------------------------------
--------------------------------------------
local function disable_interface(uniqueName)
	if uniqueInterfaceTable[uniqueName].enabled == 1 then
		instanceName = uniqueInterfaceTable[uniqueName].instanceName
		disable_apply(uniqueName)
		for k in pairs(instances[instanceName].interfaces) do
			uniqueInterfaceTable[k].enabled = 0
		end
		enabledInstances[uniqueName] = nil
	end
end

-- helper function for CLI to disable interface
local function disable_interface_helper (param)
	local uniqueName = param.uniqueName
	disable_interface(uniqueName)
end


--------------------------------------------
-- Set Display 
--
-- sets a field to be displayed or not displayed in show sample all call
--
-- input:
-- 	field:string	- name of the field to set display
--
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function set_display(field, interfaceName, flagNo)
	local currentInterface = currentInterface
	if interfaceName ~= nil then currentInterface = interfaceName end


	if flagNo then 
		if field == "all" then 
			for fieldName in pairs(uniqueInterfaceTable[currentInterface].fields) do
				uniqueInterfaceTable[currentInterface].fields[fieldName].isDisplay = false
				if enabledInstances[currentInterface] ~= nil then
					enabledInstances[currentInterface].interface.fields[fieldName].isDisplay = false
				end
			end
		else	
			uniqueInterfaceTable[currentInterface].fields[field].isDisplay = false
			if enabledInstances[currentInterface] ~= nil then enabledInstances[currentInterface].interface.fields[field].isDisplay = false end
		end
	else
		if field == "all" then 
			for fieldName in pairs(uniqueInterfaceTable[currentInterface].fields) do
				uniqueInterfaceTable[currentInterface].fields[fieldName].isDisplay = true
				if enabledInstances[currentInterface] ~= nil then
					enabledInstances[currentInterface].interface.fields[fieldName].isDisplay = true
				end
			end
		else	
			uniqueInterfaceTable[currentInterface].fields[field].isDisplay = true
			if enabledInstances[currentInterface] ~= nil then enabledInstances[currentInterface].interface.fields[field].isDisplay = true end
		end	
	end
end

-- helper function to set display in lua CLI
local function set_display_helper(param)
	local field = param.field
	local flagNo = param.flagNo
	local interfaceName = param.interfaceName
	
	set_display(field,interfaceName,flagNo )
end



--------------------------------------------
-- Enter Value To Field
--
-- enters value of rule into LUA table 
-- NOTE: must enable for rule to take effect
--
-- input:
--	value:string	- the value to be entered into the table
--	mask:string	- the mask to be entered (can be nil)
--	
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function enter_value_to_field(field, value,mask)
	local  bitstop = interfaceTable[uniqueInterfaceTable[currentInterface].id].elements[currentField].bitStop
	local  bitstart = interfaceTable[uniqueInterfaceTable[currentInterface].id].elements[currentField].bitStart
	local bitsize = bitstop - bitstart+1
	
	local fieldType = interfaceTable[uniqueInterfaceTable[currentInterface].id].elements[currentField].type
	if fieldType == "number" then value  = tonumber(value) --turns 0x(HEX) into the proper hex number  
		uniqueInterfaceTable[currentInterface].fields[currentField].rule = value
		uniqueInterfaceTable[currentInterface].fields[currentField].mask = mask
	elseif fieldType == "enum" then 
		uniqueInterfaceTable[currentInterface].fields[currentField].rule = value
		uniqueInterfaceTable[currentInterface].fields[currentField].mask = mask
	else --raw type
		uniqueInterfaceTable[currentInterface].fields[currentField].rule = value	
		uniqueInterfaceTable[currentInterface].fields[currentField].mask = mask
	end
	uniqueInterfaceTable[currentInterface].isSynced = false
end

--------------------------------------------
-- Reset Rule
--
-- resets specific field or all fields 
--
-- input:
--	interfaceName:string	- name of interface to reset the field
--	field:string			- name of the field to reset (can be "all")
--	inInterface:boolean	- should be true if located inside the mode idebug-select-interface mode
--
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function reset_rule(field, interfaceName, inInterface)
	
	if interfaceName == nil then interfaceName = currentInterface end
	if field == "all" then
		for k in pairs(uniqueInterfaceTable[interfaceName].fields) do
			uniqueInterfaceTable[interfaceName].fields[k].rule = "null"
		end
	else 
		uniqueInterfaceTable[interfaceName].fields[field].rule = "null"
	end
	if inInterface ~= false then 
		CLI_prompts["idebug-select-interface"] = "(idebug-interface-"..interfaceName .." - *UNYSNCED*)#"
		CLI_change_mode_pop()
	    	CLI_change_mode_push("idebug-select-interface")
	 end
	uniqueInterfaceTable[interfaceName].isSynced = false
	disable_interface(interfaceName)
end

--helper function for the LUACLI
local function reset_rule_helper(params)
	local field = params.field
	local interfaceName = params.interfaceName
	reset_rule(field, interfaceName, true)
end

--------------------------------------------
-- Reset Interface
--
-- resets all rules in current interface then disables sampling
--
-- input:
--	interfaceName:string	- name of interface to reset all of the fields
--
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function reset_interface(interfaceName)
	local uniqueInterface
	local inInterface = true
	if interfaceName == nil then uniqueInterface = currentInterface 
	else
		uniqueInterface = interfaceName 
		inInterface= false
	end
	if uniqueInterface == "all" then 
		for k in pairs(uniqueInterfaceTable) do 
			reset_rule("all",k, inInterface)
        		set_display("all", k, true)

		end

	else 
		reset_rule("all",uniqueInterface, inInterface)
        	set_display("all", interfaceName, true)
	end
end

local function reset_interface_helper(params)
	local interfaceName = params.interfaceName
	reset_interface(interfaceName)
end

--------------------------------------------
-- Inverse Key
--
-- sets the inverse key rule in software
--
-- input:
--	isReverse:string	- enabled or disabled
--
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function inverse_key(isReverse)
	if isReverse == "enable" then isReverse = true else isReverse = false end
	uniqueInterfaceTable[currentInterface].isReverse = isReverse
	uniqueInterfaceTable[currentInterface].isSynced = false

end


--------------------------------------------
-- Sample Mode
--
-- sets the inverse key rule in software
--
-- input:
--	isReverse:string	- enabled or disabled
--
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function sample_mode(sampleMode)	
	uniqueInterfaceTable[currentInterface].sampleMode = sampleMode	
	uniqueInterfaceTable[currentInterface].isSynced = false


end


--helper function to change rules
local function rule_function(params)
	if params.field ~= nil then enter_value_to_field(params.field, params.value, params.mask)
	elseif params.isReverse ~= nil then inverse_key(params.isReverse)
	elseif params.sampleMode ~= nil then sample_mode(params.sampleMode)
	end
	
-- desync prompt in CLI
	CLI_prompts["idebug-select-interface"] = "(idebug-interface-"..currentInterface .." - *UNYSNCED*)#"
	CLI_change_mode_pop()
    	CLI_change_mode_push("idebug-select-interface")
	disable_interface(currentInterface)
end


--------------------------------------------
-- Sample Mode
--
-- enables an interface in H/W and synchronizes software database
-- and hardware
--NOTE: this applies all rules set in the interface until called
--
-- input:
--	interfaceName:string	- interface to enable and synchronize H/W
-- 							with S/W
--
-- output:
--	none
--
--------------------------------------------
--------------------------------------------

local function apply_interface(interfaceName)
    local reg, addr, regval, dfxPipe= get_cpu_register(interfaceName, "General_Configurations")
   	  local interfaceType = interfaceTable[enabledInstances[interfaceName].interface.id]
   	  local inverseKeyEnabled = enabledInstances[interfaceName].interface.isReverse
   	  local sampleMode= enabledInstances[interfaceName].interface.sampleMode
  	  local isSampling = regval % 2

        local function set_val(fieldname, value)
    	    regval = write_field(reg, fieldname, value, regval)
   	 end	
   	 
    set_val("MatchSampleEnable", 0)
    -- disableDoubleMatch: yes/no
    local disDoubleMatch = interfaceType.disableDoubleMatch
    if disDoubleMatch == "no" then
        disDoubleMatch = 0
    else
        disDoubleMatch = 1
    end
    set_val("DisableDoubleMatch", disDoubleMatch)
    --TODO: ControlPolarity: AH/AL
    local controlPolarity = interfaceType.controlPolarity
    if controlPolarity == "AH" then
        controlPolarity = 1
    else
        controlPolarity = 0
    end
    set_val("ControlPolarity", controlPolarity)
    set_val("ExternalInterfaceSel", tonumber(enabledInstances[interfaceName].interface.index))


     if sampleMode == "first_match" then 
        set_val("MatchSampleSingleEnable", 1)
    else
        set_val("MatchSampleSingleEnable", 0)
    end
 

    if inverseKeyEnabled then 
        set_val("InverseKeyEnable", 1)
    else
        set_val("InverseKeyEnable", 0)
    end
 
    iDebug_write_register(addr, regval, dfxPipe)

    local i
    local data, mask
    local size = tonumber(interfaceType.size)
    mask = init_data(size, true) --sets mask to all true    
    data, mask = write_field_elems(enabledInstances[interfaceName].interface, data, mask, size, offset)
    reset_access_pointer(reg, addr, regval, dfxPipe)
    -- write key
    iDebug_write_key("Match_Key", data, size, dfxPipe)
    -- Reset counter
    reset_access_pointer(reg, addr, regval, dfxPipe)
    -- Write mask
    iDebug_write_key("Match_Mask", mask, size, dfxPipe)
    reset_access_pointer(reg, addr, regval, dfxPipe)
    --reset counter to 0
    get_cpu_register(interfaceName, "Match_Counter") -- reads counter which clears on read
    get_cpu_register(interfaceName, "Peak_Logging_Value")

    -- reenable sampling
    regval = write_field(reg, "MatchSampleEnable", 1, regval)
    iDebug_write_register(addr, regval, dfxPipe)
end


--------------------------------------------
--  Enable Interface
--
-- enables interface in S/W then calls apply_interface
-- Note: only works within an interface
--
-- input:
--	none
--
-- output:
--	none
--
--------------------------------------------
--------------------------------------------

local function enable_interface()
	local uniqueName = currentInterface
	instanceName = uniqueInterfaceTable[uniqueName].instanceName
	if uniqueInterfaceTable[uniqueName].enabled == 0 then
		for k in pairs(instances[instanceName].interfaces) do
			uniqueInterfaceTable[k].enabled = -1
		end
		uniqueInterfaceTable[uniqueName].enabled = 1
	elseif uniqueInterfaceTable[uniqueName].enabled == -1 then
		local interfaceEnabled
		for k in pairs(instances[instanceName].interfaces) do
			if uniqueInterfaceTable[k].enabled == 1 then interfaceEnabled = k end
		end
		print("Interface cannot be enabled, interface \""..interfaceEnabled .. "\" must be disabled")
	end	
	if uniqueInterfaceTable[uniqueName].enabled == 1 then
		local newEnabledInstance = {interface = deep_copy(uniqueInterfaceTable[uniqueName]), sample = {}, cpu = deep_copy(cpuTable)}
		enabledInstances[uniqueName] = newEnabledInstance
		apply_interface(uniqueName, isReverse)
		
		uniqueInterfaceTable[uniqueName].isSynced = true
		CLI_prompts["idebug-select-interface"] = "(idebug-interface-"..currentInterface ..")#"
		CLI_change_mode_pop()
    		CLI_change_mode_push("idebug-select-interface")
		
              print_fields(currentInterface)
	end
end


--------------------------------------------
--  Search
--
-- searches through all field names in all interfaces/descriptors and lists
-- fields that match given lua pattern and that interfaces that contain those
-- fields
--
-- input:
--	expr:string		- the LUA pattern to match the string
--	descript:boolean	- if true, will only search through fields 
--						labeled as descriptor (end in "_desc")
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function search(expr, descriptor)
	local fieldTable = {}
	local tble =  uniqueInterfaceTable
	for interfaceName,ifTable in pairs(tble) do
		for fieldName in pairs(ifTable.fields) do
			if string.find(fieldName, expr) then
				if descriptor then
					if suffix_matchi("_desc", interfaceName) then
						if fieldTable[interfaceName] == nil then
							fieldTable[interfaceName] = {}
						end
						table.insert(fieldTable[interfaceName], fieldName)
					end
				else
					if fieldTable[interfaceName] == nil then
						fieldTable[interfaceName] = {}
					end
					table.insert(fieldTable[interfaceName], fieldName)
				end
			end
		end
	end
	for interfaceName, ifTable in pairs(fieldTable) do
		print(interfaceName)
		for _, fieldName in ipairs(ifTable) do
			print("        " .. fieldName)
		end
	end
end

-- helper function for lua CLI
local function search_helper(param)
	search(param.expr, false)
end

--helper function for lua CLI
local function search_descriptor_helper(param)
	search(param.expr, true)
end
			
	
--------------------------------------------
--  Idebug Enter
--
-- enters iDebug Mode
--
-- input:
--	devID:number	- the device ID of the device being accessed
--
-- output:
--	none
--
--------------------------------------------
--------------------------------------------
local function idebug_enter(devID)
    iDebug_hw_clean()
    iDebug_hw_set(devID)
    iDebug_Instance = nil
    iDebug_dfxPipe = nil
    iDebug_interface = nil
    CLI_change_mode_push("idebug")
end

-- helper function for lua CLI
local function idebug_enter_helper(param)
	local devID = param.devID
	idebug_enter(devID)
end

--------------------------------------------
--  Idebug Read Sample
--
-- reads sample from register and return string of sample
--
-- input:
--	address:number	- the device ID of the device being accessed
--	size:number		- the size of the sample
--	dfxPipe:number	- the dfx pipe number
--
-- output:
--	sample string
--
--------------------------------------------
--------------------------------------------
local function iDebug_read_sample(address, size, dfxPipe)
    if dfxPipe == nil then dfxPipe = 0 end
    local prevDfxPipe
    --Save he current DFX pipe, return the value later
    prevDfxPipe = iDebug_save_dfx_pipe()
    if prevDfxPipe == nil then
        return nil
    end
    --Change DFX pipe
    if prevDfxPipe ~= bit_shl(1,dfxPipe) then
        iDebug_select_dfx_pipe(dfxPipe)
    else
        prevDfxPipe = nil
    end
	
    local sample = cider_ToUserdata(init_data(size))

    local i = 0
    local a = address
    while i < size do
        local result, value = iDebug_hw_read(a)
        sample = cider_SetBits(sample, i + 31, i, 32, value)
        i = i + 32
        a = a + 4
    end
    --Return DFX pipe
    if prevDfxPipe ~= nil then
        iDebug_restore_dfx_pipe(prevDfxPipe)
    end
	
    if DEBUG then
        print(string.format("iDebug_read_sample(), address=0x%x, size=%d sample=", address, size)..
                cider_DumpUserdata(sample))
    end
    return cider_DumpUserdata(sample)
end



--------------------------------------------
--  Parse Fields
--
-- parses sample into correct fields
--
-- input:
--	interfaceType:string	- the device ID of the device being accessed
--	sample:string			- the size of the sample
--
-- output:
--	the parsed value table
--
--------------------------------------------
--------------------------------------------
local function parse_fields(interfaceType, sample)
	local parsedValueTable = {}
	for fieldName, fieldTable in pairs(interfaceType.elements) do
		local bitStart = fieldTable.bitStart	
		local bitStop = fieldTable.bitStop
		local bitSize = bitStop - bitStart+1

		local fieldType = fieldTable.type
		local data = cider_GetBits(cider_ToUserdata(sample), bitStop, bitStart, bitSize)	
		if fieldType == "enum" then 
			data = tostring(fieldTable.enums[string.format("%x", data)])
		end
		parsedValueTable[fieldName] = {}
		parsedValueTable[fieldName].type = fieldType
		parsedValueTable[fieldName].data = data
		parsedValueTable[fieldName].bitStart = bitStart
		parsedValueTable[fieldName].bitStop = bitStop
		
	end
	return parsedValueTable
end



--------------------------------------------
--  Show Sample All Helper
--
-- returns a string of a field that was sampled
--
-- input:
--	interfaceName:string	- the interface to print
--	specField:string		- the specific field to print
--
-- output:
--	a string of the sample field
--
--------------------------------------------
--------------------------------------------
local function show_sample_all_helper(interfaceName, specField)
	
    local instance = instances[enabledInstances[interfaceName].interface.instanceName]
    local interfaceType = interfaceTable[enabledInstances[interfaceName].interface.id]
    local dfxPipe = tonumber(instance.dfxPipe)

    


    local baseAddr, size, sample
    baseAddr= tonumber(instance.baseAddress)
    size = tonumber(interfaceType.size)
    fieldSize = tonumber(interfaceType.elements[specField].bitStop) - tonumber(interfaceType.elements[specField].bitStart)+1
    sample = iDebug_read_sample(baseAddr, size, dfxPipe)


	local sampledTable = parse_fields(interfaceType, sample)
	if "ipv4_dip" == specField or "ipv4_sip" == specField then 
		local str = ""
		for c in string.gmatch(string.format("%08x", tonumber(sampledTable[specField].data)), "%x%x") do
		    	str = str ..(string.format("%d",tonumber(c, 16))) .. '.'
		end
		return specField .. ": " .. string.sub(str, 1, -2) .. "\n"
	elseif sampledTable[specField].type == "number" then 
			return(string.format("%s: 0x%0".. math.ceil((fieldSize)/4) .. "x\n", specField, sampledTable[specField].data))
	elseif sampledTable[specField].type == "raw" then 
		if type(sampledTable[specField].data) == "string" then
			return(string.format("%s: %s\n", specField, sampledTable[specField].data))
		else 
			return(string.format("%s: 0x%0".. math.ceil((fieldSize)/4) .. "x\n", specField, sampledTable[specField].data))
		end
	else
			return(string.format("%s: %s\n", specField, sampledTable[specField].data))
	end
end

--------------------------------------------
--  Show Sample All 
--
-- prints interfaces in show sample all
--
-- input:
-- 	none
--
-- output:
--	none (prints)
--
--------------------------------------------
--------------------------------------------
local function idebug_show_sample_all()
	print()

	for interfaceName ,v in pairs(enabledInstances) do
		local reg, addr, regval = get_cpu_register(interfaceName, "General_Configurations")

		print(interfaceName)
		for i = 0, #interfaceName do 
			io.write("-")
		end
		print()
		local strRules = ""
		local strValues = ""
		    -- disable sampling before read sample data
	      regval = write_field(reg, "MatchSampleEnable", 0, regval)
	      iDebug_write_register(addr, regval, dfxPipe)
	 	for fieldName, tble in pairs(v.interface.fields) do
			if tble.isDisplay then 
 				strValues = strValues..show_sample_all_helper(interfaceName ,fieldName)
			end
		end
		    -- reenable sampling
		  regval = write_field(reg, "MatchSampleEnable", 1, regval)
		  iDebug_write_register(addr, regval, dfxPipe)    
		
		if strValues ~= "" then print(strValues)
		else print("no fields to display") end
		_, _, countVal = get_cpu_register(interfaceName, "Match_Counter")
		local addendum = ""
		if countVal == 0 then addendum = " (NO PACKETS SAMPLED SINCE LAST CALL TO SHOW)" end
		print("\npackets sampled: " .. countVal .. addendum) -- reads counter
		
		_, _, peakLogValue = get_cpu_register(interfaceName, "Peak_Logging_Value")
		print("\npushback clock cycle count: " .. peakLogValue.."\n") 

	end
end


--------------------------------------------
--  Idebug Show Sample
--
-- displays sample on screen
--
-- input:
-- 	interfaceName:string	- name of the interface to show sample of (can be nil)
--	field:string			- name of field to display (can be nil)
--
-- output:
--	none (prints)
--
--------------------------------------------
--------------------------------------------
local  function idebug_show_sample(interfaceName, specField)
	if interfaceName == nil then interfaceName = currentInterface end
	if interfaceName == "all" then idebug_show_sample_all() return end
	
	if enabledInstances[interfaceName] == nil then 
		print("interface is not enabled")
		return
	end

    local instance = instances[enabledInstances[interfaceName].interface.instanceName]
    local interfaceType = interfaceTable[enabledInstances[interfaceName].interface.id]
    local dfxPipe = tonumber(instance.dfxPipe)
    local reg, addr, regval = get_cpu_register(interfaceName, "General_Configurations")
    local isSampling = regval % 2

    
    -- disable sampling before read sample data
    regval = write_field(reg, "MatchSampleEnable", 0, regval)
    iDebug_write_register(addr, regval, dfxPipe)

    local baseAddr, size, sample
    baseAddr= tonumber(instance.baseAddress)
    size = tonumber(interfaceType.size)
    sample = iDebug_read_sample(baseAddr, size, dfxPipe)

    -- reenable sampling
    regval = write_field(reg, "MatchSampleEnable", 1, regval)
    iDebug_write_register(addr, regval, dfxPipe)
    -- display fields in sorted order
    
	local sampledTable = parse_fields(interfaceType, sample)
	if specField ~= nil then
		if "ipv4_dip" == specField or "ipv4_sip" == specField then 
			local str = ""
			for c in string.gmatch(string.format("%08x", tonumber(sampledTable[specField].data)), "%x%x") do
			    	str = str.. (string.format("%d",tonumber(c, 16))) .. '.'
			end
			print(specField.. ": " ..string.sub(str, 1, -2))
		elseif sampledTable[specField].type == "number" then 
			print(string.format("%s: 0x%0".. math.ceil((value.bitStop - value.bitStart+1)/4) .. "x", specField, sampledTable[specField].data))
		elseif sampledTable[specField].type == "raw" then 
			if type(sampledTable[specField].data) == "string" then
				print(string.format("%s: %s", specField, sampledTable[specField].data))
			else 
--				print(string.format("%s: 0x%x", specField, sampledTable[specField].data))
				print(string.format("%s: 0x%0".. math.ceil((value.bitStop - value.bitStart+1)/4) .. "x", specField, sampledTable[specField].data))

			end
		else
				print(string.format("%s: %s", specField, sampledTable[specField].data))
		end
	else
		for fieldName, value in pairs_by_keys(sampledTable) do
		
			if "ipv4_dip" == fieldName or "ipv4_sip" == fieldName then 
				local str = ""
				for c in string.gmatch(string.format("%08x", tonumber(value.data)), "%x%x") do
				    	str = str.. (string.format("%d",tonumber(c, 16))) .. '.'
				end
				print(fieldName.. ": " ..string.sub(str, 1, -2))
			elseif value.type == "number" then 
--			print(string.format("%s: 0x%x", fieldName, value.data))
			print(string.format("%s: 0x%0".. math.ceil((value.bitStop - value.bitStart+1)/4) .. "x", fieldName, value.data))
			elseif value.type == "enum" then
				print(string.format("%s: %s", fieldName, value.data))
			else
				if type(value.data) == "string" then
					print(string.format("%s: %s", fieldName, value.data))
				else 
			print(string.format("%s: 0x%0".. math.ceil((value.bitStop - value.bitStart+1)/4) .. "x", fieldName, value.data))
				end

			end
		end 
	end
	_, _, countVal = get_cpu_register(interfaceName, "Match_Counter")
	local addendum = ""
	if countVal == 0 then addendum = " (NO PACKETS SAMPLED SINCE LAST CALL TO SHOW)" end
		print("\npackets sampled: " .. countVal .. addendum) -- reads counter

	_, _, peakLogValue = get_cpu_register(interfaceName, "Peak_Logging_Value")
		print("\npushback clock cycle count: " .. peakLogValue.."\n") 
end

-- helper function for lua CLI
 local function idebug_show_sample_helper(params)
 	local interfaceName = params.uniqueName
	local specField = params.field

	idebug_show_sample(interfaceName, specField)
end
--------------------CLI TYPES --------------------
local function check_param_idebug_directly_interface(param, data)
	for k in pairs(uniqueInterfaceTable) do 
		if param == k then
			return true, param, true
		end
	end

	if data == "directly_interface_all" and param == "all" then
		return true, param
	end

	return false, "Invalid Interface"
    
end

local function complete_value_idebug_directly_interface(param,data)

	local compl = {}
	for k in pairs(uniqueInterfaceTable) do 
		table.insert(compl,k)
	end
	help = {}
	help.def = ""
	if data == "directly_interface_all" then 
		table.insert(compl, "all")
	end

    return handle_completion_long_list(param, compl, help, help.def)
end	

CLI_type_dict["directly_interface"] = {
    checker = check_param_idebug_directly_interface,
    complete = complete_value_idebug_directly_interface,
    help = "iDebug interface name"
} 

CLI_type_dict["directly_interface_all"] = {
    checker = check_param_idebug_directly_interface,
    complete = complete_value_idebug_directly_interface,
    help = "iDebug interface name"
}

local function check_param_idebug_directly_enabled_interface(param)
	if next(enabledInstances) ~= nil then 
		if param == "all" then return true, param end
	else
		return false, "There are no enabled interfaces"
	end
	for k in pairs(enabledInstances) do 
		if param == k then
			return true, param
		end
	end
	return false, "Invalid Interface"
end

local function complete_value_idebug_directly_enabled_interface(param)
	local compl = {}
	for k in pairs(enabledInstances) do 
		table.insert(compl,k)
	end
	help = {}
	if next(compl) == nil then help.def = "There are no enabled interfaces"
	else 
		help.def = "" 
		table.insert(compl, "all")
	end
	
    return compl, help
end	


CLI_type_dict["directly_enabled_interface"] = {
    checker = check_param_idebug_directly_enabled_interface,
    complete = complete_value_idebug_directly_enabled_interface,
    help = "iDebug interface name"
} 




local function check_param_idebug_directly_instance(param)
	for k in pairs(instances) do 
		if param == k then 
			return true, param
		end
	end
	return false, "Invalid instance"
end

local function complete_value_idebug_directly_instance(param)
	local compl = {}
	for k in pairs(instances) do 
		table.insert(compl,k)
	end
	help = {}
	help.def = ""
    return handle_completion_long_list(param, compl, help, help.def)
end

CLI_type_dict["directly_instance"] = {
    checker = check_param_idebug_directly_instance,
    complete = complete_value_idebug_directly_instance,
    help = "iDebug instance name"
}

local function check_param_idebug_directly_available_interface(param)
	for k in pairs(uniqueInterfaceTable) do 
		if param == k then
			if uniqueInterfaceTable[k].enabled == 0 or uniqueInterfaceTable[k].enabled == 1 then
				return true, param
			else
				return false, "Interface is in use"
			end
		end
	end
	return false, "Invalid Interface"
end

local function complete_value_idebug_directly_available_interface(param)
	local compl = {}
	for k in pairs(uniqueInterfaceTable) do 
		if uniqueInterfaceTable[k].enabled == 0  or uniqueInterfaceTable[k].enabled == 1 then
			table.insert(compl,k)
		end
	end
	help = {}
	help.def = ""
    return handle_completion_long_list(param, compl, help, help.def)
end	

CLI_type_dict["directly_available_interface"] = {
    checker = check_param_idebug_directly_available_interface,
    complete = complete_value_idebug_directly_available_interface,
    help = "iDebug available interface name"
}


local function check_param_idebug_directly_instance(param)

	for k in pairs(instances) do 
		if param == k then 
			return true, param
		end
	end
	return false, "Invalid instance"
end

local function complete_value_idebug_directly_instance(param)

	local compl = {}
	for k in pairs(instances) do 
		table.insert(compl,k)
	end
	help = {}
	help.def = ""
    return handle_completion_long_list(param, compl, help, help.def)
end

CLI_type_dict["directly_instance"] = {
    checker = check_param_idebug_directly_instance,
    complete = complete_value_idebug_directly_instance,
    help = "iDebug instance name"
}





local function check_idebug_field(param, data)
	currentField = param
--	print("check field")
	for k in pairs(uniqueInterfaceTable[currentInterface].fields) do 
		if param == k then 
			return true, param
		end
	end
	if data == "idebug-field-all" and param == "all" then
		return true, param
	end
	return false, "Invalid Field Check"
end

local function complete_idebug_field(param, data)
	currentField = nil
--	print("complete field")
	local compl = {}
	for k in pairs(uniqueInterfaceTable[currentInterface].fields) do 
		table.insert(compl,k)
	end
	if data == "idebug-field-all" then 
		table.insert(compl, "all")
	end
	help = {}
	help.def = ""
    return  handle_completion_long_list(param, compl, help, help.def)
end

CLI_type_dict["idebug-field"] = {
    checker = check_idebug_field,
    complete = complete_idebug_field,
    help = "iDebug Field Name"
}
CLI_type_dict["idebug-field-all"] = {
    checker = check_idebug_field,
    complete = complete_idebug_field,
    help = "iDebug Field Name"
}


local function check_idebug_field_value(param)
	local field = interfaceTable[uniqueInterfaceTable[currentInterface].id].elements[currentField]
	if "ipv4_dip" == currentField or "ipv4_sip" == currentField then 
	    if string.match(param, "%d+%.%d+%.%d+%.%d+") then 
		    digits = ""
		    for d in string.gmatch(param, "%d+") do
		      	if tonumber(d) > 255 or tonumber(d) <0  then 
		      		return false, "IP address does not exist"
		      	else
		      		digits = digits .. string.format("%02x", d)
		      	end
		    end
		    return true, "0x"..digits 
	    else 
	    	return false, "not valid IP address form"
	    end
	else
--	print("check value")
		if field.type == "number" then
			if string.match(param, '0x[%x%X]')== nil then return false, "must hex beginning with 0x" end
			local maxValue = field.bitStop-field.bitStart+1
			if tonumber(param, 16) <= 2^maxValue-1  and tonumber(param, 16) >= 0 then 
				return true, param
			end
			return false, "Invalid value"
		elseif field.type == "enum" then
			for k,v in pairs(field.enums) do 
				if param == v then 
					return true, k
				end
			end
			return false, "Invalid enum"
		else
			if string.match(param,"0x[%x+%X+]") and (#param -2) * 4 <= field.bitStop-field.bitStart+1 then
				return true, param
			else 
				return false, "Invalid Value"
			end
		end
	end
end

local function complete_idebug_field_value(param)
	local field = interfaceTable[uniqueInterfaceTable[currentInterface].id].elements[currentField]
--	print("complete value")	
	help = {}
	if "ipv4_dip" == currentField or "ipv4_sip" == currentField then 
		compl = {}
		help.def = "must be of the form [0-255].[0-255].[0-255].[0-255]"
		return {}, help
	elseif field.type == "number" then
		compl = {}
		local maxV = tonumber(field.bitStop)-tonumber(field.bitStart)+1
		help.def = "a number between 0 and ".. string.format("0x%X", 2^maxV-1)
		return compl,help
	elseif field.type == "enum" then
		compl = {}
		for _,v in pairs(field.enums) do
			table.insert(compl, v) 
			help.def = "enum"
		end
		return compl, help
	else
		help.def = "enter hex number up to " .. tonumber(field.bitStop)-tonumber(field.bitStart)+1 .. " bits"
		return {}, help
	
	end
end


CLI_type_dict["idebug-field-value"] = {
    checker = check_idebug_field_value,
    complete = complete_idebug_field_value,
    help = "iDebug Field Name"
}

local function isEnabledChecker(param)
	if string.lower(param) == "enable" or string.lower(param) == "disable" then return true, string.lower(param)end
end

local function isEnabledComplete(param)
	local tble = {[1] = "enable", [2] = "disable"}
	local help = {}
	help.def = ""
	return tble,help
end

CLI_type_dict["isEnabled"] = {
    checker = isEnabledChecker,
    complete = isEnabledComplete,
    help = "Enable or Disable"
}

local function sampleModeChecker(param)
	if string.lower(param) == "last_match" or string.lower(param) == "first_match" then return true, string.lower(param)end
end

local function sampleModeComplete(param)
	local tble = {[1] = "last_match", [2] = "first_match"}
	local help = {}
	help.def = ""
	return tble,help
end

CLI_type_dict["sampleMode"] = {
    checker = sampleModeChecker,
    complete = sampleModeComplete,
    help = "last_match or first_match"
}


local function descriptorChecker(param)

	for k,v in ipairs(checkTable) do
		if param == v then return true,param end
	end
	return false, ""

end

local function descriptorComplete(param)
	matchedTable = {}
	help = {}
	help.def = ""
	for k, v in ipairs(checkTable) do 
		if prefix_matchi(param, v) then table.insert(matchedTable, v) end
	end
    return matchedTable, help

end

CLI_type_dict["descriptor"] = {
    checker = descriptorChecker,
    complete = descriptorComplete,
    help = "Descriptor"
}


local function exprChecker(param)
	return true, param
end

local function exprComplete(param)
	help = {}
	help.def =
        	    "^ - match only at the beginning of the subject string\n" .. 
		    "$ - match only at the end of the subject string\n"..
		    "* - 0 or more of previous expression\n"..
		    "+ - 1 or more of the previous expression"


    return {}, help

end

CLI_type_dict["expr"] = {
    checker = exprChecker,
    complete = exprComplete,
    help = "expression"
}


--------------------END CLI TYPES --------------------

-- adds all commands to the CLI
function register_iDebug_commands()
    CLI_addCommand("idebug", "reset interface", {
        help="resets an interface or all interfaces",
        func=reset_interface_helper,
        params={
            { type="values", 
            	{format=  "%directly_interface_all",  name = "interfaceName",  help = "interface name"}
            }
        }
    })


    CLI_addCommand("idebug", "show interface", {
        help="show all values of an interface",
        func=print_Interface_w_fields_helper,
        params={
            { type="values", 
            	{format=  "%directly_interface", name = "uniqueName", help = "interface name"},
            	   mandatory ={"uniqueName"}

            }
        }
    })

        CLI_addCommand("idebug", "show sample", {
        help="show sampled values of interface or all interfaces",
        func=idebug_show_sample_helper,
        params={
            { type="values", 
            	{format=  "%directly_enabled_interface", name = "uniqueName", help = "interface name"},
            	   mandatory ={"uniqueName"}

            }
        }
    })


    CLI_addCommand("idebug", "show configuration", {
        help="shows all enabled interface configuration",
        func=print_config
    })

    CLI_addCommand("idebug", "show instance-Interfaces", {
        help="Select an Instance -not necessary to use ever",
        func=print_instance_helper,
        params={
            { type="values", 
            	{format=  "%directly_instance",  name = "instanceName",  help = "instance name"}
            }
        }
    })

    CLI_addCommand("idebug", "interface", {
        help="Select an interface",
        func=interface_mode_helper,
        params={
            { type="values", 
            	{format=  "%directly_available_interface",  name = "interfaceName",  help = "interface name"}
            }
        }
    })


        CLI_addCommand("idebug", "descriptor", {
        help="Select a descriptor",
        func=interface_mode_helper,
        params={
            { type="values", 
            	{format=  "%descriptor",  name = "interfaceName",  help = "descriptor name"}
            }
        }
    })


    CLI_addCommand("idebug-select-interface", "enable", {
        help="enables current interface",
        func=enable_interface,
    })


    CLI_addCommand("idebug-select-interface", "rule field", {
        help="sets rule to sample only packets that contain this rule",
        func=rule_function,
        params={
            { type="values", {format=  "%idebug-field",  name = "field", multiplevalues = false,  help = "The Field Name"}},
            { type = "named", 
            				   {format=  "value %idebug-field-value",  name = "value",  multiplevalues = false, help = "The Value for Given Field"},
            				   {format = "mask %idebug-field-value-mask", name = "mask", help = "mask for field"},
           						requirements = {value = {"field"}, mask = {"value"}},
           						mandatory = {"value"}

             }
          }
         })    
    CLI_addCommand("idebug-select-interface", "rule inverse_key", {
        help="inverses rules (sample only packets that DO NOT contain set rules)",
        func=rule_function,
        params={
	        { type="values", 
	            {format=  "%isEnabled",  name = "isReverse",  help = "inverse key"}
	            }
	 }})

CLI_addCommand("idebug", "search interfaces", {
        help="search interfaces by fields using a lua pattern",

        func=search_helper,
        params={
	        { type="values", 
          		  {format=  "%expression",  name = "expr",  help =
          	    "     str - match if str is present in field name\n"..
        	    "     ^str - match only at the beginning of the field name\n" .. 
		    "     str$ - match only at the end of the field name"}
	            }

	 } 
})

CLI_addCommand("idebug", "search descriptors", {
        help="search descriptors by fields using a lua pattern",

        func=search_descriptor_helper,
        params={
	        { type="values", 
          		  {format=  "%expression",  name = "expr",  help = 
          	    "     str - match if str is present in field name\n"..
        	    "     ^str - match only at the beginning of the field name\n" .. 
		    "     str$ - match only at the end of the field name"}
	            }

	 } 
})


    CLI_addCommand("idebug-select-interface", "rule sample_mode", {
        help="samples first sampled packet or continuously samples packets",
        func=rule_function,
        params={
	        { type="values", 
          		  {format=  "%sampleMode",  name = "sampleMode",  help = "sample mode"}
	            }
	 }})




    CLI_addCommand("idebug-select-interface", "show details", {
        help="show interface information",
        func=function()
        	local params = {}
        	params.uniqueName = currentInterface
              print_Interface_w_fields_helper(params)
        end
    })    

        CLI_addCommand("idebug-select-interface", "reset", {
        help="resets rules and displaying of field or all fields",
        func=reset_rule_helper,
        params={{ type="named", {format=  "field %idebug-field-all",  name = "field", multiplevalues = false,  help = "The Field Name"}}}
    })    

        CLI_addCommand("idebug-select-interface", "display", {
        help="will display field in 'show sample all'",
        func=set_display_helper,
        params={{ type="named", {format=  "field %idebug-field-all",  name = "field", multiplevalues = false,  help = "The Field Name"}}}
    })    

        CLI_addCommand("idebug-select-interface", "no display", {
        help="will not display field in 'show sample all'",
        func= function (params)
        			params.flagNo = true
        			return set_display_helper(params)
        	     end,
        params={{ type="named", {format=  "field %idebug-field-all",  name = "field", multiplevalues = false,  help = "The Field Name"}}}
    })    

   

    CLI_addCommand("idebug-select-interface", "show rules", {
        help="shows all rules (must enable to configure)",
        func=function()
        	local params = {}
        	params.uniqueName = currentInterface
              print_fields_helper(params)
        end
    })    

        CLI_addCommand("idebug-select-interface", "show sample", {
        help="show interface sample",
        func=idebug_show_sample_helper,
        params={{ type="named", {format=  "field %idebug-field",  name = "field", multiplevalues = false,  help = "The Field Name"}}}
    })    


        CLI_addCommand("idebug-select-interface", "disable", {
        help="disables current interface",
        func=function()
        	local params = {}
        	params.uniqueName = currentInterface
              disable_interface_helper(params)
        end
    })    


    -- debug: idebug device %devID
    CLI_addCommand("debug", "idebug", {
        help="Enter iDebug command mode",
        func=idebug_enter_helper,
        params={
            { type="values",
              "device",
              "%devID"
            }
        }
    })


--    CLI_prompts["idebug-select-instance"] = "(idebug)#"
    CLI_prompts["idebug"] = "(idebug)#"
    CLI_prompts["idebug-interface"] = "(idebug-interface)#"
    -- debug: reload idebug
    CLI_addHelp("debug", "reload", "Reload")
    CLI_addCommand("debug", "reload idebug", {
        help="reload iDebug XML",
        func=function(params)
            return iDebug_load_xml(true)
        end
    })

	
end

-- load at startup if iDebug.xml present
if lookup_iDebug_xml() then
    iDebug_initialize()
end
  if not iDebug_initialized then
    CLI_addCommand("debug", "load iDebug", {
        help = "Load iDebug.xml, initialize 'call cpss api ...'",
        func = iDebug_initialize
    })
  end

end
