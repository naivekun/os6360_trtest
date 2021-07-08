--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* serdesConf.lua
--*
--* DESCRIPTION:
--*       Serdes configuration commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************


-- ************************************************************************
--  prvGetNextLine
--        @description  local function which retruns the next line of a file edited
--
--        @param fd         - The file descriptor
--
--        @return       true and the next string if there was no error otherwise false and error message
--

local function prvGetNextLine(fd)
    local str=""
    local e

    while (str~=nil) do
        str, e = fs.gets(fd)
        if str == nil then
            if e == "<EOF>" then  return false,nil  end
            return false,"error reading file: " .. e
        end
            -- remove trailing \n and \r and spaces
        local f = true
        while f do
            f = false
            local c = string.sub(str, -1)
            if ((c == "\n") or (c == "\r") or (c == "\t") or (c == " " )) then f = true end
                if f then   str = string.sub(str, 1, -2)    end
        end

        -- remove heading spaces
        local f = true
        while f do
            f = false
            local c = string.sub(str, 1, 1)
            if ((c == " ") or (c == "\t")) then f = true end
            if f then   str = string.sub(str, 2)  end
        end

        -- skip empty strings and comments
        if str ~= "" and string.sub(str, 1, 2) ~= "--" then  return true,str  end
    end
end

--*******************************************************************************
--  traceDump
--
--  @description Prints the HW trace dump for specified HW access mode
--
--  @param params[dumpMode] - HW access mode.
--
--  @return true on success, false on fail
--
--*******************************************************************************
local function traceDump(params)
    local ret, mode

    if (params["dumpMode"] == nil) then
        mode = 1
    else
        mode = tonumber(params["dumpMode"])
    end

    ret = myGenWrapper("appDemoTraceHwAccessDbDump", {
                { "IN", TYPE["ENUM"], "accessType", mode }
            })

    if (ret ~= LGT_OK) then
        print("Could not print trace, status is: " .. returnCodes[ret])
        return false
    else
        return true
    end
end


--*******************************************************************************
--  traceClear
--
--  @description Clear hw access data base
--
--  @return true on success, false on fail
--
--*******************************************************************************
local function traceClear(params)
    local ret

    ret = myGenWrapper("appDemoTraceHwAccessClearDb", {})

    if (ret ~= LGT_OK) then
        print("Could not clear HW access DB, status is: " .. returnCodes[ret])
        return false
    else
        return true
    end
end


--*******************************************************************************
--  traceAccessModeSet
--
--  @description Enables / disables HW write tracing
--
--  @param params -
--          params[devID]    - device number.
--          params[flagNo]   - true disables tracing, false enables tracing.
--          params[dumpMode] - HW access mode.
--
--  @return true on success, false on fail
--
--  Note:
--      If the command used before cpssInitSystem the default params[devID] == 0
--
--*******************************************************************************
local function traceAccessModeSet(params)
    local ret, dev, devices, i, enable, mode, tmpMode

    if (params["devID"] == nil) then
        params["devID"] = "all"
    end

    if (params["flagNo"] == true) then
        enable = false
    else
        enable = true
    end

    if (params["dumpMode"] == nil) then
        mode = 1
    else
        mode = tonumber(params["dumpMode"])
    end


    devices = wrLogWrapper("wrlDevList")

    if (#devices == 0) then
        if (params["devID"] == "all") then
            devices = { 0 }
        else
            devices = { tonumber(params["devID"]) }
        end
    elseif (params["devID"] ~= "all") then
        local device_found = false
        for i = 1, #devices do
            if (tonumber(params["devID"]) == devices[i]) then
                devices = {
                    tonumber(params["devID"])
                }
                device_found = true
                break
            end
        end

        if (device_found == false) then
            print("The device number does not exist")
            return false
        end
    end

    -- output access mode is db
    ret = myGenWrapper("appDemoTraceHwAccessOutputModeSet", {
                { "IN", TYPE["ENUM"], "mode", 2 }
            })

    -- Bind 'delayed-write' to 'write' operation when calling hw-trace API
    -- from lua, as lua uses the API in a limited way
    if mode==2 then
        mode = 4
    end

    for i = 1, #devices do
        -- accessType: 0-read, 1-write, 2-both, 3-delayed write, 4-all
        ret = myGenWrapper("appDemoTraceHwAccessEnable", {
                    { "IN", "GT_U8", "devNum", devices[i] },
                    { "IN", TYPE["ENUM"], "accessType", mode },
                    { "IN", "GT_BOOL", "enable", enable }
                })

        if (ret ~= LGT_OK) then
            print("Could not perform trace action, status is: " .. returnCodes[ret])
            return false
        end

        -- Bind 'delayed-write' to 'write' operation when calling hw-trace API
        -- from lua, as lua uses the API in a limited way
        if mode==1 then
            tmpMode = 3
            ret = myGenWrapper("appDemoTraceHwAccessEnable", {
                    { "IN", "GT_U8", "devNum", devices[i] },
                    { "IN", TYPE["ENUM"], "accessType", tmpMode },
                    { "IN", "GT_BOOL", "enable", enable }
                })
            if (ret ~= LGT_OK) then
                print("Could not perform trace action, status is: " .. returnCodes[ret])
                return false
            end
        end

    end

    return true
end


---------------------------------------------------------------------------------
-- trace help
---------------------------------------------------------------------------------
CLI_addHelp("debug", "trace", "Trace sub-commands")
CLI_addHelp("debug", "no trace", "No trace sub-commands")


---------------------------------------------------------------------------------
-- command registration: trace hw-dump
---------------------------------------------------------------------------------
CLI_addCommand("debug", "trace hw-dump", {
    func = traceDump,
    help = "Dump HW access DB to stdout",
    params = {
        {
            type = "named",
            {
                format = "mode %dumpMode",
                name   = "dumpMode",
                help   = "HW access mode (default: write)"
            }
        }
    }
})

---------------------------------------------------------------------------------
-- command registration: trace clear
---------------------------------------------------------------------------------
CLI_addCommand("debug", "trace clear", {
    func = traceClear,
    help = "Clear hardware access data base"
})

---------------------------------------------------------------------------------
-- command registration: trace hw-access
---------------------------------------------------------------------------------
CLI_addCommand("debug", "trace hw-access", {
    func = traceAccessModeSet,
    help = "Log HW access",
    params = {
        {
            type = "named",
            {
                format = "device %devID_all",
                name   = "devID",
                help   = "The device number"
            },
            {
                format = "mode %dumpMode",
                name   = "dumpMode",
                help   = "HW access mode (default: write)"
            }
        }
    }
})

---------------------------------------------------------------------------------
-- command registration: no trace hw-access
---------------------------------------------------------------------------------
CLI_addCommand("debug", "no trace hw-access", {
    func = function(params)
               params.flagNo = true
               return traceAccessModeSet(params)
           end,
    help = "Disable tracing of HW access",
    params = {
        {
            type = "named",
            {
                format = "device %devID_all",
                name   = "devID",
                help   = "The device number"
            },
            {
                format = "mode %dumpMode",
                name   = "dumpMode",
                help   = "HW access mode (default: write)"
            }
        }
    }
})

-- ************************************************************************
--  loadZarlinkConfig
--        @description   - load zarlink config file via tftp or CWD.
--
--        @param params  - params[filename] - The name of the file
--                       - params[ip]       - The ip of the server which holds the file (optional)
--                       - params[port]     - The port of the server which holds the file (default is 12346)
--
--        @return       true if there was no error otherwise false, error description
--

local function loadZarlinkConfig(params)
    local fd, e, result
    local offs, data
    local line, lineNum, m

    local slvDev = params.devID

    -- upload file
    params.source = "ip"

    if not loadable_file_check(params) then
        return false
    end

    fd, e = fs.open(params.filename, "r")

    if fd == nil then
        print("Failed to open file " .. params.filename .. ": " .. e)
        return false
    end

    printMsg("Load Zarlink configuration " .. params.filename)

    offs = 0
    lineNum = 0
    line, e = fs.gets(fd)
    while (line ~= nil) do

      lineNum = lineNum + 1

      if not prefix_match("//", line) then

        m = string.match(line, "%x+", 3)

        if (not prefix_match("0x", line)) or (m == nil) then
          fs.close(fd)
          printErr("File format error in line # " .. tostring(lineNum) .. " : " .. line)
          return false
        end

        data = tonumber(m, 16)

        result = myGenWrapper("appDemoZarlinkWriteByte", {
                                { "IN", "GT_U32", "devSlvId", slvDev       },
                                { "IN", "GT_U32", "offset",   offs         },
                                { "IN", "GT_U32", "data",     data         }  })

        if result ~= 0 then
          printErr("Error: appDemoZarlinkWriteByte() retCode=" .. tostring(result) .. " offs=" .. tostring(offs) .. "  data=" .. tostring(data))
          fs.close(fd)
          return false
        end
        offs = offs + 1
        myGenWrapper("osTimerWkAfter",{{"IN","GT_U32","mils", 50}})
      end
      line, e = fs.gets(fd)
    end

    fs.close(fd)

    --if an error has occurred
    if e ~= "<EOF>" then
      printErr("Error reading file: " .. e)
      return false
    end

    printMsg("Config file <".. params.filename .."> uploaded.")
    return true
end

-------------------------------------------------------
-- command registration: load zarlink
-------------------------------------------------------
CLI_addCommand("debug", "load zarlink", {
    func=loadZarlinkConfig,
    help="Load Zarlink configuration file via tftp",
    params={
        { type="values",
            { format="%devSlvID",     name = "devID",     help = "Slave device number"},
            { format="%filename",     name = "filename",  help = "Zarlink configuaration file name"},
            requirements={filename={"devID"}},
            mandatory={"devID", "filename"}
        },
        { type="named",
            { format="from %ipv4",    name = "ip" ,       help = "The ip to download the file from"},
            { format="port %l4port",  name = "port",      help = "The file server port" },
            requirements={port={"ip"}},
            mandatory={"ip"}
        }
    }
})

-- ************************************************************************
--  executeEepromFile
--        @description  executes a file from the RAMFS or another source.
--        the execution ignores comments in file (marked with --)
--
--        @param params         - params[filename] - The name of the file
--                - params[ip] - The ip of the server which holds the file (optional)
--                - params[port] - the port of the server which holds the file (optional, default is 12346)
--
--        @return       true if there was no error otherwise false, error description
--

local function executeEepromFile(params)
    local fd, e,str,s,state,lineNum,result,delayStart,delayEnd,delayTime,all_devices,i,lineType,writeParams

    if not loadable_file_check(params) then
        return false
    end
    fd = fs.open(params.filename, "r") --load file

    lineNum=0
    state,str=prvGetNextLine(fd)
    while (state==true) do --read file
        lineNum=lineNum+1

        writeParams={}

        --If the line is of format  "(dev) (portgroup) (Address)  (Mask) (Value) (delay)"
        s,e,writeParams["dev"],writeParams["portgroup"],writeParams["Address"],writeParams["Mask"],writeParams["Value"],writeParams["delay"]=string.find(str,"(.-)%s+(.-)%s+(.-)%s+(.-)%s+(.-)%s+(.+)")
        if (s==1) and (e==string.len(str)) and (tonumber(writeParams["dev"]) ~=nil) and (tonumber(writeParams["portgroup"])~=nil) and (tonumber(writeParams["Address"])~=nil) and (tonumber(writeParams["Mask"])~=nil) and (tonumber(writeParams["Value"])~=nil) and (tonumber(writeParams["delay"])~=nil) then
            lineType="Device and Portgroup aware"
        else
            --If the line is of format  "(portgroup) (Address)  (Mask) (Value) (delay)"
            s,e,writeParams["portgroup"],writeParams["Address"],writeParams["Mask"],writeParams["Value"],writeParams["delay"]=string.find(str,"(.-)%s+(.-)%s+(.-)%s+(.-)%s+(.+)")
            if (s==1) and (e==string.len(str)) and (tonumber(writeParams["portgroup"])~=nil) and (tonumber(writeParams["Address"])~=nil) and (tonumber(writeParams["Mask"])~=nil) and (tonumber(writeParams["Value"])~=nil) and (tonumber(writeParams["delay"])~=nil) then
                lineType="Portgroup aware"
            else
                --If the line is of format  "(Address)  (Mask) (Value) (delay)"
                s,e,writeParams["Address"],writeParams["Mask"],writeParams["Value"],writeParams["delay"]=string.find(str,"(.-)%s+(.-)%s+(.-)%s+(.+)")
                if (s==1) and (e==string.len(str)) and (tonumber(writeParams["Address"])~=nil) and (tonumber(writeParams["Mask"])~=nil) and (tonumber(writeParams["Value"])~=nil) and (tonumber(writeParams["delay"])~=nil) and (string.find(writeParams["delay"],"0x") == nil) then
                    lineType="Portgroup unaware"
                else
                    -- If the line is of format  "wrd (dev) (portgroup) (Address) (Value) [DFX]"
                    -- then we have a deal with MicroInit format
                    writeParams = {}
                    -- expression for writeParams["DFX"] defined as zero or more and will be empty string if not found
                    s,e,writeParams["dev"],writeParams["portgroup"],writeParams["Address"],writeParams["Value"],writeParams["Mask"],writeParams["DFX"]=string.find(str,"wrd %s-(%d+)%s+(%d+)%s+([^ ]+)%s+([^ ]+)%s+([^ ]+)%s-([^ ]-)$")
                    if s == 1 and (e==string.len(str)) and (string.find(writeParams["Value"],"0x") ~= nil) and
                            (tonumber(writeParams["dev"])~=nil) and (tonumber(writeParams["portgroup"])~=nil) and
                            (tonumber(writeParams["Address"])~=nil) and (tonumber(writeParams["Value"])~=nil) and
                            (tonumber(writeParams["Mask"])~=nil) then
                        lineType="MicroInit format"
                        writeParams["delay"]="0"
                    else
                        writeParams = {}
                        -- expression for writeParams["DFX"] defined as zero or more and will be empty string if not found
                        s,e,writeParams["dev"],writeParams["portgroup"],writeParams["Address"],writeParams["Value"],writeParams["DFX"]=string.find(str,"wrd %s-(%d+)%s+(%d+)%s+([^ ]+)%s+([^ ]+)%s-([^ ]-)$")
                        if s == 1 and (e==string.len(str)) and (string.find(writeParams["Value"],"0x") ~= nil) and
                                (tonumber(writeParams["dev"])~=nil) and (tonumber(writeParams["portgroup"])~=nil) and
                                (tonumber(writeParams["Address"])~=nil) and (tonumber(writeParams["Value"])~=nil) then
                            lineType="MicroInit format"
                            writeParams["Mask"]=0xFFFFFFFF
                            writeParams["delay"]="0"
                        else

                            lineType = nil
                        end
                    end
                end
            end
        end

        if writeParams["dev"]==nil then
            if all_devices == nil then all_devices = wrLogWrapper("wrlDevList") end
            writeParams["dev"]=all_devices
        else
            writeParams["dev"]={writeParams["dev"]}
        end

        if writeParams["portgroup"]==nil then
            writeParams["portgroup"]=0xFFFFFFFF  -- portgroup unaware
        end

        if lineType~=nil then
            for i=1, #writeParams["dev"] do

            --print("Dev:".. writeParams["dev"][i].." \tPortgroup:"..writeParams["portgroup"]..  " \tAddress:"..writeParams["Address"].." \tMask:"..writeParams["Mask"].." \tVal:"..tonumber(writeParams["Value"]).." \tDelay:"..tonumber(writeParams["delay"]))
                if lineType == "MicroInit format" then
                    if string.upper(writeParams["DFX"]) ~= "DFX" then 
                        result = myGenWrapper(
                            "cpssDrvPpHwRegBitMaskWrite", {
                            { "IN", "GT_U8",  "devNum", tonumber(writeParams["dev"][i]) },
                            { "IN", "GT_U32", "portGroupId", tonumber(writeParams["portgroup"]) },
                            { "IN", "GT_U32", "regAddr", tonumber(writeParams["Address"]) },
                            { "IN", "GT_U32", "mask", tonumber(writeParams["Mask"]) },
                            { "IN","GT_U32", "regValue", tonumber(writeParams["Value"]) }
                        })
                    else
                        result = myGenWrapper(
                            "cpssDrvHwPpResetAndInitControllerWriteReg", {
                            { "IN",     "GT_U8",        "devNum", tonumber(writeParams["dev"][i]) },
                            { "IN",     "GT_U32",       "regAddr", tonumber(writeParams["Address"]) },
                            { "IN",     "GT_U32",       "data", tonumber(writeParams["Value"])}
                        })
                    end
                else
                    if tonumber(writeParams["Address"]) < 0xF8000 or tonumber(writeParams["Address"]) > 0xF8FFF then 
                        result = myGenWrapper(
                            "cpssDrvPpHwRegBitMaskWrite", {
                            { "IN", "GT_U8",  "devNum", tonumber(writeParams["dev"][i]) },
                            { "IN", "GT_U32", "portGroupId", tonumber(writeParams["portgroup"]) },
                            { "IN", "GT_U32", "regAddr", tonumber(writeParams["Address"]) },
                            { "IN", "GT_U32", "mask", tonumber(writeParams["Mask"]) },
                            { "IN","GT_U32", "regValue", tonumber(writeParams["Value"]) }
                        })
                    else
                        result = myGenWrapper(
                            "cpssDrvHwPpResetAndInitControllerWriteReg", {
                            { "IN",     "GT_U8",        "devNum", tonumber(writeParams["dev"][i]) },
                            { "IN",     "GT_U32",       "regAddr", tonumber(writeParams["Address"]) },
                            { "IN",     "GT_U32",       "data", tonumber(writeParams["Value"])}
                        })
                    end

                    if result ~= 0 then print(string.format("Line %d : Failed to write device %d portgroup %d register addr=0x%X, err=%d",lineNum,tonumber(writeParams["dev"][i])  , tonumber(writeParams["portgroup"]) , tonumber(writeParams["Address"]), result))
                    elseif tonumber(writeParams["delay"])>0 then luaTaskDelay(tonumber(writeParams["delay"])) end
                end
            end
        -- If the line is a lua command
        else
            delayStart,delayEnd,delayTime = string.find(string.lower(str),"delay%s+(%d+)")
            if ((delayStart==1) and (delayEnd==string.len(str)) and (delayTime~=nil)) then
                luaTaskDelay(delayTime)  --in milliseconds
            else
                cmdLuaCLI_execute(str)
            end
        end
        state,str=prvGetNextLine(fd)
    end

    if (str~=nil) then print (str) end  --if an error has occured
    fs.close(fd)

    return true
end



-------------------------------------------------------
-- command help: debug load
-------------------------------------------------------
CLI_addHelp("debug", "load", "Load commands from file")

-------------------------------------------------------
-- command registration: execute eeprom
-------------------------------------------------------
CLI_addCommand("debug", "load eeprom", {
    func=executeEepromFile,
    help="Load Eeprom register file",
    params={
        { type="values", "%filename"},
        { type="named",
            "#loadableFile"
        }
    }
})

