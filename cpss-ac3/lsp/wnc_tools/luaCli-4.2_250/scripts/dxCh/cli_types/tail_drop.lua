--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* qos.lua
--*
--* DESCRIPTION:
--*       the file defines types needed for the qos commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************





CLI_type_dict["packet_limit"] = {
    checker = CLI_check_param_number,
    min=0,
    complete = CLI_complete_param_number,
    help="Maximal number of descriptors for a port"
}


CLI_type_dict["buffer_limit"] = {
    checker = CLI_check_param_number,
    min=0,
    complete = CLI_complete_param_number,
    help="Maximal number of buffers for a port"
}

CLI_type_dict["tc_dp_mask"] = {
    checker = CLI_check_param_number,
    min=0,
    complete = CLI_complete_param_number,
    help="tcDp mask value"
}

-- ************************************************************************
---
--  check_value_deviceid_all
--        @description  check's queue ID parameter or "all" that it is
--                      in the correct form
--
--        @param param              - parameter string
--        @param name               - parameter name string
--
--        @return        queue ID
--
-- ************************************************************************
local function check_value_queue_all(param,name,desc)
    if param == "all" then
        return true, param
    end
    return CLI_check_param_number(param,name,desc)
end


-- ************************************************************************
---
--  complete_value_queue_all
--        @description  autocompletes queue ID parameter
--
--        @param param              - parameter string
--        @param name               - parameter name string
--
--        @return        queue ID autocomplete
--
-- ************************************************************************
local function complete_value_queue_all(param,name,desc)
    local compl, help = CLI_complete_param_number(param,name,desc)
    if prefix_match(param, "all") then
        table.insert(compl,"all")
        help[#compl] = "Apply to all queue ID's"
    end
    return compl, help
end




CLI_type_dict["queue_id"] = {
    checker = check_value_queue_all,
    min=0,
	max=7,
    complete = complete_value_queue_all,
    help="Traffic Class associated with this set of Drop Parameters"
}


CLI_type_dict["drop_precedence"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="Drop Profile Parameters to associate with the Traffic Class",
    enum = {
        ["green"] = { value=0, help="green packets" },
        ["yellow"] = { value=1, help="yellow packets" },
        ["red"] = { value=2, help="red packets" },
        ["all"] = { value="all", help="all packets" }
   }
}

CLI_type_dict["queue_id_no_all"] = {
    checker = CLI_check_param_number,
    min=0,
	max=7,
    complete = CLI_complete_param_number,
    help="Traffic Class associated with this set of Drop Parameters"
}


CLI_type_dict["drop_precedence_no_all"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help="Drop Profile Parameters to associate with the Traffic Class",
    enum = {
        ["green"] = { value=0, help="green packets" },
        ["yellow"] = { value=1, help="yellow packets" },
        ["red"] = { value=2, help="red packets" },
   }
}

CLI_type_dict["tail_drop_alpha"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Alpha ratio",
    enum = {
        ["0.0"]  = {
            value = "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E",
            help = "alpha ratio 0.0"
        },
        ["0.125"] = {
            value = "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E",
            help = "alpha ratio 0.125"
        },
        ["0.25"] = {
            value = "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E",
            help = "alpha ratio 0.25"
        },
        ["0.5"]  = {
            value = "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E",
            help = "alpha ratio 0.5"
        },
        ["1.0"]  = {
            value = "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E",
            help = "alpha ratio 1.0"
        },
        ["2.0"]  = {
            value = "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E",
            help = "alpha ratio 2.0"
        },
        ["4.0"]  = {
            value = "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E",
            help = "alpha ratio 4.0"
        },
        ["8.0"]  = {
            value = "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E",
            help = "alpha ratio 8.0"
        }
    }
}
