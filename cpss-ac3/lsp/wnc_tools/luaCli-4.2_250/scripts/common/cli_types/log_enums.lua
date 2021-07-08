--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* log_enums.lua
--*
--* DESCRIPTION:
--*       log emuns types
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--------------------------------------------
-- type registration: log_lib
--------------------------------------------
CLI_type_dict["log_lib"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "Log libs",
    enum = {
        ["app-driver-call"] = {
            value = "CPSS_LOG_LIB_APP_DRIVER_CALL_E",
            help  = "driver call"
        },
        ["bridge"] = {
            value = "CPSS_LOG_LIB_BRIDGE_E",
            help  = "bridge"
        },
        ["cnc"] = {
            value = "CPSS_LOG_LIB_CNC_E",
            help  = "cnc"
        },
        ["config"] = {
            value = "CPSS_LOG_LIB_CONFIG_E",
            help  = "config"
        },
        ["cos"] = {
            value = "CPSS_LOG_LIB_COS_E",
            help  = "cos"
        },
        ["hw-init"] = {
            value = "CPSS_LOG_LIB_HW_INIT_E",
            help  = "hwinit"
        },
        ["cscd"] = {
            value = "CPSS_LOG_LIB_CSCD_E",
            help = "cscd"
        },
        ["cut-through"] = {
            value = "CPSS_LOG_LIB_CUT_THROUGH_E",
            help  = "cut through"
        },
        ["diag"] = {
            value = "CPSS_LOG_LIB_DIAG_E",
            help  = "diag"
        },
        ["fabric"] = {
            value = "CPSS_LOG_LIB_FABRIC_E",
            help = "fabric"
        },
        ["ip"] = {
            value = "CPSS_LOG_LIB_IP_E",
            help  = "ip"
        },
        ["ipfix"] = {
            value = "CPSS_LOG_LIB_IPFIX_E",
            help  = "ipfix"
        },
        ["ip-lpm"] = {
            value = "CPSS_LOG_LIB_IP_LPM_E",
            help  = "ip lpm"
        },
        ["l2-mll"] = {
            value = "CPSS_LOG_LIB_L2_MLL_E",
            help  = "l2 mll"
        },
        ["logical-target"] = {
            value = "CPSS_LOG_LIB_LOGICAL_TARGET_E",
            help  = "logical target"
        },
        ["lpm"] = {
            value = "CPSS_LOG_LIB_LPM_E",
            help  = "lpm"
        },
        ["mirror"] = {
            value = "CPSS_LOG_LIB_MIRROR_E",
            help  = "mirror"
        },
        ["multi-port-group"] = {
            value = "CPSS_LOG_LIB_MULTI_PORT_GROUP_E",
            help  = "multi port group"
        },
        ["network-if"] = {
            value = "CPSS_LOG_LIB_NETWORK_IF_E",
            help  = "network if"
        },
        ["nst"] = {
            value = "CPSS_LOG_LIB_NST_E",
            help  = "nst"
        },
        ["oam"] = {
            value = "CPSS_LOG_LIB_OAM_E",
            help  = "oam"
        },
        ["pcl"] = {
            value = "CPSS_LOG_LIB_PCL_E",
            help  = "pcl"
        },
        ["phy"] = {
            value = "CPSS_LOG_LIB_PHY_E",
            help  = "phy"
        },
        ["policer"] = {
            value = "CPSS_LOG_LIB_POLICER_E",
            help  = "policer"
        },
        ["port"] = {
            value = "CPSS_LOG_LIB_PORT_E",
            help  = "port"
        },
        ["protection"] = {
            value = "CPSS_LOG_LIB_PROTECTION_E",
            help  = "protection"
        },
        ["ptp"] = {
            value = "CPSS_LOG_LIB_PTP_E",
            help  = "ptp"
        },
        ["system-recovery"] = {
            value = "CPSS_LOG_LIB_SYSTEM_RECOVERY_E",
            help  = "system recovery"
        },
        ["tcam"] = {
            value = "CPSS_LOG_LIB_TCAM_E",
            help  = "tcam"
        },
        ["tm-glue"] = {
            value = "CPSS_LOG_LIB_TM_GLUE_E",
            help  = "tm-glue"
        },
        ["trunk"] = {
            value = "CPSS_LOG_LIB_TRUNK_E",
            help  = "trunk"
        },
        ["tti"] = {
            value = "CPSS_LOG_LIB_TTI_E",
            help  = "tti"
        },
        ["tunnel"] = {
            value = "CPSS_LOG_LIB_TUNNEL_E",
            help  = "tunnel"
        },
        ["vnt"] = {
            value = "CPSS_LOG_LIB_VNT_E",
            help  = "vnt"
        },
        ["resource-manager"] = {
            value = "CPSS_LOG_LIB_RESOURCE_MANAGER_E",
            help  = "resource manager"
        },
        ["version"] = {
            value = "CPSS_LOG_LIB_VERSION_E",
            help  = "version"
        },
        ["tm"] = {
            value = "CPSS_LOG_LIB_TM_E",
            help  = "traffic manager"
        },
        ["smi"] = {
            value = "CPSS_LOG_LIB_SMI_E",
            help  = "smi"
        },
        ["init"] = {
            value = "CPSS_LOG_LIB_INIT_E",
            help  = "init"
        },
        ["dragonite"] = {
            value = "CPSS_LOG_LIB_DRAGONITE_E",
            help  = "dragonite"
        },
        ["virtual-tcam"] = {
            value = "CPSS_LOG_LIB_VIRTUAL_TCAM_E",
            help  = "virtual tcam"
        },
        ["ingress"] = {
            value = "CPSS_LOG_LIB_INGRESS_E",
            help  = "ingress"
        },
        ["egress"] = {
            value = "CPSS_LOG_LIB_EGRESS_E",
            help  = "egress"
        },
        ["all"] = {
            value = "CPSS_LOG_LIB_ALL_E",
            help  = "all the libs"
        }
    }
}


--------------------------------------------
-- type registration: log_format
--------------------------------------------
CLI_type_dict["log_format"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Log formats",
    enum = {
        ["no-params"] = {
            value = "CPSS_LOG_API_FORMAT_NO_PARAMS_E",
            help  = "not documenting any parameter"
        },
        ["all-params"] = {
            value = "CPSS_LOG_API_FORMAT_ALL_PARAMS_E",
            help  = "documenting all the parameters values"
        },
        ["non-zero-params"] = {
            value = "CPSS_LOG_API_FORMAT_NON_ZERO_PARAMS_E",
            help  = "documenting only the non zero parameters values"
        }
    }
}


--------------------------------------------
-- type registration: log_type
--------------------------------------------
CLI_type_dict["log_type"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Log types",
    enum = {
        ["info"] = {
            value = "CPSS_LOG_TYPE_INFO_E",
            help  = "information log"
        },
        ["entry-level-function"] = {
            value = "CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E",
            help = "log of entry level of APIs only"
        },
        ["non-entry-level-function"] = {
            value = "CPSS_LOG_TYPE_NON_ENTRY_LEVEL_FUNCTION_E",
            help  = "log of internal functions and not entry level APIs"
        },
        ["driver"] = {
            value = "CPSS_LOG_TYPE_DRIVER_E",
            help  = "CPSS driver functions log"
        },
        ["error"] = {
            value = "CPSS_LOG_TYPE_ERROR_E",
            help  = "error log"
        },
        ["all"] = {
            value = "CPSS_LOG_TYPE_ALL_E",
            help  = "all the types"
        }
    }
}
