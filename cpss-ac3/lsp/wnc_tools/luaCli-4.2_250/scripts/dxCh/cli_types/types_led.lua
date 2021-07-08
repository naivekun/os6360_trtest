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

--

local lion2BlinkDuration = {
    ["0"] = { value="CPSS_LED_BLINK_DURATION_0_E", help="LED blink period is 8.1 ms   (if core clock is 270Mhz)" },
    ["1"] = { value="CPSS_LED_BLINK_DURATION_1_E", help="LED blink period is 16.2 ms  (if core clock is 270Mhz)" },
    ["2"] = { value="CPSS_LED_BLINK_DURATION_2_E", help="LED blink period is 32.5 ms  (if core clock is 270Mhz)" },
    ["3"] = { value="CPSS_LED_BLINK_DURATION_3_E", help="LED blink period is 65.1 ms  (if core clock is 270Mhz)" },
    ["4"] = { value="CPSS_LED_BLINK_DURATION_4_E", help="LED blink period is 130.3 ms (if core clock is 270Mhz)" },
    ["5"] = { value="CPSS_LED_BLINK_DURATION_5_E", help="LED blink period is 260.7 ms (if core clock is 270Mhz)" },
    ["6"] = { value="CPSS_LED_BLINK_DURATION_6_E", help="LED blink period is 524.4 ms (if core clock is 270Mhz)" }
}

local lion2PulseStretch = {
    ["0"] = { value="CPSS_LED_PULSE_STRETCH_0_NO_E", help="No stretch will be done on any indicaiton" },
    ["1"] = { value="CPSS_LED_PULSE_STRETCH_1_E", help="LED clock stretching: 4-8.1 msec" },
    ["2"] = { value="CPSS_LED_PULSE_STRETCH_2_E", help="LED clock stretching: 32.5-65.1 msec" },
    ["3"] = { value="CPSS_LED_PULSE_STRETCH_3_E", help="LED clock stretching: 65.1-130.3 msec;" },
    ["4"] = { value="CPSS_LED_PULSE_STRETCH_4_E", help="LED clock stretching: 30.3-260.7 msec" },
    ["5"] = { value="CPSS_LED_PULSE_STRETCH_5_E", help="LED clock stretching: 260.7-521.4 msec" },
}

local function setSpecificDeviceDictionary(param,name,desc,varray,params)
    local d=CLI_type_dict["duration"]
    if params.earch == true then
        d.enum = bobcat2BlinkDuration
    else
        d.enum = lion2BlinkDuration
    end

    d=CLI_type_dict["stretch"]
    if params.earch == true then
        d.enum = bobcat2PulseStretch
    else
        d.enum = lion2PulseStretch
    end

end

CLI_type_dict["interfaceNum"] = {
    checker = CLI_check_param_number_range,
    min=0,
    max=7,
    complete = CLI_complete_param_number,
    help="The range of LED interfaces. Example: 0-2,4 means interfaces 1,1,2,4 or \"all\" for all interfaces"
}

CLI_type_dict["slotNumber"] = {
    checker = CLI_check_param_number,
    min=0,
    max=11,
    complete = CLI_complete_param_number,
    help="LED port slot number"
}

CLI_type_dict["duration"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="The divider of blink frequency",
    enum = {}
}

CLI_type_dict["stretch"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="LED clock stretching",
    enum = {}
}

CLI_type_dict["ledClass5"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="Class 5 indication for ports connected to dual media PHY (combo-ports)",
    enum = {
        ["half-duplex"] = { value="CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E", help="Half Duplex is displayed on class5" },
        ["fiber-link-up"] = { value="CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E", help="Fiber Link Up is displayed on class5" },
    }
}

CLI_type_dict["ledClass13"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="Class 13 indication for ports connected to dual media PHY (combo-ports)",
    enum = {
        ["link-down"] = { value="CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E", help="Link Down is displayed on class13" },
        ["cooper-link-up"] = { value="CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E", help="Copper Link Up is displayed on class13" },
    }
}

CLI_type_dict["LED_class_num"] =
{
    checker = CLI_check_param_number,
    min=0,
    max=13,
    help="Class number 0..13. Upper range can be less for some devices, please refer functional specification"
}
CLI_type_dict["LED_group_num"] =
{
    checker = CLI_check_param_number,
    min=0,
    max=3,
    help="Group number 0..3"
}
CLI_type_dict["LED_if_range"] =
{
    checker = led_range_checker,
    min=0,
    max=7,
    help="LED interface range"
}
CLI_type_dict["port_type"] =
{
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    enum = {
        ["tri-speed"] = { value="CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E", help="Tri-speed port"},
        ["xg"] = { value="CPSS_DXCH_LED_PORT_TYPE_XG_E", help="FlexLink port"},
        ["all"] = {value="all", help="All types"}
    },
    help="Port type {tri-speed | xg | all}"
}
CLI_type_dict["LED_class_range"] =
{
    checker = led_range_checker,
    min=0,
    max=14,
    help = "LED Class range 0..13. Upper range can be less for some devices, please refer functional specification. Examples: 0,1; 0-4; 2; all"
}
CLI_type_dict["LED_force_value"] =
{
    checker = CLI_check_param_number,
    min=0,
    max=4095,
    help="LED force value 0..4095"
}
CLI_type_dict["LED_indication_name"] =
{
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    enum = {
        ["default"] = { value="CPSS_DXCH_LED_INDICATION_PRIMARY_E", help="primary indication"},
        ["link"] = { value="CPSS_DXCH_LED_INDICATION_LINK_E", help="link"},
        ["rx_activity"] = {value="CPSS_DXCH_LED_INDICATION_RX_ACT_E", help="receive activity"},
        ["tx_activity"]= {value="CPSS_DXCH_LED_INDICATION_TX_ACT_E", help="transmit activity"},
        ["local_fault/pcs_Link"]= {value="CPSS_DXCH_LED_INDICATION_LOCAL_FAULT_OR_PCS_LINK_E", help="XG mode: local fault SGMII mode: PCS link"},
        ["remote_fault/reserved"]= {value="CPSS_DXCH_LED_INDICATION_REMOTE_FAULT_OR_RESERVED_E", help="XG mode: remote fault SGMII mode: reserved"},
        ["unknown_sequence/duplex"]= {value="CPSS_DXCH_LED_INDICATION_UKN_SEQ_OR_DUPLEX_E", help="XG mode: unknown sequence SGMII mode: duplex"},
        ["p_rej"]= {value="CPSS_DXCH_LED_INDICATION_P_REJ_E", help="p rej. In case Tri speed indications are enabled for flex port, activity indication will replace the p rej one"},
        ["rx_error"]= {value="CPSS_DXCH_LED_INDICATION_RX_ERROR_E", help="receive error"},
        ["jabber"]= {value="CPSS_DXCH_LED_INDICATION_JABBER_E", help="jabber. In case Tri speed indications are enabled for flex port, GMII speed indication will replace the jabber one"},
        ["fragment"]= {value="CPSS_DXCH_LED_INDICATION_FRAGMENT_E", help="fragment. In case Tri speed indications are enabled for flex port, MII speed indication will replace the fragment one"},
        ["crc_error"]= {value="CPSS_DXCH_LED_INDICATION_CRC_ERROR_E", help="CRC error"},
        ["fc_recived"]= {value="CPSS_DXCH_LED_INDICATION_FC_RX_E", help="flow control received"},
        ["tx_bad_crc/late collision"]= {value="CPSS_DXCH_LED_INDICATION_TX_BAD_CRC_OR_LATE_COL_E", help="XG mode: transmit bad CRC SGMII mode: late collision"},
        ["tx_buffer_full/back_pressure"]= {value="CPSS_DXCH_LED_INDICATION_RX_BUFFER_FULL_OR_BACK_PR_E", help="XG mode: receive buffer full SGMII mode: back pressure"},
        ["watchdog_exp_sample"]= {value="CPSS_DXCH_LED_INDICATION_WATCHDOG_EXP_SAMPLE_E", help="watchdog expired sample"},
    },
    help="LED indication name"
}
-----------------------------------
local function devID_earch_complete(param,name,desc)
    local d=CLI_type_dict["devID_all"]
    return d.complete(param,name,d)
end
local function devID_earch_checker(param,name,desc,varray,params)
    local d=CLI_type_dict["devID_all"]
    local status, val, devices

    status, val = d.checker(param,name,desc, varray, params)
    if not status then
        return status, val
    end

    -- check specified device of first one if "all" specified
    local devToCheck = (val == "all") and wrlDevList()[1] or val
    local dev_is_eArch = is_device_eArch_enbled(devToCheck)

    if dev_is_eArch == true then
        params.earch = true
    else
        params.legacy = true
    end
    --used for led interface group and led class
    local devices = (param=="all") and wrlDevList() or {param}
   -- check if device portType relevant
    for dev_iter, devNum in pairs(devices) do
        local f = wrLogWrapper("wrlCpssDeviceFamilyGet","(devNum)",devNum)
        if (LED_IF_INFO[f].portTypeRelevant==true) then
             params.isPortTypeRelevant = true
             break
         end
     end
    ----------------------------------------

    setSpecificDeviceDictionary(param,name,desc,varray,params)

    return status, val
end

CLI_type_dict["devID_earch"] = {
    checker = devID_earch_checker,
    complete = devID_earch_complete,
    help = "Enter device ID or 'all' for all devices"
}

local function init_led_dictionaries()
    if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_XCAT3_E"] then
        CLI_type_dict["slotNumber"].max = 27
    end
end

table.insert(typeCallBacksList, init_led_dictionaries)
