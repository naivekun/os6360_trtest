--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* all.lua
--*
--* DESCRIPTION:
--*       types for the phy/smi commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

CLI_type_dict["phyMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "The phy mode\n",
    enum = {
        ["SMI"] = { value="SMI", help="SMI mode" },
        ["XSMI"] = { value="XSMI", help="XSMI mode" }
   }
}

function smiInterfacesRegister()
  if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_BOBCAT2_E"] == nil then
    -- add the suitable smiInterfaces for all devices except bobCat2
    CLI_type_dict["smiInterface"] = {
      checker = CLI_check_param_enum,
      complete = CLI_complete_param_enum,
      help = "The SMI interface",
      enum = {
          ["0"] = { value=0, help="SMI interface 0" },
          ["1"] = { value=1, help="SMI interface 1" }
      }
    }
  else
    -- add the suitable smiInterfaces for all devices except bobCat2
    CLI_type_dict["smiInterface"] = {
      checker = CLI_check_param_enum,
      complete = CLI_complete_param_enum,
      help = "The SMI interface",
      enum = {
          ["0"] = { value=0, help="SMI interface 0" },
          ["1"] = { value=1, help="SMI interface 1" },
          ["2"] = { value=2, help="SMI interface 2" },
          ["3"] = { value=3, help="SMI interface 3" }
      }
    }
  end
end

-- insert smiInterfacesRegister() callback to global list
table.insert(typeCallBacksList, smiInterfacesRegister)
