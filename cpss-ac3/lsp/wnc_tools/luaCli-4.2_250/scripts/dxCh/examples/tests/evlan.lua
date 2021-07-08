--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* evlan.lua
--*
--* DESCRIPTION:
--*       The test for testing vlan with VID > 4095 support
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local evlan_id = 4097
-- this test is relevant for SIP_5 devices (BOBCAT 2 and above)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5")
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum,""--[[no specific feature]],"EVLAN",evlan_id)
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum,"eVlan>=4K","ports",{devEnv.port[1],devEnv.port[2]})

local port1   = devEnv.port[1]
local port2   = devEnv.port[2]

local payloads
local status

--generate test packets
do
    status, payloads = pcall(dofile, "dxCh/examples/packets/evlan.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

-- clean FDB before test
executeLocalConfig("dxCh/examples/configurations/flush_fdb.txt")

printLog("============================== Set configuration ===============================")
executeLocalConfig("dxCh/examples/configurations/evlan.txt")

--reset ports counters
resetPortCounters(devEnv.dev, devEnv.port[1])
resetPortCounters(devEnv.dev, devEnv.port[2])

local transmitInfo;
transmitInfo = {devNum = devNum, portNum = port1, pktInfo = {fullPacket = payloads["my_untagged_packet"]} }
local egressInfoTable = {
    {portNum = port2, packetCount = 1 }
}
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

if not rc == 0 then
    printLog ("Test failed \n")
    setFailState()
end

local command_showFdbEntry = 
[[
    end
    show mac address-table all device ${dev}
    end
]]

local isOk, msg = pcall(executeStringCliCommands, command_showFdbEntry)
if not isOk then
    setFailState()
end

local indexCnt = 0
local validCnt = 0
local fdbEnd = false
local entries = {}
local function getNumOfFdbEntriesAndFirstEntry()
    repeat
        local result, values = myGenWrapper(
        "cpssDxChBrgFdbMacEntryRead", {
            { "IN", "GT_U8", "dev", devNum},            -- devNum
            { "IN", "GT_U32", "index", indexCnt },
            { "OUT","GT_BOOL", "valid" },               -- *validPtr,
            { "OUT","GT_BOOL", "skip" },                -- *skipPtr,
            { "OUT","GT_BOOL", "aged" },                -- *agedPtr,
            { "OUT","GT_U8", "devNum" },                -- *associatedDevNumPtr,
            { "OUT","CPSS_MAC_ENTRY_EXT_STC", "entry" } --  *entryPtr
        })

        if result == 0 then
            if values.valid and not values.skip then
                validCnt = validCnt + 1
                table.insert(entries, values)
            end
        elseif result == 3 then
            fdbEnd = true
        end
        indexCnt = indexCnt + 1
    until fdbEnd
end
-- limit the iterations (in GM it uses the 'FDB shadow' due to poor performance of the GM)
local function gmUsed_getNumOfFdbEntriesAndFirstEntry()
    local command_data = Command_Data()
    local params = {all_device = {devNum}}
-- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceMacEntryIterator(
        params, { ["dstInterface"] =
                    {["type"]    = { "CPSS_INTERFACE_PORT_E",
                                     "CPSS_INTERFACE_TRUNK_E",
                                     "CPSS_INTERFACE_VID_E"    }}})
                                     
    --printLog("params:" ,to_string(params))
                                     
    for iterator, _devNum, entry_index in
                                command_data:getValidMacEntryIterator() do
        --printLog("iterator" , to_string(iterator))
        --printLog("_devNum" , to_string(_devNum))
        --printLog("entry_index" , to_string(entry_index))
                                
        local result, values = myGenWrapper(
        "cpssDxChBrgFdbMacEntryRead", {
            { "IN", "GT_U8", "dev", devNum},            -- devNum
            { "IN", "GT_U32", "index", entry_index },
            { "OUT","GT_BOOL", "valid" },               -- *validPtr,
            { "OUT","GT_BOOL", "skip" },                -- *skipPtr,
            { "OUT","GT_BOOL", "aged" },                -- *agedPtr,
            { "OUT","GT_U8", "devNum" },                -- *associatedDevNumPtr,
            { "OUT","CPSS_MAC_ENTRY_EXT_STC", "entry" } --  *entryPtr
        })

        if result == 0 then
            if values.valid and not values.skip then
                --printLog("values.entry" , to_string(values.entry))
                validCnt = validCnt + 1
                table.insert(entries, values)
            end
        end
    end
                                     
end

if isGmUsed() or isEmulatorUsed() then
    gmUsed_getNumOfFdbEntriesAndFirstEntry()
else
    getNumOfFdbEntriesAndFirstEntry()
end

 -- printLog("DBG" .. to_string(entries))

if validCnt == 1 and entries[1].entry.key.key.macVlan.vlanId == evlan_id then
    printLog ("Found FDB entry on evlan[".. evlan_id .."]\n")
    printLog ("Test passed \n")
else
    printLog ("Error : Not Found FDB entry on evlan[".. evlan_id .."]\n")
    printLog ("Test failed \n")
    setFailState()
end

printLog("============================ Restore configuration =============================")
executeLocalConfig("dxCh/examples/configurations/evlan_deconfig.txt")
