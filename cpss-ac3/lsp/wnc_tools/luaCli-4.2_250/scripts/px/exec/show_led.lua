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
-- constants

blinkDutyCycle = {
    ["CPSS_LED_BLINK_DUTY_CYCLE_0_E"] = {value = 0, descr = "25% on, 75% off"},
    ["CPSS_LED_BLINK_DUTY_CYCLE_1_E"] = {value = 1, descr = "50% on, 50% off"},
    ["CPSS_LED_BLINK_DUTY_CYCLE_2_E"] = {value = 2, descr = "50% on, 50% off"},
    ["CPSS_LED_BLINK_DUTY_CYCLE_3_E"] = {value = 2, descr = "75% on, 25% off"},
}
blinkDuration = {
    ["CPSS_LED_BLINK_DURATION_0_E"] = {value = 0, descr = "0 - LED blink period is 32 ms"},
    ["CPSS_LED_BLINK_DURATION_1_E"] = {value = 1, descr = "1 - LED blink period is 64 ms"},
    ["CPSS_LED_BLINK_DURATION_2_E"] = {value = 2, descr = "2 - LED blink period is 128 ms"},
    ["CPSS_LED_BLINK_DURATION_3_E"] = {value = 3, descr = "3 - LED blink period is 256 ms"},
    ["CPSS_LED_BLINK_DURATION_4_E"] = {value = 4, descr = "4 - LED blink period is 512 ms"},
    ["CPSS_LED_BLINK_DURATION_5_E"] = {value = 5, descr = "5 - LED blink period is 1024 ms"},
    ["CPSS_LED_BLINK_DURATION_6_E"] = {value = 6, descr = "6 - LED blink period is 1536 ms"},
    ["CPSS_LED_BLINK_DURATION_7_E"] = {value = 7, descr = "7 - LED blink period is 2048 ms"},
}
stretchLen = {
    ["CPSS_LED_PULSE_STRETCH_0_NO_E"] = {value = 0, descr = "0 - No stretch will be done on any indicaiton"},
    ["CPSS_LED_PULSE_STRETCH_1_E"]    = {value = 1, descr = "1 - LEDCLK divided by 10000"},
    ["CPSS_LED_PULSE_STRETCH_2_E"]    = {value = 2, descr = "2 - LEDCLK divided by 500000"},
    ["CPSS_LED_PULSE_STRETCH_3_E"]    = {value = 3, descr = "3 - LEDCLK divided by 1000000"},
    ["CPSS_LED_PULSE_STRETCH_4_E"]    = {value = 4, descr = "4 - LEDCLK divided by 1500000"},
    ["CPSS_LED_PULSE_STRETCH_5_E"]    = {value = 5, descr = "5 - LEDCLK divided by 2000000"},
    ["CPSS_LED_PULSE_STRETCH_6_E"]    = {value = 6, descr = "6 - LEDCLK divided by 2500000"},
    ["CPSS_LED_PULSE_STRETCH_7_E"]    = {value = 7, descr = "7 - LEDCLK divided by 3000000"}
}
clockFreq = {
    ["CPSS_LED_CLOCK_OUT_FREQUENCY_500_E"]  = {descr = "0 - 500 KHz"},
    ["CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E"] = {descr = "1 - 1 MHz"},
    ["CPSS_LED_CLOCK_OUT_FREQUENCY_2000_E"] = {descr = "2 - 2 MHz"},
    ["CPSS_LED_CLOCK_OUT_FREQUENCY_3000_E"] = {descr = "3 - 3 MHz"},
    }

--*****************************************************************************
-- ledInterfaceInfoGet
--
-- @description  get a LED interface settings
--
-- @param command_data - a Command_Data object instance
-- @param devNum       - a device number
-- @return false       - on fail
--         true, table - if ok. table is a CPSS_PX_LED_CONF_STC.
--
function ledInterfaceInfoGet(command_data, devNum)

    local isFailed, rc, values = genericCpssApiWithErrorHandler(
        command_data, "cpssPxLedStreamConfigGet",
        {
            { "IN",     "GT_SW_DEV_NUM",           "devNum",             devNum},
            { "OUT",    "CPSS_PX_LED_CONF_STC",    "ledConfPtr"}
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
-- @param class        - a LED class number
--
-- @return false       - on fail
--         true, table - if ok. table is a CPSS_LED_CLASS_MANIPULATION_STC.

--
function ledClassInfoGet(command_data, devNum, class)
    local isFailed, rc, values = genericCpssApiWithErrorHandler(
        command_data, "cpssPxLedStreamClassManipulationGet",
        {
            {"IN",  "GT_SW_DEV_NUM",                      "devNum",          devNum},
            {"IN",  "GT_U32",                             "classNum",        class},
            {"OUT", "CPSS_PX_LED_CLASS_MANIPULATION_STC", "classParamsPtr"}
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
--
-- @return false       - on fail
--         true, table - if ok. table looks like
--                       { [group0] = group0Info,
--                         [group1] = group1Info,
--                         [group2] = group2Info,
--                         [group3] = group3Info }
--                       Group info - that is a CPSS_LED_GROUP_CONF_STC.
--
local function ledGroupsInfoGet(command_data, devNum)
    local info ={}
    local isOk = true
    for group=0,1 do
        rc, values = myGenWrapper(
            "cpssPxLedStreamGroupConfigGet", {
                {"IN", "GT_SW_DEV_NUM",               "devNum",          devNum},
                {"IN", "GT_U32",                      "groupNum",        group},
                {"OUT", "CPSS_LED_GROUP_CONF_STC",    "groupParamsPtr"}
        })
        if rc == 0 then
            info[group] = values.groupParamsPtr
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
    local classesInvertTab

    local apiName = "cpssPxLedStreamPortPositionGet"
    local isError, rc, values = genericCpssApiWithErrorHandler(
        command_data, apiName, {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum},
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
            { "OUT", "GT_U32", "positionPtr"},
    })
    posInStream = isError and nil or values.positionPtr

    classCount = 6          -- eArch devices have 6 LED classes
    classesInvertTab = {}
    local apiName = "cpssPxLedStreamPortClassPolarityInvertEnableGet"
    for class=0,classCount-1 do
        local rc, values = myGenWrapper(
            apiName, {
                { "IN",  "GT_SW_DEV_NUM",        "devNum", devNum},
                { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                { "IN",  "GT_U32",               "classNum", class},
                { "OUT", "GT_BOOL",              "invertEnablePtr"}
        })
        if rc == 0 then
            table.insert(classesInvertTab, {class, values.invertEnablePtr})
        else
            command_data:handleCpssErrorDevPort(
                rc, "getting a LED class" .. class .." invert polarity",
                devNum, portNum)
        end

    end
    return true, { posInStream    = posInStream,
                   classesInvert  = (classesInvertTab=={}) and nil or classesInvertTab }
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
            print(string.format(frmt, row[1], row[2]))
        end
    end
end


-- *****************************************************************************
--  parseLedIfAttr
--
--  @description transform a struct CPSS_PX_LED_CONF_STC to a text view in format
--               "item name - item value" and move it into the table.
--
--  @param command_data  - Command_Data class of the outer CLI command handler.
--                         Used for error handling.
--  @param ledConf       - a CPSS_PX_LED_CONF_STC struct to be parsed
--  @param tabOutput           - a table to gather text view results.
--  @return  nothing
--
local function parseLedIfAttr(command_data, ledConf, tabOutput)
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
    local val = blinkDutyCycle[ledConf.blink0DutyCycle].descr
    if not val then
        command_data:addError("Device #%d, LED interface %d. Unknown Blink0 Duty Cycle value: %s",
                              ledConf.blink0DutyCycle)
        val = "ERROR"
    end
    table.insert(tabOutput, {"Blink0 duty cycle:", val})

    -- blink0 duration
    val = blinkDuration[ledConf.blink0Duration].descr
    if not val then
        command_data:addError("Device #%d, LED interface %d. Unknown Blink0 Duty Duration value: %s",
                              ledConf.blink0Duration)
        val = "ERROR"
    end
    table.insert(tabOutput, {"Blink0 duration:", val})

    -- blink1 duty cycle
    local val = blinkDutyCycle[ledConf.blink1DutyCycle].descr
    if not val then
        command_data:addError("Device #%d, LED interface %d. Unknown Blink0 Duty Cycle value: %s",
                              ledConf.blink1DutyCycle)
        val = "ERROR"
    end
    table.insert(tabOutput, {"Blink1 duty cycle:", val})

    -- blink1 duration
    val = blinkDuration[ledConf.blink1Duration].descr
    if not val then
        command_data:addError("Device #%d, LED interface %d. Unknown Blink0 Duty Duration value: %s",
                              ledConf.blink1Duration)
        val = "ERROR"
    end
    table.insert(tabOutput, {"Blink1 duration:", val})

    -- stretch-cfg
    local attrName = isLegacy and "Stretch length" or "Stretching divider"
    val = stretchLen[ledConf.pulseStretch].descr
    if not val then
        command_data:addError("Device #%d, LED interface %d. Unknown Pulse Stretch value: %s",
                              ledConf.ledConf.pulseStretch)
        val = "ERROR"
    end
    table.insert(tabOutput, {attrName .. ":", val})

    -- clock-freq
    if not isLegacy then
        val = clockFreq[ledConf.ledClockFrequency].descr
        if not val then
            command_data:addError("Device #%d, LED interface %d. Unknown Clock Frequency value: %s",
                                  ledConf.ledConf.pulseStretch)
            val = "ERROR"
        end
        table.insert(tabOutput, {"A LEDCLK frequency:", val})
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
--
--  @return  two tables: 1) an output table and
--                       2) table with output table columns' width
--
local function gatherClassInfo(command_data, devNum)
    local outTable = {}
    local widthTable = {}

    -- Pipe devices have 6 classes. Legacy devices have 14 classes.
    local classCount = 6

    -- insert a table header.
    table.insert(outTable, {"class", "blink", "force", "stretch", "DisableOnLinkDown"})
    widthTable = {6, 12, 10, 9, 17}
   
    for class=0, classCount-1 do
        local isOk, classInfo =
            ledClassInfoGet(command_data, devNum, class)

        if isOk then
            -- {"class", "invert", "blink", "force"}
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
            -- pulse-stretch-enable
            -- disable-on-link-down
            local stretchEn         = ""
            local disableOnLinkDown = ""
            stretchEn = classInfo.pulseStretchEnable and "Enabled" or "Disabled"
            disableOnLinkDown = classInfo.disableOnLinkDown and "Yes" or "No"

            -- insert data into the output table
            table.insert(outTable, {class, blink, force, stretchEn, disableOnLinkDown})
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
--
--  @return  nothing
--
local function getAndPrintLedGroupsInfo(command_data, devNum)

    local hdr = "\n\n" --"LED groups: (classA and classB) or (classC and classD):\n"

    hdr = hdr ..
        "group  classA  classB  classC  classD\n" ..
        "------ ------- ------- ------- -------"
    local isOk, groupsInfo = ledGroupsInfoGet(command_data, devNum)
    if isOk then
        -- print a header once
        if hdr then
            print(hdr)
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
            print(st)
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
        local isOk, ledIfInfo = ledInterfaceInfoGet(command_data, devNum)

        print("-----------------------------")
        print(string.format("device #%d", devNum))
        print("-----------------------------")

        if isOk then
            -- transform struct to textual view and print it.
            local devLedIfOutput  = {}
            parseLedIfAttr(command_data, ledIfInfo, devLedIfOutput)
            printNameValueList(devLedIfOutput)
        else
            print("ERROR")
        end

        if params.detailed then
            -- get LED interface classes information
            local classOutputTable, colWidth =
                gatherClassInfo(command_data, devNum)
            -- calculate total width of all columns
            local totalWidth = 0
            for _,w in pairs(colWidth) do
                totalWidth = totalWidth + w
            end

            -- print the classes info
            print("\n")
            -- print("LED Classes attributes:")
            -- print("")
            for num, row in ipairs(classOutputTable) do
                -- collect all columns into the string and print it
                local str = ""
                for i = 1,#row do
                    str = str .. string.format("%-"..colWidth[i].."s", row[i])
                end
                print(str)
                -- print "-------------" after the header line
                if num == 1 then
                    print(string.rep("-",totalWidth))
                end
            end

            -- get and print LED interface groups information
            getAndPrintLedGroupsInfo(command_data, devNum)
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
    print(string.format("=== device #%d, port #%d:", devNum, portNum))

    if info.posInStream then
        -- 0x3f means a port doesn't have a determined slot in a LED stream.
        if info.posInStream == 0x3f then
            info.posInStream = "N/A"
        end

        print(string.format("%-46s %s",
                            "The port's position in the LED's chain:",
                            info.posInStream))
    end

    if info.classesInvert then
        print("\nclass #  Invert a polarity")
        print("-------  ------------------")
        for i=1, #info.classesInvert do
            print(string.format("%-7s  %s",
                                     info.classesInvert[i][1],
                                     info.classesInvert[i][2] and "Yes" or "No"))
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
-- command registration: show led interface
--------------------------------------------------------------------------------
CLI_addCommand({"exec"}, "show led",
{
    func = showLedInterface,
    params = {
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
