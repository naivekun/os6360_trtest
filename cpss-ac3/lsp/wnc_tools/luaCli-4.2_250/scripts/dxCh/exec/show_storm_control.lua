--
-- ************************************************************************
---
--  show_storm_control
--        @description  show_storm_control - Displays storm-control related configuration
--
--        @return       true on success, otherwise false and error message
--
local function show_storm_control(params)

    local did_error, result, values
    local enablePtr
    local command_data = Command_Data()

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)

    -- Command specific variables initialization
    header_string = "\n" ..
        "Port    Broadcast    Known    Unknown     Registered   Unregistered    TCP-SYN    Rate(64 bytes/packets)\n" ..
        "                     unicast  unicast     multicast    multicast                                        \n" ..
        "----    ---------    -------  -------     ----------   ------------    -------    ---------------------"
    footer_string = "\n"

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string, footer_string)

    for dummy, devNum, portNum in command_data:getPortIterator() do
        did_error, result, values = genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitGet", {
                                               {"IN", "GT_U8", "devNum", devNum},
                                               {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                                               {"OUT", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC", "portGfgPtr"}})
        local portGfgPtr = values["portGfgPtr"]
        did_error, result, values = genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitTcpSynGet", {
                                               {"IN", "GT_U8", "devNum", devNum},
                                               {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                                               {"OUT", "GT_BOOL", "enablePtr"}})
        local enablePtr = values["enablePtr"]
        command_data["result"] = string.format("%-7s %-12s %-8s %-10s %-13s %-15s %-10s %-9s",
                                               tostring(portNum),
                                               tostring(portGfgPtr.enableBc),
                                               tostring(portGfgPtr.enableUcKnown),
                                               tostring(portGfgPtr.enableMcReg),
                                               tostring(portGfgPtr.enableUcUnk),
                                               tostring(portGfgPtr.enableMc),
                                               tostring(enablePtr),
                                               tostring(portGfgPtr.rateLimit))
        command_data:addResultToResultArray()
        command_data:updatePorts()
    end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnPortCount(header_string, command_data["result"],
                                         footer_string, "There is no storm control information to show.\n")

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


-- exec: show storm_control device %devID
CLI_addCommand("exec", "show storm-control", {
    func = show_storm_control,
    help = "Show storm-control",
    params = {
        { type = "named",
        "#all_interfaces",
        mandatory = { "all_interfaces" }
    }
}
})
