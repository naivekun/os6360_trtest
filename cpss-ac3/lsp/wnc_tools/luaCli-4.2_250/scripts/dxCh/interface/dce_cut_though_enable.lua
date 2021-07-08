--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_cut_though_enable.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of cut-through for an interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  dce_cut_through_enable_func
--        @description  enables/disables cut-through for an interface
--
--        @param params         - params["flagNo"]: no-flag
--
--        @return       true on success, otherwise false and error message
--
local function dce_cut_through_enable_func(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local cut_through_enabling, untagged_cut_through_enabling
    local port_speed, speed_limiting_condition

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()

    -- Command specific variables initialization.
    cut_through_enabling    = getFalseIfNotNil(params["flagNo"])

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()
            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily)   or
               ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily)
               then
                -- Getting of cut-through for all packets on interface.
            if true == command_data["local_status"]     then
                    result, values =
                        myGenWrapper("cpssDxChCutThroughPortEnableGet",
                                     {{"IN", "GT_U8",   "devNum",   devNum  },
                                      {"IN", "GT_PHYSICAL_PORT_NUM",
                                                        "portNum",  portNum },
                                      {"OUT", "GT_BOOL", "enable"           },
                                      {"OUT", "GT_BOOL", "untaggedEnable"   }})
                if        0 == result then
                        untagged_cut_through_enabling   =
                            values["untaggedEnable"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                        command_data:addWarning("It is not allowed to get " ..
                                                "enabling of cut-through " ..
                                                "for all packets on device  " ..
                                                "%d port %d.", devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at enabling getting of " ..
                                              "cut-through for all on " ..
                                              "device %d port %d: %s.", devNum,
                                          portNum, returnCodes[result])
                end
            end

                -- Enabling of cut-through for untagged packets on interface.
            if true == command_data["local_status"]     then
                    result, values =
                        myGenWrapper("cpssDxChCutThroughPortEnableSet",
                                     {{"IN", "GT_U8",   "devNum",   devNum  },
                                      {"IN", "GT_PHYSICAL_PORT_NUM",
                                                        "portNum",  portNum },
                                      {"IN", "GT_BOOL", "enable",
                                                       cut_through_enabling },
                                      {"IN", "GT_BOOL", "untaggedEnable",
                                              untagged_cut_through_enabling }})
                    if     0x10 == result   then
                        command_data:setFailPortStatus()
                        command_data:addWarning("It is not allowed to " ..
                                                "enable cut-through for " ..
                                                "untagged packets for an " ..
                                                "interface on device %d.",
                                                devNum)
                    elseif    0 ~= result   then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at enabling of " ..
                                              "cut-through for untagged " ..
                                              "packets for an interface " ..
                                              "on device %d: %s.", devNum,
                                              returnCodes[result])
            end
            end
            else
                command_data:addWarningOnceOnKey(
                    devNum, "Family of device %d does not supported.", devNum)
            end

            command_data:updateStatus()

            command_data:updatePorts()
        end
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: dce cut-through enable
--------------------------------------------------------------------------------
CLI_addHelp("interface",    "dce cut-through",
                                            "Configuring of cut-through for " ..
                                            "an interface")
CLI_addCommand("interface", "dce cut-through enable", {
  func   = dce_cut_through_enable_func,
  help   = "Enabling of cut-through for an interface"
})


--------------------------------------------------------------------------------
-- command registration: no dce cut-through enable
--------------------------------------------------------------------------------
CLI_addHelp("interface",    "no dce cut-through",
                                            "Configuring of cut-through for " ..
                                            "an interface")
CLI_addCommand("interface", "no dce cut-through enable", {
  func   = function(params)
               params.flagNo = true
               return dce_cut_through_enable_func(params)
           end,
  help   = "Disabling of cut-through for an interface"
})


-- ************************************************************************
---
--  dce_cut_through_port_byte_count_update_func
--        @description  set cut-through per port byte count update
--
--        @param params         - params["correction-value "]: signed value -7..7
--
--        @return       true on success, otherwise false and error message
--
local function dce_cut_through_port_byte_count_update_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local signedAddedValue
    local toSubtractOrToAdd, subtractedOrAddedValue

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()

    signedAddedValue = params["correction-value"]
    if (signedAddedValue >= 0) then
        toSubtractOrToAdd = false
        subtractedOrAddedValue = signedAddedValue
    else
        toSubtractOrToAdd = true
        subtractedOrAddedValue = (0 - signedAddedValue)
    end

    -- Command specific variables initialization.

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            if (is_sip_6(devNum))     then
                if true == command_data["local_status"]     then
                    result, values =
                        myGenWrapper("cpssDxChCutThroughPortByteCountUpdateSet",
                                     {{"IN",  "GT_U8",   "devNum",  devNum },
                                      {"IN",  "GT_PHYSICAL_PORT_NUM",
                                                        "portNum", portNum },
                                      {"IN", "GT_BOOL", "toSubtractOrToAdd", toSubtractOrToAdd         },
                                      {"IN", "GT_U32", "subtractedOrAddedValue", subtractedOrAddedValue }
                                         })

                    if     0x10 == result   then
                            command_data:setFailPortStatus()
                            command_data:addWarning(
                                "It is not allowed to set cut-through " ..
                                " per port byte count update value" ..
                                "interface on device %d/%d",
                                 devNum,portNum )
                        elseif    0 ~= result   then
                            command_data:setFailPortAndLocalStatus()
                            command_data:addError(
                            "It is not allowed to set cut-through " ..
                            " per port byte count update value" ..
                            "interface on device %d/%d, %s",
                             devNum,portNum,  returnCodes[result])
                end
            end
            else
                command_data:addWarningOnceOnKey(
                    devNum, "Family of device %d does not supported.", devNum)
            end

            command_data:updateStatus()

            command_data:updatePorts()
        end
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- type registration: cut_throug_ude_index
--------------------------------------------
CLI_type_dict["cut_through_bc_corr_value"] = {
    checker = CLI_check_param_number,
    min=(0-7),
    max=7,
    complete = CLI_complete_param_number,
    help = "cut-through per port byte count update"
}

--------------------------------------------------------------------------------
-- command registration: dce cut-through port-byte-count-update
--------------------------------------------------------------------------------
CLI_addCommand("interface", "dce cut-through port-byte-count-update", {
    func   = dce_cut_through_port_byte_count_update_func,
    params =
    {
        {
            type = "named",
            { format = "correction-value %cut_through_bc_corr_value",
              name   = "correction-value"},
            mandatory = { "correction-value" }
        },
    },
    help   = "set cut-through per port byte count update"
})


