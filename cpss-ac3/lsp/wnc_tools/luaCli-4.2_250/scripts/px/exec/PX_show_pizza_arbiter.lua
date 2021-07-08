--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_show_pizza_arbiter.lua
--*
--* DESCRIPTION:
--*       display the pizza arbiter dump
--*
--*
--********************************************************************************



-- ************************************************************************
---
--  pizza_arbiter_dump
--        @description  show pizza arbiter dump
--
--        @param params         - params["all"]: all devices port or all
--                                ports of given device cheking switch,
--                                could be irrelevant;
--                                params["devID"]: checked device number,
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil;
--                                params["port-channel"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil
--
--        @return       true on success, otherwise false and error message
--
local function Px_pizza_arbiter_dump(params)
    -- Common variables declaration
    local devNum = params["device"]
    local command_data = Command_Data()
    local header_string, header_string2

    -- Command specific variables initialization.
    header_string = "  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n" ..
                    "\t\t\t       :------------------------------------------------------------------------------------------------"

    header_string2  = "  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19\n" ..
                      "\t\t\t       :------------------------------------------------------------"

    --print(to_string(params))

    status = true

    result, values = myGenWrapper(
        "cpssPxPortPizzaArbiterDevStateGet", {
            { "IN",  "GT_SW_DEV_NUM", "devNum", devNum },
            { "OUT","CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC" , "pizzaDeviceStatePtr"} -- data
        }
    )
    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    else
        OutValues = values["pizzaDeviceStatePtr"]
        devState = OutValues.devState
        pipe        = devState.pipe
        unitList    = pipe.unitList
        unitState   = pipe.unitState
        currentUnitName     = string.sub(unitList[0],17,-3)
        currentUnitState    = unitState[0]

        local i = 0
        while currentUnitName ~= "UNDEFINED" do
            totalConfiguredSlices   = currentUnitState.totalConfiguredSlices
            totalSlicesOnUnit       = currentUnitState.totalSlicesOnUnit
            workConservingBit       = currentUnitState.workConservingBit
            pizzaMapLoadEnBit       = currentUnitState.pizzaMapLoadEnBit
            slice_enable            = currentUnitState.slice_enable
            slice_occupied_by       = currentUnitState.slice_occupied_by
            print("   " .. currentUnitName .. " Pizza Arbiter:")
            print("     Configured Slices/Total  = " .. totalConfiguredSlices .. "/" .. totalSlicesOnUnit)
            if(workConservingBit) then
                workConservingBit = 1
            else
                workConservingBit = 0
            end
            print("     Work Conserving Bit      = " .. workConservingBit)

            print("     Slice to port map : slice :" .. header_string2)
            interval_begin = 0
            interval_end   = 19
            slices=""
            currentUnitState.slices={}
            for j = 0, totalSlicesOnUnit do
                if(interval_end < j  or  j == totalSlicesOnUnit) then
                    if(j   ==  totalSlicesOnUnit) then
                        interval_end = totalSlicesOnUnit - 1
                    end
                    print("     Slice " .. string.format("%4s", interval_begin) .. " : " .. string.format("%4s", interval_end) .. " : port  :" .. slices)
                    interval_end   = j + 19
                    interval_begin = j
                    slices = ""
                end
                if(slice_enable[j]==true) then
                    currentUnitState.slices[j] = currentUnitState.slice_occupied_by[j]
                else
                    currentUnitState.slices[j] = "X"
                end
                slices = slices .. " " .. string.format("%2s", currentUnitState.slices[j])
            end
            i = i + 1
            currentUnitName     = string.sub(unitList[i],17,-3)
            currentUnitState    = unitState[i]
        end
    end
    return status, err
end





--------------------------------------------------------------------------------
-- command registration: show pizza arbiter dump
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show pizza-arbiter-dump", {
    func = Px_pizza_arbiter_dump,
    help = "Display the pizza arbiter dump",
    params = {
        {
            type = "named",
            {
                format = "device %devID",
                name = "device",
                help = "The device ID"
            },
            mandatory = { "device" }
        }
    }
})

