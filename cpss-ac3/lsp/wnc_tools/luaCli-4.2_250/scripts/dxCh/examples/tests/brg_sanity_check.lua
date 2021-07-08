--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* brg_sanity_check.lua
--*
--* DESCRIPTION:
--*       The test for Bridge sanity check feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devNum,"SIP_5")

local payloads

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local ret, status

--generate ACL redirect test packets

ret, payloads = pcall(dofile, "dxCh/examples/packets/brg_sanity_check.lua")

--reset ports counters
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)
resetPortCounters(devNum, port4)

--------------------------------------------------------------------------------
--load configuration for test
executeLocalConfig("dxCh/examples/configurations/brg_sanity_check.txt")

printLog("Sending IPv4/IPv6 packets with SIP address equal to DIP address from port " ..  tostring(port3))
-- flooding from port 3 to all other ports
transmitInfo = {
    devNum = devNum,
    portNum = port3,
    pktInfo = {fullPacket = payloads["sipIsDip"]}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1, packetCount = 0 },
    {portNum = port2, packetCount = 0 },
    {portNum = port4, packetCount = 0 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("Sending TCP/UDP packets with source TCP/UDP port equal to destination TCP/UDP port from port " ..  tostring(port3))
-- flooding from port 3 to all other ports
transmitInfo = {
    devNum = devNum,
    portNum = port3,
    pktInfo = {fullPacket = payloads["tcpUdpSportIsDport"]}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1, packetCount = 0 },
    {portNum = port2, packetCount = 0 },
    {portNum = port4, packetCount = 0 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("Sending TCP packets with the TCP FIN flag set and the TCP ACK not set from port " ..  tostring(port3))
-- flooding from port 3 to all other ports
transmitInfo = {
    devNum = devNum,
    portNum = port3,
    pktInfo = {fullPacket = payloads["tcpFinWithoutAck"]}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1, packetCount = 0 },
    {portNum = port2, packetCount = 0 },
    {portNum = port4, packetCount = 0 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/brg_sanity_check_deconfig.txt")

