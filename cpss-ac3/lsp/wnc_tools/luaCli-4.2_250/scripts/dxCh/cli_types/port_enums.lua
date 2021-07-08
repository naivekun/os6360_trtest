--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_enums.lua
--*
--* DESCRIPTION:
--*       port emuna types dec;arations
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes


--constants



--------------------------------------------
-- type registration: port_speed
--------------------------------------------
CLI_type_dict["port_speed"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "Port speed",
    enum = {
        ["10"] =    { value="CPSS_PORT_SPEED_10_E",
                                        help="Force operation at 10Mbps"      },
        ["100"] =   { value="CPSS_PORT_SPEED_100_E",
                                        help="Force operation at 100Mbps"     },
        ["1000"] =  { value="CPSS_PORT_SPEED_1000_E",
                                        help="Force operation at 1Gbps"       },
        ["10000"] = { value="CPSS_PORT_SPEED_10000_E",
                                        help="Force operation at 10Gbps"      },
        ["12000"] = { value="CPSS_PORT_SPEED_12000_E",
                                        help="Force operation at 12Gbps"      },
        ["2500"] =  { value="CPSS_PORT_SPEED_2500_E",
                                        help="Force operation at 2.5Gbps"     },
        ["5000"] =  { value="CPSS_PORT_SPEED_5000_E",
                                        help="Force operation at 5Gbps"       },
        ["13600"] = { value="CPSS_PORT_SPEED_13600_E",
                                        help="Force operation at 13.6Gbps"    },
        ["20000"] = { value="CPSS_PORT_SPEED_20000_E",
                                        help="Force operation at 20Gbps"      },
        ["40000"] = { value="CPSS_PORT_SPEED_40000_E",
                                        help="Force operation at 40Gbps"      },
        ["16000"] = { value="CPSS_PORT_SPEED_16000_E",
                                        help="Force operation at 16Gbps"      },
        ["15000"] = { value="CPSS_PORT_SPEED_15000_E",
                                        help="Force operation at 15Gbps"      },
        ["75000"] = { value="CPSS_PORT_SPEED_75000_E",
                                        help="Force operation at 75Gbps"      },
        ["100000"] ={ value="CPSS_PORT_SPEED_100G_E",
                                        help="Force operation at 100Gbps"     },
        ["50000"] = { value="CPSS_PORT_SPEED_50000_E",
                                        help="Force operation at 50Gbps"      },
        ["140000"] ={ value="CPSS_PORT_SPEED_140G_E",
                                        help="Force operation at 140Gbps"     },
        ["11800"] = { value="CPSS_PORT_SPEED_11800_E",
                                        help="Force operation at 11.8Gbps"    },
        ["47200"] = { value="CPSS_PORT_SPEED_47200_E",
                                        help="Force operation at 47.2Gbps"    },
        ["22000"] = { value="CPSS_PORT_SPEED_22000_E",
                                        help="Force operation at 22Gbps"      },
        ["23600"] = { value="CPSS_PORT_SPEED_23600_E",
                                        help="Force operation at 23.6Gbps"    },
        ["12500"] = { value="CPSS_PORT_SPEED_12500_E",
                                        help="Force operation at 12.5Gbps"    },
        ["25000"] = { value="CPSS_PORT_SPEED_25000_E",
                                        help="Force operation at 25Gbps"      },
        ["107000"] ={ value="CPSS_PORT_SPEED_107G_E",
                                        help="Force operation at 107Gbps"     },

    }
}


--------------------------------------------
-- type registration: interface_speed
--------------------------------------------
CLI_type_dict["interface_speed"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Change port's speed",
    enum = {
        ["10"]     = { value=0,  help="Change port's speed to 10M"  },
        ["100"]    = { value=1,  help="Change port's speed to 100M" },
        ["1000"]   = { value=2,  help="Change port's speed to 1G"   },
        ["10000"]  = { value=3,  help="Change port's speed to 10G"  },
        ["12000"]  = { value=4,  help="Change port's speed to 12G"  },
        ["2500"]   = { value=5,  help="Change port's speed to 2.5G" },
        ["5000"]   = { value=6,  help="Change port's speed to 5G"   },
        ["13600"]  = { value=7,  help="Change port's speed to 13.6G"},
        ["20000"]  = { value=8,  help="Change port's speed to 20G"  },
        ["40000"]  = { value=9,  help="Change port's speed to 40G"  },
        ["16000"]  = { value=10, help="Change port's speed to 16G"  },
        ["15000"]  = { value=11, help="Change port's speed to 15G"  },
        ["75000"]  = { value=12, help="Change port's speed to 75G"  },
        ["100000"] = { value=13, help="Change port's speed to 100G" },
        ["50000"]  = { value=14, help="Change port's speed to 50G"  },
        ["140000"] = { value=15, help="Change port's speed to 140G" },
        ["11800"]  = { value=16, help="Change port's speed to 11.8G"},
        ["47200"]  = { value=17, help="Change port's speed to 47.2G"},
        ["22000"]  = { value=18, help="Change port's speed to 22G"},
        ["23600"]  = { value=19, help="Change port's speed to 23.6G"},
        ["12500"]  = { value=20, help="Change port's speed to 12.5G"},
        ["25000"]  = { value=21, help="Change port's speed to 25G"},
        ["107000"] = { value=22, help="Change port's speed to 107G"},
    }
}


--------------------------------------------
-- type registration: port_interface_mode
--------------------------------------------
CLI_type_dict["port_interface_mode"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "Port interface mode",
    enum = {
    ["REDUCED_10BIT"] = { value="CPSS_PORT_INTERFACE_REDUCED_10BIT_E",
                                        help="Reduced 10-bit interface mode"    },
    ["REDUCED_GMII"]    = { value="CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E",
                                        help="Reduced GMII interface mode"      },
    ["MII"]           = { value="CPSS_PORT_INTERFACE_MODE_MII_E",
                                        help="MII interface mode"               },
    ["SGMII"]         = { value="CPSS_PORT_INTERFACE_MODE_SGMII_E",
                                        help="SGMII interface mode"             },
    ["XAUI"]          = { value="CPSS_PORT_INTERFACE_MODE_XGMII_E",
                                        help="XGMII interface mode (alias)"     },
    ["MGMII"]         = { value="CPSS_PORT_INTERFACE_MODE_MGMII_E",
                                        help="MGMII interface mode"             },
    ["1000Base_X"]    = { value="CPSS_PORT_INTERFACE_MODE_1000BASE_X_E",
                                        help="1000 Base X interface mode"       },
    ["GMII"]          = { value="CPSS_PORT_INTERFACE_MODE_GMII_E",
                                        help="GMII interface mode"              },
    ["MII_PHY"]       = { value="CPSS_PORT_INTERFACE_MODE_MII_PHY_E",
                                        help="MII PHY interface mode"           },
    ["QX"]            = { value="CPSS_PORT_INTERFACE_MODE_QX_E",
                                        help="Quarter X interface mode"         },
    ["HX"]            = { value="CPSS_PORT_INTERFACE_MODE_HX_E",
                                        help="Half X interface mode"            },
    ["RXAUI"]         = { value="CPSS_PORT_INTERFACE_MODE_RXAUI_E",
                                        help="RXAUI interface mode"             },
    ["100Base_FX"]    = { value="CPSS_PORT_INTERFACE_MODE_100BASE_FX_E",
                                        help="100 Base-FX interface mode"       },
    ["QSGMII"]        = { value="CPSS_PORT_INTERFACE_MODE_QSGMII_E",
                                        help="Quad SGMII interface mode"        },
    ["XLG"]           = { value="CPSS_PORT_INTERFACE_MODE_XLG_E",
                                        help="XLG(40G) interface mode"          },
    ["LOCAL_XGMII"]   = { value="CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E",
                                        help="XGMII interface mode"             },
    ["NO_SERDES_PORT"] = { value="CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E",
                                        help="generic LOCAL_XGMII"              },
    ["KR"]            = { value="CPSS_PORT_INTERFACE_MODE_KR_E",
                                        help="KR interface mode"                },
    ["HGL"]           = { value="CPSS_PORT_INTERFACE_MODE_HGL_E",
                                        help="HGL interface mode"               },
    ["CHGL"]          = { value="CPSS_PORT_INTERFACE_MODE_CHGL_12_E",
                                        help="CHGL interface mode"              },
    ["ILKN12"]        = { value="CPSS_PORT_INTERFACE_MODE_ILKN12_E",
                                        help="ILKN12 interface mode"            },
    ["SR_LR"]         = { value="CPSS_PORT_INTERFACE_MODE_SR_LR_E",
                                        help="Short/Long reach interface mode"  },
    ["ILKN16"]        = { value="CPSS_PORT_INTERFACE_MODE_ILKN16_E",
                                        help="ILKN16 interface mode"            },
    ["ILKN24"]        = { value="CPSS_PORT_INTERFACE_MODE_ILKN24_E",
                                        help="ILKN24 interface mode"            },
    ["ILKN4"]         = { value="CPSS_PORT_INTERFACE_MODE_ILKN4_E",
                                        help="ILKN4 interface mode"             },
    ["ILKN8"]         = { value="CPSS_PORT_INTERFACE_MODE_ILKN8_E",
                                        help="ILKN8 interface mode"             },
    ["XHGS"]          = { value="CPSS_PORT_INTERFACE_MODE_XHGS_E",
                                        help="XHGS interface mode"              },
    ["XHGS_SR"]       = { value="CPSS_PORT_INTERFACE_MODE_XHGS_SR_E",
                                        help="XHGS SR interface mode"           },
    ["KR2"]           = { value="CPSS_PORT_INTERFACE_MODE_KR2_E",
                                        help="KR2 (2 lanes) interface mode"     },
    ["KR4"]           = { value="CPSS_PORT_INTERFACE_MODE_KR4_E",
                                        help="KR4 (4 lanes) interface mode"     },
    ["SR_LR2"]        = { value="CPSS_PORT_INTERFACE_MODE_SR_LR2_E",
                                        help="SR_LR2 (2 lanes) interface mode"  },
    ["SR_LR4"]        = { value="CPSS_PORT_INTERFACE_MODE_SR_LR4_E",
                                        help="SR_LR4 (4 lanes) interface mode"  },
    ["MLG_40G_10G_40G_10G"] = { value="CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E",
                                        help="MLG_40G_10G_40G_10G (4 lanes) interface mode" },
    ["KR_C"]          = { value="CPSS_PORT_INTERFACE_MODE_KR_C_E",
                                        help="KR consortium interface mode"     },
    ["CR_C"]          = { value="CPSS_PORT_INTERFACE_MODE_CR_C_E",
                                        help="CR consortium interface mode"     },
    ["KR2_C"]         = { value="CPSS_PORT_INTERFACE_MODE_KR2_C_E",
                                        help="KR2 consortium (2 lanes) interface mode" },
    ["CR2_C"]         = { value="CPSS_PORT_INTERFACE_MODE_CR2_C_E",
                                        help="CR2 consortium (2 lanes) interface mode" },
    ["CR"]            = { value="CPSS_PORT_INTERFACE_MODE_CR_E",
                                        help="CR interface mode"                },
    ["CR2"]            = { value="CPSS_PORT_INTERFACE_MODE_CR2_E",
                                        help="CR2 interface mode"                },
    ["CR4"]            = { value="CPSS_PORT_INTERFACE_MODE_CR4_E",
                                        help="CR4 interface mode"                },
    ["KR_S"]            = { value="CPSS_PORT_INTERFACE_MODE_KR_S_E",
                                        help="KR_S interface mode"                },
    ["CR_S"]            = { value="CPSS_PORT_INTERFACE_MODE_CR_S_E",
                                        help="CR_S interface mode"                }
    }
}

--------------------------------------------
-- type registration: interface_mode
--------------------------------------------
CLI_type_dict["interface_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Change port's default mode of serdes configuration",
    enum = {
    ["REDUCED_10BIT"] = { value=0,  help="Change port's mode to REDUCED_10BIT"},
    ["REDUCED_GMII"] =  { value=1,  help="Change port's mode to REDUCED_GMII"   }, 
    ["MII"] =           { value=2,  help="Change port's mode to MII"      },
    ["SGMII"] =         { value=3,  help="Change port's mode to SGMII"    },
    ["XAUI"] =          { value=4,  help="Change port's mode to XAUI"     },
    ["MGMII"] =         { value=5,  help="Change port's mode to MGMII"    },
    ["1000BASEX"] =     { value=6,  help="Change port's mode to 1000BASEX" },
    ["GMII"] =          { value=7,  help="Change port's mode to GMII"     },
    ["MII_PHY"] =       { value=8,  help="Change port's mode to MII_PHY"  },
    ["QX"] =            { value=9,  help="Change port's mode to QX"       },
    ["HX"] =            { value=10, help="Change port's mode to HX"       },
    ["RXAUI"] =         { value=11, help="Change port's mode to RXAUI"    },
    ["100BASEFX"] =     { value=12, help="Change port's mode to 100BASEFX"},
    ["QSGMII"] =        { value=13, help="Change port's mode to QSGMII"   },
    ["XLG"] =           { value=14, help="Change port's mode to XLG"      },
    ["LOCAL_XGMII"] =   { value=15, help="Change port's mode to LOCAL_XGMII"    },
    ["NO_SERDES_PORT"] ={ value=15, help="Change port's mode to NO_SERDES_PORT" },
    ["KR"] =            { value=16, help="Change port's mode to KR"       },
    ["HGL"] =           { value=17, help="Change port's mode to HGL"      },
    ["CHGL"] =          { value=18, help="Change port's mode to CHGL"     },
    ["ILKN12"] =        { value=19, help="Change port's mode to ILKN12"   },
    ["SR_LR"] =         { value=20, help="Change port's mode to SR_LR"    },
    ["ILKN16"] =        { value=21, help="Change port's mode to ILKN16"   },
    ["ILKN24"] =        { value=22, help="Change port's mode to ILKN24"   },
    ["ILKN4"] =         { value=23, help="Change port's mode to ILKN4"    },
    ["ILKN8"] =         { value=24, help="Change port's mode to ILKN8"    },
    ["XHGS"] =          { value=25, help="Change port's mode to XHGS"     },
    ["XHGS_SR"] =       { value=26, help="Change port's mode to XHGS_SR"  },
    ["KR2"] =           { value=27, help="Change port's mode to KR2"      },
    ["KR4"] =           { value=28, help="Change port's mode to KR4"      },
    ["SR_LR2"] =        { value=29, help="Change port's mode to SR_LR2"   },
    ["SR_LR4"] =        { value=30, help="Change port's mode to SR_LR4"   },
    ["MLG_40G_10G_40G_10G"] = { value=31, help="Change port's mode to MLG_40G_10G_40G_10G" },
    ["KR_C"]   =        { value=32, help="Change port's mode to KR consortium"  },
    ["CR_C"]   =        { value=33, help="Change port's mode to CR consortium"  },
    ["KR2_C"]  =        { value=34, help="Change port's mode to KR2 consortium" },
    ["CR2_C"]  =        { value=35, help="Change port's mode to CR2 consortium" },
    ["CR"] =            { value=36, help="Change port's mode to CR"       },
    ["CR2"] =           { value=37, help="Change port's mode to CR2"      },
    ["CR4"] =           { value=38, help="Change port's mode to CR4"      },
    ["KR_S"] =          { value=39, help="Change port's mode to KR_S"     },
    ["CR_S"] =          { value=40, help="Change port's mode to CR_S"     },

    }
}

--------------------------------------------
-- type registration: autodetect
--------------------------------------------
CLI_type_dict["autodetect"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Auto-detect port's actual mode and speed",
    enum = {
        ["XLG_40000"]      = { value = "XLG_40000",
                               help  = "The interface works in XLG mode,        40Gbps"  },
        ["RXAUI_10000"]    = { value = "RXAUI_10000",
                               help  = "The interface works in RXAUI mode,      10Gbps"  },
        ["XAUI_10000"]     = { value = "XAUI_10000",
                               help  = "The interface works in XAUI mode,       10Gbps"  },
        ["XAUI_20000"]     = { value = "XAUI_20000",
                               help  = "The interface works in XAUI mode,       20Gbps"  },
        ["1000BaseX_1000"] = { value = "1000BaseX_1000",
                               help  = "The interface works in 1000BaseX mode,  1Gbps"   },
        ["SGMII_1000"]     = { value = "SGMII_1000",
                               help  = "The interface works in SGMII mode,      1Gbps"   },
        ["SGMII_2500"]     = { value = "SGMII_2500",
                               help  = "The interface works in SGMII mode,      2.5Gbps" },
        ["QSGMII_1000"]    = { value = "QSGMII_1000",
                               help  = "The interface works in Quad SGMII mode, 1Gbps"   }
    }
}

--------------------------------------------
-- type registration: prbs_mode
--------------------------------------------

CLI_type_dict["prbs_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "PRBS mode",
    enum = {
           ["regular"]      = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E",    help="REGULAR"     },
           ["prbs"]         = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS_E",       help="PRBS"        },
           ["zeros"]        = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_ZEROS_E",      help="ZEROS"       },
           ["ones"]         = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_ONES_E",       help="ONES"        },
           ["cyclic"]       = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_CYCLIC_E",     help="CYCLIC"      },
           ["7"]            = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E",      help="PRBS 7"      },
           ["9"]            = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS9_E",      help="PRBS 9"      },
           ["15"]           = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E",     help="PRBS 15"     },
           ["23"]           = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E",     help="PRBS 23"     },
           ["31"]           = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E",     help="PRBS 31"     },
           ["1t"]           = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_1T_E",         help="MODE 1T"     },
           ["2t"]           = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_2T_E",         help="MODE 2T"     },
           ["5t"]           = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_5T_E",         help="MODE 5T"     },
           ["10t"]          = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_10T_E",        help="MODE 10T"    },
           ["dfet_raining"] = {value="CPSS_DXCH_DIAG_TRANSMIT_MODE_DFETraining",  help="DFETraining" }
    }
}

--------------------------------------------
-- type registration: auto_tune
--------------------------------------------
CLI_type_dict["auto_tune"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Auto tune mode",
    enum = {
        ["trx-training"] =  { value="CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E", help="Run TRX training" },
        ["rx-training"] =   { value="CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E", help="Run RX training"        }
    }
}

--------------------------------------------
-- type registration: auto_tune for bobCat2
--------------------------------------------
CLI_type_dict["auto_tune_for_bobCat2"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Auto tune type",
    enum = {
        ["trx"] =   { value="CPSS_PORT_SERDES_TUNING_TRX_E", help="set TRX"       },
        ["rx"] =    { value="CPSS_PORT_SERDES_TUNING_RX_ONLY_E", help="set RX"    }
    }
}

--------------------------------------------
-- type registration: stop auto_tune for bobCat2
--------------------------------------------
CLI_type_dict["stop_auto_tune_for_bobCat2"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "stop Auto tune",
    enum = {
        ["trx"] =   { value="CPSS_PORT_SERDES_TUNING_TRX_E", help="unset TRX"     },
        ["rx"] =    { value="CPSS_PORT_SERDES_TUNING_RX_ONLY_E", help="unset RX"  }
    }
}

--------------------------------------------
-- type registration: ap port fc direction enablers
--------------------------------------------
CLI_type_dict["ap_flow_control"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "enumerator of ap port fc direction enablers",
    enum = {
        ["symmetric"] =    { value="CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E", help="flow control in both directions" },
        ["asymmetric"] =   { value="CPSS_DXCH_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E", help="flow control in one directions" }
    }
}

--------------------------------------------
-- type registration: ap_port_fec_mode
--------------------------------------------
CLI_type_dict["ap_port_fec_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Ap port fec mode",
    enum = {
        ["disabled"] =    { value="CPSS_DXCH_PORT_FEC_MODE_DISABLED_E", help="FEC disabled" },
        ["fc_fec"] =   { value="CPSS_DXCH_PORT_FEC_MODE_ENABLED_E", help="FC-FEC enabled on port" },
        ["rs_fec"] =   { value="CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E", help="RS-FEC enabled on port" },
        ["both_fec"] =   { value="CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E", help="both FEC modes enabled on port" }
    }
}


--------------------------------------------
-- type registration: ap_port_fec_mode
--------------------------------------------
CLI_type_dict["port_manager_event"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Port manager event",
    enum = {
        ["create"]           = { value="CPSS_PORT_MANAGER_EVENT_CREATE_E",                   help="Create port" },
        ["delete"]           = { value="CPSS_PORT_MANAGER_EVENT_DELETE_E",                   help="Delete port" },
        ["enable"]           = { value="CPSS_PORT_MANAGER_EVENT_ENABLE_E",                   help="Enable port" },
        ["disable"]          = { value="CPSS_PORT_MANAGER_EVENT_DISABLE_E",                  help="Disable port" },
        ["init"]             = { value="CPSS_PORT_MANAGER_EVENT_INIT_E",                     help="Init port" },
        ["low_level_change"] = { value="CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E", help="Notify on low level status change" },
        ["mac_level_change"] = { value="CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E", help="Notify on mac level status change" },
        ["ap_hcd_found"]     = { value="CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E",        help="Notify on AP HCD found" }
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


--------------------------------------------
-- type registration: serdes_speed
--------------------------------------------
CLI_type_dict["serdes_speed"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Defines SERDES speed",
    enum = {
        ["1.25"] =      { value="CPSS_DXCH_PORT_SERDES_SPEED_1_25_E",    help="Serdes speed is 1.25G"  },
        ["3.125"] =     { value="CPSS_DXCH_PORT_SERDES_SPEED_3_125_E",   help="Serdes speed is 3.125G" },
        ["3.75"] =      { value="CPSS_DXCH_PORT_SERDES_SPEED_3_75_E",    help="Serdes speed is 3.75G"  },
        ["6.25"] =      { value="CPSS_DXCH_PORT_SERDES_SPEED_6_25_E",    help="Serdes speed is 6.25G"  },
        ["5"] =         { value="CPSS_DXCH_PORT_SERDES_SPEED_5_E",       help="Serdes speed is 5G"     },
        ["4.25"] =      { value="CPSS_DXCH_PORT_SERDES_SPEED_4_25_E",    help="Serdes speed is 4.25G"  },
        ["2.5"] =       { value="CPSS_DXCH_PORT_SERDES_SPEED_2_5_E",     help="Serdes speed is 2.5G"   },
        ["5.156"] =     { value="CPSS_DXCH_PORT_SERDES_SPEED_5_156_E",   help="Serdes speed is 5.156G" },
        ["10.3125"] =   { value="CPSS_DXCH_PORT_SERDES_SPEED_10_3125_E", help="used by KR"             },
        ["3.333"] =     { value="CPSS_DXCH_PORT_SERDES_SPEED_3_333_E",   help="used by HGL 16G"        },
        ["12.5"] =      { value="CPSS_DXCH_PORT_SERDES_SPEED_12_5_E",    help="for 12.5G, 25G and 50G" },
        ["7.5"] =       { value="CPSS_DXCH_PORT_SERDES_SPEED_7_5_E",     help="legacy"                 },
        ["11.25"] =     { value="CPSS_DXCH_PORT_SERDES_SPEED_11_25_E",   help="legacy"                 },
        ["11.5625"] =   { value="CPSS_DXCH_PORT_SERDES_SPEED_11_5625_E", help="for 22G SR_LR"          },
        ["10.9375"] =   { value="CPSS_DXCH_PORT_SERDES_SPEED_10_9375_E", help="legacy"                 },
        ["12.1875"] =   { value="CPSS_DXCH_PORT_SERDES_SPEED_12_1875_E", help="for 11.8G, 23.6G, 47.2G"},
        ["5.625"] =     { value="CPSS_DXCH_PORT_SERDES_SPEED_5_625_E",   help="for 5.45G in xCat3"     },
        ["12.8906"] =   { value="CPSS_DXCH_PORT_SERDES_SPEED_12_8906_E", help="for 12.5G, 25G and 50G" },
        ["20.625"] =    { value="CPSS_DXCH_PORT_SERDES_SPEED_20_625_E",  help="for 40G R2"             },
        ["25.78125"] =  { value="CPSS_DXCH_PORT_SERDES_SPEED_25_78125_E",help="for 25G, 50G, 100G"     },
        ["27.5"] =      { value="CPSS_DXCH_PORT_SERDES_SPEED_27_5",      help="for EDSA compensation mode 107G"},
        ["28.05"] =     { value="CPSS_DXCH_PORT_SERDES_SPEED_28_05",     help="for EDSA compensation mode 109G"}
    }
}

-- ************************************************************************
---
--  fc_intervals_get
--        @description  Getting the Flow Control Intervals per Speed table:
--          Key - Port Speed
--          Value - The interval in microseconds between two successive Flow Control frames.
--          The interval in micro seconds between transmission of two consecutive
--          Flow Control packets recommended interval is calculated by the following formula:
--                   period (micro seconds) = 33553920 / speed(M)
--          Exception: for 10M, 100M and 10000M Flow Control packets recommended interval is 33500
--        @return   The Table: Flow Control Intervals per Speed
function fc_intervals_get()
    local fc_intervals = {
                            CPSS_PORT_SPEED_10_E =    33500,
                            CPSS_PORT_SPEED_100_E =   33500,
                            CPSS_PORT_SPEED_1000_E =  33500,
                            CPSS_PORT_SPEED_2500_E =  13421,
                            CPSS_PORT_SPEED_5000_E =  6710,
                            CPSS_PORT_SPEED_10000_E = 3355,
                            CPSS_PORT_SPEED_11800_E = 2843,
                            CPSS_PORT_SPEED_12000_E = 2796,
                            CPSS_PORT_SPEED_13600_E = 2467,
                            CPSS_PORT_SPEED_15000_E = 2236,
                            CPSS_PORT_SPEED_16000_E = 2097,
                            CPSS_PORT_SPEED_20000_E = 1677,
                            CPSS_PORT_SPEED_22000_E = 1525,
                            CPSS_PORT_SPEED_40000_E = 838,
                            CPSS_PORT_SPEED_47200_E = 710,
                            CPSS_PORT_SPEED_50000_E = 671,
                            CPSS_PORT_SPEED_75000_E = 447,
                            CPSS_PORT_SPEED_100G_E =  335,
                            CPSS_PORT_SPEED_140G_E =  239
                        }
    return deepcopy(fc_intervals)
end


