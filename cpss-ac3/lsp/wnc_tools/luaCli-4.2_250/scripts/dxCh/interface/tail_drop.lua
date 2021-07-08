--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tail_drop.lua
--*
--* DESCRIPTION:
--*       Tail-drop per-interface commands
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
--  SetTailDrop
--
--  @description configures tail drop limits on Port level 
--
--  @param params - params["packet-limit"]    - maximal number of descriptions for a port.
--          params["buffer-limit"]    - maximal number of buffers for a port.
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function SetTailDrop(params)
  local iterator, devNum, portNum, ret, val
  local profileIndex
  local command_data = Command_Data()
  local alpha, maxBuff, maxDesc
    
    -- Common variables initialization.    
    command_data:initInterfaceRangeIterator()   
    command_data:initInterfaceDeviceRange()

   
  -- Main interface handling cycle
  if true == command_data["status"] then
    for iterator, devNum, portNum in command_data:getPortIterator() do
      
      -- get tail drop profile index for the port
      ret,val = myGenWrapper("cpssDxChPortTxBindPortToDpGet",{ 
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"OUT","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSetPtr"}
      })
      
      if ret ~= 0 or val == nil then
        print("Could not set tail-drop for port " .. portNum)
        return false
      end    
      
      profileIndex = val["profileSetPtr"]
      
      -- read profile
      ret,val = myGenWrapper("cpssDxChPortTxTailDropProfileGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSet", profileIndex},
        {"OUT","CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT","portAlphaPtr"},
        {"OUT","GT_U32","portMaxBuffLimitPtr"},
        {"OUT","GT_U32","portMaxDescrLimitPtr"}
      })
      
      if ret ~= 0 or val == nil then
        print("Could not set tail-drop for port " .. portNum)
        return false
      end    

      if (params["packet-limit"] ~= nil) then -- get packet-limit if entered, else, not changing.
        maxDesc = tonumber(params["packet-limit"])
      else
        maxDesc = val["portMaxDescrLimitPtr"]
      end  

      if (params["buffer-limit"] ~= nil) then -- get buffer-limit if entered, else, not changing.
        maxBuff = tonumber(params["buffer-limit"])
      else
        maxBuff = val["portMaxBuffLimitPtr"]
      end

      if (params["alpha"] ~= nil) then -- get port alpha if entered, else, not changing.
        alpha = params["alpha"]
      else
        alpha = val["portAlphaPtr"]
      end

      -- write profile
      ret,val = myGenWrapper("cpssDxChPortTxTailDropProfileSet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSet", profileIndex},
        {"IN","CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT","portAlpha", alpha},
        {"IN","GT_U32","portMaxBuffLimit", maxBuff},
        {"IN","GT_U32","portMaxDescrLimit", maxDesc}
      })
      
      if ret ~= 0 then
        print("Could not set tail-drop for port " .. portNum)
        return false
      end        
      
    end
  end
  
    return true
end

--------------------------------------------
-- command registration: tail-drop
--------------------------------------------

CLI_addCommand("interface", "tail-drop", {
  func=SetTailDrop,
  help="Configures tail drop limits on Port",
   params={
    { type="named",
      { format="packet-limit %packet-limit", name="packet-limit", help="Maximal number of descriptors for a port" },
      { format="buffer-limit %packet-limit", name="buffer-limit", help="Maximal number of buffers for a port" },
      { format="alpha %tail_drop_alpha",   name="alpha"  , help="Ratio of the free buffers used for the port thresholds" }
    }
  }
})

-- ************************************************************************
--  SetTailDropQueue
--
--  @description Set tail drop profiles limits for particular TC. 
--
--  @param params - params["queue"]       - Traffic Class associated with this set of Drop Parameters.
--          params["dp"]        - Drop Profile PArameters to associate with the Traffic Class.
--          params["packet-limit"]    - maximal number of descriptions for a port.
--          params["buffer-limit"]    - maximal number of buffers for a port.
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function SetTailDropQueue(params)
  local iterator, devNum, portNum, ret, val, trafficClass
  local profileParams, queueID, startID, endID, dp, startDP, endDP
  local currDescr, currBuff, profileIndex
  local command_data = Command_Data()
  local enableSharing, maxBuff, maxDesc
    
    -- Common variables initialization.    
    command_data:initInterfaceRangeIterator()   
    command_data:initInterfaceDeviceRange()
  
  -- set the loop range for the queue ID
  queueID = params["queue"]
  if queueID == "all" then
    startID = 0
    endID = 7
  else
    startID = queueID
    endID = queueID
  end
  
  -- set the loop range for the drop precedence
  dp = params["dp"]
  if dp == "all" then
    startDP = 0
    endDP = 2
  else
    startDP = dp
    endDP = dp
  end
   
  -- Main interface handling cycle
  if true == command_data["status"] then
    for iterator, devNum, portNum in command_data:getPortIterator() do
      
      -- get tail drop profile index for the port
      ret,val = myGenWrapper("cpssDxChPortTxBindPortToDpGet",{ 
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"OUT","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSetPtr"}
      })
      
      if ret ~= 0 or val == nil then
        print("Could not set tail-drop-queue for port " .. portNum)
        return false
      end    
      
      profileIndex = val["profileSetPtr"]
      
      -- loop over the queue ID's
      for trafficClass=startID, endID do 
        -- read profile
        ret,val = myGenWrapper("cpssDxChPortTx4TcTailDropProfileGet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSet", profileIndex},
          {"IN","GT_U8","trafficClass", trafficClass},
          {"OUT","CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS","tailDropProfileParamsPtr"}
        })
        
        if ret ~= 0 or val == nil then
          print("Could not set tail-drop-queue for port " .. portNum)
          return false
        end    
        
        profileParams = val["tailDropProfileParamsPtr"] 
        
        -- loop over the packet's color
        for index=startDP, endDP do 
          
          currDescr = "dp" .. index .. "MaxDescrNum" -- get the name of the descriptor parameter
          
          if (params["packet-limit"] ~= nil) then
            profileParams[currDescr] = tonumber(params["packet-limit"])
          end  

          currBuff = "dp" .. index .. "MaxBuffNum" -- get the name of the buffer parameter

          if (params["buffer-limit"] ~= nil) then
            profileParams[currBuff] = tonumber(params["buffer-limit"])
          end

          currBuff = "dp" .. index .. "QueueAlpha" -- get the name of the alpha parameter

          if (params["alpha"] ~= nil) then
            profileParams[currBuff] = params["alpha"]
          end
        end  

        -- write profile
        ret,val = myGenWrapper("cpssDxChPortTx4TcTailDropProfileSet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSet", profileIndex},
          {"IN","GT_U8","trafficClass", trafficClass},
          {"IN","CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS","tailDropProfileParamsPtr", profileParams}
        })
        
        if ret ~= 0 then
          print("Could not set tail-drop-queue for port " .. portNum)
          return false
        end            
      end
    end
  end
  
    return true
end

--------------------------------------------
-- command registration: tail-drop-queue
--------------------------------------------

CLI_addCommand("interface", "tail-drop-queue", {
  func=SetTailDropQueue,
  help="Configures tail drop limits on Port",
   params={
    { type="named",
      { format="queue %queue_id", name="queue", help="Traffic Class associated with this set of Drop Parameters" },
      { format="dp %drop_precedence", name="dp", help="Drop Profile Parameters to associate with the Traffic Class" },
      { format="packet-limit %packet-limit", name="packet-limit", help="Maximal number of descriptors for a port" },
      { format="buffer-limit %packet-limit", name="buffer-limit", help="Maximal number of buffers for a port" },
      { format="alpha %tail_drop_alpha",   name="alpha"  , help="Ratio of the free buffers used for the port thresholds" },
      requirements={
        ["dp"] = {"queue"},
    ["packet-limit"] = {"dp"},
    ["buffer-limit"] = {"dp"},
    ["alpha"] = {"dp"}
      },
          mandatory = {"dp"}
  }
  }
})

