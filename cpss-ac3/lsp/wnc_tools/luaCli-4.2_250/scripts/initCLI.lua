--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* initCLI.lua
--*
--* DESCRIPTION:
--*       cpssInitSystem and LuaCLI configuration commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


-- ************************************************************************
--  initSystem
--        @description  The cpssInitSystem function
--
--        @param params         - params[boardParams][1] - boardIdx
--                - params[boardParams][2] - boardRevId
--                - params[boardParams][3] - reloadEeprom
--                              - noPorts will not initialize any port
--
--        @return       true if there was no error otherwise false
--

cmdLuaCLI_registerCfunction("luaCLI_getDevInfo")

function initSystem(params)
    if params.noPorts ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initSerdesDefaults"},
            {"IN","GT_U32","value", 0}
        })
    end
    if params.pexOnly ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initRegDefaults"},
            {"IN","GT_U32","value", 1}
        })
    else
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initRegDefaults"},
            {"IN","GT_U32","value", 0}
        })
    end

    if params.onlyPhase1and2 ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initSystemOnlyPhase1and2"},
            {"IN","GT_U32","value", 1}
        })
    end

    if params.disableInerrupts ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "initSystemWithoutInterrupts"},
            {"IN","GT_U32","value", 1}
        })
    end

    if params.doublePhysicalPorts ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN", "string", "namePtr", "doublePhysicalPorts"},
            {"IN","GT_U32","value", 1}
        })
    end

    if params.apEnable ~= nil then
        ret,val = myGenWrapper("appDemoDbEntryAdd",{
            {"IN","string","namePtr","apEnable"},
            {"IN","GT_U32","value",1}
        })
    end

    if params.fastBoot then
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
                }}})
    end

    if params.haBoot == nil and params.haBoot2 == nil and params.haBoot3 == nil then
        if params.numberTable == nil then
            -- indication to trigger 'auto init syatem' 
            ret,val = myGenWrapper("autoInitSystem",{})
        else
            ret,val = myGenWrapper("cpssInitSystem",{
                {"IN","GT_U32","boardIdx",params["numberTable"][1]},
                {"IN","GT_U32","boardRevId",params["numberTable"][2]},
                {"IN","GT_U32","reloadEeprom",params["numberTable"][3]}
            })
        end
    end

    if ret == 0 and params.fastBoot then
        ret = myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
                }}})
        if ret ~= 0 then
            print("Failed in cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E,...) ret=", ret)
            return false
        end
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
                }}})
    end

    if params.haBoot then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_INIT_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = true,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    elseif params.haBoot2 then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_INIT_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = true },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    elseif params.haBoot3 then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_INIT_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    end

    if params.haBoot ~= nil or params.haBoot2 ~= nil or params.haBoot3 ~= nil then
        if params.numberTable == nil then
            -- indication to trigger 'auto init syatem' 
            ret,val = myGenWrapper("autoInitSystem",{})
        else
            ret,val = myGenWrapper("cpssInitSystem",{
                {"IN","GT_U32","boardIdx",params["numberTable"][1]},
                {"IN","GT_U32","boardRevId",params["numberTable"][2]},
                {"IN","GT_U32","reloadEeprom",params["numberTable"][3]}
            })
        end
    end

    if ret == 0 and params.haBoot then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E")
        ret = myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = true,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
        if ret ~= 0 then
            print("Failed in cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E,...) ret=", ret)
            return false
        end
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = true,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    elseif ret == 0 and params.haBoot2 then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E")
        ret = myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = true },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
        if ret ~= 0 then
            print("Failed in cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E,...) ret=", ret)
            return false
        end
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = true },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    elseif ret == 0 and params.haBoot3 then
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E")
        ret = myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
        if ret ~= 0 then
            print("Failed in cpssSystemRecoveryStateSet(CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E,...) ret=", ret)
            return false
        end
        print("cpssSystemRecoveryStateSet CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E")
        myGenWrapper("cpssSystemRecoveryStateSet", {
            {"IN", "CPSS_SYSTEM_RECOVERY_INFO_STC", "info", {
                    systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
                    systemRecoveryMode = {
                        continuousRx = false,
                        continuousTx = false,
                        continuousAuMessages = false,
                        continuousFuMessages = false,
                        haCpuMemoryAccessBlocked = false },
                    systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
                }}})
    end

    local dev_list = wrlDevList();

    -- ini block for dxCh family only 
    if dxCh_family == true then
      for dummy,dev in pairs(dev_list) do
          system_capability_managment({devID = dev});
      end
      print("Init system returned:"..to_string(ret))

      -- Load iDebug for Bobcat2
      if dxCh_family and (is_sip_5(nil) or DeviceFamily == "CPSS_PP_FAMILY_DXCH_XCAT3_E") then
          require("common/debug/iDebug")
      end
    end

    -- init device specific global parameters
    fillDeviceEnvironment()

    if fs.exists("cmdDefs.lua") then
        require_safe("cmdDefs")
    end

    if (ret==0) then return true
    else return false end
end

local initSystemParams_named_params =
{
    type="named",
    { format="noPorts",name="noPorts",  help="Will not initialize any port during operation"},
    { format="pexOnly",name="pexOnly",  help="Initializing only pex access during cpss init"},
    { format="onlyPhase1and2",name="onlyPhase1and2",  help="Initializing only phase 1 and phase 2 (as minimal basic init)"},
    { format="disableInerrupts",name="disableInerrupts",  help="Will not initialize and bind interrupts during operation"},
    { format="doublePhysicalPorts",name="doublePhysicalPorts",  help="Bobcat3 : Initializing with 512 physical ports"},
    { format="apEnable",name="apEnable",  help="Initializing ap engine"},
    { format="fastBoot",name="fastBoot",  help="Try to connect to running AP"},
    { format="haBoot",name="haBoot",  help="HA test"},
    { format="haBoot2",name="haBoot2",  help="HA test"},
    { format="haBoot3",name="haBoot3",  help="HA test"},
}

-------------------------------------------------------
-- command registration: cpssInitSystem
-------------------------------------------------------
local initSystemParams = {
    func=initSystem,
    help="Init system",
    params={
        { type="values",
            { format="%init_system",name="numberTable",  help="enter configuration pattern"}
        },
        initSystemParams_named_params
    }
}

CLI_addCommand("exec", "cpssInitSystem", initSystemParams)
-- also with lower case... just to make it easier
CLI_addCommand("exec", "cpssinitsystem", initSystemParams)

-------------------------------------------------------
-- command registration: autoInitSystem
-------------------------------------------------------
local autoSystemParams = {
    func=initSystem,
    help="Auto Init system (determines init params according to PCI/PEX scanning)",
    params={
        initSystemParams_named_params
    }
}

CLI_addCommand("exec", "autoInitSystem", autoSystemParams)
-- WITHOUT lower case to allow writing 'auto' and pressing 'tab' to get the command running
-- CLI_addCommand("exec", "autoinitsystem", autoSystemParams)


-- ************************************************************************
---
--  updateCLItypes()
--
--        @description   - run all registered callbacks in typeCallBacksList
--
--        @param         - devEnv
--
function updateCLItypes()

    -- is cpssInitSystem done?
    local sysInfo = luaCLI_getDevInfo();
    if type(sysInfo) ~= "table" then
        -- case of invoking before cpssInitSystem
        return;
    end

    local i
    local callbackFun
    local ret,err

    -- iterate thru all callbacks
    for i = 1, #typeCallBacksList do
        callbackFun = typeCallBacksList[i]
        ret,err = pcall(callbackFun)
        if not ret then
            print("updateCLItypes() failure in callback: "..err)
        end
    end
end

-- update device depend types by calling API's added to typeCallBacksList
updateCLItypes()

