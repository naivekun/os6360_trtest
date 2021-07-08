--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* speed.lua
--*
--* DESCRIPTION:
--*       setting of port speed and interface mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 11 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlDxChPortModeSpeedSet")
cmdLuaCLI_registerCfunction("wrlDxChPortsModeSpeedSet")
cmdLuaCLI_registerCfunction("wrlCpssDxChSerdesDefaultPowerUp")
cmdLuaCLI_registerCfunction("wrlPrvCpssDxChPortIsRemote")

--constants


-- ************************************************************************
---
--  speed_func
--        @description  sets port speed and interface mode
--
--        @param params         - params["port_speed"]: port speed;
--                                params["interface_mode"]: interface mode
--
--        @usage __global       - __global["ifRange"]: interface range
--
--        @return       true on success, otherwise false and error message
--
local function speed_func(params)
    -- Common variables declaration.
    local result, values
    local devFamily, devNum, ports, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local port_speed_changing, port_speed
    local port_interface_mode_changing, port_interface_mode_determinancy
    local port_interface_mode, port_is_remote
    local fc_intervals, interval, state

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    -- power down interface
    if params.flagNo then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()
            result, values =  wrLogWrapper("wrlDxChPortModeSpeedSet", "(devNum, portNum, false, 0, 0)", devNum, portNum, false, 0, 0)
            if  0x10 == result then
                command_data:setFailPortStatus()
                command_data:addWarning("It's not allowed to power down for device %d port %d", devNum, portNum)
            elseif 0 ~= result then
                command_data:setFailPortStatus()
                command_data:addWarning("Error of power downing for device %d port %d: %s", devNum, portNum, values)
            end
        end
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
    end

    -- Command specific variables initialization.
    port_speed_changing                 =
        command_data:getTrueIfFlagNoIsNil(params)
        port_speed = params["port_speed"]
    port_interface_mode_changing        =
        getTrueIfNotNil(params["port_interface_mode"])
    port_interface_mode_determinancy    =
        getTrueIfNotNil(params["port_interface_mode"])
    port_interface_mode = params["port_interface_mode"]

  --power up interface
  for iterator, devNum, portNum in command_data:getPortIterator() do
      command_data:clearPortStatus()
      command_data:clearLocalStatus()
      port_is_remote = wrlPrvCpssDxChPortIsRemote(devNum, portNum)
      result, values = wrLogWrapper("wrlDxChPortModeSpeedSet",
                                    "(devNum, portNum, true, port_interface_mode, port_speed)",
                                    devNum, portNum, true, port_interface_mode, port_speed)
      if  0x10 == result then
        command_data:setFailPortStatus()
        command_data:addWarning("It is not allowed to set " ..
                    "speed %s and interface " ..
                    "mode %s for device %d " ..
                    "port(s) %s",
                    speedStrGet(port_speed),
                    interfaceStrGet(
                      port_interface_mode),
                    devNum, tbltostr(ports))
      elseif 0 ~= result then
        command_data:setFailPortStatus()
        command_data:addWarning("Error at port speed %s or " ..
                    "mode %s setting of device " ..
                    "%d port(s) %s: %s",
                    speedStrGet(port_speed),
                    interfaceStrGet(
                      port_interface_mode),
                    devNum, tbltostr(ports), values)
      elseif ((0 == result) and (not port_is_remote)) then
        result, values = myGenWrapper("cpssDxChPortPeriodicFcEnableGet",
                      {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                       { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum", portNum},            -- portNum
                       { "OUT","CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT" , "statePtr"}})             -- data
        if 0 ~= result then
          command_data:setFailPortStatus()
          command_data:addWarning("Can't getting Flow Control Enable Status " ..
                      "speed %s and interface " ..
                      "mode %s for device %d " ..
                      "port(s) %s",
                      speedStrGet(port_speed),
                      interfaceStrGet(
                        port_interface_mode),
                      devNum, tbltostr(ports), values)
        else
          state = values.statePtr;
          if "CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E" ~= state then
            fc_intervals = fc_intervals_get()
            interval = fc_intervals[port_speed]
            if nil ~= interval then
              result, values = myGenWrapper("cpssDxChPortPeriodicFlowControlCounterSet",
                            {{ "IN", "GT_U8", "devNum", devNum},                                 -- devNum
                             { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum", portNum},            -- portNum
                             { "IN", "GT_U32", "value", interval}})                              -- interval
              if 0 ~= result then
                command_data:setFailPortStatus()
                command_data:addWarning("Can't setting Periodic Flow Control Counter " ..
                            "speed %s and interface " ..
                            "mode %s for device %d " ..
                            "port(s) %s",
                            speedStrGet(port_speed),
                            interfaceStrGet(
                              port_interface_mode),
                            devNum, tbltostr(ports), values)
              end
            end
          end
        end
      end
  end
  command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: speed
--------------------------------------------
CLI_addCommand("interface", "speed", {
  func   = speed_func,
  help   = "Set the transmit and receive speeds",
  params = {
    { type = "values", "%port_speed"},
    { type = "named", "#interface_mode" ,mandatory = {"port_interface_mode"}}
  }
})

CLI_addCommand("interface", "no speed", {
  func = function(params)
    params.flagNo = true
    return speed_func(params)
    end,
  help = "Powering down interface"
})
