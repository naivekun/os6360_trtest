--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* fwdtolb_fwd.lua
--*
--* DESCRIPTION:
--*       .
--* Test the feature 'Forwarding to Loopback/Service port' for FORWARD packet type.
--* CONFIGURE:
--* - Create vlan 5 with tagged ports 1, 2, 3.
--* - Configure port 1 Source Loopback Profile to be equal 3.
--* - Configure port 2 Target Loopback Profile to be equal 1.
--* - Configure port 2 Loopback Port to be equal port 3.
--* - Enable 'Forwarding to Loopback/Service port' generally for
--*   Loopback profiles pair {3,1}
--* - and for all traffic classes.
--* - Enable 'Forwarding to Loopback/Service port' for FORWARD packet type
--* - both for unicast, multi-target).
--* GENERATE TRAFFIC.
--* - Send the vlan 5 tagged packet to rx-port 1.
--*   The packet is expected on the loopback port 3.
--* - Disable 'Forwarding to Loopback/Service port' for FORWARD packet type.
--* - Send the vlan 5 tagged packet to rx-port 1.
--*   The packet is expected on the target port 2.
--* RESTORE CONFIGURATION.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev

-- this test is relevant for SIP_5 devices only. In case of xCat3x all port should
-- be Aldrin-ports.
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum, "FWD_TO_LB" , "ports" , nil)

-- local declaration
local port1 = devEnv.port[1]
local port2 = devEnv.port[2]
local port3 = devEnv.port[3]

local configFile   = "dxCh/examples/configurations/fwdtolb_fwd.txt"
local deconfigFile = "dxCh/examples/configurations/fwdtolb_fwd_deconfig.txt"

local ret, packetInfo, packet = pcall(dofile, "dxCh/examples/packets/vlan_mapping.lua")
if not ret then
   printLog ('Error in the network packet')
   setFailState()
   return
end

local function sendAndCheck(isLb)
    local transmitInfo = {devNum = devNum,
                         portNum = port1,
                         pktInfo = {fullPacket = packet}}

    local egressInfoTable
    if isLb then
        egressInfoTable = {
            {portNum = port2, packetCount = 0},
            {portNum = port3, packetCount = 1}}
    else
        egressInfoTable = {
            {portNum = port2, packetCount = 1},
            {portNum = port3, packetCount = 0}}
    end

    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,
                                                           egressInfoTable)
    if rc ~= 0 then
        error(string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
                                "#%d, \" %s \" ", rc, returnCodes[rc]))
    end
end


--==============================================================================
-- Call without deconfig file. It will be executed by the last luaTgfSimpleTest call.
luaTgfSimpleTest(configFile, nil, sendAndCheck, true)

-- disable Forwarding to Loopback/Service port for FORWARD packet type

executeStringCliCommands([[
end
configure
no forward-to-loopback packet-type-enable forward-from-cpu device ${dev}
]])

-- Call without config file. It was executed already by the first luaTgfSimpleTest call
luaTgfSimpleTest(nil, deconfigFile, sendAndCheck, false)
