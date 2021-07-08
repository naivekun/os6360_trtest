--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* eventTable.lua
--*
--* DESCRIPTION:
--*       Show/Clear event table.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants

--registering c functions
if cli_C_functions_registered("wrlCpssGenEventTableGet", "wrlCpssGenEventTableClear") then

-- ************************************************************************
---
--  eventTableShow
--        @description  shows all the interupts that happened
--
--        @param params         
--
--        @return       true on success, otherwise false and error message
--
local function eventTableShow(params)
	local eventType = params["eventType"]
	local devID = params["devID"]
	local countersTable, currEventType, tempLine
	local hasData = false
	local header = "\n"
	header = header .. "             event type                             counter value             \n"
	header = header .. "------------------------------------------------------------------------------\n"
	local body = ""
	countersTable = wrLogWrapper("wrlCpssGenEventTableGet", "(devID, eventType)", devID, eventType);
	for index,currCounter in pairs(countersTable) do
		if hasData == false then
			hasData = true
		end
		currEventType = cpssGenWrapperCheckParam("CPSS_UNI_EV_CAUSE_ENT", tonumber(index))
		tempLine = string.format("%-60s%d", currEventType, currCounter)
		body = body .. tempLine .. "\n"	
	end
	
	if hasData == true then
		print(header)
		print(body)
	else
		print("\nThere are no events to show.\n")
	end
	
	return true
end

-- ************************************************************************
---
--  eventTableShow
--        @description  clears the event table
--
--        @param params         
--
--        @return       true on success, otherwise false and error message
--
local function eventTableClear(params)
	local eventType = params["eventType"]
	local devID = params["devID"]
	local clearSuccess, currEventType, tempLine

	clearSuccess = wrLogWrapper("wrlCpssGenEventTableClear", 
	                            "(devID, eventType)", devID, eventType);
	
	if clearSuccess ~= nil then
		print("\nAll events were cleared\n")
	else
		print("\nFailed to clear the events\n")
	end
	
	return true    
end

--------------------------------------------------------------------------------
-- command registration: event-table show
--------------------------------------------------------------------------------
CLI_addHelp("debug", "event-table", "events table")
CLI_addCommand("debug", "event-table show", {
    func   = eventTableShow,
    help   = "shows all the interupts that happened",
    params = {
	{ type="named",
		{ format="device %devID",name="devID", help="The device number\n" } ,
		{ format="eventType %eventType",name="eventType", help="The type of the event\n" } ,
		mandatory={"devID","eventType"},
		requirements={eventType={"devID"}}
		}
	}
})


--------------------------------------------------------------------------------
-- command registration: event-table clear
--------------------------------------------------------------------------------
CLI_addCommand("debug", "event-table clear", {
    func   = eventTableClear,
    help   = "clears the event table",
    params = {
	{ type="named",
		{ format="device %devID",name="devID", help="The device number\n" } ,
		{ format="eventType %eventType",name="eventType", help="The type of the event\n" } ,
		mandatory={"devID","eventType"},
		requirements={eventType={"devID"}}
		}
	}
})

end -- cli_C_functions_registered("wrlCpssGenEventTableGet", "wrlCpssGenEventTableClear")
