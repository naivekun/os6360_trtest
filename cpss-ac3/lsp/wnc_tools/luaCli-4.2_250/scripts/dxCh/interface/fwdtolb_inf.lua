--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* fwdtolb_inf.lua
--*
--* DESCRIPTION:
--*     'Forwarding to loopback port' related commands configured
--*      per physical port(s).
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- ************************************************************************
---
--  lbProfileSetFunc
--        @description  set a physical port's loopback profile
--
--        @param params command parameters
--
--        @return       true on success, otherwise false and error message
--
local function lbProfileSetFunc(params)
    local command_data = Command_Data()
    local argsList = {}
    if params.rx then
        table.insert(argsList, {direction = "CPSS_DIRECTION_INGRESS_E",
                                profileType = "Source",
                                profile = params.rx})
    end
    if params.tx then
        table.insert(argsList, {direction = "CPSS_DIRECTION_EGRESS_E",
                                profileType = "Target",
                                profile = params.tx})
    end

    -- function called for every dev/port.
    local func = function(command_data, devNum, portNum, params)
        if not is_supported_feature(devNum, "FWD_TO_LB") then
            command_data:addErrorAndPrint(
                "device %d doesn't support 'Forwarding to Loopback Port'",
                devNum)

            return
        end

        for _, args in pairs(argsList) do
            local result, values = myGenWrapper(
                "cpssDxChPortLoopbackProfileSet", {
                    { "IN",  "GT_U8",                "devNum",    devNum},
                    { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum",   portNum},
                    { "IN",  "CPSS_DIRECTION_ENT",   "direction", args.direction},
                    { "IN",  "GT_U32",               "profile",   args.profile}
            })
            command_data:handleCpssErrorDevPort(
                result,
                string.format("%s loopback profile is set to %d",
                              args.profileType, args.profile),
                devNum, portNum)
        end
    end

    if #argsList == 0 then
        command_data:addWarning("Nothing is done! Please specify Source and/or Target Loopback Profile.")
    else
        command_data:initInterfaceDevPortRange()
        command_data:iterateOverPorts(func, params)
    end

   command_data:analyzeCommandExecution()
   command_data:printCommandExecutionResults()
   return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  lbPortSetFunc
--        @description  set a loopback port used by
--                      FORWARD/FROM_CPU packets destined to specified port(s)
--
--        @param params CLI command parameters
--
--        @return       true on success, otherwise false and error message
--
local function lbPortSetFunc(params)
    local command_data = Command_Data()

    -- function used by dev/port iterator
    local func = function(command_data, devNum, portNum, params)
        if not is_supported_feature(devNum, "FWD_TO_LB") then
            command_data:addErrorAndPrint(
                "device %d doesn't support 'Forwarding to Loopback Port'",
                devNum)

            return
        end
        local result, values = myGenWrapper(
            "cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet", {
                { "IN",  "GT_U8",                  "devNum",          devNum},
                { "IN",  "GT_PHYSICAL_PORT_NUM",   "portNum",         portNum},
                { "IN",  "GT_PHYSICAL_PORT_NUM",   "loopbackPortNum", params.port}
        })
        command_data:handleCpssErrorDevPort(
            result, string.format("Loopback port %d assignment", params.port),
            devNum, portNum)
    end

   command_data:initInterfaceDevPortRange()
   command_data:iterateOverPorts(func, params)
   command_data:analyzeCommandExecution()
   command_data:printCommandExecutionResults()
   return command_data:getCommandExecutionResults()
end


--******************************************************************************
--******************* COMMANDS REGISTRATIONS ***********************************
--******************************************************************************
--------------------------------------------------------------------------------
-- command registration: forward-to-loopback profile
--------------------------------------------------------------------------------
CLI_addHelp("interface", "forward-to-loopback",
            "'Forwarding to Loopback Port' commands")


CLI_addCommand("interface", "forward-to-loopback profile",
    {
        func = lbProfileSetFunc,
        help = "assign source/target loopback profiles",
        params = {
            { type = "named",
              {format = "rx %loopback_profile", name="rx",   help = "assign source Loopback profile"},
              {format = "tx %loopback_profile", name="tx",   help = "assign target Loopback profile"},
            },
        }
})

--------------------------------------------------------------------------------
-- command registration: forward-to-loopback profile
--------------------------------------------------------------------------------
CLI_addCommand("interface", "forward-to-loopback port",
    {
        func = lbPortSetFunc,
        help = "assign a Loopback Port number",
        params = {
            { type = "values",
              {format ="%port"}
            }
        }
})
