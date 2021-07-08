--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ip_arp_inspection.lua
--*
--* DESCRIPTION:
--*       showing of the arp inspection status
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_ip_arp_inspection
--        @description  Displays the status of arp inspection
--
--        @param params         - The parameters
--
--        @return       true on success, otherwise false and error message
--
local function show_ip_arp_inspection(params)
  local status, err
  local result, values
  local mode
  
  status = true
      
    result, values = myGenWrapper(
      "cpssDxChNstProtSanityCheckGet", {
        { "IN", "GT_U8" , "dev", params["devID"]}, -- devNum
        { "IN", TYPE["ENUM"], "type", 9},  --CPSS_NST_CHECK_ARP_MAC_SA_MISMATCH_E
        { "OUT", "GT_BOOL", "status"}
      }
    )
  if (result ~= 0) then
    status = false
    err = returnCodes[result]
  else 
    if (values.status == true) then
      print("IP ARP inspection is globally enabled on device " .. tostring(params["devID"]))
    else
      print("IP ARP inspection is globally disabled on device " .. tostring(params["devID"])) 
    end
  end
  
  return status, err
end
 
-- *exec*
-- show ip arp inspection %devID
CLI_addHelp("exec", "show ip", "IP information")
CLI_addCommand("exec", "show ip arp inspection", {
    func=show_ip_arp_inspection,
    help="Show ARP inspection  status",
    params={ { type="values", "%devID" } }
})
