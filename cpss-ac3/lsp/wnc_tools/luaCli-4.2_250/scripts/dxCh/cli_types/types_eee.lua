--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* types_eee.lua
--*
--* DESCRIPTION:
--*       define types for EEE.
--*       Port's IEEE 802.3az Energy Efficient Ethernet (EEE) Support.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--********************************************************************************
--      types
--********************************************************************************

-- the interfaces that the EEE supports (only ethernet)
--[[local]] eee_supported_interfaces = {ethernet = true}
 
eee_help_time_units = "(units of 1 uSec for 1000 Mbps and 10 uSec for 100 Mbps)"

eee_help_eee_mode = "Enable the EEE in auto mode or manual mode or manual mode with force transmitting of LPI signalling.\n" ..
                    "Or disable the EEE."
                    
eee_help_eee_phy_mode = "(On the PHY) Enable the EEE as Master/Slave/Force mode.\n" ..
                    "Or disable the EEE."

eee_disable     = "disable"
eee_auto        = "auto"
eee_manual      = "manual"
eee_force       = "force"
--values from MACPHY_EEE_LPI_MODE_ENT defined in ..\cpss\cpssEnabler\mainExtMac\h\extPhyM\phyMacMSec.h
eee_phy_disable = 0--"MACPHY_EEE_LPI_DISABLE_MODE_E"
eee_phy_master  = 1--"MACPHY_EEE_LPI_MASTER_MODE_E"
eee_phy_slave   = 2--"MACPHY_EEE_LPI_SLAVE_MODE_E"
eee_phy_force   = 3--"MACPHY_EEE_LPI_FORCE_MODE_E"
eee_phy_NOT_VALID = "not valid"
                    
CLI_type_dict["eee_mode_enum"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = eee_help_eee_mode,
    enum = {
        ["disable"]  = { value=eee_disable  , help="Disable EEE." },
        ["auto"]     = { value=eee_auto     , help="Set EEE into auto mode." },
        ["manual"]   = { value=eee_manual   , help="Set EEE into manual-mode." },
        ["force"]    = { value=eee_force    , help="Set EEE into manual-mode with force transmitting of LPI signalling." }
   }
}  

eee_help_tw_limit = "(optional)The minimum time from LPI de-assertion until valid data can be sent." .. eee_help_time_units
eee_help_li_limit = "(optional) The minimum time since going into LPI and allowing LPI de-assert."   .. eee_help_time_units
eee_help_ts_limit = "(optional) The minimum time from emptying of Tx FIFO and LPI assert."           .. eee_help_time_units

CLI_type_dict["eee_tw_limit"] = {
    checker = CLI_check_param_number,
    min=0,
    max=4095,
    complete = CLI_complete_param_number,
    help = eee_help_tw_limit
}
 
CLI_type_dict["eee_li_limit"] = {
    checker = CLI_check_param_number,
    min=0,
    max=254,
    complete = CLI_complete_param_number,
    help = eee_help_li_limit
}

CLI_type_dict["eee_ts_limit"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    complete = CLI_complete_param_number,
    help = eee_help_ts_limit
}

CLI_type_dict["eee_phy_mode_enum"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = eee_help_eee_phy_mode,
    enum = {
        ["disable"]  = { value= eee_phy_disable  , help = "Disable EEE on the PHY." },
        ["master"]   = { value= eee_phy_master   , help = "Set PHY EEE into Master mode." },
        ["slave"]    = { value= eee_phy_slave    , help = "Set PHY EEE into Slave mode." },
        ["force"]    = { value= eee_phy_force    , help = "Set PHY EEE into Force mode transmitting of LPI signalling to both directions." }
   }
}  
 

 
 
 
 
 