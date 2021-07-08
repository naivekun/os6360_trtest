--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* switchport_customer_vlan.lua
--*
--* DESCRIPTION:
--*       setting of the port’s outer (service) VLAN when the interface is in
--*       customer mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChIsPortTaggingCmdSupported")

--constants 


-- ************************************************************************
---
--  switchport_customer_vlan_func
--        @description  set's the port’s outer (service) VLAN when the 
--                      interface is in customer mode
--
--        @param params             - params["vlanId"]: configured vlan
--
--        @usage __global           - __global["ifRange"]: iterface range 
--
--        @return       0 on success, otherwise error code and error 
--                      message
-- 
local function switchport_customer_vlan_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum, vlanId
    local command_data = Command_Data()
    -- Command specific variables declaration
    local as_usual_port_working, vlan_tagged_packets_sending
    local port_customer_mode_staying
    local customer_mode_dev_ports
    local devFamily
    
    -- Common variables initialization    
    command_data:initInterfaceDevPortRange()
    command_data:initAllAvailableDevicesRange()
    command_data:initVlanRange(params)
        
    -- Common variables initialization
    customer_mode_dev_ports = command_data:getEmptyDeviceNestedTable()
  
    -- Main port handling cycle.
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
    
            command_data:clearLocalStatus()
            
            -- Device family recognition.
            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet","(devNum)", devNum)
    
            -- Ingress vlan access configuration getting.    
            if true == command_data["local_status"]         then
                result, values = 
                    cpssPerPortParamGet("cpssDxChBrgNestVlanAccessPortGet", 
                                        devNum, portNum, "enable", "GT_BOOL")  

                if       0 == result then
                    as_usual_port_working = logicalNot(values["enable"])
                elseif 0x10 == result then
                    command_data:setFailPortAndLocalStatus() 
                    command_data:addWarning("It is not allowed to get vlan " ..
                                            "access configuration of port " ..
                                            "%d on device %d.", portNum,
                                            devNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at vlan access configuration " ..
                                          "getting of of port %d on " ..
                                          "device %d: %s.", portNum, devNum, 
                                          returnCodes[result])
                end
            end            

            -- Determination of vlan tagged packet supporting.            
            if (true == command_data["local_status"])       and
               (true == as_usual_port_working)              then
                if ("CPSS_PP_FAMILY_DXCH_XCAT_E"  == devFamily)     or
                   ("CPSS_PP_FAMILY_DXCH_LION_E"  == devFamily)     or
                   ("CPSS_PP_FAMILY_DXCH_XCAT2_E" == devFamily)     or
                   ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily)     or
                   is_sip_5(devNum)     or
                   ("CPSS_PP_FAMILY_DXCH_LION2_E" == devFamily)     then 
                    vlan_tagged_packets_sending = true
                else
                result, values = 
                    cpssPerPortParamGet("cpssDxChBrgNestVlanEtherTypeSelectGet", 
                                        devNum, portNum, "ingressVlanSel", 
                                        "CPSS_ETHER_MODE_ENT") 
                if       0 == result then
                    vlan_tagged_packets_sending = 
                        isEquivalent(values["ingressVlanSel"], 
                                     "CPSS_VLAN_ETHERTYPE0_E")
                elseif 0x10 == result then
                    command_data:setFailPortAndLocalStatus() 
                        command_data:addWarning("It is not allowed to get " ..
                                                "VLAN EtherType on device " ..
                                                "%d port %d.", devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at VLAN EtherType " ..
                                              "getting on device %d port " ..
                                              "%d: %s.", devNum, portNum, 
                                              returnCodes[result])
                    end              
                end              
            end

            -- Determination of port customer mode staying.
            if (true == command_data["local_status"])       then            
                port_customer_mode_staying = (true == as_usual_port_working) and 
                                            (true == vlan_tagged_packets_sending) 
            end
            
            -- Dev/port range forming.
            if (true == command_data["local_status"])       and
               (true == port_customer_mode_staying)          then
                table.insert(customer_mode_dev_ports[devNum], portNum)
            end
                              
            command_data:addConditionalWarning(logicalNot(port_customer_mode_staying), 
                                               "Interface port %d of device %d " ..
                                               "in not in customer mode.", 
                                               portNum, devNum)
                                               
            command_data:updateStatusOnCondition(port_customer_mode_staying)             
            
            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not get customer mode of all processed interface ports.")
    end   
    
    -- Common variables initialization 
    command_data:initDevPortRange(customer_mode_dev_ports)
    
    -- Main vlan handling cycle.
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum, vlanId in 
                          command_data:getInterfaceDevPortVlanIterator() do
            command_data:clearVlanStatus()
            
            command_data:clearLocalStatus()
            
            -- Customer vlan access port configuring.            
            if true == command_data["local_status"]         then
                result, values = 
                    cpssPerPortParamSet("cpssDxChBrgNestVlanAccessPortSet", 
                                        devNum, portNum, true, "enable", 
                                        "GT_BOOL")  

                if  0x10 == result then
                    command_data:setFailVlanAndLocalStatus() 
                    command_data:addWarning("It is not allowed to configure " ..
                                            "nested VLAN access port %d on" ..
                                            "device %d.", portNum, devNum)
                elseif 0 ~= result then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at confiruring of nested " ..
                                          "VLAN access port %d on device %d: %s.", 
                                          portNum, devNum, returnCodes[result])
                end
    end

            -- Port's default VLAN Id setting.
            if true == command_data["local_status"]         then
                result, values = 
                    cpssPerPortParamSet("cpssDxChBrgVlanPortVidSet", 
                                        devNum, portNum, vlanId, "vlanId", 
                                        "GT_U16")  
                if  0x10 == result then
                    command_data:setFailVlanAndLocalStatus() 
                    command_data:addWarning("It is not allowed to set " ..
                                            "default VLAN Id of port %d on" ..
                                            "device %d.", portNum, devNum)
                elseif 0 ~= result then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at default VLAN Id setting " ..
                                          "of port %d on device %d: %s.", 
                                          portNum, devNum, returnCodes[result])
                end
            end  
            
            command_data:updateStatus()             
            
            command_data:updateVlans()            
        end
        
        command_data:addWarningIfNoSuccessVlans(
            "Can not set none of the processed vlans as port's default.")        
    end
            
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()             
end            


-- ************************************************************************
---
--  no_switchport_customer_vlan_func
--        @description  set's using of interface’s PVID as the customer 
--                      Vlan
--
--        @param params             - command entry params: does not used
--
--        @usage __global           - __global["ifRange"]: iterface range 
--
--        @return       0 on success, otherwise error code and error
--                      message
-- 
local function no_switchport_customer_vlan_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum, vlanId
    local command_data = Command_Data()
   
    -- Common variables initialization    
    command_data:initInterfaceDevPortRange()   
   
    -- Main port handling cycle.
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
    
            command_data:clearLocalStatus()
   
            -- Nested vlan access port configuring.            
            if true == command_data["local_status"]         then
                result, values = 
                    cpssPerPortParamSet("cpssDxChBrgNestVlanAccessPortSet", 
                                        devNum, portNum, false, "enable", 
                                        "GT_BOOL")  

                if  0x10 == result then
                    command_data:setFailVlanAndLocalStatus() 
                    command_data:addWarning("It is not allowed to configure " ..
                                            "nested VLAN access port %d on" ..
                                            "device %d.", portNum, devNum)
                elseif 0 ~= result then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at confiruring of nested " ..
                                          "VLAN access port %d on device %d: %s.", 
                                          portNum, devNum, returnCodes[result])
                end
            end
   
            command_data:updateStatus()
            
            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not set nested vlan access of all processed interface ports.")
    end   
    
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()     
end

--------------------------------------------------------
-- command registration: switchport customer vlan 
--------------------------------------------------------
CLI_addHelp({ "interface", "port-channel" }, "switchport customer", 
                                    "The port is connected to customer equipment")
CLI_addCommand({ "interface", "port-channel" }, "switchport customer vlan", {
  func   = switchport_customer_vlan_func,
  help   = "vlan",
  params = {
      { type = "values",
          "%vlanId"
      }
  }
})


--------------------------------------------------------
-- command registration: no switchport customer vlan
--------------------------------------------------------
CLI_addHelp({ "interface", "port-channel" }, "no switchport", 
                                    "Configure switch port definition in vlan")
CLI_addHelp({ "interface", "port-channel" }, "no switchport customer", 
                                    "The port is connected to customer equipment")
CLI_addCommand({ "interface", "port-channel" }, "no switchport customer vlan", {
  func   = no_switchport_customer_vlan_func,
  help   = "vlan"
})
