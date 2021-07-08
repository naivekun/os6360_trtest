--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_types_ap.lua
--*
--* DESCRIPTION:
--*       The file defines types needed for AP attributes commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


--------------------------------------------
-- type registration: ap port fc direction enablers
--------------------------------------------
CLI_type_dict["ap_flow_control"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "enumerator of ap port fc direction enablers",
    enum = {
        ["symmetric"] =    { value="CPSS_PX_PORT_AP_FLOW_CONTROL_SYMMETRIC_E", help="flow control in both directions" },
        ["asymmetric"] =   { value="CPSS_PX_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E", help="flow control in one directions" }
    }
}

--------------------------------------------
-- type registration: ap_port_interface_mode
--------------------------------------------
CLI_type_dict["ap_port_interface_mode"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "Ap Port interface mode",
    enum = {
        ["XGMII"]       = { value="CPSS_PORT_INTERFACE_MODE_XGMII_E",
                                        help="XGMII interface mode"             },
        ["1000Base_X"]  = { value="CPSS_PORT_INTERFACE_MODE_1000BASE_X_E",
                                        help="1000 Base X interface mode"       },
        ["KR"]          = { value="CPSS_PORT_INTERFACE_MODE_KR_E",
                                        help="KR interface mode"                },
        ["KR_C"]        = { value="CPSS_PORT_INTERFACE_MODE_KR_C_E",
                                        help="KR consortium interface mode"     },
        ["KR2_C"]       = { value="CPSS_PORT_INTERFACE_MODE_KR2_C_E",
                                        help="KR2 consortium interface mode"    },
        ["CR"]          = { value="CPSS_PORT_INTERFACE_MODE_CR_E",
                                        help="CR interface mode"                },
        ["CR4"]         = { value="CPSS_PORT_INTERFACE_MODE_CR4_E",
                                        help="CR4 interface mode"               },
        ["KR_S"]        = { value="CPSS_PORT_INTERFACE_MODE_KR_S_E",
                                        help="KR_S interface mode"              },
        ["CR_S"]        = { value="CPSS_PORT_INTERFACE_MODE_CR_S_E",
                                        help="CR_S interface mode"              },
        ["KR4"]         = { value="CPSS_PORT_INTERFACE_MODE_KR4_E",
                                        help="KR4 (4 lanes) interface mode"     }

    }
}

--------------------------------------------
-- type registration: ap_port_speed
--------------------------------------------
CLI_type_dict["ap_port_speed"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "Ap Port speed",
    enum = {
        ["1000"] =  { value="CPSS_PORT_SPEED_1000_E",
                                        help="Force operation at 1Gbps"       },
        ["10000"] = { value="CPSS_PORT_SPEED_10000_E",
                                        help="Force operation at 10Gbps"      },
        ["25000"] = { value="CPSS_PORT_SPEED_25000_E",
                                        help="Force operation at 25Gbps"      },
        ["40000"] = { value="CPSS_PORT_SPEED_40000_E",
                                        help="Force operation at 40Gbps"      },
        ["50000"] = { value="CPSS_PORT_SPEED_50000_E",
                                        help="Force operation at 50Gbps"      },
        ["100000"] ={ value="CPSS_PORT_SPEED_100G_E",
                                        help="Force operation at 100Gbps"     }
    }
}


