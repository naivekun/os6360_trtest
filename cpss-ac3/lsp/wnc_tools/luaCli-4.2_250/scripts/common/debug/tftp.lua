--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tftp.lua
--*
--* DESCRIPTION:
--*       TFTP client for luaCLI
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--
-- The following commands are defined here
--
-- mode: all
--     tftp %server [%port]
-- mode: tftp
--     get %remotename [%localname]
--     put %localname [%remotename]

-- externs
cmdLuaCLI_registerCfunction("lua_tftp")
do

CLI_type_dict["udp_port"] = {
    help = "UDP port number",
    checker = CLI_check_param_number,
    min = 1,
    max = 65535
}

local tftp_server
local tftp_port
local compress_data
local function do_tftp(params)
    tftp_server = params.server.string
    tftp_port = 69
    if params.port ~= nil then
        tftp_port = params.port
    end
    compress_data = false
    CLI_prompts["tftp"] = ":tftp("..tftp_server..")#"
    CLI_change_mode_push("tftp")
    return true
end

-- tftp %server [%port]
CLI_addCommand("debug", "tftp", {
    help = "Enter TFTP client ",
    func = do_tftp,
    params = {
        { type="values",
            { format="%ipv4", name="server", help="TFTP server address" }
        },
        { type="named",
            { format="%udp_port", name="port", help="TFTP service port (default 69)" }
        }
    }
})

local function do_tftp_get(params)
    local cmd = "get"
    if compress_data then
        cmd = "get_z"
    end
    if params.localname == nil then
        params.localname = params.remotename
    end
    local rc, err = lua_tftp(cmd, tftp_server, tftp_port, 
            params.localname, params.remotename)
    if rc == 0 then
        return true
    end
    print("tftp operation hasn't been completed: "..err)
    return false
end
-- get %remotename [%localname]
CLI_addCommand("tftp", "get", {
    help = "Get file from server using TFTP protocol",
    func = do_tftp_get,
    params = {
        { type="values",
            { format="%string", name="remotename", help="remote file name" }
        },
        { type="named",
            { format="%string", name="localname", help="local file name" }
        }
    }
})

-- get-compressed %remotename [%localname]
CLI_addCommand("tftp", "get-compressed", {
    help = "Get file from server using TFTP protocol, compress local file",
    func = function(params)
        local c = compress_data
        compress_data = true
        local ret = do_tftp_get(params)
        compress_data = c
        return ret
    end,
    params = {
        { type="values",
            { format="%string", name="remotename", help="remote file name" }
        },
        { type="named",
            { format="%string", name="localname", help="local file name" }
        }
    }
})

local function do_tftp_put(params)
    if params.remotename == nil then
        params.remotename = params.localname
    end
    local rc, err = lua_tftp("put", tftp_server, tftp_port, 
            params.localname, params.remotename)
    if rc == 0 then
        return true
    end
    print("error: "..err)
    return false
end
-- put %localname [%remotename]
CLI_addCommand("tftp", "put", {
    help = "Put file to server using TFTP protocol",
    func = do_tftp_put,
    params = {
        { type="values",
            { format="%string", name="localname", help="local file name" }
        },
        { type="named",
            { format="%string", name="remotename", help="remote file name" }
        }
    }
})

end
