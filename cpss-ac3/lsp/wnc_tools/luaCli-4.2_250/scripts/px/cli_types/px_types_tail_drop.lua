--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* px_types_tail_drop.lua
--*
--* DESCRIPTION:
--*       The file defines types needed for the Tail Drop commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


--*******************************************************************************
-- check_value_deviceid_all
--
--  @description Check's queue ID parameter or "all" that it is in the correct
--               form
--
--  @param param - parameter string
--  @param name  - parameter name string
--
--  @return Queue ID
--
--*******************************************************************************
local function check_value_queue_all(param, name, desc)
    if (param == "all") then
        return true, param
    end
    return CLI_check_param_number(param, name, desc)
end


--*******************************************************************************
--  complete_value_queue_all
--
--  @description Autocompletes queue ID parameter
--
--  @param param - parameter string
--  @param name  - parameter name string
--
--  @return Queue ID autocomplete
--
--*******************************************************************************
local function complete_value_queue_all(param, name, desc)
    local compl, help = CLI_complete_param_number(param, name, desc)
    if (prefix_match(param, "all")) then
        table.insert(compl, "all")
        help[#compl] = "Apply to all queue ID's"
    end
    return compl, help
end



CLI_type_dict["tail_drop_packet_limit"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    help = "Maximal number of descriptors for the port"
}

CLI_type_dict["tail_drop_buffer_limit"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    help = "Maximal number of buffers for the port"
}

CLI_type_dict["tail_drop_dba_buffer_limit"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    help = "Amount of buffers available for dynamic allocation (DBA mode only)"
}

CLI_type_dict["tail_drop_traffic_class"] = {
    checker = check_value_queue_all,
    complete = complete_value_queue_all,
    min = 0,
    max = 7,
    help = "Traffic class associated with the set of drop parameters"
}

CLI_type_dict["tail_drop_drop_precedence"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Drop precedence",
    enum = {
        ["green"]  = { value = 0,     help = "green packets"  },
        ["yellow"] = { value = 1,     help = "yellow packets" },
        ["red"]    = { value = 2,     help = "red packets"    },
        ["all"]    = { value = "all", help = "all packets"    }
    }
}

CLI_type_dict["tail_drop_alpha"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Alpha ratio",
    enum = {
        ["0.0"]  = {
            value = "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E",
            help = "alpha ratio 0.0"
        },
        ["0.25"] = {
            value = "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E",
            help = "alpha ratio 0.25"
        },
        ["0.5"]  = {
            value = "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E",
            help = "alpha ratio 0.5"
        },
        ["1.0"]  = {
            value = "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E",
            help = "alpha ratio 1.0"
        },
        ["2.0"]  = {
            value = "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E",
            help = "alpha ratio 2.0"
        },
        ["4.0"]  = {
            value = "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E",
            help = "alpha ratio 4.0"
        }
    }
}

CLI_type_dict["tail_drop_tc_dp_mask"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    max = 20,
    help = "The number of LSBs masked for TC/DP limits"
}

CLI_type_dict["tail_drop_policy"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Tail Drop resources allocation policy",
    enum = {
        ["static"]  = {
            value = 0,
            help  = "Competing queues receive an equal amount of resources (fair mode)"
        },
        ["dynamic"] = {
            value = 1,
            help  = "Dynamic allocation of queueing resources (DBA mode)"
        }
    }
}
