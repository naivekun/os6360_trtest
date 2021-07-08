--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_dce_qcn.lua
--*
--* DESCRIPTION:
--*       showing of the information of the QCN feature
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_dce_qcn_func
--        @description  show's the information of the QCN feature
--
--        @param params         - params["all"]: all devices port or all 
--                                ports of given device cheking switch, 
--                                could be irrelevant;
--                                params["devID"]: checked device number, 
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil;
--                                params["port-channel"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil
--
--        @return       true on success, otherwise false and error message
--
local function show_dce_qcn_func(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration 
    local qcn_enabling_string_prefix, qcn_enabling_string_suffix
    local qcn_enabling, qcn_enabling_string 
    local cnm_priority_string_prefix, cnm_priority_string_suffix
    local cnm_priority, cnm_priority_string   
    local qcn_cnm_text_block_header_string, qcn_cnm_text_block_footer_string
    local qcn_cnm_text_block_string   
    local qcn_admin_enabling, qcn_admin_enabling_string
    local queue, queue_string
    local qcn_oper_enabling, qcn_oper_enabling_string    
    local priority_table_header_string, priority_table_footer_string
    local priority_table_string
    
    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)
    command_data:initInterfaceDeviceRange()
    command_data:initDevicesPriorityDataRange()
    command_data:clearPrioritiesCount()    

    -- Command specific variables initialization. 
    qcn_enabling_string_prefix          = "QCN is "
    qcn_enabling_string_suffix          = ""
    cnm_priority_string_prefix          = "CNM priority: "
    cnm_priority_string_suffix          = "\n\n"
    qcn_cnm_text_block_header_string    = ""
    qcn_cnm_text_block_footer_string    = ""
    priority_table_header_string        =
        "Priority  QCN Admin   Queue   QCN oper \n" ..
        "--------  ----------  -----  ----------\n"
    priority_table_footer_string        = "\n"

    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in  command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()  
                    
            command_data:clearLocalStatus()   

            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
            
            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
                -- Getting of QCN enabling.
                command_data:clearLocalStatus() 
                
                if true  == command_data["local_status"] then  
                    result, values = 
                        cpssPerDeviceParamGet("cpssDxChPortCnModeEnableGet",
                                              devNum, "enable", "CPSS_DXCH_PORT_CN_MODE_ENT")  
                    if        0 == result   then
                        qcn_enabling    = values["enable"]
                    elseif 0x10 == result   then
                        command_data:setFailEntryStatus() 
                        command_data:addWarning("It is not allowed to get " ..
                                                "QCN enabling on device %d.", 
                                                devNum) 
                    elseif    0 ~= result   then
                        command_data:setFailEntryAndLocalStatus()
                        command_data:addError("Error at getting of QCN " ..
                                              "enabling on device %d: %s.",  
                                              devNum, returnCodes[result])
                    end               
                
                    if         0 == result  then
                        qcn_enabling_string = 
                            boolEnabledLowerStrGet(qcn_enabling)
                    else
                        qcn_enabling_string = "n/a"
                    end
                end
                
                command_data:updateStatus()
                
                -- QCN enabling string formatting and adding.   
                command_data:setResultStr(qcn_enabling_string_prefix, 
                                          qcn_enabling_string,
                                          qcn_enabling_string_suffix)
                command_data:addResultToResultArray()             
                
                -- Geting of CNM priority.
                command_data:clearLocalStatus()
                
                if true == command_data["local_status"]     then   
                    result, values = 
                        cpssPerDeviceParamGet("cpssDxChPortCnFrameQueueGet",
                                              devNum, "tcQueue", "GT_U8")  
                    if        0 == result   then
                        cnm_priority    = values["tcQueue"]
                    elseif 0x10 == result   then
                        command_data:setFailEntryStatus() 
                        command_data:addWarning("It is not allowed to get " ..
                                                "CNM priority on device %d.", 
                                                devNum) 
                    elseif    0 ~= result   then
                        command_data:setFailEntryAndLocalStatus()
                        command_data:addError("Error at getting of CNM " ..
                                              "priority on device %d: %s.",  
                                              devNum, returnCodes[result])
                    end               
                
                    if         0 == result  then
                        cnm_priority_string = tostring(cnm_priority)
                    else
                        cnm_priority_string = "n/a"
                    end
                end
                
                command_data:updateStatus()
                
                -- CNM priority string formatting and adding.   
                command_data:setResultStr(cnm_priority_string_prefix, 
                                          cnm_priority_string,
                                          cnm_priority_string_suffix)
                command_data:addResultToResultArray()
                
                command_data:updateDevices()                
            else
                command_data:addWarning("Family of device %d does not " ..
                                        "supported.", devNum)                   
            end            
        end
    end  

    -- QCN/CNM text block string formatting.
    qcn_cnm_text_block_string       = command_data:getResultArrayStr() 
    qcn_cnm_text_block_string       =
        command_data:getStrOnDevicesCount(
            qcn_cnm_text_block_header_string, qcn_cnm_text_block_string, 
            qcn_cnm_text_block_footer_string,
            "There is no QCN/CNM information to show.\n")         
    
    -- Common variables initialization
    command_data:clearResultArray()    
    
    -- Main device priorities handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, priority in  
                                command_data:getDevicesPriorityIterator() do
            command_data:clearPriorutyStatus()  
                 
            command_data:clearLocalStatus()  
            
            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
            
            if ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
               ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then
                qcn_admin_enabling_string   = "n/a"
                queue_string                = "n/a"
                qcn_oper_enabling_string    = "n/a"
    
                command_data:updateStatus()
                
                -- Resulting string formatting and adding.
                command_data["result"] = 
                    string.format("%-10s %-11s%-7s %-11s", 
                                  alignLeftToCenterStr(tostring(priority), 8),
                                  qcn_admin_enabling_string,
                                  alignLeftToCenterStr(tostring(queue_string), 
                                                       5),
                                  qcn_oper_enabling_string)
                command_data:addResultToResultArray()
                
                command_data:updatePriorities()                
            end
        end
    end   
	  
    -- Resulting table string formatting.
    priority_table_string   = command_data:getResultArrayStr()
    priority_table_string   =
        command_data:getStrOnPrioritiesCount(
            priority_table_header_string, priority_table_string, 
            priority_table_footer_string,
            "There is no priority information to show.\n") 
        
    -- Resulting table string formatting
    command_data["result"] = qcn_cnm_text_block_string .. 
                             priority_table_string  
        
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()            
end


--------------------------------------------------------------------------------
-- command registration: show dce qcn
-------------------------------------------------------------------------------- 
CLI_addCommand("exec", "show dce qcn", {
    func   = show_dce_qcn_func,
    help   = "Showing of the information of the QCN feature",
    params = {
        { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
        }
    } 
})
