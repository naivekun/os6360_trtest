--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vsi.lua
--*
--* DESCRIPTION:
--*       vsi types
--*
--* FILE REVISION NUMBER:
--*       $Revision: 8 $
--*
--********************************************************************************



CLI_type_dict["vsi-id"] = {
    checker = CLI_check_param_number,
    min=0,
    max=16777215,
    complete = CLI_complete_param_number,
    help="vsi number (24 bit)"
}

CLI_type_dict["vsi_tagged_untagged"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Tagged or Untagged",
    enum = {
        ["tagged"] = { value="CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E", help="Tagged" },
        ["untagged"] =  { value="CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E", help="Untagged" }
   }
}

CLI_type_dict["pw_tag_mode_raw_mode_tag_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "raw-mode or tag-mode",
    enum = {
        ["raw-mode"] = { value="CPSS_DXCH_TTI_PW_TAG_RAW_PW_MODE_E", help="raw-mode" },
        ["tag-mode"] = { value="CPSS_DXCH_TTI_PW_TAG_TAGGED_PW_MODE_E", help="tag-mode" }
   }
}

