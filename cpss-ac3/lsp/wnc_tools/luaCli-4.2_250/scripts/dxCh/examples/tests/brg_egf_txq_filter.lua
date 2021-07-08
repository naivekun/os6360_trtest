--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* brg_egf_txq_filter.lua
--*
--* DESCRIPTION:
--*       check bridge egress filtering
--*       send unknown UC packet in default VLAN 1 - check flooding on all ports in VLAN
--*         check egress MIB counters: outBcFrames == 3, other counters - 0  
--*       send known UC packet with learnt DA - check one packet egressed on destination port
--*         check egress MIB counters: outUcFrames == 1, other counters - 0  
--*       link down one destination port and send known UC packet with learnt DA - check no packets on destination port
--*         check egress MIB counters: brgEgrFilterDisc = 1, orther counters - 0
--*       link down all destination ports and send BC packet - check no egress packet on all ports in VLAN
--*         check egress MIB counters: all counters - 0 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


-- . There is erratum in BobK - skip test
SUPPORTED_FEATURE_DECLARE(devNum, "BRIDGE_EGR_COUNTER")  

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]


-- this test is relevant to ALL tested devices
--##################################
--##################################

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

local vlanTag = "81000002"

--generate test packet
local packetPayload = "6666"..-- dummy ethertype
        "000002030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"

local macOfPort1   = "000000001111"
local macOfPort2   = "000000002222"
local macOfPort4   = "000000004444"
local macBroadCast = "FFFFFFFFFFFF"

local packetFromPort1 = macBroadCast .. macOfPort1 .. vlanTag .. packetPayload
local packetFromPort2 = macOfPort4 .. macOfPort2 .. vlanTag .. packetPayload
local packetFromPort4 = macOfPort2 .. macOfPort4 .. vlanTag .. packetPayload

--##################################
--##################################
local fromPort1Traffic = {
    transmitInfo = {portNum = port1 , pktInfo = {fullPacket = packetFromPort1} },
    egressInfoTable = {
        -- flooded as unknownd
        {portNum = port2, packetCount = 0},
        {portNum = port3, packetCount = 0},
        {portNum = port4, packetCount = 0}
    }
}

local fromPort2Traffic = {
    transmitInfo = {portNum = port2 , pktInfo = {fullPacket = packetFromPort2} },
    egressInfoTable = {
        -- flooded as unknown
        {portNum = port1, packetCount = 1},
        {portNum = port3, packetCount = 1},
        {portNum = port4, packetCount = 1}
    }
}

local fromPort4Traffic = {
    transmitInfo = {portNum = port4 , pktInfo = {fullPacket = packetFromPort4} },
    egressInfoTable = {
        -- mac already learned to port 2
        {portNum = port1, packetCount = 0},
        {portNum = port2, packetCount = 1},
        {portNum = port3, packetCount = 0}
    }
}

local function portStateEnable(port, enable)
    local rc
    
    str = string.format("portStateEnable: port %-4s, %s", port, tostring(enable))
    printLog(str)

    rc = myGenWrapper("cpssDxChPortForceLinkPassEnableSet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
        {"IN","GT_BOOL","state", enable}
    })

    if rc ~= 0 then
        print(string.format("ERROR: cpssDxChPortForceLinkPassEnableSet: port %2d, rc %2d", port, rc))
    end

    rc = myGenWrapper("cpssDxChPortForceLinkDownEnableSet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
        {"IN","GT_BOOL","state", not enable}
    })

    if rc ~= 0 then
        print(string.format("ERROR: cpssDxChPortForceLinkDownEnableSet: port %2d, rc %2d", port, rc))
    end

    rc = myGenWrapper("cpssDxChPortEnableSet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
        {"IN","GT_BOOL","enable", enable}
    })

    if rc ~= 0 then
        print(string.format("ERROR: cpssDxChPortEnableSet: port %2d, rc %2d", port, rc))
    end

    print(string.format("delay(100)"))
    delay(100)
end

local function doTest()
    local i, egrCntrPtr, val
    local sectionName = "check for flooding: unknown UC"
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
        fromPort2Traffic.transmitInfo, fromPort2Traffic.egressInfoTable)

    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED."
        printLog ("ENDED : " .. pass_string .. "\n")
    else
        local error_string = "Section " .. sectionName .. " FAILED."
        printLog ("ENDED : " .. error_string .. "\n")
        setFailState()
    end

    sectionName = "check for unicast: known UC"
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
        fromPort4Traffic.transmitInfo, fromPort4Traffic.egressInfoTable)

    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED."
        printLog ("ENDED : " .. pass_string .. "\n")
    else
        local error_string = "Section " .. sectionName .. " FAILED."
        printLog ("ENDED : " .. error_string .. "\n")
        setFailState()
    end

    sectionName = "check for egress filtering: known UC"

    str = string.format("Disable destination port %-4s", port2)
    printLog(str)
    -- disable port
    portStateEnable(port2, false)

    printLog (delimiter)

    local egressInfoTableLinkDown = {
        -- port 2 is disabled
        {portNum = port1, packetCount = 0},
        {portNum = port2, packetCount = 0},
        {portNum = port3, packetCount = 0}
    }
    fromPort4Traffic.egressInfoTable = egressInfoTableLinkDown

    --clear egres counters
    rc,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_U8","cntrSetNum",0},
          {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
    })
    if rc ~= 0 then
        print(string.format("ERROR: cpssDxChPortEgressCntrsGet return: %2d", rc))
    end
    --send known UC packet
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
        fromPort4Traffic.transmitInfo, fromPort4Traffic.egressInfoTable)
    
    --check egres counters
    rc,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_U8","cntrSetNum",0},
          {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
    })
    if rc ~= 0 then
        print(string.format("ERROR: cpssDxChPortEgressCntrsGet return: %2d", rc))
    end

    egrCntrPtr = val["egrCntrPtr"]

    printLog (delimiter)
    str = string.format("outUcFrames %-4s", egrCntrPtr["outUcFrames"]) .. " " .. 
          string.format("outMcFrames %-4s", egrCntrPtr["outMcFrames"]) .. " " .. 
          string.format("outBcFrames %-4s", egrCntrPtr["outBcFrames"]) .. " " .. 
          string.format("brgEgrFilterDisc %-4s", egrCntrPtr["brgEgrFilterDisc"])
    printLog(str)

    if egrCntrPtr["outUcFrames"] == 0 and egrCntrPtr["outMcFrames"] == 0 and egrCntrPtr["outBcFrames"] == 0 and 
       egrCntrPtr["brgEgrFilterDisc"] ~= 0 then
            local pass_string = "Section " .. sectionName .. " PASSED."
            printLog ("ENDED : " .. pass_string .. "\n")
    else
            local error_string = "Section " .. sectionName .. " FAILED."
            printLog ("ENDED : " .. error_string .. "\n")
            setFailState()
    end
        
    sectionName = "check non-egress filtering: BC"
    -- link down all destination ports
    for i = 2, 4 do
        str = string.format("Disable destination port %-4s", devEnv.port[i])
        printLog(str)
        -- disable port
        portStateEnable(devEnv.port[i], false)
    end

    --clear egres counters
    rc,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_U8","cntrSetNum",0},
          {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
    })
    if rc ~= 0 then
        print(string.format("ERROR: cpssDxChPortEgressCntrsGet return: %2d", rc))
    end

    --send BC packet
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
    fromPort1Traffic.transmitInfo, fromPort1Traffic.egressInfoTable)

    --check egres counters
    rc,val = myGenWrapper("cpssDxChPortEgressCntrsGet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_U8","cntrSetNum",0},
          {"OUT","CPSS_PORT_EGRESS_CNTR_STC","egrCntrPtr"}
    })
    if rc ~= 0 then
        print(string.format("ERROR: cpssDxChPortEgressCntrsGet rc: %2d", rc))
    end

    egrCntrPtr = val["egrCntrPtr"]

    printLog (delimiter)
    str = string.format("outUcFrames %-4s", egrCntrPtr["outUcFrames"]) .. " " .. 
          string.format("outMcFrames %-4s", egrCntrPtr["outMcFrames"]) .. " " .. 
          string.format("outBcFrames %-4s", egrCntrPtr["outBcFrames"]) .. " " .. 
          string.format("brgEgrFilterDisc %-4s", egrCntrPtr["brgEgrFilterDisc"])
    printLog(str)

    if egrCntrPtr["outUcFrames"] == 0 and egrCntrPtr["outMcFrames"] == 0 and egrCntrPtr["brgEgrFilterDisc"] == 0 and egrCntrPtr["outBcFrames"] == 0 then
            local pass_string = "Section " .. sectionName .. " PASSED."
            printLog ("ENDED : " .. pass_string .. "\n")
    else
            local error_string = "Section " .. sectionName .. " FAILED."
            printLog ("ENDED : " .. error_string .. "\n")
            setFailState()
    end

    for i = 2, 4 do
        str = string.format("Ensable destination port %-4s", devEnv.port[i])
        printLog(str)
        -- enable port
        portStateEnable(devEnv.port[i], true)
    end
end
str = string.format("Test Port1 %-4s", devEnv.port[1]) .. " " .. 
      string.format("Test Port2 %-4s", devEnv.port[2]) .. " " .. 
      string.format("Test Port3 %-4s", devEnv.port[3]) .. " " .. 
      string.format("Test Port4 %-4s", devEnv.port[4])
printLog(str)

-- set configure test configurations
executeLocalConfig("dxCh/examples/configurations/brg_egf_txq_filter.txt")

-- run the test
doTest()

rc = pcall(
    executeStringCliCommands,
    "end\n" .. "clear bridge type " .. to_string(devNum) .. " all")
if (rc == false) then
    setFailState()
end 
-- clear counters and reset ports to default status
resetPortCounters(devEnv.dev, devEnv.port[1])
resetPortCounters(devEnv.dev, devEnv.port[2])
resetPortCounters(devEnv.dev, devEnv.port[3]) 
resetPortCounters(devEnv.dev, devEnv.port[4])
-- reset configure test configurations
executeLocalConfig("dxCh/examples/configurations/brg_egf_txq_filter_deconfig.txt")


