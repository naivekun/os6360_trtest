--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mac_pdu.lua
--*
--* DESCRIPTION:
--*       Use the mac pdu in Global Configuration mode to trap, mirror, forward
--*       or drop specific or entire IEEE Reserved packets
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


--mac pdu protocol {bpdu | lacp| pae | lldp | gmrp | gvrp | providerBPDU | providerGVRP | all}  {trap | mirror | forward |drop} device {all | devNum}

local function mac_pdu_function(params)
    local command_data = Command_Data()
    local profileIndex = 0
    local iterator
    local result

    startAPILog("Start of <mac_pdu_function> (" .. to_string(params) .. ")")

    params.all = true
    if params.devID == "all" then
        params.devID = nil
    end

    command_data:initAllInterfacesPortIterator(params)

    if  params.pdu_protocols == "all" then -- ALL
        params.pdu_protocols = {}
        for i = 0,0xFF do
            table.insert(params.pdu_protocols, i)
        end
    else
        params.pdu_protocols = {params.pdu_protocols}
    end

   startLoopLog("for devNum, portNum")
   
   local lastdev = nil
   for iterator, devNum, portNum in command_data:getPortIterator() do
        if devNum ~= lastdev then
            result = myGenWrapper(
                "cpssDxChBrgGenBpduTrapEnableSet", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_BOOL", "enable", 0}
                })
            command_data:handleCpssErrorDevPort(result, "cpssDxChBrgGenBpduTrapEnableSet()")

            result = myGenWrapper(
                "cpssDxChBrgGenIeeeReservedMcastTrapEnable", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_BOOL", "enable", 1}
                })
            command_data:handleCpssErrorDevPort(result, "cpssDxChBrgGenIeeeReservedMcastTrapEnable()")

            startLoopLog("for pdu_protocols")
            for i,pdu_protocols in pairs(params.pdu_protocols) do
                local result, enable

                result = myGenWrapper(
                    "cpssDxChBrgGenIeeeReservedMcastProtCmdSet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN", "GT_U32", "profileIndex", profileIndex},
                        { "IN", "GT_U8", "protocol", pdu_protocols},
                        { "IN", "CPSS_PACKET_CMD_ENT", "cmd", params.trap_cmd}
                })
                command_data:handleCpssErrorDevPort(result, "cpssDxChBrgGenIeeeReservedMcastProtCmdSet()")
            end
            stopLoopLog()
        end
        lastdev = devNum

        result = myGenWrapper(
            "cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet", {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_PORT_NUM", "portNum", portNum},
                { "IN", "GT_U32", "profileIndex", profileIndex}
            })
        command_data:handleCpssErrorDevPort(result, "cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet()")
    end
    stopLoopLog()

    leaveAPILog("end of <mac_pdu_function>  ")

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


CLI_type_dict["trap_cmd"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Command for trap",
    enum = {
        ["trap"]      = {value="CPSS_PACKET_CMD_TRAP_TO_CPU_E",   help="Trap packet to CPU"},
        ["mirror"]    = {value="CPSS_PACKET_CMD_MIRROR_TO_CPU_E", help="Mirror packet to CPU"},
        ["forward"]   = {value="CPSS_PACKET_CMD_FORWARD_E",       help="Forward packet"},
        ["drop"]      = {value="CPSS_PACKET_CMD_DROP_SOFT_E",     help="Hard drop packet"}
   }
}

CLI_type_dict["pdu_protocols"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Pdu protocols",
    enum = {
        ["bpdu"]         = {value=0x0,  help="IEEE 802.1Q BPDU"},
        ["lacp"]         = {value=0x2,  help="IEEE 802.3 Slow Protocols"},
        ["pae"]          = {value=0x3,  help="IEEE 802.1X PAE"},
        ["lldp"]         = {value=0xE,  help="IEEE 802.1AB LLDP"},
        ["gmrp"]         = {value=0x20, help="IEEE 802.1D GMRP"},
        ["gvrp"]         = {value=0x21, help="IEEE 802.1Q GVRP"},
        ["providerBPDU"] = {value=0x8,  help="IEEE 802.1Q Provider BPDU"},
        ["providerGVRP"] = {value=0xD,  help="IEEE 802.1Q Provider Bridge GVRP"},
        ["all"]          = {value="all", help="All IEEE Reserved protocols"}
   }
}


CLI_addCommand("config", "mac pdu protocol", {
  func   = mac_pdu_function,
  help   = "Configure specific or entire IEEE Reserved packets",
  params = {
        { type = "values",
            { format = "%pdu_protocols", name="pdu_protocols", help="IEEE Reserved protocol" },
            { format = "%trap_cmd", name="trap_cmd", help="Trap command" }
        },
        {
        type = "named",
            { format = "device %devID_all",   name="devID",   help="The device number" },
            mandatory = {"devID"}
        }
  }
})

local function no_mac_pdu_function(params)
    local command_data = Command_Data()
    local iterator
    local portNum
    local devNum
    local profileIndex = 0
    local result
    
    params.all=true

    startAPILog("Start of <no_mac_pdu> function (" .. to_string(params) .. ")")

    command_data:initAllInterfacesPortIterator(params)

   startLoopLog("for <devNum, portNum>")

    local lastdev = nil
    for iterator, devNum, portNum in command_data:getPortIterator() do
        if devNum ~= lastdev then
            result = myGenWrapper(
                "cpssDxChBrgGenBpduTrapEnableSet", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_BOOL", "enable", 1}
                })
            command_data:handleCpssErrorDevPort(result, "cpssDxChBrgGenBpduTrapEnableSet()")

            result = myGenWrapper(
                "cpssDxChBrgGenIeeeReservedMcastTrapEnable", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_BOOL", "enable", 0}
                })
            command_data:handleCpssErrorDevPort(result, "cpssDxChBrgGenIeeeReservedMcastTrapEnable()")
        end
        lastdev = devNum

        startLoopLog("for pdu_protocols")
        for pdu_protocols = 0, 0xFF do
            local result, enable

            result = myGenWrapper(
                "cpssDxChBrgGenIeeeReservedMcastProtCmdSet", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_U32", "profileIndex", profileIndex},
                    { "IN", "GT_U8", "protocol", pdu_protocols},
                    { "IN", "CPSS_PACKET_CMD_ENT", "cmd", "CPSS_PACKET_CMD_FORWARD_E"}
                })
            command_data:handleCpssErrorDevPort(result, "cpssDxChBrgGenIeeeReservedMcastProtCmdSet()")
        end
        stopLoopLog("end pdu_protocols loop")

        result = myGenWrapper(
            "cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet", {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_PORT_NUM", "portNum", portNum},
                { "IN", "GT_U32", "profileIndex", profileIndex}
            })
        command_data:handleCpssErrorDevPort(result, "cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet()")
    end
    stopLoopLog("end <devNum, portNum> loop")

    leaveAPILog("end of <no_mac_pdu>_function() ")

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

CLI_addCommand("config", "no mac pdu", {
  func   = no_mac_pdu_function,
  help   = "Disable mac pbu",
})