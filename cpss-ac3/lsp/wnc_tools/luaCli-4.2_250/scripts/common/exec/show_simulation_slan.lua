--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_simulation_slan.lua
--*
--* DESCRIPTION:
--*       showing of simulation SLAN connections
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssIsAsicSimulation")
if not cli_C_functions_registered("wrlSimulationSlanGet") then
    function wrlSimulationSlanGet()
        return 0x10 -- GT_NOT_SUPPORTED
    end
end
--constants


-- ************************************************************************
---
--  show_simulation_slan
--        @description  show SLAN information of interfaces
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
local function show_simulation_slan(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local port_slan_name, port_is_connected
    local header_string, footer_string

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)

    -- Command specific variables initialization
    header_string =
        "\n" ..
        "Dev/Port         SLAN        \n" ..
        "---------  ----------------  \n"
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

            port_slan_name = "n/a"
            port_is_connected = false

            -- Port interface mode getting.
            if true == command_data["local_status"] then
                result, port_is_connected, port_slan_name  = wrLogWrapper("wrlSimulationSlanGet","(devNum, portNum)",devNum, portNum)
                if 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at slan" ..
                                          "getting of device %d port %d: %s",
                                          devNum, portNum, returnCodes[result])
                end
            end

            command_data:updateStatus()

            -- Resulting string formatting and adding.
            devPort=tostring(devNum).."/"..tostring(portNum)
            
            if (port_is_connected ~= false) then
            
                command_data["result"] =
                    string.format("%-11s%-18s", devPort, port_slan_name)

                command_data:addResultToResultArray()
            end
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


-------------------------------------------------
-- command registration: show interfaces status
-------------------------------------------------
CLI_addCommand("exec", "show simulation slan", {
    func = show_simulation_slan,
    help = "Simulation SLAN connection information",
    params = {
        { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
        }
    }
})
