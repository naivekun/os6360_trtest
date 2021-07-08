--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_map.lua
--*
--* DESCRIPTION:
--*       Implement port-mapping features
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

do

cmdLuaCLI_registerCfunction("wrlCpssDxChPortPhysicalPortMapSet");

--
-- port_map[devId][portNum] = {
--      mtype     = port_mapping_type,
--      interface = interface_number,
--      txq       = txq_portnum,
--      tmenable  = true/false,
--      tmPortInd = tmPortInd
-- }
--
local port_map = {}

-- config: port-map
CLI_prompts["port-map"] = "(port-map)#"
CLI_addCommand("config", "port-map", {
    func = function()
        port_map = {}
        CLI_change_mode_push("port-map")
    end,
    help = "Enter port-mapping mode"
})


local function port_map_apply()
    local devId, portMapArray, ret

    -- iterate thru all devices(if not only 0)
    for devId, portMapArray in pairs(port_map) do
      print("Applyng mapping for device #"..tostring(devId))
      print("portMapArray=" .. to_string(portMapArray))
      local portMapArrayLen_ = #portMapArray
      ret= wrLogWrapper("wrlCpssDxChPortPhysicalPortMapSet", "(devId, portMapArrayLen_, portMapArray)", devId, portMapArrayLen_, portMapArray)
      if ret ~= 0 then
        print("  => failed, ret="..to_string(ret))
      end
    end
    CLI_change_mode_pop()
end


-- port-map: exit
CLI_addCommand("port-map", "exit", {
    func = port_map_apply,
    help = "Apply configured mapping"
})
-- port-map: abort
CLI_addCommand("port-map", "abort", {
    func = CLI_change_mode_pop,
    help = "Exit without applying (drop mapping)"
})

-- port-map: list
local function port_map_list()
    --TODO
    print(to_string(port_map))
end
CLI_addCommand("port-map", "list", {
    func = port_map_list,
    help = "Show mapping (not commited)"
})

-- %port_maping_type
local function CLI_check_param_port_mapping_type(param,name,desc,varray,params)
    local valid, val = CLI_check_param_enum(param,name,desc)
    if not valid then
        return valid, val
    end
    if not is_sip_5(nil) then
        -- check for params.dev_port.devId
        -- tm applicable for Bobcat2 only
        params.tm = false
    end
    local range = nil
    if val == "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" or
       val == "CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E" then
        range = 31
        if is_sip_5(nil) then
            range = 71
        end
    end
    if val == "CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E" then
        range = 61
    end
    if range ~= nil then
        CLI_type_dict["port_map_interface_number"] = {
            checker = CLI_check_param_number,
            min = 0,
            max = range,
            help = "interface number (0.."..tostring(range)..")"
        }
    else
        -- not appicable, set default value
        params["interface-number"] = 0xffffffff
    end
    return valid, val
end
CLI_type_dict["port_mapping_type"] = {
    checker=CLI_check_param_port_mapping_type,
    complete=CLI_complete_param_enum,
    help="Port mapping type",
    enum={
        ["ethernet_mac"] = { value="CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E",
                    help="Ethernet MAC"},
        ["cpu_sdma"]     = { value="CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E",
                    help="CPU SDMA"},
        ["ilkn_mac"]     = { value="CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E",
                    help="Interlaken Channel (all channels 0-63)"},
        ["remote_port"]  = { value="CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E",
                    help="Port represent physical interface on remote device or Interlaken channel"}
    }
}

-- for BC3 txQues amount - 96 * 6 = 576

-- ************************************************************************
---
--  CLI_check_param_number
--        @description  function to check port_map_txq_portnum_ext parameter
--
--        @param param          - parameter string
--        @param name           - parameter name
--        @param desc           - type description reference
--
--        @return       true, value
--                      false, error_string
--
local function port_map_txq_portnum_ext_checker(param, name, desc)
    local res, val = CLI_check_param_number(param, name, desc)
    if not res then return res, val; end
    local devNum, val_max;
    local dev_range = wrLogWrapper("wrlDevList");
    local devNum = dev_range[1];
    local devFamily , subFamily = wrlCpssDeviceFamilyGet(devNum);
    local val_max = 71;
    if (devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT3_E") then val_max = 575; end;
    if (devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E") then val_max =  99; end;
    -- CPSS_TBD_BOOKMRK_FALCON -- currently Falcon as BC3 
    if (devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E")  then val_max = 575; end;
    if val <= val_max then return true, val; end;              -- correct
    return false, ("value more then " ..  tostring(val_max)); -- out of range
end

CLI_type_dict["port_map_txq_portnum_ext"] = {
    checker = port_map_txq_portnum_ext_checker,
    complete = CLI_complete_param_number,
    help = "TXQ port number"
}

local function port_map_ethernet(params)

    -- create record according to input params
    local port_record = {}

    local devId=params.dev_port.devId       -- device number

    local tmenable = true
    local tm = params.tm
    if not tm then
        tmenable = false
        tm = 0
    end

    -- fill CPSS_DXCH_PORT_MAP_STC record
    port_record = {
        physicalPortNumber=params.dev_port.portNum,
        mappingType=params.port_mapping_type,
        portGroup = 0xffffffff,
        interfaceNum=params["interface-number"],
        txqPortNumber=params.port_map_txq_portnum_ext,
        tmEnable=tmenable,
        tmPortInd=tm
    }

    -- for new device --> create new array of records
    if port_map[devId] == nil then
        port_map[devId] = {}
    end

    -- add record to port_map structure
    table.insert(port_map[devId], port_record)
end

local function CLI_check_param_tm_index(param)
    local val = tonumber(param)
    if val == nil then
        return false, "invalid tm index"
    end
    if val >= 0 and val <= 71 then
        return true, val
    end
    if val >= 128 and val <= 191 then
        return true, val
    end
    return false, "invalid value, must be in range 0..71 and 128..191"
end
CLI_type_dict["port_map_tm_index"] = {
    checker = CLI_check_param_tm_index,
    min = 0,
    max = 71,
    help = "TM port index (0..71 and 128..191)"
}

-- port-map: ethernet %dev_port type %port_mapping_type txq-port %port_map_txq_portnum_ext [interface-number %port_map_interface_number] [tm %port_map_tm_index]
CLI_addCommand("port-map", "ethernet", {
    func = port_map_ethernet,
    help = "Configure mapping for one port",
    params={
        { type="values",
            "%dev_port",
            "type", "%port_mapping_type",
            "txq-port", "%port_map_txq_portnum_ext"
        },
        { type="named",
            { format="interface-number %port_map_interface_number", help="interface-number" },
            { format="tm %port_map_tm_index", help="enable tm" }
        },
        mandatory={"interface-number", "tm" }
    }
})

local function show_port_map(params)
    local ret,vals
    local portNum, maxPortNum
    local command_data = Command_Data()
    local function mapping_type(str)
        if not prefix_match("CPSS_DXCH_PORT_MAPPING_TYPE_",str) then
            return str
        end
        str = prefix_del("CPSS_DXCH_PORT_MAPPING_TYPE_", str)
        if string.sub(str,-2) == "_E" then
            str = string.sub(str,1,-3)
        end
        str = string.lower(str)
        local map = {
            remote_physical_port="remote_port",
            ilkn_channel="ilkn_mac"
        }
        if map[str] ~= nil then
            return map[str]
        end
        return str
    end
    local table_info = {
        { head="Port",len=4,align="r",path="physicalPortNumber",type="number" },
        { head="Mapping type",len=12,align="l",path="mappingType",type=mapping_type },
        { head="txq",len=4,align="r",path="txqPortNumber",type="number" },
        { head="ilkn",len=4,align="r",path="interfaceNum",type="number" },
        { head="tm",len=4,align="r",path="tmPortInd",type="number" }
    }

    ret, vals = cpssPerDeviceParamGet("cpssDxChCfgDevInfoGet", params.devID,
                        "info", "CPSS_DXCH_CFG_DEV_INFO_STC")

    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)

    if ret ~= 0 then
        return false
    end
    maxPortNum = vals.info.genDevInfo.maxPortNum

    command_data:clearResultArray()
    command_data:initTable(table_info)

    portNum=0
    for portNum, iterator in command_data:getPortIterator() do

        command_data:clearEntryStatus()
        command_data:clearLocalStatus()
        ret, vals = myGenWrapper("cpssDxChPortPhysicalPortMapGet", {
            {"IN", "GT_U8","devNum",params.devID},
            {"IN", "GT_U32","firstPhysicalPortNumber",portNum},
            {"IN", "GT_U32", "portMapArraySize", 1 },
            {"OUT","CPSS_DXCH_PORT_MAP_STC","portMap"}  })
        command_data:handleCpssErrorDevice(ret,
                "cpssDxChPortPhysicalPortMapGet failed", params.devID)

        if true == command_data["local_status"] then
                if vals.portMap.mappingType ~= "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" and
                    vals.portMap.mappingType ~= "CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E" then
                    vals.portMap.interfaceNum = -1
		end
                if not vals.portMap.tmEnable then
                    vals.portMap.tmPortInd = -1
                end
            command_data:addTableRow(vals.portMap)
            command_data:updateEntries()
        end
    end

    command_data:showTblResult("There is no port mapping to show.")
    return command_data:getCommandExecutionResults()
end

-- exec: show port-map device %devID
CLI_addCommand("exec", "show port-map", {
    func = show_port_map,
    help = "Show port-map",
    params={
        { type="values",
            "device",
            "%devID"
        }
    }
})

end -- do
