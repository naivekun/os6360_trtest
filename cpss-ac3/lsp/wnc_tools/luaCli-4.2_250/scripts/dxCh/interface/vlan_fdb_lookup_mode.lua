--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_fdb_lookup_mode.lua
--*
--* DESCRIPTION:
--*       add commands to set FDB FID or FID_VID1 lookup mode per VLAN
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


--includes

--constants
--------------------------------------------
-- type registration: fdb_lookup_fid_fid-vid1
--------------------------------------------
CLI_type_dict["fdb_lookup_fid_fid-vid1"] = {
    checker     = CLI_check_param_enum,
    complete    = CLI_complete_param_enum,
    help        = "",
    enum = {
        ["fid"] = { value=0, -- "CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E"
            help="FDB lookup uses Single Tag Key" },
        ["fid-vid1"] = { value=1, -- "CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E"
            help="FDB lookup uses Double Tag Key" },
        }
}

-- ************************************************************************
--  fdb_lookup_mode_set
--
--  @description  set FDB FID or FID_VID1 lookup mode per VLAN
--
--  @param params   - parameters
--     - params["fdb_mode"]    -
--          0 == CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E or
--          1 == CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E
--
--  @return command status and error messages
--
-- ************************************************************************

local function fdb_lookup_mode_set(params)
    local command_data = Command_Data()
    local fdb_mode  = params["fdb_mode"]

    command_data:initInterfaceDevVlanRange()
    command_data:initInterfaceDeviceRange("dev_vlan_range")
    command_data:initDevVlanInfoIterator()

    -- iterate through valid VLANs
    command_data:clearVlanStatus()
    command_data:clearLocalStatus()
    for _dummy, devNum, vlanId, vlanInfo in command_data:getDeviceVlanInfoIterator() do
        vlanInfo["fdbLookupKeyMode"] = fdb_mode;
        local rc, errMsg = vlan_info_set(devNum, vlanId, vlanInfo)
        if rc ~=0 then
            command_data:setFailVlanAndLocalStatus()

            if rc == 0x10 then
                command_data:addWarning("Information setting of vlan %d is not " ..
                                   "allowed on device %d.", vlanId, devNum)
            else
                command_data:addError("Error at information setting of vlan %d " ..
                                 "on device %d: %s.", vlanId, devNum, errMsg)
            end
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end
--------------------------------------------------------------------------------
-- command registration: fdb-key-lookup-mode
--------------------------------------------------------------------------------
CLI_addCommand({"vlan_configuration"}, "fdb-key-lookup-mode", {
  func   = fdb_lookup_mode_set,
  help   = "set FDB FID or FID_VID1 lookup mode per VLAN",
  params = {{type="values", {format="%fdb_lookup_fid_fid-vid1", name = "fdb_mode"}}}
})
-- DB for the sip6 devices to support the function for 'vid1 mode'
local sip6_BWC_vid1_mode_support = {}
-- ************************************************************************
--  fdb_lookup_mode_set
--
--  @description  set FDB FID or FID_VID1 lookup mode per VLAN
--
--  @param params   - parameters
--     - params["fdb_mode"]    -
--          0 == CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E or
--          1 == CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E
--
--  @return command status and error messages
--
-- ************************************************************************

local function fdb_vid1_assignmemt_set(params)
    local command_data = Command_Data();
    local fdb_vid1_mode  = params["fdb_vid1_mode"];

    -- Common variables initialization
    command_data:initAllAvailableDevicesRange();
    command_data:clearLocalStatus();
    for iterator, devNum in  command_data:getDevicesIterator() do
        command_data:clearDeviceStatus();
        if(is_sip_6(devNum)) then
            if (sip6_BWC_vid1_mode_support[devNum] == nil) then
                sip6_BWC_vid1_mode_support[devNum] = { orig_muxingMode = nil}
            end
            
            local muxingMode
            if(fdb_vid1_mode == true) then
                muxingMode = 0 -- CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_TAG_1_VID_E
                if sip6_BWC_vid1_mode_support[devNum].orig_muxingMode == nil then
                    -- we not know the orig value yet
                    local result, values = cpssPerDeviceParamGet(
                        "cpssDxChBrgFdbMacEntryMuxingModeGet",
                        devNum, "muxingMode")--"CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT"
                    
                    if result ~= 0 then
                        command_data:addWarning(
                           "cpssDxChBrgFdbMacEntryMuxingModeGet failed" ..
                           "result is " ..  tosting(result));
                    end
                    -- save the value
                    sip6_BWC_vid1_mode_support[devNum].orig_muxingMode = values.muxingMode
                end

            else -- restore default
                if sip6_BWC_vid1_mode_support[devNum].orig_muxingMode then
                    muxingMode = sip6_BWC_vid1_mode_support[devNum].orig_muxingMode
                    
                    -- indicate that we not have default 
                    sip6_BWC_vid1_mode_support[devNum].orig_muxingMode = nil
                else
                    muxingMode = 3 -- CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_UDB_4_AND_4_SRC_ID_3_DA_ACCESS_LEVEL_E
                end
            end
            
            local result, values = cpssPerDeviceParamSet(
                "cpssDxChBrgFdbMacEntryMuxingModeSet",
                devNum, muxingMode, "muxingMode", "GT_U32")--"CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT"
            
            if result ~= 0 then
                command_data:addWarning(
                   "cpssDxChBrgFdbMacEntryMuxingModeSet failed" ..
                   "result is " ..  tosting(result));
            end
        else
            local result, values = cpssPerDeviceParamSet(
                "cpssDxChBrgFdbVid1AssignmentEnableSet",
                devNum, fdb_vid1_mode, "enable", "GT_BOOL");
            if (result ~= 0) then
                command_data:addWarning(
                   "cpssDxChBrgFdbVid1AssignmentEnableSet failed" ..
                   "result is " ..  tosting(result));
            end
        end
        command_data:updateStatus();
        command_data:updateDevices();
    end
    command_data:analyzeCommandExecution();
    command_data:printCommandExecutionResults();
    return command_data:getCommandExecutionResults();
end

--------------------------------------------------------------------------------
-- command registration: bridge fdb-vid1-assignmemt-mode
--------------------------------------------------------------------------------
CLI_addCommand({"config"}, "bridge fdb-vid1-assignmemt-mode", {
  func   = fdb_vid1_assignmemt_set,
  help   = "set FDB VID1 assignment mode enable/disable",
  params = {{type="values", {format="%bool", name = "fdb_vid1_mode"}}}
})

