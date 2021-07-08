--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_aging_time.lua
--*
--* DESCRIPTION:
--*       changing of the aging time
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  bridge_aging_time
--        @description  Changes the aging time of the device 
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--
local function bridge_aging_time(params)
    local status, err
    local result, values
    local dev, timeout
    
    status = true
    
    dev = params["devID"]
    timeout = params["timeout"]
    if (type(timeout) == "nil") then
        timeout = 300
    end
    
    result, values = cpssPerDeviceParamSet(
            "cpssDxChBrgFdbAgingTimeoutSet",
            dev, timeout)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end		
    
    return status, err
end
 

--------------------------------------------
-- type registration: timeout
--------------------------------------------
CLI_type_dict["aging_timeout"] = {
    checker = CLI_check_param_number,
    min=0,
    max=630,
    complete = CLI_complete_param_number,
    help = "Aging-time range in seconds indicating how long an entry remains " ..
           "in address table"
}

--------------------------------------------
-- command registration: bridge aging-time
--------------------------------------------
CLI_addCommand("config", "bridge aging-time", {
    func=bridge_aging_time,
    help="Set the aging time of the address table",
    params={
        {   type="values", "%aging_timeout" },
        {   type="named",
            { format="device %devID", name="devID", help = "Device ID"},
            mandatory={"devID"}
        }
    }
})
