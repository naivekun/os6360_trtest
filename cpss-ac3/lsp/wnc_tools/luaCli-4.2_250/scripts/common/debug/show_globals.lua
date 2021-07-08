--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_globals.lua
--*
--* DESCRIPTION:
--*       printing all LUA_CLI globals
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- show LUA_CLI globals

-- ************************************************************************
--  printLuaGlobals
--        @description  Show registered Lua globals
--
--        @return       true
--

local function printLuaGlobals(params)
  local k, n, v
  k=1
  for n,v in pairs(_G) do
    if type(v) == "function" then
      printMsg ("#", k, "<function>", n)
    elseif type(v) == "table" then
      printMsg ("#", k, "<table>   ", n)
    else
      printMsg ("#", k, "<variable>", n, "+"..type(v).."+")
    end
    k=k+1
  end
  return true
end

-------------------------------------------------------
-- command registration: showLuaGlobals
-------------------------------------------------------
local showLuaGlobals = {
    func=printLuaGlobals,
    help="printLuaGlobals",
    params={}
}

CLI_addCommand("debug", "showLuaGlobals", showLuaGlobals)
