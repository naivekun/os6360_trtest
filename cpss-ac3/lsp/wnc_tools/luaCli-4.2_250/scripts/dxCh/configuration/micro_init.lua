--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* micro_init.lua
--*
--* DESCRIPTION:
--*       lua CLI micro init comands implementation
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssCaelumCheck")
cmdLuaCLI_registerCfunction("luaCLI_getTraceDBLine")
cmdLuaCLI_registerCfunction("wrlCpssDxChPortPhysicalPortMapSet")

local microInitFileName = nil
local smiInit = false
local port_map = {}
local port_smi = {}
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
		if ((params.deviceId > 0xc0000000) and (params.deviceId < 0xd0000000)) then
			saveConfig(microInitFileName.."-device0.txt")
		else
			saveConfig(microInitFileName.."-device.txt")
		end

    else 
        print("cpssMicroInitSequenceCreate failed, ret="..to_string(ret))
        return false
    end
	if ((params.deviceId > 0xc0000000) and (params.deviceId < 0xd0000000)) then
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
        -- Caelum/Cygnus
        ["CPSS_98DX3346_CNS"]   = { value=0xBC1611AB,  help="Cygnus device" },
        ["CPSS_98DX3347_CNS"]   = { value=0xBC1711AB,  help="Cygnus device" },
        ["CPSS_98DX4203_CNS"]   = { value=0xBC0011AB,  help="Caelum device" },
        ["CPSS_98DX4204_CNS"]   = { value=0xBC0111AB,  help="Caelum device" },
        ["CPSS_98DX4210_CNS"]   = { value=0xBC1111AB,  help="Cygnus device" },
        ["CPSS_98DX4211_CNS"]   = { value=0xBC1011AB,  help="Cygnus device\n" },
        -- Cetus/Lewis
        ["CPSS_98DX4235_CNS"]   = { value=0xBE0011AB,  help="Cetus device" },
        ["CPSS_98DX8208_CNS"]   = { value=0xBE1111AB,  help="Lewis device" },
        ["CPSS_98DX8212_CNS"]   = { value=0xBE1011AB,  help="Lewis device\n" },
        -- Aldrin
        ["CPSS_98DX8308_CNS"]   = { value=0xC81911AB,  help="Aldrin device" },
        ["CPSS_98DX8312_CNS"]   = { value=0xC81C11AB,  help="Aldrin device" },
        ["CPSS_98DX8314_CNS"]   = { value=0xC80F11AB,  help="Aldrin device" },
        ["CPSS_98DX8315_CNS"]   = { value=0xC80E11AB,  help="Aldrin device" },
        ["CPSS_98DX8316_CNS"]   = { value=0xC81D11AB,  help="Aldrin device" },
        ["CPSS_98DX8324_CNS"]   = { value=0xC81E11AB,  help="Aldrin device" },
        ["CPSS_98DX8332_CNS"]   = { value=0xC81F11AB,  help="Aldrin device" },
        ["CPSS_98DX8332_Z0_CNS"]= { value=0xBC1F11AB,  help="Aldrin device" },
        ["CPSS_98DXZ832_CNS"]   = { value=0xC81811AB,  help="Aldrin device\n" },
        -- xCat3
        ["CPSS_98DX1233_CNS"]   = { value=0xF40811AB,  help="xCat3 device" },
        ["CPSS_98DX1235_CNS"]   = { value=0xF40911AB,  help="xCat3 device" },
            -- Series: AlleyCat3-GE
        ["CPSS_98DX3223_1_CNS"] = { value=0xF40711AB,  help="xCat3 device" },
        ["CPSS_98DX3224_CNS"]   = { value=0xF40611AB,  help="xCat3 device" },
        ["CPSS_98DX3225_CNS"]   = { value=0xF40511AB,  help="xCat3 device" },
        ["CPSS_98DX3226_CNS"]   = { value=0xF40411AB,  help="xCat3 device" },
        ["CPSS_98DX3233_CNS"]   = { value=0xF41311AB,  help="xCat3 device" },
        ["CPSS_98DX3234_CNS"]   = { value=0xF41211AB,  help="xCat3 device" },
        ["CPSS_98DX3235_CNS"]   = { value=0xF41111AB,  help="xCat3 device" },
        ["CPSS_98DX3236_CNS"]   = { value=0xF41011AB,  help="xCat3 device" },
        --["CPSS_98DXH333_CNS"]   = { value=0xF41411AB,  help="xCat3 device" },
        --["CPSS_98DXT323_CNS"]   = { value=0xF41511AB,  help="xCat3 device" },
        --["CPSS_98DXT321_CNS"]   = { value=0xF41611AB,  help="xCat3 device" },
            -- Series: AlleyCat3-Plus
        ["CPSS_98DX3243_CNS"]   = { value=0xF40F11AB,  help="xCat3 device" },
        ["CPSS_98DX3244_CNS"]   = { value=0xF40E11AB,  help="xCat3 device" },
        ["CPSS_98DX3245_CNS"]   = { value=0xF40D11AB,  help="xCat3 device" },
        ["CPSS_98DX3246_CNS"]   = { value=0xF40C11AB,  help="xCat3 device" },
        ["CPSS_98DX3247_CNS"]   = { value=0xF40A11AB,  help="xCat3 device" },
        --["CPSS_98DXC323_CNS"]   = { value=0xF41B11AB,  help="xCat3 device" },
        --["CPSS_98DXN323_CNS"]   = { value=0xF41C11AB,  help="xCat3 device" },
            -- Series: PonCat3-FE
        ["CPSS_98DX1333_CNS"]   = { value=0xF41811AB,  help="xCat3 device" },
        ["CPSS_98DX1335_CNS"]   = { value=0xF41911AB,  help="xCat3 device" },
        ["CPSS_98DX1336_CNS"]   = { value=0xF41A11AB,  help="xCat3 device" },
            -- Series: PonCat3-GE
        ["CPSS_98DX3333_CNS"]   = { value=0xF40311AB,  help="xCat3 device" },
        ["CPSS_98DX3334_CNS"]   = { value=0xF40211AB,  help="xCat3 device" },
        ["CPSS_98DX3335_CNS"]   = { value=0xF40111AB,  help="xCat3 device" },
        ["CPSS_98DX3336_CNS"]   = { value=0xF40011AB,  help="xCat3 device\n" },
        -- Bobcat2
        ["CPSS_98DX4251_CNS"]   = { value=0xFC0011AB,  help="Bobcat2 device" },
        ["CPSS_98DX4220_CNS"]   = { value=0xFC0311AB,  help="Bobcat2 device" },
        ["CPSS_98DX8216_CNS"]   = { value=0xFC0411AB,  help="Bobcat2 device" },
        ["CPSS_98DX8224_CNS"]   = { value=0xFC0511AB,  help="Bobcat2 device" },
        ["CPSS_98DX4221_CNS"]   = { value=0xFC0211AB,  help="Bobcat2 device" },
        ["CPSS_98DX4222_CNS"]   = { value=0xFC0611AB,  help="Bobcat2 device" },
        ["CPSS_98DX8219_CNS"]   = { value=0xFC0811AB,  help="Bobcat2 device" },
        ["CPSS_98DX4223_CNS"]   = { value=0xFC0911AB,  help="Bobcat2 device" },
        ["CPSS_98DX4253_CNS"]   = { value=0xFC0711AB,  help="Bobcat2 device" },
        -- Bobcat3
        ["CPSS_98CX8410_CNS"]   = { value=0xD40011AB,  help="Bobcat3 device" },
        ["CPSS_98CX8420_CNS"]   = { value=0xD40F11AB,  help="Bobcat3 device\n" },
        -- Aldrin2
        ["CPSS_98DX8448_CNS"]   = { value=0xCC1E11AB,  help="Aldrin2 device" },
        ["CPSS_98DX8548_CNS"]   = { value=0xCC1F11AB,  help="Aldrin2 device" },
        ["CPSS_98EX5520_CNS"]   = { value=0xCC0F11AB,  help="Aldrin2 device\n" }
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
        ["167Mhz"]  = { value=167,  help="not relevant for Aldrin" },
        ["175Mhz"]  = { value=175,  help="Bobcat2 only" },
        ["200Mhz"]  = { value=200,  help="" },
        ["250Mhz"]  = { value=250,  help="" },
        ["290Mhz"]  = { value=290,  help="XCAT3 only" },
        ["362Mhz"]  = { value=362,  help="Bobcat2 only" },
        ["365Mhz"]  = { value=365,  help="not relevant for XCAT3" },
        ["480Mhz"]  = { value=480,  help="Aldrin only" },
        ["521Mhz"]  = { value=521,  help="Bobcat2 only" },
        ["525Mhz"]  = { value=525,  help="Bobcat3 and Aldrin2" },
        ["600Mhz"]  = { value=600,  help="Bobcat3 and Aldrin2" },
    }
}


local function CLI_check_param_feature_list(str, name, desc, varray, params)
    if params.deviceId > 0xF0000000 then
        desc = CLI_type_dict["features-list-xcat3"]
    else
        desc = CLI_type_dict["features-list-regular"]
    end
    return CLI_check_param_enum(str, name, desc, varray, params)
end

local function CLI_complete_param_feature_list(str, name, desc, varray, params)
    if params.deviceId > 0xF0000000 then
        desc = CLI_type_dict["features-list-xcat3"]
    else
        desc = CLI_type_dict["features-list-regular"]
    end
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
        ["EPLR_RM"]                         = { value=0x1,  help="Fix default value of Egress Policer memories" },
        ["EXTENDED_DSA_BYPASS_BRIDGE_RM"]   = { value=0x2,  help="enable Bypass Bridge for Extended DSA tag packets\n"..
                                "                                 enable on CPU port Bypass Bridge for Extended DSA tag packets\n"..
                                "                                 set all ePort entries to support Extended DSA bypass bridge " },
        ["LED_INIT"]                        = { value=0x4,  help="Device Led Init" }
    }
}
CLI_type_dict["features-list-xcat3"] = {
    help = "The features list",
    enum = {
        ["EXTENDED_DSA_BYPASS_BRIDGE_RM"]   = { value=0x2,  help="enable Bypass Bridge for Extended DSA tag packets\n"..
                                "                                 enable on CPU port Bypass Bridge for Extended DSA tag packets\n"..
                                "                                 set all ePort entries to support Extended DSA bypass bridge " }
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
    local devId, portMapArray, ret=0xffffffff

    enable_microInit_tracing()

    -- iterate thru all devices(if not only 0)
    for devId, portMapArray in pairs(port_map) do
      print("Applying mapping for device #"..tostring(devId))
      print("portMapArray=" .. to_string(portMapArray))
      local portMapArrayLen_ = #portMapArray
      ret= wrLogWrapper("wrlCpssDxChPortPhysicalPortMapSet", "(devId, portMapArrayLen_, portMapArray)", devId, portMapArrayLen_, portMapArray)
      if ret ~= 0 then
        print("  wrlCpssDxChPortPhysicalPortMapSet failed, ret="..to_string(ret))
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

    if not is_sip_5(devId) then
        -- check for params.dev_port_mic.devId
        -- tm applicable for Bobcat2 only
        params.tm = false
    end
    local range = nil
    if val == "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" or
       val == "CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E" then
        if is_sip_5_25(devId) then
            range = 77
        elseif is_sip_5_20(devId) then
            range = 78
        elseif is_sip_5(devId) then
            range = 71
        else
            range = 31
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
        ["ethernet_mac"] = { value="CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E", help="Ethernet MAC"},
        ["cpu_sdma"]     = { value="CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E",     help="CPU SDMA"},
        ["ilkn_mac"]     = { value="CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E", help="Interlaken Channel (all channels 0-63)"},
        ["remote_port"]  = { value="CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E",
                    help="Port represent physical interface on remote device or Interlaken channel"}
    }
}

CLI_type_dict["port_map_txq_portnum"] = {
    checker = CLI_check_param_number,
    min = 0,
    max = 492,
    help = "TXQ port number (0..492)"
}

local function port_map_ethernet(params)
    
    -- create record according to input params 
    local port_record = {}

    local devId=params.dev_port_mic.devId       -- device number

    local txq_port
    
    local tmenable = true
    local tm = params.tm
    if not tm then
        tmenable = false
        tm = 0xffffffff
    end

    if params["txq-port"] == nil then
        txq_port = params.dev_port_mic.portNum
    else
        txq_port = params["txq-port"]
    end

    -- fill CPSS_DXCH_PORT_MAP_STC record
    port_record = {
        physicalPortNumber=params.dev_port_mic.portNum,
        mappingType=params.port_mapping_type,
        portGroup=0,
        interfaceNum=params["interface-number"],
        txqPortNumber=txq_port,
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

-- port-map: list
local function port_map_list()
    --TODO
    print(to_string(port_map))
end

-- port-map: list
local function port_smi_list()
    --TODO
    print(to_string(port_smi))
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
            { format="txq-port %port_map_txq_portnum", help="txq/portNum" },
            { format="interface-number %port_map_interface_number", help="interface-number" },
            { format="tm %port_map_tm_index", help="enable tm" }
        }
    }
})

local function port_map_range(params)
    local start_interface_number, start_tm, start_txq_portnum
    local i
    if (params["interface-number"] ~= nil) then
        start_interface_number = params["interface-number"] 
    end
    if (params["txq-port"] ~= nil) then
        start_txq_portnum = params["txq-port"]
    end
    if (params.tm ~= nil) then 
        start_tm = params.tm
    end
    for dev,portTable in pairs(params["port-range"]) do
        i = 0
        for port=1,#portTable do
            params.dev_port_mic = { devId = dev, portNum = portTable[port] }
            if (params["interface-number"] ~= nil) then
                params["interface-number"] = start_interface_number + i
            end
            if (params["txq-port"] ~= nil) then
                params["txq-port"] = start_txq_portnum + i
            end
            if (params.tm ~= nil) then 
                params.tm = start_tm + i
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
            { format="txq-port %port_map_txq_portnum", help="txq/portNum" },
            { format="interface-number %port_map_interface_number", help="interface-number" },
            { format="tm %port_map_tm_index", help="enable tm" }
        }
    }
})
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
            ret, val = myGenWrapper("cpssDxChPortInbandAutoNegEnableSet", {
                { "IN", "GT_U8", "devNum", devId },
                { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", entry.poolPortNumber },
                { "IN", "GT_BOOL", "enable", enable }
            })
            if ret ~= 0 then
                print("cpssDxChPortInbandAutoNegEnableSet failed, ret="..to_string(ret))
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
        local isCaelum = wrlCpssCaelumCheck(devId)
        local devFamily = wrlCpssDeviceFamilyGet(devId)
        local count = 0
        local last_low_port, last_high_port = nil, nil
        if isCaelum == true then 
        print("Caelum device")
            for _, entry in pairs(portPizzaArray) do
                if entry.physicalPortNumber < 48 then 
                    last_low_port = entry.physicalPortNumber
                else
                    last_high_port = entry.physicalPortNumber
                end                
            end
        end

        for _, entry in pairs(portPizzaArray) do
            count = count + 1
            print("portPizzaArray=" .. to_string(entry))

            if ("CPSS_PP_FAMILY_DXCH_XCAT3_E" ~= devFamily) then
                if (count == #portPizzaArray) then
                    enable_microInit_tracing()
                end
                ret, val = myGenWrapper("cpssDxChPortPizzaArbiterIfConfigSet", {
                    { "IN", "GT_U8",                        "devNum",   devId },
                    { "IN", "GT_PHYSICAL_PORT_NUM",         "portNum",  entry.physicalPortNumber },
                    { "IN", "CPSS_PORT_SPEED_ENT",          "speed",    entry.speed }
                })
                if ret ~= 0 then
                    print("cpssDxChPortPizzaArbiterIfConfigSet failed, ret="..to_string(ret))
                    break
                end
                if (count == #portPizzaArray) then
                    disable_microInit_tracing()
                end
            end
            if isCaelum == true then 
                if (entry.physicalPortNumber == last_low_port) or (entry.physicalPortNumber == last_high_port) then 
                    enable_microInit_tracing()
                end
                ret, val = myGenWrapper("cpssDxChPortResourcesConfigSet", {
                    { "IN", "GT_U8",                        "devNum",   devId },
                    { "IN", "GT_PHYSICAL_PORT_NUM",         "portNum",  entry.physicalPortNumber },
                    { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode",   entry.ifmode },
                    { "IN", "CPSS_PORT_SPEED_ENT",          "speed",    entry.speed }
                })
                if ret ~= 0 then
                    print("cpssDxChPortResourcesConfigSet failed, ret="..to_string(ret))
                    break
                end
                disable_microInit_tracing()
            else
                enable_microInit_tracing()
                ret, val = myGenWrapper("cpssDxChPortResourcesConfigSet", {
                    { "IN", "GT_U8",                        "devNum",   devId },
                    { "IN", "GT_PHYSICAL_PORT_NUM",         "portNum",  entry.physicalPortNumber },
                    { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode",   entry.ifmode },
                    { "IN", "CPSS_PORT_SPEED_ENT",          "speed",    entry.speed }
                })
                if ret ~= 0 then
                    print("cpssDxChPortResourcesConfigSet failed, ret="..to_string(ret))
                    break
                end
                disable_microInit_tracing()
            end
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
