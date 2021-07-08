--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_bridge_address_table.lua
--*
--* DESCRIPTION:
--*       showing of FDB
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants

--
-- CPSS DxCh Vlan functions
--

--
-- Import Lua C functions from wraplCpssDxChVlan.c
--


-- ************************************************************************
---
--  applyFDBFilters
--        @description  Checks if the entry satisfies the filters in
--                      params
--
--        @param entry          - The FDB entry to check
--        @param params         - The hash table with the parameters /
--                                filters
--
--        @return       true if passes otherwise false
--
function applyFDBFilters(entry, params)
	--vlan
	if params.vlan ~= nil then
--  if entry.entry.key.vlanId ~= params.vlan then
		if ((entry.entry["key"]["key"] or {})["macVlan"] or {})["vlanId"] ~= params.vlan then
			return false
		end
	end
	--interface
	if params["ethernet"] ~= nil then
		if (entry.entry.dstInterface.type ~= "CPSS_INTERFACE_PORT_E") then
			return false
		else
			if (entry.entry.dstInterface.index ~= params["ethernet"]) then
				return false
			end
		end
	end
	--port-channel
	if params["port-channel"] ~= nil then
		if (entry.entry.dstInterface.type ~= "CPSS_INTERFACE_TRUNK_E") then
			return false
		else
			if (entry.entry.dstInterface.trunkId ~= params["port-channel"]) then
				return false
			end
		end
	end

	--mac-address
	if params["address"] ~= nil then
--		if (entry.entry.key.macAddr ~= params["address"]["string"]) then
    if ( ((entry.entry["key"]["key"] or {})["macVlan"] or {})["macAddr"] ~= params["address"]["string"]) then
			return false
		end
	end

	--type
	if params["entry_type"] ~= nil then
		if (params.entry_type == "static") and (entry.entry.isStatic == false) then
			return false
		elseif (params.entry_type == "dynamic") and (entry.entry.isStatic == true) then
			return false
		end
	end


	return true
end


-- ************************************************************************
---
--  printFDBEntry
--        @description  Prints the entry
--
--        @param index          - The index of the entry
--        @param entry          - The FDB entry to check
--
--        @return       nothing
--
function printFDBEntry(index, entry)
	local interface, da_route, static, skip

	if (entry.entry.dstInterface.type == "CPSS_INTERFACE_PORT_E") then
		interface = "Eth " ..   entry.entry.dstInterface.devPort.portNum
	elseif (entry.entry.dstInterface.type == "CPSS_INTERFACE_VIDX_E") then
		interface = "VIDX " ..   entry.entry.dstInterface.vidx
	elseif (entry.entry.dstInterface.type == "CPSS_INTERFACE_VID_E") then
		interface = "VLAN " ..   entry.entry.dstInterface.vlanId
	elseif (entry.entry.dstInterface.type == "CPSS_INTERFACE_TRUNK_E") then
		interface = "Ch " ..   entry.entry.dstInterface.trunkId
	else
		print("unkown interface type " .. entry.entry.dstInterface.type)
		interface = "ERR"
	end

	if (entry.entry.isStatic) then
		static = "true"
	else
		static = "false"
	end
	if (entry.entry.daRoute) then
		da_route = "true"
	else
		da_route = "false"
	end
	if (entry.skip) then
		skip = "true"
	else
		skip = "false"
	end
	print(string.format("%-8d%-19s%-9d%-8s%-11s%-8s%s", index, entry.entry.key.key.macVlan.macAddr, entry.entry.key.key.macVlan.vlanId, skip, interface, static, da_route))
end

-- limit the iterations (in GM it uses the 'FDB shadow' due to poor performance of the GM)
local function gmUsed_show_FdbEntries(params)
    local command_data = Command_Data()
    local local_params = {all_device = {devNum}}
-- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceMacEntryIterator(
        local_params, { ["dstInterface"] =
                    {["type"]    = { "CPSS_INTERFACE_PORT_E",
                                     "CPSS_INTERFACE_TRUNK_E",
                                     "CPSS_INTERFACE_VID_E"    }}})
                                     
    --printLog("local_params:" ,to_string(local_params))
                                     
    for iterator, _devNum, entry_index in
                                command_data:getValidMacEntryIterator() do
        --printLog("iterator" , to_string(iterator))
        --printLog("_devNum" , to_string(_devNum))
        --printLog("entry_index" , to_string(entry_index))
                                
        local result, values = myGenWrapper(
        "cpssDxChBrgFdbMacEntryRead", {
            { "IN", "GT_U8", "dev", devNum},            -- devNum
            { "IN", "GT_U32", "index", entry_index },
            { "OUT","GT_BOOL", "valid" },               -- *validPtr,
            { "OUT","GT_BOOL", "skip" },                -- *skipPtr,
            { "OUT","GT_BOOL", "aged" },                -- *agedPtr,
            { "OUT","GT_U8", "devNum" },                -- *associatedDevNumPtr,
            { "OUT","CPSS_MAC_ENTRY_EXT_STC", "entry" } --  *entryPtr
        })

        if result == 0 then
			if values.valid then -- valid
				result = applyFDBFilters(values, params)
				if (result) then
					printFDBEntry(entry_index, values)
				end
            end
        end
    end

    print("")

	return true

end

-- ************************************************************************
---
--  show_bridge_address_table
--        @description  Displays the FDB entries and filters them
--
--        @param params         - The hash table with the parameters
--
--        @return       true, if successful otherwise the error message
--
function show_bridge_address_table(params)
	local indexCnt
	local fdbEnd
	local result, values
	local matches
    local fromShadow = false

	matches = 0
	indexCnt = 0
	fdbEnd = false

    if isGmUsed() or isEmulatorUsed() then
        fromShadow = true
    end
    
    if fromShadow then
        print(" PRINT FROM SHADOW ... due to poor performance")
    end

	print("")
	print("Index   Address            Vlan     Skip    Interface  Static  DA_Route")
	print("-----   -----------------  ------   ------  ---------  ------  --------")

    if fromShadow then
        return gmUsed_show_FdbEntries(params)
    end

	repeat
		result, values = myGenWrapper(
			"cpssDxChBrgFdbMacEntryRead", {
				{ "IN", "GT_U8", "dev", params["devID"]}, -- devNum
				{ "IN", "GT_U32", "index", indexCnt },
				{ "OUT","GT_BOOL", "valid" }, -- *validPtr,
				{ "OUT","GT_BOOL", "skip" }, -- *skipPtr,
				{ "OUT","GT_BOOL", "aged" }, -- *agedPtr,
				{ "OUT","GT_U8", "devNum" },   -- *associatedDevNumPtr,
				{ "OUT","CPSS_MAC_ENTRY_EXT_STC", "entry" } --  *entryPtr
			})
		if result == 0 then
			if values.valid then -- valid
				result = applyFDBFilters(values, params)
				if (result) then
					matches = matches + 1
					printFDBEntry(indexCnt, values)
				end
            end
        elseif result == 3 then
        	fdbEnd = true
        else
        	return false, returnCodes[result]
        end
        indexCnt = indexCnt + 1
    until fdbEnd
    print("")

	return true
end

-- *exec*
-- show bridge address-table %devID [ vlan %vlan ] [ ethernet %port-range | port-channel %port-range ] [address %mac-address ] [ static | dynamic | all ] [ count ]
CLI_addHelp("exec", "show bridge", "Bridging information")
CLI_addCommand("exec", "show bridge address-table", {
    func = show_bridge_address_table,
    help = "Display forwarding table",
    params={
        { type="values", "%devID" },
        { type="named",
            { format="vlan %vlan", help="Filter by VLAN" },
            "#interface",
            { format="address %mac-address", help="Sorce MAC-address" },
            { format="static", name="entry_type", help="Display only the statically configured addresses" },
            { format="dynamic", name="entry_type", help="Display only the dynamically configured addresses"  },
            { format="all", name="entry_type", help="Display only the dynamically and statically configured addresses" },
            { format="count", help="Displays the amount of entries instead of the entries themself" }
        }
    }
})
