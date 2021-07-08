--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_types_cos.lua
--*
--* DESCRIPTION:
--*       The file defines types needed for CoS attributes commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


-------------------------------------------------------
-- type registration: cos_map_l2_entry_index
-------------------------------------------------------
CLI_type_dict["cos_map_l2_entry_index"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 15,
    help     = "Port L2 CoS Mapping entry"
}

-------------------------------------------------------
-- type registration: cos_map_l3_entry_index
-------------------------------------------------------
CLI_type_dict["cos_map_l3_entry_index"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 63,
    help     = "L3 CoS Mapping entry"
}

-------------------------------------------------------
-- type registration: cos_map_mpls_entry_index
-------------------------------------------------------
CLI_type_dict["cos_map_mpls_entry_index"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 7,
    help     = "MPLS CoS Mapping entry"
}

-------------------------------------------------------
-- type registration: cos_map_dsa_entry_index
-------------------------------------------------------
CLI_type_dict["cos_map_dsa_entry_index"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 127,
    help     = "DSA CoS Mapping entry"
}

-------------------------------------------------------
-- type registration: cos_format_entry_packet_type
-------------------------------------------------------
CLI_type_dict["cos_format_entry_packet_type"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 31,
    help     = "Packet Type"
}

-------------------------------------------------------
-- type registration: cos_map_cos_type
-------------------------------------------------------
CLI_type_dict["cos_map_cos_type"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "CoS type",
    enum = {
        ["l2"]   = { value = "l2",   help = "Port L2 CoS Mapping entries" },
        ["l3"]   = { value = "l3",   help = "L3 CoS Mapping entries"      },
        ["mpls"] = { value = "mpls", help = "MPLS CoS Mapping entries"    },
        ["dsa"]  = { value = "dsa",  help = "DSA CoS Mapping entries"     }
    }
}

-------------------------------------------------------
-- type registration: cos_format_entry_byte_offset
-------------------------------------------------------
CLI_type_dict["cos_format_entry_byte_offset"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 7,
    help     = "CoS Byte Offset"
}

-------------------------------------------------------
-- type registration: cos_format_entry_bit_offset
-------------------------------------------------------
CLI_type_dict["cos_format_entry_bit_offset"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 7,
    help     = "CoS Bit Offset"
}

-------------------------------------------------------
-- type registration: cos_format_entry_num_of_bits
-------------------------------------------------------
CLI_type_dict["cos_format_entry_num_of_bits"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 7,
    help     = "Number of CoS bits to extract from the packet"
}

-------------------------------------------------------
-- type registration: cos_drop_eligibility_indicator
-------------------------------------------------------
CLI_type_dict["cos_drop_eligibility_indicator"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 1,
    help     = "Drop Eligibility Indicator"
}

-------------------------------------------------------
-- type registration: cos_user_priority
-------------------------------------------------------
CLI_type_dict["cos_user_priority"] =
{
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min      = 0,
    max      = 7,
    help     = "User Priority"
}

-------------------------------------------------------
-- type registration: cos_traffic_class
-------------------------------------------------------
CLI_type_dict["cos_traffic_class"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    max = 7,
    help = "Traffic Class"
}

-------------------------------------------------------
-- type registration: cos_drop_precedence
-------------------------------------------------------
CLI_type_dict["cos_drop_precedence"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Drop Precedence",
    enum = {
        ["green"]  = { value = 0,     help = "green packets"  },
        ["yellow"] = { value = 1,     help = "yellow packets" },
        ["red"]    = { value = 2,     help = "red packets"    },
    }
}
