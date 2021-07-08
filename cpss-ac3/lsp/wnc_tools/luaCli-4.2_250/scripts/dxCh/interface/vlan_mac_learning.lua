--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_mac_learning.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of learning of MAC addresses on a specific Vlans
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  vlan_mac_learning_func
--        @description  enables/disables of learning of MAC addresses on 
--                      a specific Vlans
--
--        @param params             - params["flagNo"]: no command property
--
--        @usage common_global      - common_global[ifRange]: interface 
--                                    vlan/port range;
--                                    common_global[ifType]: interface type
--
--        @return       true on success, otherwise false and error message
--
local function vlan_mac_learning_func(params)
    -- Common variables declaration
    local result, values
    local devNum, vlanId
    local command_data = Command_Data()
    -- Command specific variables declaration
    local mac_address_lerning    
        
    -- Common variables initialization    
    command_data:initInterfaceDevVlanRange()

    -- Command specific variables initialization
    mac_address_lerning = getTrueIfNil(params["flagNo"])
    
    -- Main vlan handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId in 
                                    command_data:getInterfaceVlanIterator() do
            command_data:clearVlanStatus()

            -- Vlan mac adress learning setting.
            command_data:clearLocalStatus(command_data)
            
            if true == command_data["local_status"]     then            
                result, values = 
                    cpssPerVlanParamSet("cpssDxChBrgVlanLearningStateSet", 
                                        devNum, vlanId, mac_address_lerning, 
                                        "state", "GT_BOOL")
                if     0x10 == result then
                    command_data:setFailVlanStatus() 
                    command_data:addWarning("It is not allowed to %s mac adress " ..
                                            "learning for device %d vlan %d.", 
                                            boolPredicatStrGet(mac_address_lerning), 
                                            devNum, vlanId)                                                   
                elseif 0 ~= result then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at mac adress learning setting " ..
                                          "of device %d vlan %d: %s", devNum, 
                                          vlanId, returnCodes[result])
                end  
            end
    
            command_data:updateStatus()
            
            command_data:updateVlans()
        end

        command_data:addWarningIfNoSuccessVlans(
            "Can not configure ports of all processed vlans.")
    end
    
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()    
end


--------------------------------------------------------------------------------
-- command registration: vlan-mac learning
--------------------------------------------------------------------------------
CLI_addHelp("vlan_configuration", "vlan-mac", "Vlan mac subcommands")
CLI_addCommand("vlan_configuration", "vlan-mac learning", {
  func   = vlan_mac_learning_func,
  help   = "Enabling of learning of MAC addresses on a specific Vlans"
})

--------------------------------------------------------------------------------
-- command registration: no vlan-mac learning
--------------------------------------------------------------------------------
CLI_addHelp("vlan_configuration", "no vlan-mac", "Vlan mac no-subcommands")
CLI_addCommand("vlan_configuration", "no vlan-mac learning", {
  func   = function(params)
               params.flagNo = true
               return vlan_mac_learning_func(params)
           end,
  help   = "Disabling of learning of MAC addresses on a specific Vlans"
})

-- ************************************************************************
---
--  vlan_mac_na_to_cpu_func
--        @description  enables/disables of NA-to-CPU of MAC addresses on 
--                      a specific Vlans
--
--        @param params             - params["flagNo"]: no command property
--
--        @usage common_global      - common_global[ifRange]: interface 
--                                    vlan/port range;
--                                    common_global[ifType]: interface type
--
--        @return       true on success, otherwise false and error message
--
local function vlan_mac_na_to_cpu_func(params)
    -- Common variables declaration
    local result, values
    local devNum, vlanId
    local command_data = Command_Data()
    -- Command specific variables declaration
    local na_to_cpu    
        
    -- Common variables initialization    
    command_data:initInterfaceDevVlanRange()

    -- Command specific variables initialization
    na_to_cpu = getTrueIfNil(params["flagNo"])
    
    -- Main vlan handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId in 
                                    command_data:getInterfaceVlanIterator() do
            command_data:clearVlanStatus()

            -- Vlan mac adress learning setting.
            command_data:clearLocalStatus(command_data)
            
            local apiName = "cpssDxChBrgVlanNaToCpuEnable"
            local isError , result, values = genericCpssApiWithErrorHandler(command_data,
                apiName, {
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U16","vlanId",vlanId},
                {"IN","GT_BOOL","enable",na_to_cpu}
            })

            command_data:updateStatus()
            command_data:updateVlans()
        end

        command_data:addWarningIfNoSuccessVlans(
            "Can not configure ports of all processed vlans.")
    end
    
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()    
end

--------------------------------------------------------------------------------
-- command registration: vlan-mac na-to-cpu
--------------------------------------------------------------------------------
CLI_addHelp("vlan_configuration", "vlan-mac na-to-cpu", "Per vlan enable 'NA to CPU' (New Addresses)")
CLI_addCommand("vlan_configuration", "vlan-mac na-to-cpu", {
  func   = vlan_mac_na_to_cpu_func,
  help   = "Enabling of 'NA to CPU' (New Addresses) on a specific Vlan(s)"
})

--------------------------------------------------------------------------------
-- command registration: no vlan-mac na-to-cpu
--------------------------------------------------------------------------------
CLI_addHelp("vlan_configuration", "no vlan-mac na-to-cpu", "Per vlan disable 'NA to CPU' (New Addresses)")
CLI_addCommand("vlan_configuration", "no vlan-mac na-to-cpu", {
  func   = function(params)
               params.flagNo = true
               return vlan_mac_na_to_cpu_func(params)
           end,
  help   = "Disabling of 'NA to CPU' (New Addresses) on a specific Vlan(s)"
})
