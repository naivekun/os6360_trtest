--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_interface_dce_cut_through.lua
--*
--* DESCRIPTION:
--*       Interface configuration of Cut-Through on PIPE.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--******************************************************************************
--  dceCutThroughEnableSet
--
--  @description  Enable/Disable Cut Through forwarding for packets received on
--                the port.
--
--  @params  params["untaggedMode"] - Is configuration mode untagged.
--                                    Applicable values: true,false.
--           params["flagNo"]       - Is flag NO used.
--                                    Applicable values: true,false.
--
--  @return true on success. Otherwise false and error message
--
--******************************************************************************
local function dceCutThroughEnableSet(command_data, devNum, portNum, params)
    local ret, val
    local enable, untaggedEnable

    -- get current configuration
    ret, val = myGenWrapper("cpssPxCutThroughPortEnableGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
            { "OUT", "GT_BOOL", "enablePtr" },
            { "OUT", "GT_BOOL", "untaggedEnablePtr" }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxCutThroughPortEnableGet" ..
              " devNum=" .. devNum ..
              " portNum=" .. portNum ..
              " failed: " .. returnCodes[ret])
        return false, "cpssPxCutThroughPortEnableGet" ..
                      " devNum=" .. devNum ..
                      " portNum=" .. portNum ..
                      " failed: rc[" .. ret .. "]"
    end

    enable = val["enablePtr"]
    untaggedEnable = val["untaggedEnablePtr"]

    if (params["untaggedMode"]) then
        if (params["flagNo"]) then
            untaggedEnable = false  -- interface / no dce cut-through untagged enable
        else
            untaggedEnable = true   -- interface / dce cut-through untagged enable
        end
    else
        if (params["flagNo"]) then
            enable = false          -- interface / no dce cut-through enable
        else
            enable = true           -- interface / dce cut-through enable
        end
    end

    -- set configuration
    ret, val = myGenWrapper("cpssPxCutThroughPortEnableSet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
            { "IN", "GT_BOOL", "enable", enable },
            { "IN", "GT_BOOL", "untaggedEnable", untaggedEnable }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxCutThroughPortEnableSet" ..
              " devNum=" .. devNum ..
              " portNum=" .. portNum ..
              " enable=" .. enable ..
              " untaggedEnable=" .. untaggedEnable ..
              " failed: " .. returnCodes[ret])
        return false, "cpssPxCutThroughPortEnableSet" ..
                      " devNum=" .. devNum ..
                      " portNum=" .. portNum ..
                      " enable=" .. enable ..
                      " untaggedEnable=" .. untaggedEnable ..
                      " failed: rc[" .. ret .. "]"
    end

    return true
end


--******************************************************************************
-- add help: dce
-- add help: dce cut-through
-- add help: dce cut-through untagged
-- add help: no dce
-- add help: no dce cut-through
-- add help: no dce cut-through untagged
--******************************************************************************
CLI_addHelp("interface", "dce", "Configure DCE")
CLI_addHelp("interface", "dce cut-through", "Configure Cut-Through for an interface")
CLI_addHelp("interface", "dce cut-through untagged",
            "Enable Cut-Through forwarding for untagged packets received on the port")
CLI_addHelp("interface", "no dce", "Configure DCE")
CLI_addHelp("interface", "no dce cut-through", "Configure Cut-Through for an interface")
CLI_addHelp("interface", "no dce cut-through untagged",
            "Disable Cut-Through forwarding for untagged packets received on the port")

--******************************************************************************
-- command registration: dce cut-through enable
--******************************************************************************
CLI_addCommand("interface", "dce cut-through enable", {
    func = function(params)
               params["untaggedMode"] = false
               params["flagNo"] = false
               return generic_port_range_func(dceCutThroughEnableSet, params)
           end,
    help = "Enable Cut-Through forwarding for packets received on the port"
})

--******************************************************************************
-- command registration: no dce cut-through enable
--******************************************************************************
CLI_addCommand("interface", "no dce cut-through enable", {
    func = function(params)
               params["untaggedMode"] = false
               params["flagNo"] = true
               return generic_port_range_func(dceCutThroughEnableSet, params)
           end,
    help = "Disable Cut-Through forwarding for packets received on the port"
})

--******************************************************************************
-- command registration: dce cut-through untagged enable
--******************************************************************************
CLI_addCommand("interface", "dce cut-through untagged enable", {
    func = function(params)
               params["untaggedMode"] = true
               params["flagNo"] = false
               return generic_port_range_func(dceCutThroughEnableSet, params)
           end,
    help = "Enable Cut-Through forwarding for untagged packets received on the port"
})

--******************************************************************************
-- command registration: no dce cut-through untagged enable
--******************************************************************************
CLI_addCommand("interface", "no dce cut-through untagged enable", {
    func = function(params)
               params["untaggedMode"] = true
               params["flagNo"] = true
               return generic_port_range_func(dceCutThroughEnableSet, params)
           end,
    help = "Disable Cut-Through forwarding for untagged packets received on the port"
})

--******************************************************************************
--  dceCutThroughMaxBuffersLimitSet
--
--  @description  Set max buffers limit for Cut-Through packets.
--                Packets are handled as non-Cut-Through when number of
--                allocated buffers larger then limit.
--
--  @params  params["buffersLimit"] - Max amount of buffers for Cut-Through
--                                    traffic.
--
--  @return true on success. Otherwise false and error message
--
--******************************************************************************
local function dceCutThroughMaxBuffersLimitSet(command_data, devNum, portNum, params)
    local ret, val
    local buffersLimit

    buffersLimit = params["buffersLimit"]

    ret, val = myGenWrapper("cpssPxCutThroughMaxBuffersLimitSet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
            { "IN", "GT_U32", "buffersLimit", buffersLimit }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxCutThroughMaxBuffersLimitSet" ..
              " devNum=" .. devNum ..
              " portNum=" .. portNum ..
              " buffersLimit=" .. buffersLimit ..
              " failed: " .. returnCodes[ret])
        return false
    end

    return true
end


--******************************************************************************
-- command registration: dce cut-through buffers-limit
--******************************************************************************
CLI_addCommand("interface", "dce cut-through buffers-limit", {
    func = function(params)
               return generic_port_range_func(dceCutThroughMaxBuffersLimitSet, params)
           end,
    help = "Configure max buffers limit for Cut-Through packets",
    params = {
        {
            type = "values",
            {
                format = "%dce_cut_through_buffer_limits",
                name = "buffersLimit",
                help = "Max amount of buffers for Cut-Through traffic"
            },
            mandatory = {
                "buffersLimit"
            }
        }
    }
})
