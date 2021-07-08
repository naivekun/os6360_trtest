--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_interface_dce_qcn.lua
--*
--* DESCRIPTION:
--*       DCE Quantized Congestion Notification interface configuration on PIPE
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


--*******************************************************************************
--  setDceCnTermination
--
--  @description  Enable/Disable termination of CNM (Congestion Notification
--                Message) on given port.
--
--  @param  params["flagNo"]     - if true configuration will set to default
--
--  @return  true on success, otherwise false and error message
--
--*******************************************************************************
local function setDceCnTermination(command_data, devNum, portNum, params)
    local ret, val
    local enable

    if (params["flagNo"]) then
        enable = false
    else
        enable = true
    end

    ret, val = myGenWrapper("cpssPxPortCnTerminationEnableSet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
            { "IN", "GT_BOOL", "enable", enable }
        })
    if (ret ~= LGT_OK) then
        print("cpssPxPortCnTerminationEnableSet" ..
              " devNum=" .. devNum ..
              " portNum=" .. portNum ..
              " enable=" .. tostring(enable) ..
              " failed: " .. returnCodes[ret])

        return false, "cpssPxPortCnTerminationEnableSet" ..
                      " devNum=" .. devNum ..
                      " portNum=" .. portNum ..
                      " enable=" .. tostring(enable) ..
                      " failed: rc=" .. ret
    end

    return true
end


--******************************************************************************
-- add help: dce qcn
-- add help: dce qcn cp
-- add help: no dce qcn
-- add help: no dce qcn cp
--******************************************************************************
CLI_addHelp("interface", "dce qcn", "Configure QCN")
CLI_addHelp("interface", "dce qcn cp", "Configure Congestion Point")
CLI_addHelp("interface", "no dce qcn", "Configure QCN")
CLI_addHelp("interface", "no dce qcn cp", "Configure Congestion Point")

--******************************************************************************
-- command registration: dce qcn cp enable queue
--******************************************************************************
CLI_addCommand("interface", "dce qcn cp enable queue", {
    func = function(params)
               params["flagNo"] = false
               return generic_port_range_func(setDceCnTermination, params)
           end,
    help = "Enable Congestion Point creation for an interface"
})

--******************************************************************************
-- command registration: no dce qcn cp enable queue
--******************************************************************************
CLI_addCommand("interface", "no dce qcn cp enable queue", {
    func = function(params)
               params["flagNo"] = true
               return generic_port_range_func(setDceCnTermination, params)
           end,
    help = "Disable Congestion Point creation for an interface"
})
