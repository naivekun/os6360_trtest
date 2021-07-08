--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_qcn_cp_set_point.lua
--*
--* DESCRIPTION:
--*       configuring of the QCN set-point of an egress queue
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  dce_qcn_cp_set_point_func
--        @description  configures of the QCN set-point of an egress queue
--
--        @param params         - params["pfc"]: point for PFC queues;
--                                params["all_device"]: property of
--                                applying of all avaible devices, could
--                                be inrelevant;
--                                params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function dce_qcn_cp_set_point_func(params)
    -- Common variables declaration.
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.

    -- Common variables initialization.
    command_data:initAllDeviceRange(params)

    -- Main port handling cycle.
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum, portNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
                command_data:setFailDeviceStatus()
                command_data:addWarning("Command will be implementer later")
            else
                command_data:setFailDeviceStatus()
                command_data:addWarning("Family of device %d does not " ..
                                        "supported.", devNum)
            end

            command_data:updateStatus()

            command_data:updateDevices()
        end

        command_data:addWarningIfNoSuccessDevices(
            "Can not set feedback calculation configuration for all " ..
            "processed devices.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: dce qcn cp set-point
--------------------------------------------------------------------------------
CLI_addCommand("config", "dce qcn cp set-point", {
  func   = dce_qcn_cp_set_point_func,
  help   = "Configuring of the QCN set-point of an egress queue",
  params = {
      { type = "named",
          "#pfc",
          "#non-pfc",
          "#all_device",
          alt   = { any_pfc = { "pfc", "non-pfc" }},
        mandatory = { "any_pfc", "all_device" }
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no dce qcn cp set-point pfc
--------------------------------------------------------------------------------
CLI_addCommand("config", "no dce qcn cp set-point pfc", {
  func   = function(params)
               params.flagNo    = true
               dce_qcn_cp_set_point_func(params)
           end,
  help   = "Setting to default of the QCN set-point of an egress queue",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no dce qcn cp set-point pfc
--------------------------------------------------------------------------------
CLI_addHelp("config",    "no dce qcn cp set-point", "Setting to default of " ..
                                                    "the QCN set-point of " ..
                                                    "an egress queue")
CLI_addCommand("config", "no dce qcn cp set-point non-pfc", {
  func   = function(params)
               params.flagNo    = true
               params.any_pfc   = "non-pfc"
               dce_qcn_cp_set_point_func(params)
           end,
  help   = "Setting to default of the QCN set-point of an egress queue",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
    }
  }
})
