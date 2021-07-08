--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_micro_init.lua
--*
--* DESCRIPTION:
--*       lua CLI micro init comands implementation
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("luaCLI_getTraceDBLine")
cmdLuaCLI_registerCfunction("wrlCpssPxPortPhysicalPortMapSet")

local microInitFileName = nil
local port_map = {}
--[[--wait SMI implementation 
local smiInit = false
local port_smi = {}
]]--wait SMI implementation
local port_pizza = {}
-- enable trace
local function enable_microInit_tracing()

    local ret,devices
    
    ret = myGenWrapper("appDemoTraceHwAccessOutputModeSet",{{"IN",TYPE["ENUM"],"mode",2}})  --output to db
    if ret ~= 0 then
        print("appDemoTraceHwAccessOutputModeSet failed, ret="..to_string(ret))
    end

    devices=wrLogWrapper("wrlDevList")

    for j=1,#devices do
        ret = myGenWrapper("appDemoTraceHwAccessEnable",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN",TYPE["ENUM"],"accessType",1},  -- 0 -read, 1--write 2-both 3-delay&write 4-all
            {"IN","GT_BOOL","enable",true}
        })
        if (ret~=0) then
            print("Could not perform trace action, status is:"..ret)
        end
    end
end

-- disable trace
local function disable_microInit_tracing()

    local ret,devices

    devices=wrLogWrapper("wrlDevList")
    
    for j=1,#devices do
        ret = myGenWrapper("appDemoTraceHwAccessEnable",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN",TYPE["ENUM"],"accessType",1},  -- 0 -read, 1--write 2-both 3-delay&write 4-all
            {"IN","GT_BOOL","enable",false}
        })
        if (ret~=0) then
            print("Could not perform trace action, status is:"..ret)
        end
    end
end

local function traceClear()
    
    local ret
    
    ret = myGenWrapper("appDemoTraceHwAccessClearDb",{})
    if (ret~=0) then
        print("Could clear HW access db, status is:"..ret)
        return false
    else
        return true
    end
end

-- saves configuration in micro-init file
local function saveConfig(filename)

    local save_tracing=[[
    do debug
    trace save file ]]..filename..[[ as micro-init saveToHd
    exit
    ]]

    if (luaCLI_getTraceDBLine(0)==nil) then
        print("No trace is stored")
        return false
    end
    executeStringCliCommands(save_tracing)
    traceClear()
    return true
end

CLI_prompts["micro-init"] = "(micro-init)#"
CLI_addCommand("exec", "micro-init", {
    func = function(params)
        microInitFileName = params.filename
        CLI_change_mode_push("micro-init")
        traceClear()
    end,
    help="Switch to micro-init mode",
    params = {
        {
            type = "named",
            { format = "file %filename", name = "filename", help = "configuration file for micro-init" },
            mandatory = { "filename" }
        }
    }
})

-----------------------device-config-------------------------------------------------

-- get microinit configuration sequence
local function deviceConfig(params)

    local e, key, val, features, ret=0xffffffff
    local microInitParamsPtr = {}

    features=0
    if params["features_list"] ~= nil then
        for key, val in pairs(params["features_list"]) do
            features = bit_or(features, val)
        end
    end

    microInitParamsPtr.deviceId = params["deviceId"]
    microInitParamsPtr.mngInterfaceType = params["mngInterfaceType"]
    microInitParamsPtr.coreClock = params["coreClock"]
    microInitParamsPtr.featuresBitmap = features
    
    enable_microInit_tracing()
    
    ret, val = myGenWrapper("cpssMicroInitSequenceCreate", {
        { "IN", "APP_DEMO_MICRO_INIT_STC", "microInitParamsPtr", microInitParamsPtr }
    })

    disable_microInit_tracing()

    -- save configuration to file
    if ret == 0 then
		saveConfig(microInitFileName.."-device0.txt")
    else
        print("cpssMicroInitSequenceCreate failed, ret="..to_string(ret))
        return false
    end
    enable_microInit_tracing()

    ret, val = myGenWrapper("cpssMicroInitSequenceCreate", {
        { "IN", "APP_DEMO_MICRO_INIT_STC", "microInitParamsPtr", microInitParamsPtr }
    })

    disable_microInit_tracing()

    if ret == 0 then
        saveConfig(microInitFileName.."-device1.txt")
    else 
        print("cpssMicroInitSequenceCreate failed, ret="..to_string(ret))
        return false
    end

    return true
end

--------------------------------------------
-- type registration: deviceId
--------------------------------------------
CLI_type_dict["deviceId"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "The device id should be selected",
    enum = {
        -- PIPE devices
        ["CPSS_98PX1008_CNS"]   = { value=0xC40111AB,  help="PIPE device" },
        ["CPSS_98PX1012_CNS"]   = { value=0xC40011AB,  help="PIPE device" },
        ["CPSS_98PX1022_CNS"]   = { value=0xC40311AB,  help="PIPE device" },
        ["CPSS_98PX1024_CNS"]   = { value=0xC40211AB,  help="PIPE device\n" },
    }
}
--------------------------------------------
-- type registration: mngInterfaceType
--------------------------------------------
CLI_type_dict["mngInterfaceType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "The interface channel to CPU  to be selected",
    enum = {
        ["CPSS_CHANNEL_PCI_E"]      = { value=0,    help="" },
        ["CPSS_CHANNEL_SMI_E"]      = { value=1,    help="" },
        ["CPSS_CHANNEL_TWSI_E"]     = { value=2,    help="" },
        ["CPSS_CHANNEL_PEX_E"]      = { value=3,    help="" },
        ["CPSS_CHANNEL_PEX_MBUS_E"] = { value=4,    help="" }
    }
}

--------------------------------------------
-- type registration: coreClock
--------------------------------------------
CLI_type_dict["coreClock"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "The interface channel to CPU  to be selected",
    enum = {
        ["500Mhz"]  = { value=500,  help="500Mhz" },
        ["450Mhz"]  = { value=450,  help="450Mhz" },
        ["288Mhz"]  = { value=288,  help="288Mhz" },
        ["350Mhz"]  = { value=350,  help="350Mhz" },
        ["531Mhz"]  = { value=531,  help="531Mhz" },
        ["550Mhz"]  = { value=550,  help="550Mhz" },
        ["575Mhz"]  = { value=575,  help="575Mhz" },
    }
}

local function CLI_check_param_feature_list(str, name, desc, varray, params)
    desc = CLI_type_dict["features-list-regular"]
    return CLI_check_param_enum(str, name, desc, varray, params)
end

local function CLI_complete_param_feature_list(str, name, desc, varray, params)
    desc = CLI_type_dict["features-list-regular"]
    return CLI_complete_param_enum(str, name, desc, varray, params)
end
--------------------------------------------
-- type registration: features-list
--------------------------------------------
CLI_type_dict["features-list"] = {
    checker = CLI_check_param_feature_list,
    complete = CLI_complete_param_feature_list,
    help = "The features list",
}
CLI_type_dict["features-list-regular"] = {
    help = "The features list",
    enum = {
        ["LED_POSITION_JUST_ON_USED_MAC"]   = { value=0x10,  help="All not used mac's LED port number shall be set to 0x3F" },
        ["RM_MG_TERMINATOR_BYPASS_ENABLE"]  = { value=0x20,  help="Read access to CG MAC registers is masked by a MG terminator"},
        ["RM_TXQ_QCN_GLOBAL_MISS_CONFIG"]   = { value=0x40,  help="QCN does not drop CN packets when any of the following clients are not ready" },
        ["RM_AVS_VDD_LIMITS"]               = { value=0x80,  help="Change AVS set point from its default value" }
    }
}

--------------------------------------------------------------------------------
-- command registration: device-config
--------------------------------------------------------------------------------
CLI_addCommand("micro-init", "device-config", {
    func   = deviceConfig,
    help   = "Generate micro-init basic configurations",
    params = {
        {
            type = "named",
            { format = "deviceId %deviceId", name = "deviceId", help = "device Id describing device family" },
            { format = "mngInterfaceType %mngInterfaceType", name = "mngInterfaceType", help = "interface channel to CPU" },
            { format = "coreClock %coreClock", name = "coreClock", help = "core clock" },
            { format = "features-list @features-list", name = "features_list", help = "features used to generate code for init sequence" },
            requirements = {
                ["mngInterfaceType"] = { "deviceId" },
                ["coreClock"] = { "mngInterfaceType" },
                ["features-list"] = { "coreClock" }
            },
            mandatory = { "deviceId",  
                          "mngInterfaceType", 
                          "coreClock"}
        }
    }
})

-----------------------port-map-------------------------------------------------

-- micro-init: port-map
CLI_prompts["micro-init-port-map"] = "(port-map)#"
CLI_addCommand("micro-init", "port-map", {
    func = function()
        port_map = {}
        CLI_change_mode_push("micro-init-port-map")
    end,
    help = "Generate port-mapping configurations"
})

local function port_map_apply()
    local devId, portMapArray, val, ret=0xffffffff

    enable_microInit_tracing()

    -- iterate thru all devices(if not only 0)
    for devId, portMapArray in pairs(port_map) do
      print("Applying mapping for device #"..tostring(devId))
      print("portMapArray=" .. to_string(portMapArray))
      local portMapArrayLen_ = #portMapArray
      ret= wrLogWrapper("wrlCpssPxPortPhysicalPortMapSet", "(devId, portMapArrayLen_, portMapArray)", devId, portMapArrayLen_, portMapArray)
      if ret ~= 0 then
        print("  wrlCpssPxPortPhysicalPortMapSet failed, ret="..to_string(ret))
        break
      end
        ret, val = myGenWrapper("cpssMicroInitBasicForwardingTablesSet", {
            { "IN", "GT_SW_DEV_NUM",                "devNum",   devId }
        })
        if ret ~= 0 then
            print("cpssMicroInitBasicForwardingTablesSet failed, ret="..to_string(ret))
            break
        end
    end

    disable_microInit_tracing()

    -- save configuration to file
    if ret == 0 then
        saveConfig(microInitFileName.."-map.txt")
    end

    CLI_change_mode_pop()
end

-- %port_maping_type
local function CLI_check_param_port_mapping_type(param,name,desc,varray,params)
    local devId       -- device number
    local valid, val = CLI_check_param_enum(param,name,desc)
    if not valid then
        return valid, val
    end

    if (params["port-range"] ~= nil) then
        devId = next(params["port-range"])
    else
        devId=params.dev_port_mic.devId
    end

    local range = nil
    if val == "CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E" then
        range = 15
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
        ["ethernet_mac"] = { value="CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E", help="Ethernet MAC"},
        ["cpu_sdma"]     = { value="CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E",     help="CPU SDMA"}
    }
}

local function port_map_ethernet(params)
    
    -- create record according to input params 
    local port_record = {}

    local devId=params.dev_port_mic.devId       -- device number

    -- fill CPSS_PX_PORT_MAP_STC record
    port_record = {
        physicalPortNumber=params.dev_port_mic.portNum,
        mappingType=params.port_mapping_type,
        interfaceNum=params["interface-number"]
    }

    -- for new device --> create new array of records
    if port_map[devId] == nil then
        port_map[devId] = {}
    end

    -- add record to port_map structure
    table.insert(port_map[devId], port_record)
end

-- port-map: list
local function port_map_list()
    --TODO
    print(to_string(port_map))
end

-- port-map: list
CLI_addCommand("micro-init-port-map", "list", {
    func = port_map_list,
    help = "Show mapping (not commited)"
})

-- port-map: exit
CLI_addCommand("micro-init-port-map", "exit", {
    func = port_map_apply,
    help = "Apply configured mapping"
})

-- port-map: abort
CLI_addCommand("micro-init-port-map", "abort", {
    func = CLI_change_mode_pop,
    help = "Exit without applying (drop mapping)"
})

-- port-map: ethernet %dev_port_mic type %port_mapping_type txq-port %port_map_txq_portnum [interface-number %port_map_interface_number] [tm %port_map_tm_index]
CLI_addCommand("micro-init-port-map", "ethernet", {
    func = port_map_ethernet,
    help = "Configure mapping for one port",
    params={
        { type="values",
            "%dev_port_mic",
            "type", "%port_mapping_type"
        },
        { type="named",
            { format="interface-number %port_map_interface_number", help="interface-number" }
        }
    }
})

local function port_map_range(params)
    local start_interface_number
    local i
    if (params["interface-number"] ~= nil) then
        start_interface_number = params["interface-number"] 
    end
    for dev,portTable in pairs(params["port-range"]) do
        i = 0
        for port=1,#portTable do
            params.dev_port_mic = { devId = dev, portNum = portTable[port] }
            if (params["interface-number"] ~= nil) then  
                params["interface-number"] = portTable[port] 
            end
            port_map_ethernet(params)
            i = i + 1
        end
    end
end

-- port-map: range
CLI_addCommand("micro-init-port-map", "range", {
    func = port_map_range,
    help = "Configure range of ports",
    params={
        { type="values",
            "%port-range",
            "type", "%port_mapping_type"
        },
        { type="named",
            { format="interface-number %port_map_interface_number", help="interface-number" }
        }
    }
})

--[[--wait SMI implementation 
    -----------------------port-smi-------------------------------------------------
    -- micro-init: port-smi
    CLI_prompts["port-smi"] = "(port-smi)#"
    CLI_addCommand("micro-init", "port-smi", {
        func = function()
            port_smi = {}
            CLI_change_mode_push("port-smi")
        end,
        help = "Generate board auto-pulling configurations and apply errata initialization fixes"
    })

    -- port-map: list
    local function port_smi_list()
        --TODO
        print(to_string(port_smi))
    end

    -- port-smi: list
    CLI_addCommand("port-smi", "list", {
        func = port_smi_list,
        help = "Show port smi cnfg (not commited)"
    })

    -- port-smi: abort
    CLI_addCommand("port-smi", "abort", {
        func = CLI_change_mode_pop,
        help = "Exit without applying (drop smi cnfg)"
    })

    local function port_smi_apply(params)
        local val, ret=0xffffffff
        local devFamily, subFamily, devType

        enable_microInit_tracing()

        -- iterate thru all devices
        for devId, portSmiArray in pairs(port_smi) do
            print("Applying smi configuration for device #"..tostring(devId))
            devFamily, subFamily = wrlCpssDeviceFamilyGet(devId)
            devType = wrlCpssDeviceTypeGet(devId)
            -- iterate thru all entries
            for _, entry in pairs(portSmiArray) do
                print("portSmiArray=" .. to_string(entry))
                if (("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) and ("CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" == subFamily)) then
                    if (math.floor(devType / 0x10000) == CAELUM_DEV_ID) then

                        local autoPollNumber = {}
                        autoPollNumber[0] = "CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E"
                        autoPollNumber[1] = "CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E"
                        autoPollNumber[2] = "CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E"
                        autoPollNumber[3] = "CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_12_E"

                        ret, val = myGenWrapper("cpssDxChPhyAutoPollNumOfPortsSet", {
                            { "IN", "GT_U8", "devNum", devId },
                            { "IN", "CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT", "autoPollNumOfPortsSmi0", autoPollNumber[0] },
                            { "IN", "CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT", "autoPollNumOfPortsSmi1", autoPollNumber[1] },
                            { "IN", "CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT", "autoPollNumOfPortsSmi2", autoPollNumber[2] },
                            { "IN", "CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT", "autoPollNumOfPortsSmi3", autoPollNumber[3] }
                        })
                        if ret ~= 0 then
                            print("cpssDxChPhyAutoPollNumOfPortsSet failed, ret="..to_string(ret))
                            break
                        end
                    end
                end

                ret, val = myGenWrapper("cpssDxChPhyPortAddrSet", {
                    { "IN", "GT_U8", "devNum", tonumber(devId) },
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", tonumber(entry.poolPortNumber) },
                    { "IN", "GT_U8", "phyAddr", tonumber(entry.phy_addr) }
                })
                if ret ~= 0 then
                    print("cpssDxChPhyPortAddrSet failed, ret="..to_string(ret))
                    break
                end

                ret, val = myGenWrapper("cpssDxChPhyPortSmiInterfaceSet", {
                    { "IN", "GT_U8", "devNum", devId },
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", entry.poolPortNumber },
                    { "IN", "CPSS_PHY_SMI_INTERFACE_ENT", "smiInterface", entry.smi_interface }
                })
                if ret ~= 0 then
                    print("cpssDxChPhyPortSmiInterfaceSet failed, ret="..to_string(ret))
                    break
                end

                if entry.inbanAutNegEnable == "enable" then
                    enable = true
                else
                    enable = false
                end
                ret, val = myGenWrapper("cpssPxPortInbandAutoNegEnableSet", {
                    { "IN", "GT_U8", "devNum", devId },
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", entry.poolPortNumber },
                    { "IN", "GT_BOOL", "enable", enable }
                })
                if ret ~= 0 then
                    print("cpssPxPortInbandAutoNegEnableSet failed, ret="..to_string(ret))
                    break
                end
            end
        end

            if smiInit ~= false then
                ret, val = myGenWrapper("cpssDxChPhyPortSmiInit", {
                    { "IN", "GT_U8", "devNum", devId }
                })
                if ret ~= 0 then
                    print("cpssDxChPhyPortSmiInit failed, ret="..to_string(ret))
                end
                smiInit = false
            end

        disable_microInit_tracing()

        -- save configuration to file
        if ret == 0 then
            saveConfig(microInitFileName.."-smi.txt")
        end

        CLI_change_mode_pop()
    end

    local function port_smi_config(params)
        
        -- create record according to input params 
        local port_record = {}

        local devId=params.dev_port_mic.devId       -- device number

        port_record = {
            poolPortNumber=params.dev_port_mic.portNum,
            inbanAutNegEnable=params.inband_auto_neg,
            smiInterface = params.smi_interface,
            phyAddr=params.phy_addr
        }

        -- for new device --> create new array of records
        if port_smi[devId] == nil then
            port_smi[devId] = {}
        end

        -- add record to port_smi structure
        table.insert(port_smi[devId], port_record)

    end
    -- port-smi: phy-port-smi-init
    CLI_addCommand("port-smi", "phy-port-smi-init", {
        func = function()
        smiInit = true
        end,
        help = "SMI init. Must be called in order to config phys"
    })

    -- port-smi: exit
    CLI_addCommand("port-smi", "exit", {
        func = port_smi_apply,
        help = "Apply all port smi configurations"
    })

    --------------------------------------------
    -- type registration: inband-auto-neg-enable
    --------------------------------------------
    CLI_type_dict["inband-auto-neg-enable"] = {
        checker = CLI_check_param_enum,
        complete = CLI_complete_param_enum,
        help = "",
        enum = {
            ["true"]    = { value="enable",  help="inband auto negotiation enable" },
            ["false"]    = { value="disable",  help="inband auto negotiation disable" }
        }
    }

    --------------------------------------------
    -- type registration: smi-interface
    --------------------------------------------
    CLI_type_dict["smi-interface"] = {
        checker = CLI_check_param_enum,
        complete = CLI_complete_param_enum,
        help="Choose smi interface:",
        enum={
            ["CPSS_PHY_SMI_INTERFACE_0_E"] = { value="CPSS_PHY_SMI_INTERFACE_0_E", help=""},
            ["CPSS_PHY_SMI_INTERFACE_1_E"] = { value="CPSS_PHY_SMI_INTERFACE_1_E", help=""},
            ["CPSS_PHY_SMI_INTERFACE_2_E"] = { value="CPSS_PHY_SMI_INTERFACE_2_E", help=""},
            ["CPSS_PHY_SMI_INTERFACE_3_E"] = { value="CPSS_PHY_SMI_INTERFACE_3_E", help=""}
        }
    }

    --------------------------------------------
    -- type registration: phy-addr
    --------------------------------------------
    CLI_type_dict["phy-addr"] = {
        checker = CLI_check_param_number,
        min = 0,
        max = 31,
        help = "Set phy-addr : Applicable ranges : 0-31"
    }

    -- port-smi: auto-poll-cfg
    CLI_addCommand("port-smi", "auto-poll-cfg", {
        func = port_smi_config,
        help = "Configure smi auto-polling. Must be called before phy-port-smi-init",
        params={
            { type = "values",
                "%dev_port_mic"
            },
            { type = "named",
                { format="inband-auto-neg-enable %inband-auto-neg-enable", name="inband_auto_neg", help="enable/disable inband auto negotiation" },
                { format="smi-interface %smi-interface", name="smi_interface", help="smi interface id" },
                { format="phy-addr %phy-addr", name="phy_addr", help="phy port address" },
                mandatory = { "inband_auto_neg","smi_interface", "phy_addr" }
            }
        }
    })

    local function port_smi_range(params)
        for dev,portTable in pairs(params["port-range"]) do
            for port=1,#portTable do
                params.dev_port_mic = { devId = dev, portNum = portTable[port] }
                port_smi_config(params)
            end
        end
    end

    -- port-map: range
    CLI_addCommand("port-smi", "range", {
        func = port_smi_range,
        help = "Configure range of ports",
        params={
            { type="values",
                "%port-range"
            },
            { type = "named",
                { format="inband-auto-neg-enable %inband-auto-neg-enable", name="inband_auto_neg", help="enable/disable inband auto negotiation" },
                { format="smi-interface %smi-interface", name="smi_interface", help="smi interface id" },
                { format="phy-addr %phy-addr", name="phy_addr", help="phy port address" },
                mandatory = { "inband_auto_neg","smi_interface", "phy_addr" }
            }
        }
    })
]]--wait SMI implementation
--------------------port-pizza-resources----------------------------------------

-- micro-init: port-pizza-resources
CLI_prompts["port-pizza-resources"] = "(port-pizza-resources)#"
CLI_addCommand("micro-init", "port-pizza-resources", {
    func = function()
        port_pizza = {}
        CLI_change_mode_push("port-pizza-resources")
    end,
    help = "Generate pizza and port resources configurations for port"
})

-- port-map: list
local function port_pizza_list()
    --TODO
    print(to_string(port_pizza))
end

-- port-pizza-resources: list
CLI_addCommand("port-pizza-resources", "list", {
    func = port_pizza_list,
    help = "Show port configurations (not commited)"
})

local function port_pizza_apply(params)
    local val, ret=0xffffffff

    -- iterate thru all devices
    for devId, portPizzaArray in pairs(port_pizza) do
        print("Applying pizza configuration and port resources for device #"..tostring(devId))
        -- iterate thru all entries
        local count = 0
        for _, entry in pairs(portPizzaArray) do
            count = count + 1
            print("portPizzaArray=" .. to_string(entry))

                if (count == #portPizzaArray) then
                    enable_microInit_tracing()
                end
                ret, val = myGenWrapper("prvCpssPxPortPizzaArbiterIfConfigure", {
                    { "IN", "GT_U8",                        "devNum",   devId },
                    { "IN", "GT_PHYSICAL_PORT_NUM",         "portNum",  entry.physicalPortNumber },
                    { "IN", "CPSS_PORT_SPEED_ENT",          "speed",    entry.speed }
                })
                if ret ~= 0 then
                    print("prvCpssPxPortPizzaArbiterIfConfigure failed, ret="..to_string(ret))
                    break
                end
                if (count == #portPizzaArray) then
                    disable_microInit_tracing()
                end

                enable_microInit_tracing()
                ret, val = myGenWrapper("prvCpssPxPortResourcesConfig", {
                    { "IN", "GT_SW_DEV_NUM",                "devNum",   devId },
                    { "IN", "GT_PHYSICAL_PORT_NUM",         "portNum",  entry.physicalPortNumber },
                    { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode",   entry.ifmode },
                    { "IN", "CPSS_PORT_SPEED_ENT",          "speed",    entry.speed }
                })
                if ret ~= 0 then
                    print("prvCpssPxPortResourcesConfig failed, ret="..to_string(ret))
                    break
                end
                disable_microInit_tracing()
        end
        for _, entry in pairs(portPizzaArray) do
                enable_microInit_tracing()
                ret, val = myGenWrapper("cpssPxPortEnableSet", {
                    { "IN", "GT_SW_DEV_NUM",                "devNum",   devId },
                    { "IN", "GT_PHYSICAL_PORT_NUM",         "portNum",  entry.physicalPortNumber },
                    { "IN", "GT_BOOL",                      "enable",   true }
                })
                if ret ~= 0 then
                    print("cpssPxPortEnableSet failed, ret="..to_string(ret))
                    break
                end
                disable_microInit_tracing()
        end
    end

    -- save configuration to file
    if ret == 0 then
        saveConfig(microInitFileName.."-pizza.txt")
    end

    CLI_change_mode_pop()
end

-- port-pizza-resources: exit
CLI_addCommand("port-pizza-resources", "exit", {
    func = port_pizza_apply,
    help = "Apply port configurations"
})

-- port-map: list
local function port_pizza_abort()
    port_pizza = {}
    CLI_change_mode_pop()
end

-- port-pizza-resources: abort
CLI_addCommand("port-pizza-resources", "abort", {
    func = port_pizza_abort,
    help = "Exit without applying (drop port resources cnfg)"
})

local function port_pizza_ethernet(params)
    --print(to_string(params))
    -- create record according to input params
    local port_record = {}

    local devId=params.dev_port_mic.devId       -- device number

    -- fill record
    port_record = {
        physicalPortNumber=params.dev_port_mic.portNum,
        ifmode = params.port_interface_mode,
        speed = params.interface_speed,
    }

    -- for new device --> create new array of records
    if port_pizza[devId] == nil then
        port_pizza[devId] = {}
    end

    -- add record to port_map structure
    table.insert(port_pizza[devId], port_record)
end

-- port-pizza-resources: ethernet
CLI_addCommand("port-pizza-resources", "ethernet", {
    func = port_pizza_ethernet,
    help = "Configure resources for one port",
    params={
        { type="values",
            "%dev_port_mic",
            --"ifMode", "%port_interface_mode",
            --"speed", "%port_speed",
            "ifMode", "%port_interface_mode",
            "speed", "%interface_speed",
        },
    }
})

local function port_pizza_range(params)
    for dev,portTable in pairs(params["port-range"]) do
        for port=1,#portTable do
            params.dev_port_mic = { devId = dev, portNum = portTable[port] }
            port_pizza_ethernet(params)
        end
    end
end

-- port-pizza-resources: range
CLI_addCommand("port-pizza-resources", "range", {
    func = port_pizza_range,
    help = "Configure range of ports",
    params={
        { type="values",
            "%port-range",
            --"ifMode", "%port_interface_mode",
            --"speed", "%port_speed",
            "ifMode", "%port_interface_mode",
            "speed", "%interface_speed",
        },
    }
})
