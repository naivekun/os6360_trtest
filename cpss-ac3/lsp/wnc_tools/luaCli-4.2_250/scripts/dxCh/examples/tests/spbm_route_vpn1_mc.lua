--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* spbm_route_vpn1.lua
--*
--* DESCRIPTION:
--*       An SPBM routing test: MC Routing in VPN-1.
--*       Two differences from definition in SPBM presentaion:
--*       1.Packet should not be routed to the port he was sent from
--*         because we use the loopback send mechanism and it will cause storming
--*       2.Packet should not be bridged to any port that it will be routed to it
--*         because our egressed packets checking mechanism does not support
--*         different packets egressed from the same port. The packet bridged
--*         to all Back Bone connection ports only in each device and filtered
--*         at ingress port.  (it is enough to check bridging also).
--*       Send IP MC packet to a device A, port1=0.
--*       DIP=224.1.1.1, SIP=172.16.1.1,
--*       MAC_DA=01:00:5e:01:01:01, MAC_SA=00:00:00:00:01:23, VID=1000
--*       The packets will be bridged to other devices only.
--*       The packet will be routed with the same IPV4 header and MAC_DA to:
--*       to A/port2=18 MAC SA = devA, vlan=2000.
--*       to B/port3=36 MAC SA = devB, vlan=3000.
--*       to C/port1=0  MAC SA = devC, untagged (vlan=1000).
--*
--*       A more detailed description of the Backbone network you can find
--*       in config/deconfig-files of the test.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

-- local declaration
local strPacketFileName   = "dxCh/examples/packets/spbm_route_vpn1_mc.lua"

local configFileA   = "dxCh/examples/configurations/spbm_route_vpn_dev_a.txt"
local deconfigFileA = "dxCh/examples/configurations/spbm_route_vpn_dev_a_deconfig.txt"

local configFileB   = "dxCh/examples/configurations/spbm_route_vpn_dev_b.txt"
local deconfigFileB = "dxCh/examples/configurations/spbm_route_vpn_dev_b_deconfig.txt"

local configFileC   = "dxCh/examples/configurations/spbm_route_vpn_dev_c.txt"
local deconfigFileC = "dxCh/examples/configurations/spbm_route_vpn_dev_c_deconfig.txt"

local function trafficCheck(trafficScenario)
    local key, scenario;
    for key, scenario in pairs(trafficScenario) do
        print("========================================")
            print("Send a packet #".. tostring(key))
        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
           scenario.transmitInfo, scenario.egressInfoTable);
        assert(
            rc==0, string.format("packet transmitting error: %s",
            returnCodes[rc]))
    end
end

-- check the packets transmission on the device A.
local function testDevA(scenario)
    return trafficCheck(scenario["a"]);
end
-- check the packets transmission on the device B.
local function testDevB(scenario)
    return trafficCheck(scenario["b"]);
end
-- check the packets transmission on the device C.
local function testDevC(scenario)
    return trafficCheck(scenario["c"]);
end

------------------------------ MAIN PART ---------------------------------------
-- read the packets from the file
local isOk;
local ret, scenario = pcall(dofile, strPacketFileName);
if not ret then
    error("\nError while reading the packets");
end

xcat3x_start(true)

print("-------------------------------------------------")
print("SPBM route test VPN 1 MC (vrf-id 1), device A")
isOk = luaTgfSimpleTest(configFileA, deconfigFileA, testDevA, scenario)
if isOk then
   print("-------------------------------------------------")
   print("SPBM route test VPN 1 MC (vrf-id 1), device B")
   isOk = luaTgfSimpleTest(configFileB, deconfigFileB, testDevB, scenario)
end
if isOk then
   print("-------------------------------------------------")
   print("SPBM route test VPN 1 MC (vrf-id 1), device C")
   isOk = luaTgfSimpleTest(configFileC, deconfigFileC, testDevC, scenario)
end

xcat3x_start(false)
