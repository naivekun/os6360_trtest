--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* fdb_learn_from_edsa_tag.lua
--*
--* DESCRIPTION:
--*       Test FDB learning on device that got eDSA with srcEPort and src vid info
--*       Test cover scenario of: [JIRA] : CPSS-4289 : When packet is received untagged 
--*       on the cascade port it doesn't take the DSA tag VID.       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev

-- this test is relevant for SIP_5 devices (BOBCAT 2 and above)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5") 

--- AC3X need proper ports for this test
-- only devEnv.port[2] must support the "DSA_CASCADE"
-- the call to SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM may change the value of devEnv.port[2] !!!
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum, "DSA_CASCADE" , "ports" , {devEnv.port[2]})

local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4] 

local ingressPort = port2


--##################################
--##################################
local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"
--[[ 
packet from the JIRA (include CRC)
NOTE: it seems that the packet holds 3 CRCs ! "1d c0 d0 c4" , "1c df 44 21" , "1c df 44 21"
--> most inner is from SMB simulation
--> second is from 'out of first device'
--> third one ??

0x0000 : 00 00 00 00 00 01 00 00 00 00 00 03 d0 00 10 0a 
0x0010 : 80 00 1f ff 80 00 00 00 00 00 00 40 00 64 00 00 
0x0020 : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
0x0030 : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
0x0040 : 00 00 00 00 00 00 00 00 00 00 00 00 1d c0 d0 c4 
0x0050 : 1c df 44 21 1c df 44 21
]]--
local srcMacAddr = "000000000003"
local expectedVid = 0x00a
local expectedVidString = "00a"

--packet from the JIRA (without CRC)
local ingressWithEDsa_fullPacket = 
"000000000001"..srcMacAddr.."d0001"..expectedVidString..
"80001fff800000000000004000640000"..
"00000000000000000000000000000000"..
"00000000000000000000000000000000"..
"0000000000000000000000001dc0d0c4"..
"1cdf4421"

local additionalVlanTag = "81000016"

--additional case : JIRA was not closed due to bug in packets with vlan tag after the eDsa
local ingressWithEDsa_with_additional_vlanTag_fullPacket = 
"000000000001"..srcMacAddr.."d0001"..expectedVidString..
"80001fff8000000000000040" .. additionalVlanTag .. "00640000"..
"00000000000000000000000000000000"..
"00000000000000000000000000000000"..
"0000000000000000000000001dc0d0c4"..
"1cdf4421"

-- set configurations
local function setConfig()
    --set config
    executeLocalConfig(luaTgfBuildConfigFileName("fdb_learn_from_edsa_tag"))
end
-- unset configurations
local function unsetConfig()
    --unset config
    executeLocalConfig(luaTgfBuildConfigFileName("fdb_learn_from_edsa_tag",true))
end

-- caseIndex - 0 --> ingress packet without vlan tag after the eDSA
-- caseIndex - 1 --> ingress packet with    vlan tag after the eDSA
local function send_eDsa_for_fdb_learn(caseIndex)
    local transmitInfo = {portNum = ingressPort , pktInfo = {fullPacket = ingressWithEDsa_fullPacket}} 
    local sectionName = "ingress eDSA for learning purpose"
    
    if(caseIndex == 1) then
        transmitInfo.pktInfo.fullPacket = ingressWithEDsa_with_additional_vlanTag_fullPacket
        sectionName = "ingress eDSA for learning purpose (ingress packet with    vlan tag after the eDSA)"
    end
    
    -- we not care about the forwarding of this packet... only about what is the 'vlanId' assigned to it.
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)
    
    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED"
        printLog ("ENDED : " .. pass_string .. "\n")
        testAddPassString(pass_string)
    else
        local error_string = "Section " .. sectionName .. " FAILED"
        printLog ("ENDED : " .. error_string .. "\n")
        testAddErrorString(error_string)
    end 
    printLog (delimiter)
end

-- check that the entry is in the FDB with the expected vlanId
local function checkFdbEntry()
    local sectionName = "check that the entry is in the FDB with the expected vlanId (".. expectedVid ..")"
    
    -- do visualization of the FDB table .. for this mac address
    local command_showFdbEntry = 
    [[
    end
    show mac address-table dynamic device ${dev} address ]].. srcMacAddr ..[[
    ]]
    executeStringCliCommands(command_showFdbEntry)

    local mac_filter = 
        { ["key"]                   =
            { ["entryType"]             = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
              ["key"] = 
               { ["macVlan"] =
                   {
                      ["macAddr"]               = srcMacAddr,
                      ["vlanId"]                = expectedVid
                   }
               }
            }
        } 
    
    
    result, values, mac_entry_index, dummyVidx =
        wrlCpssDxChBrgIsEntryExists(devNum, 0,
                                    mac_filter) 
                                    
    if result ~= 0 or not values then
        -- error , no entry with this macAddr was found
        local error_string = "Section " .. sectionName .. " FAILED. as the FDB entry not found"
        printLog ("ENDED : " .. error_string .. "\n")
        testAddErrorString(error_string)
    else
        -- OK .. as expected
        local pass_string = "Section " .. sectionName .. " PASSED"
        printLog ("ENDED : " .. pass_string .. "\n")
        testAddPassString(pass_string)
    end
                                    
    return
end

-- flush the FDB (use LUA CLI command !)
local function flushFdb()

    local fileName = "flush_fdb_only_dynamic"
    printLog("call to Flush the FDB")
    executeLocalConfig(luaTgfBuildConfigFileName(fileName))
   
end 

-- config
setConfig()
-- send the traffic with eDSA with vlan tag after the eDSA
send_eDsa_for_fdb_learn(1)
-- check that the entry is in the FDB with the expected vlanId
checkFdbEntry()

-- flush the FDB between iterations
flushFdb()

send_eDsa_for_fdb_learn(0)
-- check that the entry is in the FDB with the expected vlanId
checkFdbEntry()

-- deconfig
unsetConfig()
-- print results summary 
testPrintResultSummary("FDB learn from eDSA")
