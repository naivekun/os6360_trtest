--*******************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.  *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE       *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.    *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,      *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.  *
--*******************************************************************************
--* execute_galtis_cmd.lua
--*
--* DESCRIPTION:
--*       Execute Galtis commands from input file
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--*******************************************************************************


-- externs
if (cli_C_functions_registered("wrapl_cmdEventFields", "wrapl_cmdEventRun")) then


--*******************************************************************************
--  readLines
--
--  @description Read Galtis file main iterator
--
--*******************************************************************************
local function readLines(filename)
    local fd = fs.open(filename)

    if (fd == nil) then
        return nil
    end

    local i = 0
    return function()
        local s = fs.gets(fd)
        if (s == nil) then
            fs.close(fd)
            return nil
        end

        -- delete spaces
        s = string.gsub(s, "^%s*(.-)%s*$", "%1")
        i = i + 1
        if (i > 500) then
            collectgarbage("collect")
            i = 0
        end

        return s
    end
end


--*******************************************************************************
--  executeGaltisCmd
--
--  @description  Executes file with Galtis instructions
--
--  @param params.filename  - file name to Galtis commands from
--  @param params.trace     - trace enable flag
--
--  @return  true on success, otherwise false and error message
--
--*******************************************************************************
local function ExecuteGaltisCmd(params)
    local fid
    local i
    local line, output, status, ln
    local nextLine = nil
    local delayStart, delayEnd, delayTime
    local galtisLines = {}

    if (not loadable_file_check(params)) then
        return false
    end

    for line in readLines(params.filename) do
        -- for debug purposes
        -- print (line)

        -- skip comments and return codes from log file
        if ((line ~= nil) and (line ~= "") and
            (string.sub(line, 1, 2) ~= "//") and
            (string.sub(line, 1, 3) ~= "@@@")) then
            table.insert(galtisLines, line)
        end
    end


    i = 0
    while (i < #galtisLines) do
        i = i + 1
        line = galtisLines[i]

        if (params["trace"] ~= nil) then
            print(string.format("%04d", i) .. "  try >>>  " .. line)
        end

        delayStart, delayEnd, delayTime = string.find(string.lower(line), "delay%s+(%d+)")
        if ((delayStart == 1) and (delayTime ~= nil)) then
            luaTaskDelay(delayTime)  --in milliseconds
        else
            CLI_execution_unlock()

            if (wrapl_cmdEventFields(line)) then
                i = i + 1
                nextLine = galtisLines[i]
                status, output = wrapl_cmdEventRun(line, nextLine)
            else
                nextLine = nil
                status, output = wrapl_cmdEventRun(line)
            end

            CLI_execution_lock()
        end

        -- check return codes
        if (status) then
            if (params["trace"] ~= nil) then
                print(output)
            end
        else
            print(" The command  " .. line)
            if (nextLine ~= nil) then
                print(nextLine)
            end
            print(" *** failed: " .. output)

            return false
        end
    end

    return true
end


--******************************************************************************
-- command registration: execute galtis %filename %ip %port [trace]
--******************************************************************************
CLI_addCommand("debug", "load galtis", {
    help = "Load Galtis commands from given file",
    func = ExecuteGaltisCmd,
    params = {
        {
            type = "values",
            {
                format = "%string",
                name   = "filename",
                help   = "Local file name"
            }
        },
        {
            type="named",
            "#loadableFile",
            {
                format = "trace",
                help   = "trace execution"
            }
        }
    }
})


end -- (cli_C_functions_registered("wrapl_cmdEventFields", "wrapl_cmdEventRun"))
