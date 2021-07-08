--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* running_config.lua
--*
--* DESCRIPTION:
--*       saving of the configuration file to ram or to a server
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--constants


--includes
require("common/running_config/TCP_support")
require("common/running_config/config_file_support")
if not cli_C_functions_registered("cmdCopyFromRAMFStoDisk") then
    cmdCopyFromRAMFStoDisk = function()
        return -1
    end
end


-- ************************************************************************
---
--  save_running_config
--        @description  saves the configuration made in this session to
--                      a file
--
--        @param params         - params["filename"]: the name of the file
--                              params["to"]: save the file to this ipv4 address
--                            params["port"]: the layer 4 port of the ip address
--
--        @return       true always since we must return a value,
--                      create_config_from_history handles the errors
--
local function save_running_config(params)
    local res,e
    if (create_config_from_cfgTbl(params["filename"])==true) then
        print("\nSaving config as file: \"".. params["filename"] .. "\"\n")
    
        if params["ip"]~=nil then

      if params["port"]==nil then
                params["port"]=69   --the default port
            end
            res,e = lua_tftp("put", params.ip["string"], params["port"], 
                params["filename"], params["filename"])

            if (res==0) then
                print("File sent successfuly")
            else
                print("Sending file failed : " .. e )
            end
        end
    if params["HDflag"] ~= nil then
      if (cmdCopyFromRAMFStoDisk(params["filename"]) == -1) then
        print("\nUnable to copy file from RAMFS to disk\n")
      else
        print("\nSaved config.txt in hard-drive \n")
      end
    end
    else
        print("\nUnable to save file\n")
    end

    return true  -- return true anyway since create_config_from_history handles the error
end


-- ************************************************************************
---
--  load_running_config
--        @description  loads the configuration from a file
--
--        @param params         - params[filename]: the name of the file;
--                                params[add]: if true adds the contents of the file to the history;
--                              params[silent]: if true only outputs error messages
--                  params["from"]: load the file from this ipv4 address
--                  params["port"]: the layer 4 port of the ip address
--
--        @return       true always since we must return a value,
--                      create_config_from_history handles the errors
--
function load_running_config(params)
    if not loadable_file_check(params) then
        return false
    end
  CLI_change_mode_pop_all()
    if (config_file_load(params["filename"],params)==true)  then
        print("\nConfig file \"".. params["filename"] .. "\" loaded\n")

    else
    CLI_change_mode_push("config")
    CLI_change_mode_push("running-config")
        print("\nUnable to load file\n")
    end

    return true  -- return true anyway since config_file_load handles the error
end

--------------------------------------------
-- command registration: load
--------------------------------------------
CLI_addCommand({"running-config", "exec"}, "load", {
    func=load_running_config,
    help="Load a configuration file from ram or from a server",
    params={
        { type="values", "%filename"},
        { type="named",
            { format="silent", help = "Only outputs errors"},
            { format="add",help = "Adds the loaded commands to history"},
            "#loadableFile"
        }
    }
})
--------------------------------------------
-- command registration: save
--------------------------------------------

CLI_addCommand("running-config", "save", {
    func=save_running_config,
    help="Save the configuration file to ram or to a server",
    params={
        { type="values", "%filename"},
        { type="named",
      { format="saveToHd", name="HDflag", help = "on simulation, creates config.txt file in CWD (optional)" }, 
            { format="to %ipv4", name="ip" ,help = "The ip to upload the file to (optional)"},
            { format="port %l4port",name="port", help = "The file server port" },
            requirements={port={"ip"}}
        }
    }
})


-- ************************************************************************
--
--  clearHistory
--        @description  Clears all saved history
--

local function clearHistory(params)

    cmdLuaCLI_history={}
end
--------------------------------------------
-- command registration: clear
--------------------------------------------

CLI_addHelp("debug", "clear", "Clear sub-commands")
CLI_addCommand("debug", "clear history", {
    func=clearHistory,
    help="Clears all user history",
})
