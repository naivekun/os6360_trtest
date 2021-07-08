--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_port_smi-mapping.lua
--*
--* DESCRIPTION:
--*       display the port smi mapping list
--*
--*
--********************************************************************************

	
cmdLuaCLI_registerCfunction("wrlCpssDxChBcat2SMIMappingDump")
	

-- ************************************************************************
---
--  DEC_HEX_String
--        @description  get configuration number
--
--        @param params         - inserted number                   
--
--        @return        result number(string)
--
local function DEC_HEX_String(IN)
    local IN_length   = string.len(IN)
    for i = IN_length , 1 do
        i   = i + 1
        IN  = "0"   ..  IN
    end
    IN  = "0x"  .. IN
    return IN
end


-- ************************************************************************
---
--  DEC_HEX
--        @description  get configuration number
--
--        @param params         - inserted number                   
--
--        @return        result number
--
local function DEC_HEX(IN)
    local B, K, OUT, I, D = 16, "0123456789ABCDEF", "", 0
    while IN > 0 do
        I		= I + 1
        IN, D	= math.floor(IN/B), math.mod(IN,B) + 1
        OUT		= string.sub(K,D,D) .. OUT
    end
    return DEC_HEX_String(OUT)
end
	
	
-- ************************************************************************
---
--  print_header_port_smi
--        @description  print header for smi
--
--
local function print_header_port_smi()
	local header_string = 
		  "+------+-------+-------+-------+-------+"	..
		"\n| Port | SMI-0 | SMI-1 | SMI-2 | SMI-3 |"	..
		"\n+------+-------+-------+-------+-------+"
	print(header_string)
end


-- ************************************************************************
---
--  print_footer_port_smi
--        @description  print footer for smi
--
--
local function print_footer_port_smi()
	local footer_string =
		"+------+-------+-------+-------+-------+"
	print(footer_string)
end


-- ************************************************************************
---
--  print_header_autopolling_number
--        @description  print header for autopolling number
--
--
local function print_header_autopolling_number()
	local header_string = 		
		  "+-----+--------------------+"	..
		"\n| SMI |  Autopolling Number|"	..
		"\n+-----+--------------------+"
	print(header_string)
end


-- ************************************************************************
---
--  show_port_smi
--        @description  show port smi list
--
--
local function show_port_smi(state)
	print_header_port_smi()
	local table_info 						= ""
	local CPSS_PHY_SMI_INTERFACE_MAX_E		= state["CPSS_PHY_SMI_INTERFACE_MAX_E"]
	local NUMBER_OF_PORTS_PER_SMI_UNIT_CNS	= state["NUMBER_OF_PORTS_PER_SMI_UNIT_CNS"]
	
	for smiLocalPort = 0, NUMBER_OF_PORTS_PER_SMI_UNIT_CNS - 1 do
		if(smiLocalPort ~= 0) then
			table_info = table_info .. "\n"
		end
		table_info = table_info .. 
			"| "	..	string.format("%4d", smiLocalPort) .. " |"
		for smiInstance = 0, CPSS_PHY_SMI_INTERFACE_MAX_E - 1 do
			current_value	= state.phyAddrRegArr[to_string(smiInstance)][smiLocalPort]
			table_info 		= table_info ..
				"  "	..	string.format("%4s", DEC_HEX(current_value))	.. " |"
		end  
	end
	print(table_info)
	print_footer_port_smi()
end


-- ************************************************************************
---
--  show_autopolling_number
--        @description  show autopolling number list
--
--
local function show_autopolling_number(state)
	print_header_autopolling_number()
	local table_info 					= ""
	local CPSS_PHY_SMI_INTERFACE_MAX_E	= state["CPSS_PHY_SMI_INTERFACE_MAX_E"]
	
	for smiInstance = 0, CPSS_PHY_SMI_INTERFACE_MAX_E - 1 do
		if(smiInstance ~= 0) then
			table_info = table_info .. "\n"
		end
		table_info = table_info ..
			"| "	..	string.format("%3d", smiInstance)	..
			" | "	..	string.format("%4d", state.autoPollNumOfPortsArr[smiInstance])
	end           							
	print(table_info)
end
		
		
-- ************************************************************************
---
--  show_port_smi_func
--        @description  show  autopolling number and smi lists 
--
--        @param params         - params["devID"]: checked device number
--
--
local function show_port_smi_func(params) 
	local GT_OK 		= 0
	local device		= params["devID"]
	local portGroup		= params["portGroup"]
	local values  		= wrLogWrapper("wrlCpssDxChBcat2SMIMappingDump","(device, portGroup)",device, portGroup)
	local rc 			= values["status"]
    if (rc ~= GT_OK) then
        local index = string.find(returnCodes[rc],"=")
        print("Error at show port smi:", string.sub(returnCodes[rc],index + 1))
        do return end
	end
	show_autopolling_number(values)
	show_port_smi(values)
end


--------------------------------------------------------------------------------
-- command registration: show port smi-mapping
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show port smi-mapping", {
    func = show_port_smi_func,
    help = "Display the port smi mapping list",
    params = {
			  {
				type="named",   { format="device %devID"		,name="devID"		, help="The device number" 		},
								{ format="portGroup %portGroup"	,name="portGroup"	, help="The portGroup number" 	},
								mandatory={"devID", "portGroup"}
			  }
    }
})
    
