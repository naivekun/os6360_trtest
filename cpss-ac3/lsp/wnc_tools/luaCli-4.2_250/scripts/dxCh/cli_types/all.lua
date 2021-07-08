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
--*       including of all types
--*
--* FILE REVISION NUMBER:
--*       $Revision: 15 $
--*
--********************************************************************************

--includes
require("dxCh/cli_types/arp_entry_index")
require("dxCh/cli_types/channel_group")
-- require("dxCh/cli_types/devID")
require("dxCh/cli_types/drop_threshold_bit11")
require("dxCh/cli_types/feedback_weight")
require("dxCh/cli_types/ipv4_mask")
require("dxCh/cli_types/ipv6_mask")
require("dxCh/cli_types/max_received_packet_size")
require("dxCh/cli_types/max_vlan_packet_size")
require("dxCh/cli_types/min_sample_base_bytes")
require("dxCh/cli_types/multicast")
require("dxCh/cli_types/next_hop_id")
require("dxCh/cli_types/packet_length_bytes")
require("dxCh/cli_types/port_enums")
require("dxCh/cli_types/portNum")
require("dxCh/cli_types/second")
require("dxCh/cli_types/set_point_bytes")
require("dxCh/cli_types/trunkID")
require("dxCh/cli_types/vlanID")
require("dxCh/cli_types/vlan_list")
require("dxCh/cli_types/descriptor_types")
require("dxCh/cli_types/pcl")
require("dxCh/cli_types/pfc_profile")
require("dxCh/cli_types/qos")
require("dxCh/cli_types/xoff_bit11")
require("dxCh/cli_types/xon_bit11")
require("dxCh/cli_types/phySmi")
require("dxCh/cli_types/loopback_serdes_type")
require("dxCh/cli_types/lane_number_type")
require("dxCh/cli_types/packetNumberToTransmit")
require("dxCh/cli_types/bridge_drop_enum")
require("dxCh/cli_types/tti")
require("dxCh/cli_types/tpid_range")
require("dxCh/cli_types/tail_drop")
require("dxCh/cli_types/vsi")
require("dxCh/cli_types/bpe_802_1_br_types")
require("dxCh/cli_types/vss_types")
require("dxCh/cli_types/asicSimulation")
require("dxCh/cli_types/vrf_id")
require("dxCh/cli_types/types_eee")
require("dxCh/cli_types/types_pip")
require("dxCh/cli_types/types_network")
require("dxCh/cli_types/packet_cmd_enum")
require("dxCh/cli_types/types_fwdtolb")



CLI_type_dict["GT_SW_DEV_NUM"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    help="Device number"
}


--------------------------------------------
-- CPSS integer type registration
--------------------------------------------
do
  local inttypes = {"GT_TRUNK_ID", "GT_HW_DEV_NUM"}
  local i, n
  for i, n in pairs(inttypes) do
    _G["mgmType_to_c_"..n] = mgmType_to_c_int
    _G["mgmType_to_lua_"..n] = mgmType_to_lua_int
  end
end
