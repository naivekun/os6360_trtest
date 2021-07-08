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
--*       PX specific serdes configuration commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--*******************************************************************************
cmdLuaCLI_registerCfunction("luaCLI_getTraceDBLine")
cmdLuaCLI_registerCfunction("wrlCpssCommonCpssVersionGet")

-- ************************************************************************
--  prvGetCpssVersion
--        @description  local function which retruns CPSS baseline version
--
--        @return       CPSS baseline version
--

local function prvGetCpssVersion()
local ret,val,cpss_version
    -- CPSS version data getting
    ret, val = wrLogWrapper("wrlCpssCommonCpssVersionGet")
    if (ret == LGT_OK) then
        cpss_version = separatedNumberStrGet(val)
    else
        cpss_version = MSG_NOT_APPLICABLE
    end
    return cpss_version

end

--*******************************************************************************
--  traceSave
--
--  @description Save dump into a file
--
--  @param params -
--          params[filename]  - name of the file to save the dump to.
--          params[devID]     - device number.
--          params[apiEeprom] - format of the output file, either APIs or EEPROM.
--          params[saveToHd]  - save file to current work directory.
--          params[ip]        - ip to send the file to.
--          params[port]      - port to send the file to.
--
--  @return true on success, false on fail
--
--*******************************************************************************
local function traceSave(params)
    local data, address, mask, paramsBMP, dev, fd, e
    local i, j
    local accessType
    local addrSpace
    local mode
    local dfx = false
    local ret, val
    local firstLine = true


    if (params["dumpMode"] == nil) then
        mode = 1
    else
        mode = tonumber(params["dumpMode"])
    end

    if (params["devID"] == "all") then
        params["devID"] = nil
    end

    if (fs.fileexists(params["filename"])) then
        copy_file(params["filename"], params["filename"] .. ".bak")
    end

    fs.unlink(params["filename"])

    address, mask, data, paramsBMP = luaCLI_getTraceDBLine(0)

    if ((address == nil) and (data == nil)) then
        print("No trace is stored")
        return false
    end

    fd, e = fs.open(params["filename"], "w")
    if (fd == nil) then
        print("Failed to open file " .. params["filename"] .. ": " .. e)
        return false
    end

    -- check DB for been corrupted
    ret, val = myGenWrapper("appDemoTraceHwAccessDbIsCorrupted", {
                { "OUT", "GT_BOOL", "corruptedPtr" }
            })

    if (val["corruptedPtr"] == true) then
        print("\nWARNING:\n" ..
              "DB is corrupted. The HW access database has reached the limit " ..
              "and the recording has been stopped!\n" ..
              "Use trace clean in order to clear database")
        fs.write(fd, "WARNING: DB is corrupted. The HW access database has " ..
                     "reached the limit and the recording has been stopped!\n")
    end


    i = 0
    while ((address ~= nil) and (data ~= nil)) do
        dev = paramsBMP % 256
        addrSpace  = math.floor((paramsBMP % 0x00800000) / 0x00020000)
        accessType = math.floor((paramsBMP % 0x08000000) / 0x04000000)

        -- filter by device number
        if ((params["devID"] == nil) or (dev == params["devID"])) then

            -- write as APIs
            if (params["apiEeprom"] == "api") then
                if (address ~= 0) then
                    if ((accessType == mode) or (mode == 2)) then
                        if (accessType == 0) then
                            accessType = "cpssDrvPpHwRegisterRead "
                        else
                            accessType = "cpssDrvPpHwRegisterWrite "
                        end
                        fs.write(fd, accessType .. string.format(" 0x%x, 0, 0x%08x, 0x%08x\n",
                                                                 dev, address, data))
                    end
                else
                    fs.write(fd, "cpssOsDelay " .. (data * 1000) .. "\n")
                end

            -- write as EEPROM
            elseif (params["apiEeprom"] == "eeprom") then
                if (address ~= 0) then
                    if ((accessType == mode) or (mode == 2)) then
                        fs.write(fd, "offset " .. string.format("%08x  0x%08x\n",
                                                                address, data))
                    end
                else
                    fs.write(fd, "//----Delay " .. data .. " mSec----\n")
                    for j = 0, (data - 1) do
                        fs.write(fd, "offset 000000f0 0x" .. string.format("%08x\n", j))
                    end
                    fs.write(fd, "//------End of delay------\n")
                end

    --[[
            --
            -- !!! LUA PART OF CIDER HAS NOT BEEN PORTED FROM DXCH YET !!!
            --

            -- write as CIDER
            elseif (params["apiEeprom"] == "cider") then
                local k, searchRes, cider_string, index_string

                if (address ~= 0) then
                    searchRes = {}
                    searchRes["address"] = address

                    if (do_search_unit_by_address(nil, searchRes, {}, 0,
                                                  cider_RootNode()) == true) then
                        cider_string = "set unit " .. searchRes.foundpath

                        if (type(searchRes.i_indices) == "table") then
                            for k = 1, #searchRes.i_indices do
                                if (searchRes.i_indices[k].do_print) then
                                    index_string = " index "

                                    if (k ~= 1) then
                                        index_string = string.format(" index%d ", k)
                                    end

                                    cider_string = cider_string .. index_string ..
                                                   tostring(searchRes.i_indices[k].current_value)
                                end
                            end
                        end

                        cider_string = cider_string .. string.format(" value 0x%08X\n", data)
                        fs.write(fd, cider_string)
                    elseif ((accessType == mode) or (mode == 2)) then
                        if (accessType == 0) then
                            fs.write(fd, string.format("register read device %d " ..
                                                       "offset 0x%08X data 0x%08X\n",
                                                       dev, address, data))
                        else
                            fs.write(fd, string.format("register write device %d " ..
                                                       "offset 0x%08X data 0x%08X\n",
                                                       dev, address, data))
                        end
                    end
                else
                    fs.write(fd, "delay " .. data .. "\n")
                end
    ]]--

            -- write as Micro-Init sequence
            elseif (params["apiEeprom"] == "micInit") then
                -- check DFX address
                if (addrSpace == 8) then
                    dfx = true
                else
                    dfx = false
                end

                if firstLine == true then
                    local version = prvGetCpssVersion();
                    fs.write(fd, "-- CPSS baseline version: "..version.."\n")
                    firstLine = false
                end

                if (address ~= 0) then
                    if ((accessType == mode) or (mode == 2)) then
                        if (dfx == false) then
                            fs.write(fd, string.format("wrd %x %x 0x%08x 0x%08x 0x%08x\n",
                                                       dev, 0, address, data, mask))
                        else
                            fs.write(fd, string.format("wrd %x %x 0x%08x 0x%08x 0x%08x DFX\n",
                                                       dev, 0, address, data, mask))
                        end
                    end
                else
                    fs.write(fd, "delay " .. data .. "\n")
                end

            -- write as CLI register read/write commands
            else
                if (address ~= 0) then
                    if ((accessType == mode) or (mode == 2)) then
                        if (accessType == 0) then
                            fs.write(fd, string.format("register read device %d " ..
                                                       "offset 0x%08X data 0x%08X\n",
                                                       dev, address, data))
                        else
                            fs.write(fd, string.format("register write device %d " ..
                                                       "offset 0x%08X data 0x%08X\n",
                                                       dev, address, data))
                        end
                    end
                else
                    fs.write(fd, "delay " .. data .. "\n")
                end
            end
        end

        i = i + 1

        address, mask, data, paramsBMP = luaCLI_getTraceDBLine(i)
    end

    fs.close(fd)

    if (params["saveToHd"] ~= nil) then
        if (cmdCopyFromRAMFStoDisk(params["filename"]) == -1) then
            print("Unable to save file " .. params["filename"] .. " to disk")
            return false
        end
    end

    if (params["ip"] ~= nil) then
        if (params["port"] == nil) then
            params["port"] = 12346   --the default port
        end

        ret, e = send_file(params["filename"], params.ip["string"], params["port"])

        if (ret == true) then
            print("File sent successfully")
        else
            print("Sending file failed: " .. e)
            return false
        end
    end

    return true
end


---------------------------------------------------------------------------------
-- command registration: trace save
---------------------------------------------------------------------------------
CLI_addCommand("debug", "trace save", {
    func = traceSave,
    help = "Save dump into file",
    params = {
        {
            type = "named",
            {
                format = "file %filename",
                name   = "filename",
                help   = "File name to output trace to"
            },
            {
                format = "as %cpssApi_eeprom__without_cider",
                name   = "apiEeprom",
                help   = "Type of file to create"
            },
            {
                format = "device %devID_all",
                name   = "devID",
                help   = "Filter by device number"
            },
            {
                format = "to %ipv4",
                name   = "ip",
                help   = "The IP to upload the file to"
            },
            {
                format = "port %l4port",
                name   = "port",
                help   = "The file server port"
            },
            {
                format = "saveToHd",
                name   = "saveToHd",
                help   = "Save file in current work directory"
            },
            {
                format = "mode %dumpMode",
                name   = "dumpMode",
                help   = "HW access mode (default: write)"
            },
            requirements = {
                ["apiEeprom"] = {
                    "filename"
                },
                ["devID"] = {
                    "apiEeprom"
                },
                ["saveToHd"] = {
                    "apiEeprom"
                },
                ["ip"] = {
                    "apiEeprom"
                },
                ["port"] = {
                    "ip"
                }
            },
            mandatory = { "filename", "apiEeprom" }
        },
    }
})
