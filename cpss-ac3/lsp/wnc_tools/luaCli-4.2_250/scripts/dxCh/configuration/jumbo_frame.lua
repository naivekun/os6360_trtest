--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* jumbo_frame.lua
--*
--* DESCRIPTION:
--*       Enabling of jumbo frames on the interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 9 $
--*
--********************************************************************************

--includes

--constants
local default_normal_maximum_received_packet_size      = 
    DEFAULT_NORMAL_MAXIMUM_RECEIVED_PACKET_SIZE
local default_jumbo_frame_maximum_received_packet_size = 
    DEFAULT_JUMBO_FRAME_MAXIMUM_RECEIVED_PACKET_SIZE


-- ************************************************************************
---
--  interface_jumbo_frame_func
--        @description  sets given or default Maximum Received Packet size 
--
--        @param params         - params["max_received_packet_size"]: 
--                                maximum received packet size
--
--        @usage __global       - __global["ifRange"]: iterface range 
--
--        @return       true on success, otherwise false and error message
--
local function interface_jumbo_frame_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local max_received_packet_size
    local port_vid, port_vids

    -- Common variables initialization  
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()    

    -- Command specific variables initialization    
    max_received_packet_size = 
        valueIfConditionNotNil(params["max_received_packet_size"], 
                               params["max_received_packet_size"], 
                               default_jumbo_frame_maximum_received_packet_size)
    max_received_packet_size = 
        command_data:getValueIfFlagNoIsNil(
            params, max_received_packet_size, 
            default_normal_maximum_received_packet_size)
    if (params.flagNo == true) and is_xCat3x_in_system() then
        -- the 88e1690 ports supports only next values : 10240
        max_received_packet_size = 10240
    end
    port_vids   = {}
   
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()
            
            -- Maximum port received packet size setting.
            if true == command_data["local_status"]     then
                result, values = cpssPerPortParamSet("cpssDxChPortMruSet",
                                                     devNum, portNum, 
                                                     max_received_packet_size, 
                                                     "mruSize", "GT_U32")   
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to set maximum " ..
                                            "received packet size %d for device " ..
                                            "%d port %d.", max_received_packet_size, 
                                            devNum, portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at maximum received packet size " ..
                                          "setting of device %d port %d: %s", 
                                          devNum, portNum, returnCodes[result])
                end               
            end
            
            -- Port vid getting.
            if true == command_data["local_status"]     then    
                result, values = cpssPerPortParamGet("cpssDxChBrgVlanPortVidGet",
                                                     devNum, portNum, "vid", 
                                                     "GT_U16")
                if        0 == result then
                    port_vid    = values["vid"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to get default " ..
                                            "vlan id on device %d port %d.",
                                            devNum, portNum) 
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at default vlan id getting " ..
                                          "on device %d port %d: %s.", devNum, 
                                          portNum, returnCodes[result])
                end                
            end             
            
            -- Port vid adding.
            if true == command_data["local_status"]     then
                insertInNumberLists(port_vid, port_vids)
            end
            
            command_data:updateStatus()
               
            command_data:updatePorts() 
        end     

        command_data:addWarningIfNoSuccessPorts(
            "Can not set maximum received packet size for all processed ports.")
    end
    
    -- Common variables initialization    
    command_data:initDeviceVlanRange({ ["vlan-range"] = port_vids })

    if true == command_data["status"] then    
        command_data:configure_vlan_mru(max_received_packet_size)
    end
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()             
end


-- ************************************************************************
---
--  vlan_jumbo_frame_func
--        @description  set's given or default Maximum Received Packet size
--
--        @param params         - params["flagNo"]: no-flag;
--                                params["max_received_packet_size"]:    
--                                maximum received packet size
--
--        @usage __global       - __global["devID"]: current vlan device 
--
--        @return       true on success, otherwise false and error message
--
local function vlan_jumbo_frame_func(params)
    -- Common variables declaration
    local result, values
    local devNum, vlanId, vlan_info
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local received_packet_size_index_determinancy, received_packet_size_index
    local max_received_packet_size_changing, max_received_packet_size
    local mru_indexes
    
    -- Common variables initialization    
    command_data:initInterfaceDevVlanRange()    
    command_data:initInterfaceDeviceRange("dev_vlan_range")
    command_data:initDevVlanInfoIterator()
 
    -- Command specific variables initialization    
    if nil == params.flagNo then
        if nil ~= params["max_received_packet_size"] then
            received_packet_size_index_determinancy = false
            max_received_packet_size_changing       = true
            max_received_packet_size                = 
                params["max_received_packet_size"]
        else
            received_packet_size_index_determinancy = false
            max_received_packet_size_changing       = true
            max_received_packet_size                = 
                default_jumbo_frame_maximum_received_packet_size
        end
    else
        received_packet_size_index_determinancy = false
        max_received_packet_size_changing       = true
        max_received_packet_size                = 
            default_normal_maximum_received_packet_size
    end     
    mru_indexes = command_data:getEmptyDeviceNestedTable()
    
    -- Main vlan handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId, vlan_info in 
                                    command_data:getDeviceVlanInfoIterator() do
            command_data:clearVlanStatus()

            command_data:clearLocalStatus()    
            
            -- Maximum received packet size getting.            
            if (true  == command_data["local_status"])            and 
               (false == received_packet_size_index_determinancy) and
               (true  == max_received_packet_size_changing)       then 
                received_packet_size_index = vlan_info["mruIdx"]
            end
            
            -- Maximum received packet size setting.
            if (true  == command_data["local_status"])            and 
               (true  == max_received_packet_size_changing)       then 
                result, values = 
                    cpssPerPortParamSet("cpssDxChBrgVlanMruProfileValueSet",
                                        devNum, received_packet_size_index, 
                                        max_received_packet_size, "mruValue", 
                                        "GT_U32")   
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to set " ..
                                            "maximum received packet size %d " ..
                                            "for device %d vlan %d.", 
                                            max_received_packet_size, devNum, 
                                            vlanId)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at maximum received packet " ..
                                         "size setting of device %d vlan %d: %s", 
                              devNum, vlanId, returnCodes[result])
                end  
            end
            
            -- Saving of maximum received packet size setting.
            if (true  == command_data["local_status"])            and 
               (true  == max_received_packet_size_changing)       then             
                mru_indexes[devNum] = received_packet_size_index
            end
    
            command_data:updateStatus()
               
            command_data:updateVlans() 
        end     
    
        command_data:addWarningIfNoSuccessVlans("Can not set maximum " ..
                                                "received packet size for " ..
                                                "all processed vlans.")
    end
    
    -- Main vlan ports handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId, portNum in 
                                    command_data:getDeviceVlanPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus() 
            
            -- Maximum received packet size getting.            
            if (true  == command_data["local_status"])            and 
               (false == received_packet_size_index_determinancy) and
               (true  == max_received_packet_size_changing)       then 
                received_packet_size_index = mru_indexes[devNum]
            end            
            
            -- Maximum port received packet size setting.
            if (true  == command_data["local_status"])            and 
               (true  == max_received_packet_size_changing)       then 
                result, values = cpssPerPortParamSet("cpssDxChPortMruSet",
                                                     devNum, portNum, 
                                                     max_received_packet_size, 
                                                     "mruSize", "GT_U32")   
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to set " ..
                                            "maximum received packet size %d " ..
                                            "for device  %d port %d.", 
                                            max_received_packet_size, devNum, 
                                            portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at maximum received packet " ..
                                          "size setting of device %d port %d: %s", 
                                          devNum, portNum, returnCodes[result])
                end  
            end
            
            command_data:updateStatus()
               
            command_data:updatePorts()             
        end    
        
        command_data:addWarningIfNoSuccessPorts(
            "Vlan does not contain ports or can not set maximum received " ..
            "packet size for all processed ports.")        
    end
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()             
end


--------------------------------------------
-- command registration: jumbo-frame
--------------------------------------------
CLI_addCommand("interface", "jumbo-frame", {
  func=interface_jumbo_frame_func,
  help="Enable jumbo frames for the device",
  params={
    { type="named",
      { format = "%max_received_packet_size", name = "max_received_packet_size",
                                            help = "Maximum received packed size"
      }
    }
  }
})

CLI_addCommand("interface", "no jumbo-frame", {
  func=function(params)
      params.flagNo=true
      return interface_jumbo_frame_func(params)
  end,
  help="Disable jumbo frames for the device"
})

CLI_addCommand("vlan_configuration", "jumbo-frame", {
  func=vlan_jumbo_frame_func,
  help="Enable jumbo frames for the device",
  params={
    { type="named",
      { format="%max_received_packet_size", name="max_received_packet_size",
                                            help = "Maximum received packed size"
      }
    }
  }
})

CLI_addCommand("vlan_configuration", "no jumbo-frame", {
  func=function(params)
      params.flagNo=true
      return vlan_jumbo_frame_func(params)
  end,
  help="Disable jumbo frames for the device"
})

