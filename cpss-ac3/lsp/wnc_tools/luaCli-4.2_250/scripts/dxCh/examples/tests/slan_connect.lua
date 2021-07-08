--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* slan_connect.lua
--*
--* DESCRIPTION:
--*       The test for SLAN Connect feature. Check that connected ports are link UP.
--*       Relevant for Simulation only
--*       Connect ports 64, 66 and 80 to SLANs, add them to VLAN 10
--*       Send packet to port 0 with VLAN 10 and see flooding on all ports of VLAN.
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--==============================================================================
--=============================== MAIN PART ====================================

local devNum  = devEnv.dev

-- skip test for unsupported devices
if(not is_sip_5(devNum) and "CPSS_PP_FAMILY_CHEETAH_E" ~= devFamily) then
    setTestStateSkipped() 
    return
end

-- only on simulation because using SLANs
SUPPORTED_FEATURE_DECLARE(devNum, "DYNAMIC_SLAN")
SUPPORTED_FEATURE_DECLARE(devNum, "NOT_BC2_GM")

if not wrlCpssIsAsicSimulation() then
    setTestStateSkipped();
    return;
end

local payloads

-- look for first four port not used in regular tests
local dev_ports_info = luaCLI_getDevInfo();
local dev_ports = dev_ports_info[devNum];
local dev_ports_num = #(dev_ports);
local i;
local is_dev_port = {};
for i = 1,dev_ports_num do
    is_dev_port[dev_ports[i]] = {};
end
local default_ext_ports = {64,66,80,49};
local default_ext_ports_num = #(default_ext_ports);
local default_ext_ports_present = true;
for i = 1,default_ext_ports_num do
    if not is_dev_port[default_ext_ports[i]] then
        default_ext_ports_present = false;
    end
end
local ext_ports;
if default_ext_ports_present then
    ext_ports = default_ext_ports;
else
    local test_ports_num = #(devEnv.port);
    local is_test_port = {};
    for i = 1,test_ports_num do
        is_test_port[devEnv.port[i]] = {};
    end
    local ext_ports_index = 1;
    ext_ports = {};
    for i = dev_ports_num,1,-1 do
        if not is_test_port[dev_ports[i]]
            and dev_ports[i] ~= 63
            and not check_port_is_cascade(devNum, dev_ports[i])
        then
            ext_ports[ext_ports_index] = dev_ports[i];
            ext_ports_index = ext_ports_index + 1;
        end
    end
end

local port1   = devEnv.port[1];
local port2   = ext_ports[1];
local port3   = ext_ports[2];
local port4   = ext_ports[3];

global_test_data_env.ext_port1 = ext_ports[1];
global_test_data_env.ext_port2 = ext_ports[2];
global_test_data_env.ext_port3 = ext_ports[3];
global_test_data_env.ext_port4 = ext_ports[4];

local ret, status

--generate test packets
ret, payloads = pcall(dofile, "dxCh/examples/packets/vlan10_20.lua")

--we expect flooding on the MAC DA ... so make sure FDB is empty
pcall(executeLocalConfig,"dxCh/examples/configurations/flush_fdb.txt")
--load configuration for test
executeLocalConfig("dxCh/examples/configurations/slan_connect.txt")
executeLocalConfig("dxCh/examples/configurations/loopback.txt")

-- provide time for simulation to manage link states
delay(500)
printLog("Sending flooded traffic from port ", port1)

-- reset ports counters only after configuration to avoid bandwidth conflict.
-- resetPortCounters configures portModeSpeedSet for all ports with NA interface.
-- the configuration configures NA ports to avoid bandwidth conflict.
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)
resetPortCounters(devNum, port4)

local transmitInfo = {portNum = port1 , pktInfo = {fullPacket = payloads["vlan10"]} }
local egressInfoTable = {
    {portNum = port2  , packetCount = 1},
    {portNum = port3  , packetCount = 1},
    {portNum = port4  , packetCount = 1},
}

-- check that packet egress the needed port(s) , when the 'threshold' allow it
local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\n check of flood : passed \n")
else
    printLog ("\n check of flood : failed \n")
    setFailState()
end
--loopback for port #1
executeLocalConfig("dxCh/examples/configurations/loopback.txt")
printLog("Force Link down on port ", port3)
executeLocalConfig("dxCh/examples/configurations/shutdown_port.txt")
-- provide time for simulation to manage link states
local needed_delay = 500
delay(needed_delay)
--[[
    !!!!  can't use : luaTgfTransmitPacketsWithExpectedEgressInfo
        because this function FORCE link up on all ports , 
        and remove the 'force link down' that needed for this test 

local egressInfoTable = {
    {portNum = port2  , packetCount = 1},
    {portNum = port3  , packetCount = 0}, -- port is forced down
    {portNum = port4  , packetCount = 1},
}

printLog("Sending flooded traffic from port ", port1)
local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\n check of flood with port " .. port3 .. " is down : passed \n")
else
    printLog ("\n check of flood with port " .. port3 .. " is down : failed \n")
    setFailState()
end
]]--
ret, status = sendPacket({[devNum]={port1}}, payloads["vlan10"])
delay(needed_delay)
printLog("Check counters")
local isFail = checkExpectedCounters(debug.getinfo(1).currentline,
    {[devNum] = { [port1]  = {Rx=1},
        [port2] = {Rx=0, Tx = 1},
        [port3] = {Rx=0, Tx = 0},
        [port4] = {Rx=0, Tx = 1}        }})
if not isFail then
    printLog ("\n check of flood with port " .. port3 .. " is down : passed \n")
else
    printLog ("\n check of flood with port " .. port3 .. " is down : failed \n")
    setFailState()
end

-- remove the loopback on port 0
executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")
delay(needed_delay)

-- check 2 ports that connected on the same slan
-- 1. set port 1,2 with same slan
-- 2. set ports 1,3,4 in vlan 10 as UNTAGGED members (egress untagged)
-- 3. set port 2 with pvid 0
-- 4. send packet ingress port 3 to flood ports 1,4 (in vlan 10)
--      since port 1 connected to port 2 , than port 2 should get ingress packets (that egress port 1)
--      but note that packet egress port 2 without vlan tag !
-- 5. check that port 2 got single ingress packet , and got no egress packets.
-- NOTE: the ingress packet from port 2 will be dropped because vlan 0 is not valid


local cmdBindSlanTo2Ports = [[
end
configure
interface range ethernet ${dev}/${port[1]},${port[3]},${port[4]}
switchport allowed vlan add 10 untagged
exit

end
debug-mode
cpss-api call cpssDxChBrgVlanPortVidSet devNum ${dev} portNum ${port[2]} direction CPSS_DIRECTION_INGRESS_E vlanId 0

simulation slan connect ethernet ${dev}/${port[1]} slan33
simulation slan connect ethernet ${dev}/${port[2]} slan33 keep-other-ports-on-slan enable
exit
end

show simulation slan all
end

]]

local isOk, msg  = pcall(executeStringCliCommands, cmdBindSlanTo2Ports)
if not isOk then
    print(msg)
    setFailState()
end


-- from port 3 to cause flood to ports 1,4 .. 1 connected to 2 .. 2 will drop the packet on pvid=0
local transmitInfo = {portNum = devEnv.port[3] ,
                        pktInfo = {fullPacket = payloads["vlan10"]}}
local egressInfoTable = {
     {portNum = devEnv.port[1] } -- flood to port 1
    ,{portNum = devEnv.port[2] , packetCount = 0 --[[egress]] , packetCountIngress = 1 --[[ingress]]} -- connected to port 1
    ,{portNum = devEnv.port[4] } -- flood to port 4
}

rc = 0
local section_name = "\n check of : from port " .. devEnv.port[3] .. " to cause flood to ports : "..devEnv.port[1]..","..devEnv.port[4].. 
                     ". port : "..devEnv.port[1].." connected to port : " ..devEnv.port[3] .." that will drop the packet on pvid=0"
-- transmit packet and check that egress as expected
local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog (section_name.." : passed \n")
else
    printLog (section_name.." : failed \n")
    setFailState()
end

-- restore pvid 1 to port 2
-- add port 2 to vlan 10
-- unbind the ports from slan
local cmdUnBindSlanTo2Ports = [[
end
configure
interface range ethernet ${dev}/${port[2]}
switchport allowed vlan add 10 untagged
exit

end
debug-mode
cpss-api call cpssDxChBrgVlanPortVidSet devNum ${dev} portNum ${port[2]} direction CPSS_DIRECTION_INGRESS_E vlanId 1

no simulation slan connect ethernet ${dev}/${port[1]}
no simulation slan connect ethernet ${dev}/${port[2]}
exit
end

show simulation slan all
end
]]

-- remove the connection of the 2 ports by slan
local isOk, msg  = pcall(executeStringCliCommands, cmdUnBindSlanTo2Ports)
if not isOk then
    print(msg)
    setFailState()
end



-- check that configuration back to normal
-- from port 3 to cause flood to ports 1,4 
local transmitInfo = {portNum = devEnv.port[3] ,
                        pktInfo = {fullPacket = payloads["vlan10"]}}
local egressInfoTable = {
     {portNum = devEnv.port[1] } -- flood to port 1
    ,{portNum = devEnv.port[2] } -- flood to port 2
    ,{portNum = devEnv.port[4] } -- flood to port 4
}

local section_name = "\n check of : after unbind of connecting slan between the 2 ports"

-- transmit packet and check that egress as expected
local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog (section_name.." : passed \n")
else
    printLog (section_name.." : failed \n")
    setFailState()
end

-- remove ports from vlan
local cmdRemovePortsFromVlan = [[
end
configure
interface range ethernet ${dev}/${port[1]},${port[2]},${port[3]},${port[4]}
switchport allowed vlan remove 10
exit
]]
-- remove added ports from vlan
local isOk, msg  = pcall(executeStringCliCommands, cmdRemovePortsFromVlan)
if not isOk then
    print(msg)
    setFailState()
end



printLog("Restore configuration")
executeLocalConfig("dxCh/examples/configurations/slan_connect_deconfig.txt")

global_test_data_env.ext_port1 = nil
global_test_data_env.ext_port2 = nil
global_test_data_env.ext_port3 = nil
global_test_data_env.ext_port4 = nil
