--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* shutdown.lua
--*
--* DESCRIPTION:
--*       disabling/restarting of an interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  shutdown_func
--        @description  disables/restart's of an interface
--
--        @return       true on success, otherwise false and error message
--
local function shutdown_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local port_state

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()

    -- System specific data initialization.
    wrlOsSaveCurrentStdOutFunction()
    wrlOsResetStdOutFunction()

    -- Command specific variables initialization
    port_state = command_data:getTrueIfFlagNoIsNil(params)

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            -- Port enabling.
            command_data:clearLocalStatus()

            if true == command_data["local_status"]             then
                local result, waNeeded = prvLuaTgfIsPortForceLinkUpWaReq(devNum, portNum)
                if waNeeded == false then
                    result =
                        cpssPerPortParamSet("cpssDxChPortForceLinkDownEnableSet",
                                            devNum, portNum, port_state, "state",
                                                         "GT_BOOL")
                    if     0x10 == result then
                        command_data:setFailPortStatus()
                        command_data:addWarning("\"shutdown\" is not supported on AP ports. " ..
                                                "In order to bring the AP port link down, use the \"no ap-port\" command");

                    elseif 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at going to down port %d " ..
                                              "device %d: %s.", portNum, devNum,
                                              returnCodes[result])
                    end
                else
                -- reverse state for 'link up' logic
                    port_state = not port_state
                    result = prvLuaTgfPortForceLinkUpEnableSet(devNum, portNum, port_state)
                    if 0 ~= result then
                        command_data:addError("Error: prvLuaTgfPortForceLinkUpEnableSet port %d " ..
                                              "device %d: %s.", portNum, devNum,
                                              returnCodes[result])
                    end
                end
            end

            command_data:updateStatus()

            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not going to down all processed ports.")
    end

    -- System specific data updating.
    wrlOsRestoreStdOutFunction()

    -- Command data postprocessing
    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------
-- command registration: vlan-mtu
--------------------------------------------------------
CLI_addCommand("interface", "shutdown", {
  func   = shutdown_func,
  help   = "Shutdown the selected interface"
})

--------------------------------------------------------
-- command registration: vlan-mtu
--------------------------------------------------------
CLI_addCommand("interface", "no shutdown", {
  func   = function(params)
               params.flagNo = true
               shutdown_func(params)
           end,
  help   = "Restarting the selected interface"
})

