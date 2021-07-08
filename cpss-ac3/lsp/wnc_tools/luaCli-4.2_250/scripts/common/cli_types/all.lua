require("common/cli_types/dev_port")
require("common/cli_types/devID")
require("common/cli_types/port_range")
require("common/cli_types/mac")
require("common/cli_types/hexstring")
require("common/cli_types/num_range")
require("common/cli_types/filename")
require("common/cli_types/ipv4")
require("common/cli_types/ipv6")
require("common/cli_types/types_led")
require("common/cli_types/log_enums")
require("common/cli_types/type_cpss_enable")
require("common/cli_types/trafficGeneratorTypes")
require("common/cli_types/hostname")
require("common/cli_types/eventTable_types")


CLI_type_dict["GT_U32"] = {
    checker = CLI_check_param_number,
    min=0,
    max=4294967295,
    help="32 bit unsigned integer value"
}

CLI_type_dict["GT_U16"] = {
    checker = CLI_check_param_number,
    min=0,
    max=65535,
    help="16 bit unsigned integer value"
}

CLI_type_dict["GT_U8"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    help="8 bit unsigned integer value"
}

CLI_type_dict["priority"] = {
    checker = CLI_check_param_number,
    min=0,
    max=7,
    help = "Enter priority 0..7"
}


CLI_type_dict["enable_disable"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Disable or enable\n",
    enum = {
        ["enable"] = { value=true, help="True" },
        ["disable"] = { value=false, help="False" }
   }
}

CLI_type_dict["dumpMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "HW access mode",
    enum = {
        ["read"] = {
            value = 0,
            help  = "read register access"
        },
        ["write"] = {
            value = 1,
            help  = "write register access"
        },
        ["both"] = {
            value = 2,
            help  = "both read and write register access"
        }
    }
}

CLI_type_dict["GT_SW_DEV_NUM"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    help="Device number"
}

-- ************************************************************************
---
--  complete_param_with_predefined_values
--        @description  complete parameters with a set of predefined values
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--        @param values         - array of predefined values to complete
--                                Example:
--                                {{"00:00:00:00:00:01",  "Unicast mac-address"}}
--
--        @return       complete and help lists
--
function complete_param_with_predefined_values(param, name, desc, values)
    local compl, help, index

    compl   = { }
    help    = { }

    for index = 1, #values do
        if prefix_match(param, tostring(values[index][1])) then
            table.insert(compl, tostring(values[index][1]))
            help[#compl] = values[index][2]
        end
    end

    return compl, help
end

--------------------------------------------
-- CPSS integer type registration
--------------------------------------------
do
  local inttypes = {"GT_SW_DEV_NUM"}
  local i, n
  for i, n in pairs(inttypes) do
    _G["mgmType_to_c_"..n] = mgmType_to_c_int
    _G["mgmType_to_lua_"..n] = mgmType_to_lua_int
  end
end
require_safe("common/cli_types/init_system_type")
if dxCh_family == true then
    require_safe_dx("cli_types/all")
end

if px_family == true then
    require_safe_px("cli_types/all")
end

