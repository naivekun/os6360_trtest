--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_cut_through_untagged_enable.lua
--*
--* DESCRIPTION:
--*       enabling cut-through for untagged packets for an interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  dce_cut_through_untagged_enable_func
--        @description  enables cut-through for untagged packets for an 
--                      interface
--
--        @return       true on success, otherwise false and error message
--
local function dce_cut_through_untagged_enable_func(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local cut_through_enabling

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()
    
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
                       
            command_data:clearLocalStatus()  
            
            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
            
            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily)   or
               ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily)    
               then
               -- Getting of cut-through for all packets on interface.
                if true == command_data["local_status"]     then   
                    result, values =         
                        myGenWrapper("cpssDxChCutThroughPortEnableGet",
                                     {{"IN", "GT_U8",   "devNum",   devNum  },
                                      {"IN", "GT_PHYSICAL_PORT_NUM",
                                                        "portNum",  portNum }, 
                                      {"OUT", "GT_BOOL", "enable"           },
                                      {"OUT", "GT_BOOL", "untaggedEnable"   }}) 
                    if        0 == result   then  
                        cut_through_enabling    = values["enable"]
                    elseif 0x10 == result   then
                        command_data:setFailPortStatus() 
                        command_data:addWarning("It is not allowed to get " ..
                                                "enabling of cut-through " ..
                                                "for all packets on device " ..
                                                "%d port %d.", devNum, portNum) 
                    elseif    0 ~= result   then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at enabling getting of " ..
                                              "cut-through for all on " ..
                                              "device %d port %d: %s.", devNum, 
                                              portNum, returnCodes[result])
                    end               
                end      
            
                -- Enabling of cut-through for untagged packets on interface.
                if true == command_data["local_status"]     then   
                    result, values =         
                        myGenWrapper("cpssDxChCutThroughPortEnableSet",
                                     {{"IN", "GT_U8",   "devNum",   devNum  },
                                      {"IN", "GT_PHYSICAL_PORT_NUM",
                                                        "portNum",  portNum }, 
                                      {"IN", "GT_BOOL", "enable",   
                                                       cut_through_enabling },
                                      {"IN", "GT_BOOL", "untaggedEnable",   
                                                                    true    }})                                            
                    if     0x10 == result   then
                        command_data:setFailPortStatus() 
                        command_data:addWarning("It is not allowed to " ..
                                                "enable cut-through for " ..
                                                "untagged packets for an " ..
                                                "interface on device %d.",
                                                devNum) 
                    elseif    0 ~= result   then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at enabling of " ..
                                              "cut-through for untagged " ..
                                              "packets for an interface " ..
                                              "on device %d: %s.", devNum, 
                                              returnCodes[result])
                    end               
                end      
            else
                command_data:addWarningOnceOnKey(
                    devNum, "Family of device %d does not supported.", devNum)           
            end            
            
            command_data:updateStatus() 
      
            command_data:updatePorts()
        end      
        
        command_data:addWarningIfNoSuccessPorts(
            "Can not enable cut-through for untagged packets for all " ..
            "processed ports.")        
    end            
    
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: dce cut-through untagged enable
--------------------------------------------------------------------------------
CLI_addCommand("interface", "dce cut-through untagged enable", {
  func   = dce_cut_through_untagged_enable_func,
  help   = "Enabling cut-through for untagged packets for an interface"
})
