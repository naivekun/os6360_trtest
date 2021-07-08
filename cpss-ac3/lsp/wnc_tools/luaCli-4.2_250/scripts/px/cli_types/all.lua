--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* all.lua
--*
--* DESCRIPTION:
--*       Including of all types
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************

-- includes
require("px/cli_types/px_types_tail_drop")
require("px/cli_types/px_types_qos")
require("px/cli_types/px_types_cos")
require("common/cli_types/port_enums")
require("px/cli_types/px_lane_number_type")
require("px/cli_types/px_loopback_serdes_type")
require("px/cli_types/px_types_ap")
require("px/cli_types/px_types_dce")


-------------------------------------------------------
-- type registration: portNum
-------------------------------------------------------
CLI_type_dict["portNum"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 16,
    help     = "Port number"
}

-------------------------------------------------------
-- type registration: devNum
-------------------------------------------------------
CLI_type_dict["devNum"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=0,
    help="The device ID"
}

--------------------------------------------
-- CPSS integer type registration
--------------------------------------------
do
  local inttypes = {"CPSS_PX_PORTS_BMP", "CPSS_PX_PACKET_TYPE"}
  local i, n
  for i, n in pairs(inttypes) do
    _G["mgmType_to_c_"..n] = mgmType_to_c_int
    _G["mgmType_to_lua_"..n] = mgmType_to_lua_int
  end
end

