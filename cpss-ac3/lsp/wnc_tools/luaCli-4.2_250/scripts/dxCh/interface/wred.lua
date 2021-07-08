--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* policyIf.lua
--*
--* DESCRIPTION:
--*       policy rules related commands for vlan and ports
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
--  setWRED
--
--  @description Set tail drop profiles limits for particular TC. 
--
--  @param params - params["queue"]       - Traffic Class associated with this set of Drop Parameters.
--          params["dp"]        - Drop Profile PArameters to associate with the Traffic Class.
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************


local function setWRED(params)
  local iterator, devNum, portNum, ret, val, trafficClass
  local enablers, queueID, startID, endID, startDP, endDP, profileIndex
  local command_data = Command_Data()
    
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
  local dp = params["dp"]
  if dp == "all" then
    startDP = 0
    endDP = 2
  else
    startDP = dp
    endDP = dp
  end
  
  enablers = {}
  
  if (params.flagNo == nil) then -- enable or disable
    enablers["tcDpLimit"] = true
  else
    enablers["tcDpLimit"] = false
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
        print("Could not set WRED for port " .. portNum)
        return false
      end    
      
      profileIndex = val["profileSetPtr"]
      
      -- loop over the queue ID's
      for trafficClass=startID, endID do 

        -- loop over the drop precedence (packet's color)
        for index=startDP, endDP do 

        ret,val = myGenWrapper("cpssDxChPortTxProfileWeightedRandomTailDropEnableSet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","pfSet", profileIndex},
          {"IN","GT_U32","dp", index},
          {"IN","GT_U8","tc", trafficClass},
          {"IN","CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC","enablersPtr", enablers}
        })
        
        if ret ~= 0 then
          print("Could not set WRED for port " .. portNum)
          return false
        end            

        end    
      end
    end
  end
  
    return true
end

--------------------------------------------
-- command registration: wred
--------------------------------------------

CLI_addCommand("interface", "wred", {
  func=setWRED,
  help="Enables WRED for TC/DP limits",
   params={
    { type="named",
      { format="queue %queue_id", name="queue", help="Traffic Class associated with this set of Drop Parameters" },
      { format="dp %drop_precedence", name="dp", help="Drop Profile Parameters to associate with the Traffic Class" },
      requirements={
        ["dp"] = {"queue"}
      },
          mandatory = {"dp"}
  }
  }
})

--------------------------------------------
-- command registration: no wred
--------------------------------------------

CLI_addCommand("interface", "no wred", {
  func=function(params)
      params["flagNo"] = true
      return setWRED(params)
    end,
  help="Enables WRED for TC/DP limits",
   params={
    { type="named",
      { format="queue %queue_id", name="queue", help="Traffic Class associated with this set of Drop Parameters" },
      { format="dp %drop_precedence", name="dp", help="Drop Profile Parameters to associate with the Traffic Class" },
      requirements={
        ["dp"] = {"queue"}
      },
          mandatory = {"dp"}
  }
  }
})

