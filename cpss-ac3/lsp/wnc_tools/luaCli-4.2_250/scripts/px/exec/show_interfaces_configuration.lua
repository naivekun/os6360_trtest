--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_configuration.lua
--*
--* DESCRIPTION:
--*       showing of interfaces configuration
--*
--* FILE REVISION NUMBER:
--*       $Revision: 9 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_interfaces_configuration
--        @description  shows interfaces configuration
--
--        @param params         - params["all"]: all devices port or all
--                                ports of given device cheking switch,
--                                could be irrelevant;
--                                params["devID"]: checked device number,
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil;
--                                params["port-channel"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_configuration(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local port_speed_autonegotiation, port_speed_autonegotiation_string
    local duplex_autonegotiation, duplex_autonegotiation_string
    local flow_control_autonegatiation, flow_control_autonegatiation_string
    local flow_control, flow_control_string
    local pause_advertise, pause_advertise_string
    local maximum_received_packet_size, maximum_received_packet_size_string
    local mac_addresses_learning, mac_addresses_learning_string
    local port_type, port_type_string, crc_check, crc_check_string
    local port_interface_mode, port_interface_mode_string
    local port_speed, port_speed_string
    local header_string, footer_string
    local direction
    local hash_in_dsa

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)

    -- Command specific variables initialization
    header_string =
        "\n" ..
        "Port  MAC Speed  MAC Duplex     FC      FC   Pause       MRU   CRC         Mode        Speed\n" ..
        "      Auto-Neg   Auto-Neg    Auto-Neg        Advertise                                      \n" ..
        "----- ---------- ----------- --------- ----- ---------  -----  ----  ----------------  -----\n"
    footer_string = "\n"

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string,
                                                     footer_string)

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            -- Port speed autonegatiation mode getting.
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssPxPortSpeedAutoNegEnableGet",
                                                     devNum, portNum, "state",
                                                     "GT_BOOL")
                if        0 == result then
                    port_speed_autonegotiation = values["state"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Speed auto-negatiation check is not allowed " ..
                                            "in device %d port %d.", devNum,
                                            portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at speed auto-negatiation mode getting " ..
                                          "of device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end

                if 0 == result then
                    port_speed_autonegotiation_string = boolStrGet(port_speed_autonegotiation)
                else
                    port_speed_autonegotiation_string = "n/a"
                end
            end

            -- Port duplex autonegatiation mode getting.
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssPxPortDuplexAutoNegEnableGet",
                                                     devNum, portNum, "state",
                                                     "GT_BOOL")
                if        0 == result then
                    duplex_autonegotiation = values["state"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Duplex auto-negatiation check is not allowed " ..
                                            "in device %d port %d.", devNum,
                                            portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at duplex auto-negatiation mode getting " ..
                                          "of device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end

                if 0 == result then
                    duplex_autonegotiation_string = boolStrGet(duplex_autonegotiation)
                else
                    duplex_autonegotiation_string = "n/a"
                end
            end

            -- Port flow-control autonegatiation mode and pause advertising getting.
            if true == command_data["local_status"] then
                result, values =
                    myGenWrapper("cpssPxPortFlowCntrlAutoNegEnableGet",
                                 {{ "IN",  "GT_SW_DEV_NUM", "dev",  devNum},
                                  { "IN",  "GT_U8",    "port", portNum  },
                                  { "OUT", "GT_BOOL",  "state"          },
                                  { "OUT", "GT_BOOL",  "pauseAdvertise" }})
                if        0 == result then
                    flow_control_autonegatiation    = values["state"]
                    pause_advertise                 = values["pauseAdvertise"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Flow-control autonegatiation mode and pause " ..
                                            "advertising are not allowed in device %d port %d.",
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at flow-control autonegatiation mode " ..
                                          "and pause advertising getting of device %d port %d: %s",
                                          devNum, portNum, returnCodes[result])
                end

                if 0 == result then
                    flow_control_autonegatiation_string = boolStrGet(flow_control_autonegatiation)
                    pause_advertise_string              = boolStrGet(pause_advertise)
                else
                    flow_control_autonegatiation_string = "n/a"
                    pause_advertise_string              = "n/a"
                end
            end

            -- Port flow-control getting.
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssPxPortFlowControlEnableGet",
                                                     devNum, portNum, "state",
                                                     "CPSS_PORT_FLOW_CONTROL_ENT")
                if        0 == result then
                    flow_control = values["state"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Flow-control getting is not allowed in " ..
                                            "device %d port %d.", devNum,
                                            portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at flow-control getting of device %d " ..
                                          "port %d: %s", devNum, portNum,
                                          returnCodes[result])
                end

                if 0 == result then
                    if     "CPSS_PORT_FLOW_CONTROL_RX_TX_E"   == flow_control then
                        flow_control_string = "Yes"
                    elseif "CPSS_PORT_FLOW_CONTROL_DISABLE_E" == flow_control then
                        flow_control_string = "No"
                    elseif "CPSS_PORT_FLOW_CONTROL_RX_ONLY_E" == flow_control then
                        flow_control_string = "Rx"
                    elseif "CPSS_PORT_FLOW_CONTROL_TX_ONLY_E" == flow_control then
                        flow_control_string = "Tx"
                    else
                        flow_control_string = "n/a"
                    end
                else
                    flow_control_string = "n/a"
                end
            end

            -- Maximum received packet size getting.
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssPxPortMruGet",
                                                     devNum, portNum, "mru")
                if        0 == result then
                    maximum_received_packet_size = values["mru"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Maximum received packet size getting is " ..
                                            "not allowed in device %d port %d.",
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at maximum received packet size getting " ..
                                          "of device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end

                if 0 == result then
                    maximum_received_packet_size_string =
                        tostring(maximum_received_packet_size)
                else
                    maximum_received_packet_size_string = "n/a"
                end
            end
--[[
            -- Cascade port type setting.
            if true == command_data["local_status"] then
                direction = 2
                result, values =
                    myGenWrapper("cpssDxChCscdPortTypeGet",
                                 {{ "IN",  "GT_U8",    "dev",  devNum   },
                                  { "IN",  "GT_U8",    "port", portNum  },
                                  { "IN",  "CPSS_PORT_DIRECTION_ENT",  "portDirection", direction},
                                  { "OUT", "CPSS_CSCD_PORT_TYPE_ENT",  "portType" }})
                if        0 == result then
                    port_type = values["portType"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to check DSA " ..
                                            "mode check on device %d port %d.",
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at DSA mode checking on " ..
                                          "device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end

                if 0 == result then
                    port_type_string    = cascadePortTypeStrGet(port_type)
                else
                    port_type_string = "n/a"
                end
            end

            -- hash in DSA
            if  command_data["local_status"] then
                result, values =
                    myGenWrapper("cpssDxChCscdHashInDsaEnableGet",
                                 {{ "IN",  "GT_U8",    "devNum",  devNum   },
                                  { "IN",  "GT_PHYSICAL_PORT_NUM",    "portNum", portNum  },
                                  { "IN",  "CPSS_PORT_DIRECTION_ENT",  "direction", "CPSS_PORT_DIRECTION_RX_E"},
                                  { "OUT", "GT_BOOL",  "enablePtr" }})

                local isErr = (0 ~= result)
                hash_in_dsa = (isErr and "n/a") or  (values.enablePtr and "Rx")

                if 0x1E == result then -- not applicable
                    command_data:setFailPortStatus()
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("ERROR: cpssDxChCscdHashInDsaEnableGet FAILED")
                else
                    result, values =
                        myGenWrapper("cpssDxChCscdHashInDsaEnableGet",
                                 {{ "IN",  "GT_U8",    "devNum",  devNum   },
                                  { "IN",  "GT_PHYSICAL_PORT_NUM",    "portNum", portNum  },
                                  { "IN",  "CPSS_PORT_DIRECTION_ENT",  "direction", "CPSS_PORT_DIRECTION_TX_E"},
                                  { "OUT", "GT_BOOL",  "enablePtr" }})

                    if 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("ERROR: cpssDxChCscdHashInDsaEnableGet FAILED")
                        hash_in_dsa = "n/a"
                    elseif  (hash_in_dsa == "Rx") then
                        hash_in_dsa = values.enablePtr and "Rx/Tx" or "Tx"
                    else
                        hash_in_dsa = values.enablePtr and "Tx" or "No"
                    end
                end
            end
]]--
            -- CRC check getting.
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssPxPortCrcCheckEnableGet",
                                                     devNum, portNum, "enable",
                                                     "GT_BOOL")
                if        0 == result then
                    crc_check = values["enable"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("CRC check is not allowed in device %d " ..
                                            "port %d.", devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at crc check getting of device %d " ..
                                          "port %d: %s", devNum, portNum,
                                          returnCodes[result])
                end

                if 0 == result then
                    if true == crc_check then
                        crc_check_string = "Yes"
                    else
                        crc_check_string = "No"
                    end
                else
                    crc_check_string = "n/a"
                end
            end

            -- Port interface mode getting.
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssPxPortInterfaceModeGet",
                                                     devNum, portNum, "mode",
                                                     "CPSS_PORT_INTERFACE_MODE_ENT")
                if        0 == result then
                    port_interface_mode = interfaceStrGet(values["mode"])
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Port interface mode getting is not allowed in device %d " ..
                                            "port %d.", devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port interface mode getting of device %d " ..
                                          "port %d: %s", devNum, portNum,
                                          returnCodes[result])
                end

                if 0 == result then
                    port_interface_mode_string = port_interface_mode
                else
                    port_interface_mode_string = "n/a"
                end
            end

            -- Port speed getting.
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssPxPortSpeedGet",
                                                     devNum, portNum, "speed",
                                                     "CPSS_PORT_SPEED_ENT")
                if        0 == result then
                    port_speed_string, port_speed = speedStrGet(values["speed"])
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Port speed getting is not allowed in device %d " ..
                                            "port %d.", devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port speed getting of device %d " ..
                                          "port %d: %s", devNum, portNum,
                                          returnCodes[result])
                end

                if 0 ~= result then
                    port_speed_string = "n/a"
                end
            end

            command_data:updateStatus()

            -- Resulting string formatting and adding.
            command_data["result"] =
                string.format("%-5s %-10s %-11s %-9s %-5s %-9s  %-5s  %-4s  %-16s  %-5s",
                              tostring(portNum), port_speed_autonegotiation_string,
                              duplex_autonegotiation_string,
                              flow_control_autonegatiation_string,
                              alignLeftToCenterStr(flow_control_string, 5),
                              pause_advertise_string,
                              alignLeftToCenterStr(maximum_received_packet_size_string,5),
                              alignLeftToCenterStr(crc_check_string, 4),
                              alignLeftToCenterStr(port_interface_mode_string, 16),
                              alignLeftToCenterStr(port_speed_string, 5))
            command_data:addResultToResultArray()

            command_data:updatePorts()
        end
    end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnPortCount(header_string, command_data["result"],
                                         footer_string,
                                         "There is no port information to show.\n")

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

        return command_data:getCommandExecutionResults()
end


-------------------------------------------------------
-- command registration: show interfaces configuration
-------------------------------------------------------
CLI_addHelp("exec", "show interfaces", "interfaces")
CLI_addCommand("exec", "show interfaces configuration", {
    func = show_interfaces_configuration,
    help = "Interface configuration",
    params = {
        { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
        }
    }
})
