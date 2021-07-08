--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* qos_default_up.lua
--*
--* DESCRIPTION:
--*       changing of the port's default UP
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  qos_default_up
--        @description  Changes the default up (user priority) of the port 
--
--        @param params         - params["vlan-range"]
--
--        @return       true if there was no error otherwise false
--
local function qos_default_up(params)
  local all_ports, dev_ports
  local dev, k, port, status, err, pvid
  local result, values
  
    
    --is it working on the right interface type, (not vlan or anything)
  if (tostring(getGlobal("ifType")) ~= "ethernet") then
    return false, "Can only be used on an ethernet type interface"
  end
    all_ports = getGlobal("ifRange")
    status = true
  
  for dev, dev_ports in pairs(all_ports) do
    for k, port in pairs(dev_ports) do
        result, values = cpssPerPortParamSet(
          "cpssDxChPortDefaultUPSet",
                    dev, port, params["default_up"], "up", "GT_U8")
      if (result ~= 0) then
        status = false
        err = returnCodes[result]
      end    
    end
  end
    
  return status, err
end
 
CLI_type_dict["default_up"] = {
    checker = CLI_check_param_number,
    min=0,
    max=7,
    help = "The new default UP (0-7)"
}
-- *interface*
-- qos default-up %default_up
CLI_addHelp({"interface", "interface_eport"}, "qos", "Configure QoS")
CLI_addCommand({"interface", "interface_eport"}, "qos default-up", {
    func = qos_default_up,
    help = "Change port's default UP",
    params = {{type="values", "%default_up" }}
})
