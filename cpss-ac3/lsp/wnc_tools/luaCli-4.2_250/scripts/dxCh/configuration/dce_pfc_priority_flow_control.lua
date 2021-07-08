--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_pfc_priority_flow_control.lua
--*
--* DESCRIPTION:
--*       enabling of pfc on interface/range
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants

--
-- CPSS DxCh priority_flow_control
--[[
typedef enum
{
    CPSS_DXCH_PORT_PFC_DISABLE_ALL_E,
    CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_ONLY_E,
    CPSS_DXCH_PORT_PFC_ENABLE_RESPONSE_ONLY_E,
    CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E
}CPSS_DXCH_PORT_PFC_ENABLE_ENT;
]]--

--require("exec/system_managment")

--[[   
GT_STATUS cpssDxChPortPfcProfileQueueConfigSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileIndex,
    IN GT_U8    tcQueue,
    IN CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
);

typedef struct {
    GT_U32 xonThreshold;
    GT_U32 xoffThreshold;
} CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC;
]]--

local Enable_profile  = 1
local Disable_profile = 0

-- entire threashold values should be verified 
local xonThresholdEnableConstant  = 20
local xoffthresholdEnableConstant = 200

-- entire threashold values should be verified 
local xonThresholdDisableConstant  = 2000
local xoffthresholdDisableConstant = 2000

 
-- ************************************************************************
---
--  show_dce_pfc
--        @description  Displays dce priority flow control on the device
--
--        @param params         - params["devId"]: the device ID
--
--        @return       true
--
local function show_dce_pfc(params)
    local ii, status, err
    local devNum, portNum
    local pfc_enable
    local numberOfPortsInDevice, numberOfTc
    local arr 
    local result, values
    
    print("")
    print("DCE PFC summary")  
    print("---------------")
    
    
    devNum=params.devID
    status = true
    arr = {}
        
--  get the numberOfTrunks capabilities
--  values = system_capability_managment(params)
    numberOfPortsInDevice = NumberOf_PORTS_IN_DEVICE
    numberOfTc = NumberOf_PRIO
    ii = 0
    
--  get global PFC state
    result, values = cpssPerDeviceParamGet("cpssDxChPortPfcEnableGet",
            devNum, "pfc_enable")
--  retive results rom output           
    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    else 
            pfc_enable=values.pfc_enable
    end
        

    if (pfc_enable == 3) then
         print(string.format("%-11s%-22s","PFC is globally ENABLED on device ",tostring (devNum)))      
    else if (pfc_enable == 0) then
        print(string.format("%-11s%-22s","PFC is globally DISABLED on device ",tostring (devNum)))
    else
      print(string.format("ERROR: PFC is not configured "))
    end
    end
-- print(string.format("%s %q %q %q %q", "channel_group:cpssDxChTrunkMemberRemove dev trunk entry.device entry.port", tostring(dev),tostring(port_channel), tostring(entry.device),tostring(entry.port)))    
-- it is interesting to show only enable profile
    local profileIndex = Enable_profile
-- orginize header     
    print("")
    print("Priority   PFC")  
    print("--------   ---")
-- loop on entire priorities in the device    
    while ii < NumberOf_PRIO do
    
            result, values = myGenWrapper(
              "cpssDxChPortPfcProfileQueueConfigGet", {
                    { "IN", "GT_U8" ,"dev", devNum},         
                { "IN", "GT_U32","profileIndex", profileIndex},
                { "IN", "GT_U8", "tcQueue",ii},
                { "OUT", "CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC", "pfcProfileCfgPtr"},
            }
          )
          
        if (result ~=0) then
          status = false
          err = returnCodes[result]              
        end
  
--      prbably it seems as not nessary now, but i prefer to store info into arr[], and then print      
       if values.pfcProfileCfgPtr.xonThreshold == xonThresholdEnableConstant then 
           arr[ii] = "enable" 
         else
        arr[ii] = "disable" 
    end 
      
      -- prepare the next iteration
      ii = ii + 1        
      
  end
  
--  print the data from arr[], in loop on entire priorities
  ii = 0
  while ii<NumberOf_PRIO do
    if type(arr[ii]) ~= "nil" then
        print(string.format("%-11s%-22s", tostring(ii), arr[ii]))
    end
    ii = ii + 1
  end

--  clear array
  arr = nil
  arr = {}
    ii = 0  
--  loop on ports in device
  while ii < NumberOf_PORTS_IN_DEVICE do
        
        -- check if specific port does exist in    
        if does_port_exist(dev,ii) then
            
          result, values = cpssPerPortParamGet(
              "cpssDxChPortPfcProfileIndexGet",
                        devNum, ii, "profileIndex")

          if (result ~=0) then
          status = false
          err = returnCodes[result]              
          end
          --probably it seems as not nessary now
          --but it's preferable to store info into arr[], and then print      
           if values.profileIndex == Enable_profile then 
             arr[ii] = "enable" 
           else
          arr[ii] = "disable" 
        end 
      end

      
      -- prepare the next iteration
      ii = ii + 1        
      
  end
-- orginize header     
    print("")
    print("Interface   PFC")  
    print("---------   ---")
  
--  print the data from arr[], in loop on entire device ports
  ii = 0
  while ii<NumberOf_PORTS_IN_DEVICE do
    if type(arr[ii]) ~= "nil" then
        print(string.format("%s%s%s %-22s",tostring(devNum),"/",tostring(ii),arr[ii]))
    end
    ii = ii + 1
  end      
        
      
return status, err    
  
end


-- ************************************************************************
---
--  set_global_dce_pfc
--        @description  set dce priority flow control on the device 
--
--        @param params         - params["devID"]: the device ID
--
--        @return       true 
--
local function set_global_dce_pfc(params)
    local ii, status, err
    local devNum, portNum
    local pfc_enable,FC_or_HOL
    local numberOfPortsInDevice, numberOfTc
    local result, values
    
    
    status = true
    
    if params["flagNo"] == nil then
      pfc_enable = 3 --CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E
      FC_or_HOL = 1 -- CPSS_DXCH_PORT_HOL_E 
    else 
      pfc_enable = 0 --CPSS_DXCH_PORT_PFC_DISABLE_ALL_E
      FC_or_HOL = 0 --CPSS_DXCH_PORT_FC_E
    end
    
-- device number from parameters (SW device_id)    
    devNum=params.devID
            
--  set global PFC state
    result, values = cpssPerDeviceParamSet(
              "cpssDxChPortPfcEnableSet",
                        devNum, pfc_enable, "pfc_enable")
    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end

--  set global HOL state
    result, values = cpssPerDeviceParamSet(
              "cpssDxChPortFcHolSysModeSet",
                        devNum, FC_or_HOL, "modeFcHol")
    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end

-- need to configure the SHAPER ????
      
return status, err    
  
end


-- ************************************************************************
---
--  set_prio_dce_pfc
--        @description  set dce priority flow control on the device 
--
--        @param params         - params["devID"]: The device ID
--
--        @return       true
--
local function set_prio_dce_pfc(params)
    local ii, status, err
    local devNum, portNum
    local profileIndex, tcQueue
    local numberOfTc
    local entry = {}
    local result, values
    
    status = true
       
-- get device number and tcQueue from parameters (SW device_id)    
    devNum=params.devID
    tcQueue=params.prio
-- only enable profile should be configured, in both, positive and negative cases    
    profileIndex = Enable_profile
        
--  set threshoulds accordingly in x-on, x-off parameters
    if (params.flagNo == nil) then
       entry = {xonThreshold = xonThresholdEnableConstant, xoffThreshold = xoffThresholdEnableConstant}
    else
       entry = {xonThreshold = xonThresholdDisableConstant, xoffThreshold = xoffThresholdDisableConstant}
    end      
            
--  set threshoulds per tc into enable profile state
    result, values = myGenWrapper(
              "cpssDxChPortPfcProfileQueueConfigSet", {
                    { "IN", "GT_U8" ,"dev", devNum},         
                { "IN", "GT_U32","profileIndex", profileIndex},
                { "IN", "GT_U8", "tcQueue", tcQueue},
                { "IN", "CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC", "pfcProfileCfgPtr",entry},
            }
          )
          

    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end

      
return status, err    
  
end


-- ************************************************************************
---
--  set_interface_dce_pfc
--        @description  set dce priority flow control on the interfaces 
--
--        @param params         - params["devID"]: The device ID
--
--        @return       true 
--
local function set_interface_dce_pfc(params)
    local ii, status, err
    local devNum, portNum
    local profileIndex, enablePFC
    local result, values
    
    status = true
    --check "intrface type" of this command. should be ethrnet or range-ethernet
    --is it working on the right interface type (not vlan or anything)
  if (tostring(getGlobal("ifType")) ~= "ethernet") then
    return false, "Can only be used on an ethernet type interface or ethernet range"
  end
  
  -- get entire interfaces range
    local all_ports = getGlobal("ifRange")
    
         
  -- get device number and tcQueue from parameters (SW device_id)    
    devNum=params.devID
    --print(string.format("%s %q", "dce pfc: set_interface_dce_pfc dev port profile_index", tostring(devNum)))
        
  --  set profile accordingly negative and positive cases
    if (params.no == nil) then
       profileIndex = Enable_profile
       enablePFC = 1 -- CPSS_DXCH_PORT_FC_MODE_PFC_E
    else
       profileIndex = Disable_profile
       enablePFC = 0 -- CPSS_DXCH_PORT_FC_MODE_802_3X_E
    end      

  --loop on all prots in range
    --loop on entire ports in "ethernet" or "ethernet range" and add them to the port channel  
  for dev, dev_ports in pairs(all_ports) do
    for key, port in pairs(dev_ports) do

            -- print(string.format("%s %q %q %q", "dce pfc: set_interface_dce_pfc dev port profile_index", tostring(dev),tostring(port), tostring(profileIndex)))           
      -- set threshoulds per tc into enable profile state
      -- only in case that port exists in choosed range
      if does_port_exist(dev,port) then
      
          result, values = cpssPerPortParamSet(
              "cpssDxChPortPfcProfileIndexSet",
                         dev, port, profileIndex, "profileIndex")
          
          if (result ~=0) then
          status = false
          err = returnCodes[result]
                end      
                  
        -- set flow control mode on port   
        result, values = cpssPerPortParamSet(
              "cpssDxChPortFlowControlModeSet",
                         dev, port, enablePFC, "fcMode")
          
          if (result ~=0) then
          status = false
          err = returnCodes[result]
        end
        
        -- set periodic flow control on port   
        result, values = cpssPerPortParamSet(
              "cpssDxChPortPeriodicFcEnableSet",
                        dev, port, enablePFC, "enable", "GT_BOOL")
          
          if (result ~=0) then
          status = false
          err = returnCodes[result]
        end
        
                  
        end
        end
     end
      
return status, err    
  
end


-- *exec*
-- [no] show dce priority-flow-control aging %devID
CLI_addHelp("exec", "show", "Show running system information")
CLI_addHelp("exec", "show dce", "Show dce data center environment")
CLI_addCommand("exec", "show dce priority-flow-control", {
    func=show_dce_pfc,
    help="show dce priority-flow-control information",
    params={{type="values", "%devID"}}
})

-- *config*
-- [no] dce priority-flow-control enable %devID
CLI_addHelp("config", "dce", "dce subcommands")
CLI_addHelp("config", "dce priority-flow-control", "PFC configurations")
CLI_addCommand("config", "dce priority-flow-control enable", {
    func=set_global_dce_pfc,
    help="Globally enabled PFC",
    params={{type="values", "%devID"}}
})
  
-- *config*
-- [no] dce priority-flow-control priority %prio enable %devID
CLI_addCommand("config", "dce priority-flow-control priority", {
    func=set_prio_dce_pfc,
    help="PFC configurations for priority",
    params={
        {   type="values",
            { format="%priority", name="prio", help="The priority (0-7)on which PFC is to be enabled" },
            { format="enable", name="const_enable", help="Priority enabled PFC" },
            "%devID"
        }
    }
})

-- *interface*
-- [no]dce priority-flow-control enable
CLI_addHelp("interface", "dce priority-flow-control", "enable priority flow control on interface/range")
CLI_addCommand("interface", "dce priority-flow-control enable", {
    func=set_interface_dce_pfc,
    help="enable pfc on interface/range"
})
