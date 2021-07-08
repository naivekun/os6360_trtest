--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_ingress_port_redirect.lua
--*
--* DESCRIPTION:
--*     Example of packets redirecting from selected ingress port to 
--*      list of egress ports.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


local devNum  = devEnv.dev
local port0 = devEnv.port[1]  -- physPort #00
local port1 = devEnv.port[5]  -- physPort #01
local port2 = devEnv.port[6]  -- physPort #02
local port4 = devEnv.port[2]  -- physPort #04
local port5 = devEnv.port[3]  -- physPort #05


-- test packet
local packetPayload = "000102030405060708090A0B0C0D0E0F" ..
                      "101112131415161718191A1B1C1D1E1F" ..
                      "202122232425262728292A2B2C2D2E2F" ..
                      "303132333435363738393A3B3C3D3E3F" ..
                      "404142434445464748494A4B4C4D4E4F"
 
local macDa       = "000000001111"
local macSa       = "000000002222"
local ethertype   = "6666" -- dummy ethertype

local function buildPacket()
    return macDa .. macSa ..  ethertype .. packetPayload
end

-- create test packet
local ingressPacket = buildPacket()
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = ingressPacket} }

-- expected counters
local egressInfoTable = {
    {
      {portNum = port0  , packetCount = 1},
      {portNum = port1  , packetCount = 1},
      {portNum = port4  , packetCount = 1},
      {portNum = port5  , packetCount = 1}
    },
    {
      {portNum = port0  , packetCount = 1},
      {portNum = port1  , packetCount = 1},
      {portNum = port4  , packetCount = 0},
      {portNum = port5  , packetCount = 0}
    },
    {
      {portNum = port0  , packetCount = 0},
      {portNum = port1  , packetCount = 0},
      {portNum = port4  , packetCount = 0},
      {portNum = port5  , packetCount = 0}
    }
}

-- list of egress ports
local portBmp  = { 0xFFFF, 0x03, 0x00 }


-- Check egress  traffic
local function ingressPortRedirectTest()
    local stage
    local command_data = Command_Data()

    stage = 1
    -- enable redirection with portBmp  = 0xFFFF 
    ingressPort = port2
    -- Remove ingress port from destination ports bitmap to prevent endless loop
    portBmp[stage] = bit_and(portBmp[stage], bit_not(bit_shl(1,ingressPort)))

    portsBitmap = string.format('0x%X', portBmp[stage]) 
    printLog ("Stage# " .. stage .. ": portsBmp  = " .. portsBitmap .. "\n")

    executeLocalConfig(luaTgfBuildConfigFileName("PX_ingress_port_redirect"))

    -- check traffic
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo, egressInfoTable[stage])
    if rc ~= 0 then
      local error_string = "Test FAILED."
      printLog ("ENDED : " .. error_string .. "\n")
      setFailState()
      return
    end


    stage = 2
    -- enable redirection with portBmp  = 0x03
    ingressPort = port2
    portsBitmap = string.format('0x%X', portBmp[stage]) 
    printLog ("Stage #" .. stage .. ": portsBmp  = " .. portsBitmap .. "\n")

    executeLocalConfig(luaTgfBuildConfigFileName("PX_ingress_port_redirect"))

    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo, egressInfoTable[stage])
    if rc ~= 0 then
      local error_string = "Test FAILED."
      printLog ("ENDED : " .. error_string .. "\n")
      setFailState()
      return
    end


    stage = 3
    -- Disable ingress port redirection --> run deconfig with portBmp  = 0x0000
    -- Check if traffic not redirected (all counters are zeroed)
    ingressPort = port2
    portsBitmap = string.format('0x%X', portBmp[stage]) 
    printLog ("Stage#" .. stage .. ": portsBmp  = " .. portsBitmap .. "\n")
    
    executeLocalConfig(luaTgfBuildConfigFileName("PX_ingress_port_redirect_deconfig"))

    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo, egressInfoTable[stage])
    if rc ~= 0 then
      local error_string = "Test FAILED."
      printLog ("ENDED : " .. error_string .. "\n")
      setFailState()
      return
    end

end

ingressPortRedirectTest()