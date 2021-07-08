--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_running_config.lua
--*
--* DESCRIPTION:
--*       showing of the running config
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_running_config
--        @description  shows only the commands used to configure, the
--                      filter can be changed to filter out commands
--                      contained in it
--
--        @return       true
--
local function show_running_config()
  local i, v = next(CLI_running_Cfg_Tbl,nil)
--  local filters = {"running%-config","exit","debug","configure","traffic","show","end"}  --commands containing these words will be filtered in the show running config command
  local filters = {"running%-config"}  --commands containing these words will be filtered in the show running config command
  local filtered,j

  while i do
    filtered=false
        for j = 1, #filters do
      if string.match(v, filters[j]) ~= nil then
        filtered=true
        break
      end
    end
    if (filtered==false) then   print(v) end
    i,v=next(CLI_running_Cfg_Tbl,i)
  end

  return true
end

CLI_addCommand("exec", "show running-config", {
    func=show_running_config,
    help="Current operating configuration"
})
