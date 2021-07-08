--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ipv4_routing_default_route_entry_cmd.lua
--*
--* DESCRIPTION:
--*       Test loose uRPF on sip entry assigned with 'default route entry' packet command.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev 
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local pkt
local pktInfo
local payload
local status

local transmitInfo;
local egressInfoTable;
local rc = 0;

SUPPORTED_SIP_DECLARE(devNum, "SIP_5_15")

--generate test packet
do
    status, payload = pcall(dofile, "dxCh/examples/packets/ipv4_routing_default_route_entry_cmd.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

transmitInfo = {devNum = devNum, portNum = port1, pktInfo = {fullPacket = payload} }

printLog("========================== Set configuration ===========================")

executeLocalConfig("dxCh/examples/configurations/ipv4_routing_default_route_entry_cmd.txt")

--Get hwDevNum
local ret, val = myGenWrapper("cpssDxChCfgHwDevNumGet",{
    {"IN",  "GT_U8",         "devNum",     devNum},
    {"OUT", "GT_HW_DEV_NUM", "hwDevNumPtr"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChCfgHwDevNumGet")
    setFailState()
end
local hw_device_id_number = val["hwDevNumPtr"];

--Write MAC entry
local mac_entry = {
        key = {
            entryType = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
            key = {
                macVlan = {
                    macAddr = "00:00:00:01:01:01",
                    vlanId = 200
                }
            }
        },
        dstInterface = {
            type = "CPSS_INTERFACE_PORT_E",
            devPort = {
                hwDevNum = hw_device_id_number,
                portNum = 61 -- tunnel, will be overriden
            }
        },
        isStatic = true,
        daRoute  = true,
        daCommand = "CPSS_MAC_TABLE_FRWRD_E"
    }

local ret, val = mac_entry_set(devNum, mac_entry);
if ret ~= 0 then
    printLog ("Error in mac_entry_set")
    setFailState()
end

printLog("================= case 1: Sip packet command is ROUTE ==================")

executeLocalConfig("dxCh/examples/configurations/ipv4_routing_default_route_entry_cmd_case_route_cmd.txt")

-- disable uRPF on the incoming VLAN
local ret, val = myGenWrapper("cpssDxChIpUcRpfModeSet",{
    {"IN", "GT_U8",                      "devNum",   devNum},
	{"IN", "GT_U16",                     "vid",      200},
    {"IN", "CPSS_DXCH_IP_URPF_MODE_ENT", "uRpfMode", "CPSS_DXCH_IP_URPF_DISABLE_MODE_E"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChIpUcRpfModeSet")
    setFailState()
end

-- IPv4 packet to port 1, check on all ports.
-- we expect packet to egress out port 2.

egressInfoTable =
{
    {portNum = port2  , packetCount = 1 },
    {portNum = port3  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo, egressInfoTable)

if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("=========== case 2: Sip packet command is DEFAULT_ROUTE_ENTRY ============")

executeLocalConfig("dxCh/examples/configurations/ipv4_routing_default_route_entry_cmd_case_default_route_entry_cmd.txt")

-- set loose uRPF on the incoming VLAN
local ret, val = myGenWrapper("cpssDxChIpUcRpfModeSet",{
    {"IN", "GT_U8",                      "devNum",   devNum},
	{"IN", "GT_U16",                     "vid",      200},
    {"IN", "CPSS_DXCH_IP_URPF_MODE_ENT", "uRpfMode", "CPSS_DXCH_IP_URPF_LOOSE_MODE_E"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChIpUcRpfModeSet")
    setFailState()
end

-- IPv4 packet to port 1, check on all ports.
-- we expect packet to be dropped by loose uRPF check.

egressInfoTable =
{
    {portNum = port2  , packetCount = 0 },
    {portNum = port3  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo, egressInfoTable)

if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("=========== case 3: Sip packet command is DEFAULT_ROUTE_ENTRY ============")
printLog("====================== but SIP lookup is disabled. =======================")

-- disable SIP lookup
local ret, val = myGenWrapper("cpssDxChLpmPortSipLookupEnableSet",{
    {"IN", "GT_U8",       "devNum",  devNum},
	{"IN", "GT_PORT_NUM", "portNum", port1},
    {"IN", "GT_BOOL",     "enable",  false}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChLpmPortSipLookupEnableSet")
    setFailState()
end

-- IPv4 packet to port 1, check on all ports.
-- we expect packet to egress out port 2.

egressInfoTable =
{
    {portNum = port2  , packetCount = 1 },
    {portNum = port3  , packetCount = 0 },
    {portNum = port4  , packetCount = 0 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo, egressInfoTable)

if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("======================== Restore configuration =========================")

--Delete MAC entry
local ret, val = mac_entry_delete(devNum, mac_entry);
if ret ~= 0 then
    printLog ("Error in mac_entry_delete")
    setFailState()
end

-- disable uRPF on the incoming VLAN
local ret, val = myGenWrapper("cpssDxChIpUcRpfModeSet",{
    {"IN", "GT_U8",                      "devNum",   devNum},
	{"IN", "GT_U16",                     "vid",      200},
    {"IN", "CPSS_DXCH_IP_URPF_MODE_ENT", "uRpfMode", "CPSS_DXCH_IP_URPF_DISABLE_MODE_E"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChIpUcRpfModeSet")
    setFailState()
end

-- enable SIP lookup
local ret, val = myGenWrapper("cpssDxChLpmPortSipLookupEnableSet",{
    {"IN", "GT_U8",       "devNum",  devNum},
	{"IN", "GT_PORT_NUM", "portNum", port1},
    {"IN", "GT_BOOL",     "enable",  true}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChLpmPortSipLookupEnableSet")
    setFailState()
end

executeLocalConfig("dxCh/examples/configurations/ipv4_routing_default_route_entry_cmd_deconfig.txt")
