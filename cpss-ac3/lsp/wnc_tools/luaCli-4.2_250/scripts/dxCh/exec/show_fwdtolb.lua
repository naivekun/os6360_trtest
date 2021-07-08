--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_fwdtolb.lua
--*
--* DESCRIPTION:
--*     'Forwarding to loopback/service port' related "show" commands.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssDxChMaximumMirrorAnalyzerIndex")


-- ************************************************************************
--  fwdToLbGeneralTriggerEnGet
---
--  @description  get general status of 'Forwarding to Loopback/Sevice port'
--                feature
--
--  @param command_data - a Command_Data object instance
--  @param devNum       - a device number
--
--  @return false       - on fail
--          true, array - on ok. Array item looks like:
--                        bobcat2 a0:   {rxProfile=<num>, txProfile=<num>,
--                                       tcEn={[0]=<bool>}}
--                        rest of sip5: {rxProfile=<x>, txProfile=<y>,
--                                       tcEn={[0]=<bool>,..., [7]=<bool>}}
--                        tcEn means "enabled per traffic class"
--
--
local function fwdToLbGeneralTriggerEnGet(command_data, devNum)
    local result      = {}
    local tcMax = trafficClassMaxGet(devNum)
    local profileMax = loopbackProfileMaxGet(devNum)

    if not is_sip_5_10(devNum) then
        -- tc is ignored by cpssDxChPortLoopbackEnableGet in bobcat2 a0
        tcMax = 0
    end

    for rxProf = 0, profileMax do
        for txProf = 0, profileMax do
            local line = {rxProfile=rxProf, txProfile=txProf, tcEn ={}}
            for tc = 0, tcMax do
                local isErr, result, values = genericCpssApiWithErrorHandler(
                    command_data, "cpssDxChPortLoopbackEnableGet",
                    {
                        {"IN", "GT_U8",    "devNum",     devNum},
                        {"IN", "GT_U32",   "srcProfile", rxProf},
                        {"IN", "GT_U32",   "trgProfile", txProf},
                        {"IN", "GT_U32",   "tc",         tc},
                        {"OUT", "GT_BOOL", "enablePtr"}
                })
                if isErr then
                    return false
                end
                line.tcEn[tc] = values.enablePtr
            end
            table.insert(result, line)
        end
    end

    return true, result
end

-- ************************************************************************
--  fwdToLbVidxMappingEnGet
---
--  @description  Get status of VIDX mapping for multi-target traffic forwarded
--                to loopback/service port and get a Loopback
--                eVIDX offset value.
--
--  @param command_data - a Command_Data object instance
--  @param devNum       - a device number
--
--  @return false       - on fail
--          true, <eVidx Mapping Enabled>, <eVidx Offset> - if ok.
--
local function fwdToLbVidxMappingEnGet(command_data, devNum)
    local isErr, result, values = genericCpssApiWithErrorHandler(
        command_data, "cpssDxChPortLoopbackEvidxMappingGet",
        {
            {"IN",  "GT_U8",   "devNum",     devNum},
            {"OUT", "GT_U32",  "vidxOffsetPtr"},
            {"OUT", "GT_BOOL", "enablePtr"}
    })
    if isErr then
        return false
    end
    return true, values.vidxOffsetPtr, values.enablePtr
end


-- ************************************************************************
--  fwdToLbEgressMirrorEnGet
---
--  @description  Get status of egress mirroring for packets forwarded to
--                loopback/service port.
--
--  @param command_data - a Command_Data object instance
--  @param devNum       - a device number
--
--  @return false                 - on fail
--          true, status(boolean) - if ok
--
local function fwdToLbEgressMirrorEnGet(command_data, devNum)
    local isErr, result, values = genericCpssApiWithErrorHandler(
        command_data, "cpssDxChPortLoopbackEnableEgressMirroringGet",
        {
            {"IN",  "GT_U8",  "devNum", devNum},
            {"OUT", "GT_BOOL","enablePtr"}
    })
    if isErr then
        return false
    end
    return true, values.enablePtr
end


local function showGeneralStatusPerDev(command_data, devNum, params)
    -- show 'forwarding to loopback/services port' feature's parameters:
    --     1) generic trigger status,
    --     2) 'eVidx mapping' status
    --     3) 'Egress mirroring' status.

    if not is_supported_feature(devNum, "FWD_TO_LB") then
        command_data:addErrorAndPrint(
            "device %d doesn't support the command", devNum)
        return
    end

    local out = {} -- table to be printed

    local isOk, egrMirrorEn           = fwdToLbEgressMirrorEnGet(command_data, devNum)
    -- it is not necessary check isOk. egrMirrorEn can be checked for nil instead.
    if egrMirrorEn ~= nil then
        table.insert(out,
                     string.format(
                         "Device #%d: Target Port ePort/eVlan Egress Mirroring is %s",
                         devNum,
                         egrMirrorEn and "enabled" or "disabled"))
    end

    -- it is not necessary check isOk. vidxOffset can be checked for nil instead.
    local isOk, vidxOffset, vidxMapEn = fwdToLbVidxMappingEnGet(command_data, devNum)
    if vidxMapEn ~= nil then
        if vidxMapEn then
            table.insert(out,
                         string.format("Device #%d: eVIDX mapping is enabled."
                                       .." VIDX offset is %d.",
                                       devNum, vidxOffset))
        else
            table.insert(out,
                         string.format("Device #%d: eVIDX mapping is disabled.",
                                       devNum))
        end
    end

    -- it is not necessary check isOk. statusArr can be checked for nil instead.
    local isOk, statusArr = fwdToLbGeneralTriggerEnGet(command_data, devNum)
    if statusArr ~= nil then

        table.insert(out, "General Trigger status (*-enabled):")

        local strYes, strNo
        if is_sip_5_10(devNum) then
            table.insert(out, "-----------------------------------------------------")
            table.insert(out, string.format("| dev#%-3d profiles ||     traffic classes           |", devNum))
            table.insert(out, "----------------------------------------------------|")
            table.insert(out, "| rx     | tx      || 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |")
            table.insert(out, "-----------------------------------------------------")
            strYes = "*"
            strNo  = " "
        else
            -- bobcat2 a0
            table.insert(out, "-------------------------------")
            table.insert(out, string.format("| dev %3d profiles || enabled |", devNum))
            table.insert(out, "-------------------||         |")
            table.insert(out, "| rx     | tx      ||         |")
            table.insert(out, "-------------------------------")
            strYes = "*      "
            strNo  = "       "
        end
        -- fill table body
        for _, item in ipairs(statusArr) do
            local line = string.format("| %-7d| %-7d ||", item.rxProfile, item.txProfile)
            for tc = 0, #item.tcEn do
                line = line .. string.format(" %s |",
                                             item.tcEn[tc] and strYes or strNo)
            end
            table.insert(out, line)
        end

        -- print a bottom frame line of drawn table
        if is_sip_5_10(devNum) then
            table.insert(out, "-----------------------------------------------------")
        else
            -- bobcat2 a0
            table.insert(out, "-------------------------------")
        end
    end

    reset_paused_printing()
    for _, line in ipairs(out) do
        print_nice(line)
    end
end


-- loopback profiles and loopback port number of specified ports
local function fwdToLbShowIfFunc(params)
    local out = {}

    table.insert(out, "Dev/Port  rx loopback profile  tx loopback profile  loopback port")
    table.insert(out, "--------  -------------------  -------------------  -------------")
    local emptyOutLen = #out
    local failedDevices={}
    local command_data = Command_Data()
    command_data:initAllInterfacesPortIterator(params)
    for _, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        if not is_supported_feature(devNum, "FWD_TO_LB") then
            if (not failedDevices[devNum]) then
                command_data:addErrorAndPrint(
                    "device %d doesn't support the command",
                    devNum)
                failedDevices[devNum] = true
            end
        else
            local direction = "CPSS_DIRECTION_INGRESS_E"
            local isErr, result, values = genericCpssApiWithErrorHandler(
                command_data, "cpssDxChPortLoopbackProfileGet",
                {
                    { "IN",  "GT_U8",                "devNum",    devNum},
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",   portNum},
                    { "IN",  "CPSS_DIRECTION_ENT",   "direction", direction},
                    { "OUT", "GT_U32",               "profilePtr"}
            })
            if isErr then
                break
            end
            local rxProfile = values.profilePtr

            local direction = "CPSS_DIRECTION_EGRESS_E"
            local isErr, result, values = genericCpssApiWithErrorHandler(
                command_data, "cpssDxChPortLoopbackProfileGet",
                {
                    { "IN",  "GT_U8",                "devNum",    devNum},
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",   portNum},
                    { "IN",  "CPSS_DIRECTION_ENT",   "direction", direction},
                    { "OUT", "GT_U32",               "profilePtr"}
            })
            if isErr then
                break
            end
            local txProfile = values.profilePtr

            local direction = "CPSS_DIRECTION_EGRESS_E"
            local isErr, result, values = genericCpssApiWithErrorHandler(
                command_data, "cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet",
                {
                    { "IN",  "GT_U8",                "devNum",    devNum},
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",   portNum},
                    { "OUT", "GT_U32",               "loopbackPortNumPtr"}
            })
            if isErr then
                break
            end
            local lbPort = values.loopbackPortNumPtr

            local devPortSt = to_string(devNum) .."/"..to_string(portNum)
            local lbDevPortSt  = to_string(devNum).."/"..to_string(lbPort)
            table.insert(out, string.format("%-8s  %-19d  %-19d  %-13s",
                                  devPortSt, rxProfile, txProfile, lbDevPortSt))
        end
    end

    if #out > emptyOutLen then
        reset_paused_printing()
        for _, line in ipairs(out) do
            print_nice(line)
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


local function fwdToLbShowPktTypeFwdFromCpuFunc(params)
    local out = {}
    table.insert(out, "device  from-cpu  unicast  multi-target")
    table.insert(out, "------  --------  -------  ------------")
    local emptyOutLen = #out

    local command_data = Command_Data()
    command_data:initAllDeviceRange(params)


    for _, devNum in command_data:getDevicesIterator() do
        if not is_supported_feature(devNum, "FWD_TO_LB") then
            command_data:addErrorAndPrint(
                "device %d doesn't support the command",
                devNum)

        else
            local isErr, result, values = genericCpssApiWithErrorHandler(
                command_data, "cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet",
                {
                    { "IN", "GT_U8",     "devNum",           devNum},
                    { "OUT", "GT_BOOL",   "fromCpuEnPtr"},
                    { "OUT", "GT_BOOL",   "singleTargetEnPtr"},
                    { "OUT", "GT_BOOL",   "multiTargetEnPtr"}
            })
            if isErr then
                break
            end
            local fromCpuEn = values.fromCpuEnPtr      and "yes" or "no"
            local singleEn  = values.singleTargetEnPtr and "yes" or "no"
            local multiEn   = values.multiTargetEnPtr  and "yes" or "no"
            table.insert(out, string.format("%-6d  %-8s  %-7s  %-7s",
                                            devNum, fromCpuEn, singleEn, multiEn))
        end
    end

    if #out > emptyOutLen then
        reset_paused_printing()
        for _, line in ipairs(out) do
            print_nice(line)
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


local function fwdToLbShowPktTypeToAnalyzerFunc(params)
    local out = {}
    table.insert(out, "device  analyzer index  enabled  loopback port")
    table.insert(out, "------  --------------  -------  -------------")
    local emptyOutLen = #out

    local command_data = Command_Data()
    command_data:initAllDeviceRange(params)


    for _, devNum in command_data:getDevicesIterator() do
        if not is_supported_feature(devNum, "FWD_TO_LB") then
            command_data:addErrorAndPrint(
                "device %d doesn't support the command",
                devNum)
        else
            local result, maxAnIx = wrlCpssDxChMaximumMirrorAnalyzerIndex()
            if result ~=0 then
                command_data:addErrorAndPrint(
                    "error on wrlCpssDxChMaximumMirrorAnalyzerIndex")
                break
            end
            for anIx=0, maxAnIx do
                local isErr, result, values = genericCpssApiWithErrorHandler(
                    command_data, "cpssDxChPortLoopbackPktTypeToAnalyzerGet",
                    {
                        {"IN",  "GT_U8",               "devNum",          devNum},
                        {"IN",  "GT_U32",              "analyzerIndex",   anIx},
                        {"OUT", "GT_PHYSICAL_PORT_NUM", "loopbackPortNumPtr"},
                        {"OUT", "GT_BOOL",              "enablePtr"}
                })
                if isErr then
                    break
                end

                local enStr  = values.enablePtr and "yes" or "no "
                local lbPort = values.loopbackPortNumPtr
                table.insert(out, string.format("%-6d  %-14d  %-7s  %-7d",
                                                devNum, anIx, enStr, lbPort))
            end
        end
    end

    if #out > emptyOutLen then
        reset_paused_printing()
        for _, line in ipairs(out) do
            print_nice(line)
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end




local function fwdToLbShowPktTypeToCpuFunc(params)
    local showAll = not params.enabledOnly
    local out = {}

    if not showAll then
        table.insert(out, "Only CPU codes which the feature is enabled for are shown.)")
    end
    table.insert(out, "\n")
    table.insert(out, "device  CPU code        enabled  loopback port")
    table.insert(out, "------  --------------  -------  -------------")
    local emptyOutLen = #out

    local command_data = Command_Data()
    command_data:initAllDeviceRange(params)


    for _, devNum in command_data:getDevicesIterator() do
        if not is_supported_feature(devNum, "FWD_TO_LB") then
            command_data:addErrorAndPrint(
                "device %d doesn't support the command",
                devNum)
        else
            for _, cpuCode in pairs(params.cpuCodeRange) do
                local apiCpuCode = dsa_cpu_code_to_api_convert(cpuCode)

                local isErr, result, values = genericCpssApiWithErrorHandler(
                    command_data, "cpssDxChPortLoopbackPktTypeToCpuGet",
                    {
                        {"IN",  "GT_U8",                    "devNum",  devNum},
                        {"IN",  "CPSS_NET_RX_CPU_CODE_ENT", "cpuCode", apiCpuCode},
                        {"OUT", "GT_PHYSICAL_PORT_NUM",     "loopbackPortNumPtr"},
                        {"OUT", "GT_BOOL",                  "enablePtr"}
                })
                if isErr then
                    break
                end
                if values.enablePtr or showAll then
                    local enStr = values.enablePtr and "yes" or "no "
                    local lbPort     = values.loopbackPortNumPtr
                    table.insert(out, string.format("%-6d  %-14d  %-7s  %-7d",
                                                    devNum, cpuCode, enStr, lbPort))
                end
            end
        end
    end

    if #out > emptyOutLen then
        reset_paused_printing()
        for _, line in ipairs(out) do
            print_nice(line)
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: show forward-to-loopback status
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show forward-to-loopback",
            "'Forwarding to Loopback Port' commands")

CLI_addCommand({"exec"}, "show forward-to-loopback status",
    {
        func = function(params)
            generic_all_device_func(showGeneralStatusPerDev, params)
        end,
        help = "'Forwarding to loopback port' status.",
        params = {
            { type = "named",
              "#all_device",
              mandatory = {"all_device"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: show forward-to-loopback status
--------------------------------------------------------------------------------
CLI_addCommand({"exec"}, "show interfaces forward-to-loopback",
    {
        func = fwdToLbShowIfFunc,
        help =
            "'forwarding to loopback port' options configured per port:\n"..
            "- source/target loopback profiles,\n"..
            "- loopback port number (used by FORWARD/FROM_CPU traffic)",
        params = {
            { type = "named",
              {format = "ethernet %port-range"},
              mandatory = {"ethernet"}
            }
        }
    }
)



CLI_addHelp("exec", "show forward-to-loopback packet-type-status", "'forwarding to loopback port' status for packet type(s)")
--------------------------------------------------------------------------------
-- command registration: show forward-to-loopback packet-type forward-from-cpu
--------------------------------------------------------------------------------
CLI_addCommand({"exec"}, "show forward-to-loopback packet-type-status forward-from-cpu",
    {
        func = fwdToLbShowPktTypeFwdFromCpuFunc,
        help =  "'forwarding to loopback port' status"..
                " for FORWARD/FROM_CPU packet type",
        params = {
            { type = "named",
              "#all_device",
              mandatory = {"all_device"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: show forward-to-loopback packet-type to-analyzer
--------------------------------------------------------------------------------
CLI_addCommand({"exec"}, "show forward-to-loopback packet-type-status to-analyzer",
    {
        func = fwdToLbShowPktTypeToAnalyzerFunc,
        help = "'forwarding to loopback port' status"..
               " for TO_ANALYZER packet type",
        params = {
            { type = "named",
              "#all_device",
              mandatory = {"all_device"}
            }
        }
    }
)

--------------------------------------------------------------------------------
-- command registration: show forward-to-loopback packet-type to-cpu
--------------------------------------------------------------------------------
CLI_addCommand({"exec"}, "show forward-to-loopback packet-type-status to-cpu",
    {
        func = fwdToLbShowPktTypeToCpuFunc,
        help = "'forwarding to loopback port' status"..
               " for TO_CPU packet type",
        params = {
            { type = "named",
              "#all_device",
              {format = "cpu-codes %cpu_code_range", name = "cpuCodeRange", help = "CPU codes range"},
              {format = "enabled-only", name = "enabledOnly", help = "show only CPU codes with enabled 'fowarding to lopback/service port'"},
              requirements = {["cpuCodeRange"] = {"all_device"}},
              mandatory = {"all_device", "cpuCodeRange"}
            }
        }
    }
)
