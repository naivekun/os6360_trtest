--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* clear_mac.lua
--*
--* DESCRIPTION:
--*             Test for command clear mac.
--*             It creates static amd dynamic mac addresses on different VLAN's and interfaces
--*             and clear mac entries on specified VLAN or ethernet interface. 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local function printMacAddressCount(values)
    printLog("Static: "..values.static.." | Dynamic: "..values.dynamic)
end
local function createMacEntries()
    local ret,errorMessage
    executeLocalConfig("dxCh/examples/configurations/clear_mac.txt")
    local rc,values=wrlCpssDxChBrgFdbMacAddressTableCountGet(devEnv.dev,0)
    if (rc~=0) then
        printLog("Failed to count MAC addresses")
        setFailState()
    end
    printLog("Created MAC addresses")
    printMacAddressCount(values)
end
--Counts MAC addresses on specific VLAN or ethernet port. If there are no input parameters it counts MAC's on all VLAN's or all ports
local function countMacEntries(params)
    local localParams=params
    if (params == nil) then
        localParams={}
    end
    local counters={vlan_static=0, vlan_dynamic=0, port_static=0, port_dynamic=0}
    --vlan_static - Number of static MAC addresses at specified VLAN
    --vlan_dynamic - Number of dynamic MAC addresses at specified VLAN
    --port_static - Number of static MAC addresses at specified port nubmer
    --port_dynamic - Number of dynamic MAC addresses at specified port_number
    local rc=0
    local mac_index=0
    while (rc==0) do
        rc, values, skip_valid_aged, mac_entry=next_valid_mac_entry_get(devEnv.dev,mac_index)
        if (rc==0 and mac_entry~=nil) then
            if (mac_entry.key.key.macVlan.vlanId == localParams.vlanId or not localParams.vlanId) then
                if (mac_entry.isStatic==true) then
                    counters.vlan_static=counters.vlan_static+1
                else
                   counters.vlan_dynamic=counters.vlan_dynamic+1
                end
            end
            if (mac_entry.dstInterface.devPort.portNum == localParams.portNum or not localParams.portNum) then
                if (mac_entry.isStatic==true) then
                    counters.port_static=counters.port_static+1
                else
                    counters.port_dynamic=counters.port_dynamic+1
                end
            end
            mac_index=values+1
        else
            break
        end
    end
    return counters
end
local function doMacAddressTableClear(mode, param, value)
    local cmdString
    if (param=="vlan") then
        cmdString="vlan "..to_string(value)
    elseif (param=="portNum") then
        cmdString="ethernet ${dev}/"..to_string(value)
    end
    ret=pcall(executeStringCliCommands,"clear mac address-table "..mode.." device ${dev} "..cmdString)
    if (ret==false) then
        setFailState()
    end
end

-- Test checks aging deamon that is not supported in GM
SUPPORTED_FEATURE_DECLARE(devEnv.dev, "NOT_BC2_GM")

local rc,values
local counters_before
local counters_after
--Test 1:
--       - Create MAC entries
--       - Delete static entries on VLAN 1
--       - Check number of MAC addresses
printLog("\n")
printLog("***********************************************************************")
printLog("***             Testing clear mac address-table on VLAN 1           ***")
printLog("***********************************************************************")
wraplCpssDxChBrgFdbFlush(devEnv.dev,1)
createMacEntries()
counters_before=countMacEntries({vlanId=1})
printLog("--- Counters before clearing static enrtries on VLAN 1 ---")
printLog("- Static MAC's on VLAN 1: "..counters_before.vlan_static)
printLog("- Dynamic MAC's on VLAN 1: "..counters_before.vlan_dynamic)
doMacAddressTableClear("static","vlan",1)
counters_after=countMacEntries({vlanId=1})
printLog("--- Counters after clearing static enrtries on VLAN 1 ---")
printLog("- Static MAC's on VLAN 1: "..counters_after.vlan_static)
printLog("- Dynamic MAC's on VLAN 1: "..counters_after.vlan_dynamic)
if (counters_before.vlan_dynamic~=counters_after.vlan_dynamic or counters_after.vlan_static~=0) then
    printLog("Error: Failed to clear static MAC address on VLAN 1")
    setFailState()
else
    printLog("Test Passed")
end
--Test 2:
--       - Create MAC entries
--       - Delete dynamic entries on VLAN 3
--       - Check number of MAC addresses
printLog("\n")
printLog("***********************************************************************")
printLog("***             Testing clear mac address-table on VLAN 3           ***")
printLog("***********************************************************************")
wraplCpssDxChBrgFdbFlush(devEnv.dev,1)
createMacEntries()
counters_before=countMacEntries({vlanId=3})
printLog("--- Counters before clearing dynamic enrtries on VLAN 3 ---")
printLog("- Static MAC's on VLAN 3: "..counters_before.vlan_static)
printLog("- Dynamic MAC's on VLAN 3: "..counters_before.vlan_dynamic)
doMacAddressTableClear("dynamic","vlan",3)
counters_after=countMacEntries({vlanId=3})
printLog("--- Counters after clearing dynamic enrtries on VLAN 3 ---")
printLog("- Static MAC's on VLAN 3: "..counters_after.vlan_static)
printLog("- Dynamic MAC's on VLAN 3: "..counters_after.vlan_dynamic)
if (counters_before.vlan_static~=counters_after.vlan_static or counters_after.vlan_dynamic~=0) then
    printLog("Error: Failed to clear dynamic MAC address on VLAN 3")
    setFailState()
else
    printLog("Test Passed")
end
--Test 3:
--       - Create MAC entries
--       - Delete static entries on specified ethernet port
--       - Check number of MAC addresses
printLog("\n")
printLog("***********************************************************************")
printLog("***   Testing clear mac address-table on specified ethernet port    ***")
printLog("***********************************************************************")
printLog("Tested ethernet port: "..devEnv.port[2])
wraplCpssDxChBrgFdbFlush(devEnv.dev,1);
createMacEntries()
local counters_after_port1
local counters_after_port2
local counters_before_port1
local counters_before_port2
counters_before_port1=countMacEntries({portNum=devEnv.port[1]})
counters_before_port2=countMacEntries({portNum=devEnv.port[2]})
printLog("--- Counters before clearing ---")
printLog("- Static MAC's on port "..devEnv.port[1]..": "..counters_before_port1.port_static)
printLog("- Dynamic MAC's on port "..devEnv.port[1]..": "..counters_before_port1.port_dynamic)
printLog("- Static MAC's on port "..devEnv.port[2]..": "..counters_before_port2.port_static)
printLog("- Dynamic MAC's on port "..devEnv.port[2]..": "..counters_before_port2.port_dynamic)
doMacAddressTableClear("all","portNum",devEnv.port[2])
counters_after_port1=countMacEntries({portNum=devEnv.port[1]})
counters_after_port2=countMacEntries({portNum=devEnv.port[2]})
print("Counters after clearing")
printLog("- Static MAC's on port "..devEnv.port[1]..": "..counters_after_port1.port_static)
printLog("- Dynamic MAC's on port "..devEnv.port[1]..": "..counters_after_port1.port_dynamic)
printLog("- Static MAC's on port "..devEnv.port[2]..": "..counters_after_port2.port_static)
printLog("- Dynamic MAC's on port "..devEnv.port[2]..": "..counters_after_port2.port_dynamic)
if (counters_after_port2.port_dynamic~=0 or
    counters_after_port2.port_static~=0 or
    counters_before_port1.port_dynamic~=counters_after_port1.port_dynamic or
    counters_before_port1.port_static~=counters_after_port1.port_static) then
    printLog("Failed")
    setFailState()
else
    print("Test Passed")
end
executeLocalConfig("dxCh/examples/configurations/clear_mac_deconfig.txt")
