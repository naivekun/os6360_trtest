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
--  @description Changes WRED mask for tcDp limits
--
--  @param params - params["queue"]       - Traffic Class associated with this set of Drop Parameters.
--          params["dp"]        - Drop Profile PArameters to associate with the Traffic Class.
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************


local function setWRED_mask(params)
  local ret, val
  local maskLsb
  local devNum = tonumber(params["devID"])
  ret,val = myGenWrapper("cpssDxChPortTxTailDropWrtdMasksGet",{ 
    {"IN","GT_U8","devNum",devNum},
    {"OUT","CPSS_PORT_TX_WRTD_MASK_LSB_STC","maskLsbPtr"}
  })
  
  if ret ~= 0 or val == nil then
    print("Could not set wred-mask")
    return false
  end    
  
  maskLsb = val["maskLsbPtr"]
  
  maskLsb["tcDp"] = tonumber(params["tc_dp_mask"]) -- change tcDp according to user's parameter
  
  ret,val = myGenWrapper("cpssDxChPortTxTailDropWrtdMasksSet",{ 
    {"IN","GT_U8","devNum",devNum},
    {"IN","CPSS_PORT_TX_WRTD_MASK_LSB_STC","maskLsbPtr", maskLsb}
  })
  
  if ret ~= 0 then
    print("Could not set wred-mask")
    return false
  end  
  
    return true
end

--------------------------------------------
-- command registration: wred-mask
--------------------------------------------

CLI_addCommand("config", "wred-mask", {
  func=setWRED_mask,
  help="Change WRED mask for tcDp limits",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" }, 
      { format="tc-dp-mask %tc_dp_mask", name="tc_dp_mask", help="tcDp mask value" },
      requirements={
        ["tc_dp_mask"] = {"devID"}
      },
          mandatory = {"tc_dp_mask"}
  }
  }
})

