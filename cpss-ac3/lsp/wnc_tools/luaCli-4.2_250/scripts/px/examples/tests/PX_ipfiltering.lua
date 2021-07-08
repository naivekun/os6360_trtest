--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_ipfiltering.lua
--*
--* DESCRIPTION:
--*     Example of packets IP filtering
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlPxRevisionA1AndAboveCheck")
cmdLuaCLI_registerCfunction("prvLuaTgfPxNetIfSdmaSyncTxPacketSend")

local port0 = devEnv.port[1]
local port1 = devEnv.port[2]
local port2 = devEnv.port[3]

function makeSinglePortBitmap(portIndex)
    return string.format("0x%05X", bit_shl(1, devEnv.port[portIndex]))
end

local expectedCounters = {
    ["ipv4_match_packet"] =   {[0]=0, [1]=0, [2]=0,  [3]=0,  [4]=0,  [5]=0,  [6]=0,  [7]=0, 
                               [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0},
    ["ipv4_nomatch_packet"] = {[0]=0, [1]=0, [2]=0,  [3]=0,  [4]=0,  [5]=0,  [6]=0,  [7]=0, 
                               [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0},
    ["ipv6_match_packet"] =   {[0]=0, [1]=0, [2]=0,  [3]=0,  [4]=0,  [5]=0,  [6]=0,  [7]=0, 
                               [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0},
    ["ipv6_nomatch_packet"] = {[0]=0, [1]=0, [2]=0,  [3]=0,  [4]=0,  [5]=0,  [6]=0,  [7]=0, 
                               [8]=0, [9]=0, [10]=0, [11]=0, [12]=0, [13]=0, [14]=0, [15]=0},
}

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
        if val.countersPtr.mibCounter[15].l[0] ~= expected[port] then
            printLog (string.format("Unexpected counters got %d, expected %d on port %d",
                val.countersPtr.mibCounter[15].l[0], expected[port], port))
            setFailState()
        end
    end
end

local function ipFilteringExecute(payloads)
    printLog("ipv4_match_packet")
    expectedCounters["ipv4_match_packet"][port0] = 1
    local status = prvLuaTgfPxNetIfSdmaSyncTxPacketSend(devEnv.dev, payloads["ipv4_match_packet"])
    if status ~= LGT_OK then
        printLog ("Failed to call prvLuaTgfPxNetIfSdmaSyncTxPacketSend")
        setFailState()
        return
    end
    delay(100)
    checkUnicastPacketCounters(expectedCounters["ipv4_match_packet"])

    printLog("ipv4_nomatch_packet")
    expectedCounters["ipv4_nomatch_packet"][port2] = 1
    local status = prvLuaTgfPxNetIfSdmaSyncTxPacketSend(devEnv.dev, payloads["ipv4_nomatch_packet"])
    if status ~= LGT_OK then
        printLog ("Failed to call prvLuaTgfPxNetIfSdmaSyncTxPacketSend")
        setFailState()
        return
    end
    delay(100)
    checkUnicastPacketCounters(expectedCounters["ipv4_nomatch_packet"])

    printLog("ipv6_match_packet")
    expectedCounters["ipv6_match_packet"][port1] = 1
    local status = prvLuaTgfPxNetIfSdmaSyncTxPacketSend(devEnv.dev, payloads["ipv6_match_packet"])
    if status ~= LGT_OK then
        printLog ("Failed to call prvLuaTgfPxNetIfSdmaSyncTxPacketSend")
        setFailState()
        return
    end
    delay(100)
    checkUnicastPacketCounters(expectedCounters["ipv6_match_packet"])

    printLog("ipv6_nomatch_packet")
    expectedCounters["ipv6_nomatch_packet"][port2] = 1
    local status = prvLuaTgfPxNetIfSdmaSyncTxPacketSend(devEnv.dev, payloads["ipv6_nomatch_packet"])
    if status ~= LGT_OK then
        printLog ("Failed to call prvLuaTgfPxNetIfSdmaSyncTxPacketSend")
        setFailState()
        return
    end
    delay(100)
    checkUnicastPacketCounters(expectedCounters["ipv6_nomatch_packet"])
end

local function ipFilteringTest()
    local status, applicable = wrlPxRevisionA1AndAboveCheck(devEnv.dev)
    if status ~= LGT_OK then
        printLog ("Failed to call wrlPxRevisionA1AndAboveCheck")
        setFailState()
        return
    end
    if applicable ~= true then
        printLog ("PIPE is not revision A1 and above")
        return
    end

    local ret, payloads = pcall(dofile, "px/examples/packets/PX_ipfiltering.lua")
    if not ret then
        printLog ("Error reading packets")
        setFailState()
        return
    end

    executeLocalConfig(luaTgfBuildConfigFileName("PX_ipfiltering"))

    ipFilteringExecute(payloads)

    executeLocalConfig(luaTgfBuildConfigFileName("PX_ipfiltering_deconfig"))
end

ipFilteringTest()
