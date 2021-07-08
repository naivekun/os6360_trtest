--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_mac_pdu.lua
--*
--* DESCRIPTION:
--*       Show IEEE Reserved packets
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local function show_mac_pdu_function(params)
    local devlist
    local pdu_protocols = {
        {value = 0x0,  protocol = "bpdu",         descr = "802.1Q BPDU DA=*-00"},
        {value = 0x2,  protocol = "lacp",         descr = "802.3 Slow Protocols DA=*-02"},
        {value = 0x3,  protocol = "pae",          descr = "802.1X PAE address DA=*-03"},
        {value = 0xE,  protocol = "lldp",         descr = "802.1AB LLDP DA=*-0E"},
        {value = 0x20, protocol = "gmrp",         descr = "802.1D GMRP DA=*-20"},
        {value = 0x21, protocol = "gvrp",         descr = "802.1Q GVRP DA=*-21"},
        {value = 0x8,  protocol = "providerBPDU", descr = "802.1Q Provider BPDU DA=*-08"},
        {value = 0xD,  protocol = "providerGVRP", descr = "802.1Q Bridge GVRP DA=*-0D"}
    }

    local packet_cmd = {
        ["CPSS_PACKET_CMD_FORWARD_E"]           = "forward",
        ["CPSS_PACKET_CMD_MIRROR_TO_CPU_E"]     = "mirror_to_cpu",
        ["CPSS_PACKET_CMD_TRAP_TO_CPU_E"]       = "trap_to_cpu",
        ["CPSS_PACKET_CMD_DROP_HARD_E"]         = "drop",
        ["CPSS_PACKET_CMD_DROP_SOFT_E"]         = "drop",
        ["CPSS_PACKET_CMD_ROUTE_E"]             = "route",
        ["CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E"]  = "route_and_mirror",
        ["CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E"] = "bridge_and_mirror",
        ["CPSS_PACKET_CMD_BRIDGE_E"]            = "bridge",
        ["CPSS_PACKET_CMD_NONE_E"]              = "none",
        ["CPSS_PACKET_CMD_LOOPBACK_E"]          = "loopback"
    }

    print("Dev   Protocol    Descript DA=01-80-C2-00-00-XX  Packet command")
    print("--- ------------ ------------------------------- --------------")
    
    if params.devID ~= "all" then
        devlist = {params.devID}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    local i, devNum, pri
    local values
    for i, devNum in pairs(devlist) do
        for pri=1,#pdu_protocols do
            local result, trap_cmd
            local profileIndex = 0
                result, values = myGenWrapper(
                    "cpssDxChBrgGenIeeeReservedMcastProtCmdGet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN", "GT_U32", "profileIndex", profileIndex},
                        { "IN", "GT_U8", "protocol", pdu_protocols[pri]["value"]},
                        { "OUT", "CPSS_PACKET_CMD_ENT", "trap_cmd"}
                    })
            if  result~=0 then
                print("cpssDxChBrgGenIeeeReservedMcastProtCmdGet()return an error code.")
            end

            print(string.format("%-4s%-14s%-31.30s%-14s",
                  to_string(devNum),
                  pdu_protocols[pri]["protocol"],
                  pdu_protocols[pri]["descr"],
                  packet_cmd[values["trap_cmd"]]
            ))
        end
    end

    return true
end

CLI_addCommand("exec", "show mac pdu", {
  func   = show_mac_pdu_function,
  help   = "Configure specific or entire IEEE Reserved packets",
  params = {
        {
        type = "named",
            { format = "device %devID_all",   name="devID",   help="The device number" },
            mandatory = {"devID"}
        }
  }
})