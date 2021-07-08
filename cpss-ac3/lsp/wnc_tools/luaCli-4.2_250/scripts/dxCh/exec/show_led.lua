--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_led.lua
--*
--* DESCRIPTION:
--*       LED related show commands.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDevicePortGroupNumberGet")
cmdLuaCLI_registerCfunction("wrlCpssDeviceFamilyGet")
-- constants

local legacyEnums = { -- DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; xCat3; Lion; Lion2
    blinkDutyCycle = {
        ["CPSS_LED_BLINK_DUTY_CYCLE_0_E"] = {value = 0, descr = "25% on, 75% off"},
        ["CPSS_LED_BLINK_DUTY_CYCLE_1_E"] = {value = 1, descr = "50% on, 50% off"},
        ["CPSS_LED_BLINK_DUTY_CYCLE_2_E"] = {value = 2, descr = "50% on, 50% off"},
        ["CPSS_LED_BLINK_DUTY_CYCLE_3_E"] = {value = 3, descr = "75% on, 25% off"},
    },
    blinkDuration = {
        ["CPSS_LED_BLINK_DURATION_0_E"] = {value = 0, descr = "0 - LED blink period is 8.1 ms   (if core clock is 270Mhz)"},
        ["CPSS_LED_BLINK_DURATION_1_E"] = {value = 1, descr = "1 - LED blink period is 16.2 ms  (if core clock is 270Mhz)"},
        ["CPSS_LED_BLINK_DURATION_2_E"] = {value = 2, descr = "2 - LED blink period is 32.5 ms  (if core clock is 270Mhz)"},
        ["CPSS_LED_BLINK_DURATION_3_E"] = {value = 3, descr = "3 - LED blink period is 65.1 ms  (if core clock is 270Mhz)"},
        ["CPSS_LED_BLINK_DURATION_4_E"] = {value = 4, descr = "4 - LED blink period is 130.3 ms (if core clock is 270Mhz)"},
        ["CPSS_LED_BLINK_DURATION_5_E"] = {value = 5, descr = "5 - LED blink period is 260.7 ms (if core clock is 270Mhz)"},
        ["CPSS_LED_BLINK_DURATION_6_E"] = {value = 6, descr = "6 - LED blink period is 524.4 ms (if core clock is 270Mhz)"}
    },
    stretchLen = {
        ["CPSS_LED_PULSE_STRETCH_0_NO_E"] = { value = 0, descr = "0 - No stretch will be done on any indicaiton"},
        ["CPSS_LED_PULSE_STRETCH_1_E"]    = { value = 1, descr = "1 - LED clock stretching: 4-8.1 msec"},
        ["CPSS_LED_PULSE_STRETCH_2_E"]    = { value = 2, descr = "2 - LED clock stretching: 32.5-65.1 msec"},
        ["CPSS_LED_PULSE_STRETCH_3_E"]    = { value = 3, descr = "3 - LED clock stretching: 65.1-130.3 msec"},
        ["CPSS_LED_PULSE_STRETCH_4_E"]    = { value = 4, descr = "4 - LED clock stretching: 130.3-260.7 msec"},
        ["CPSS_LED_PULSE_STRETCH_5_E"]    = { value = 5, descr = "5 - LED clock stretching: 260.7-521.4 msec"},
    }
}
local eArchEnums = { -- Bobcat2, Caelum, Aldrin, AC3X, Bobcat3
    blinkDutyCycle = {
        ["CPSS_LED_BLINK_DUTY_CYCLE_0_E"] = {value = 0, descr = "25% on, 75% off"},
        ["CPSS_LED_BLINK_DUTY_CYCLE_1_E"] = {value = 1, descr = "50% on, 50% off"},
        ["CPSS_LED_BLINK_DUTY_CYCLE_2_E"] = {value = 2, descr = "50% on, 50% off"},
        ["CPSS_LED_BLINK_DUTY_CYCLE_3_E"] = {value = 2, descr = "75% on, 25% off"},
    },
    blinkDuration = {
        ["CPSS_LED_BLINK_DURATION_0_E"] = {value = 0, descr = "0 - LED blink period is 32 ms"},
        ["CPSS_LED_BLINK_DURATION_1_E"] = {value = 1, descr = "1 - LED blink period is 64 ms"},
        ["CPSS_LED_BLINK_DURATION_2_E"] = {value = 2, descr = "2 - LED blink period is 128 ms"},
        ["CPSS_LED_BLINK_DURATION_3_E"] = {value = 3, descr = "3 - LED blink period is 256 ms"},
        ["CPSS_LED_BLINK_DURATION_4_E"] = {value = 4, descr = "4 - LED blink period is 512 ms"},
        ["CPSS_LED_BLINK_DURATION_5_E"] = {value = 5, descr = "5 - LED blink period is 1024 ms"},
        ["CPSS_LED_BLINK_DURATION_6_E"] = {value = 6, descr = "6 - LED blink period is 1536 ms"},
        ["CPSS_LED_BLINK_DURATION_7_E"] = {value = 7, descr = "7 - LED blink period is 2048 ms"},
    },
    stretchLen = {
        ["CPSS_LED_PULSE_STRETCH_0_NO_E"] = {value = 0, descr = "0 - No stretch will be done on any indicaiton"},
        ["CPSS_LED_PULSE_STRETCH_1_E"]    = {value = 1, descr = "1 - LEDCLK divided by 10000"},
        ["CPSS_LED_PULSE_STRETCH_2_E"]    = {value = 2, descr = "2 - LEDCLK divided by 500000"},
        ["CPSS_LED_PULSE_STRETCH_3_E"]    = {value = 3, descr = "3 - LEDCLK divided by 1000000"},
        ["CPSS_LED_PULSE_STRETCH_4_E"]    = {value = 4, descr = "4 - LEDCLK divided by 1500000"},
        ["CPSS_LED_PULSE_STRETCH_5_E"]    = {value = 5, descr = "5 - LEDCLK divided by 2000000"},
        ["CPSS_LED_PULSE_STRETCH_6_E"]    = {value = 6, descr = "6 - LEDCLK divided by 2500000"},
        ["CPSS_LED_PULSE_STRETCH_7_E"]    = {value = 7, descr = "7 - LEDCLK divided by 3000000"}
    },
    clockFreq = {
        ["CPSS_LED_CLOCK_OUT_FREQUENCY_500_E"]  = {descr = "0 - 500 KHz"},
        ["CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E"] = {descr = "1 - 1 MHz"},
        ["CPSS_LED_CLOCK_OUT_FREQUENCY_2000_E"] = {descr = "2 - 2 MHz"},
        ["CPSS_LED_CLOCK_OUT_FREQUENCY_3000_E"] = {descr = "3 - 3 MHz"},
    }
}


-- *****************************************************************************
--  ledIfCountPerDev
--
--  @description return a number of LED interfaces in the device. Note
--               in case of Lion/Lion2 the <LED interface number per device> =
--               <LED interface number per core> * <core number +1>
--
--  @param devNum - a device number
--
--  @return
--     if success: <LED Interfaces count per device>,
--                 <LED Interfaces count per device core>
--     if failed: nil, <error message>
--
-- *****************************************************************************
function ledIfCountPerDev(devNum)
    if devNum == nil then
        return nil, "ledIfCountPerDev: device number can't be emtpy"
    end
    local rc, values = myGenWrapper(
        "cpssDxChCfgDevInfoGet",{
            {"IN", "GT_U8", "devNum", devNum},
            {"OUT", "CPSS_DXCH_CFG_DEV_INFO_STC", "devInfoPtr"}
    })
    if rc ~= 0 then
        return nil, returnCodes[rc]
    end
    local ledCntPerCore = values.devInfoPtr.genDevInfo.numOfLedInfPerPortGroup
    local rc, coreNum = wrLogWrapper("wrlCpssDevicePortGroupNumberGet", "(devNum)", devNum)
    if rc ~= 0 then
        return nil, returnCodes[rc]
    end
    return coreNum * ledCntPerCore, ledCntPerCore
end

-- *****************************************************************************
--  ledIfDev2Core
--
--  @description transform a LED interface number (global for device) to
--               LED interface number per core and PortGroup (core) number.
--
--  @param ledIf      - a LED interface number per device
--  @param ledNumDev  - a number of LED interfaces in device. Can be omitted.
--  @param ledNumCore - a number of LED interfaces in device's core. Can be omitted.

--  @return  <LED interface number per core>, <Core Number> - for multi-core devices
--           <LED interface number per device> - for single-core device
--
function ledIfDev2Core(ledIf, ledNumDev, ledNumCore)
    if ledNumDev == ledNumCore then
        return ledIf
    else
        return ledIf % ledNumCore, math.floor(ledIf / ledNumCore)
    end

end

--*****************************************************************************
-- ledGlobalInfoGet
--
-- @description  get LED information unbound to any LED interface
--
-- @param command_data - a Command_Data object instance
-- @param devNum - a device number
--
-- @return
--    false - if failed
--    true, table - if ok. A table contains keys:
--           isCpuIndEn    - (bool) CPU indications are enabled instead of port 27
--           isTxqStatusEn - (bool) TxQ Status indication enabled for FlexLink ports in XG mode
--           isLedSync     - (bool) LED streams are sinchronized
--         Any key can be absent if it is not applicable for a device.
--
local function ledGlobalInfoGet(command_data, devNum)
    local info = {}        -- will be returned
    local rc,values = myGenWrapper(
        "cpssDxChLedStreamCpuOrPort27ModeGet", {
            {"IN", "GT_U8", "devNum", devNum},
            {"OUT", "CPSS_DXCH_LED_CPU_OR_PORT27_ENT", "indicatedPort"}})
    if rc == 0x1e or rc == 0x10 then  -- GT_NOT_APPLICABLE_DEVICE, GT_NOT_SUPPORTED
        -- ignore
    elseif rc ~= 0 then
        command_data:setFailLocalStatus()
        command_data:addError(returnCodes[rc])
    else
        info.isCpuIndEn = (values.indicatedPort == "CPSS_DXCH_LED_CPU_E")
    end


    local rc, values = myGenWrapper(
        "cpssDxChLedStreamHyperGStackTxQStatusEnableGet", {
            {"IN", "GT_U8", "devNum", devNum},
            {"OUT", "GT_BOOL", "enable"}})
    if rc == 0x1e or rc == 0x10 then  -- GT_NOT_APPLICABLE_DEVICE, GT_NOT_SUPPORTED
        -- ignore
    elseif rc ~= 0 then
        command_data:setFailLocalStatus()
        command_data:addError(returnCodes[rc])
    else
        info.isTxqStatusEn = values.enable
    end


    local rc, values = myGenWrapper(
        "cpssDxChLedStreamSyncLedsEnableGet", {
            {"IN", "GT_U8", "devNum", devNum},
            {"OUT", "GT_BOOL", "enable"}})
    if rc == 0x1e or rc == 0x10 then  -- GT_NOT_APPLICABLE_DEVICE, GT_NOT_SUPPORTED
        -- ignore
    elseif rc ~= 0 then
        command_data:setFailLocalStatus()
        command_data:addError(returnCodes[rc])
    else
        info.isLedSync = values.enable
    end
    return true, info
end

--*****************************************************************************
-- ledInterfaceInfoGet
--
-- @description  get a LED interface settings
--
-- @param command_data - a Command_Data object instance
-- @param devNum       - a device number
-- @param ledIf        - a LED interface number. If core is not nil it is
--                       treated as a LED interface per device's core.
-- @param core      - a core (portGroup) number in multi-core device. In                        --                        case of single-core device it should be nil
--
-- @return false       - on fail
--         true, table - if ok. table is a CPSS_LED_CONF_STC.
--
function ledInterfaceInfoGet(command_data, devNum, ledIf, core)

    local portGroupBmp = core and bit_shl(1, core) or 0xFFFFFFFF

    local isFailed, rc, values = genericCpssApiWithErrorHandler(
        command_data, "cpssDxChLedStreamPortGroupConfigGet",
        {
            { "IN",     "GT_U8",                "devNum",             devNum},
            {"IN",      "GT_PORT_GROUPS_BMP",   "portGroupsBmp",      portGroupBmp},
            { "IN",     "GT_U32",               "ledInterfaceNum",    ledIf},
            { "OUT",    "CPSS_LED_CONF_STC",    "ledConfPtr"}
    })

    if isFailed then
        return false
    else
        return true, values.ledConfPtr
    end
end


--*****************************************************************************
-- ledClassInfoGet
--
-- @description  get a LED class specific settings
--
-- @param command_data - a Command_Data object instance
-- @param devNum       - a device number
-- @param ledIf        - a LED interface number. If core is not nil it is
--                       treated as a LED interface per device's core.
-- @param core      - a core (portGroup) number in multi-core device. In
--                       case of single-core device it should be nil
-- @param class        - a LED class number
-- @param portType     - a CPSS_DXCH_LED_PORT_TYPE_ENT enum value
--                       If nil it will be CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E
--
-- @return false       - on fail
--         true, table - if ok. table is a CPSS_LED_CLASS_MANIPULATION_STC.

--
function ledClassInfoGet(command_data, devNum, ledIf, core, class, portType)

    if not portType then portType = "CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E" end
    local portGroupBmp = core and bit_shl(1, core) or 0xFFFFFFFF

    local isFailed, rc, values = genericCpssApiWithErrorHandler(
        command_data, "cpssDxChLedStreamPortGroupClassManipulationGet",
        {
            {"IN",  "GT_U8",                           "devNum",          devNum},
            {"IN",  "GT_PORT_GROUPS_BMP",              "portGroupsBmp",   portGroupBmp},
            {"IN",  "GT_U32",                          "ledInterfaceNum", ledIf},
            {"IN",  "CPSS_DXCH_LED_PORT_TYPE_ENT",     "portType",        portType},
            {"IN",  "GT_U32",                          "classNum",        class},
            {"OUT", "CPSS_LED_CLASS_MANIPULATION_STC", "classParamsPtr"}
    })
    if isFailed then
        return false
    else
        return true, values.classParamsPtr
    end
end


--*****************************************************************************
-- ledGroupsInfoGet
--
-- @description get all LED interface groups information
--
-- @param command_data - a Command_Data object instance
-- @param devNum       - a device number
-- @param ledIf        - a LED interface number. If core is not nil it is
--                       treated as a LED interface per device's core.
-- @param core         - a core (portGroup) number in multi-core device. In
--                       case of single-core device it should be nil
-- @param portType     - a CPSS_DXCH_LED_PORT_TYPE_ENT enum value. Is ignored
--                       by eArch-devices.
--
-- @return false       - on fail
--         true, table - if ok. table looks like
--                       { [group0] = group0Info,
--                         [group1] = group1Info,
--                         [group2] = group2Info,
--                         [group3] = group3Info }
--                       Group info - that is a CPSS_LED_GROUP_CONF_STC.
--
local function ledGroupsInfoGet(command_data, devNum, ledIf, core, portType)
    local portGroupsBmp = core and bit_shl(1, core) or 0xFFFFFFFF
    local info ={}
    local isOk = true
    for group=0,3 do
        rc, values = myGenWrapper(
            "cpssDxChLedStreamPortGroupGroupConfigGet", {
                {"IN", "GT_U8",                       "devNum",          devNum},
                {"IN", "GT_PORT_GROUPS_BMP",          "portGroupsBmp",   portGroupsBmp},
                {"IN", "GT_U32",                      "ledInterfaceNum", ledIf},
                {"IN", "CPSS_DXCH_LED_PORT_TYPE_ENT", "portType",        portType},
                {"IN", "GT_U32",                      "groupNum",        group},
                {"OUT", "CPSS_LED_GROUP_CONF_STC",    "groupParamsPtr"}
        })
        if rc == 0 then
            info[group] = values.groupParamsPtr
        elseif rc == 4 and group > 0 then -- GT_BAD_PARAM
            -- group > 0 means that at least one iteration (group = 0) with
            -- the same devNum, ledIf, portType was passed ok.
            -- So bad param is group.
            break
        elseif rc == 0x10 then
            isOk = false
            command_data:addWarning(
                "Device %d doesn't support LED group reading", devNum)
            break
        else
            isOk = false
            command_data:setFailDeviceAndLocalStatus()
            command_data:addError(
                "Error on LED Group #%d reading (device %d, LED interface %d): %s",
                group, devNum, ledIf, returnCodes[rc])
        end
    end
    if info == {} then info = nil end
    return isOk, info
end


--*****************************************************************************
-- ledInfoPerPortGet
--
-- @description  get LED information relevant to port
--
-- @param command_data - a Command_Data object instance
-- @param devNum       - a device number
-- @param portNum      - a port number
--
-- @returrn false       - on fail
--          true, info - if ok. info is a table:
--          { posInStream    = <number>,
--            isExtraSgmiiEn = <bool>,
--            classesInvert  = { {0, <bool>},
--                               {1, <bool>), ...
--                             }
--           }
--           Any info table's key can be absent if it is not
--           applicable for a device.
--
function ledInfoPerPortGet(command_data, devNum, portNum)
    local posInStream
    local isExtraSgmiiEn
    local classesInvertTab
    local classesIndicationTab

    local isEarch = is_sip_5(devNum)
    local family = wrLogWrapper("wrlCpssDeviceFamilyGet","(devNum)",devNum)
    if not isEarch
        and family ~= "CPSS_PP_FAMILY_DXCH_XCAT3_E"
        and family ~= "CPSS_PP_FAMILY_DXCH_XCAT2_E"
    then
        -- skip the device
        return
    end

    if isEarch or family == "CPSS_PP_FAMILY_DXCH_XCAT3_E" then -- xCat3; Bobcat2
        local apiName = "cpssDxChLedStreamPortPositionGet"
        local isError, rc, values = genericCpssApiWithErrorHandler(
            command_data, apiName, {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                { "OUT", "GT_U32", "positionPtr"},
        })
        posInStream = isError and nil or values.positionPtr
    end

    if family == "CPSS_PP_FAMILY_DXCH_XCAT2_E" then -- xCat2 only
        local apiName = "cpssDxChLedStreamPortFlexTriSpeedIndicationEnableGet"
        local isError, rc, values = genericCpssApiWithErrorHandler(
            command_data, apiName, {
                {"IN", "GT_U8",                "devNum",    devNum},
                {"IN", "GT_PHYSICAL_PORT_NUM", "portNum",   portNum},
                {"OUT","GT_BOOL",              "enablePtr"}}
        )
        isExtraSgmiiEn = isError and nil or values.enablePtr
    end

    if isEarch then
        classCount = 6          -- eArch devices have 6 LED classes
        classesInvertTab = {}
        classesIndicationTab = {}
        local apiName = "cpssDxChLedStreamPortClassPolarityInvertEnableGet"
        local indicationApi = "cpssDxChLedStreamPortClassIndicationGet"
        for class=0,classCount-1 do
            local rc, values = myGenWrapper(
                apiName, {
                    { "IN",  "GT_U8", "devNum", devNum},
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                    { "IN",  "GT_U32", "classNum", class},
                    { "OUT", "GT_BOOL", "invertEnablePtr"}
            })
            if rc == 0 then
                table.insert(classesInvertTab, {class, values.invertEnablePtr})
            elseif rc == 0x12 then --GT_NOT_INITIALIZED
                    command_data:addWarning("device %d, port %d:" ..
                      " a class invert polarity can't be read because"..
                      " a port's slot in a LED stream is not specified",
                      devNum, portNum)
                    break
            else
                command_data:handleCpssErrorDevPort(
                    rc, "getting a LED class" .. class .." invert polarity",
                    devNum, portNum)
            end
            local rc, values = myGenWrapper(
                indicationApi, {
                    { "IN",  "GT_U8", "devNum", devNum},
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                    { "IN",  "GT_U32", "classNum", class},
                    { "OUT", "CPSS_DXCH_LED_INDICATION_ENT", "indicationPtr"}
            })
            if rc == 0 then
                table.insert(classesIndicationTab, {class, classIndicationStrGet(values["indicationPtr"])})
            else
                command_data:handleCpssErrorDevPort(
                    rc, "getting a LED class" .. class .." indication",
                    devNum, portNum)
            end
        end
    end
    return true, { posInStream    = posInStream,
                   isExtraSgmiiEn = isExtraSgmiiEn,
                   classesInvert  = (classesInvertTab=={}) and nil or classesInvertTab,
                   classesIndication = classesIndicationTab}
end


-- *****************************************************************************
--  printNameValueList
--
--  @description print a list (table) of pairs name - vale.
--
--  @param tab  - a table to be printed. Looks like
--               { {name1, value1},
--                 {name2, value2},
--                 ...,
--                 {nameN, valueN} }
--  @return nothing
--
-- *****************************************************************************
local function printNameValueList(tab)
    local w1, w2

    -- calculate columns' minimal width to be enougth to display all values
    local maxw1, maxw2 = 0, 0
    for _, row in pairs(tab) do
        if #row >= 2 then
            w1, w2  = #row[1], #row[2]
            maxw1 = (w1 > maxw1) and w1 or maxw1
            maxw2 = (w2 > maxw2) and w2 or maxw2
        end
    end
    w1, w2 = maxw1, maxw2

    -- specify format string for table row output. If w1=10,w2=20: "%-10s, %-20s"
    local frmt = string.format("%%-%ds %%-%ds", w1, w2)

    -- print the table row by row
    for _, row in pairs(tab) do
        if #row >= 2 then
            print_nice(string.format(frmt, row[1], row[2]))
        end
    end
end


-- *****************************************************************************
--  showLedInfo
--
--  @description show some global LED information.
--
--  @param params   - not used.
--
--  @return  true on success, otherwise false and error message
--
local function showLedInfo(params)
    local devices = (params.devNum == "all") and wrlDevList() or {params.devNum}
    local command_data = Command_Data()
    for _, devNum in ipairs(devices) do
        local ledIfCount, errMsg = ledIfCountPerDev(devNum)
        if ledIfCount == nil then
            command_data:setFailLocalStatus()
            command_data:addError("an error occured during getting of LED"..
                                  " interfaces count per device #%d: %s",
                                  devNum, errMsg)
            ledIfCount = "ERROR"
        end
        local isOk, tab = ledGlobalInfoGet(command_data, devNum)

        local isCpuIndEn = tab.isCpuIndEn
        local isTxqStatusEn = tab.isTxqStatusEn
        local isLedSync = tab.isLedSync

        -- display info
        print("-------------------------------------------------------")
        print("Device #:                                         "
                  .. devNum)
        print("LED interfaces count:                             "
                  .. ledIfCount)

        if isCpuIndEn ~= nil then
            print("CPU instead of port 27 indications:               "
                      .. (isCpuIndEn and "Yes" or "No"))
        end

        if isTxqStatusEn ~= nil then
            print("TxQ Status indication (FlexLink ports, XG mode):  "
                      .. (isTxqStatusEn and "Yes" or "No"))
        end

        if isLedSync ~= nil then
            print("LED streams sinchronization:                      "
                      .. (isLedSync and "Yes" or "No"))
        end

        command_data:updateStatus()
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end



-- *****************************************************************************
--  parseLedIfAttr
--
--  @description transform a struct CPSS_LED_CONF_STC to a text view in format
--               "item name - item value" and move it into the table.
--
--  @param command_data  - Command_Data class of the outer CLI command handler.
--                         Used for error handling.
--  @param ledConf       - a CPSS_LED_CONF_STC struct to be parsed
--  @param tabOutput           - a table to gather text view results.
--  @return  nothing
--
local function parseLedIfAttr(command_data, ledConf, isLegacy, tabOutput)
    local enums = isLegacy and legacyEnums or eArchEnums

    -- stream-order-by
    local val
    val = (ledConf.ledOrganize == "CPSS_LED_ORDER_MODE_BY_PORT_E"
               and "Port" or "Class")
    table.insert(tabOutput, {"Stream is ordered by:", val})

    -- disable-on-link-down
    val = (ledConf.disableOnLinkDown and "Yes" or "No")
    table.insert(tabOutput, {"Link down affects on other indications:", val})

    -- stream-length
    val = ledConf.ledStart .. ".."..ledConf.ledEnd
    table.insert(tabOutput, {"Stream first..last bits:", val})

    -- blink0 duty cycle
    local val = enums.blinkDutyCycle[ledConf.blink0DutyCycle].descr
    if not val then
        command_data:addError("Device #%d, LED interface %d. Unknown Blink0 Duty Cycle value: %s",
                              ledConf.blink0DutyCycle)
        val = "ERROR"
    end
    table.insert(tabOutput, {"Blink0 duty cycle:", val})

    -- blink0 duration
    val = enums.blinkDuration[ledConf.blink0Duration].descr
    if not val then
        command_data:addError("Device #%d, LED interface %d. Unknown Blink0 Duty Duration value: %s",
                              ledConf.blink0Duration)
        val = "ERROR"
    end
    table.insert(tabOutput, {"Blink0 duration:", val})

    -- blink1 duty cycle
    local val = enums.blinkDutyCycle[ledConf.blink1DutyCycle].descr
    if not val then
        command_data:addError("Device #%d, LED interface %d. Unknown Blink0 Duty Cycle value: %s",
                              ledConf.blink1DutyCycle)
        val = "ERROR"
    end
    table.insert(tabOutput, {"Blink1 duty cycle:", val})

    -- blink1 duration
    val = enums.blinkDuration[ledConf.blink1Duration].descr
    if not val then
        command_data:addError("Device #%d, LED interface %d. Unknown Blink0 Duty Duration value: %s",
                              ledConf.blink1Duration)
        val = "ERROR"
    end
    table.insert(tabOutput, {"Blink1 duration:", val})

    -- stretch-cfg
    local attrName = isLegacy and "Stretch length" or "Stretching divider"
    val = enums.stretchLen[ledConf.pulseStretch].descr
    if not val then
        command_data:addError("Device #%d, LED interface %d. Unknown Pulse Stretch value: %s",
                              ledConf.ledConf.pulseStretch)
        val = "ERROR"
    end
    table.insert(tabOutput, {attrName .. ":", val})

    -- clock-freq
    if not isLegacy then
        val = enums.clockFreq[ledConf.ledClockFrequency].descr
        if not val then
            command_data:addError("Device #%d, LED interface %d. Unknown Clock Frequency value: %s",
                                  ledConf.ledConf.pulseStretch)
            val = "ERROR"
        end
        table.insert(tabOutput, {"A LEDCLK frequency:", val})
    end

    if isLegacy then
        -- clock-invert
        val = (ledConf.clkInvert and "Yes" or "No")
        table.insert(tabOutput, {"Invert LEDCLK signal:", val})

        -- dual-media-class-5
        val = (ledConf.class5select == "CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E"
                   and "Half Duplex" or "Fiber Link Up")
        table.insert(tabOutput, {"A dual-media (combo) ports class 5 indication:", val})

        -- dual-media-class-13
        val = (ledConf.class13select == "CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E"
                   and "Link Down" or "Copper Link Up")
        table.insert(tabOutput, {"A dual-media (combo) ports class 13 indication:", val})
    end
end

-- *****************************************************************************
--  gatherClassInfo
--
--  @description get class info CPSS_LED_CLASS_MANIPULATION_STC
--               per LED interface and gather information into the output table
--               to be print later. First line of the table is a table header.
--
--  @param command_data  - Command_Data class of the outer CLI command handler.
--                         Used for error handling.
--  @param devNum        - a device number
--  @param ledIfCore     - a LED interface number per core. portGroupBmp,
--                         ledIfCore are calculated from a LED
--                         inverface number per device
--  @param core          - a device's core number. In case of single-core device
--                         it should be nil!
--  @param isLegacy      - if this device is eArch device (false )or legacy
--                         device (true)
--
--  @return  two tables: 1) an output table and
--                       2) table with output table columns' width
--
local function gatherClassInfo(command_data, devNum, ledIfCore, core, isLegacy)
    local outTable = {}
    local widthTable = {}
    local family = wrLogWrapper("wrlCpssDeviceFamilyGet","(devNum)", devNum)
    local isLion =
        family == "CPSS_PP_FAMILY_DXCH_LION_E" or
        family == "CPSS_PP_FAMILY_DXCH_LION2_E"

    local portTypes = (isLegacy and not isLion)
        and {"CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E", "CPSS_DXCH_LED_PORT_TYPE_XG_E"}
        or  {"CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E"}

    -- eArch devices have 6 classes. Legacy devices have 14 classes.
    local classCount = isLegacy and 14 or 6

    -- insert a table header.
    if not isLegacy then        -- eArch
        table.insert(outTable, {"class", "blink", "force", "stretch", "DisableOnLinkDown"})
        widthTable = {6, 12, 10, 9, 17}
    elseif isLion then          -- Lion, Lion2
        table.insert(outTable, {"class", "invert", "blink", "force"})
        widthTable = {6, 7, 12, 10}
    else                        -- legacy not Lion/Lion2 devices
        table.insert(outTable, {"portType", "class", "invert", "blink", "force"})
        widthTable = {10, 6, 7, 12, 10}
    end

    for _, portType in pairs(portTypes) do
        for class=0, classCount-1 do
            local isOk, classInfo =
                ledClassInfoGet(command_data, devNum, ledIfCore, core, class, portType)

            if isOk then
                -- {"class", "invert", "blink", "force"}

                -- invert (not relavant for eArch)
                local invert = ""
                if isLegacy then
                    invert = classInfo.invertEnable and "Yes" or "No"
                end
                -- blink
                local blink = "No"
                if classInfo.blinkEnable then
                    blink = (classInfo.blinkSelect == "CPSS_LED_BLINK_SELECT_0_E")
                        and "blink0" or "blink1"
                end
                -- force
                local force = "No"
                if classInfo.forceEnable then
                    force = string.format("%x", classInfo.forceData) -- For GE devices: range 0x0..0xFFF (12 bits
                end
                -- pulse-stretch-enable (eArch only)
                -- disable-on-link-down (eArch only)
                local stretchEn         = ""
                local disableOnLinkDown = ""
                if not isLegacy then
                    stretchEn = classInfo.pulseStretchEnable and "Enabled" or "Disabled"
                    disableOnLinkDown = classInfo.disableOnLinkDown and "Yes" or "No"
                end

                -- insert data into the output table
                if not isLegacy then -- eArch
                    table.insert(outTable, {class, blink, force, stretchEn, disableOnLinkDown})
                elseif isLion then   -- Lion, Lion2
                    table.insert(outTable, {class, invert, blink, force})
                else                 -- legacy not Lion/Lion2 devices
                    local ptype = (portType == "CPSS_DXCH_LED_PORT_TYPE_XG_E")
                        and  "xg" or "tri-speed"
                    table.insert(outTable, {ptype, class, invert, blink, force})
                end
            end
        end
    end
    return outTable, widthTable
end


-- *****************************************************************************
--  getAndPrintLedGroupsInfo
--
--  @description read and print information about all LED groups of LED
--               interface
--
--  @param command_data  - Command_Data class of the outer CLI command handler.
--  @param devNum        - a device number
--  @param ledIfCore     - a LED interface number per core. portGroupBmp,
--                         ledIfCore are calculated from a LED
--                         inverface number per device
--  @param core          - a device's core number. In case of single-core device
--                         it should be nil!
--  @param ignorePortTypes - ignore portType (xg or tri-port)
--
--  @return  nothing
--
local function getAndPrintLedGroupsInfo(command_data, devNum, ledIfCore, core, ignorePortTypes)

    local portTypes
    local hdr = "\n\n" --"LED groups: (classA and classB) or (classC and classD):\n"

    if ignorePortTypes then
        portTypes = {"CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E"}
        hdr = hdr ..
            "group  classA  classB  classC  classD\n" ..
            "------ ------- ------- ------- -------"
    else
        portTypes = { "CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E",
                      "CPSS_DXCH_LED_PORT_TYPE_XG_E" }
        hdr = hdr ..
            "portType  group  classA  classB  classC  classD\n" ..
            "--------- ------ ------- ------- ------- -------"
    end
    for _, portType in pairs(portTypes) do
        local isOk, groupsInfo = ledGroupsInfoGet(
            command_data, devNum, ledIfCore, core, portType)
        if isOk then
            -- print a header once
            if hdr then
                print_nice(hdr)
                hdr = nil
            end

            -- index 0 is not iterated by ipairs because lua arrays starts from
            -- 1. Aand pairs() can't be used becase it doesn't order elements.
            -- so lets gather all keys (group numbers), order them by
            -- table.sort and iterate through them with ipairs()
            local groupsInfoKeys = {}
            for gr,_ in pairs(groupsInfo) do
                table.insert(groupsInfoKeys, gr)
            end
            table.sort(groupsInfoKeys)

            for _, group in ipairs(groupsInfoKeys) do
                local info = groupsInfo[group]
                local st = string.format("%-7d%-8d%-8d%-8d%-8d", group,
                                         info.classA, info.classB,
                                         info.classC, info.classD)
                if not ignorePortTypes then
                    ptype = (portType == "CPSS_DXCH_LED_PORT_TYPE_XG_E")
                        and "xg" or "tri-speed"
                    st = string.format("%-10s", ptype) ..st
                end
                print_nice(st)
            end
        end
    end
end

-- *****************************************************************************
--  showLedInterface
--
--  @description show LED interfaces attributes information.
--               Additionally information about LED classes can be displayed.
--
--  @param params - command's parameters
--         params.brief
--
--  @return  true on success, otherwise false and error message
--
local function showLedInterface(params)
    local command_data = Command_Data()
    local devices = (params.devNum == "all") and wrlDevList() or {params.devNum}
    local ledIfRange = params.ledIfRange

    for _, devNum in ipairs(devices) do
        local isLegacy = not is_sip_5(devNum)

        local ledIfCount, ledIfCountPerCore = ledIfCountPerDev(devNum)
        if ledIfCount == nil then
            command_data:addError(
                "An error occured during getting LED interfaces number on device %d",
                devNum)
            break -- go to the final command_data:analyzeCommandExecution()
        end

        for _, ledIf in ipairs(ledIfRange) do

            -- check correctness of LED interface number
            if ledIf >= ledIfCount then
                break
            end

            -- get LED interface related info (as struct)

            local ledIfCore, core = ledIfDev2Core(ledIf, ledIfCount, ledIfCountPerCore)

            local isOk, ledIfInfo = ledInterfaceInfoGet(command_data, devNum, ledIfCore, core)

            print("-----------------------------")
            print(string.format("device #%d, LED interface #%d", devNum, ledIf))
            print("-----------------------------")

            if isOk then
                -- transform struct to textual view and print it.
                local devLedIfOutput  = {}
                parseLedIfAttr(command_data, ledIfInfo, isLegacy, devLedIfOutput)
                printNameValueList(devLedIfOutput)
            else
                print("ERROR")
            end

            if params.detailed then
                -- get LED interface classes information
                local classOutputTable, colWidth =
                    gatherClassInfo(command_data, devNum, ledIfCore, core, isLegacy)
                -- calculate total width of all columns
                local totalWidth = 0
                for _,w in pairs(colWidth) do
                    totalWidth = totalWidth + w
                end

                -- print the classes info
                print_nice("\n")
                -- print_nice("LED Classes attributes:")
                -- print_nice("")
                for num, row in ipairs(classOutputTable) do
                    -- collect all columns into the string and print it
                    local str = ""
                    for i = 1,#row do
                        str = str .. string.format("%-"..colWidth[i].."s", row[i])
                    end
                    print_nice(str)
                    -- print "-------------" after the header line
                    if num == 1 then
                        print_nice(string.rep("-",totalWidth))
                    end
                end

                -- get and print LED interface groups information

                local family = wrLogWrapper("wrlCpssDeviceFamilyGet","(devNum)", devNum)
                local isLion =
                    family == "CPSS_PP_FAMILY_DXCH_LION_E" or
                    family == "CPSS_PP_FAMILY_DXCH_LION2_E"

                getAndPrintLedGroupsInfo(command_data, devNum, ledIfCore, core, isLion or not isLegacy)
            end
        end
    end
    command_data:updateStatus()
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


--*****************************************************************************
-- showLedPerPort
--
-- @description  show LED information relevant to port
--
-- @param command_data - the Command_Data() class instance
-- @param devNum       - a device number
-- @param portNum      - a port number
-- @param params       - the CLI command parameters (not used here)
--
--
-- @return nothing
--
local function showLedPerPort(command_data, devNum, portNum, params)

    local isOk, info = ledInfoPerPortGet(command_data, devNum, portNum)

    if not isOk then
        return
    end

    -- print info
    print_nice(string.format("=== device #%d, port #%d:", devNum, portNum))

    if info.posInStream then
        -- 0x3f means a port doesn't have a determined slot in a LED stream.
        if info.posInStream == 0x3f then
            info.posInStream = "N/A"
        end

        print_nice(string.format("%-46s %s",
                            "The port's position in the LED's chain:",
                            info.posInStream))
    end

    if info.isExtraSgmiiEn then
        print_nice(string.format("%-46s %s",
                            "enabling extra GMII indication in SGMII mode:",
                            info.isExtraSgmiiEn and "Yes" or "No"))
    end

    if info.classesInvert then
        print("\nclass #  Invert a polarity  Indication")
        print("-------  -----------------  ----------")
        for i=1, #info.classesInvert do
            print_nice(string.format("%-7s  %-17s  %-10s",
                                     info.classesInvert[i][1],
                                     info.classesInvert[i][2] and "Yes" or "No",
                                     info.classesIndication[i][2]))
        end
    end
end


local function showLedPerPortFunc(params)
    command_data = Command_Data()
    command_data:initAllInterfacesPortIterator(params)
    command_data:iterateOverPorts(showLedPerPort, params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--********************** CLI COMMANDS DECLARATION ******************************

--------------------------------------------------------------------------------
-- command registration: show led info
--------------------------------------------------------------------------------
CLI_addCommand({"exec"}, "show led info",
{
    func = showLedInfo,
    params = {{ type = "named",
                {format = "device %devID_all", name = "devNum"},
                mandatory = {"devNum"}}},
    help = "Show some global LED information unbound to any LED interface."
})

--------------------------------------------------------------------------------
-- command registration: show led interface
--------------------------------------------------------------------------------
CLI_addCommand({"exec"}, "show led interface",
{
    func = showLedInterface,
    params = {
        { type="values",
          {format = "%interfaceNum", name="ledIfRange"}
        },
        { type = "named",
          {format = "device %devID_all", name = "devNum"},
          {format = "brief", name = "brief", help = "a LED interfaces info. No classes details."},
          {format = "detailed", name = "detailed", help = "LED interfaces and LED classes info."},
          alt = {detailes = {"brief", "detailed"}},
          mandatory = {"devNum"}}},
    help = "show LED interface information"
})

--------------------------------------------------------------------------------
-- command registration: show interface led
--------------------------------------------------------------------------------
CLI_addCommand({"exec"}, "show interfaces led",
{
    func = showLedPerPortFunc,
    help = "show port related LED information",
    params = {
        { type = "named",
          {format ="ethernet %port-range"}
        },
        mandatory = {"portNum"}}
})
