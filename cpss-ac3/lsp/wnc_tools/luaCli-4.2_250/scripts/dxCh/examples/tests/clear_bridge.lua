--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* clear_bridge.lua
--*
--* DESCRIPTION:
--*             Test for commands clear bridge interface and clear bridge type.
--*             It creates static amd dynamic mac addresses then delete static_only
--*             or all MAC addresses from FDB table. After clearing FDB table test checks
--*             expected number of dynamic and static MAC addresses
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local function printMacAddressCount(values)
    printLog("Static: "..values.static.." | Dynamic: "..values.dynamic)
end
local function checkMacAddressCountResult(rc)
    if (rc~=0) then
        printLog("Failed to count MAC addresses")
        setFailState()
    end
end
local function createMacEntries()
    local ret,errorMessage
    local macs=[[
        end
        configure
        mac address-table static 00:11:00:00:00:00 device ${dev} vlan 1 ethernet ${dev}/${port[1]}
        mac address-table dynamic 00:22:00:00:00:00 device ${dev} vlan 1 ethernet ${dev}/${port[1]}
        mac address-table static 00:11:11:00:00:00 device ${dev} vlan 1 ethernet ${dev}/${port[1]}
        mac address-table dynamic 00:22:11:00:00:00 device ${dev} vlan 1 ethernet ${dev}/${port[1]}
        mac address-table static 00:11:22:00:00:00 device ${dev} vlan 1 ethernet ${dev}/${port[1]}
        mac address-table dynamic 00:22:22:00:00:00 device ${dev} vlan 1 ethernet ${dev}/${port[1]}
        mac address-table static 00:11:33:00:00:00 device ${dev} vlan 1 ethernet ${dev}/${port[1]}
        mac address-table dynamic 00:22:33:00:00:00 device ${dev} vlan 1 ethernet ${dev}/${port[1]}
    ]]
    ret = pcall(executeStringCliCommands,macs)
    if (ret==false) then
        setFailState()
    end
    local rc,values=wrlCpssDxChBrgFdbMacAddressTableCountGet(devEnv.dev,0)
    checkMacAddressCountResult(rc)
    printLog("Created MAC addresses")
    printMacAddressCount(values)
    return rc,values
end
local function doFdbClear(command, del_mode, fdb_values_before)
    local static_after, static_before
    local dynamic_after, dynamic_before
    static_before=fdb_values_before.static
    dynamic_before=fdb_values_before.dynamic
    ret=pcall(executeStringCliCommands,"end\nclear bridge "..command.." ${dev} "..del_mode)
    if (ret==false) then
        setFailState()
    end
    rc,values=wrlCpssDxChBrgFdbMacAddressTableCountGet(devEnv.dev,0)
    static_after=values.static
    dynamic_after=values.dynamic
    checkMacAddressCountResult(rc)
    printLog("MAC address count after clearing "..del_mode.." entries")
    printMacAddressCount(values)
    if (del_mode=="static") then
        if (dynamic_before~=dynamic_after) then
            printLog ("Number of dynamic entries before and after clearing FDB: "..to_string(dynamic_before).."/"..to_string(dynamic_after))
            setFailState()
        end
        if (static_after~=0) then
            printLog ("There are "..to_string(static_after).."static enrties after FDB clearing")
            setFailState()
        end
    elseif (del_mode=="dynamic") then
        if (static_before~=static_after) then
            printLog ("Number of static entries before and after clearing FDB: "..to_string(dynamic_before).."/"..to_string(dynamic_after))
            setFailState()
        end
        if (dynamic_after~=0) then
            printLog ("There are "..to_string(dynamic_after).."dynamic enrties after FDB clearing")
            setFailState()
        end
    elseif (del_mode=="all") then
        if (dynamic_after~=0 or static_after~=0) then
            printLog ("Failed to clear FDB table")
            setFailState()
        end
    end
    return values
end
local function doClearBridgeTest(hash_mode, command)
    local rc,ret,values
    printLog("------------------Hash mode: "..hash_mode..", Command: clear bridge "..command.."------------------")
    --Clear FDB Table
    wraplCpssDxChBrgFdbFlush(devEnv.dev,1);
    --Setting Hash Mode
    ret = pcall(executeStringCliCommands,"end\nconfigure\nbridge fdb-hash-mode "..hash_mode.." device ${dev}")
    if (ret==false) then
        setFailState()
    else
        rc,values=createMacEntries()
        --Clear Static MAC entries
        if rc==0 then
            values=doFdbClear(command, "static",values)
        end
        rc,values=createMacEntries()
        --Clear All MAC entries
        if rc==0 then
            values=doFdbClear(command, "all",values)
        end
    end
end
SUPPORTED_SIP_DECLARE(devNum,"SIP_5")
-- Test checks aging deamon that is not supported in GM
SUPPORTED_FEATURE_DECLARE(devEnv.dev, "NOT_BC2_GM")
local rc, values, hash_mode
rc, values = myGenWrapper(
                               "cpssDxChBrgFdbHashModeGet", {
                                 { "IN", "GT_U8", "dev", devEnv.dev },
                                 { "OUT", "CPSS_MAC_HASH_FUNC_MODE_ENT", "mode"}
                               }
                            )
hash_mode=values.mode
if (rc~=0) then
    printLog("Failed to Get Hash Mode")
    setFailState()
else
    if(not is_sip_6(devNum))then--sip6 not support those
        doClearBridgeTest("xor","type")
        doClearBridgeTest("crc","type")
    end

    doClearBridgeTest("crc-multi","type")

    if(not is_sip_6(devNum))then--sip6 not support those
        doClearBridgeTest("xor","interface")
        doClearBridgeTest("crc","interface")
    end

    doClearBridgeTest("crc-multi","interface")
    --Clear FDB Table (Dynamic and Static entries)
    wraplCpssDxChBrgFdbFlush(devEnv.dev,1);
    --Set Default value of FDB hash mode
    rc = myGenWrapper(
                       "cpssDxChBrgFdbHashModeSet", {
                           { "IN", "GT_U8", "dev", devEnv.dev },
                           { "IN", "CPSS_MAC_HASH_FUNC_MODE_ENT", "mode", hash_mode}
                        }
                     )
    if (rc~=0) then
        printLog("Failed to Set Hash Mode")
        setFailState()
    end
    executeStringCliCommands("end")
end

