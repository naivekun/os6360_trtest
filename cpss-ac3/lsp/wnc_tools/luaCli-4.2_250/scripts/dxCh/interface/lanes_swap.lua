--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* lanes_swap.lua
--*
--* DESCRIPTION:
--*       binding port Rx/Tx PCS and SerDes lanes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlDxChPortXgPcsLanesSwapSet")

local function lanes_swap_func(params)
    -- Common variables declaration.
    local result, values
    local devNum, portNum
    local iterator
    local command_data = Command_Data()

    -- Command  specific variables
    local rx0 = params["rx_lane_0"]
    local rx1 = params["rx_lane_1"]
    local rx2 = params["rx_lane_2"]
    local rx3 = params["rx_lane_3"]
    local tx0 = params["tx_lane_0"]
    local tx1 = params["tx_lane_1"]
    local tx2 = params["tx_lane_2"]
    local tx3 = params["tx_lane_3"]

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()


    for iterator, devNum, portNum in command_data:getPortIterator() do
      command_data:clearPortStatus()
      command_data:clearLocalStatus()
      result, values =  wrLogWrapper("wrlDxChPortXgPcsLanesSwapSet",
                                     "(devNum, portNum, rx0, rx1, rx2, rx3, tx0, tx1, tx2, tx3)",
                                       devNum, portNum, rx0, rx1, rx2, rx3, tx0, tx1, tx2, tx3)
      if 0x4 == result then
          -- bad param
          command_data:setFailPortStatus()
          command_data:addWarning("It is not allowed to set lane swap " ..
                    "for device %d " ..
                    "port %s",
                    devNum, portNum)
      elseif 0x3 == result then
          -- out of range
          command_data:setFailPortStatus()
          command_data:addWarning("Serdes lane numbers out of range")
      elseif 0x2 == result then
          -- bad value
          command_data:setFailPortStatus()
          command_data:addWarning("Multiple connections detected")
      elseif 0x1E == result then
          -- not applicable device
          command_data:setFailPortStatus()
          command_data:addWarning("Lane swapping not applicable for this device")
      elseif 0 ~= result then
          command_data:setFailPortStatus()
          command_data:addWarning("Error setting lane swap " ..
                    "for device %d " ..
                    "port %s",
                    devNum, portNum)
      end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------
-- command registration: lanes-swap
--------------------------------------------
CLI_addCommand("interface", "lanes-swap", {
  func   = lanes_swap_func,
  help   = "Set swapping configuration of XAUI or HyperG.Stack port SERDES Lanes",
  params = {{ type = "named",
      { format = "rx_lane_0 %GT_U32", name="rx_lane_0", help = "The serdes lane to be bound to pcs lane 0" },
      { format = "rx_lane_1 %GT_U32", name="rx_lane_1", help = "The serdes lane to be bound to pcs lane 1" },
      { format = "rx_lane_2 %GT_U32", name="rx_lane_2", help = "The serdes lane to be bound to pcs lane 2" },
      { format = "rx_lane_3 %GT_U32", name="rx_lane_3", help = "The serdes lane to be bound to pcs lane 3" },
      { format = "tx_lane_0 %GT_U32", name="tx_lane_0", help = "The serdes lane to be bound to pcs lane 0" },
      { format = "tx_lane_1 %GT_U32", name="tx_lane_1", help = "The serdes lane to be bound to pcs lane 1" },
      { format = "tx_lane_2 %GT_U32", name="tx_lane_2", help = "The serdes lane to be bound to pcs lane 2" },
      { format = "tx_lane_3 %GT_U32", name="tx_lane_3", help = "The serdes lane to be bound to pcs lane 3" },
      mandatory = { "rx_lane_0", "rx_lane_1", "rx_lane_2", "rx_lane_3",
                    "tx_lane_0", "tx_lane_1", "tx_lane_2", "tx_lane_3" }
  }}
})
