--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_arp_inspection.lua
--*
--* DESCRIPTION:
--*       changing of the arp inspection status
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  ip_arp_inspection
--        @description  Changes the status of the arp inspection
--
--        @param params         - The parameters
--
--        @usage __global       - __global["ifRange"]: iterface range 
--
--        @return       true if there was no error otherwise false
--
local function ip_arp_inspection(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local mode
    
    -- Command specific variables initialization	
    devNum  = params["devID"]
    mode    = getTrueIfNotNil(params["flagNo"])
    
    -- Setting of port sanity checking.
    if true == command_data["status"]     then
        result, values = 
            myGenWrapper("cpssDxChNstProtSanityCheckSet", {
                         { "IN", "GT_U8",               "dev",      devNum }, 
                         { "IN", "CPSS_NST_CHECK_ENT",  "checkType",  
                                      CPSS_NST_CHECK_ARP_MAC_SA_MISMATCH_E },  
                         { "IN", "GT_BOOL",             "mode",     mode   }})   
        if     0x10 == result then
            command_data:setFailStatus() 
            command_data:addWarning("Sanity checking %s is not allowed on " ..
                                    "device %d.", boolEnablingLowerStrGet(mode), 
                                    devNum)      
        elseif 0x1E == result then      
            command_data:setFailStatus()
            command_data:addError("Sanity checking %s does not supported on " ..
                                  "device %d: %s.", 
                                  boolEnablingLowerStrGet(enable_routing),
                                  devNum, returnCodes[result])                                    
        elseif    0 ~= result then           
            command_data:setFailStatus()
            command_data:addError("Error at sanity checking %s on device %d: %s.", 
                                  boolEnablingLowerStrGet(enable_routing),
                                  devNum, returnCodes[result])
        end
    end
    
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()      
end
 
-- *config*
-- [no] ip arp inspection %devID
CLI_addHelp("config", "ip", "Global IP configuration commands")
CLI_addCommand("config", "ip arp inspection", {
    func=ip_arp_inspection,
    help="ARP inspection set enable status",
    params={{type="values", 
        { format="%devID", help="device id"}
    }}
})
 
CLI_addHelp("config", "no ip", "Global IP configuration commands")
