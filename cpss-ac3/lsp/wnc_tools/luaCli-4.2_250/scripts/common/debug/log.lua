--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* log.lua
--*
--* DESCRIPTION:
--*       enable/disable cPSS log
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

-- ************************************************************************
---
--  log_func
--        @description  enable the CPSS log
--
--        @param params The parameters
--
--        @return       true if there was no error otherwise false
--
local function enable_func(params)
    if (params.flagNo) then
        myGenWrapper("cpssLogEnableSet", {
                    { "IN", "GT_BOOL", "enable", false }
                })
    else
        myGenWrapper("cpssLogEnableSet", {
                    { "IN", "GT_BOOL", "enable", true }
                })
    end
end


local function lib_configuration_func(params)
    local status, err, result
    local defaultType = "CPSS_LOG_TYPE_ALL_E"    --default value
    local enable
    local currLib = params.libName

    if (params.typeName ~= nil) then
        defaultType = params.typeName
    end

    myGenWrapper("cpssLogEnableSet", {
                { "IN", "GT_BOOL", "enable", true }
            })

    if (params.flagNo) then
        enable = false
    else
        enable = true
    end

    result = myGenWrapper("cpssLogLibEnableSet", {
                { "IN", "CPSS_LOG_LIB_ENT", "lib", params.libName },
                { "IN", "CPSS_LOG_TYPE_ENT", "type", defaultType },
                { "IN", "GT_BOOL", "enable", enable }
            })

    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end

    return status, err
end


local function format_func(params)
    local defaultType = "CPSS_LOG_API_FORMAT_NO_PARAMS_E"    --default value

    if (params.flagNo) then
        myGenWrapper("cpssLogApiFormatSet", {
                    { "IN", "CPSS_LOG_API_FORMAT_ENT", "format", defaultType }
                })
    else
        myGenWrapper("cpssLogApiFormatSet", {
                    { "IN", "CPSS_LOG_API_FORMAT_ENT", "enable", params.formatName }
                })
    end
end


local function convert_lib(n)
    if n==0 then
        return "CPSS_LOG_LIB_APP_DRIVER_CALL_E"
    elseif n==1 then
        return "CPSS_LOG_LIB_BRIDGE_E"
    elseif n==2 then
        return "CPSS_LOG_LIB_CNC_E"
    elseif n==3 then
        return "CPSS_LOG_LIB_CONFIG_E"
    elseif n==4 then
        return "CPSS_LOG_LIB_COS_E"
    elseif n==5 then
        return "CPSS_LOG_LIB_HW_INIT_E"
    elseif n==6 then
        return "CPSS_LOG_LIB_CSCD_E"
    elseif n==7 then
        return "CPSS_LOG_LIB_CUT_THROUGH_E"
    elseif n==8 then
        return "CPSS_LOG_LIB_DIAG_E"
    elseif n==9 then
        return "CPSS_LOG_LIB_FABRIC_E"
    elseif n==10 then
        return "CPSS_LOG_LIB_IP_E"
    elseif n==11 then
        return "CPSS_LOG_LIB_IPFIX_E"
    elseif n==12 then
        return "CPSS_LOG_LIB_IP_LPM_E"
    elseif n==13 then
        return "CPSS_LOG_LIB_L2_MLL_E"
    elseif n==14 then
        return "CPSS_LOG_LIB_LOGICAL_TARGET_E"
    elseif n==15 then
        return "CPSS_LOG_LIB_LPM_E"
    elseif n==16 then
        return "CPSS_LOG_LIB_MIRROR_E"
    elseif n==17 then
        return "CPSS_LOG_LIB_MULTI_PORT_GROUP_E"
    elseif n==18 then
        return "CPSS_LOG_LIB_NETWORK_IF_E"
    elseif n==19 then
        return "CPSS_LOG_LIB_NST_E"
    elseif n==20 then
        return "CPSS_LOG_LIB_OAM_E"
    elseif n==21 then
        return "CPSS_LOG_LIB_PCL_E"
    elseif n==22 then
        return "CPSS_LOG_LIB_PHY_E"
    elseif n==23 then
        return "CPSS_LOG_LIB_POLICER_E"
    elseif n==24 then
        return "CPSS_LOG_LIB_PORT_E"
    elseif n==25 then
        return "CPSS_LOG_LIB_PROTECTION_E"
    elseif n==26 then
        return "CPSS_LOG_LIB_PTP_E"
    elseif n==27 then
        return "CPSS_LOG_LIB_SYSTEM_RECOVERY_E"
    elseif n==28 then
        return "CPSS_LOG_LIB_TCAM_E"
    elseif n==29 then
        return "CPSS_LOG_LIB_TM_GLUE_E"
    elseif n==30 then
        return "CPSS_LOG_LIB_TRUNK_E"
    elseif n==31 then
        return "CPSS_LOG_LIB_TTI_E"
    elseif n==32 then
        return "CPSS_LOG_LIB_TUNNEL_E"
    elseif n==33 then
        return "CPSS_LOG_LIB_VNT_E"
    elseif n==34 then
        return "CPSS_LOG_LIB_RESOURCE_MANAGER_E"
    elseif n==35 then
        return "CPSS_LOG_LIB_VERSION_E"
    elseif n==36 then
        return "CPSS_LOG_LIB_TM_E"
    elseif n==37 then
        return "CPSS_LOG_LIB_SMI_E"
    elseif n==38 then
        return "CPSS_LOG_LIB_INIT_E"
    elseif n==39 then
        return "CPSS_LOG_LIB_DRAGONITE_E"
    elseif n==40 then
        return "CPSS_LOG_LIB_VIRTUAL_TCAM_E"
    elseif n==41 then
        return "CPSS_LOG_LIB_INGRESS_E"
    elseif n==42 then
        return "CPSS_LOG_LIB_EGRESS_E"

--------- !!! MUST BE LAST !!! ---------
    else
        return "CPSS_LOG_LIB_ALL_E"
    end
end


local function convert_type(n)
    if n==0 then
        return "CPSS_LOG_TYPE_INFO_E"
    elseif n==1 then
        return "CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E"
    elseif n==2 then
        return "CPSS_LOG_TYPE_NON_ENTRY_LEVEL_FUNCTION_E"
    elseif n==3 then
        return "CPSS_LOG_TYPE_DRIVER_E"
    elseif n==4 then
        return "CPSS_LOG_TYPE_ERROR_E"
    elseif n==5 then
        return "CPSS_LOG_TYPE_ALL_E"
    end
end


local function show_status_func(params)
    local result, index, values, j, x, y, z

    result = myGenWrapper("cpssLogEnableGet", {})
    if (result ~= 0) then
        print ("Global status:  enabled")
    else
        print ("Global status:  disabled")
    end

    result = myGenWrapper("cpssLogApiFormatGet", {})
    if (result == 0) then
        print ("Format:         no-params")
    elseif (result == 1) then
        print ("Format:         all-params")
    elseif (result == 2) then
        print ("Format:         non-zero-params")
    end

    result = myGenWrapper("cpssLogStackSizeGet", {})
    print("stack size:     " .. result .. "\n")

    print ("Lib enabled status:")
    x = "Lib                         info       entry-level  non-entry-level     driver         error\n" ..
        "----------------          --------     -----------  ---------------    --------       --------"
    print (x)

    i = 0
    while (true) do
        x = convert_lib(i)
        if (x == "CPSS_LOG_LIB_ALL_E") then
            break
        end

        x = string.sub(x, 14, -3)
        y = string.len(x)

        if (y < 26) then
            for z = 1, (26 - y), 1 do
                x = x .. " "
            end
        end

        for j = 0, 4, 1 do
            result, values = myGenWrapper("cpssLogLibEnableGet", {
                        { "IN", "CPSS_LOG_LIB_ENT", "lib", convert_lib(i) },
                        { "IN", "CPSS_LOG_TYPE_ENT", "type", convert_type(j) },
                        { "OUT", "GT_BOOL", "enablePtr" }
                    })

            if (values["enablePtr"]) then
                x = x .. "enabled "
            else
                x = x .. "disabled"
            end

            x = x .. "       "
        end

        print(x)

        i = i + 1
    end
end


local function stack_size_func(params)
    local result, status, err, size

    if (params.flagNo) then
        size = tonumber(params.stackSize)
    else
        size = 1
    end

    result = myGenWrapper("cpssLogStackSizeSet", {
                { "IN", "GT_U32", "size", size }
            })
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end

    return status, err
end


--------------------------------------------------------------------
-- command registration: trace cpss-log/no trace cpss-log
--------------------------------------------------------------------
CLI_addHelp("debug", "cpss-log", "enable CPSS LOG")
CLI_addCommand("debug", "trace cpss-log", {
    func   = enable_func,
    help   = "enable CPSS LOG",
    params = {}
})

CLI_addCommand("debug", "no trace cpss-log", {
    func   = function(params)
                 params.flagNo = true
                 return enable_func(params)
             end,
    help   = "disable CPSS log",
    params = {}
})

----------------------------------------------------------------------
-- command registration: trace cpss-log lib /no trace cpss-log lib
----------------------------------------------------------------------
CLI_addCommand("debug", "trace cpss-log lib", {
    func   = lib_configuration_func,
    help   = "enable logging of specific info for specific or all libs",
    params = {
        {
            type = "values",
            {
                format = "%log_lib",
                name   = "libName"
            }
        },
        {
            type = "named",
            {
                format = "log-type %log_type",
                name   = "typeName"
            },
            mandatory = { "libName" }
        }
    }
})

CLI_addCommand("debug", "no trace cpss-log lib", {
    func   = function(params)
                 params.flagNo = true
                 return lib_configuration_func(params)
             end,
    help   = "disable logging of specific info for specific or all libs",
    params = {
        {
            type = "values",
            {
                format = "%log_lib",
                name   = "libName"
            }
        },
        {
            type = "named",
            {
                format = "log-type %log_type",
                name   = "typeName"
            },
            mandatory = { "libName" }
        }
    }
})

-------------------------------------------------------------------------
-- command registration: trace cpss-log format /no trace cpss-log format
-------------------------------------------------------------------------
CLI_addCommand("debug", "trace cpss-log format", {
    func   = format_func,
    help   = "configure format of CPSS log",
    params = {
        {
            type = "named",
            {
                format = "%log_format",
                name   = "formatName"
            }
        }
    }
})

CLI_addCommand("debug", "no trace cpss-log format", {
    func   = function(params)
                 params.flagNo = true
                 return format_func(params)
             end,
    help   = "set CPSS log format - no params",
    params = {
        {
            type = "named",
            {
                format = "%log_format",
                name   = "formatName"
            }
        }
    }
})

--------------------------------------------------------------------
-- command registration: trace cpss-log/no trace cpss-log
--------------------------------------------------------------------
CLI_addHelp("debug", "show cpss-log status", "Show the CPSS log status")
CLI_addCommand("debug", "show cpss-log status", {
    func   = show_status_func,
    help   = "show the CPSS log status including per-lib configuration",
    params = {}
})

--------------------------------------------------------------------
-- command registration: trace cpss-log stack size
--------------------------------------------------------------------
CLI_addHelp("debug", "trace cpss-log stack size", "Set the CPSS log stack size")
CLI_addCommand("debug", "trace cpss-log stack size", {
    func   = function(params)
                 params.flagNo = true
                 return stack_size_func(params)
             end,
    help   = "Set the CPSS log stack size",
    params = {
        {
            type = "named",
            {
                format = "%GT_U32",
                name   = "stackSize"
            }
        }
    }
})
