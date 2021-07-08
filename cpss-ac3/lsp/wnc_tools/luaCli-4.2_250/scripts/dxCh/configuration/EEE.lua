--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* EEE.lua
--*
--* DESCRIPTION:
--*       getting of EEE mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  EEEModeSet
--        @description  set's EEE
--
--        @param params         - params["port"]: port-range;
--        @param params         - params["mode"]: EEE-mode
--
local function EEEModeSet(params)
  local dev,i
  local ret
  
  dev = next(params.port)

  for i=1,#params.port[dev] do
    ret = myGenWrapper("phyMacEEEModeSet",{
      {"IN","GT_U8","devNum",dev},
      {"IN","GT_U8","port",params.port[dev][i]},
      {"IN","GT_U32","mode",params.mode}
    })

    if (ret==0x10) then
      print("The feature is not supported on the device")
    elseif (ret~=0) then
      print ("Could not set dev "..dev.." port "..params.port[dev][i])
    end
  end
end


-- ************************************************************************
---
--  EEEModeGet
--        @description  get's EEE
--
--        @param params         - params["port"]: port-range
--
local function EEEModeGet(params)
  local dev,i
  local ret, val
  
  dev = next(params.port)

  for i=1,#params.port[dev] do
    ret,val = myGenWrapper("phyMacEEEModeGet",{
      {"IN","GT_U8","devNum", dev},
      {"IN","GT_U8","port", params.port[dev][i]},
      {"OUT","GT_U32","modePtr"}
    })

    if (ret==0x10) then
      print("The feature is not supported on the device")
    elseif (ret~=0) then
      print("Could not get EEE mode")
    else
      print("Dev:"..dev.."  Port:"..params.port[dev][i].."   Value:"..val["modePtr"])
    end
  end
end

CLI_addHelp("config", "EEE", "Energy Efficient Ethernet related commands")


CLI_type_dict["EEEmode"] = {
    checker = CLI_check_param_number,
    min=0,
    max=3,
    help=":  0-Disable   1-Master   2-Slave   3-Force"
}

CLI_addCommand("config", "EEE set", {
    func=EEEModeSet,
  help="Set EEE mode",
  params={{ type="named",
    { format="port %port-range",name="port", help="The device and port number" },
    {format="mode %EEEmode",name="mode", help="The EEE mode : 0-Disable   1-Master   2-Slave   3-Force" },
    mandatory={"mode"},requirements={mode={"port"}}}
   }
})

CLI_addCommand("config", "EEE get", {
    func=EEEModeGet,
  help="Get EEE mode",
    params={ { type="named",
          { format="port %port-range",name="port", help="The device and port number" }  }}

})


