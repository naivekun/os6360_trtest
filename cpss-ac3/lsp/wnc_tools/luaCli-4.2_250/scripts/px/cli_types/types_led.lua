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

CLI_type_dict["duration"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="The divider of blink frequency",
    enum = bobcat2BlinkDuration
}

CLI_type_dict["stretch"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="LED clock stretching",
    enum = bobcat2PulseStretch
}

CLI_type_dict["LED_class_num"] =
{
    checker = CLI_check_param_number,
    min=0,
    max=5,
    help="Class number 0..5"
}

CLI_type_dict["LED_class_range"] =
{
    checker = led_range_checker,
    min=0,
    max=5,
    help = "LED Class range 0..5. Examples: 0,1; 0-4; 2; all"
}

CLI_type_dict["LED_force_value"] =
{
    checker = CLI_check_param_number,
    min=0,
    max=4294967295,
    help="LED force value 0..4294967295"
}

CLI_type_dict["slotNumber"] = {
    checker = CLI_check_param_number,
    min=0,
    max=12,
    complete = CLI_complete_param_number,
    help="LED port slot number"
}
