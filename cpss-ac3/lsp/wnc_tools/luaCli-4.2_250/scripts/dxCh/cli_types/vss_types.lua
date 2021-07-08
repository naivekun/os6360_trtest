--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vss_types.lua
--*
--* DESCRIPTION:
--*     'config' commands for the 'vss of bc2-ac3' system
--*     according to doc:
--*     http://docil.marvell.com/webtop/drl/objectId/0900dd88801a06b3 
--*       cc – Control Card device --  Bc2
--*       lc – line-card device - Ac3
--*	    Native traffic received on AC3 LC are sent with 8B Extended DSA to BC2
--*	    BC2 transmits 16B eDSA packets to AC3 line cards
--*	    Cross platform VSS traffic
--*	    AC3 passes eDSA packets transparently over the VSS interface to remote platform
--*	    AC3 passes eDSA packets received over VSS to BC2
--*	    BC2 device learns MAC address on remote platform based on eDSA <Src ePort>
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--********************************************
--********************************************
local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(devNum)
if "CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily then
    vss_global_eport_range_start_eport = 900
    vss_global_eport_range_end_eport   = 999
else
    vss_global_eport_range_start_eport = 0x1100 --dec 4352
    vss_global_eport_range_end_eport   = 0x11ff --dec 4607
end
description___vss_global_eport_range = "the VSS global eport valid range [" .. vss_global_eport_range_start_eport .. ".." .. vss_global_eport_range_end_eport .. "]"

CLI_type_dict["type___vss_global_eport"] = {
    checker = CLI_check_param_number,
    min=vss_global_eport_range_start_eport,
    max=vss_global_eport_range_end_eport,
    complete = CLI_complete_param_number,
    help=description___vss_global_eport_range
}

--********************************************
--********************************************
local vss_role_cc = "cc"
local vss_role_lc = "lc"
vss_role_cc_full_name = "Control Card device"
vss_role_lc_full_name = "line card device"
description___vss_role = "The role (cc - Control Card device , lc - line card device)"
description___neighbor_vss_name = "The unique name of the neighbor vss"
 
CLI_type_dict["type___vss_role"] =
{
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = vss_role_description,
    enum =
    {
        [vss_role_cc] = { value = vss_role_cc_full_name , help = vss_role_cc_full_name },
        [vss_role_lc] = { value = vss_role_lc_full_name , help = vss_role_lc_full_name  }
    }
}

--********************************************
--********************************************
local vss_id_min = 1
local vss_id_max = 31 -- 5 bits (limited by EPCL rule that need to hold this value on the xcat3 !!!)
description___vss_id = "identify this 'vss' in multi-vss system. valid range [" .. vss_id_min .. ".." .. vss_id_max .. "]"
CLI_type_dict["type___vss_id"] = {
    checker = CLI_check_param_number,
    min=vss_id_min,
    max=vss_id_max,
    complete = CLI_complete_param_number,
    help = description___vss_id
}

--********************************************
--********************************************
-- pcl rules on lc (xcat3)
vss_on_lc_pcl_rule_size_STD = 1 -- the PCL rules on the lc (xcat3) are according to 'STD' rule size
vss_on_lc_pcl_rule_size_STD_name = "CPSS_PCL_RULE_SIZE_STD_E"
vss_on_lc_tti_rule_size     = 1 -- the TTI rules on the lc (xcat3) are with single size .
vss_pcl_id_mask_10_bits     = 0x3ff -- mask for pcl-id --> 10 bits
vss_eport_mask_17_bits      = 0x1ffff -- mask for 'eport' on dsa --> 17 bits
vss_on_lc_src_dev_mask_5_bits    = 0x1f -- mask for srcDev --> 5 bits (for lc device)
vss_on_lc_designated_table_size = 8 -- 8 entries in the 'designated table'
cpss_enum_postfix   = "_E"
cpss_STC_postfix = "_STC"

cpss_pcl_egr_std_not_ip_name_prefix     = "CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP"
cpss_pcl_egr_std_ip_l2_qos_name_prefix  = "CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS"

vss_on_lc_pcl_rule_forma_ing_std_udb_name       = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"--> 'STD' in enum replaced by 'STANDARD' in STC !!!!
vss_on_lc_pcl_rule_forma_egr_std_not_ip_name    = cpss_pcl_egr_std_not_ip_name_prefix       .. cpss_enum_postfix
vss_on_lc_pcl_rule_forma_egr_std_ip_l2_qos_name = cpss_pcl_egr_std_ip_l2_qos_name_prefix    .. cpss_enum_postfix
vss_on_lc_pcl_rule_STC_ing_std_udb_name         = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC" --> 'STD' in enum replaced by 'STANDARD' in STC !!!!
vss_on_lc_pcl_rule_STC_egr_std_not_ip_name      = cpss_pcl_egr_std_not_ip_name_prefix       .. cpss_STC_postfix
vss_on_lc_pcl_rule_STC_egr_std_ip_l2_qos_name   = cpss_pcl_egr_std_ip_l2_qos_name_prefix    .. cpss_STC_postfix

--********************************************
--********************************************
vss_on_lc_min_port_num = 0
vss_on_lc_max_port_num = 27
description___vss_on_lc_port_range = "the lc ports valid range [" .. vss_on_lc_min_port_num .. ".." .. vss_on_lc_max_port_num .. "]"
CLI_type_dict["type___vss_lc_port_range"] = {
    checker = CLI_check_param_number,
    min=vss_on_lc_min_port_num,
    max=vss_on_lc_max_port_num,
    complete = CLI_complete_param_number,
    help=description___vss_on_lc_port_range
}
--********************************************
--********************************************

local vss_add_port = "add"
local vss_remove_port = "remove"
vss_add_port_full_name = "add port"
vss_remove_port_full_name = "remove port"
description___vss_add_remove_port = "add/remove port"

 
CLI_type_dict["type___vss_add_remove_port"] =
{
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = description___vss_add_remove_port,
    enum =
    {
        [vss_add_port]    = { value = vss_add_port_full_name    , help = vss_add_port_full_name },
        [vss_remove_port] = { value = vss_remove_port_full_name , help = vss_remove_port_full_name  }
    }
}
