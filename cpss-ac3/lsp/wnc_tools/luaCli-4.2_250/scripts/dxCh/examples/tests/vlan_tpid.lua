--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_tpid.lua
--*
--* DESCRIPTION:
--*       The test for testing tpid feature
--*         packet ingress vlan tag 0x8100 and egress vlan tag 0x88a8
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


local function expr_q(expr,val_true,val_false)
    -- C:    expr ? val_true : val_false
    if expr then
        return val_true
    end
    return val_false
end

local pName
local payload
local status = false

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local pktInfo1

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

xcat3x_start(true)

executeLocalConfig("dxCh/examples/configurations/vlan_tpid.txt")
--executeLocalConfig("dxCh/examples/configurations/loopback.txt")

local tpid_8100 = 0x8100
local tpid_88a8 = 0x88a8
local tpid_88a8_str = string.format("%4.4x",tpid_88a8)
local vid = 20
local vid_str = string.format("%4.4x",vid)

local payload = "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
local saMac="00:00:00:11:22:33"
local daMac="00:00:00:44:55:66"
pktInfo1={
    partsArray={
        {type="TGF_PACKET_PART_L2_E", partPtr={saMac=saMac, daMac=daMac}},
        {type="TGF_PACKET_PART_VLAN_TAG_E", partPtr={etherType=tpid_8100, pri=0, cfi=0, vid=vid}},
        {type="TGF_PACKET_PART_PAYLOAD_E", partPtr=payload}
    }
}

local saMacStr="000000112233"
local daMacStr="000000445566"
local egrVlaTag = tpid_88a8_str..vid_str
-- egress based on ingress but replacing the TPID
local egress_pktInfo = {fullPacket = daMacStr..saMacStr..egrVlaTag..payload}
--[[
00 00 00 44 55 66 00 00 00 11 22 33 88 a8 00 14
11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
11 11 11 11 11 11 11 11 11 11 11 11 
]]--

local transmitInfo = {portNum = port1 , pktInfo = {partsArray = pktInfo1.partsArray} }
local egressInfoTable = {
    {portNum = port2  , pktInfo = {fullPacket = egress_pktInfo.fullPacket}},
    {portNum = port3  , packetCount = 0},
    {portNum = port4  , packetCount = 0},
}

-- check that packet egress the needed port(s) , when the 'threshold' allow it
local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\n check of flood : passed \n")
else
    printLog ("\n check of flood : failed \n")
    setFailState()
end 

--[[
rc=prvLuaTgfTransmitPacketsWithCapture(
    devNum,
    port1,
    pktInfo1,
    1,
    devNum,
    port2,
    "TGF_CAPTURE_MODE_MIRRORING_E",
    500)

rc,pkt=prvLuaTgfRxCapturedPacket(
    devNum,
    port2,
    "TGF_PACKET_TYPE_ANY_E",
    true)

if  rc == 0xC then
    printLog("There's no packet received")
    status = false
elseif rc ~= 0 then
    printLog("Error while executing prvLuaTgfRxCapturedPacket")
    status = false
else
    local i, part
    for i,part in pairs(pkt.partsArray) do
        print(to_string(part))
        if part.type == "TGF_PACKET_PART_PAYLOAD_E" then
            if prefix_match(tpid_88a8_str, part.partPtr.dataPtr) then
                status = true
            end
        end
        if part.type == "TGF_PACKET_PART_VLAN_TAG_E" then
            if tpid_88a8 == part.partPtr.etherType then
                status = true
            end
        end
    end
    printLog(expr_q(status,"PASSED","FAILED"))
end

if status ~= true then setFailState() end
]]--
printLog("STAGE1: all forward")
executeLocalConfig("dxCh/examples/configurations/vlan_tpid_deconfig.txt")
--executeLocalConfig("dxCh/examples/configurations/loopback_deconfig.txt")

printLog("STAGE2: all forward")
printLog("___________________________")

xcat3x_start(false)