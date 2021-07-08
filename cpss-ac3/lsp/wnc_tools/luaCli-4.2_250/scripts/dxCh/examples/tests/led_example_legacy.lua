--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* led_example_legacy.lua
--*
--* DESCRIPTION:
--*       example of configuring of LED interface for non-eArch devices.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
SUPPORTED_DEV_DECLARE(devEnv.dev,
                      "CPSS_PP_FAMILY_CHEETAH_E",
                      "CPSS_PP_FAMILY_CHEETAH2_E",
                      "CPSS_PP_FAMILY_CHEETAH3_E",
                      "CPSS_PP_FAMILY_DXCH_XCAT_E",
                      "CPSS_PP_FAMILY_DXCH_XCAT2_E",
                      "CPSS_PP_FAMILY_DXCH_XCAT3_E")

local function expectedCheckLedIf(expectedLedIfList, expectedLedClassList, prefix)

    local command_data = Command_Data()
    local devNum       = devNum

    ----------------------- wrapper functions ----------------------------------
    local wrapperLedInterfaceInfoGet = function(command_data, entry)
        local ledIf         = entry.ledIf
        local desc = prefix .. " "..string.format("dev %d, LED if %d", devNum, ledIf)

        local ledIfCore, core = ledIfDev2Core(ledIf, ledIfCount, ledIfCountPerCore)
        local isOk, actualInfo = ledInterfaceInfoGet(command_data, devNum, ledIfCore, core)

        -- For expectedInfo = {x=1}, actualInfo = {x=1, y=2} a comparison will
        -- be failed because of field y. Copy absent fields into expectedInfo
        -- from actualInfo to avoid the fail
        local expectedInfo = deepcopy(actualInfo) or {}
        mergeTablesRightJoin(expectedInfo, entry.info)

        return (not isOk), actualInfo, expectedInfo, desc
    end

    local wrapperLedClassInfoGet = function(command_data, entry)
        local ledIf    = entry.ledIf
        local class    = entry.class
        local portType = entry.portType or "CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E"
        local desc = prefix .. " "..
            string.format("dev %d, LED if %d, class %d, port-type %s",
                          devNum, ledIf, class,
                          (portType == "CPSS_DXCH_LED_PORT_TYPE_XG_E")
                              and "xg" or "tri-speed")

        local ledIfCore, core = ledIfDev2Core(ledIf, ledIfCount, ledIfCountPerCore)
        local isOk, actualInfo =
            ledClassInfoGet(command_data, devNum, ledIfCore, core, class, portType)

        -- For expectedInfo = {x=1}, actualInfo = {x=1, y=2} a comparison will
        -- be failed because of field y. Copy absent fields into expectedInfo
        -- from actualInfo to avoid the fail
        local expectedInfo = deepcopy(actualInfo) or {}
        mergeTablesRightJoin(expectedInfo, entry.info)

        return (not isOk), actualInfo, expectedInfo, desc
    end
    ----------------------------------------------------------------------------

    testCheckExpectedValues(command_data,
                            wrapperLedInterfaceInfoGet, expectedLedIfList)
    testCheckExpectedValues(command_data,
                            wrapperLedClassInfoGet, expectedLedClassList)

    command_data:updateStatus()
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end


------------------------------ MAIN PART ---------------------------------------
local ledIfInfoAfterConfig = {
    {ledIf = 0, info = {
         ledOrganize      = "CPSS_LED_ORDER_MODE_BY_PORT_E",
         ledStart          = 100,
         ledEnd            = 255,
         blink0DutyCycle   = "CPSS_LED_BLINK_DUTY_CYCLE_3_E",
         blink0Duration    = "CPSS_LED_BLINK_DURATION_2_E",
         pulseStretch      = "CPSS_LED_PULSE_STRETCH_2_E",
         clkInvert         = true,
         disableOnLinkDown = true,
         class5select      = "CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E",
         class13select     = "CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E"}
}}

local ledIfInfoAfterDeconfig = {
    {ledIf = 0, info = {
         ledOrganize      = "CPSS_LED_ORDER_MODE_BY_CLASS_E",
         ledStart          = 0,
         ledEnd            = 255,
         blink0DutyCycle   = "CPSS_LED_BLINK_DUTY_CYCLE_0_E",
         blink0Duration    = "CPSS_LED_BLINK_DURATION_1_E",
         pulseStretch      = "CPSS_LED_PULSE_STRETCH_1_E",
         clkInvert         = false,
         disableOnLinkDown = false,
         class5select      = "CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E",
         class13select     = "CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E"}
}}

local ledClassInfoAfterConfig = {
    {ledIf = 0, class = 3, portType = "CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E",
     info = {
         invertEnable      = false,
         blinkEnable       = true,
         blinkSelect       = "CPSS_LED_BLINK_SELECT_0_E",
         forceEnable       = true,
         forceData         = 0x5
    }},
    {ledIf = 0, class = 3, portType = "CPSS_DXCH_LED_PORT_TYPE_XG_E",
     info = {
         invertEnable      = true,
         blinkEnable       = true,
         blinkSelect       = "CPSS_LED_BLINK_SELECT_0_E",
         forceEnable       = false,
    }}
}

local ledClassInfoAfterDeconfig = {
    {ledIf = 0, class = 3, portType = "CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E",
     info = {
         invertEnable      = false,
         blinkEnable       = false,
         forceEnable       = false
    }},
    {ledIf = 0, class = 3, portType = "CPSS_DXCH_LED_PORT_TYPE_XG_E",
     info = {
         invertEnable      = false,
         blinkEnable       = false,
         forceEnable       = false
    }}
}


local devNum = devEnv.dev
local ledIfCount, ledIfCountPerCore = ledIfCountPerDev(devNum)
if ledIfCount == nil then
    testAddErrorString (
        "An error occured during getting LED interfaces count per device %d",
        devNum)
    return
end

local isOk = luaTgfSimpleTest(
    "dxCh/examples/configurations/led_example_legacy.txt",
    "dxCh/examples/configurations/led_example_legacy_deconfig.txt",
    expectedCheckLedIf, ledIfInfoAfterConfig, ledClassInfoAfterConfig,
    "led_legacy_config:"
)
expectedCheckLedIf(ledIfInfoAfterDeconfig, ledClassInfoAfterDeconfig,
                   "led_legacy_deconfig:")
testPrintResultSummary("LED")
