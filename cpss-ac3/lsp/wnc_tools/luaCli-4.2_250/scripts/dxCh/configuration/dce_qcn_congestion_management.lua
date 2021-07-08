--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_qcn_congestion_management.lua
--*
--* DESCRIPTION:
--*       enabling of priority flow control on interface/range
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes

--constants

--
-- CPSS DxCh congestion notification
--
--
--


-- default QCN configuration parameters (should be tuned)
local DefaultScaleFactor = 512
local DefaultCnProfileConfigthreshold = 10
local DefaultCnProfileSet = 0
local OriginalPacketLenagh = 0
 

-- ************************************************************************
---
--  show_dce_qcn
--        @description  Displays dce qcn on the device 
--
--        @param params         - params["devID"]: The device ID
--
--        @return       true 
--
local function show_dce_qcn(params)
    local ii, status, err
    local devNum, portNum
    local arr 
    
    print("")
    print("DCE QCN summary")  
    print("---------------")
    
    
    devNum=params.devID
    status = true
    arr = {}
    ii = 0
    
--  get global QCN state
    local result, values = cpssPerDeviceParamGet("cpssDxChPortCnModeEnableGet",
            devNum, "qcn_enable", "CPSS_DXCH_PORT_CN_MODE_ENT")
--  retive results from output           
    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end
        
  
    if (values.qcn_enable == true) then
         print(string.format("%-11s%-22s","QCN is globally ENABLED on device ",tostring (devNum)))      
    else 
        print(string.format("%-11s%-22s","QCN is globally DISABLED on device ",tostring (devNum)))
    end

    -- get CN messages ethertype
    result, values = cpssPerDeviceParamGet("cpssDxChPortCnEtherTypeGet", 
            devNum, "ethertype")
                      
  -- retive results from output           
    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end
    
    
    print(string.format("%-11s%-22x","Ethertype for CN messages is 0x",values.ethertype))      
    
    -- get CN messages prioirity queue
    result, values = cpssPerDeviceParamGet(
        "cpssDxChPortCnFrameQueueGet", devNum, "tcQueue")
          

    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end
    
   
    print(string.format("%-11s%-22s","CNM (congestion notification message) Priority Queue is ",tostring(values.tcQueue)))      
    
    
-- orginize header     
    print("")
    print("Priority   QCN")  
    print("--------   ---")
-- loop on entire priorities in the device  
    local profileIndex = DefaultCnProfileSet  
    while ii < NumberOf_PRIO do
    
            result, values = myGenWrapper(
              "cpssDxChPortCnProfileQueueConfigGet", {
                    { "IN", "GT_U8" ,"dev", devNum},                         
                { "IN", "GT_U32", "profileIndex", profileIndex},
                { "IN", "GT_U8", "tcQueue", ii},
                { "OUT", "CPSS_PORT_CN_PROFILE_CONFIG_STC", "cnProfileCfgPtr"},
              }  
          )          
          
        if (result ~=0) then
          status = false
          err = returnCodes[result]
                    return status, err                        
        end
-- print(string.format("%s %q", "dce qcn: get_global_dce_qcn dev, aware ", tostring(type(values.cnProfileCfgPtr.cnAware))))
-- print(string.format("%s %q", "dce qcn: get_global_dce_qcn dev, threshold ", tostring(values.cnProfileCfgPtr.threshold)))
-- probably it seems as not nessary now, but i prefer to store info into arr[], and then print      
       if (values.cnProfileCfgPtr.cnAware == true) then 
           arr[ii] = "enable" 
         else
        arr[ii] = "disable" 
    end 
      
      -- prepare the next iteration
      ii = ii + 1        
      
  end
  
--  print the data from arr[], in loop on entire priorities
  ii = 0
  while ii < NumberOf_PRIO do
    if type(arr[ii]) ~= "nil" then
        print(string.format("%-11s%-22s", tostring(ii), arr[ii]))
    end
    ii = ii + 1
  end
  
-- clear array
  ii = 0
  arr = nil
  arr = {}
--  loop on ports in device
  while ii < NumberOf_PORTS_IN_DEVICE do
        
        -- check if specific port does exist in range
        if does_port_exist(dev,ii) then
            
          result, values = cpssPerPortParamGet(
              "cpssDxChPortCnFcEnableGet",
                            dev, port, "enable","GT_BOOL")
                            
          if (result ~=0) then
          status = false
          err = returnCodes[result]              
          end
          --probably it seems as not nessary now
          --but it's preferable to store info into arr_port[], and then print      
           if values.enable == true then 
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
    print("Interface   QCN")  
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
--  set_global_dce_qcn
--        @description  set dce congestion notification on the device e 
--
--        @param params         - params["devID"]: The device ID
--
--        @return       true
--
local function set_global_dce_qcn(params)
    local ii, status, err
    local devNum, portNum
    local qcn_enable
    local entry = {}
    
    status = true
    
    if params.flagNo == nil then
      qcn_enable = true
    else 
      qcn_enable = false
    end
    
-- device number from parameters (SW device_id)    
    devNum=params.devID
            
--  set global QCN state
--  print(string.format("%s %q", "dce qcn: set_global_dce_qcn dev ", tostring(qcn_enable)))
    local result, values = cpssPerDeviceParamSet("cpssDxChPortCnModeEnableSet", devNum,
            qcn_enable, "qcn_enable", "GT_BOOL")
    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end
    
    if (true == qcn_enable) then
        -- configure entry
 
        entry = {qosProfileId = 0, 
                 isRouted = 0,
                 overrideUp = false, 
                 cnmUP = 0,
                 defaultVlanId = 1,
                 scaleFactor = DefaultScaleFactor,
                 version = 1,
                 cpidMsb = {l={[0] = 0,[1] = 0}}, -- "GT_U64 union" entry configuration
                 cnUntaggedEnable = true,
                 forceCnTag = false,
                 flowIdTag = 0, 
                 appendPacket = false
                 }
                                  
      result, values = cpssPerDeviceParamSet(
              "cpssDxChPortCnMessageGenerationConfigSet", devNum,
                        entry, "cnmGenerationCfgPtr", "CPSS_PORT_CNM_GENERATION_CONFIG_STC")
          
        -- set CN packets length (currently to original)            
    result, values = cpssPerDeviceParamSet(
              "cpssDxChPortCnPacketLengthSet", devNum,
                        OriginalPacketLenagh)
          
      if (result ~=0) then
        status = false
        err = returnCodes[result]              
        --print(string.format("%s %q", "dce qcn: set_global_dce_qcn ERROR ", tostring(entry.cpidMsb)))
      end
     end
--print(string.format("%s %q", "dce qcn: set_global_dce_qcn dev ", tostring(entry.cpidMsb)))
-- need to configure the SHAPER ????
      
return status, err    
  
end


-- ************************************************************************
---
--  set_prio_dce_qcn
--        @description  set dce qcn on the devicee 
--
--        @param params         - params["devID"]: the device ID
--
--        @return       true 
--
local function set_prio_dce_qcn(params)
    local ii, status, err
    local devNum
    local tcQueue,AwareCn
    local entry = {}
    
    status = true
       
-- get device number and tcQueue from parameters (SW device_id)    
    devNum = params.devID
    tcQueue = params.prio
    local profileSet = DefaultCnProfileSet
        
--  set tcQueue 
    if (params.flagNo == nil) then
       AwareCn = true
    else
       AwareCn = false
    end      
  
    entry = {cnAware = AwareCn, threshold = DefaultCnProfileConfigthreshold}
        
             
--  set threshoulds per tc into enable profile state
    local result, values = myGenWrapper(
              "cpssDxChPortCnProfileQueueConfigSet", {
                    { "IN", "GT_U8" ,"dev", devNum},                         
                { "IN", "GT_U32", "profileSet", profileSet},
                { "IN", "GT_U8", "tcQueue", tcQueue},
                { "IN", "CPSS_PORT_CN_PROFILE_CONFIG_STC", "cnProfileCfgPtr", entry},
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
--  set_ethertype_dce_qcn
--        @description  set dce qcn ethrtype (ethertype) dec (devID 
--
--        @param params         - params["devID"]: the device ID
--
--        @return       true 
--
local function set_ethertype_dce_qcn(params)
    local ii, status, err
    local devNum
    local ethertype
    
    status = true
       
-- get device number and ethrtype from parameters    
    devNum    = params.devID
    
    if (params.flagNo == nil) then
     ethertype =  tonumber(params.ethertype, 16) -- convert of string to number with base 16
    else
       ethertype = 0x5555 --default value
    end
    
-- call cpss configuration function        
    local result, values = cpssPerDeviceParamSet(
              "cpssDxChPortCnEtherTypeSet",
                        devNum, ethertype)
    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end

return status, err    
end


-- ************************************************************************
---
--  set_dce_qcn_cp_feedback
--        @description  set dce qcn on the device 
--
--        @param params         - params["devID"]: the device ID
--
--        @return       true
--
local function set_dce_qcn_cp_feedback(params)
    local status, err
    local devNum
    local feedback_value, feedback_enable
    local entry = {}
    
    status = true
       
-- get device number and tcQueue from parameters (SW device_id)    
    devNum   = params.devID
    feedback_value = params.feedback
-- it seems that we need to calculate the parameters from the input ????????????? 
        
--  set tcQueue 
    if (params.flagNo == nil) then
       feedback_enable = true
    else
       feedback_enable = false
    end      
    entry ={
            wExp = 0,
            fbLsb = 0,
            deltaEnable = feedback_enable,
         fbMin = 0,
            fbMax =0
           }
                
--  configure feedback parametrs
    local result, values = myGenWrapper(
              "cpssDxChPortCnFbCalcConfigSet", {
                    { "IN", "GT_U8" ,"dev", devNum},                         
                { "IN", "CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC", "fbCalcCfgPtr", entry}
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
--  set_prio_dce_qcn_cnm
--        @description  set dce priority queueu of the CN messages on the 
--                      device 
--
--        @param params         - params["devID"]: the device ID
--
local function set_prio_dce_qcn_cnm(params)
    local ii, status, err
    local devNum
    local tcQueue
     
    status = true
       
-- get device number and tcQueue from parameters (SW device_id)    
    devNum     = params.devID
    tcQueue    = params.prio
         
--  set tcQueue 
    if (params.flagNo == nil) then
       tcQueue    = params.prio
    else
       tcQueue = 0
    end      
             
--  set tc for CN messages
    local result, values = cpssPerDeviceParamSet(
              "cpssDxChPortCnFrameQueueSet",
                        devNum,
                        tcQueue, "tcQueue", "GT_U8")

    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end

      
return status, err    
  
end


-- ************************************************************************
---
--  set_interface_dce_qcn
--        @description  set dce qcn on the interfaces  
--
--        @param params         - params["devID"]: the device ID
--
local function set_interface_dce_qcn(params)
    local ii, status, err
    local devNum, portNum
    local enable, result, values
    
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
 
        
  --  set profile accordingly negative and positive cases
    if (params.flagNo == nil) then
       enable = true 
    else
       enable = false
    end      

  --loop on all prots in range
    --loop on entire ports in "ethernet" or "ethernet range" and add them to the port channel  
  for dev, dev_ports in pairs(all_ports) do
    for key, port in pairs(dev_ports) do

                      
      -- set threshoulds per tc into enable profile state
      -- only in case that port exists in choosed range
      if does_port_exist(dev,port) then
      
          result, values = cpssPerPortParamSet(
                    "cpssDxChPortCnFcEnableSet",
                            dev, port,
                            enable, "enable", "GT_BOOL")
          
        if (result ~=0) then
          status = false
          err = returnCodes[result]          
        end  
        
        result, values = cpssPerDeviceParamSet(
                    "cpssDxChPortCnTerminationEnableSet",
                            dev, port,
                            enable, "enable", "GT_BOOL")
          
          
          if (result ~=0) then
          status = false
          err = returnCodes[result]
        end                  
        end
        end
     end
      
return status, err    
  
end


-- *config*
-- [no] dce qcn priority %prio enable %devID
CLI_addCommand("config", "dce qcn priority", {
    func=set_prio_dce_qcn,
    help="dce qcn priority (0-7) enable DEV_ID",
    params={
        {   type="values",
            { format="%priority", name="prio", help="The priority (0-7)on which PFC is to be enabled" },
            "enable",
            "%devID"
        }
    }
})

CLI_type_dict["ethertype"] = {
    checker = CLI_check_param_hexstring,
    min=4,
    max=4,
    help="The ethertype of the CN messages , mandatory two bytes in hex (i.e. 5556, etc)"
}
-- *config*
-- [no] dce qcn ethertype %ethertype dev %devID
CLI_addCommand("config", "dce qcn ethertype", {
    func=set_ethertype_dce_qcn,
    help="dce qcn ethertype (for example :5556, in two bytes,hex) dev DEV_ID",
    params={
        {   type="values",
            "%ethertype",
            "dev",
            "%devID"
        }
    }
})
