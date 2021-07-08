--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* sanity_acl.lua
--* 
--* based on test : acl_redirect.lua
--*
--* DESCRIPTION:
--*       The test send SINGLE packet and expect ACL Redirect feature.
--*       Single packet only to allow DEBUG device with TCAM problems (during 
--*       bring up or on EMULATOR)
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

local ret, status , rc

if(devEnv.run_iteration_id == nil) then
    devEnv.run_iteration_id = 0
end 

devEnv.run_iteration_id = devEnv.run_iteration_id + 1

--generate ACL redirect test packets

ret, payloads = pcall(dofile, "dxCh/examples/packets/acl_redirect.lua")

--reset ports counters
resetPortCounters(devNum, port1)
resetPortCounters(devNum, port2)
resetPortCounters(devNum, port3)
resetPortCounters(devNum, port4)

local do_config = true

if devEnv.run_iteration_id > 1 and 
   devEnv.run_specific == true then
    -- running the specific test at this iteration not need to set config again
    -- just need to send the packet (and check egress ports counters)
    do_config = false
    printLog("DO NOT set configuration (already done on first run of the test) ..")
end

if do_config then
    --------------------------------------------------------------------------------
    --load configuration for test
    if is_device_eArch_enbled(devNum) then
        executeLocalConfig("dxCh/examples/configurations/acl_redirect.txt")
    else
        executeLocalConfig("dxCh/examples/configurations/acl_redirect_no_tpid.txt")
    end
end

printLog("Sending matched redirected traffic from port " ..  tostring(port3) .. " to egress ONLY port " .. tostring(port2) .. " (instead of flooding)")
-- PCL redirects from port 3 to port2
local transmitInfo = {
    devNum = devNum,
    portNum = port3,
    pktInfo = {fullPacket = payloads["aclMatch"]}
}
local egressInfoTable = {
    -- expected egress port (apply also mask on needed fields)
    {portNum = port1, packetCount = 0 },
    {portNum = port2 },
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

if(devEnv.run_specific == true) then
    printLog("DO NOT Restore configuration (allow debug current config) ..")
else
    printLog("Restore configuration ..")
    if is_device_eArch_enbled(devNum) then
        executeLocalConfig("dxCh/examples/configurations/acl_redirect_deconfig.txt")
    else
        executeLocalConfig("dxCh/examples/configurations/acl_redirect_deconfig_no_tpid.txt")
    end
end
