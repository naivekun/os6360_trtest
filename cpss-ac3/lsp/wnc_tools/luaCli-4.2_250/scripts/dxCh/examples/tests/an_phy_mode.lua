--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* an_phy_mode.lua
--*
--* DESCRIPTION:
--*       The test for testing port auto-neg feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--* 
--********************************************************************************

local devNum = devEnv.dev

SUPPORTED_FEATURE_DECLARE(devNum, "SIP5_PORT_AUTONEG")

local defaultErrorSpeed = 10000
local defaultErrorMode = "KR"
local function getPortSpeed(devNum,portNum)
    local port_speed_string,port_speed_value
    local result, values = cpssPerPortParamGet("cpssDxChPortSpeedGet",
                                         devNum, portNum, "speed",
                                         "CPSS_PORT_SPEED_ENT")
    if 0 == result then
        port_speed = values["speed"]
        port_speed_string,port_speed_value = speedStrGet(port_speed)
    else
        port_speed_value = defaultErrorSpeed
    end
    
    port_speed_string = to_string(port_speed_value)
    
    return port_speed_string
end
local function getPortMode(devNum,portNum)
    local port_interface_mode
    local result, values =
        cpssPerPortParamGet("cpssDxChPortInterfaceModeGet",
                                         devNum, portNum, "mode",
                                         "CPSS_PORT_INTERFACE_MODE_ENT")
    if 0 == result then
        port_interface_mode = interfaceStrGet(values["mode"])
    else
        port_interface_mode = defaultErrorMode
    end
    
    return port_interface_mode
end

local orig_autoNegOrigInitSave = {}

local function getPortNumFromIndex(portIndex)
    return multiDevEnv[1].port[portIndex]
end
--
-- NOTE: function is called from config file , like : {@@autoNegOrigInitSave(4)}
-- save value of speed and mode , like : "speed 10000 mode KR"
function autoNegOrigInitSave(portIndex)
    local portNum = getPortNumFromIndex(portIndex)
    local devPort = tostring(devNum) .. "/" tostring(portNum)
    
    if(orig_autoNegOrigInitSave[devPort]==nil) then
        orig_autoNegOrigInitSave[devPort] = {}
    end
    
    local speed = getPortSpeed(devNum,portNum)
    local mode = getPortMode(devNum,portNum)
    
    orig_autoNegOrigInitSave[devPort] = {speed = speed , mode = mode}
    
    -- return "" meaning that the call to this function leave no trace of the CLI commands.
    return ""
end 

--
-- NOTE: function is called from de-config file , like : {@@autoNegSpeedOrigGet(4)}
-- return value of speed "10000"
function autoNegSpeedOrigGet(portIndex)
    local portNum = getPortNumFromIndex(portIndex)
    local devPort = tostring(devNum) .. "/" tostring(portNum)
    
    if(orig_autoNegOrigInitSave[devPort]==nil) then
        return tostring(defaultErrorSpeed)
    end
    
    return orig_autoNegOrigInitSave[devPort].speed
end 
-- NOTE: function is called from de-config file , like : {@@autoNegModeOrigGet(4)}
-- return value of mode "KR"
function autoNegModeOrigGet(portIndex)
    local portNum = getPortNumFromIndex(portIndex)
    local devPort = tostring(devNum) .. "/" tostring(portNum)
    
    if(orig_autoNegOrigInitSave[devPort]==nil) then
        return defaultErrorMode
    end
    
    return orig_autoNegOrigInitSave[devPort].mode
end 



--load configuration
printLog("Load configuration ..")
executeLocalConfig("dxCh/examples/configurations/an_phy_mode.txt")

--restore configuration
printLog("Restore configuration ..")
executeLocalConfig("dxCh/examples/configurations/an_phy_mode_deconfig.txt")


