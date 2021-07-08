--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* trunks_change_max_num_members.lua
--*
--* DESCRIPTION:
--*       The test Allow to re-init the Trunk lib to have new number of trunks 
--*       all with new max number of members 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 
 
-- implement few function needed by the config files (to make the config file generic for all devices)

--
-- NOTE: function is called from config file , like : {@@getExistingPortsRange(16)}
-- the caller needs valid range for command : interface range ethernet
-- the ports need be 'valid' those that have 'mapping'
function getExistingPortsRange(numOfPorts)
    -- get list of physical ports with 'mac'
    local port_list = get_port_list(devNum)
    local maxNum = #port_list
    local maxLoop

    --print("port_list=",to_string(port_list))
    --print("maxNum=",to_string(maxNum))
    
    if maxNum < numOfPorts then
        maxLoop = maxNum
    else
        maxLoop = numOfPorts
    end
    
    local retStr = ""
    
    for ii = 1 , maxLoop do
        if ii > 1 then
            retStr = retStr .. ","
        end
        retStr = retStr .. to_string(port_list[ii])
    end

    return retStr
end

local origNumTrunks = 1 -- dummy value
local origMaxNumMembers = 2 -- dummy value

--
-- NOTE: function is called from config file , like : {@@trunkOrigNumOfTrunks()}
-- save value of origNumTrunks , origMaxNumMembers
function trunkOrigInitSave()
    local trunkId = 1 -- representative trunkId
    
    -- use global variable (NOTE: it will be updated by command : port-channel fixed-num-members ...)
    origNumTrunks = NumberOfEntriesIn_TRUNK_table

    local result, values = myGenWrapper(
    "cpssDxChTrunkDbFlexInfoGet", {
        { "IN", "GT_U8" , "dev", devNum}, -- devNum
        { "IN", "GT_TRUNK_ID" , "trunkId", trunkId}, -- devNum
        { "OUT","GT_U32", "l2EcmpStartIndex"},
        { "OUT", "GT_U32",  "maxNumOfMembers"}
    })

    if result == 0 then
        origMaxNumMembers = values.maxNumOfMembers
    else
        origMaxNumMembers = 8
    end
    
    return ""
end

--
-- NOTE: function is called from config file , like : {@@trunkOrigNumOfTrunks()}
--                                          or like : {@@trunkOrigNumOfTrunks(1)}
-- get the number of origNumTrunks + addedValue
--
function trunkOrigNumOfTrunks(addedValue)
    if addedValue then
        addedValue = tonumber(addedValue)
    else
        addedValue = 0
    end
    return origNumTrunks + addedValue
end
--
-- NOTE: function is called from config file , like : {@@trunkOrigMaxNumMembers()}
-- get the number of origMaxNumMembers
-- 
function trunkOrigMaxNumMembers()
    return origMaxNumMembers
end

--===============--
--===============--
--===============--
executeLocalConfig("dxCh/examples/configurations/trunks_change_max_num_members.txt")
printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/trunks_change_max_num_members_deconfig.txt")
