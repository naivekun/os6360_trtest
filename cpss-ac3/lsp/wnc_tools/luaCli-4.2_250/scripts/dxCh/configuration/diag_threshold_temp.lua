--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* diag_threshold_temp.lua
--*
--* DESCRIPTION:
--*       Set Threshold for Interrupt. If Temperature Sensors is equal or above
--*       the threshold, interrupt is activated. 
--*                              
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  diag_threshold_temp
--        @description  Changes the aging time of the device 
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--

local function diag_threshold_temp(params)

    local status, err
    local result, values

    result, values = myGenWrapper(

        "cpssDxChDiagDeviceTemperatureThresholdSet", {
            { "IN", "GT_U8"  , "devNum", params.devID}, -- devNum
            { "IN", "GT_32" , "threshold",  params.threshold}
            }
    )

    if (result ~= 0) then
        status = false
        err = returnCodes[result]

    end
    return status, err
end

--------------------------------------------

-- command registration: diag threshold-temperature

--------------------------------------------

CLI_type_dict["threshold"] = {
    checker = CLI_check_param_number,
    min=-142,
    max=228,
    help = "Set Threshold for Interrupt. If Temperature Sensors is equal or above " ..
           "the threshold, interrupt is activated."
}  

CLI_addCommand("config", "diag threshold-temperature", {

    func = diag_threshold_temp,
    help = "Set Threshold for Interrupt",
    params={
        { type="values",
            "%devID",
            "%threshold"
        }
    }
}) 
