--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* enums.lua
--*
--* DESCRIPTION:
--*       base lue enums
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes


ENUM = {}

ENUM["CPSS_PORT_SPEED_ENT"] = {
    [0]             = 10,
    [1]             = 100,
    [2]             = 1000,
    [3]             = 10000,
    [4]             = 12000,
    [5]             = 2500,
    [6]             = 5000,
    [7]             = 13600,
    [8]             = 20000,
    [9]             = 40000,
    [10]            = 16000,
    [11]            = 15000,
    [12]            = 75000,
    [13]            = 100000,
    [14]            = 50000,
    [15]            = 140000,
    [16]            = 11800,
    [17]            = 47200,
    [18]            = 22000,
    ["speed_10"]    = 10,
    ["speed_100"]   = 100,
    ["speed_1G"]    = 1000,
    ["speed_10G"]   = 10000,
    ["speed_12G"]   = 12000,
    ["speed_2.5G"]  = 2500,
    ["speed_5G"]    = 5000,
    ["speed_13.6G"] = 13600,
    ["speed_20G"]   = 20000,
    ["speed_40G"]   = 40000,
    ["speed_16G"]   = 16000,
    ["speed_15G"]   = 15000,
    ["speed_75G"]   = 75000,
    ["speed_100G"]  = 100000,
    ["speed_50G"]   = 50000,
    ["speed_140G"]  = 140000,
    ["speed_11.8G"] = 11800,
    ["speed_47.2G"] = 47200,
    ["speed_22G"]   = 22000,

    ["n/a"]         = nil
    }

ENUM["CPSS_PORT_DUPLEX_ENT"] = {
    [0] = "Full",
    [1] = "Half",
    ["CPSS_PORT_FULL_DUPLEX_E"] = "Full",
    ["CPSS_PORT_HALF_DUPLEX_E"] = "Half",   
    ["Full"] = 0,
    ["Half"] = 1
    }

ENUM["CPSS_NST_CHECK_ENT"] = {
   [0] = "CPSS_NST_CHECK_TCP_SYN_DATA_E",
   [1] = "CPSS_NST_CHECK_TCP_OVER_MAC_MC_BC_E",
   [2] = "CPSS_NST_CHECK_TCP_FLAG_ZERO",
   [3] = "CPSS_NST_CHECK_TCP_FLAGS_FIN_URG_PSH_E",
   [4] = "CPSS_NST_CHECK_TCP_FLAGS_SYN_FIN_E",
   [5] = "CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E",
   [6] = "CPSS_NST_CHECK_TCP_UDP_PORT_ZERO_E",
   [7] = "CPSS_NST_CHECK_TCP_ALL_E",
   [8] = "CPSS_NST_CHECK_FRAG_IPV4_ICMP_E",
   [9] = "CPSS_NST_CHECK_ARP_MAC_SA_MISMATCH_E"
    }
    
ENUM["PORT_SPEED"] = {
    [0]             = 10,
    [1]             = 100,
    [2]             = 1000,
    [3]             = 10000,
    [4]             = 12000,
    [5]             = 2500,
    [6]             = 5000,
    [7]             = 13600,
    [8]             = 20000,
    [9]             = 40000,
    [10]            = 16000,
    [11]            = 15000,
    [12]            = 75000,
    [13]            = 100000,
    [14]            = 50000,
    [15]            = 140000,
    [16]            = 11800,
    [17]            = 47200,
    [18]            = 22000,
    [19]            = "NA",
    [20]            = nil
    }

 ENUM["INTERFACE_MODE"] = {
    [0]             = "REDUCED_10BIT",
    [1]             = "REDUCED_GMII",
    [2]             = "MII",
    [3]             = "SGMII",
    [4]             = "XGMII",
    [5]             = "MGMII",
    [6]             = "1000BASE_X",
    [7]             = "GMII",
    [8]             = "MII_PHY",
    [9]             = "QX",
    [10]            = "HX",
    [11]            = "RXAUI",
    [12]            = "100BASE_FX",
    [13]            = "QSGMII",
    [14]            = "XLG",
    [15]            = "LOCAL_XGMII",
    [16]            = "KR",
    [17]            = "HGL",
    [18]            = "CHGL_12",
    [19]            = "ILKN12",
    [20]            = "SR_LR",
    [21]            = "ILKN16",
    [22]            = "ILKN24",
    [23]            = "ILKN4",
    [24]            = "ILKN8",
    [25]            = "XHGS",
    [26]            = "XHGS_SR",
    [27]            = "NA",
    [28]            = nil
    }
