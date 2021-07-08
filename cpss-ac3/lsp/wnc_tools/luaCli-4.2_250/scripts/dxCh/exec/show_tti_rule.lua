--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tti_rule.lua
--*
--* DESCRIPTION:
--*       tti commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--constants


--includes


-- variables definitions
local 	 enumKeyTypeTbl


-- ************************************************************************
---
--  listToSequences
--        @description  convert list of ports to a list of sequences
--						for example: {1,2,3,4,8,10,11,12} -> {1-4,8,10-12}
--

local function listToSequences(portsList)
	local res = ""
	local startSeq = nil
	local last
	local endSeq = nil
	for k,v in pairs(portsList) do
		if startSeq == nil then
			startSeq = v
		else
			if (v ~= last + 1) then
				endSeq = last
				if startSeq ~= endSeq then
					res = res .. "," .. startSeq .. "-" .. endSeq
				else
					res = res .. "," .. startSeq
				end
				startSeq = v
			end
			
		end
		last = v
	end
	if (startSeq ~= nil) then
		endSeq = last
		if startSeq ~= endSeq then
			res = res .. "," .. startSeq .. "-" .. endSeq
		else
			res = res .. "," .. startSeq
		end		
	end
	
	if (startSeq ~= nil) then
		if (prefix_match(",",res)) then
			res = string.sub(res, 2)
		end
	end
	return res
end

-- ************************************************************************
---
--  getKeyTypeName
--        @description  get the TTI rule type from the table
--
local function getKeyTypeName(key)
	return enumKeyTypeTbl[key]
end

-- ************************************************************************
---
--  createKeyTypeTbl
--        @description  if the TTI rule types table is null, than create it
--        @param dev             - device number
--

local function createKeyTypeTbl(dev)
    if (enumKeyTypeTbl == nil) then
		enumKeyTypeTbl = {}
		table.insert(enumKeyTypeTbl,"legacy-ipv4-tunnel")
		table.insert(enumKeyTypeTbl,"legacy-mpls")
		table.insert(enumKeyTypeTbl,"legacy-ethernet")
		table.insert(enumKeyTypeTbl,"legacy-mim")
        if is_device_eArch_enbled(dev) then
    		table.insert(enumKeyTypeTbl,"ipv4-tcp")
    		table.insert(enumKeyTypeTbl,"ipv4-udp")
    		table.insert(enumKeyTypeTbl,"mpls")
    		table.insert(enumKeyTypeTbl,"ipv4-fragment")
    		table.insert(enumKeyTypeTbl,"ipv4")
    		table.insert(enumKeyTypeTbl,"other-ethernet")
    		table.insert(enumKeyTypeTbl,"ipv6")
    		table.insert(enumKeyTypeTbl,"ipv6-tcp")
    		table.insert(enumKeyTypeTbl,"ipv6-udp")
    		table.insert(enumKeyTypeTbl,"ether-type1")
    		table.insert(enumKeyTypeTbl,"ether-type2")
    		table.insert(enumKeyTypeTbl,"ether-type3")
    		table.insert(enumKeyTypeTbl,"ether-type4")
    		table.insert(enumKeyTypeTbl,"ether-type5")
    		table.insert(enumKeyTypeTbl,"ether-type6")
    		table.insert(enumKeyTypeTbl,"ether-type7")
        end
	end
end

-- ************************************************************************
---
--  tti_show_enable
--        @description  show for each TTI rule type, its enabled ports
--
--        @param devNum             - device number
--        @param packetType		    - TTI rule type, if the param is entered, 
--									  shows ports only for the given type
--
local function tti_show_enable(params)
    -- Main port handling cycle
    local command_data = Command_Data()
    local keyType = params["packetType"]
    local header_string = 
        "\n" ..
        "   packet Type               Enabled On     \n" ..
        "------------------     ---------------------\n"
    local footer_string = "\n"  
    local devId = params["devID"]
    local keyTypesPortsTable = {}
	local portsAsSequences = ""
	createKeyTypeTbl(devId)
	
	
    params["all"] = true
    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)       
    if true == command_data["status"] then
        local iterator
		
		for i=0, #enumKeyTypeTbl-1 do
			key = i
			if keyType ~= nil then
				key = keyType
			end
			local portsList = {}
			for iterator, devNum, portNum in command_data:getPortIterator() do
				if (devNum == devId) then
					ret,val = myGenWrapper("cpssDxChTtiPortLookupEnableGet",{
						{"IN","GT_U8","devNum",devNum},
						{"IN","GT_PORT_NUM","portNum",portNum},
						{"IN","CPSS_DXCH_TTI_KEY_TYPE_ENT","keyType",key},
						{"OUT","GT_BOOL","enablePtr"}
					})

					if (ret ~= 0) then
							print("failed to get tti enabled/disable on device " .. devNum .. " port " .. portNum)                
					else
						if (val ~= nil and val.enablePtr==true) then
							table.insert(portsList,portNum)
						end
					end
				end
				
			end
			portsAsSequences = listToSequences(portsList)
			
			keyTypesPortsTable[key+1] = portsAsSequences
			if keyType ~= nil then
				break
			end
		end
    end
	
    print(header_string)
	
	for k,v in pairs(keyTypesPortsTable) do
		print(string.format("%-25s%-40s", getKeyTypeName(k),v))
	end
	
	print(footer_string)

end


local function tti_show_pclid(params)
    local keyType = params["packetType"]
    local header_string = 
        "\n" ..
        "   packet Type          PCLID   \n" ..
        "------------------   -----------\n"
    local footer_string = "\n"  
    local devId = params["devID"]
    local keyTypesPortsTable = {}
	local portsAsSequences = ""
	createKeyTypeTbl(devId)
	
	
	for i=0, #enumKeyTypeTbl-1 do
		key = i
		if keyType ~= nil then
			key = keyType
		end
		ret,val = myGenWrapper("cpssDxChTtiPclIdGet",{
			{"IN","GT_U8","devNum",devId},
			{"IN","CPSS_DXCH_TTI_KEY_TYPE_ENT","keyType",key},
			{"OUT","GT_U32","pclIdPtr"}
		})

		if (ret ~= 0) then
			print("failed to get tti pclID on device " .. devNum)                
		else
			if (val ~= nil and val.pclIdPtr ~= nil) then
				keyTypesPortsTable[key+1] = val.pclIdPtr
			end
		end

		if keyType ~= nil then
			break
		end
	end
	
    print(header_string)
	
	for k,v in pairs(keyTypesPortsTable) do
		print(string.format("%-25s%-40s", getKeyTypeName(k),v))
	end
	
	print(footer_string)

end

local function getListOfUnmaskedKeys(keys, masks)
	local list = {}
	for k,v in pairs(masks) do
		if type(v) == "table" then
			local tmpTbl = getListOfUnmaskedKeys(keys[k], v)
			for tmpK,tmpV in pairs(tmpTbl) do
				list[tmpK] = tmpV
			end
		else
			if ( v ~= 0 and v ~= false and v ~= "00:00:00:00:00:00") then
				list[k] = keys[k]
			end
		end
	end
	return list
end

local function commandEnum(v)
    local enum = {}
	enum["CPSS_PACKET_CMD_FORWARD_E"] = "forward"
	enum["CPSS_PACKET_CMD_MIRROR_TO_CPU_E"] = "mirror"
	enum["CPSS_PACKET_CMD_TRAP_TO_CPU_E"] = "trap"
	enum["CPSS_PACKET_CMD_DROP_HARD_E"] = "hard-drop"
	enum["CPSS_PACKET_CMD_DROP_SOFT_E"] = "soft-drop"
	return enum[v]
end

local function modifyEnum(v)
    local enum = {}
	enum["CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E"] = "not modify"
	enum["CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"] = "untagged"
	enum["CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E"] = "tagged"
	enum["CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E"] = "all"
	return enum[v]
end

local function getListOfActions(actions)
	local list, tagCmd, tagVid
	list = {}
	for k,v in pairs(actions) do
		if k == "command" then
			list[k] = commandEnum(v)
		end
		if k == "tag0VlanCmd" then
			tagCmd = v
		end
		if k == "tag0VlanId" then
			tagVid = v
		end
	end
	if (tagCmd ~= nil and tagVid ~= nil) then
		list["vlan0-assign"] = modifyEnum(tagCmd) .. ", " .. to_string(tagVid)
	end
	return list
end

local function tti_show_rule(params)
    local keyType = params["packetType"]
    local devId = params["devID"]
	local ruleIndex = params["rule-index"]
	local actionType = 1
	local portsAsSequences = ""
	createKeyTypeTbl(devId)
	local numberOfindices
    local offsetInTCAM,v 
	numberOfindices = getNumberOfTTIindices(devId)

	print("Device: " .. devId)
-- for i=0, #enumKeyTypeTbl-1 do
	-- local key = i
	-- if keyType ~= nil then
		-- key = keyType
	-- end
	keyType = 2
	key = keyType
	for j=0, numberOfindices-1 do 
		local currRuleIndex = j
		if ruleIndex ~= nil then
			currRuleIndex = ruleIndex
		end
		ret,val = myGenWrapper("cpssDxChTtiRuleGet",{
			{"IN","GT_U8","devNum",devId},
			{"IN","GT_U32","index",getIndexInTCAM(currRuleIndex,devId)},
			{"IN","CPSS_DXCH_TTI_KEY_TYPE_ENT","ruleType",key},
			{"OUT","CPSS_DXCH_TTI_RULE_UNT_eth","patternPtr"},
			{"OUT","CPSS_DXCH_TTI_RULE_UNT_eth","maskPtr"},
			{"IN","CPSS_DXCH_TTI_ACTION_TYPE_ENT","actionType", actionType},
			{"OUT","CPSS_DXCH_TTI_ACTION_UNT_type2","actionPtr"}
		})

		if (ret ~= 0) then
			print("failed to get rule " .. currRuleIndex .. " on device " .. devId .. ", failed to get packet-type " .. getKeyTypeName(key+1))                
		else
			print("Rule " .. currRuleIndex)
			print("  Packet Type: " .. getKeyTypeName(key+1))
			print("  Keys:")
			
			local list = getListOfUnmaskedKeys(val["patternPtr"], val["maskPtr"])
			for k,v in pairs(list) do
				print("    " .. k .. ": " .. to_string(v))
			end				
			
			print("  Actions:")
			list = getListOfActions(val["actionPtr"]["type2"])
			for k,v in pairs(list) do
				print("    " .. k .. ": " .. to_string(v))
			end		

		end
		if ruleIndex ~= nil then
			break
		end
	end
		
	-- if keyType ~= nil then
		-- break
	-- end
-- end
	
	

end

--------------------------------------------
-- command registration: show tti enable
--------------------------------------------
CLI_addHelp("exec", "show", "Display running system information")
CLI_addHelp("exec", "show tti", "show tti configurations")
CLI_addCommand("exec", "show tti enable", {
    help="display ports on which TTI Lookup is enabled",
    func=tti_show_enable,
    params={
        { type="values",
          "%devID"
        },
        { type="named",
            { format="%packetType", name="packetType", help = "Display ports on which TTI Lookup is enabled for the given packet type" }
        }
    }
})

--------------------------------------------
-- command registration: show tti pclid
--------------------------------------------
CLI_addCommand("exec", "show tti pclid", {
    help="display TTI PCLIDs",
    func=tti_show_pclid,
    params={
        { type="values",
          "%devID"
        },
        { type="named",
            { format="%packetType", name="packetType", help = "Display PCLID of the given packet type" }
        }
    }
})

-- --------------------------------------------
-- -- command registration: show tti rule
-- --------------------------------------------
-- CLI_addCommand("exec", "show tti rule", {
    -- help="display TTI PCLIDs",
    -- func=tti_show_rule,
    -- params={
        -- { type="values",
          -- "%devID"
        -- },
        -- { type="named",
            -- { format="packetType %packetType", name="alt1", help = "Display PCLID of the given packet type" },
            -- { format="rule-index %index", name="alt1", help = "Display the given rule" }
        -- }
    -- }
-- })

