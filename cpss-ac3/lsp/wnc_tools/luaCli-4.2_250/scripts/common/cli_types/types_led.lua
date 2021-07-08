--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* types_led.lua
--*
--* DESCRIPTION:
--*       the file defines types needed for the led commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

bobcat2BlinkDuration = {
    ["0"] = { value="CPSS_LED_BLINK_DURATION_0_E", help="LED blink period is 32 ms" },
    ["1"] = { value="CPSS_LED_BLINK_DURATION_1_E", help="LED blink period is 64 ms" },
    ["2"] = { value="CPSS_LED_BLINK_DURATION_2_E", help="LED blink period is 128 ms" },
    ["3"] = { value="CPSS_LED_BLINK_DURATION_3_E", help="LED blink period is 256 ms" },
    ["4"] = { value="CPSS_LED_BLINK_DURATION_4_E", help="LED blink period is 512 ms" },
    ["5"] = { value="CPSS_LED_BLINK_DURATION_5_E", help="LED blink period is 1024 ms" },
    ["6"] = { value="CPSS_LED_BLINK_DURATION_6_E", help="LED blink period is 1536 ms" },
    ["7"] = { value="CPSS_LED_BLINK_DURATION_7_E", help="LED blink period is 2048 ms" }
}

bobcat2PulseStretch = {
    ["0"] = { value="CPSS_LED_PULSE_STRETCH_0_NO_E", help="No stretch will be done on any indicaiton" },
    ["1"] = { value="CPSS_LED_PULSE_STRETCH_1_E", help="LED clock divided by 10000" },
    ["2"] = { value="CPSS_LED_PULSE_STRETCH_2_E", help="LED clock divided by 500000" },
    ["3"] = { value="CPSS_LED_PULSE_STRETCH_3_E", help="LED clock divided by 1000000" },
    ["4"] = { value="CPSS_LED_PULSE_STRETCH_4_E", help="LED clock divided by 1500000" },
    ["5"] = { value="CPSS_LED_PULSE_STRETCH_5_E", help="LED clock divided by 2000000" },
    ["6"] = { value="CPSS_LED_PULSE_STRETCH_6_E", help="LED clock divided by 2500000" },
    ["7"] = { value="CPSS_LED_PULSE_STRETCH_7_E", help="LED clock divided by 3000000" }
}

CLI_type_dict["classRange"] = {
    checker = CLI_check_param_number_range,
    min=0,
    max=5,
    --complete = CLI_complete_param_number_range,
    help = "The range of LED classes. Example: 1-3,5 means classes 1,2,3,5 or \"all\" for all classes"
}

CLI_type_dict["streamBitPosition"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    complete = CLI_complete_param_number,
    help="LED stream bit position"
}

CLI_type_dict["blinkNumber"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1,
    complete = CLI_complete_param_number,
    help="LED blink configuration number"
}

CLI_type_dict["dutyCycle"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "LED blink duty cycle signal",
    enum = {
        ["25/75"] = { value="CPSS_LED_BLINK_DUTY_CYCLE_0_E", help="LED blink duty cycle is 25%" },
        ["50/50"] = { value="CPSS_LED_BLINK_DUTY_CYCLE_1_E", help="LED blink duty cycle is 50%" },
        ["75/25"] = { value="CPSS_LED_BLINK_DUTY_CYCLE_3_E", help="LED blink duty cycle is 75%" }
    }
}

CLI_type_dict["ledOrder"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "The order of the signals driven in the LED stream",
    enum = {
        ["port"] =  { value="CPSS_LED_ORDER_MODE_BY_PORT_E", help="The indication order is arranged by port" },
        ["class"] = { value="CPSS_LED_ORDER_MODE_BY_CLASS_E", help="The indication order is arranged by class"}
   }
}

CLI_type_dict["clockOutFrequency"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "LED clock out frequencies",
    enum = {
        ["0"] =  { value="CPSS_LED_CLOCK_OUT_FREQUENCY_500_E", help="500 KHz LED clock frequency" },
        ["1"] =  { value="CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E", help="1 MHz LED clock frequency" },
        ["2"] =  { value="CPSS_LED_CLOCK_OUT_FREQUENCY_2000_E", help="2 MHz LED clock frequency" },
        ["3"] =  { value="CPSS_LED_CLOCK_OUT_FREQUENCY_3000_E", help="3 MHz LED clock frequency" }
   }
}

CLI_type_dict["LED_blink_select"] =
{
    checker = CLI_check_param_number,
    min=0,
    max=1,
    help="LED blink select 0..1"
}

---------------------------------
function led_range_checker(param, name, desc)
    if param=="all" then
       return true,"all"
    else
       return CLI_check_param_number_range(param, name,desc)
    end
end

if dxCh_family == true then
    require_safe_dx("cli_types/types_led")
end

if px_family == true then
    require_safe_px("cli_types/types_led")
end
