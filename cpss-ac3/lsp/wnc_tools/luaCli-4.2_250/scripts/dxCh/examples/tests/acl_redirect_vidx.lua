--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* acl_redirect.lua
--*
--* DESCRIPTION:
--*       The test for ACL Redirect feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local payloads

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local ret, status

--generate ACL redirect test packets

ret, payloads = pcall(dofile, "dxCh/examples/packets/acl_redirect.lua")

--reset ports counters
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)
resetPortCounters(devNum, port4)

--------------------------------------------------------------------------------
local vidx_name0 = "TEST_ACL_REDIRECT_VIDX0"; -- to exclude vidx==0
local vidx_name = "TEST_ACL_REDIRECT_VIDX";
local vidx = sharedResourceNamedVidxMemoAlloc(vidx_name0, 1 --[[size--]])
vidx = sharedResourceNamedVidxMemoAlloc(vidx_name, 1 --[[size--]])
global_test_data_env.vidx = vidx;
local port_bitmap = {[0]=0,0,0,0,0,0,0,0};
local word_num = math.floor(port2 / 32)
local bit_num = port2 - (word_num * 32);
port_bitmap[word_num] = bit_shl(1, bit_num);
global_test_data_env.port_bitmap = to_string(port_bitmap);

--load configuration for test
executeLocalConfig("dxCh/examples/configurations/acl_redirect_vidx.txt")

printLog("Sending not matched flooded traffic from port " .. port3)

-- flooding from port 3 to all other ports
transmitInfo = {
    devNum = devNum,
    portNum = port3 ,
    pktInfo = {fullPacket = payloads["aclNotMatch"]}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
     {portNum = port1 },
     {portNum = port2 },
     {portNum = port4 }
}
-- transmit packet and check that egress as expected
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
if rc == 0 then
    printLog ("Test passed \n")
else
    printLog ("Test failed \n")
    setFailState()
end

printLog("Sending matched redirected traffic from port ".. port1)
-- PCL redirects from port 1 to port2
transmitInfo = {
    devNum = devNum,
    portNum = port1 ,
    pktInfo = {fullPacket = payloads["aclMatch"]}
}
egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port2 },
    {portNum = port3, packetCount = 0 },
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
executeLocalConfig("dxCh/examples/configurations/acl_redirect_vidx_deconfig.txt")
sharedResourceNamedVidxMemoFree(vidx_name);
sharedResourceNamedVidxMemoFree(vidx_name0);
global_test_data_env.vidx = nil;
global_test_data_env.port_bitmap = nil;

