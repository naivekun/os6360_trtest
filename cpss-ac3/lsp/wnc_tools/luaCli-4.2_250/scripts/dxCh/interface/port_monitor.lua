--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_monitor.lua
--*
--* DESCRIPTION:
--*       setting of the port monitoring
--*
--* FILE REVISION NUMBER:
--*       $Revision: 8 $
--*
--********************************************************************************

--constants
-- for mirror restore
local apiName
local mirroringModeGet = nil

-- ************************************************************************
---
--  port_monitor_func
--        @description  start's port monitoring session
--
--        @param params             - params["dev_port"]: analyzer device
--                                    and port;
--                                    params["rx"]: rx mirrored ports
--                                    analysing property, could be
--                                    irrelevant;
--                                    params["tx"]: tx mirrored ports
--                                    analysing property, could be
--                                    irrelevant;
--                                    params["both"]: rx and tx mirrored
--                                    ports analysing property, could be
--                                    irrelevant
--
--        @return       true on success, otherwise false and error message
--
local function port_monitor_func(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local analyzer_port, analyzer_device
    local analyzer_interface_index, analyzer_interface
    local rx_analyzer_setting, tx_analyzer_setting
    local is_port_phys


    analyzer_interface_index = params.analyzerIndex -- will be nil if params.analyzerIndex is not specified

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    -- Command specific variables initialization
    analyzer_device         = params["dev_port"]["devId"]
    analyzer_port           = params["dev_port"]["portNum"]
    rx_analyzer_setting     = getTrueIfNil(params["tx"])
    tx_analyzer_setting     = getTrueIfNil(params["rx"])

    local mirror_analyzer_entries = command_data:getEmptyDeviceNestedTable()

    -- Conversion of analyzer device id and port number to hardware format.
    if (true == command_data["status"])             and
       ((true == rx_analyzer_setting)               or
        (true == tx_analyzer_setting))              then
        result, values, analyzer_port =
            device_port_to_hardware_format_convert(analyzer_device,
                                                   analyzer_port)
        if        0 == result                   then
            analyzer_device = values
        elseif 0x10 == result                   then
            command_data:setFailStatus()
            command_data:addError("Device id %d and port number %d " ..
                                  "converting is not allowed.", devNum, portNum)
        elseif    0 ~= result                   then
            command_data:setFailStatus()
            command_data:addError("Error at device id %d and port number %d " ..
                                  "converting: %s.", devNum, portNum, values)
        end
    end

    -- Main port handling cycle
    if true == command_data["status"]               then
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()
            if mirroringModeGet == nil and not is_sip_6(devNum) then
                apiName = "cpssDxChMirrorToAnalyzerForwardingModeGet"
                result,val = myGenWrapper(apiName, {
                    { "IN",     "GT_U8",                        "devNum",       devNum},
                    { "OUT",    "CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT",  "mode"}
                })
                mirroringModeGet  = val["mode"]
            end

            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

            if  is_device_xCat_or_higher(devFamily)  then
                -- Setting of forwarding mode to Analyzer for egress/ingress
                -- mirroring.
                if (true == command_data["local_status"]  and not is_sip_6(devNum)) then
                    result, values =
                        cpssPerDeviceParamSet(
                            "cpssDxChMirrorToAnalyzerForwardingModeSet",
                            devNum,
                            "CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E",
                            "mode",
                            "CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT")
                    if     0x10 == result                       then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to set " ..
                                                "forwarding mode to " ..
                                                "Analyzer for egress/" ..
                                                "ingress mirroring " ..
                                                "on device %d.", devNum)
                    elseif    0 ~= result                       then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at setting of " ..
                                              "forwarding mode to Analyzer " ..
                                              "for egress/ingress mirroring " ..
                                              "on device %d: %s", devNum,
                                              returnCodes[result])
                    end
                end

                -- Getting of necessary interface index.
                if (true == command_data["local_status"] and
                    analyzer_inderface_index == nil) then
                    -- analyzer index is not specified in the CLI command.

                    result, values, analyzer_interface =
                        given_mirror_analyzer_or_invalid_index_get(
                            devNum,
                            { ["interface"] =
                                { ["type"]      = "CPSS_INTERFACE_PORT_E",
                                  ["devPort"]   = { ["devNum"]  = analyzer_device,
                                                    ["portNum"] = analyzer_port    }}})
                    if       (0 == result) and
                           (nil ~= values)                      then
                        analyzer_interface_index    = values
                    elseif (0   == result)                      and
                           (nil == values)                      then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addWarning("Could not add new analazer " ..
                                                "interface entry, because all " ..
                                                "entries are busy on device %d.",
                                                devNum)
                    elseif 0x10 == result                       then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addWarning("It is not allowed to get the " ..
                                                "analyzer interface index " ..
                                                "on device %d.", devNum)
                    elseif    0 ~= result                       then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at analyzer interface " ..
                                              "index getting of on device " ..
                                              "%d: %s", devNum,
                                              returnCodes[result])
                    end
                end

                -- The analyzer interface setting.
                if true == command_data["local_status"]             then
                    analyzer_interface = {
                        interface = {
                            ["type"]    ="CPSS_INTERFACE_PORT_E",
                            ["devPort"] = {["devNum"]  = analyzer_device,
                                           ["portNum"] = analyzer_port}}}

                -- The mirror analyzer interface setting.
                    result, values =
                        myGenWrapper("cpssDxChMirrorAnalyzerInterfaceSet",
                                     {{ "IN",   "GT_U8",         "dev",  devNum },
                                      { "IN",   "GT_U32",        "index",
                                                       analyzer_interface_index },
                                      { "IN",
     "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC", "interface", analyzer_interface }})
                    if     0x10 == result                       then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to set the " ..
                                                "analyzer port of rx mirrored " ..
                                                "ports on device %d.", devNum)
                    elseif    0 ~= result                       then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at the analyzer port " ..
                                              "setting of rx mirrored ports " ..
                                              "on device %d: %s", devNum,
                                              returnCodes[result])
                    end
                end

                -- Mirror analyzer entry saving.
                if true == command_data["local_status"]             then
                    mirror_analyzer_entries[devNum]["index"] =
                                                        analyzer_interface_index
                    mirror_analyzer_entries[devNum]["entry"] =
                                                        analyzer_interface
                end
            else
                -- The analyzer port setting of rx mirrored ports.
                if (true == command_data["local_status"])           and
                   (true == rx_analyzer_setting)                    then
                    result, values =
                        myGenWrapper("cpssDxChMirrorRxAnalyzerPortSet",
                                     {{ "IN",  "GT_U8",        "devNum",
                                                                       devNum },
                                      { "IN", "GT_PORT_NUM",   "analyzerPort",
                                                                analyzer_port },
                                      { "IN", "GT_HW_DEV_NUM", "analyzerDev",
                                                              analyzer_device }})
                    if     0x10 == result then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to set the " ..
                                                "analyzer port of rx mirrored " ..
                                                "ports on device %d.", devNum)
                    elseif    0 ~= result then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at the analyzer port " ..
                                              "setting of rx mirrored ports on " ..
                                              "device %d: %s", devNum,
                                              returnCodes[result])
                    end
                end

                -- The analyzer port setting of tx mirrored ports.
                if (true == command_data["local_status"])           and
                   (true == tx_analyzer_setting)                    then
                    result, values =
                        myGenWrapper("cpssDxChMirrorTxAnalyzerPortSet",
                                     {{ "IN",  "GT_U8",        "devNum",
                                                                       devNum },
                                      { "IN", "GT_PORT_NUM",   "analyzerPort",
                                                                analyzer_port },
                                      { "IN", "GT_HW_DEV_NUM", "analyzerDev",
                                                              analyzer_device }})
                    if     0x10 == result then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to set the " ..
                                                "analyzer port of tx mirrored " ..
                                                "ports on device %d.", devNum)
                    elseif    0 ~= result then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at the analyzer port " ..
                                              "setting of tx mirrored ports on " ..
                                              "device %d: %s", devNum,
                                              returnCodes[result])
                    end
                end

                -- Mirror analyzer entry index saving.
                if true == command_data["local_status"]             then
                    mirror_analyzer_entries[devNum]["index"] = 0
                end
            end

            command_data:updateStatus()

            command_data:updateDevices()
        end

        command_data:addWarningIfNoSuccessDevices(
            "Can not configure mirror analyser port for all processed devices")
    end

    -- Main port handling cycle
    if true == command_data["status"]               then
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            -- Mirror analyzer entry index restoring.
            if true == command_data["local_status"]             then
                analyzer_interface_index =
                                        mirror_analyzer_entries[devNum]["index"]
            end

            -- Rx port mirroring setting.
            if (true == command_data["local_status"])           and
               (true == rx_analyzer_setting)                    then

                is_port_phys = (portNum<256)
                result, values =
                    myGenWrapper("cpssDxChMirrorRxPortSet",
                                 {{ "IN", "GT_U8",       "devNum",         devNum       },
                                  { "IN", "GT_PORT_NUM", "mirrPort",       portNum      },
                                  { "IN", "GT_BOOL",     "isPhysicalPort", is_port_phys },
                                  { "IN", "GT_BOOL",     "enable",         true         },
                                  { "IN", "GT_U32",      "index",
                                                         analyzer_interface_index  }})
                if     0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to set a " ..
                                            "specific port %d as rx mirrored " ..
                                            "port on device %d.", portNum,
                                            devNum)
                elseif    0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at setting of a specific " ..
                                          "port %d as rx mirrored port " ..
                                          "on device %d.", portNum, devNum)
                end
            end

            -- Tx port mirroring setting.
            if (true == command_data["local_status"])           and
               (true == tx_analyzer_setting)                    then

               is_port_phys = (portNum<256)

                result, values =
                    myGenWrapper("cpssDxChMirrorTxPortSet",
                                 {{ "IN", "GT_U8",       "devNum",         devNum       },
                                  { "IN", "GT_PORT_NUM", "mirrPort",       portNum      },
                                  { "IN", "GT_BOOL",     "isPhysicalPort", is_port_phys },
                                  { "IN", "GT_BOOL",     "enable",         true         },
                                  { "IN", "GT_U32",      "index",
                                                         analyzer_interface_index  }})
                if     0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to set a " ..
                                            "specific port %d as tx mirrored " ..
                                            "port on device %d.", portNum,
                                            devNum)
                elseif    0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at setting of a specific " ..
                                          "port %d as tx mirrored port " ..
                                          "on device %d.", portNum, devNum)
                end
            end

            command_data:updateStatus()

            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not specified as mirrored all processed ports")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  no_port_monitor_func
--        @description  stop's port monitoring session
--
--        @param params             - params["dev_port"]: analyzer device
--                                    and port
--
--        @return       true on success, otherwise false and error message
--
local function no_port_monitor_func(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local analyzer_device, analyzer_port
    local current_analyzer_device, current_analyzer_port
    local analyzer_interface_enabling, analyzer_interface_index, analyzer_interface
    local is_analyzer_interface_exist, are_analyzer_interface_potrs_exist
    local analyzer_invalidating
    local rx_mirrored_ports_invalidating, tx_mirrored_ports_invalidating
    local is_port_phys
    local defaultAnalyzerInf

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    -- Command specific variables initialization
    analyzer_device             = params["dev_port"]["devId"]
    analyzer_port               = params["dev_port"]["portNum"]
    local mirror_analyzer_entries     = command_data:getEmptyDeviceNestedTable()
    defaultAnalyzerInf = { ["interface"] =
                                { ["type"]      = "CPSS_INTERFACE_PORT_E",
                                  ["devPort"]   = { ["devNum"]  = 0,
                                                    ["portNum"] = 0}}}

    -- Conversion of analyzer device id and port number to hardware format.
    if true == command_data["status"]               then
        result, values, analyzer_port =
            device_port_to_hardware_format_convert(analyzer_device,
                                                   analyzer_port)
        if        0 == result                   then
            analyzer_device = values
        elseif 0x10 == result                   then
            command_data:setFailStatus()
            command_data:addError("Device id %d and port number %d " ..
                                  "converting is not allowed.", devNum, portNum)
        elseif    0 ~= result                   then
            command_data:setFailStatus()
            command_data:addError("Error at device id %d and port number %d " ..
                                  "converting: %s.", devNum, portNum, values)
        end
    end

    -- Main device handling cycle
    if true == command_data["status"]               then
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

            if  is_device_xCat_or_higher(devFamily) then
                -- Getting of necessary interface index.
                if (true == command_data["local_status"])           then
                    result, analyzer_interface_index, analyzer_interface =
                        next_mirror_analyzer_index_get(
                            devNum, 0, true,
                            { ["interface"] =
                                { ["type"]      = "CPSS_INTERFACE_PORT_E",
                                  ["devPort"]   = { ["devNum"]  = analyzer_device,
                                                    ["portNum"] = analyzer_port    }}})
                    if        0 == result                       then
                        mirror_analyzer_entries[devNum]["entry_existance"]  =
                                            isNotNil(analyzer_interface_index)
                        mirror_analyzer_entries[devNum]["index"]            =
                                                        analyzer_interface_index
                        mirror_analyzer_entries[devNum]["entry"]            =
                                                            analyzer_interface
                    elseif 0x10 == result                       then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addWarning("It is not allowed to get the " ..
                                                "analyzer interface index " ..
                                                "on device %d.", devNum)
                    elseif    0 ~= result                       then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at analyzer interface " ..
                                              "index getting of on device " ..
                                              "%d: %s", devNum,
                                              returnCodes[result])
                    end
                end
            else
                -- The analyzer port getting of rx mirrored ports.
                if true == command_data["local_status"]             then
                    result, values =
                        myGenWrapper("cpssDxChMirrorRxAnalyzerPortGet",
                                     {{ "IN",  "GT_U8",         "dev",  devNum },
                                      { "OUT", "GT_PORT_NUM",   "analyzerPort" },
                                      { "OUT", "GT_HW_DEV_NUM", "analyzerDev"  }})
                    if        0 == result then
                        current_analyzer_device  = values["analyzerDev"]
                        current_analyzer_port    = values["analyzerPort"]
                    elseif 0x10 == result then
                        command_data:setFailPortStatus()
                        command_data:addWarning("It is not allowed to get the " ..
                                                "analyzer port of rx mirrored " ..
                                                "ports on device %d.", devNum)
                    elseif    0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at the analyzer port " ..
                                              "getting of rx mirrored ports on " ..
                                              "device %d: %s", devNum,
                                              returnCodes[result])
                    end
                end

                -- Detecting of rx mirrored ports invalidating property.
                if true == command_data["local_status"]             then
                    rx_mirrored_ports_invalidating =
                        (analyzer_device == current_analyzer_device) and
                        (analyzer_port == current_analyzer_port)
                end

                -- The analyzer port setting of rx mirrored ports.
                if (true == command_data["local_status"])           and
                   (true == rx_mirrored_ports_invalidating)         then
                    result, values =
                        myGenWrapper("cpssDxChMirrorRxAnalyzerPortSet",
                                     {{ "IN",  "GT_U8",        "devNum",
                                                                       devNum },
                                      { "IN", "GT_PORT_NUM",   "analyzerPort",
                                                                analyzer_port },
                                      { "IN", "GT_HW_DEV_NUM", "analyzerDev",
                                                           get_device_count() }})
                    if     0x10 == result then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to set the " ..
                                                "analyzer port of rx mirrored " ..
                                                "ports on device %d.", devNum)
                    elseif    0 ~= result then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at the analyzer port " ..
                                              "setting of rx mirrored ports on " ..
                                              "device %d: %s", devNum,
                                              returnCodes[result])
                    end
                end

                -- The analyzer port getting of tx mirrored ports.
                if true == command_data["local_status"]             then
                    result, values =
                        myGenWrapper("cpssDxChMirrorTxAnalyzerPortGet",
                                     {{ "IN",  "GT_U8",         "dev",  devNum },
                                      { "OUT", "GT_PORT_NUM",   "analyzerPort" },
                                      { "OUT", "GT_HW_DEV_NUM", "analyzerDev"  }})
                    if        0 == result then
                        current_analyzer_device  = values["analyzerDev"]
                        current_analyzer_port    = values["analyzerPort"]
                    elseif 0x10 == result then
                        command_data:setFailPortStatus()
                        command_data:addWarning("It is not allowed to get the " ..
                                                "analyzer port of tx mirrored " ..
                                                "ports on device %d.", devNum)
                    elseif    0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at the analyzer port " ..
                                              "getting of tx mirrored ports on " ..
                                              "device %d: %s", devNum,
                                              returnCodes[result])
                    end
                end

                -- Detecting of tx mirrored ports invalidating property.
                if true == command_data["local_status"]             then
                    tx_mirrored_ports_invalidating =
                        (analyzer_device == current_analyzer_device) and
                        (analyzer_port == current_analyzer_port)
                end

                -- The analyzer port setting of tx mirrored ports.
                if (true == command_data["local_status"])           and
                   (true == tx_mirrored_ports_invalidating)         then
                    result, values =
                        myGenWrapper("cpssDxChMirrorTxAnalyzerPortSet",
                                     {{ "IN",  "GT_U8",        "devNum",
                                                                       devNum },
                                      { "IN", "GT_PORT_NUM",   "analyzerPort",
                                                                analyzer_port },
                                      { "IN", "GT_HW_DEV_NUM", "analyzerDev",
                                                           get_device_count() }})
                    if     0x10 == result then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to set the " ..
                                                "analyzer port of tx mirrored " ..
                                                "ports on device %d.", devNum)
                    elseif    0 ~= result then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at the analyzer port " ..
                                              "setting of tx mirrored ports on " ..
                                              "device %d: %s", devNum,
                                              returnCodes[result])
                    end
                end

                if true == command_data["local_status"]             then
                    mirror_analyzer_entries[devNum]["entry_existance"] = true
                    mirror_analyzer_entries[devNum]["rx_invalidation"] =
                                                rx_mirrored_ports_invalidating
                    mirror_analyzer_entries[devNum]["tx_invalidation"] =
                                                tx_mirrored_ports_invalidating
                end
            end

            command_data:updateStatus()

            command_data:updateDevices()
        end

        command_data:addWarningIfNoSuccessDevices(
            "Can not configure mirror analyser port for all processed devices")
    end

    -- Main port handling cycle
    if true == command_data["status"]               then
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            -- Mirror analysing entry getting.
            if true == command_data["local_status"]                 then
                is_analyzer_interface_exist =
                            mirror_analyzer_entries[devNum]["entry_existance"]
            end

            -- Rx port mirroring getting.
            if (true == command_data["local_status"]) and is_analyzer_interface_exist then
                is_port_phys = (portNum<256)
                result, values =
                    myGenWrapper("cpssDxChMirrorRxPortGet",
                                 {{ "IN",  "GT_U8",       "devNum",         devNum       },
                                  { "IN",  "GT_PORT_NUM", "mirrPort",       portNum      },
                                  { "IN",  "GT_BOOL",     "isPhysicalPort", is_port_phys },
                                  { "OUT", "GT_BOOL",     "enable",                      },
                                  { "OUT", "GT_U32",      "index",                       }})
                if       0 == result then
                    analyzer_interface_enabling = values["enable"]
                    analyzer_interface_index    = values["index"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to set a " ..
                                            "specific port %d as rx mirrored " ..
                                            "port on device %d.", portNum,
                                            devNum)
                elseif    0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at setting of a specific " ..
                                          "port %d as rx mirrored port " ..
                                          "on device %d.", portNum, devNum)
                end
            end

            -- Detecting of rx port mirroring disabling property.
            if (true == command_data["local_status"])               and
               (true == is_analyzer_interface_exist)                then
                analyzer_invalidating =
                    (true == analyzer_interface_enabling) and
                    ((mirror_analyzer_entries[devNum]["index"] ==
                        analyzer_interface_index)         or
                     (true ==
                        mirror_analyzer_entries[devNum]["rx_invalidation"]))
            end

            -- Disabling of rx port mirroring.
            if (true == command_data["local_status"])               and
               (true == is_analyzer_interface_exist)                and
               (true == analyzer_invalidating)                      then

                is_port_phys = (portNum<256)

                result, values =
                    myGenWrapper("cpssDxChMirrorRxPortSet",
                                 {{ "IN", "GT_U8",       "devNum",         devNum       },
                                  { "IN", "GT_PORT_NUM", "mirrPort",       portNum      },
                                  { "IN", "GT_BOOL",     "isPhysicalPort", is_port_phys },
                                  { "IN", "GT_BOOL",     "enable",         false        },
                                  { "IN", "GT_U32",      "index", analyzer_interface_index  }})
                if     0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to set a " ..
                                            "specific port %d as rx mirrored " ..
                                            "port on device %d.", portNum,
                                            devNum)
                elseif    0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at setting of a specific " ..
                                          "port %d as rx mirrored port " ..
                                          "on device %d.", portNum, devNum)
                end
            end

            -- Tx port mirroring getting.
            if (true == command_data["local_status"])               and
               (true == is_analyzer_interface_exist)                then

                is_port_phys = (portNum<256)

                result, values =
                    myGenWrapper("cpssDxChMirrorTxPortGet",
                                 {{ "IN",  "GT_U8",       "devNum",         devNum       },
                                  { "IN",  "GT_PORT_NUM", "mirrPort",       portNum      },
                                  { "IN",  "GT_BOOL",     "isPhysicalPort", is_port_phys },
                                  { "OUT", "GT_BOOL",     "enable",                      },
                                  { "OUT", "GT_U32",      "index",                       }})
                if       0 == result then
                    analyzer_interface_enabling = values["enable"]
                    analyzer_interface_index    = values["index"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to set a " ..
                                            "specific port %d as tx mirrored " ..
                                            "port on device %d.", portNum,
                                            devNum)
                elseif    0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at setting of a specific " ..
                                          "port %d as tx mirrored port " ..
                                          "on device %d.", portNum, devNum)
                end
            end

            -- Detecting of tx port mirroring disabling property.
            if (true == command_data["local_status"])               and
               (true == is_analyzer_interface_exist)                then
                analyzer_invalidating =
                    (true == analyzer_interface_enabling) and
                    ((mirror_analyzer_entries[devNum]["index"] ==
                        analyzer_interface_index)         or
                     (true ==
                        mirror_analyzer_entries[devNum]["tx_invalidation"]))
            end

            -- Disabling of tx port mirroring.
            if (true == command_data["local_status"])               and
               (true == is_analyzer_interface_exist)                and
               (true == analyzer_invalidating)                      then

                is_port_phys = (portNum<256)

                result, values =
                    myGenWrapper("cpssDxChMirrorTxPortSet",
                                 {{ "IN", "GT_U8",       "devNum",         devNum       },
                                  { "IN", "GT_PORT_NUM", "mirrPort",       portNum      },
                                  { "IN", "GT_BOOL",     "isPhysicalPort", is_port_phys },
                                  { "IN", "GT_BOOL",     "enable",         false        },
                                  { "IN", "GT_U32",      "index",          analyzer_interface_index }})
                if     0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to set a " ..
                                            "specific port %d as tx mirrored " ..
                                            "port on device %d.", portNum,
                                            devNum)
                elseif    0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at setting of a specific " ..
                                          "port %d as tx mirrored port " ..
                                          "on device %d.", portNum, devNum)
                end
            end

            command_data:updateStatus()

            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not stopping mirroring at all processed ports")
    end

    -- Main device handling cycle
    if true == command_data["status"]               then
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

            -- Mirror analyzing entry getting.
            if  is_device_xCat_or_higher(devFamily)   and
               (true == mirror_analyzer_entries[devNum]["entry_existance"]) then
                -- Analyzer interface getting and invalidating.
                if true == command_data["local_status"]         then
                    analyzer_interface_index    =
                                        mirror_analyzer_entries[devNum]["index"]
                    analyzer_interface          =
                                        mirror_analyzer_entries[devNum]["entry"]
                    analyzer_interface["interface"]["devPort"]["devNum"]    =
                                                            get_device_count()

                    -- Checking of port existing with given mirror analysing
                    -- interface.
                    result, values =
                        is_mirrored_port_exist("both", devNum, 0,
                                               analyzer_interface_index)
                    if        0 == result                       then
                        are_analyzer_interface_potrs_exist = values
                    elseif 0x10 == result                       then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to check " ..
                                                "the mirror analysing " ..
                                                "interface on device %d.",
                                                devNum)
                    elseif    0 ~= result                       then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at checking of  the" ..
                                              "mirror analysing interface " ..
                                              "on device %d: %s", devNum,
                                              returnCodes[result])
                    end
                end

                -- The mirror analyzer interface setting.
                if (true == command_data["local_status"])       and
                   (false == are_analyzer_interface_potrs_exist) then
                    result, values =
                        myGenWrapper("cpssDxChMirrorAnalyzerInterfaceSet",
                                     {{ "IN",   "GT_U8",         "dev",  devNum },
                                      { "IN",   "GT_U32",        "index",
                                                       analyzer_interface_index },
                                      { "IN",
     "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC", "interface", defaultAnalyzerInf }})
                    if     0x10 == result                       then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to set the " ..
                                                "analyzer port of rx mirrored " ..
                                                "ports on device %d.", devNum)
                    elseif    0 ~= result                       then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at the analyzer port " ..
                                              "setting of rx mirrored ports " ..
                                              "on device %d: %s", devNum,
                                              returnCodes[result])
                    end
                end
            end

            if mirroringModeGet ~= nil then
                apiName = "cpssDxChMirrorToAnalyzerForwardingModeSet"
                result,val = myGenWrapper(apiName, {
                    { "IN",    "GT_U8",                                 "devNum",       devNum},
                    { "IN",    "CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT",  "mode", mirroringModeGet}
                })
                mirroringModeGet = nil
            end

            command_data:updateStatus()

            command_data:updateDevices()
        end

        command_data:addWarningIfNoSuccessDevices(
            "Can not mirror analysing entries on all processed devices")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------
-- command registration: port monitor
--------------------------------------------------------
CLI_addCommand({"interface", "interface_eport"}, "port monitor", {
  func   = port_monitor_func,
  help   = "Monitor another interface",
  params = {
      { type = "values",
        "%dev_port"
      },
      { type="named",
        "#rx_tx_both",
        {format = "analyzer-index %analyzer_index", name="analyzerIndex", help = "an analyzer index"},
      }
  }
})

CLI_addCommand({"interface", "interface_eport"}, "no port monitor", {
  func   = no_port_monitor_func,
  help   = "Monitor another interface",
  params = {
      { type = "values",
         "%dev_port"
      }
  }
})
