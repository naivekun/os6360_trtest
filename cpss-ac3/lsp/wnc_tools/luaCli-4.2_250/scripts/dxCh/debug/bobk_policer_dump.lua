--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bobk_policer_dump.lua
--*
--* DESCRIPTION:
--*       dumping bobk policer tables
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

local bobk_buckets_table_address = {
	["CPSS_DXCH_POLICER_STAGE_INGRESS_0_E"] = 0x0B100000,
	["CPSS_DXCH_POLICER_STAGE_INGRESS_1_E"] = 0x20100000,
	["CPSS_DXCH_POLICER_STAGE_EGRESS_E"   ] = 0x0E100000,
};

local bobk_buckets_entry_fields = {
	last_time0 = {0,27},
	last_time1 = {28,55},
	wrap_around0 = {56,56},
	wrap_around1 = {57,57},
	bucket_size0 = {58,89},
	bucket_size1 = {90,121},
	rate_type0 = {122,124},
	rate_type1 = {125,127},
	rate0 = {128,144},
	rate0 = {145,161},
	max_burst_size0 = {162,177},
	max_burst_size1 = {178,193},
	bucket_rank0 = {194,196},
	bucket_rank1 = {197,199},
	bucket_color0 = {200,200},
	bucket_color1 = {201,201},
	coupling_flag = {202,202},
	coupling_flag0 = {203,203},
	max_rate_index = {204,210},
};

local bobk_buckets_entry_fields_values_names = {
	bucket_color0 = {[0] = "green",[1] = "yellow",},
	bucket_color1 = {[0] = "green",[1] = "yellow",},
	coupling_flag = {
		[0] = "spill to next rank green",
		[1] = "spill to same rank yellow",
	},
	coupling_flag0 = {
		[0] = "don't speel",
		[1] = "spill to higest rank yellow",
	},
};

local bobk_metering_ingress_cfg_entry_fields = {
	color_mode = {0,0},
	metering_algorithm = {1,3},
	mng_counters_set = {4,5},
	counter_pointer = {6,21},
	byte_or_packet_conting = {22,22},
	packet_size_mode = {23,23},
	tt_packet_size_mode = {24,24},
	include_l1_mode = {25,25},
	dsa_tag_mode = {26,26},
	tymestamp_mode = {27,27},
	red_command = {28,29},
	yellow_command = {30,31},
	green_command = {32,33},
	modify_dscp = {34,35},
	modify_up = {36,37},
	qos_profile = {38,47},
    envelope_size = {48,50},
};

local bobk_metering_ingress_cfg_entry_cmd = {
	[0] = "no change",
	[1] = "drop",
	[2] = "qos profile mark by table",
	[3] = "qos profile mark by entry",
};
local bobk_metering_cfg_entry_algorithm = {
	[0] = "single rate TCM",
	[1] = "two rate TCM",
	[2] = "MEF0",
	[3] = "MEF1",
	[4] = "MEF10.3 Start",
	[5] = "MEF10.3 Not Start",
};
local bobk_metering_ingress_cfg_entry_fields_values_names = {
	color_mode = {[0] = "color blind",[1] = "color aware"},
	metering_algorithm = bobk_metering_cfg_entry_algorithm,
	mng_counters_set = {
		[0]= "set0",[1]= "set1",[2]= "set2",[3]= "disable",},
	byte_or_packet_conting = {[0]="byte",[1]="packet",},
	packet_size_mode = {[0]="L3",[1]="L2",},
	tt_packet_size_mode = {[0]="regular",[1]="passenger",},
	include_l1_mode = {[0]="exclude",[1]="include",},
	dsa_tag_mode = {[0]="included",[1]="compemsated",},
	tymestamp_mode = {[0]="include",[1]="exclude",},
	red_command = bobk_metering_ingress_cfg_entry_cmd,
	yellow_command = bobk_metering_ingress_cfg_entry_cmd,
	green_command = bobk_metering_ingress_cfg_entry_cmd,
	modify_dscp = {[0]="keep previous",[1]="enable",[2]="disable",},
	modify_up = {[0]="keep previous",[1]="enable",[2]="disable",},
};


local bobk_metering_egress_cfg_entry_fields = {
	color_mode = {0,0},
	metering_algorithm = {1,3},
	mng_counters_set = {4,5},
	counter_pointer = {6,21},
	byte_or_packet_conting = {22,22},
	packet_size_mode = {23,23},
	tt_packet_size_mode = {24,24},
	include_l1_mode = {25,25},
	dsa_tag_mode = {26,26},
	tymestamp_mode = {27,27},
	ip_mpls_remark_mode = {28,28},
	red_command = {29,29},
	modify_exp = {30,30},
	modify_dscp = {31,32},
	modify_tc = {33,33},
	modify_up = {34,35},
	modify_dp = {36,36},
	yellow_ecn_remarking = {37,37},
	reserved1 = {38,47},
    envelope_size = {48,50},
};

local bobk_metering_egress_cfg_entry_fields_values_names = {
	color_mode = {[0] = "color blind",[1] = "color aware"},
	metering_algorithm = bobk_metering_cfg_entry_algorithm,
	mng_counters_set = {
		[0]= "set0",[1]= "set1",[2]= "set2",[3]= "disable",},
	byte_or_packet_conting = {[0]="byte",[1]="packet",},
	packet_size_mode = {[0]="L3",[1]="L2",},
	tt_packet_size_mode = {[0]="regular",[1]="passenger",},
	include_l1_mode = {[0]="exclude",[1]="include",},
	dsa_tag_mode = {[0]="included",[1]="compemsated",},
	tymestamp_mode = {[0]="include",[1]="exclude",},
	ip_mpls_remark_mode = {[0]="L2-UP/TC",[1]="L3-DSCP/EXP",},
	red_command = {[0]="forward",[1]="drop",},
	modify_exp = {[0]="disable",[1]="enable",},
	modify_dscp = {[0]="keep previous",[1]="modify outer",[2]="modify inner",},
	modify_tc = {[0]="disable",[1]="enable",},
	modify_up = {[0]="don't modify",[1]="modify outer tag",[2]="modify tag0",},
	modify_dp = {[0]="disable",[1]="enable",},
	yellow_ecn_remarking = {[0]="disable",[1]="enable",},
};

local bobk_metering_cfg_format = {
	["CPSS_DXCH_POLICER_STAGE_INGRESS_0_E"] = bobk_metering_ingress_cfg_entry_fields,
	["CPSS_DXCH_POLICER_STAGE_INGRESS_1_E"] = bobk_metering_ingress_cfg_entry_fields,
	["CPSS_DXCH_POLICER_STAGE_EGRESS_E"   ] = bobk_metering_egress_cfg_entry_fields,
};

local bobk_metering_cfg_values_names = {
	["CPSS_DXCH_POLICER_STAGE_INGRESS_0_E"] = bobk_metering_ingress_cfg_entry_fields_values_names,
	["CPSS_DXCH_POLICER_STAGE_INGRESS_1_E"] = bobk_metering_ingress_cfg_entry_fields_values_names,
	["CPSS_DXCH_POLICER_STAGE_EGRESS_E"   ] = bobk_metering_egress_cfg_entry_fields_values_names,
};

local bobk_metering_cfg_table_address = {
	["CPSS_DXCH_POLICER_STAGE_INGRESS_0_E"] = 0x0B300000,
	["CPSS_DXCH_POLICER_STAGE_INGRESS_1_E"] = 0x20300000,
	["CPSS_DXCH_POLICER_STAGE_EGRESS_E"   ] = 0x0E300000,
};

local function policy_bobk_buckets_dump(params)
    local devices,j;
	local memo_base = bobk_buckets_table_address[params.stage];

    if (params["devID"]=="all") then
        devices=wrLogWrapper("wrlDevList");
    else
        devices={params["devID"]};
    end
    for j=1,#devices do
		local data = generic_pp_memory_row_data_to_fields_values_named(
			devices[j], 0 --[[port_group--]],
			(memo_base + (params.entry_index * 32))--[[address--]],
			7 --[[num_of_words--]],
			bobk_buckets_entry_fields,
			bobk_buckets_entry_fields_values_names);
		print("dev " .. tostring(devices[j]));
		print(to_string(data));
	end
end

CLI_addCommand("debug", "dump bobk-policy-buckets", {
   func = policy_bobk_buckets_dump,
   help="dump policy-bobk-buckets entry",
   params={
	{ type="named",
		{ format= "device %devID_all", name="devID", help="The device number" },
		{ format = "stage %policer_stage", name = "stage", help = "Policer stage"},
	    { format = "entry-index %GT_U32", name = "entry_index", help = "Index in HW"},
		mandatory = {"devID", "stage", "entry_index"},
		requirements={
			["stage"] = {"devID"},
			["entry_index"] = {"stage"},
		},
	}
  }
})

local function policy_bobk_metering_cfg_dump(params)
	local devices,j;
	local memo_base = bobk_metering_cfg_table_address[params.stage];

	if (params["devID"]=="all") then
		devices=wrLogWrapper("wrlDevList");
	else
		devices={params["devID"]};
	end
	for j=1,#devices do
		local data = generic_pp_memory_row_data_to_fields_values_named(
			devices[j], 0 --[[port_group--]],
			(memo_base + (params.entry_index * 8))--[[address--]],
			2 --[[num_of_words--]],
			bobk_metering_cfg_format[params.stage],
			bobk_metering_cfg_values_names[params.stage]);
		print("dev " .. tostring(devices[j]));
		print(to_string(data));
	end
end

CLI_addCommand("debug", "dump bobk-policy-metering-cfg", {
   func = policy_bobk_metering_cfg_dump,
   help="dump policy-bobk-metering-cfg entry",
   params={
	{ type="named",
		{ format= "device %devID_all", name="devID", help="The device number" },
		{ format = "stage %policer_stage", name = "stage", help = "Policer stage"},
		{ format = "entry-index %GT_U32", name = "entry_index", help = "Index in HW"},
		mandatory = {"devID", "stage", "entry_index"},
		requirements={
			["stage"] = {"devID"},
			["entry_index"] = {"stage"},
		},
	}
  }
})

local bobk_conformance_sign_entry_fields = {
	bucket0_sign = {0,0},
	bucket1_sign = {1,1},
};

local bobk_conformance_sign_values_names = {};

local bobk_conformance_sign_table_address = {
	["CPSS_DXCH_POLICER_STAGE_INGRESS_0_E"] = 0x0B400000,
	["CPSS_DXCH_POLICER_STAGE_INGRESS_1_E"] = 0x20400000,
	["CPSS_DXCH_POLICER_STAGE_EGRESS_E"   ] = 0x0E400000,
};

local function policy_bobk_conformance_sign_dump(params)
	local devices,j;
	local memo_base = bobk_conformance_sign_table_address[params.stage];

	if (params["devID"]=="all") then
		devices=wrLogWrapper("wrlDevList");
	else
		devices={params["devID"]};
	end
	for j=1,#devices do
		local data = generic_pp_memory_row_data_to_fields_values_named(
			devices[j], 0 --[[port_group--]],
			(memo_base + (params.entry_index * 4))--[[address--]],
			2 --[[num_of_words--]],
			bobk_conformance_sign_entry_fields,
			bobk_conformance_sign_values_names);
		print("dev " .. tostring(devices[j]));
		print(to_string(data));
	end
end

CLI_addCommand("debug", "dump bobk-policy-conformance-sign", {
   func = policy_bobk_conformance_sign_dump,
   help="dump policy-bobk-conformance-sign entry",
   params={
	{ type="named",
		{ format= "device %devID_all", name="devID", help="The device number" },
		{ format = "stage %policer_stage", name = "stage", help = "Policer stage"},
		{ format = "entry-index %GT_U32", name = "entry_index", help = "Index in HW"},
		mandatory = {"devID", "stage", "entry_index"},
		requirements={
			["stage"] = {"devID"},
			["entry_index"] = {"stage"},
		},
	}
  }
})





