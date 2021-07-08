--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* jumbo_bridging.lua
--*
--* DESCRIPTION:
--*       The test for bridging Jumbo 8K size in vlan
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_FEATURE_DECLARE(devEnv.dev, "JUMBO_FRAME") 
--SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

-- function called from the test
-- see : ${@@jumboSize(9000)} 
-- function convert the needed MRU to the 'upper' value that the device supports
function jumboSize(neededMru)
    if is_xCat3x_in_system() then
        -- the 88e1690 ports supports only next values : 10240
        if neededMru > 10240 then
            return neededMru
        else
            return  10240
        end
    end
    
    return neededMru
end

local error = false

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local macSa = "000222222222"
local macDa = "000123456789"-- unknown DA
local vlanTag = "81000002" -- vid 2 (defined in jumbo_bridging.txt)
local etherType = "6666" -- not special EtherType
local packetHeader = macDa --[[6]].. macSa --[[6]].. vlanTag --[[4]].. etherType--[[2]]

local payload_32 = 
    "00112233445566778899" .. --10
    "aabbccddeeff" .. --6
    "0123456789abcdef" .. --8
    "55aaaa55a5a5a5a5"    --8
local payload_128 = payload_32 .. payload_32 .. payload_32 .. payload_32
local payload_1K = payload_128 .. payload_128 .. payload_128 .. payload_128 .. payload_128 .. payload_128 .. payload_128 .. payload_128
local payload_8K = payload_1K  .. payload_1K  .. payload_1K  .. payload_1K  .. payload_1K  .. payload_1K  .. payload_1K  .. payload_1K
local jumboFrame8K = packetHeader .. payload_8K

local packetHeader_untagged = macDa --[[6]].. macSa --[[6]].. etherType--[[2]]
local jumboFrame8K_untagged = packetHeader_untagged .. payload_8K

executeLocalConfig("dxCh/examples/configurations/jumbo_bridging.txt")

-- test part
--------------

-- from port 2 to port 3
local transmitInfo = {devNum = devNum, portNum = port2 ,
    pktInfo = {fullPacket = jumboFrame8K} }
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1  }--check only mac counters
    ,{portNum = port3 , pktInfo = {fullPacket = jumboFrame8K}}--check mac counters + expected egress packet
    ,{portNum = port4 , pktInfo = {fullPacket = jumboFrame8K_untagged}}--check mac counters + expected egress packet
}

-- transmit packet and check that egress as expected
local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("\n The test passed \n")
else
    printLog ("\n ERROR : The test failed \n")
    error = true
end

-- flush the FDB (use LUA CLI command !)
local function flushFdbDynamic()
    local fileName = "flush_fdb_only_dynamic"
    executeLocalConfig(luaTgfBuildConfigFileName(fileName))
end
-- flush the FDB
printLog("Flush FDB as 'clean-up' for next test ...")
flushFdbDynamic()
printLog("Restore configuration ...")
executeLocalConfig("dxCh/examples/configurations/jumbo_bridging_deconfig.txt")

if error == true then setFailState() end




