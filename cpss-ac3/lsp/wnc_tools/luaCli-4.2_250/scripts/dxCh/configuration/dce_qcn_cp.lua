--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_qcn_cp_feedback_weight.lua
--*
--* DESCRIPTION:
--*       configuring of QCN egress queue
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes
local DEFAULT_QCN_FEEDBACK_WEIGHT   = 2

--constants

-- ************************************************************************
---
--  dce_qcn_cp_func
--        @description  configures of QCN egress queue
--
--        @param params         - params["feedback_weight"]: feedback
--                                weight;
--                                params["all_device"]: property of
--                                applying of all avaible devices, could
--                                be inrelevant;
--                                params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function dce_qcn_cp_func(params)
    -- Common variables declaration.
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local feedback_weight, calculation_config
    local min_sample_base, min_sample_base_interval, min_sample_base_bitmap
    local sample_interval_entry_index, sample_interval_entry
    local feedback_weight_exponent

    -- Common variables initialization.
    command_data:initAllDeviceRange(params)

    -- Command specific variables initialization.
    feedback_weight = isEquivalent(params["qcn"], "feedback_weight")
    feedback_weight_exponent    =
        command_data:getValueIfFlagNoIsNil(params, params["feedback_weight"], 1)
    calculation_config          = { wExp        = feedback_weight_exponent,
                                    fbLsb       = 0,
                                    deltaEnable = true,
   			            fbMin       = 0,
                                    fbMax       = 0 }
    min_sample_base             = isEquivalent(params["qcn"], "min_sample_base")
    sample_interval_entry_index = 0
    min_sample_base_interval    =
        command_data:getValueIfFlagNoIsNil(params, params["min_sample_base"],
                                           150000)
    min_sample_base_bitmap      = bit_shr(min_sample_base_interval, 16)
    min_sample_base_interval    = bit_and(min_sample_base_interval, 0xFFFF)
    sample_interval_entry       = {interval     = min_sample_base_interval,
                                   randBitmap   = min_sample_base_bitmap    }

    -- Main port handling cycle.
    if true == command_data["status"]               then
        local iterator
        for iterator, devNum, portNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
                -- Seting of feedback calculation configuration.
                if (true == command_data["local_status"])   and
                   (true == feedback_weight)                then
                    result, values =
                        cpssPerDeviceParamSet("cpssDxChPortCnFbCalcConfigSet",
                                              devNum, calculation_config,
                                              "fbCalcCfg",
                                              "CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC")
                    if     0x10 == result   then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to set " ..
                                                "feedback calculation " ..
                                                "configuration on device %d.",
                                                devNum)
                    elseif    0 ~= result   then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at setting of feedback " ..
                                              "calculation configuration on " ..
                                              "device %d: %s.", devNum,
                                              returnCodes[result])
                    end
                end

                -- Seting of minimum number of bytes to enqueue.
                if (true == command_data["local_status"])   and
                   (true == min_sample_base)                then
                    result, values =
                        myGenWrapper("cpssDxChPortCnSampleEntrySet",
                                     {{"IN", "GT_U8",    "devNum",   devNum },
                                      {"IN", "GT_U8",    "entryIndex",
                                                sample_interval_entry_index },
                                      {"IN", "CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC",
                                             "entry", sample_interval_entry }})
                    if     0x10 == result   then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("It is not allowed to set " ..
                                                "minimum number of bytes to " ..
                                                "enqueue on device %d.",
                                                devNum)
                    elseif    0 ~= result   then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at setting of minimum " ..
                                              "number of bytes to enqueue " ..
                                              "on device %d: %s.", devNum,
                                              returnCodes[result])
                    end
                end
            else
                command_data:setFailDeviceStatus()
                command_data:addWarning("Family of device %d does not " ..
                                        "supported.", devNum)
            end

            command_data:updateStatus()

            command_data:updateDevices()
        end

        command_data:addWarningIfNoSuccessDevices(
            "Can not configure QCN egress queue on all processed devices.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: dce qcn cp
--------------------------------------------------------------------------------
CLI_addCommand("config", "dce qcn cp", {
  func   = dce_qcn_cp_func,
  help   = "Configuring of the QCN egress queue",
  params = {
      { type = "named",
          "#feedback_weight",
          "#min_sample_base",
          alt   = { qcn = { "feedback_weight", "min_sample_base" } },
          "#all_device",
        mandatory = { "qcn", "all_device" }
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no dce qcn cp feedback-weight
--------------------------------------------------------------------------------
CLI_addHelp("config",       "no dce qcn cp",        "Setting of QCN egress " ..
                                                    "queue properties")
CLI_addCommand("config",    "no dce qcn cp feedback-weight", {
  func   = function(params)
               params.flagNo    = true
               params.qcn       = "feedback_weight"
               dce_qcn_cp_func(params)
           end,
  help   = "Setting to default of the Feedback Weight of a QCN egress queue",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no dce qcn cp feedback-weight
--------------------------------------------------------------------------------
CLI_addCommand("config", "no dce qcn cp min-sample-base", {
  func   = function(params)
               params.flagNo = true
               params.qcn       = "min_sample_base"
               dce_qcn_cp_func(params)
           end,
  help   = "Returning to default of the minimum number of bytes to enqueue " ..
           "in a QCN egress queue",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})
