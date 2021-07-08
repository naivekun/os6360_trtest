--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* cpss_enable.lua
--*
--* DESCRIPTION:
--*       Changing of the CPSS debug printing status
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************

-- register wrappers
cli_C_functions_registered("wrlCpssGetStartupDirectory")


-- log filtering mode. By default setters only
loggingSetOnly = false

-- file descriptor to logging cpss API
traceCpssAPIfd = nil

-- logging lua cpss wrappers
myCPSS_API_Debug = false

-- logging in C-style
myCPSS_API_log_in_C_Style = true


--*******************************************************************************
--  cpss_enable
--
--  @description Enable/disable the CPSS command printing
--
--  @param params -
--              params["flagNo"]         - no command property
--              params["cpssApiLogMode"] - logging mode
--              params["luaFileName"]    - convert lua script to C code file
--              params["fileName"]       - file name in local FS to store output
--                                         trace to
--              params["ip"]             - IP to upload file to
--              params["port"]           - file server port
--
--  @return true on success, false on fail
--
--*******************************************************************************
local function traceCpssApi(params)
    -- stop logging
    if (params["flagNo"] == true) then
        myCPSS_API_Debug = false
        trace.stop()
        trace.setOnScreen(false)

        -- return to previous state for examples
        trace.setLogVerboseMode(trace.getLogVerboseMode())

        -- close log file
        if (traceCpssAPIfd ~= nil) then
            fs.close(traceCpssAPIfd)
            traceCpssAPIfd = nil
        end

        return true
    end


    if (params["traceMode"] ~= nil) then
        -- lua to c converter
        if (params["traceMode"] == "luaFileName") then
            local luaSource = params["luaFileName"]

            -- stop trace
            myCPSS_API_Debug = false
            trace.stop()

            -- close log file
            if (traceCpssAPIfd ~= nil) then
                fs.close(traceCpssAPIfd)
                traceCpssAPIfd = nil
            end

            print("Warning: trace cpss-api stopped.")

            -- remove traceCpssApi.tmp in RAMFS
            fs.unlink("traceCpssApi.tmp")

            print("Converting Lua script " .. tostring(luaSource) ..  " to C-source \n")

            return lua2c(luaSource, "traceCpssApi.tmp")
        end

        -- check if save command
        if (params["fileName"] ~= nil) then
            local traceInProgress = false
            local oldmyCPSS_API_Debug = myCPSS_API_Debug

            -- check if trace in progress (file is open)
            if (traceCpssAPIfd ~= nil) then
                traceInProgress = true

                -- stop trace
                myCPSS_API_Debug = false
                trace.stop()

                -- close log file
                if (traceCpssAPIfd ~= nil) then
                    fs.close(traceCpssAPIfd)
                    traceCpssAPIfd = nil
                end

                print("Warning: trace cpss-api stopped.\n")
            end

            if (not fs.fileexists("traceCpssApi.tmp")) then
                print("Error: The cpss-api logFile not found. Possible trace not started.")
                return false
            end

            -- check destination for log
            if (params["ip"] ~= nil) then
                -- send file over tftp
                if (params["port"] == nil) then
                    params["port"] = 69   --the default port
                end

                local ret, e
                ret, e = lua_tftp("put", params.ip["string"], params["port"],
                                  "traceCpssApi.tmp", params["save"])

                if (ret == 0) then
                    print("The logFile " .. params["save"] .. "sent successfully")
                else
                    print("Error: Sending file failed : " .. to_string(e))
                    return false
                end
            else
                -- store file only if simulation
                if (wrLogWrapper("wrlCpssIsAsicSimulation")) then
                    -- delete previous version of traceCpssApi.tmp (if exists)
                    os.remove(wrlCpssGetStartupDirectory() .. "traceCpssApi.tmp")

                    if (cmdCopyFromRAMFStoDisk("traceCpssApi.tmp") == -1) then
                        print("Error: Unable to save file traceCpssApi.tmp to disk\n")
                        return false
                    end

                    -- delete previous version of fileName (if exists)
                    os.remove(wrlCpssGetStartupDirectory() .. params["fileName"])

                    -- rename traceCpssApi.tmp in CWD
                    os.rename (wrlCpssGetStartupDirectory() .. "traceCpssApi.tmp",
                               wrlCpssGetStartupDirectory() .. params["fileName"])
                else
                    print("Error: Cannot store file on board")
                    return false
                end
            end

            -- remove traceCpssApi.tmp in RAMFS
            fs.unlink("traceCpssApi.tmp")
            print("The logFile " .. params["fileName"] .. " succesfully saved in CWD")

            -- restart logging if was in progress
            if (traceInProgress == true) then
                -- create new log
                traceCpssAPIfd, e = fs.open("traceCpssApi.tmp", "w")
                if (traceCpssAPIfd == nil) then
                    print("Error: failed to open trace file traceCpssApi.tmp: " .. e)
                    return false
                end

                myCPSS_API_Debug = oldmyCPSS_API_Debug
                trace.start()
                print("Warning: trace cpss-api successfully restarted.\n")
            end

            return true
        end

        -- enable logging
        myCPSS_API_Debug = true
        trace.start()

        -- set desired mode
        if (params["traceMode"] == "c-style") then
            -- set trace mode is C-style
            myCPSS_API_log_in_C_Style = true

            if (params["cpssApiLogMode"] ~= nil) then
                if (params["cpssApiLogMode"] == "set-only") then
                    loggingSetOnly = true
                elseif (params["cpssApiLogMode"] == "set-and-get") then
                    loggingSetOnly = false
                elseif (params["cpssApiLogMode"] == "verbose") then
                    -- enable output on screen for trace cpss-api
                    trace.setOnScreen(true)
                elseif (params["cpssApiLogMode"] == "laconic") then
                    -- disable output on screen for laconic mode of trace cpss-api
                    trace.setOnScreen(false)
                end
            else
                 -- default when cpssApiLogMode omitted (jira: cpss-6255)
                 loggingSetOnly = false
                 trace.setOnScreen(true)
            end
        end

        -- start logging to temporary file
        if (traceCpssAPIfd == nil) then
            -- remove old log
            fs.unlink("traceCpssApi.tmp")

            -- create new one
            traceCpssAPIfd, e = fs.open("traceCpssApi.tmp", "w")
            if (traceCpssAPIfd == nil) then
                myCPSS_API_Debug = false
                trace.stop()

                print("Error: Logging stopped.")
                print("Error: failed to open trace file traceCpssApi.tmp: "..e)
                return false
            end
        end

        return true
    end

    return true
end


---------------------------------------------------------------------------------
-- command registration: trace cpss-api
---------------------------------------------------------------------------------
CLI_addCommand("debug", "trace cpss-api", {
    func = traceCpssApi,
    help = "Control of CPSS APIs logging",
    params = {
        {
            type = "named",
            {
                format = "c-style",
                help   = "Start logging in C-style"
            },
            {
                format = "convert lua2c %filename",
                name   = "luaFileName",
                help   = "Convert lua script to C code"
            },
            {
                format = "save %filename",
                name   = "fileName",
                help   = "File name in local FS to store output trace to"
            },
            {
                format = "to %ipv4",
                name   = "ip",
                help   = "The ip to upload the file to (optional)"
            },
            {
                format = "port %l4port",
                name   = "port",
                help   = "The file server port"
            },
            {
                format = "%cpssApiLogMode",
                name   = "cpssApiLogMode",
                help   = "Logging mode"
            },
            requirements = {
                ["ip"] = {
                    "fileName"
                },
                ["port"] = {
                    "ip"
                },
                ["cpssApiLogMode"] = {
                    "c-style"
                }
            },
            alt = {
                traceMode = { "c-style", "luaFileName", "fileName" }
            },
            mandatory = { "traceMode" }
        }
    }
})

---------------------------------------------------------------------------------
-- command registration: no trace cpss-api
---------------------------------------------------------------------------------
CLI_addCommand("debug", "no trace cpss-api", {
    func = function(params)
               params.flagNo = true
               return traceCpssApi(params)
           end,
    help = "Stop APIs logging"
})
