--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_interface_dce_pfc.lua
--*
--* DESCRIPTION:
--*       Interface configuration of PFC
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--*******************************************************************************
--  dcePfcProfileIndexSet
--
--  @description  Bind source port to a PFC profile
--
--  @param  params["profileIndex"] - PFC profile index
--
--  @return  true on success, otherwise false and error message
--
--*******************************************************************************
local function dcePfcProfileIndexSet(command_data, devNum, portNum, params)
    local ret, val

    ret, val = myGenWrapper("cpssPxPortPfcProfileIndexSet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
            { "IN", "GT_U32", "profileIndex", params["profileIndex"] }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxPortPfcProfileIndexSet" ..
              " devNum=" .. devNum ..
              " portNum=" .. portNum ..
              " profileIndex=" .. params["profileIndex"] ..
              " failed: " .. returnCodes[ret])
        return false, "cpssPxPortPfcProfileIndexSet" ..
                      " devNum=" .. devNum ..
                      " portNum=" .. portNum ..
                      " profileIndex=" .. params["profileIndex"] ..
                      " failed: rc[" .. ret .. "]"
    end

    return true
end


--******************************************************************************
-- add help: dce
-- add help: dce -priority-flow-control
--******************************************************************************
CLI_addHelp("interface", "dce", "Configure DCE")
CLI_addHelp("interface", "dce priority-flow-control", "Configure PFC")

--******************************************************************************
-- command registration: dce priority-flow-control bind profile
--******************************************************************************
CLI_addCommand("interface", "dce priority-flow-control bind profile", {
    func = function(params)
               return generic_port_range_func(dcePfcProfileIndexSet, params)
           end,
    help ="Bind PFC profile to an interfaces",
    params = {
        {
            type = "values",
            {
                format = "%dce_pfc_profile",
                name = "profileIndex",
                help = "PFC profile index"
            },
            mandatory = {
                "profileIndex"
            }
        }
    }
})
