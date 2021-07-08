--/*******************************************************************************
--*              (c), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--*/
--/**
--********************************************************************************
--* @PX_cpu_send.lua
--*
--* @brief CPSS PX implementation for packet hash calculation.
--* 
--* @version   1
--********************************************************************************
--*/

local payload
local expectedCounters = {
 {[0]=1, [1]=1, [2]=1,  [3]=0,  [4]=0,  [5]=0,  [6]=0,  [7]=0, 
  [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0},
 {[0]=0, [1]=0, [2]=0,  [3]=0,  [4]=1,  [5]=1,  [6]=0,  [7]=0, 
  [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0}
}

-- ************************************************************************
--
--  checkUnicastPacketCounters
--        @description  Check unicast counters
--
--        @param device            - device number
--        @param ports             - list of ports, can be nil
--
local function checkUnicastPacketCounters(expected)
    local port
    for port=0,15 do
        local ret, val = myGenWrapper("cpssPxPortMacCountersOnPortGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", port },
            { "OUT", "CPSS_PX_PORT_MAC_COUNTERS_STC", "countersPtr" }
        })
        if (ret ~= LGT_OK) then
            printLog("ERROR: cpssPxPortMacCountersOnPortGet failed: " .. returnCodes[ret])
            setFailState()
        end
        if expected ~= nil then
            if val.countersPtr.mibCounter[15].l[0] ~= expected[port] then
                printLog (string.format("Unexpected counters got %d, expected %d on port %d",
                    val.countersPtr.mibCounter[15].l[0], expected[port], port))
                setFailState()
            end
        end
    end
end

-- ************************************************************************
--
--  execute_send
--        @description  Execute send command
--
--        @param device            - device number
--        @param ports             - list of ports, can be nil
--
function execute_send(device, ports)
    local i, port
    if ports == nil then
        executeStringCliCommands(string.format("end\ntraffic\nsend device %d data %s", device, payload))
    else
        port_list = ""
        for i, port in pairs(ports) do
            port_list = port_list .. tostring(port) ..","
        end
        port_list = string.sub(port_list, 1, -2)
        executeStringCliCommands(string.format("end\ntraffic\nsend port %d/%s data %s", device, port_list, payload))
    end
end


function main()
    executeLocalConfig(luaTgfBuildConfigFileName("PX_cpu_send"))
    local ret, payloads = pcall(dofile, "px/examples/packets/PX_ipfiltering.lua")
    if not ret then
        printLog ("Error reading packets")
        setFailState()
        return
    end
    payload = payloads["ipv4_match_packet"]

    execute_send(0, nil)
    checkUnicastPacketCounters(nil)
    execute_send(0, {0,1,2})
    checkUnicastPacketCounters(expectedCounters[1])
    execute_send(0, {4,5})
    checkUnicastPacketCounters(expectedCounters[2])
    executeLocalConfig(luaTgfBuildConfigFileName("PX_cpu_send_deconfig"))
end

main()