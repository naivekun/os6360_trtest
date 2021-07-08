--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_drop_enum.lua
--*
--* DESCRIPTION:
--*       Modes enum for bridge drop counter
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes


--constants


--------------------------------------------
-- type registration: bridge_drop_counter_mode
--------------------------------------------
CLI_type_dict["bridge_drop_counter_mode"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "Enter counter mode",
    enum = {
    ["COUNT_ALL"]=          {value="CPSS_DXCH_BRG_DROP_CNTR_COUNT_ALL_E ",
                                                  help="Count All."},
    ["FDB_ENTRY_CMD"]=      {value="CPSS_DXCH_BRG_DROP_CNTR_FDB_ENTRY_CMD_E",
                                                  help="FDB Entry command Drop."},
    ["UNKNOWN_MAC_SA"]=     {value="CPSS_DXCH_BRG_DROP_CNTR_UNKNOWN_MAC_SA_E",
                                                  help="Unknown MAC SA Drop."},
    ["INVALID_SA"]=         {value="CPSS_DXCH_BRG_DROP_CNTR_INVALID_SA_E",
                                                  help="Invalid SA Drop."},
    ["INVALID_VLAN"]=       {value="CPSS_DXCH_BRG_DROP_CNTR_INVALID_VLAN_E",
                                                  help="VLAN not valid Drop."},
    ["PORT_NOT_IN_VLAN"]=   {value="CPSS_DXCH_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E",
                                                  help="Port not Member in VLAN Drop."},
    ["VLAN_RANGE"]=         {value="CPSS_DXCH_BRG_DROP_CNTR_VLAN_RANGE_E",
                                                  help="VLAN Range Drop."},
    ["MOVED_STATIC_ADDR"]=  {value="CPSS_DXCH_BRG_DROP_CNTR_MOVED_STATIC_ADDR_E",
                                                  help="Moved Static Address Drop."},
    ["ARP_SA_MISMATCH"]=    {value="CPSS_DXCH_BRG_DROP_CNTR_ARP_SA_MISMATCH_E",
                                                  help="ARP SA Mismatch Drop."},
    ["SYN_WITH_DATA"]=      {value="CPSS_DXCH_BRG_DROP_CNTR_SYN_WITH_DATA_E",
                                                  help="SYN with Data Drop."},
    ["TCP_OVER_MC_OR_BC"]=  {value="CPSS_DXCH_BRG_DROP_CNTR_TCP_OVER_MC_OR_BC_E",
                                                  help="TCP over MC or BC Drop."},
    ["ACCESS_MATRIX"]=      {value="CPSS_DXCH_BRG_DROP_CNTR_ACCESS_MATRIX_E",
                                                  help="Access Matrix Drop."},
    ["SEC_LEARNING"]=       {value="CPSS_DXCH_BRG_DROP_CNTR_SEC_LEARNING_E",
                                                  help="Secure Learning Drop."},
    ["ACCEPT_FRAME_TYPE"]=  {value="CPSS_DXCH_BRG_DROP_CNTR_ACCEPT_FRAME_TYPE_E",
                                                  help="Accept Frame Type Drop."},
    ["FRAG_ICMP"]=          {value="CPSS_DXCH_BRG_DROP_CNTR_FRAG_ICMP_E",
                                                  help="Fragmented ICMP Drop."},
    ["TCP_FLAGS_ZERO"]=     {value="CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_ZERO_E",
                                                  help="TCP Flags Zero Drop."},
    ["TCP_FLAGS_FUP_SET"]=  {value="CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_FUP_SET_E",
                                                  help="TCP flags FIN, URG, PUSH Set Drop."},
    ["TCP_FLAGS_SF_SET"]=   {value="CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_SF_SET_E",
                                                  help="TCP Flags SYN,FIN Set Drop."},
    ["TCP_FLAGS_SR_SET"]=   {value="CPSS_DXCH_BRG_DROP_CNTR_TCP_FLAGS_SR_SET_E",
                                                  help="TCP Flags SYN, RST Set Drop."},
    ["TCP_UDP_PORT_ZERO"]=  {value="CPSS_DXCH_BRG_DROP_CNTR_TCP_UDP_PORT_ZERO_E",
                                                  help="TCP UDP Port Zero Drop."},
    ["VLAN_MRU"]=           {value="CPSS_DXCH_BRG_DROP_CNTR_VLAN_MRU_E",
                                                  help="VLAN MRU Drop."},
    ["RATE_LIMIT"]=         {value="CPSS_DXCH_BRG_DROP_CNTR_RATE_LIMIT_E",
                                                  help="Rate Limiting Drop."},
    ["LOCAL_PORT"]=         {value="CPSS_DXCH_BRG_DROP_CNTR_LOCAL_PORT_E",
                                                  help="Local Port Drop."},
    ["SPAN_TREE_PORT_ST"]=  {value="CPSS_DXCH_BRG_DROP_CNTR_SPAN_TREE_PORT_ST_E",
                                                  help="Spanning Tree Port state Drop."},
    ["IP_MC"]=              {value="CPSS_DXCH_BRG_DROP_CNTR_IP_MC_E",
                                                  help="IP Multicas tDrop."},
    ["NON_IP_MC"]=          {value="CPSS_DXCH_BRG_DROP_CNTR_NON_IP_MC_E",
                                                  help="NON-IP Multicast Drop."},
    ["DSATAG_LOCAL_DEV"]=   {value="CPSS_DXCH_BRG_DROP_CNTR_DSATAG_LOCAL_DEV_E",
                                                  help="DSA Tag due to Local device Drop."},
    ["IEEE_RESERVED"]=      {value="CPSS_DXCH_BRG_DROP_CNTR_IEEE_RESERVED_E",
                                                  help="IEEE Reserved Drop."},
    ["UREG_L2_NON_IPM_MC"]= {value="CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPM_MC_E",
                                                  help="Unregistered L2 NON-IPM Multicast Drop."},
    ["UREG_L2_IPV6_MC"]=    {value="CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV6_MC_E",
                                                  help="Unregistered L2 IPv6 Multicast Drop."},
    ["UREG_L2_IPV4_MC"]=    {value="CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV4_MC_E",
                                                  help="Unregistered L2 IPv4 Multicast Drop."},
    ["UNKNOWN_L2_UC"]=      {value="CPSS_DXCH_BRG_DROP_CNTR_UNKNOWN_L2_UC_E",
                                                  help="Unknown L2 Unicast Drop."},
    ["UREG_L2_IPV4_BC"]=    {value="CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_IPV4_BC_E",
                                                  help="Unregistered L2 IPv4 Broadcast Drop."},
    ["UREG_L2_NON_IPV4_BC"]={value="CPSS_DXCH_BRG_DROP_CNTR_UREG_L2_NON_IPV4_BC_E",
                                                  help="Unregistered L2 NON-IPv4 Broadcast Drop."}
    }
}



