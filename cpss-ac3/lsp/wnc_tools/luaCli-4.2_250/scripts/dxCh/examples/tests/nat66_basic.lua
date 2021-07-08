--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* nat66_basic.lua
--*
--* DESCRIPTION:
--*       The test for Basic NAT66: Modifying IP addresses
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

dofile ("dxCh/examples/common/nat66_utils.lua") -- show_nat66_test function declaration

local error = false

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
-- this test is relevant for SIP_5_15 devices (BobK)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5_15")

local pktInfo
local payload
local status
local rc

local dummy_name = "dummy name"
-- get first index that the NAT will get ... and than release it.
local firstNatIndex = sharedResourceNamedNatMemoAlloc(dummy_name,1)
sharedResourceNamedNatMemoFree(dummy_name)
printLog("First Index that Nat entry will use is : " .. firstNatIndex)


local configuration_1 = [[
end
configure
no nat66 nat-name NAT66:1
nat66 nat-name NAT66:1 local-ethernet-interface ${dev}/${port[2]} outer-ethernet-interface ${dev}/${port[3]} local-eport 0x218 outer-eport 0x236 local-mac-addr 00:04:05:06:07:11 outer-mac-addr 00:04:05:06:07:13 local-vid 5 outer-vid 6 private-ip-addr FD01:0203:0405:0607:0809::1234 public-ip-addr 2001:0203::5678 server-ip-addr 6545::5678 prefix-length 32
end
]]

local configuration_2 = [[
end
configure
no nat66 nat-name NAT66:1
nat66 nat-name NAT66:1 local-ethernet-interface ${dev}/${port[2]} outer-ethernet-interface ${dev}/${port[3]} local-eport 0x218 outer-eport 0x236 local-mac-addr 00:04:05:06:07:11 outer-mac-addr 00:04:05:06:07:13 local-vid 5 outer-vid 6 private-ip-addr FD01:0203:0405:0607:0809::1234 public-ip-addr 2001:0203:0405:0600::5678 server-ip-addr 6545::5678 prefix-length 56
end
]]

-- do not delete NAT66:1 defined in configuration_2, just add a new configutration to the same NAT66:1
local configuration_2_secondNat66_1_config = [[
end
configure
nat66 nat-name NAT66:1 local-ethernet-interface ${dev}/${port[1]} outer-ethernet-interface ${dev}/${port[3]} local-eport 0x200 outer-eport 0x236 local-mac-addr 00:04:05:06:07:22 outer-mac-addr 00:04:05:06:07:13 local-vid 7 outer-vid 6 private-ip-addr FD01:2222:3333:4444:5555::6666 public-ip-addr 2001:0203:0405:0600::5679 server-ip-addr 6545::5678 prefix-length 56
end
]]

local configuration_3 = [[
end
configure
no nat66 nat-name NAT66:1
nat66 nat-name NAT66:1 local-ethernet-interface ${dev}/${port[2]} outer-ethernet-interface ${dev}/${port[3]} local-eport 0x218 outer-eport 0x236 local-mac-addr 00:04:05:06:07:11 outer-mac-addr 00:04:05:06:07:13 local-vid 5 outer-vid 6 private-ip-addr FD01:0203:0405:0607:0809::1234 public-ip-addr 2001:0203:0405:0607::5678 server-ip-addr 6545::5678 prefix-length 64
end
]]

local configuration_4 = [[
end
configure
no nat66 nat-name NAT66:1
nat66 nat-name NAT66:1 local-ethernet-interface ${dev}/${port[2]} outer-ethernet-interface ${dev}/${port[3]} local-eport 0x218 outer-eport 0x236 local-mac-addr 00:22:BB:BB:BB:BB outer-mac-addr 00:04:05:06:07:13 local-vid 5 outer-vid 6 private-ip-addr FD01:0203:0405:0607:0809::1234 public-ip-addr 2001:0203:0405:0607:0809::5678 server-ip-addr 6545::5678 prefix-length 72
end
]]

local configuration_5 = [[
end
configure
counters ARP_NAT device 0
counters nat-offset device 0 offset 0
end
]]
local configuration_6 = [[
end
configure
no counters ARP_NAT device 0
no counters nat-offset device 0
end
]]

--generate test packets
do
    status, payload = pcall(dofile, "dxCh/examples/packets/nat66_basic.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(devNum)
if "CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily then
    global_test_data_env.localEPort1 = 0x218
    global_test_data_env.localEPort2 = 0x210
    global_test_data_env.outerEPort  = 0x236
else
    global_test_data_env.localEPort1 = 0x1018
    global_test_data_env.localEPort2 = 0x1000
    global_test_data_env.outerEPort = 0x1036
end

executeLocalConfig("dxCh/examples/configurations/nat66_basic.txt")

show_nat66_test()

--configuration 5: check CNC counters
executeStringCliCommands(configuration_5)
-- test part: private to public udp packet
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_2_to_3_udp"]}}
local egressInfoTable = {
    -- this port need to get traffic
    {portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_2_to_3_udp"]}},
    -- other port should not
    {portNum = port1  , packetCount = 0},
    {portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

if rc == 0 then
    printLog ("\nUDP Packet: The test part1 passed \n")
else
    printLog ("\nUDP Packet: The test part1 failed (ERROR) \n")
    error = true
end


-- check CNC counters
local params = {}
params["devID"] = devNum
params["arpNatCncIdx"] = firstNatIndex + 0 --check CNC counters for NAT index 0
local numberOfPackets, numberOfBytes
local ret, showVals = showCountersArpNat(params)
if (ret == false) then
    printLog ("\nError while reading NAT counter \n")
    error = true
else
    if showVals["pass"] == nil or showVals["pass"][1] == nil then
        numberOfPackets = 0
        numberOfBytes = 0
    else
        numberOfPackets = showVals["pass"][1]["Packet-Counter"]
        numberOfBytes = showVals["pass"][1]["Byte-count counter"]
    end

    if not (numberOfPackets >= 1 and numberOfBytes >= 64) then
        printLog ("\nNAT counter was not incrmented\n")
        error = true
    end
end

-- test part: public to private udp packet
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_3_to_2_udp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_3_to_2_udp"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nUDP Packet: The test part2 passed \n")
else
    printLog ("\nUDP Packet: The test part2 failed (ERROR) \n")
    error = true
end
params["arpNatCncIdx"] = firstNatIndex + 1 --check CNC counters for NAT index 1
ret, showVals = showCountersArpNat(params)
if (ret == false) then
    printLog ("\nError while reading NAT counter \n")
    error = true
else
    if showVals["pass"] == nil or showVals["pass"][1] == nil then
        numberOfPackets = 0
        numberOfBytes = 0
    else
        numberOfPackets = showVals["pass"][1]["Packet-Counter"]
        numberOfBytes = showVals["pass"][1]["Byte-count counter"]
    end
     if not (numberOfPackets >= 1 and numberOfBytes >= 64) then
        printLog ("\nNAT counter was not incrmented\n")
        error = true
    end
end
--configuration 6: remove CNC counters
executeStringCliCommands(configuration_6)

-- test part: private to public tcp packet
------------------------------------------
-- from port 1 to port 3
local transmitInfo = {portNum = port1 , pktInfo = {fullPacket = payload["private_to_public_1_to_3_tcp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port2 , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_1_to_3_tcp"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nTCP Packet: The test part1 passed \n")
else
    printLog ("\nTCP Packet: The test part1 failed (ERROR) \n")
    error = true
end

-- test part: public to private tcp packet
------------------------------------------
-- from port 3 to port 1
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_3_to_1_tcp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , pktInfo = {fullPacket = payload["egress_public_to_private_3_to_1_tcp"]}}
    ,{portNum = port2 , packetCount = 0 }
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nTCP Packet: The test part2 passed \n")
else
    printLog ("\nTCP Packet: The test part2 failed (ERROR) \n")
    error = true
end

-- test part: public to public ipv6 packet
------------------------------------------
-- from port 2 to port 1
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["public_to_public_2_to_1_ipv6"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , pktInfo = {fullPacket = payload["egress_public_to_public_2_to_1_ipv6"]}}
    ,{portNum = port3 , packetCount = 0 }
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nIPv6 Packet: The test part1 passed \n")
else
    printLog ("\nIPv6 Packet: The test part1 failed (ERROR) \n")
    error = true
end

-- test part: public to public ipv6 packet
------------------------------------------
-- from port 1 to port 2
local transmitInfo = {portNum = port1 , pktInfo = {fullPacket = payload["public_to_public_1_to_2_ipv6"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port3 , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_public_1_to_2_ipv6"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nIPv6 Packet: The test part2 passed \n")
else
    printLog ("\nIPv6 Packet: The test part2 failed (ERROR) \n")
    error = true
end


-- test part: private to public udp packet
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_2_to_3_udp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_2_to_3_udp"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 32: private to public part1 passed \n")
else
    printLog ("\nPrefix 32: private to public part1 failed (ERROR) \n")
    error = true
end

-- test part: public to private udp packet
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_3_to_2_udp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_3_to_2_udp"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 32: public to private udp packet part2 passed \n")
else
    printLog ("\nPrefix 32: public to private udp packet part2 failed (ERROR) \n")
    error = true
end


--configuration 1: set prefix length 32
executeStringCliCommands(configuration_1)

-- test part: private to public prefix 32 bit replacement
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_2_to_3_udp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_32"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 32: private to public part1 passed \n")
else
    printLog ("\nPrefix 32: private to public part1 failed (ERROR) \n")
    error = true
end

-- test part: public to private prefix 32 bit replacement
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_32"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_32"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 32: public to private part2 passed \n")
else
    printLog ("\nPrefix 32: public to private part2 failed (ERROR) \n")
    error = true
end

--configuration 2: set prefix length 56
executeStringCliCommands(configuration_2)

-- test part: private to public prefix 56 bit replacement
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_2_to_3_udp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_56"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 56: The test part1 passed \n")
else
    printLog ("\nPrefix 56: The test part1 failed (ERROR) \n")
    error = true
end

-- test part: public to private prefix 56 bit replacement
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_56"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_56"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 56: The test part2 passed \n")
else
    printLog ("\nPrefix 56: The test part2 failed (ERROR) \n")
    error = true
end


--configuration_2_secondNat66_1_config: add addition to NAT66:1 configuration
executeStringCliCommands(configuration_2_secondNat66_1_config)

-- test part: private to public prefix 56 bit replacement
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port1 , pktInfo = {fullPacket = payload["private_to_public_2_to_3_udp_secondNat66_1_config"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port2 , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_56_secondNat66_1_config"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 56 secondNat66_1_config: The test part1 passed \n")
else
    printLog ("\nPrefix 56 secondNat66_1_config: The test part1 failed (ERROR) \n")
    error = true
end


-- test part: public to private prefix 56 bit replacement
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_56_secondNat66_1_config"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , pktInfo = {fullPacket = payload["egress_public_to_private_56_secondNat66_1_config"]}}
    ,{portNum = port2 , packetCount = 0 }
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 56 secondNat66_1_config: The test part2 passed \n")
else
    printLog ("\nPrefix 56 secondNat66_1_config: The test part2 failed (ERROR) \n")
    error = true
end

--configuration 1: set prefix length 64
executeStringCliCommands(configuration_3)

-- test part: private to public prefix 64 bit replacement
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_2_to_3_udp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port3 , pktInfo = {fullPacket = payload["egress_private_to_public_64"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 64: The test part1 passed \n")
else
    printLog ("\nPrefix 64: The test part1 failed (ERROR) \n")
    error = true
end

-- test part: public to private prefix 64 bit replacement
------------------------------------------
-- from port 3 to port 2
local transmitInfo = {portNum = port3 , pktInfo = {fullPacket = payload["public_to_private_64"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port2 , pktInfo = {fullPacket = payload["egress_public_to_private_64"]}}
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 64: The test part2 passed \n")
else
    printLog ("\nPrefix 64: The test part2 failed (ERROR) \n")
    error = true
end

--configuration 1: set wrong prefix length 65
executeStringCliCommands(configuration_4)

-- test part: private to public prefix 64 bit replacement
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_2_to_3_udp"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port3 , packetCount = 0 }
    ,{portNum = port4 , packetCount = 0 }
}

-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 64: The test part1 passed \n")
else
    printLog ("\nPrefix 64: The test part1 failed (ERROR) \n")
    error = true
end

--configuration 2: set wrong prefix length 56 --> packet is dropped; nat counters incremented
executeStringCliCommands(configuration_2)
-- test part: private to public prefix 56 bit replacement
------------------------------------------
-- from port 2 to port 3
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = payload["private_to_public_2_to_3_56_dropped"]} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 , packetCount = 0 }
    ,{portNum = port3 , packetCount = 0 }
    ,{portNum = port4 , packetCount = 0 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\nPrefix 56 packet is dropped: passed \n")
else
    printLog ("\nPrefix 56 packet is dropped: failed (ERROR) \n")
    error = true
end
--check that NAT drop counter was incremented
local params = {}
params["device"] = devNum
local showVals = nat_drop_show(params)
if (showVals == false) then
    printLog ("\nError while reading NAT drop counter \n")
    error = true
else
    numberOfPackets = tonumber(showVals)
    if not (numberOfPackets == 1) then
        printLog ("\n Error : Prefix 56 packet is dropped but NAT drop counter was not incremented \n")
        error = true
    end
end

printLog("Restore configuration ...")
executeLocalConfig("dxCh/examples/configurations/nat66_basic_deconfig.txt")

if error == true then setFailState() end


