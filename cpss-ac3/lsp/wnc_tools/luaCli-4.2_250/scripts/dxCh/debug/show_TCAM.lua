--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_TCAM.lua
--*
--* DESCRIPTION:
--*       shows the unparsed TCAM memory
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
--  dumpTCAM
--
--  @description prints all the valid TCAM rules in a given range
--
--  @param params - params["devID"] - the device number
--					params["from"] - the TCAM entry to start from
--					params["to"] - last entry to display
--

--	@return true on success (and prints to screen), otherwise false and error message
--
-- ************************************************************************

local function dumpTCAM(params)
  local ret,devices,j,len,i
  local val
  len=params["to"] - params["from"]
  if (len>=0) then
    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end
    for j=1,#devices do
      for i=params["from"],params["to"] do
        ret,val = myGenWrapper("cpssDxChPclPortGroupRuleAnyStateGet",{
          {"IN","GT_U8","devNum",devices[j]},
          {"IN","GT_U32","portGroupsBmp",0xFFFFFFFF},
          {"IN",TYPE["ENUM"],"ruleSize",1}, --ext rule
          {"IN","GT_U32","ruleIndex",i},   -- index of the rule
          {"OUT","GT_BOOL","validPtr"},   -- index of the rule
          {"OUT",TYPE["ENUM"],"ruleSizePtr"}   -- index of the rule
        })

        if (ret==0) and (val["validPtr"]==true) then
          print("Device "..devices[j]..":")

          ret = myGenWrapper("utilCpssDxChPclRulesDump",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN","GT_U32","portGroup",0},
            {"IN",TYPE["ENUM"],"ruleSize",1}, --ext rule
            {"IN","GT_U32","startIndex",params["from"]},   -- index of the rule
            {"IN","GT_U32","rulesAmount",len}
          })

          if (ret~=0) then return false,"Could not print TCAM memory" end
          break
        end
      end
    end
  end
  return true
end


--------------------------------------------
-- command registration: show TCAM
--------------------------------------------
CLI_addHelp("debug", "show PCL", "Display PCL subcommands")
CLI_addCommand("debug", "show PCL TCAM", {
  func=dumpTCAM,
  help="Show used TCAM memory",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="from %ruleID", name="from", help="The rule start index" },
    { format="to %ruleID", name="to", help="The rule end index" },
    requirements = {["from"]={"devID"},["to"]={"from"}},
    mandatory = {"to"}
    }
  }
})
