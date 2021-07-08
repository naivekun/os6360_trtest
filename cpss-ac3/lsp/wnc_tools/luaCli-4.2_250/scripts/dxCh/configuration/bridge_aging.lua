--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_aging.lua
--*
--* DESCRIPTION:
--*       enabling / disaling if the aging time on a device
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


---
--  bridge_aging
--        @description  Enables / disales the aging time on a device
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--
local function bridge_aging(params)
    local status, err
    local result, values
    local dev, mode
    
    status = true
    
    dev = params["devID"]
    mode = 0
    if params.flagNo then
        mode = 1
    end
    
    result, values = cpssPerDeviceParamSet(
            "cpssDxChBrgFdbMacTriggerModeSet",
            dev, mode)
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end
    
    -- always use CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E
    result, values = cpssPerDeviceParamSet(
            "cpssDxChBrgFdbActionModeSet",
            dev, "CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E","mode","CPSS_FDB_ACTION_MODE_ENT")
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end
    
    return status, err
end
 
 
--------------------------------------------
-- command registration: bridge aging
--------------------------------------------
CLI_addHelp("config", "bridge", "Global Bridge table configuration commands")
CLI_addCommand("config", "bridge aging", {
    func=bridge_aging,
    help="Set the aging of the address table",
    --params={{type="values", "%devID"}}
    
    params={ { type="named", { format="device %devID",name="devID", help="device number" }
			}}
})

--------------------------------------------
-- command registration: no bridge aging
--------------------------------------------
CLI_addHelp("config", "no bridge", "Global Bridge table configuration commands")
CLI_addCommand("config", "no bridge aging", {
    func=function(params)
        params.flagNo=true
        return bridge_aging(params)
    end,
    help="Set the aging of the address table",
    -- params={{type="values", "%devID"}}
    params={ { type="named", { format="device %devID",name="devID", help="device number" }
        }}
})
