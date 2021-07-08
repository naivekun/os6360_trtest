--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* system_capabilities.lua
--*
--* DESCRIPTION:
--*       common system, device and port functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 23 $
--*
--********************************************************************************

cmdLuaCLI_callCfunction("registerBitwizeFunctions")

-- ************************************************************************
---
--  get_port_list
--        @description  Get the port range that is supported in the device
--
--        @param dev            - The device ID
--
--        @return       The list of ports on the device
--
function get_port_list(dev)
    local i, result, max_port_number

    local ret_new = {}

    local sysInfo = luaCLI_getDevInfo(dev)
    if type(sysInfo) ~= "table" or sysInfo[dev] == nil then
        -- case of invoking before cpssInitSystem
        return ret_new
    end

    local ports = sysInfo[dev]
    for i = 1,#(ports) do
        local portNum = ports[i]
        table.insert(ret_new, portNum)
    end

    return ret_new
end

-- ************************************************************************
---
--  get_device_count
--        @description  getting of device count
--
--        @return       device count
--
function get_device_count()
    return wrlCpssDeviceCountGet()
end


-- ************************************************************************
---
--  does_device_exist
--        @description  checking of device existance
--
--        @param devID          - device ID
--
--        @return       true if exists, otherwise false
--
function does_device_exist(devID)
    return wrlIsDevIdValid(devID)
end

-- ************************************************************************
---
--  is_dev_port_range
--        @description  checking if table is in dev/port format
--
--        @param dev_port_table - entry table
--
--        @return       true if it could be dev/port table, otherwise false
--
function is_dev_port_range(dev_port_table)
    local devNum, ports, index, portNum

    if ("table" ~= type(dev_port_table)) then
        return false
    end

    for devNum, ports in pairs(dev_port_table) do
        if ("number" ~= type(devNum)) or ("table" ~= type(ports)) then
            return false
        end

        for index, portNum in pairs(ports) do
            if ("number" ~= type(index)) or ("number" ~= type(portNum)) then
                return false
            end
        end
    end

    return true
end


if dxCh_family == true then
    require_safe_dx("generic/system_capabilities")
end

if px_family == true then
    require_safe_px("generic/system_capabilities")
end

-- ************************************************************************
---
--  generic_getFilteredDeviceList
--        @description  builds list of devices filterd by validate function
--        Parameters:
--        devNumOrAll - device number or string "all"
--        validateFunction, validateFunctionData - filter function and data:
--        if value returned by validateFunction(devNum, validateFunctionData)
--        is not nil or false - devNum included to result list.
function generic_getFilteredDeviceList(devNumOrAll, validateFunction, validateFunctionData)
    local devices,j,all_devs,dev_amount,dev_idx;

    if (devNumOrAll == "all") then
        all_devs=wrlDevList()
    else
        all_devs={devNumOrAll}
    end
    dev_amount = #all_devs;

    -- exclude not relevant devices
    dev_idx = 1;
    devices = {};
    for j=1, dev_amount do
        if validateFunction(all_devs[j], validateFunctionData) then
            devices[dev_idx] = all_devs[j];
            dev_idx = dev_idx + 1;
        end
    end
    return devices;
end

-- ************************************************************************
---
--  generic_IsDeviceInFamilyList
--        @description  checks if given device family belongs to given family list
--        Parameters:
--        devNum - device number
--        familyList - list of device families
--        returns true or false
function generic_IsDeviceInFamilyList(devNum, familyList)
    local dev_family = wrlCpssDeviceFamilyGet(devNum);
    local i;
    for i=1,#familyList do
        if familyList[i] == dev_family then
            return true;
        end
    end
    return false
end

-- check if device is port extender
function isPortExtender(devNum)
    return generic_IsDeviceInFamilyList(devNum, {"CPSS_PX_FAMILY_PIPE_E"})
end

-- ************************************************************************
---
--  generic_getDeviceOfFamilyList
--        @description  builds list of devices belongs to given device-fimilies
--        Parameters:
--        devNumOrAll - device number or string "all"
--        familyList  - list of device families.
function generic_getDeviceOfFamilyList(devNumOrAll, familyList)
    return generic_getFilteredDeviceList(
        devNumOrAll,
        generic_IsDeviceInFamilyList,
        familyList);
end

-- ************************************************************************
---
--  generic_getDeviceOfNotFamilyList
--        @description  builds list of devices not belongs to given device-fimilies
--        Parameters:
--        devNumOrAll - device number or string "all"
--        familyList  - list of device families.
function generic_getDeviceOfNotFamilyList(devNumOrAll, familyList)
    return generic_getFilteredDeviceList(
        devNumOrAll,
        function(dev, list)
            return (not generic_IsDeviceInFamilyList(dev, list));
        end,
        familyList);
end
