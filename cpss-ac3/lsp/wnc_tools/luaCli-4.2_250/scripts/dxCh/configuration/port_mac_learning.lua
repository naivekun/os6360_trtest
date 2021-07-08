--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_mac_learning.lua
--*
--* DESCRIPTION:
--*       enabling of MAC addresses learning
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  interface_port_mac_learning_func
--        @description  enables of MAC addresses learning of interface port
--
--        @param params         - params["flagNo"]: no-flag
--
--        @usage __global       - __global["ifRange"]: iterface range
--
--        @return       true on success, otherwise false and error message
--
local function interface_port_mac_learning_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local mac_address_lerning
    local cpu_forwarding_enabling
    local port_lock_command

    -- print("params " .. to_string(params))
    -- Command specific variables initialization
    if nil == params.flagNo then
        -- SET command default (without parameters)
        mac_address_lerning         = true
        cpu_forwarding_enabling     = true
        if params["controlled"] then
            mac_address_lerning     = false
        end
        if params["automatic"] then
            cpu_forwarding_enabling = false
        end
    else
        -- NO command
        mac_address_lerning             = false
        cpu_forwarding_enabling         = false
    end

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()

            -- Port port lock command getting.
            command_data:clearLocalStatus()

            if (true  == command_data["local_status"]) then
                result, values =
                    myGenWrapper("cpssDxChBrgFdbPortLearnStatusGet",
                                 {{ "IN",  "GT_U8",         "devNum",  devNum},
                                  { "IN",  "GT_U32",        "portNum", portNum},
                                  { "OUT", "GT_BOOL",       "status"},
                                  { "OUT", "CPSS_PORT_LOCK_CMD_ENT",
                                                            "cmd"}})
                if     0 == result then
                    port_lock_command = values["cmd"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Mac adress learning is not supported " ..
                                           "in device %d port %d.", devNum,
                                           portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port lock command getting " ..
                                          "of device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end
            end

            -- Port mac adress learning and port_lock command setting.
            if (true == command_data["local_status"]) then
                result, values =
                    myGenWrapper("cpssDxChBrgFdbPortLearnStatusSet",
                                 {{ "IN", "GT_U8",       "devNum",  devNum},
                                  { "IN", "GT_U32",      "portNum", portNum},
                                  { "IN", "GT_BOOL",     "status",  mac_address_lerning},
                                  { "IN", "CPSS_PORT_LOCK_CMD_ENT",
                                                         "cmd",     port_lock_command}})
                if     0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to %s mac adress " ..
                                            "learning for device %d port %d.",
                                            boolPredicatStrGet(mac_address_lerning),
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at mac adress learning setting " ..
                                          "of device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end
            end

            -- Setting of forwarding a new mac address message to CPU.
            if (true == command_data["local_status"]) then
                result, values =
                    cpssPerPortParamSet("cpssDxChBrgFdbNaToCpuPerPortSet",
                                        devNum, portNum,
                                        cpu_forwarding_enabling,
                                        "enable", "GT_BOOL")
                if     0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to %s " ..
                                            "forwarding a new mac address " ..
                                            "message to CPU on device " ..
                                            "%d port %d.",
                                            boolPredicatStrGet(cpu_forwarding_enabling),
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at %s of forwarding a new " ..
                                          "mac address message to CPU on " ..
                                          "device %d port %d: %s.",
                                          boolNounPredicatStrGet(cpu_forwarding_enabling),
                                          devNum, portNum, returnCodes[result])
                end
            end

            command_data:updateStatus()

            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts("Can not set mac adress learning " ..
                                                "for all processed ports.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: port-mac learning
--------------------------------------------
CLI_addHelp({"interface", "interface_eport"}, "port", "Perform port configuration")
CLI_addHelp({"interface", "interface_eport"}, "port-mac", "Port MAC address subcommands")
CLI_addCommand({"interface", "interface_eport"}, "port-mac learning", {
  func=interface_port_mac_learning_func,
  help="Enabling of MAC addresses learning and NA messages to CPU",
    params={
        { type = "named",
            {format = "controlled",
             help = "Learning done by CPU and based on handling of NA messages from device."},
            {format = "automatic",
             help = "Device learns MAC addresses automatically. NA messages to CPU disabled"},
            alt = {option = {"controlled", "automatic"}},
        }
    }
})
CLI_addHelp({"interface", "interface_eport"}, "no port", "Perform port configuration")
CLI_addHelp({"interface", "interface_eport"}, "no port-mac", "Port MAC address subcommands")
CLI_addCommand({"interface", "interface_eport"}, "no port-mac learning", {
  func=function(params)
      params.flagNo=true
      return interface_port_mac_learning_func(params)
  end,
  help="Disabling of MAC addresses learning and NA messages to CPU"
})

