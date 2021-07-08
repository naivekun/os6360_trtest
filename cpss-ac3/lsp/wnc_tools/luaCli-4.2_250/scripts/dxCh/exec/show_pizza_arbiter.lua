--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_pizza_arbiter.lua
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
local function pizza_arbiter_dump(params)
    -- Common variables declaration
	local devices
    local command_data = Command_Data()
	local header_string, header_string2

    -- Command specific variables initialization.
	header_string = "  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n" ..
					"\t\t\t       :------------------------------------------------------------------------------------------------"

	header_string2 	= "  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19\n" ..
					  "\t\t\t       :------------------------------------------------------------"

    if (params["portGroup"]==nil) or (params["portGroup"]=="all") then params["portGroup"]=0xFFFFFFFF end -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
	devices={params["devID"]}

	status = true

	result, values = myGenWrapper(
		"cpssDxChPortPizzaArbiterDevStateGet", {
			{ "IN", "GT_U8"  , "dev", devices[j]},                 					 -- devNum
			{ "IN", "GT_U32" , "portGroupId", params["portGroup"]},					 -- portGroup
			{ "OUT","CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC" , "pizzaDeviceStatePtr"} -- data
		}
	)
	if (result ~= 0) then
		status = false
		err = returnCodes[result]
	end
	OutValues = values["pizzaDeviceStatePtr"]
	devType = OutValues.devType
	if(devType == "CPSS_PP_FAMILY_DXCH_LION2_E") then  -- Lion2
		devState = OutValues.devState
		lion2=devState.lion2
		lionTable={}
		lionTable[0]=lion2.rxDMA
		lionTable[0].name="RxDMA"
		lionTable[1]=lion2.rxDMA_CTU
		lionTable[1].name="RxDMA CTU"
		lionTable[2]=lion2.txDMA
		lionTable[2].name="TxDMA"
		lionTable[3]=lion2.txDMA_CTU
		lionTable[3].name="TxDMA CTU"
		lionTable[4]=lion2.BM
		lionTable[4].name="BM"
		lionTable[5]=lion2.TxQ
		lionTable[5].name="TxQ"

		for i = 0,4 do
			print("   " .. lionTable[i].name .. " Pizza Arbiter:")
			print("     Total Slices       = " .. lionTable[i].totalConfiguredSlices)
			if(lionTable[i].strictPriorityBit==true) then
				strictPriorityBit=1
			else
				strictPriorityBit=0
			end
			print("     StrictPriority Bit = " .. strictPriorityBit)

			lionTable[i].slices={}
			slices=""
			for j = 0,31 do
				if(lionTable[i].slice_enable[j]==true) then
					lionTable[i].slices[j]=lionTable[i].slice_occupied_by[j]
				else
					lionTable[i].slices[j]="X"
				end
				slices=slices .. " " .. string.format("%2s", lionTable[i].slices[j])
			end
			print("     Slice to port map : slice :" .. header_string)
			print("     Slice to port map : port  :" .. slices)
		end
		print("   " .. lionTable[5].name .. " Pizza Arbiter:")
		print("     Total Slices       = " .. lionTable[5].totalConfiguredSlices)
		print("     Slices in GOP      = " .. lionTable[5].slicesNumInGop)
		print("     Slice to port map : slice :" .. header_string)
		interval_begin = 0
		interval_end   = 31
		slices         = ""
		lionTable[5].slices = {}
		for j = 0, lionTable[5].totalConfiguredSlices do
			if(interval_end < j  or  j == lionTable[5].totalConfiguredSlices) then
				if(j   ==  lionTable[5].totalConfiguredSlices) then
					interval_end = lionTable[5].totalConfiguredSlices-1
				end
				print("     Slice " .. string.format("%4s", interval_begin) .. " : " .. string.format("%4s", interval_end) .. " : port  :" .. slices)
				interval_end   = j + 31
				interval_begin = j
				slices = ""
			end
			if(lionTable[5].slice_enable[j]==true) then
				lionTable[5].slices[j]=lionTable[5].slice_occupied_by[j]
			else
				lionTable[5].slices[j]="X"
			end
			slices=slices .. " " .. string.format("%2s", lionTable[5].slices[j])
		end
		print("     CPU type : " .. string.lower (string.sub(lionTable[5].cpu_port_type,24,-3)))
		for j = 0 , 4 do
			if(lionTable[5].cpu_port_slice_is_enable[j]) then
				print("     CPU port Slice " .. lionTable[5].cpu_port_sliceId[j] .. " : " .. lionTable[5].cpu_port_slice_occupied_by[j])
			end
		end
	else if((devType == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E") or
	        (devType == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or
	        (devType == "CPSS_PP_FAMILY_DXCH_AC3X_E"))
	then  -- Bobcat2/bobK/Aldrin/AC3X
        devState    = OutValues.devState
        bobK        = devState.bobK
        unitList    = bobK.unitList
        unitState   = bobK.unitState
        currentUnitName     = string.sub(unitList[0],19,-3)
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
            currentUnitName     = string.sub(unitList[i],19,-3)
            currentUnitState    = unitState[i]
        end
	else
		print("Wrong Device.")
	end
	end
    return status, err
end





--------------------------------------------------------------------------------
-- command registration: show pizza arbiter dump
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show pizza-arbiter-dump all", {
    func = pizza_arbiter_dump,
    help = "Display the pizza arbiter dump",
    params={
        { type="named",	{ format="device %devID_all",name="devID", help="The device number" },
						{ format="port-group %portGroup",name="portGroup", help="The port-group (optional, default is all)" },
						mandatory={"devID"}
        }
    }
})
