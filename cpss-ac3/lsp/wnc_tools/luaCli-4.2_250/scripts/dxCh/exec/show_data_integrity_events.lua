--********************************************************************************
--*              (c), Copyright 2014, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_data_integrity_events.lua
--*
--* DESCRIPTION:
--*       showing of TODO
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
require("common/generic/tables")

--constants 

--registering c functions
if cli_C_functions_registered("wrlCpssDxChDiagDataIntegrityEventsGet") then

local function init_tables()
    local tblCommon = {
        { head="evType", path="eventsType",
            len=30,
            rmprefix="CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_",
            rmsuffix="_E" },
        { head="pgroup", path="causePortGroupId",
            len=6 },
        { head="memType", path="memType",
            len=20,
            rmprefix="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_",
            rmsuffix="_E" }
    }

    local tblMem = Nice_Table(tblCommon, {
        { head="dfxPipeId", path="location.memLocation.dfxPipeId",
            len=9 },
        { head="dfxClientId", path="location.memLocation.dfxClientId",
            len=11 },
        { head="dfxMemoryId", path="location.memLocation.dfxMemoryId",
            len=11 }
    })

    local tblMppm = Nice_Table(tblCommon, {
        { head="portGroupId", path="location.mppmMemLocation.portGroupId",
            len=12 },
        { head="mppmId", path="location.mppmMemLocation.mppmId",
            len=6 },
        { head="bankId", path="location.mppmMemLocation.bankId",
            len=6 }
    })
    local tblTcam = Nice_Table(tblCommon, {
        { head="arrayType", path="location.tcamMemLocation.arrayType",
            len=9,
            rmprefix="CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_",
            rmsuffix="_E" },
        { head="ruleIndex", path="location.tcamMemLocation.ruleIndex",
            len=9 }

    })
    return tblMem, tblMppm, tblTcam
end
-- ************************************************************************
---
--  show_data_integrity_events
--        @description  shows TODO
--
--        @param params.devID   -
--        @param params.portGroupID   -
--        @param params.evType   -
--
--        @return       true on success, otherwise false and error message
--
local function show_data_integrity_events(params)
    local rc, tbl, i, p

    if params.portgroup == nil then
        params.portgroup = "all"
    end
    if params.portgroup == "all" then
        params.portgroup = luaCLI_getPortGroups(params.devID)
    else
        params.portgroup = { params.portgroup }
    end
    if params.evType == nil then
        params.evType = "all"
    end
    
    -- init tables
    local tblMem, tblMppm, tblTcam = init_tables()
    local found = false
    for p=1,#params.portgroup do
    
        local devID_ = params.devID
        local portgroup_ = params.portgroup[p]
        local evType_ = params.evType
        rc, tbl = wrLogWrapper("wrlCpssDxChDiagDataIntegrityEventsGet", 
                               "(devID_, portgroup_, evType_)", 
                               devID_, portgroup_, evType_)
        if rc ~= 0 then
            --TODO: err
        else
            for i=1,#tbl do
                local matched = false
                if params.evType == "all" then
                    matched = true
                elseif params.evType == "parity" then
                    if string.find(tbl[i].eventsType,"_PARITY_",1,true) ~= nil then
                        matched = true
                    end
                elseif params.evType == "ecc" then
                    if string.find(tbl[i].eventsType,"_ECC_",1,true) ~= nil then
                        matched = true
                    end
                end
                if matched then
                    found = true

                    if tbl[i].location.memLocation ~= nil then
                        tblMem:addRow(tbl[i])
                    end
                    if tbl[i].location.mppmMemLocation ~= nil then
                        tblMppm:addRow(tbl[i])
                    end
                    if tbl[i].location.tcamMemLocation ~= nil then
                        tblTcam:addRow(tbl[i])
                    end
                end
            end
        end
    end

    if not found then
        print("No events")
    end
    if #tblMem.rows > 0 then
        tblMem:print()
    end
    if #tblMppm.rows > 0 then
        tblMppm:print()
    end
    if #tblTcam.rows > 0 then
        tblTcam:print()
    end
end

CLI_type_dict["DIAG_EV_ADDITION_TYPE"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Event type",
    enum = {
        all = { val="all", help="Show all events" },
        parity = { val="parity", help="Show parity events only" },
        ecc = { val="parity", help="Show ECC events only" }
    }
}

--CLI_addHelp("exec", "show counters", "Display counters")
CLI_addCommand("exec", "show data-integrity-events", {
  func   = show_data_integrity_events,
  help   = "Show data-integrity-events", --TODO
  params={{type= "named",
        {format="device %devID", name="devID", help="ID of the device"},
        {format="portgroup %portGroup", name="portgroup", help="Port group"},
        {format="type %DIAG_EV_ADDITION_TYPE",name="evType", help="Event type"},
        mandatory = {"devID"}
    }
  }
})
end

--[[
Console# lua
lua>require("generic/show_data_integrity_events")
lua>show_data_integrity_events({devID=0,portGroupID=0,evType="PRV_CPSS_LION2_TCC_LOWER_GEN_TCAM_ERROR_DETECTED_E"})
]]--

