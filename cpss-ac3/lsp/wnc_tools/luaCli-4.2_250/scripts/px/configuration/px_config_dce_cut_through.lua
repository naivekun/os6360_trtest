--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_config_dce_cut_through.lua
--*
--* DESCRIPTION:
--*       Configuration of Cut-Through for specified User Priority on PIPE.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--******************************************************************************
--  dceCutThroughUpEnableSet
--
--  @description  Enable/Disable tagged packets with the specified UP
--                to be Cut-Through.
--
--  @params  params["device"]  - device number
--           params["up"]      - User Priority of VLAN or DSA tagged packet.
--           params["flagNo"]  - Is flag NO used.
--                               Applicable values: true,false
--
--  @return true on success. Otherwise false and error message
--
--******************************************************************************
local function dceCutThroughUpEnableSet(params)
    local ret, val
    local enable

    if (params["flagNo"]) then
        enable = false
    else
        enable = true
    end

    ret, val = myGenWrapper("cpssPxCutThroughUpEnableSet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", params["device"] },
            { "IN", "GT_U32", "up", params["up"] },
            { "IN", "GT_BOOL", "enable", enable }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxCutThroughUpEnableSet" ..
              " devNum=" .. params["device"] ..
              " up=" .. params["up"] ..
              " enable=" .. enable ..
              " failed: " .. returnCodes[ret])
        return false, "cpssPxCutThroughPortEnableGet" ..
                      " devNum=" .. params["device"] ..
                      " up=" .. params["up"] ..
                      " enable=" .. enable ..
                      " failed: rc[" .. ret .. "]"
    end

    return true
end


--******************************************************************************
-- add help: dce
-- add help: dce cut-through
-- add help: no dce
-- add help: no dce cut-through
--******************************************************************************
CLI_addHelp("config", "dce", "Configure DCE")
CLI_addHelp("config", "dce cut-through", "Configure Cut-Through")
CLI_addHelp("config", "no dce", "Configure DCE")
CLI_addHelp("config", "no dce cut-through", "Configure Cut-Through")

--******************************************************************************
-- command registration: dce cut-through priority
--******************************************************************************
CLI_addCommand("config", "dce cut-through priority", {
    func = function(params)
               params["flagNo"] = false
               return dceCutThroughUpEnableSet(params)
           end,
    help = "Enable tagged packets with the specified UP to be Cut-Through",
    params = {
        {
            type = "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            {
                format = "up %cos_user_priority",
                name = "up",
                help = "User Priority"
            },
            mandatory = {
                "device", "up"
            }
        }
    }
})

--******************************************************************************
-- command registration: no dce cut-through priority
--******************************************************************************
CLI_addCommand("config", "no dce cut-through priority", {
    func = function(params)
               params["flagNo"] = true
               return dceCutThroughUpEnableSet(params)
           end,
    help = "Disable tagged packets with the specified UP to be Cut-Through",
    params = {
        {
            type = "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            {
                format = "up %cos_user_priority",
                name = "up",
                help = "User Priority"
            },
            mandatory = {
                "device", "up"
            }
        }
    }
})
