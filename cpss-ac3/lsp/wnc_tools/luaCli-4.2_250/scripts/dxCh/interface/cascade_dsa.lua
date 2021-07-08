--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cascade_dsa.lua
--*
--* DESCRIPTION:
--*       setting of the port to be DSA-tagged
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

local function cascadeDsaFormat(params)
    local command_data = Command_Data()

    local func = function (command_data, devNum, portNum, params)

        if not is_supported_feature(devNum, "HASH_IN_DSA") then
            command_data:addErrorAndPrint(
                string.format("device %d doesn't support feature \"hash in DSA tag\"", devNum))
            return
        end

        local enable = not (params.flagNo==true)

        local direction = "CPSS_PORT_DIRECTION_BOTH_E"
        if params.rx then
            direction = "CPSS_PORT_DIRECTION_RX_E"
        elseif params.tx then
            direction = "CPSS_PORT_DIRECTION_TX_E"
        end -- else BOTH

        local result, values = myGenWrapper(
            "cpssDxChCscdHashInDsaEnableSet", {
                { "IN", "GT_U8",                   "devNum",    devNum},
                { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",   portNum},
                { "IN", "CPSS_PORT_DIRECTION_ENT", "direction", direction},
                { "IN", "GT_BOOL",                 "enable",    enable}
            })

        command_data:handleCpssErrorDevPort(
            result,
            string.format("enabling (%s) hash in eDSA tag", enable and "true" or "false"),
            devNum, portNum)
    end --func

   command_data:initInterfaceDevPortRange()
   command_data:iterateOverPorts(func, params)
   command_data:analyzeCommandExecution()
   command_data:printCommandExecutionResults()
   return command_data:getCommandExecutionResults()

end

local function trunk_db_type_get(command_data, devNum, port_channel)
    local result, values
    local trunkType
    result, values = myGenWrapper("prvCpssGenericTrunkDbTrunkTypeGet",
                {{ "IN", "GT_U8",    "dev",  devNum },
                { "IN", "GT_TRUNK_ID",    "trunkId", port_channel  },
                { "OUT", "CPSS_TRUNK_TYPE_ENT",  "trunkType", trunkType}})

    if 0 ~= result then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error getting generic trunk: %s",
                                  to_string(returnCodes[result]))
        return nil
    end
    return values.trunkType
end

local function convert_devport_to_hw(command_data, devNum, portNum)
    local result, hwPortNum, hwDevNum
        result, hwDevNum, hwPortNum =
                    device_port_to_hardware_format_convert(devNum, portNum)
        if        0 == result                   then
            return true, hwDevNum, hwPortNum
        elseif 0x10 == result                   then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Device id %d and port number %d " ..
                                  "converting is not allowed.", devNum,
                                  portNum)
        elseif    0 ~= result                   then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error at device id %d and port " ..
                                  "number %d converting: %s.", devNum,
                                  portNum, to_string(returnCodes[result]))
        end
    return false
end

--add or remove port to regular
local function add_remove_trunk_member(command_data,action,devNum,portNum,port_channel)
    local result
    local hwDevNum, hwPortNum
    result, hwDevNum, hwPortNum =
            convert_devport_to_hw(command_data, devNum, portNum)
    if not result then
        return false
    end
    local trunk_member = { device = hwDevNum, port = hwPortNum }
    result = cpssPerTrunkParamSet("cpssDxChTrunkMember"..action,
                                                  devNum, port_channel,
                                                  trunk_member, "member",
                                                  "CPSS_TRUNK_MEMBER_STC")
    if result == 0 then
        return true
    end
    if 0x10 == result                   then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Can't "..action.." port member to trunk %d"..
                              " on device %d: not allowed", port_channel, devNum)
    elseif    0 ~= result                   then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error at device id %d and port " ..
                              "number %d converting: %s.", devNum,
                              portNum, to_string(returnCodes[result]))
    end
    return false
end


local function cascade_dsa_one_port(command_data, devNum, portNum, port_type)
    local result, values
    command_data:clearPortStatus()

    command_data:clearLocalStatus()

    -- Cascade port type setting.
    if true == command_data["local_status"] then
        result, values = myGenWrapper("cpssDxChCscdPortTypeSet",
                            {{ "IN", "GT_U8",    "dev",  devNum   },
                               { "IN", "GT_U8",    "port", portNum  },
                               { "IN", "CPSS_PORT_DIRECTION_ENT",  "portDirection", "CPSS_PORT_DIRECTION_BOTH_E"},
                               { "IN", "CPSS_CSCD_PORT_TYPE_ENT",  "portType", port_type }}
                              )
        if  0x10 == result then
            command_data:setFailPortStatus()
            command_data:addWarning("It is not allowed to set DSA " ..
                                    "mode \"%s\" on device %d port %d.",
                                    cascadePortTypeStrGet(port_type),
                                    devNum, portNum)
        elseif 0 ~= result then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error at \"%s\" DSA mode setting " ..
                                  "on device %s port %s: %s",
                                  to_string(cascadePortTypeStrGet(port_type)),
                                  to_string(devNum), to_string(portNum), to_string(returnCodes[result]))
        end

    end


    command_data:updateStatus()

    command_data:updatePorts()
end


local function change_trunk_type_to_cascade(command_data,port_channel)
    local portsMembers = wrLogWrapper("wrlCpssClearBmpPort")
    local iterator, devNum, portNum
    local devNumPrev = nil
    local result, values

    -- step1: check all trunk members belong to the same device
    command_data:initInterfaceRangeIterator()

    if #command_data.dev_port_range > 1 then
        print("error in device number. Port range: " .. to_string(command_data.dev_port_range))
        return false
    end

    -- Main port handling cycle.
    if not command_data["status"] then
        return false
    end

    command_data:initInterfaceRangeIterator()
    for iterator, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        command_data:clearLocalStatus()

        devNumPrev = devNum

        local trunk_type = trunk_db_type_get(command_data, devNum, port_channel)
        if trunk_type == "CPSS_TRUNK_TYPE_CASCADE_E" then
            return true
        end
        
        
        --Remove port from trunk
        result = add_remove_trunk_member(command_data,
                "Remove",devNum,portNum,port_channel)
        if not result then
            return false
        end
        portsMembers = wrLogWrapper("wrlCpssSetBmpPort","(portsMembers, portNum)",portsMembers, portNum)
            
    end

    if devNumPrev == nil then
        return -- nothing to do
    end
    -- add ports to cascade
    result, values = myGenWrapper("cpssDxChTrunkCascadeTrunkPortsSet", {
                    { "IN", "GT_U8",    "dev",  devNumPrev   },
                    { "IN", "GT_TRUNK_ID",    "trunkId", port_channel  },
                    { "IN", "CPSS_PORTS_BMP_STC",  "portsMembers", portsMembers}})

    if 0 ~= result then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error at cascade trunk ports setting " ..
                                      "on device %d: %s",
                                      devNum, to_string(returnCodes[result]))
    end
end

local function no_cascade_dsa_port_channel(command_data, params, port_type)
    local result, values
    local devNum, portNum

    local port_channel = getGlobal("ifPortChannel")

    devNum = params.devID


    local trunk_type = trunk_db_type_get(command_data, devNum, port_channel)
    if trunk_type ~= "CPSS_TRUNK_TYPE_CASCADE_E" then
        return -- nothing to do
    end


    result, values = myGenWrapper("cpssDxChTrunkCascadeTrunkPortsGet", {
                    { "IN", "GT_U8",    "dev",  devNum   },
                    { "IN", "GT_TRUNK_ID",    "trunkId", port_channel  },
                    { "OUT", "CPSS_PORTS_BMP_STC",  "portsMembers"}})

    if 0 ~= result then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error at cascade trunk ports getting " ..
                                  "on device %d: %s",
                                  devNum, to_string(returnCodes[result]))
    end

    local portsMembers = values.portsMembers
    for portNum = 0, 255 do
        if wrLogWrapper("wrlCpssIsPortSetInPortsBmp","(portsMembers, portNum)", portsMembers, portNum) then
            -- do for port portNum
            --print("process port "..tostring(portNum))
            cascade_dsa_one_port(command_data, devNum, portNum, "CPSS_CSCD_PORT_NETWORK_E")
        end
    end

    -- remove all ports from cascade
    result, values = myGenWrapper("cpssDxChTrunkCascadeTrunkPortsSet", {
                    { "IN", "GT_U8",    "dev",  devNumPrev   },
                    { "IN", "GT_TRUNK_ID",    "trunkId", port_channel  },
                    { "IN", "GT_PTR", "portsMembers", nil}})

    if 0 ~= result then
            command_data:setFailPortAndLocalStatus()
            command_data:addError("Error at cascade trunk ports setting " ..
                                      "on device %d: %s",
                                      devNum, to_string(returnCodes[result]))
    end

    for portNum = 0, 255 do
        if wrLogWrapper("wrlCpssIsPortSetInPortsBmp","(portsMembers, portNum)", portsMembers, portNum) then
            -- do for port portNum
            --TODO: add port to regular trunk --cpssDxChTrunkMemberAdd
            result = add_remove_trunk_member(command_data,
                "Add",devNum,portNum,port_channel)
        end
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()

end

--cascade dsa
local function cascade_dsa_port_channel(command_data, params, port_type)
    local result, values
    local devNum, portNum

    local port_channel = getGlobal("ifPortChannel")

    devNum = params.devID

    local trunk_type = trunk_db_type_get(command_data, devNum, port_channel)
    if trunk_type ~= "CPSS_TRUNK_TYPE_CASCADE_E" then
        if not change_trunk_type_to_cascade(command_data,port_channel) then
            command_data:analyzeCommandExecution()

            command_data:printCommandExecutionResults()

            return command_data:getCommandExecutionResults()
        end
    end


    result, values = myGenWrapper("cpssDxChTrunkCascadeTrunkPortsGet", {
                    { "IN", "GT_U8",    "dev",  devNum   },
                    { "IN", "GT_TRUNK_ID",    "trunkId", port_channel  },
                    { "OUT", "CPSS_PORTS_BMP_STC",  "portsMembers"}})

    if 0 ~= result then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error at cascade trunk ports getting " ..
                                  "on device %d: %s",
                                  devNum, to_string(returnCodes[result]))
    end

    local portsMembers = values.portsMembers
    for portNum = 0, 255 do
        if wrLogWrapper("wrlCpssIsPortSetInPortsBmp","(portsMembers, portNum)", portsMembers, portNum) then
            -- do for port portNum
            --print("process port "..tostring(portNum))
            cascade_dsa_one_port(command_data, devNum, portNum, port_type)
        end
    end
    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()

end

-- ************************************************************************
---
--  cascade_dsa_func
--        @description  set's of the port to be DSA-tagged
--
--        @param params         - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function cascade_dsa_func(params)
    local command_data = Command_Data()

    -- Command specific variables initialization.
    local port_type
    port_type   =
        command_data:getValueIfFlagNoIsNil(
            params, cascadePortTypeGet(params["regular_extended"]),
            "CPSS_CSCD_PORT_NETWORK_E")

    if getGlobal("ifPortChannel") ~= nil then
        if params.flagNo then
            return no_cascade_dsa_port_channel(command_data, params, port_type)
        else
            return cascade_dsa_port_channel(command_data, params, port_type)
        end
    end

    local result, values
    local devNum, portNum

    command_data:initInterfaceRangeIterator()


    -- Main port handling cycle.
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            cascade_dsa_one_port(command_data, devNum, portNum, port_type)
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not set the cascade port type for all processed ports.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: cascade dsa
--------------------------------------------------------------------------------
CLI_addHelp("interface",    "cascade",  "Setting of the port to be DSA-tagged")
CLI_addCommand("interface", "cascade dsa", {
  func   = cascade_dsa_func,
  help   = "Setting of the port to be DSA-tagged ",
  params = {
      { type = "named",
          "#regular_extended",
        mandatory = { "regular_extended" }
      }
  }
})


--------------------------------------------------------------------------------
-- command registration: no cascade dsa
--------------------------------------------------------------------------------
CLI_addHelp("interface",    "no cascade",
                                        "Returning to default of the port " ..
                                        "to be DSA-tagged")
CLI_addCommand("interface", "no cascade dsa", {
  func   = function(params)
               params.flagNo = true
               cascade_dsa_func(params)
           end,
  help   = "Returning to default of the port to be DSA-tagged "
})

--------------------------------------------------------------------------------

local function cascade_device_func(params)
    local command_data = Command_Data()
    if (tonumber(params.targDev) < 0 or tonumber(params.targDev)>127) then
		command_data:addError("The target device ID is out of range: 0..127")
	elseif params.ether ~= nil then
        cascade_device_ether(command_data, params)
    elseif params.chan ~= nil then
        cascade_device_chan_gr(command_data, params)
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    command_data:updateStatus()
    command_data:updatePorts()

    return command_data:getCommandExecutionResults()
end

local function lookup_mode_set(command_data, devNum, portNum, targDev)
    local result, values

    if (is_sip_5(devNum)) or (DeviceFamily == "CPSS_PP_FAMILY_DXCH_LION2_E") then
	    result, values = myGenWrapper("cpssDxChCscdDevMapLookupModeSet",
	        {{ "IN", "GT_U8",    "dev",  devNum   },
	        { "IN", "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT", "mode", "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E"}})
	    if 0 ~= result then
	        command_data:setFailPortAndLocalStatus()
	        command_data:addError("Error at device map lookup mode setting " ..
	                              "on device %d port %d: %s",
	                              devNum, portNum, to_string(returnCodes[result]))
	    end
    end

    result, values = myGenWrapper("cpssDxChCscdPortTypeGet",
        {{ "IN", "GT_U8",    "dev",  devNum},
         { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
         { "IN", "CPSS_PORT_DIRECTION_ENT", "portDirection", "CPSS_PORT_DIRECTION_BOTH_E"},
         { "OUT", "CPSS_CSCD_PORT_TYPE_ENT", "portType" }})

    if 0 ~= result then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error at cscd port type getting "..
                              "on device %d port %d: %s",
                              devNum, portNum, to_string(returnCodes[result]))
    end

    if values.portType == "CPSS_CSCD_PORT_NETWORK_E" then

	    result, values = myGenWrapper("cpssDxChCscdPortTypeSet",
	        {{ "IN", "GT_U8",    "dev",  devNum},
	         { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
	         { "IN", "CPSS_PORT_DIRECTION_ENT", "portDirection", "CPSS_PORT_DIRECTION_BOTH_E"},
	         { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType", "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E"}})

	    if 0 ~= result then
	        command_data:setFailPortAndLocalStatus()
	        command_data:addError("Error at cscd port type setting "..
	                              "on device %d port %d: %s",
	                              devNum, portNum, to_string(returnCodes[result]))
	    end

    end

    local cascadeLink = {}

    cascadeLink["linkNum"] = portNum
    --cascadeLink["linkType"] = "CPSS_CSCD_LINK_TYPE_TRUNK_E"
    cascadeLink["linkType"] = "CPSS_CSCD_LINK_TYPE_PORT_E"

    local hw_targDevNum, hw_srcDevNum
    local result1, hw_scrcDevNum    =  device_to_hardware_format_convert(devNum);

    result, values = myGenWrapper("cpssDxChCscdDevMapTableSet",
         {{ "IN", "GT_U8",    "dev",  devNum},
           {"IN", "GT_HW_DEV_NUM", "targDev", targDev},
           {"IN", "GT_HW_DEV_NUM", "srcDev", hw_scrcDevNum},
           {"IN", "GT_PORT_NUM", "portNum", portNum},
           {"IN", "CPSS_CSCD_LINK_TYPE_STC", "cascadeLink", cascadeLink},
           {"IN", "CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT", "srcPortTrunkHashEn", "CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E"},
           {"IN", "GT_BOOL", "egressAttributesLocallyEn", false}})

    if 0 ~= result then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error at cscd device map table setting "..
                              "on device %d port %d: %s",
                              devNum, portNum, to_string(returnCodes[result]))
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    --command_data:updateStatus()
    command_data:updatePorts()

end

function cascade_device_ether(command_data, params)
    lookup_mode_set(command_data, params.devID, params.ether, tonumber(params.targDev))
end

function cascade_device_chan_gr(command_data, params)
    local result, values
    local devNum, portNum
    local port_channel = params.chan

    devNum = params.devID

    local trunk_type = trunk_db_type_get(command_data, devNum, port_channel)
    if trunk_type ~= "CPSS_TRUNK_TYPE_CASCADE_E" then
        if not change_trunk_type_to_cascade(command_data,port_channel) then
            command_data:analyzeCommandExecution()

            command_data:printCommandExecutionResults()

            return command_data:getCommandExecutionResults()
        end
    end

    result, values = myGenWrapper("cpssDxChTrunkCascadeTrunkPortsGet", {
                    { "IN", "GT_U8",    "dev",  devNum   },
                    { "IN", "GT_TRUNK_ID",    "trunkId", port_channel  },
                    { "OUT", "CPSS_PORTS_BMP_STC",  "portsMembers"}})

    if 0 ~= result then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error at cascade trunk ports getting " ..
                                  "on device %d: %s",
                                  devNum, values)
    end

    local portsMembers = values.portsMembers
    for portNum = 0, 255 do
        if wrLogWrapper("wrlCpssIsPortSetInPortsBmp","(portsMembers, portNum)", portsMembers, portNum) then
            lookup_mode_set(command_data, devNum, portNum, tonumber(params.targDev))
        end
    end
end

--------------------------------------------------------------------------------
-- command registration: cascade dsa
--------------------------------------------------------------------------------
CLI_addHelp("interface",    "cascade device",  "")
CLI_addCommand("interface", "cascade device", {
  func   = cascade_device_func,
  help   = "Configure device map table",
  params = {
      { type = "values",
        { format="%devID", name="devID", help="The device number"},
        { format="target_device", name="target_device_word", help="target_device" },
        { format="%integer", name="targDev", help="Target hardware device number" },
		{ format="interface", name="interface_word", help="interface" }
      },
      { type = "named",
        --{ format="device %devID", name="devID", help="The device number" },
        --{ format="target_device %devID", name="targDev", help="Target device number" },
        { format="ethernet %portNum", name="ether", help="Interface" },
        { format="chanel-group %portNum", name="chan", help="Interface"},
        alt={iface={"ether","chan"}},
        mandatory = {"iface"}
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: cascade dsa-format
--------------------------------------------------------------------------------
CLI_addCommand("interface", "cascade dsa-format hash", {
  func   = cascadeDsaFormat,
  help   = "enable using of Hash inside eDSA FORWARD tag",
  params = {
      { type = "named",
          "#rx_tx_both"
      },
      mandatory = {"hash"}
  }
})

CLI_addCommand("interface", "no cascade dsa-format hash", {
  func   = function(params)
      params.flagNo = true
      return cascadeDsaFormat(params)
  end,
  help   = "disable using of Hash inside eDSA FORWARD tag",
  params = {
      { type = "named",
          "#rx_tx_both"
      },
      mandatory = {"hash"}
  }
})
