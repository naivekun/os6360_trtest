--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* hostname.lua
--*
--* DESCRIPTION:
--*       The host name definition.  
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes

--constants
local default_hostname = "Console"


-- ************************************************************************
---
--  set_hostname
--        @description  sets the common_global[hostname] variable
--
--        @param params         - params["hostname"]: host name string
--
--        @return       true on success, otherwise false and error message
--
local function set_hostname(params)
    if true ~= params.flagNo then
        setGlobal("hostname", params["hostname"])
        CLI_setCmdPrompt(params["hostname"])
    else
        setGlobal("hostname", default_hostname)
        CLI_setCmdPrompt()
    end
    
    return true
end
    
--------------------------------------------
-- command registration: hostname
--------------------------------------------
CLI_addCommand("config", "hostname", {
    func=set_hostname,
    help="Set system's network name",
    params={ { type="values", "%hostname" } }
})
CLI_addCommand("config", "no hostname", {
    func=function(params)
        params.flagNo=true
        return set_hostname(params)
    end,
    help="Set system's network name"
})

