--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* rmon_histogram.lua
--*
--* DESCRIPTION:
--*       The test for testing rmon histogram [rx-tx|rx|tx|disable] feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

-- the GM not supports the full mac config and counters
SUPPORTED_FEATURE_DECLARE(devNum,"FULL_MAC_CONFIG_AND_COUNTERS") 
SUPPORTED_FEATURE_DECLARE(devNum,"RMON_HISTOGRAM")

local payload
local counters1, counters2
local ret, status

local stage1 = false
local stage2 = false

--generate broadcast packet
do
  ret, payload = pcall(dofile, "dxCh/examples/packets/broadcast.lua")

  if not ret then
    printLog ('Error in packet')
    setFailState()
    return
  end
end

local devNum1 = devEnv.dev
local portNum1 = devEnv.port[1]
local devNum2 = devEnv.dev
local portNum2 = devEnv.port[2]
local port1_str = devNum1 .."/".. portNum1
local port2_str = devNum2 .."/".. portNum2

local trgDevPortList = {[devNum1]={portNum1,portNum2}}

-- check if port is CG port (100G port)
function is_cg_port(devNum, portNum)
    local result, values = 
        cpssPerPortParamGet("cpssDxChPortMacTypeGet",
            devNum, portNum, "portMacTypePtr",
            "CPSS_PORT_MAC_TYPE_ENT")

    if result == 0 and values and values.portMacTypePtr == "CPSS_PORT_MAC_TYPE_CG_E" then
        return true
    end
    
    return false
end

-- check if port is MTI port
function is_mti_port(devNum, portNum)
    local result, values = 
    cpssPerPortParamGet("cpssDxChPortMacTypeGet",
        devNum, portNum, "portMacTypePtr",
        "CPSS_PORT_MAC_TYPE_ENT")
    if result == 0 and values then
        if values.portMacTypePtr == "CPSS_PORT_MAC_TYPE_MTI_64_E" or values.portMacTypePtr == "CPSS_PORT_MAC_TYPE_MTI_400_E" then
            return true
        end
    end
    
    return false
end

-- tested 4 cases for modes disable, rx, tx, rx-tx
-- rx-tx mode is default and should be last to avoid deconfiguration
-- same mode used on both ports
-- mode configured in HW for groups of ports (for CH and xCat: 0-5, 6-11, 12-18, 18-23)
-- the tested ports numbers calculated by the system,
-- so the ports can be in the same group and the different modes for ports not tested

local function check_stage(mode, port1Expected, port2Expected)
    local isCgPort1 = is_cg_port(devNum1, portNum1)
    local isCgPort2 = is_cg_port(devNum2, portNum2)
    local isMtiPort1 = is_mti_port(devNum1, portNum1)
    local isMtiPort2 = is_mti_port(devNum2, portNum2)

    printLog(
        "MODE: " .. mode ..
        " pots: " .. port1_str .. " and " .. port2_str)
        
    if isCgPort1 then 
        printLog("port " .. portNum1 .. " is CG port")
        printLog("CG port --never-- count Tx histogram")
        printLog("CG port --always-- count Rx histogram")

        port1Expected = 1 -- counts only 'rx' regardless to modes
    end
    
    if isCgPort2 then 
        printLog("port " .. portNum2 .. " is CG port") 
        printLog("CG port --never-- count Tx histogram")
        
        port2Expected = 0 -- never counts 'tx' regardless to modes
    end

    if isMtiPort1 or isMtiPort2 then 
        printLog("MTI port --always-- counts Rx and Tx histogram")

        port1Expected = 2 -- counts 'rx' regardless to modes
        port2Expected = 2 -- counts 'tx' regardless to modes
    end

    
    resetPortCounters(devNum1, portNum1)
    resetPortCounters(devNum2, portNum2)
    local config_cmd = [[
    end
    configure
    interface ethernet ${dev}/${port[2]}
    ]] ..
    "rmon histogram " .. mode .. "\n" .. [[
    exit
    interface ethernet ${dev}/${port[1]}
    ]] ..
    "rmon histogram " .. mode .. "\n" .. [[
    end
    ]]
    ret = pcall(executeStringCliCommands, config_cmd)
    if (ret == false) then
        printLog("ERROR : Failed to run commands (1)")
        setFailState()
    end

    --sending packet
    printLog("Sending traffic ..")
    -- [dev] = {ports list}

    ret,status = sendPacket(trgDevPortList, payload)

    printLog("Check counters ..")
    counters1 = getTrafficCounters(devNum1, portNum1)
    counters2 = getTrafficCounters(devNum2, portNum2)

    --check counters:
    -- for dev0/port0 expected Rcv ~= 0; Sent ~= 0; pkts == 2; [histogram rx-tx]
    -- for dev0/port18 expected Rcv == 0; Sent ~= 0; pkts == 0; [histogram disabled]
    if counters1.goodOctetsRcv["l"][0] ~= 0 and counters1.goodOctetsRcv["l"][1] == 0 and
       counters1.goodOctetsSent["l"][0] ~= 0 and counters1.goodOctetsSent["l"][1] == 0 and
       counters1.pkts64Octets["l"][0] == port1Expected and counters1.pkts64Octets["l"][1] == 0 and
       counters2.goodOctetsRcv["l"][0] == 0 and counters2.goodOctetsRcv["l"][1] == 0 and
       counters2.goodOctetsSent["l"][0] ~= 0 and counters2.goodOctetsSent["l"][1] == 0 and
       counters2.pkts64Octets["l"][0] == port2Expected and counters2.pkts64Octets["l"][1] == 0 then

      printLog("Counters check succeeded\n");
    else
        printLog("ERROR : counters check failed")

        if(counters1.goodOctetsRcv["l"][0] == 0) then 
            printLog("ERROR : port " .. portNum1 .. " : expected NON-ZERO goodOctetsRcv")
        end
        
        if(counters1.goodOctetsSent["l"][0] == 0) then 
            printLog("ERROR : port " .. portNum1 .. " : expected NON-ZERO goodOctetsSent") 
        end
        
        if(counters1.pkts64Octets["l"][0] ~= port1Expected) then 
            printLog("ERROR : port " .. portNum1 .. " expected pkts64Octets =" .. port1Expected ) 
        end

        if(counters2.goodOctetsRcv["l"][0] == 0)  then 
            printLog("ERROR : port " .. portNum2 .. " : expected NON-ZERO goodOctetsRcv")  
        end
        
        if(counters2.goodOctetsSent["l"][0] == 0) then 
            printLog("ERROR : port " .. portNum2 .. " : expected NON-ZERO goodOctetsSent") 
        end
        
        if(counters2.pkts64Octets["l"][0] ~= port1Expected) then 
            printLog("ERROR : port " .. portNum2 .. " expected pkts64Octets =" .. port2Expected ) 
        end

        setFailState();
    end

    printLog("PORT " .. port1_str .. " goodOctetsRcv =      ",counters1.goodOctetsRcv["l"][0])
    printLog("PORT " .. port1_str .. " goodOctetsSent =     ",counters1.goodOctetsSent["l"][0])
    printLog("PORT " .. port1_str .. " pkts64Octets =       ",counters1.pkts64Octets["l"][0])
    printLog("PORT " .. port2_str .. " goodOctetsRcv =      ",counters2.goodOctetsRcv["l"][0])
    printLog("PORT " .. port2_str .. " goodOctetsSent =     ",counters2.goodOctetsSent["l"][0])
    printLog("PORT " .. port2_str .. " pkts64Octets =       ",counters2.pkts64Octets["l"][0])
end

local loopback_port1 = [[
end
configure
interface ethernet ${dev}/${port[1]}
loopback mode internal
end
]]

-- tested 4 cases for modes disable, rx, tx, rx-tx
-- rx-tx mode is default and should be last to avoid deconfiguration
local stages = {
    {mode = "disable", port1Expected = 0, port2Expected = 0},
    {mode = "rx",      port1Expected = 1, port2Expected = 0},
    {mode = "tx",      port1Expected = 1, port2Expected = 2},
    {mode = "rx-tx",   port1Expected = 2, port2Expected = 2},
}

local i;

local no_loopback_port1 = [[
end
configure
interface ethernet ${dev}/${port[1]}
no loopback
end
]]

ret = pcall(executeStringCliCommands, loopback_port1)
if (ret == false) then
    printLog("ERROR : Failed to run commands (2)")
    setFailState()
end

for i = 1,#stages do
    check_stage(stages[i].mode, stages[i].port1Expected, stages[i].port2Expected);
end

ret = pcall(executeStringCliCommands, no_loopback_port1)
if (ret == false) then
    printLog("ERROR : Failed to run commands (3)")
    setFailState()
end

local clear_fdb = [[
end
clear bridge interface ${dev} all
]]

ret = pcall(executeStringCliCommands, clear_fdb)
if (ret == false) then
    printLog("ERROR : Failed to run commands (4)")
    setFailState()
end


