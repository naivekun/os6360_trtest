--********************************************************************************
--*              (c), Copyright 2017, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_version.lua
--*
--* DESCRIPTION:
--*       Showing of version information for the system
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlOsResetStdOutFunction")
cmdLuaCLI_registerCfunction("wrlOsSaveCurrentStdOutFunction")
cmdLuaCLI_registerCfunction("wrlCpssCommonCpssVersionGet")
cmdLuaCLI_registerCfunction("wrlCpssCommonBspVersionGet")
cmdLuaCLI_registerCfunction("wrlCpssCommonBoardNameGet")
cmdLuaCLI_registerCfunction("wrlOsRestoreStdOutFunction")

if (px_family) then
    cmdLuaCLI_registerCfunction("wrlMvPxVersionGet")
end
if (dxCh_family) then
    cmdLuaCLI_registerCfunction("wrlMvDxchVersionGet")
end


--constants
local MSG_NOT_APPLICABLE = "N/A"


--********************************************************************************
--
--  getLspVersionString
--
--  @description    Return LSP version string for Linux-based devices
--
--  @param params   Not used
--
--  @return         LSP_string
--
--********************************************************************************
local function getLspVersionString(params)
    local LSP_string = nil

    -- is Win?
    if (package.config:sub(1, 1) == "\\") then
        return nil
    end

    -- is Simulation?
    if (wrlCpssIsAsicSimulation()) then
        return nil
    end

    local result = ""
    local f = io.open("/dev/shm/LSP_VERS.TXT")

    if (f ~= nil) then
        result = f:read("*a")
        f:close()

        if (result ~= nil) then
            -- trim string
            LSP_string = result:match "^%s*(.-)%s*$"
        end
    end

    return LSP_string
end


--********************************************************************************
--
--  show_version_func
--
--  @description    Shows version information for the system
--
--  @param params   Not used
--
--  @return         true on success, otherwise false and error message
--
--********************************************************************************
local function show_version_func(params)
    -- Common variables declaration
    local ret, val
    local cpss_version, lsp_version, os_version
    local dxch_or_px_version, shared_lib_build, linux_no_kernel_module, is_64bit_os

    -- CPSS version data getting
    ret, val = wrLogWrapper("wrlCpssCommonCpssVersionGet")
    if (ret == LGT_OK) then
        cpss_version = separatedNumberStrGet(val)
    else
        cpss_version = MSG_NOT_APPLICABLE
    end

    print(" \nCPSS version:  " .. cpss_version)

    -- DxCh/Px version
    if (dxCh_family) then
        dxch_or_px_version, shared_lib_build, linux_no_kernel_module, is_64bit_os =
                wrLogWrapper("wrlMvDxchVersionGet")

        print("DxCh version:  " .. dxch_or_px_version)
    end
    if (px_family) then
        dxch_or_px_version, shared_lib_build, linux_no_kernel_module, is_64bit_os =
                wrLogWrapper("wrlMvPxVersionGet")

        print("PX version:    " .. dxch_or_px_version)
    end

    -- LSP version
    lsp_version = getLspVersionString()

    if (lsp_version ~= nil) then
        if (linux_no_kernel_module) then
            lsp_version = "No Kernel Module"
        end

        if (is_64bit_os) then
            lsp_version = lsp_version .. " 64 bit"
        else
            lsp_version = lsp_version .. " 32 bit"
        end

        print("LSP version:   " .. lsp_version)
    end

    -- Build Type (just for Linux)
    if (package.config:sub(1, 1) ~= "\\") then
        if (shared_lib_build) then
            shared_lib_build = "Shared Lib"
        else
            shared_lib_build = "Standalone"
        end

        print("Build type:    Linux " .. shared_lib_build)
    end

    -- Windows OS or Linux Simulation 32bit / 64bit
    if ((package.config:sub(1, 1) == "\\") or (lsp_version == nil)) then
        if (is_64bit_os) then
            os_version = "64 bit Operating System"
        else
            os_version = "32 bit Operating System"
        end

        print("System type:   " .. os_version .. "\n")
    end

    return true
end



----------------------------------------------------------------------------------
-- command registration: show version
----------------------------------------------------------------------------------
CLI_addCommand("exec", "show version", {
    func = show_version_func,
    help = "System hardware and software version"
})
