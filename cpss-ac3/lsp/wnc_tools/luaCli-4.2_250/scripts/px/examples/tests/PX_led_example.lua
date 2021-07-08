--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_led_example.lua
--*
--* DESCRIPTION:
--*       example of configuring of LED interface for PIPE.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum = devEnv.dev

local function expectedCheckLed(expectedLedIfList, expectedLedPerPortList,
                                expectedLedClassList, prefix)
    local command_data = Command_Data()

    ----------------------- wrapper functions ----------------------------------
    local wrapperledInterfaceInfoGet = function(command_data, entry)
        local desc = prefix .. " "..string.format("device %d", devNum)

        local isOk, actualInfo = ledInterfaceInfoGet(command_data, devNum)

        -- For expectedInfo = {x=1}, actualInfo = {x=1, y=2} a comparison will
        -- be failed because of field y. Copy absent fields into expectedInfo
        -- from actualInfo to avoid the fail
        local expectedInfo = deepcopy(actualInfo) or {}
        mergeTablesRightJoin(expectedInfo, entry.info)

        return (not isOk), actualInfo, expectedInfo, desc
    end

    local wrapperLedInfoPerPortGet = function(command_data, entry)
        local portNum = entry.portNum
        local desc = prefix .. " " .. string.format("device %d, port %d", devNum, portNum)

        local isOk, actualInfo = ledInfoPerPortGet(command_data, devNum, portNum)
        local expectedInfo = deepcopy(actualInfo) or {}
        mergeTablesRightJoin(expectedInfo, entry.info)
        return (not isOk), actualInfo, expectedInfo, desc
    end

    local wrapperLedClassInfoGet = function(command_data, entry)
        local class    = entry.class
        local desc = prefix .. " "..
        string.format("dev %d, class %d", devNum, class)
        local isOk, actualInfo =
            ledClassInfoGet(command_data, devNum, class)

        -- For expectedInfo = {x=1}, actualInfo = {x=1, y=2} a comparison will
        -- be failed because of field y. Copy absent fields into expectedInfo
        -- from actualInfo to avoid the fail
        local expectedInfo = deepcopy(actualInfo) or {}
        mergeTablesRightJoin(expectedInfo, entry.info)

        return (not isOk), actualInfo, expectedInfo, desc
    end
    ----------------------------------------------------------------------------

    testCheckExpectedValues(command_data,
                            wrapperledInterfaceInfoGet, expectedLedIfList)

    testCheckExpectedValues(command_data,
                            wrapperLedInfoPerPortGet, expectedLedPerPortList)

    testCheckExpectedValues(command_data,
                            wrapperLedClassInfoGet, expectedLedClassList)

    command_data:updateStatus()
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


------------------------------ MAIN PART ---------------------------------------
local port1 = devEnv.port[1]
local port2 = devEnv.port[2]
local port3 = devEnv.port[3]
local port4 = devEnv.port[4]

local ledIfInfoAfterConfig = {
    {info = {
         ledOrganize      = "CPSS_LED_ORDER_MODE_BY_PORT_E",
         ledStart          = 0,
         ledEnd            = 100,
         blink0DutyCycle   = "CPSS_LED_BLINK_DUTY_CYCLE_1_E",
         blink0Duration    = "CPSS_LED_BLINK_DURATION_7_E",
         ledClockFrequency = "CPSS_LED_CLOCK_OUT_FREQUENCY_2000_E",
         pulseStretch      = "CPSS_LED_PULSE_STRETCH_7_E"
    }
}}

local ledIfInfoAfterDeconfig = {
    {info = {
         ledOrganize      = "CPSS_LED_ORDER_MODE_BY_CLASS_E",
         ledStart          = 0,
         ledEnd            = 255,
         blink0DutyCycle   = "CPSS_LED_BLINK_DUTY_CYCLE_0_E",
         blink0Duration    = "CPSS_LED_BLINK_DURATION_1_E",
         ledClockFrequency = "CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E",
         pulseStretch      = "CPSS_LED_PULSE_STRETCH_1_E"
    }
}}

local ledPerPortInfoAfterConfig = {
    {portNum = port1, info = {
         posInStream = 2,
         classesInvert = {
             [3] = {2, true},
             [4] = {3, true},
             [5] = {4, true},
             [6] = {5, true} }
    }},
    {portNum = port2, info = {posInStream = 3}},
    {portNum = port3, info = {posInStream = 4}},
    {portNum = port4, info = {posInStream = 5}},

}
local ledPerPortInfoAfterDeconfig = {
    {portNum = port1, info = {
         posInStream = 0x3f
         -- classesInvert = {} - can't be read because a posInStream is 0x3f :
         -- i.e.port's LED slot is out of LED stream applicable range.
         -- an API cpssPxLedStreamPortClassPolarityInvertEnableGet
         -- returns GT_NOT_INITIALIZED in such case.
    }},
    {portNum = port2, info = {posInStream = 0x3f}},
    {portNum = port3, info = {posInStream = 0x3f}},
    {portNum = port4, info = {posInStream = 0x3f}},

}

local ledClassInfoAfterConfig = {
    {class = 0, info = { disableOnLinkDown = true }},
    {class = 1, info = { disableOnLinkDown = true }},
    {class = 2, info = { disableOnLinkDown = true }},
    {class = 5, info = { disableOnLinkDown = true }},

    {class = 3,
     info = {
         invertEnable      = false,
         blinkEnable       = true,
         blinkSelect       = "CPSS_LED_BLINK_SELECT_0_E",
         forceEnable       = true,
         forceData         = 0x5,
         disableOnLinkDown = true
    }},
    {class = 4, info = {
         disableOnLinkDown  = true,
         pulseStretchEnable = false }}

}

local ledClassInfoAfterDeconfig = {
   {class = 0, info = { disableOnLinkDown = false }},
   {class = 1, info = { disableOnLinkDown = false }},
   {class = 2, info = { disableOnLinkDown = false }},
   {class = 5, info = { disableOnLinkDown = false }},

    {class = 3,
     info = {
         invertEnable      = false,
         blinkEnable       = false,
         forceEnable       = false
    }},
    {class = 4, info = {
         disableOnLinkDown  = false,
         pulseStretchEnable = true }}

}

local isOk = luaTgfSimpleTest(
    "px/examples/configurations/PX_led_example.txt",
    "px/examples/configurations/PX_led_example_deconfig.txt",
    expectedCheckLed,
    ledIfInfoAfterConfig, ledPerPortInfoAfterConfig, ledClassInfoAfterConfig,
    "led_config:"
)
expectedCheckLed(
    ledIfInfoAfterDeconfig, ledPerPortInfoAfterDeconfig, ledClassInfoAfterDeconfig,
    "led_deconfig:")
testPrintResultSummary("LED")
