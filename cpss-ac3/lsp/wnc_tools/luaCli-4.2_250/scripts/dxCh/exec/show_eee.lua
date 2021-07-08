--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_eee.lua
--*
--* DESCRIPTION:
--*       show global and interface configurations and counters related to EEE.
--*       Port's IEEE 802.3az Energy Efficient Ethernet (EEE) Support.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--[[
-- for dev,portNum get config :
-- function returns isError , portInfo
--      portInfo --> {mode , tw_limit , li_limit  , ts_limit , phyMacSecMode }
function eee_get_info_port_config(command_data,devNum,portNum)

-- for dev,port get info :
-- function returns isError , portInfo
--      portInfo --> {macTxPathLpi,macTxPathLpWait,macRxPathLpi,pcsTxPathLpi,pcsRxPathLpi}
function eee_get_info_port_status(command_data,devNum,portNum)
]]--
--includes

local delemiter = "*-*-*-*-*-*-*-*-*-* \n"
 
--##################################
--##################################
-- for dev,portNum get config :
-- function returns isError , portInfo
--      portInfo --> {mode , tw_limit , li_limit  , ts_limit , phyMacSecMode }
function eee_get_info_port_config(command_data,devNum,portNum)
    local portInfo = {}

    --====================
    --====================
    command_data.dontStateErrorOnCpssFail = true
    local apiName = "phyMacEEEModeGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        --{ "OUT",    "MACPHY_EEE_LPI_MODE_ENT",    "modePtr"}
        { "OUT",    "GT_U32",    "modePtr"}
    })
    command_data.dontStateErrorOnCpssFail = nil
    if isError then
        isError = false
        portInfo.phyMacSecMode = eee_phy_NOT_VALID
    elseif values.modePtr == eee_phy_disable then
        portInfo.phyMacSecMode = "Disabled"
    elseif values.modePtr == eee_phy_master then
        portInfo.phyMacSecMode = "Master"
    elseif values.modePtr == eee_phy_slave then
        portInfo.phyMacSecMode = "Slave"
    elseif values.modePtr == eee_phy_force then
        portInfo.phyMacSecMode = "Forced"
    else 
        portInfo.phyMacSecMode = eee_phy_NOT_VALID
    end
    
    --====================
    --====================
    local apiName = "cpssDxChPortEeeLpiRequestEnableGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "OUT",    "GT_BOOL",    "enablePtr"}
    })

    if isError then
        return isError , portInfo
    end
    
    if values.enablePtr == false then
        portInfo.mode = eee_disable
    else
        --====================
        --====================
        local apiName = "cpssDxChPortEeeLpiManualModeGet"
        local isError , result, values = 
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",      "devNum",     devNum },
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
--            { "OUT",    "CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT",    "modePtr"}
            { "OUT",    "GT_U32",    "modePtr"}
        })

        if isError then
            return isError , portInfo
        end
        
        if values.modePtr == 0--"CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_DISABLED_E" 
        then
            portInfo.mode = eee_auto
        elseif values.modePtr == 1--"CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENABLED_E" 
        then
            portInfo.mode = eee_manual 
        else --2--"CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_FORCED_E"
            portInfo.mode = eee_force 
        end
    end
    
    --====================
    --====================
    local apiName = "cpssDxChPortEeeLpiTimeLimitsGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "OUT",    "GT_U32",    "twLimitPtr"},
        { "OUT",    "GT_U32",    "liLimitPtr"},
        { "OUT",    "GT_U32",    "tsLimitPtr"}
    })

    if isError then
        return isError , portInfo
    end

    portInfo.tw_limit = values.twLimitPtr
    portInfo.li_limit = values.liLimitPtr
    portInfo.ts_limit = values.tsLimitPtr
    
    return isError, portInfo
end

-- for dev,port get info :
-- function returns isError , portInfo
--      portInfo --> {macTxPathLpi,macTxPathLpWait,macRxPathLpi,pcsTxPathLpi,pcsRxPathLpi}
function eee_get_info_port_status(command_data,devNum,portNum)
    local portInfo = {}

    local apiName = "cpssDxChPortEeeLpiStatusGet"
    local isError , result, values = 
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "OUT",    "CPSS_DXCH_PORT_EEE_LPI_STATUS_STC",    "statusPtr"}
    })

    if isError then
        return isError , portInfo
    end

    portInfo = values.statusPtr
    
    return isError, portInfo
end    

CLI_addHelp("exec", "show eee", "show EEE - Port's IEEE 802.3az Energy Efficient Ethernet (EEE) related configurations and status")

--##################################
--##################################
local function per_port__show_eee_port_config(command_data, devNum, portNum, params)
    local mapInfo = getPortMapInfo(devNum,portNum) 
    if mapInfo and 
       mapInfo.mappingType ~= "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" then
        --skipped
        return
    end
    
    
    --portInfo --> {mode , tw_limit , li_limit  , ts_limit , phyMacSecMode }
    local isError , portInfo = eee_get_info_port_config(command_data, devNum, portNum)
    if isError then
        return
    end
 
    local tw_limit = string.format("0x%3.3x",portInfo.tw_limit)
    local li_limit = string.format("0x%3.3x",portInfo.li_limit)
    local ts_limit = string.format("0x%3.3x",portInfo.ts_limit)
        
    -- Resulting string formatting and adding.
    local devPortStr=tostring(devNum).."/"..tostring(portNum)
    command_data["result"] =
        string.format("%-11s%-10s%-10s%-9s%-9s%-10s", 
                devPortStr, portInfo.mode , 
                tw_limit , li_limit , ts_limit , 
                portInfo.phyMacSecMode)

    command_data:addResultToResultArray()
end


local config_params_help = 
[[
mode - the EEE is disabled , Or in auto mode or manual mode or manual mode with force transmitting of LPI signalling.
  --> next timers with units of 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.
tw_limit - The minimum time from LPI de-assertion until valid data can be sent.
li_limit - The minimum time since going into LPI and allowing LPI de-assert.
ts_limit - The minimum time from emptying of Tx FIFO and LPI assert.
phyMacSecMode - the phy mac-sec mode : NA/disabled/slave/master/force. (NA when phy-mac-sec driver was not initialized)
]]

local function show_eee_port_config(params)
    local header_string, footer_string

    header_string =
        "\n" ..
        config_params_help ..
        delemiter ..
        "Dev/Port   mode     tw_limit  li_limit ts_limit phyMacSecMode \n" ..
        "---------  --------------------------------------------------  \n"
    footer_string = "\n"
    
    params.header_string = header_string
    params.footer_string = footer_string

    return generic_all_ports_show_func(per_port__show_eee_port_config,params,eee_supported_interfaces)
end


--[[
Command to show the eee configurations of  EEE on the port(s).

Next information is shown (per port):
Configurations of:

mode -the EEE is disabled , 
Or in auto mode or manual mode or manual mode with force transmitting of LPI 
signaling.
tw_limit - The minimum time from LPI de-assertion until valid data can be sent.(units of 
1 uSec for 1000 Mbps and 10 uSec for 100 Mbps.)
li_limit - The minimum time since going into LPI and allowing LPI de-assert.(units of 1 
uSec for 1000 Mbps and 10 uSec for 100 Mbps.)
ts_limit - The minimum time from emptying of Tx FIFO and LPI assert.(units of 1 uSec 
for 1000 Mbps and 10 uSec for 100 Mbps.)
phyMacSecMode – the phy mac-sec mode : NA/disabled/slave/master/force. (NA when 
phy-mac-sec driver was not initialized)

Context – exec (no context)

Format: 
Show eee port-config <all/Ethernet/port-channel >

Parameters:
interface -
  all                  Apply to all ports of specific device or all available devices
  ethernet             Ethernet interface to configure
  port-channel         Valid Port-Channel interface
]]--
local help_string_show_eee_port_config = "Show the EEE configurations of the port(s)."
CLI_addHelp("exec", "show eee port-config", help_string_show_eee_port_config)
CLI_addCommand("exec", "show eee port-config", {
  func   = show_eee_port_config,
  help   = help_string_show_eee_port_config,
  params = {
      { type = "named",
          "#all_interfaces",
          mandatory = { "all_interfaces" }
      }
  }
})
--##################################
--##################################
local function per_port__show_eee_port_status(command_data, devNum, portNum, params)
    local mapInfo = getPortMapInfo(devNum,portNum) 
    if mapInfo and 
       mapInfo.mappingType ~= "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" then
        --skipped
        return
    end
    
    --portInfo --> {macTxPathLpi,macTxPathLpWait,macRxPathLpi,pcsTxPathLpi,pcsRxPathLpi}
    local isError , portInfo = eee_get_info_port_status(command_data, devNum, portNum)
    if isError then
        return
    end
 
    -- Resulting string formatting and adding.
    local devPortStr=tostring(devNum).."/"..tostring(portNum)
    command_data["result"] =
        string.format("%-11s%-13s%-16s%-13s%-13s%-13s", 
                devPortStr, 
                tostring(portInfo.macTxPathLpi),
                tostring(portInfo.macTxPathLpWait),
                tostring(portInfo.macRxPathLpi),
                tostring(portInfo.pcsTxPathLpi),
                tostring(portInfo.pcsRxPathLpi))

    command_data:addResultToResultArray()
end

local status_params_help = 
[[
macTxPathLpi    - Indicates that the MAC is sending LPI to the Tx PHY.
                false - not sending.
                true  - sending.
macTxPathLpWait - Indicates that the MAC is sending IDLE to the Tx PHY and waiting for Tx timer to end.
                false - not sending.
                true  - sending and wait.
macRxPathLpi    - Indicates that the PHY has detected the assertion / de-assertion of LP_IDLE from link partner (through the PCS).
                false - not detected.
                true  - detected.
pcsTxPathLpi    - PCS Tx path LPI status .
                false - Does not receive: Tx PCS does not receive LP_IDLE.
                true  - Receives: Tx PCS receives LP_IDLE.
pcsRxPathLpi    - PCS Rx path LPI status.
                false - Does not receive: Rx PCS does not receive LP_IDLE.
                true  - Receives: Rx PCS receives LP_IDLE.
]]
local function show_eee_port_status(params)
    local header_string, footer_string

    header_string =
        "\n" ..
        status_params_help ..
        delemiter ..
        "Dev/Port   macTxPathLpi macTxPathLpWait macRxPathLpi pcsTxPathLpi pcsRxPathLpi \n" ..
        "---------  ---------------------------------------------------------------------  \n"
    footer_string = "\n"
    
    params.header_string = header_string
    params.footer_string = footer_string

    return generic_all_ports_show_func(per_port__show_eee_port_status,params,eee_supported_interfaces)
end


--[[
Command to show the eee status of  EEE of the port(s).

Next information is shown (per port):

Context – exec (no context)

Format: 
Show eee port-status <all/Ethernet/port-channel >

Parameters:
  all                  Apply to all ports of specific device or all available devices
  ethernet             Ethernet interface to configure
  port-channel         Valid Port-Channel interface

Status of :
 *  macTxPathLpi    - Indicates that the MAC is sending LPI to the Tx PHY.
 *                  GT_FALSE - not sending.
 *                  GT_TRUE  - sending.
 *  macTxPathLpWait - Indicates that the MAC is sending IDLE to the Tx PHY and waiting for Tx timer to end.
 *                  GT_FALSE - not sending.
 *                  GT_TRUE  - sending and wait.
 *  macRxPathLpi    - Indicates that the PHY has detected the assertion / de-assertion of LP_IDLE from link partner (through the PCS).
 *                  GT_FALSE - not detected.
 *                  GT_TRUE  - detected.
 *  pcsTxPathLpi    - PCS Tx path LPI status .
 *                  GT_FALSE - Does not receive: Tx PCS does not receive LP_IDLE.
 *                  GT_TRUE  - Receives: Tx PCS receives LP_IDLE.
 *  pcsRxPathLpi    - PCS Rx path LPI status.
 *                  GT_FALSE - Does not receive: Rx PCS does not receive LP_IDLE.
 *                  GT_TRUE  - Receives: Rx PCS receives LP_IDLE.

]]--
local help_string_show_eee_port_status = "Show the EEE status of the port(s)."
CLI_addHelp("exec", "show eee port-status", help_string_show_eee_port_status)
CLI_addCommand("exec", "show eee port-status", {
  func   = show_eee_port_status,
  help   = help_string_show_eee_port_status,
  params = {
      { type = "named",
          "#all_interfaces",
          mandatory = { "all_interfaces" }
      }
  }
})


