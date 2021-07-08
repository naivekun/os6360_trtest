--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* interface_eee.lua
--*
--* DESCRIPTION:
--*       Port's IEEE 802.3az Energy Efficient Ethernet (EEE) Support.
--*       NOTEs:
--*       1. The device support relevant when the PHY works in EEE slave mode.
--*          When the PHY works in EEE master mode, the device is not involved in
--*          EEE processing.
--*       2. The feature not relevant when port is not 'tri-speed'
--*       3. LPI is short for 'Low Power Idle'
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--        printFromShow(555)
--[[ this file commands:
CLI_addCommand("interface", "eee mode", {
]]--
--includes

--constants

--##################################
--##################################

local function update_timers(command_data, devNum, portNum, params) 
    local tw_limit = params.tw_limit
    local li_limit = params.li_limit
    local ts_limit = params.ts_limit

    if tw_limit == nil and li_limit == nil and ts_limit == nil then
        return
    end

    -- we need to update the timers
    if tw_limit and li_limit and ts_limit then
        -- not need to read the current setting before modifying it.
        local apiName = "cpssDxChPortEeeLpiTimeLimitsSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",      "devNum",     devNum },
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
            { "IN",     "GT_U32",    "twLimit",   tw_limit },
            { "IN",     "GT_U32",    "liLimit",   li_limit },
            { "IN",     "GT_U32",    "tsLimit",   ts_limit }
        })
    else
        -- read the current setting before modifying it.
        local apiName = "cpssDxChPortEeeLpiTimeLimitsGet"
        local isError , result, values =  
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",      "devNum",     devNum },
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
            { "OUT",    "GT_U32",    "twLimitPtr" },
            { "OUT",    "GT_U32",    "liLimitPtr" },
            { "OUT",    "GT_U32",    "tsLimitPtr" }
        })
        
        if isError then 
            return true-- error
        end
        
        if tw_limit == nil then 
            tw_limit = values.twLimitPtr 
        end
        if li_limit == nil then 
            li_limit = values.liLimitPtr 
        end
        if ts_limit == nil then 
            ts_limit = values.tsLimitPtr 
        end

        -- modifying the counters.
        local apiName = "cpssDxChPortEeeLpiTimeLimitsSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",      "devNum",     devNum },
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
            { "IN",     "GT_U32",    "twLimit",   tw_limit },
            { "IN",     "GT_U32",    "liLimit",   li_limit },
            { "IN",     "GT_U32",    "tsLimit",   ts_limit }
        })
    end
end

-- implement per port : 'interface' --> eee mode command
local function per_port__interface_eee_mode(command_data, devNum, portNum, params) 
    local enable
    local manualMode
    
    mapInfo = getPortMapInfo(devNum,portNum) 
    if mapInfo and 
       mapInfo.mappingType ~= "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" then
        printLog("NOTE: port[" .. portNum .."] skipped because type is ".. to_string(mapInfo.mappingType))
        return
    end
    
    -- check if need to update the timers
    local isError = update_timers(command_data, devNum, portNum, params) 
    if isError then 
        return 
    end
    
    if params.mode == eee_disable then
        enable = false
    else
        enable = true
        if params.mode == eee_auto then
            manualMode = 0--"CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E"
        elseif params.mode == eee_manual then
            manualMode = 1--"CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENABLED_E"
        else --eee_force
            manualMode = 2--"CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_FORCED_E"
        end
    end
    
    if manualMode then
        local apiName = "cpssDxChPortEeeLpiManualModeSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",      "devNum",     devNum },
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
--            { "IN",     "CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT",    "mode",   manualMode }
            { "IN",     "GT_U32",    "mode",   manualMode }
        })
    end
    
    local apiName = "cpssDxChPortEeeLpiRequestEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "IN",     "GT_BOOL",    "enable",   enable }
    })
end
-- implement : 'interface' --> eee mode command
local function interface_eee_mode(params)
    --print(to_string(params))
    
    return generic_port_range_func(per_port__interface_eee_mode,params,eee_supported_interfaces)
end

local help_string_eee_mode = "state EEE mode on the Ethernet port, and optionally allow setting timers values"

CLI_addHelp("interface", "eee", "Support EEE - IEEE 802.3az Energy Efficient Ethernet (EEE)")
CLI_addHelp("interface", "eee mode", help_string_eee_mode)

--[[
Command to enable EEE on the Ethernet port, and optionally allow setting timers values.

NOTE: assumption that the PHY is in 'EEE SLAVE' mode.

Context – ethernet

Format: 
eee mode <disable/auto/manual/manual-force>  [timer-limit-tw <tw_limit>] [timer-limit-li <li_limit>] [timer-limit-ts <ts_limit>] 

Parameters:
mode – enable the EEE in auto mode or manual mode or manual mode with force transmitting of LPI signalling.
	Or disable the EEE.
tw_limit – (optional)The minimum time from LPI de-assertion until valid data can be sent.(units of 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.)
li_limit - (optional) The minimum time since going into LPI and allowing LPI de-assert.(units of 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.)
ts_limit - (optional) The minimum time from emptying of Tx FIFO and LPI assert.(units of 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.)
]]--
CLI_addCommand("interface", "eee mode", {
    func = interface_eee_mode,
    help = help_string_eee_mode,
	params={
        { type="values",
            { format = "%eee_mode_enum", name = "mode", help = eee_help_eee_mode},
        },
        { type="named",
            {format="timer-limit-tw %eee_tw_limit",name="tw_limit", help = eee_help_tw_limit },
            {format="timer-limit-li %eee_li_limit",name="li_limit", help = eee_help_li_limit },
            {format="timer-limit-ts %eee_ts_limit",name="ts_limit", help = eee_help_ts_limit },
        }
   }
})

--##################################
--##################################
-- implement per port : 'interface' --> phy mac-sec eee mode command
local function per_port__interface_phy_mac_sec_eee_mode(command_data, devNum, portNum, params) 
    local phyMode = params.mode -- value from MACPHY_EEE_LPI_MODE_ENT
    
    local apiName = "phyMacEEEModeGet"
    local isError , result, values =
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_U8","devNum", devNum},
        {"IN","GT_U8","port", portNum},
        --{ "IN",    "MACPHY_EEE_LPI_MODE_ENT",    "mode" ,phyMode }
		{"IN","GT_U32","mode",phyMode}
    })

    if (isError and result==0x10) then
        command_data:addError("ERROR : The feature is not supported on the device OR the MAC-PHY driver was not initialized.")
        command_data:setFailStatus()
        command_data:setFailLocalStatus() 
    end 
end

-- implement : 'interface' --> phy mac-sec eee mode command
local function interface_phy_mac_sec_eee_mode(params)
    --print(to_string(params))
    
    return generic_port_range_func(per_port__interface_phy_mac_sec_eee_mode,params,eee_supported_interfaces)
end

local help_string_phy_mac_sec_eee_mode = "Support EEE for the MAC-SEC."

CLI_addHelp("interface", "phy", "Support PHY features on this interface.")
CLI_addHelp("interface", "phy mac-sec", "Support MAC-SEC features of the PHY (requires that PHY-MAC-SEC driver was initialized).")
CLI_addHelp("interface", "phy mac-sec eee", help_string_phy_mac_sec_eee_mode)
--[[
Command to set the 'mac-sec' part of the PHY with EEE mode : disable  / master / slave / force.

NOTE: replace old command "EEE set" (config)

Context – ethernet

Format: 
phy mac-sec eee mode < disable  / master / slave / force >

Parameters:
mode - The phy-mac-sec EEE mode : disable  / master / slave / force

NOTE: if the PHY-MAC-SEC driver is not enabled than the function will return error.
]]--
CLI_addCommand("interface", "phy mac-sec eee mode", {
    func = interface_phy_mac_sec_eee_mode,
    help = help_string_phy_mac_sec_eee_mode,
	params={
        { type="values",
            { format = "%eee_phy_mode_enum", name = "mode", help = eee_help_eee_phy_mode},
        }
   }
})



