--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_show_capabilities_device.lua
--*
--* DESCRIPTION:
--*       showing of system capabilities
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDevicePortNumberGet")
cmdLuaCLI_registerCfunction("wrlCpssDeviceTypeGet")
cmdLuaCLI_registerCfunction("wrlCpssDeviceRevisionGet")

--constants

-- ************************************************************************
---
--  show_capabilities_device_func_given_device
--        @description  shows system capabilities for given device
--
--        @param command_data   - command execution data object
--        @param devNum         - device number
--        @param header_string  - printed header string
--
--        @return       true on success, otherwise false and error message
--
local function show_capabilities_device_func_given_device(command_data, devNum, header_str)
    -- Common variables declaration
    local result, values
    -- Command  specific variables declaration
    local centralized_counters_entries_number_string_prefix
    local centralized_counters_entries_number_string_suffix
    local centralized_counters_entries_number
    local centralized_counters_entries_number_string
    local pfc_counters_string_prefix
    local pfc_counters_string_suffix
    local pfc_counters_string, pfc_counters_number
    local bma_multicast_counters_string_prefix
    local bma_multicast_counters_string_suffix
    local bma_multicast_counters_string, bma_multicast_counters_number
    local cnc_blocks_number_string_prefix
    local cnc_blocks_number_string_suffix
    local cnc_blocks_number, cnc_blocks_number_string
    local max_port_number_string_prefix
    local max_port_number_string_suffix
    local max_port_number, max_port_number_string
    local device_type_string_prefix, device_type_string_suffix
    local device_type, device_type_string
    local device_revision_string_prefix, device_revision_string_suffix
    local device_revision, device_revision_string
    local device_exisitg_ports_bitmap0_string_prefix
    local device_exisitg_ports_bitmap0_string_suffix
    local device_exisitg_ports_bitmap0, device_exisitg_ports_bitmap0_string
    local device_exisitg_ports_bitmap1_string_prefix
    local device_exisitg_ports_bitmap1_string_suffix
    local device_exisitg_ports_bitmap1, device_exisitg_ports_bitmap1_string
    local device_exisitg_ports_bitmap2_string_prefix
    local device_exisitg_ports_bitmap2_string_suffix
    local device_exisitg_ports_bitmap2, device_exisitg_ports_bitmap2_string
    local device_exisitg_ports_bitmap3_string_prefix
    local device_exisitg_ports_bitmap3_string_suffix
    local device_exisitg_ports_bitmap3, device_exisitg_ports_bitmap3_string
    local supportedSip_prefix
    local supportedSip
    local header_string, footer_string

    -- Command specific variables initialization.
    if nil == header_str then
        header_string                        = ""
    else
        header_string                        = header_str
    end
    centralized_counters_entries_number_string_prefix =
                                "Number of entries in CNC table\t\t\t\t\t"
    centralized_counters_entries_number_string_suffix = ""
    cnc_blocks_number_string_prefix          =
                                 "Number of CNC blocks\t\t\t\t\t\t"
    cnc_blocks_number_string_suffix          = ""
    pfc_counters_string_prefix               = "Number of entries in PFC Counters table\t\t\t\t"
    pfc_counters_string_suffix               = ""
    bma_multicast_counters_string_prefix     = "Number of entries in BMA multicast counters table\t\t"
    bma_multicast_counters_string_suffix     = ""
    max_port_number_string_prefix            = "Max port number\t\t\t\t\t\t\t"
    max_port_number_string_suffix            = ""
    device_type_string_prefix                = "Device Type\t\t\t\t\t\t\t"
    device_type_string_suffix                = ""
    device_revision_string_prefix            = "Device Revision\t\t\t\t\t\t\t"
    device_revision_string_suffix            = ""
    device_exisitg_ports_bitmap0_string_prefix  =
                                        "DeviceExistingPortsBitmap.0\t\t\t\t\t"
    device_exisitg_ports_bitmap0_string_suffix  = ""
    device_exisitg_ports_bitmap1_string_prefix  =
                                        "DeviceExistingPortsBitmap.1\t\t\t\t\t"
    device_exisitg_ports_bitmap1_string_suffix  = ""
    device_exisitg_ports_bitmap2_string_prefix  =
                                        "DeviceExistingPortsBitmap.2\t\t\t\t\t"
    device_exisitg_ports_bitmap2_string_suffix  = ""
    device_exisitg_ports_bitmap3_string_prefix  =
                                        "DeviceExistingPortsBitmap.3\t\t\t\t\t"
    device_exisitg_ports_bitmap3_string_suffix  = ""
    footer_string                            = "\n"

    -- Getting of CNC (centralized counters) entries number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        centralized_counters_entries_number = system_capability_get_table_size(
                                                  devNum, "CNC_0_COUNTERS")
        centralized_counters_entries_number_string =
            strIfNotNil(centralized_counters_entries_number)
    end

    command_data:updateStatus()

    -- CNC BLOCK (centralized counters) entries number string
    -- formatting and adding.
    command_data:setResultStr(centralized_counters_entries_number_string_prefix,
                              centralized_counters_entries_number_string,
                              centralized_counters_entries_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of CNC BLOCKS number in system
    command_data:clearStatus()

    if true  == command_data["status"] then
        cnc_blocks_number = 2
        cnc_blocks_number_string = strIfNotNil(cnc_blocks_number)
    end

    command_data:updateStatus()

    -- CNC BLOCKS number string formatting and adding.
    command_data:setResultStr(cnc_blocks_number_string_prefix,
                              cnc_blocks_number_string,
                              cnc_blocks_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of BMA multicast counters number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        bma_multicast_counters_number = system_capability_get_table_size(
                                                  devNum, "BMA_MULTICAST_COUNTERS")
        bma_multicast_counters_string =
            strIfNotNil(bma_multicast_counters_number)
    end

    command_data:updateStatus()

    -- BMA multicast counters number string
    -- formatting and adding.
    command_data:setResultStr(bma_multicast_counters_string_prefix,
                              bma_multicast_counters_string,
                              bma_multicast_counters_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of PFC counters number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        pfc_counters_number = system_capability_get_table_size(
                                devNum, "PFC_LLFC_COUNTERS")
        pfc_counters_string =
            strIfNotNil(pfc_counters_number)
    end

    command_data:updateStatus()

    -- PFC counters number string
    -- formatting and adding.
    command_data:setResultStr(pfc_counters_string_prefix,
                              pfc_counters_string,
                              pfc_counters_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of device max port number.
    command_data:clearStatus()

    if true  == command_data["status"] then
        result, values = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
        if     0 == result then
            max_port_number = values
        elseif 0 ~= result then
            command_data:setFailStatus()
            command_data:addError("Error at max port number getting of device %d: %s ",
                                  devNum, values)
        end

        if 0 == result then
            max_port_number_string = strIfNotNil(max_port_number - 1)
        else
            max_port_number_string = "n/a"
        end
    end

    command_data:updateStatus()

    -- Device max port number string formatting and adding.
    command_data:setResultStr(max_port_number_string_prefix,
                              max_port_number_string,
                              max_port_number_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of device type.
    command_data:clearStatus()

    if true  == command_data["status"] then
        result = wrLogWrapper("wrlCpssDeviceTypeGet", "(devNum)", devNum)
        if nil ~= result then
            device_type_string = string.format("%x", result)
        else
            device_type_string = "n/a"
            command_data:setFailStatus()
            command_data:addError("Error at type getting of device %d", devNum)
        end
    end

    command_data:updateStatus()

    -- Device max port number string formatting and adding.
    command_data:setResultStr(device_type_string_prefix,
                              device_type_string,
                              device_type_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of device revision.
    command_data:clearStatus()

    if true  == command_data["status"] then
        result = wrLogWrapper("wrlCpssDeviceRevisionGet", "(devNum)", devNum)

        if nil ~= result then
            device_revision_string = strIfNotNil(result)
        else
            device_revision_string = "n/a"
            command_data:setFailStatus()
            command_data:addError("Error at type getting of device %d", devNum)
        end
    end

    command_data:updateStatus()

    -- Device max port number string formatting and adding.
    command_data:setResultStr(device_revision_string_prefix,
                              device_revision_string,
                              device_revision_string_suffix)
    command_data:addResultToResultArray()

    -- Getting of device exisitg ports bitmap.
    command_data:clearStatus()

    if true  == command_data["status"] then
        result, values = cpssPerDeviceParamGet("cpssPxCfgDevInfoGet",
                                               devNum, "devInfo",
                                               "CPSS_PX_CFG_DEV_INFO_STC")
        if        0 == result then
            device_exisitg_ports_bitmap0 =
                values["devInfo"]["genDevInfo"]["existingPorts"]["ports"][0]
            device_exisitg_ports_bitmap1 =
                values["devInfo"]["genDevInfo"]["existingPorts"]["ports"][1]
            device_exisitg_ports_bitmap2 =
                values["devInfo"]["genDevInfo"]["existingPorts"]["ports"][2]
            device_exisitg_ports_bitmap3 =
                values["devInfo"]["genDevInfo"]["existingPorts"]["ports"][3]

            supportedSip = values["devInfo"]["genDevInfo"]["supportedSipBmp"]

        elseif 0x10 == result then
            command_data:setFailStatus()
            command_data:addWarning("Exisitg ports bitmap getting is not " ..
                                    "allowed on device %d.", devNum)
        elseif    0 ~= result then
            command_data:setFailStatus()
            command_data:addError("Error at exisitg ports bitmap getting " ..
                                  "on device %d: %s.", devNum,
                                  returnCodes[result])
        end

        if 0 == result then
        device_exisitg_ports_bitmap0_string =
            string.format("%x", device_exisitg_ports_bitmap0)
            device_exisitg_ports_bitmap1_string =
                string.format("%x", device_exisitg_ports_bitmap1)
            device_exisitg_ports_bitmap2_string =
                string.format("%x", device_exisitg_ports_bitmap2)
            device_exisitg_ports_bitmap3_string =
                string.format("%x", device_exisitg_ports_bitmap3)
        else
            device_exisitg_ports_bitmap0_string = "n/a"
            device_exisitg_ports_bitmap1_string = "n/a"
            device_exisitg_ports_bitmap2_string = "n/a"
            device_exisitg_ports_bitmap3_string = "n/a"
        end
    end

    command_data:updateStatus()

    -- Device exisitg ports bitmap string formatting and adding.
    command_data:setResultStr(device_exisitg_ports_bitmap0_string_prefix,
                              device_exisitg_ports_bitmap0_string,
                              device_exisitg_ports_bitmap0_string_suffix)
    command_data:addResultToResultArray()
    command_data:setResultStr(device_exisitg_ports_bitmap1_string_prefix,
                              device_exisitg_ports_bitmap1_string,
                              device_exisitg_ports_bitmap1_string_suffix)
    command_data:addResultToResultArray()
    command_data:setResultStr(device_exisitg_ports_bitmap2_string_prefix,
                              device_exisitg_ports_bitmap2_string,
                              device_exisitg_ports_bitmap2_string_suffix)
    command_data:addResultToResultArray()
    command_data:setResultStr(device_exisitg_ports_bitmap3_string_prefix,
                              device_exisitg_ports_bitmap3_string,
                              device_exisitg_ports_bitmap3_string_suffix)
    command_data:addResultToResultArray()

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStr(header_string, command_data["result"],
                              footer_string)

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

	return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  show_capabilities_device_func_all_devices
--        @description  shows system capabilities for all avaible devices
--
--        @param command_data   - command execution data object
--
--        @return       true on success, otherwise false and error message
--
local function show_capabilities_device_func_all_devices(command_data)
    -- Common variables declaration
    local res = true
    local val = nil
    local result, values
    local devNum

    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getAllAvailableDevicesIterator() do
            result, values = show_capabilities_device_func_given_device( command_data, devNum,
                                                                         "Device " .. tostring(devNum) .. ":")
            if result ~= true then
                res = result
                val = values
            end
        end
    end

    return res, val
end


-- ************************************************************************
---
--  show_capabilities_device_func
--        @description  shows of version information for the system
--
--        @param params         - params["all"]: property of all avaible
--                                device capabilities showing (alternative
--                                to params["devID"]);
--                                params["devID"]: checked device id
--                                (alternative to params["all"])
--
--        @return       true on success, otherwise false and error message
--
local function show_capabilities_device_func(params)
    -- Common variables declaration
    local result, values
    local command_data = Command_Data()

    -- Common variables initialization.
    command_data:clearResultArray()

    if params["all_device"] ~= "all" then
        result, values =
            show_capabilities_device_func_given_device(command_data, params["all_device"])
    else
        result, values =
            show_capabilities_device_func_all_devices(command_data)
    end

    return result, values
end


--------------------------------------------------------------------------------
-- command registration: show capabilities
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show capabilities", {
  func   = show_capabilities_device_func,
  help   = "Displaing of system capabilities",
  params = {
      { type = "named",
          "#all_device",
          mandatory = { "all_device" }
      }
  }
})
