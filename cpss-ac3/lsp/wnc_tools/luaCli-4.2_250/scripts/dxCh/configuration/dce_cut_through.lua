--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_cut_through.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of cut-through for a priority
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  dce_cut_through_func
--        @description  enables/disables of cut-through for a priority
--
--        @param params         - params["priority"]: 802.1Q Priority;
--                                params["all_device"]: property of
--                                applying of all avaible devices, could
--                                be inrelevant;
--                                params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function dce_cut_through_func(params)
    -- Common variables declaration.
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local priority_setting, priority, priority_enabling
    local packet_length_setting, packet_length

    -- Common variables initialization.
    command_data:initAllDeviceRange(params)

    -- Command specific variables initialization.
    priority_setting        = isEquivalent(params.packet_length_priority,
                                           "priority")
    priority                = params["priority"]
    priority_enabling       = command_data:getTrueIfFlagNoIsNil(params)
    packet_length_setting   = isEquivalent(params.packet_length_priority,
                                           "packet_length")
    packet_length           = params["packet_length"]

    -- Main port handling cycle.
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum, portNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     or
                (is_sip_5_20(devNum))     then
                -- Enabling/disabling of cut-through for a priority.
                if (true == command_data["local_status"])   and
                   (true == priority_setting)               then
                    result, values =
                        myGenWrapper("cpssDxChCutThroughUpEnableSet",
                                     {{"IN", "GT_U8",    "devNum",  devNum   },
                                      {"IN", "GT_U8",    "up",      priority },
                                      {"IN", "GT_BOOL",  "enable",
                                                           priority_enabling }})
                    if     0x10 == result   then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to %s " ..
                                                "of cut-through for a " ..
                                                "priority %d on device %d.",
                                                boolEnableLowerStrGet(
                                                    priority_enabling),
                                                priority, devNum)
                    elseif    0 ~= result   then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at %s of cut-through " ..
                                              "for a priority %d on device " ..
                                              "%d: %s.",
                                              boolEnablingLowerStrGet(
                                                priority_enabling),
                                              devNum,
                                              returnCodes[result])
                    end
                end

               -- Setting of the default packet length.
                if (true == command_data["local_status"])   and
                   (true == packet_length_setting)          then
                    command_data:setFailDeviceStatus()
                command_data:addWarning("Command will be implementer later")
                end
            else
                command_data:setFailDeviceStatus()
                command_data:addWarning("Family of device %d does not " ..
                                        "supported.", devNum)
            end

            command_data:updateStatus()

            command_data:updateDevices()
        end

        command_data:addWarningIfNoSuccessDevices(
            "Can not configure cut-through for a priority on all processed " ..
            "devices.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: dce cut-through
--------------------------------------------------------------------------------
CLI_addCommand("config", "dce cut-through", {
  func   = dce_cut_through_func,
  help   = "Enabling of cut-through for a priority",
  params = {
      { type = "named",
          "#packet_length",
          "#priority_enable",
          "#all_device",
        alt       = { packet_length_priority    = { "packet_length",
                                                    "priority"              }},
        mandatory = { "packet_length_priority", "all_device" }
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no dce cut-through
--------------------------------------------------------------------------------
CLI_addCommand("config", "no dce cut-through", {
  func   = function(params)
               params.flagNo                    = true
               params.packet_length_priority    = "priority"
               dce_cut_through_func(params)
           end,
  help   = "Disable of cut-through for a priority",
  params = {
      { type = "named",
          "#priority_enable",
          "#all_device",
        mandatory = { "priority", "all_device" }
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no dce cut-through packet-length
--------------------------------------------------------------------------------
CLI_addCommand("config", "no dce cut-through packet-length", {
  func   = function(params)
               params.flagNo                    = true
               params.packet_length_priority    = "packet_length"
               dce_cut_through_func(params)
           end,
  help   = "Returning to default of the default packet length that is " ..
           "assigned to a packet in the Cut-Through mode",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})

-- ************************************************************************
---
--  dce_cut_through_ude_func
--        @description  enables/disables user definded ethernet cut-through mode
--
--        @param params         - params["all_devices"]: property of
--                                applying of all avaible devices, could
--                                be inrelevant;
--                                params["ude-index"]:
--                                params["flagNo"]: no command property
--                                params["byte-count"]:
--
--        @return       true on success, otherwise false and error message
--
local function dce_cut_through_ude_func(params)
    -- Common variables declaration.
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local ude_index, ude_enable, byte_count

    -- Common variables initialization.
    command_data:initAllDeviceRange(params)

    -- Command specific variables initialization.
    ude_enable           = command_data:getTrueIfFlagNoIsNil(params)
    ude_index            = params["ude-index"];
    byte_count           = (params["byte-count"] or 0);
    -- Main port handling cycle.
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            if (is_sip_6(devNum))     then
                if (true == command_data["local_status"])   then
                    result, values =
                        myGenWrapper("cpssDxChCutThroughUdeCfgSet",
                         {{"IN", "GT_U8",    "devNum",               devNum   },
                          {"IN", "GT_U32",   "udeIndex",             ude_index },
                          {"IN", "GT_BOOL",  "udeCutThroughEnable",  ude_enable },
                          {"IN", "GT_U32",   "udeByteCount",         byte_count }
                             })
                    if     0x10 == result   then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning(
                            "It is not allowed to %s of cut-through for a " ..
                            "ude index %d with byte count %d on device %d.",
                            boolEnableLowerStrGet(ude_enable),
                            ude_index, byte_count, devNum)
                    elseif    0 ~= result   then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError(
                            "It is not allowed to %s of cut-through for a " ..
                            "ude index %d with byte count %d on device %d. %s",
                            boolEnableLowerStrGet(ude_enable),
                            ude_index, byte_count, devNum, returnCodes[result])
                    end
                end
            else
                command_data:setFailDeviceStatus()
                command_data:addWarning("Family of device %d does not " ..
                                        "supported.", devNum)
            end

            command_data:updateStatus()

            command_data:updateDevices()
        end

        command_data:addWarningIfNoSuccessDevices(
            "Can not configure cut-through UDE for a priority on all processed " ..
            "devices.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- type registration: cut_throug_ude_index
--------------------------------------------
CLI_type_dict["cut_throug_ude_index"] = {
    checker = CLI_check_param_number,
    min=0,
    max=3,
    complete = CLI_complete_param_number,
    help = "Cut Through User Defined Ethernet index"
}

--------------------------------------------
-- type registration: cut_throug_byte_count
--------------------------------------------
CLI_type_dict["cut_throug_byte_count"] = {
    checker = CLI_check_param_number,
    min=0,
    max=0x3FFF,
    complete = CLI_complete_param_number,
    help = "Cut Through packet byte count"
}

--------------------------------------------------------------------------------
-- command registration: dce cut-through ude
--------------------------------------------------------------------------------
CLI_addCommand("config", "dce cut-through ude", {
    func   = dce_cut_through_ude_func,
    help   = "Enabling of cut-through for UDE",
    params = {
        { type = "named",
            "#all_device",
            { format = "ude-index %cut_throug_ude_index", name = "ude-index"},
            { format = "byte-count %cut_throug_byte_count", name = "byte-count"},
            mandatory = { "byte-count" },
            requirements =
            {
                ["byte-count"] = {"ude-index"},
                ["ude-index"] = {"all_device"}
            }
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: no dce cut-through ude
--------------------------------------------------------------------------------
CLI_addCommand("config", "no dce cut-through ude", {
    func   = function(params)
               params.flagNo                    = true
               dce_cut_through_ude_func(params)
           end,
    help   = "Disable of cut-through for UDE",
    params = {
      { type = "named",
          "#all_device",
          { format = "ude-index %cut_throug_ude_index", name = "ude-index"},
          mandatory = { "ude-index" },
          requirements =
          {
              ["ude-index"] = {"all_device"}
          }
      }
    }
})

-- ************************************************************************
---
--  dce_cut_through_passenger_parsing_func
--        @description  set cut through passenger packet parsing mode
--
--        @param params         - params["parsing-mode"]:
--                              - params["all_devices"]: property of
--                                applying of all avaible devices, could
--                                be inrelevant;
--
--        @return       true on success, otherwise false and error message
--
local function dce_cut_through_passenger_parsing_func(params)
    -- Common variables declaration.
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local parsing_mode

    -- Common variables initialization.
    command_data:initAllDeviceRange(params)

    -- Command specific variables initialization.
    parsing_mode = params["parsing-mode"]
    -- Main port handling cycle.
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            if (is_sip_6(devNum))     then
                if (true == command_data["local_status"])   then
                    result, values =
                        myGenWrapper("cpssDxChCutThroughPassengerParsingModeSet",
                         {{"IN", "GT_U8",    "devNum",               devNum   },
                          {"IN", "CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_MODE_ENT",
                              "parsingMode", parsing_mode }
                             })
                    if     0x10 == result   then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning(
                            "It is not allowed to set cut-through " ..
                            "passenger packet parsing mode " ..
                            "on device %d.",
                            devNum)
                    elseif    0 ~= result   then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError(
                            "It is not allowed to set cut-through " ..
                            "passenger packet parsing mode " ..
                            "on device %d. %s",
                            devNum, returnCodes[result])
                    end
                end
            else
                command_data:setFailDeviceStatus()
                command_data:addWarning("Family of device %d does not " ..
                                        "supported.", devNum)
            end

            command_data:updateStatus()

            command_data:updateDevices()
        end

        command_data:addWarningIfNoSuccessDevices(
            "Can not set cut-through passenger packet parsing mode on all processed " ..
            "devices.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- type registration: cut_throug_passenger_parsing_mode
--------------------------------------------
CLI_type_dict["cut_throug_passenger_parsing_mode"] = {
    checker     = CLI_check_param_enum,
    complete    = CLI_complete_param_enum,
    help        = "cut through passenger parsing packet headers mode",
    enum = {
        ["byte-count-only"] = {
            value = "CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_BYTE_COUNT_ONLY_E",
            help  = "use calculated Packet Byte Count only" },
        ["all-data"] = {
            value = "CPSS_DXCH_CUT_THROUGH_PASSENGER_PARSING_ALL_DATA_E",
            help  = "use calculated Packet Byte Count, QoS, Hash and PCL UDB’s" }
    }
}

--------------------------------------------------------------------------------
-- command registration: dce cut-through passenger-parsing
--------------------------------------------------------------------------------
CLI_addCommand("config", "dce cut-through passenger-parsing", {
    func   = dce_cut_through_passenger_parsing_func,
    help   = "set cut through passenger packet parsing mode",
    params = {
        { type = "values",
            { format = "%cut_throug_passenger_parsing_mode", name = "parsing-mode"}
        },
        { type = "named",
            "#all_device",
            mandatory = { "all_device" }
        }
    }
})

-- ************************************************************************
---
--  dce_cut_through_hdr_integrity_func
--        @description  Set Packet Header Integrity Check configuration
--
--        @param params         - params["all_devices"]: property of
--                                applying of all avaible devices, could
--                                be inrelevant;
--                                params["flagNo"]: no command property
--                                parameters below relevant only when params["flagNo"] == false
--                                params["bc-boundaries-check"]:
--                                params["bc-boundaries-low"]:
--                                params["bc-boundaries-high"]:
--                                params["ipv4-hdr-cs-check"]:
--                                params["pkt-cmd"]:
--                                params["ud-cpu-code"]:
--
--        @return       true on success, otherwise false and error message
--
local function dce_cut_through_hdr_integrity_func(params)
    -- Common variables declaration.
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local enable, cfg
    local bc_boundaries_check, bc_boundaries_low, bc_boundaries_high
    local ipv4_hdr_cs_check, pkt_cmd, ud_cpu_code
    local CPSS_NET_FIRST_USER_DEFINED_E = 500

    -- Common variables initialization.
    command_data:initAllDeviceRange(params)

    -- Command specific variables initialization.
    enable              = command_data:getTrueIfFlagNoIsNil(params)
    if enable then
        bc_boundaries_check = params["bc-boundaries-check"]
        bc_boundaries_low   = params["bc-boundaries-low"]
        bc_boundaries_high  = params["bc-boundaries-high"]
        ipv4_hdr_cs_check   = params["ipv4-hdr-cs-check"]
        pkt_cmd             = params["pkt-cmd"]
        ud_cpu_code         = CPSS_NET_FIRST_USER_DEFINED_E + params["ud-cpu-code"]
    else
        bc_boundaries_check = false
        bc_boundaries_low   = 0
        bc_boundaries_high  = 0
        ipv4_hdr_cs_check   = false
        pkt_cmd             = "CPSS_PACKET_CMD_FORWARD_E"
        ud_cpu_code         = CPSS_NET_FIRST_USER_DEFINED_E
    end

    cfg =
    {
        enableByteCountBoundariesCheck = bc_boundaries_check,
        minByteCountBoundary           = bc_boundaries_low,
        maxByteCountBoundary           = bc_boundaries_high,
        enableIpv4HdrCheckByChecksum   = ipv4_hdr_cs_check,
        hdrIntergrityExceptionPktCmd   = pkt_cmd,
        hdrIntergrityExceptionCpuCode  = ud_cpu_code
    }

    -- Main port handling cycle.
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            if (is_sip_6(devNum))     then
                if (true == command_data["local_status"])   then
                    result, values =
                        myGenWrapper("cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet",
                         {{"IN", "GT_U8",    "devNum",               devNum   },
                          {"IN", "CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC",
                              "cfgPtr",             cfg }
                             })
                    if     0x10 == result   then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning(
                            "It is not allowed to set cut-through " ..
                            "Packet Header Integrity Check configuration " ..
                            "on device %d.",
                            devNum)
                    elseif    0 ~= result   then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError(
                            "It is not allowed to set cut-through " ..
                            "Packet Header Integrity Check configuration " ..
                            "on device %d. %s",
                            devNum, returnCodes[result])
                    end
                end
            else
                command_data:setFailDeviceStatus()
                command_data:addWarning("Family of device %d does not " ..
                                        "supported.", devNum)
            end

            command_data:updateStatus()

            command_data:updateDevices()
        end

        command_data:addWarningIfNoSuccessDevices(
            "Can not set cut-through Packet Header Integrity Check configuration on all processed " ..
            "devices.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

--------------------------------------------
-- type registration: cut_throug_user_cpu_code
--------------------------------------------
CLI_type_dict["cut_throug_user_cpu_code"] = {
    checker = CLI_check_param_number,
    min=0,
    max=63,
    complete = CLI_complete_param_number,
    help = "Cut Through User CPU Code"
}

--------------------------------------------
-- type registration: cut_through_pkt_cmd
--------------------------------------------
CLI_type_dict["cut_through_pkt_cmd"] = {
checker = CLI_check_param_enum,
complete = CLI_complete_param_enum,
help = "cut through Packet Header Integrity Check packet command\n",
    enum = {
        ["forward"]       = { value="CPSS_PACKET_CMD_FORWARD_E", help="Forward packet" },
        ["mirror-to-cpu"] = { value="CPSS_PACKET_CMD_MIRROR_TO_CPU_E", help="Mirror packet to CPU" },
        ["trap-to-cpu"]   = { value="CPSS_PACKET_CMD_TRAP_TO_CPU_E", help="Trap packet to CPU" },
        ["soft-drop"]     = { value="CPSS_PACKET_CMD_DROP_SOFT_E", help="Soft Drop packet" },
        ["hard-drop"]     = { value="CPSS_PACKET_CMD_DROP_HARD_E", help="Hard Drop packet" }
   }
}

--------------------------------------------------------------------------------
-- command registration: dce cut-through header-integrity
--------------------------------------------------------------------------------
CLI_addCommand("config", "dce cut-through header-integrity", {
    func   = dce_cut_through_hdr_integrity_func,
    help   = "Set Packet Header Integrity Check configuration",
    params = {
        { type = "named",
            "#all_device",
            { format = "bc-boundaries-check %bool", name = "bc-boundaries-check"},
            { format = "bc-boundaries-low %cut_throug_byte_count", name = "bc-boundaries-low"},
            { format = "bc-boundaries-high %cut_throug_byte_count", name = "bc-boundaries-high"},
            { format = "ipv4-checksum-check %bool", name = "ipv4-hdr-cs-check"},
            { format = "packet-command %cut_through_pkt_cmd", name = "pkt-cmd"},
            { format = "ud-cpu-code %cut_throug_user_cpu_code", name = "ud-cpu-code"},
            mandatory = { "ud-cpu-code" },
            requirements =
            {
                ["ud-cpu-code"] = {"pkt-cmd"},
                ["pkt-cmd"] = {"ipv4-hdr-cs-check"},
                ["ipv4-hdr-cs-check"] = {"bc-boundaries-high"},
                ["bc-boundaries-high"] = {"bc-boundaries-low"},
                ["bc-boundaries-low"] = {"bc-boundaries-check"},
                ["bc-boundaries-check"] = {"all_device"}
            }
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: dce cut-through hdr-integrity
--------------------------------------------------------------------------------
CLI_addCommand("config", "no dce cut-through hdr-integrity", {
    func   = function(params)
               params.flagNo                    = true
               dce_cut_through_hdr_integrity_func(params)
           end,
    help   = "Reset Packet Header Integrity Check configuration",
    params = {
        { type = "named",
            "#all_device",
            mandatory = { "all_device" }
        }
    }
})
