--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* phySmi.lua
--*
--* DESCRIPTION:
--*       read and write commands using phy/smi
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
--  phyRegisterRead
--
--  @description reads data from phy using the smi
--
--  @param devID    - The device number
--  @param port    - The port number
--  @param phyReg    - The phy register number number
--
--
-- ************************************************************************

local function phyRegisterRead(params)
	local res,val,devices,j

	print(to_string(params))
	if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

	for j=1,#devices do
		res, val = myGenWrapper("cpssDxChPhyPortSmiRegisterRead", {
				{ "IN", "GT_U8"  , "devNum", devices[j]},
				{ "IN", "GT_U32" , "portNum",  params["portNum"]},
				{ "IN", "GT_U8", "phyReg", params["phyReg"]}, --(0-31)
				{ "OUT", "GT_U16"  , "dataPtr"}
		})

		if (res==0) then print("Device :"..devices[j].."\tValue:"..string.format("\t0x%04X",val["dataPtr"]))
		else
			print("Error reading from device "..devices[j])
		end
	end
end

--------------------------------------------
-- command registration: phy register read
--------------------------------------------
CLI_addHelp("debug", "phy", "PHY sub-commands")
CLI_addHelp("debug", "phy register", "PHY register sub-commands")

CLI_addCommand("debug", "phy register read", {
    func=phyRegisterRead,
	help="Read specified SMI Register",
	params={
		{ type="named",
		{ format="device %devID_all",name="devID",help="The device number" } ,
		{ format="port %portNum",name="portNum", help="The port number" } ,
		{ format="register %phyReg",name="phyReg", help="Phy register number (0..31)" } ,
		mandatory={"devID","portNum","phyReg"},
		}
    }
})

-- ************************************************************************
--  phyRegisterWrite
--
--  @description write data to a phy using the smi
--
--  @param devID    - The device number
--  @param port    - The port number
--  @param phyReg    - The phy register number number
--  @param data    - The data to be written
--
--
-- ************************************************************************
local function phyRegisterWrite(params)
	local res,devices,j

	print(to_string(params))
	if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

	for j=1,#devices do

		res = myGenWrapper("cpssDxChPhyPortSmiRegisterWrite", {
				{ "IN", "GT_U8"  , "devNum", devices[j]},
				{ "IN", "GT_U32" , "portNum",  params["portNum"]},
				{ "IN", "GT_U8", "phyReg", params["phyReg"]}, --(0-31)
				{ "IN", "GT_U16" , "data"  , params["data"]}
		})

		if (res~=0) then print("Error writing to device "..devices[j]) end
	end
end

--------------------------------------------
-- command registration: phy register write
--------------------------------------------
CLI_addCommand("debug", "phy register write", {
    func=phyRegisterWrite,
	help="Write value to specified SMI Register",
	params={
		{ type="named",
		{ format="device %devID_all",name="devID",help="The device number" } ,
		{ format="port %portNum",name="portNum", help="The port number" } ,
		{ format="register %phyReg",name="phyReg", help="Phy register number (0..31)" } ,
		{ format="data %GT_U16",name="data", help="The data to write" } ,
		mandatory={"devID","portNum","phyReg","data"},
		}
    }
})


-- ************************************************************************
--  phy10GRegisterRead
--
--  @description reads data from phy using the smi
--
--  @param devID    - The device number
--  @param port    - The port number
--  @param phyReg    - The phy register number number
--
--
-- ************************************************************************

local function phy10GRegisterRead(params)
	local res,val,devices,j,useExternal

	if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end
	if (params["phyID"]==nil) then useExternal=false else useExternal = true end

	for j=1,#devices do
		res, val = myGenWrapper("cpssDxChPhyPort10GSmiRegisterRead", {
			{ "IN", "GT_U8"  , "devNum", devices[j]},
			{ "IN", "GT_U32" , "portNum",  params["portNum"]},
			{ "IN", "GT_U8" , "phyId", params["phyID"]},  --(0-31)
			{ "IN", "GT_BOOL", "useExternalPhy", useExternal},
			{ "IN", "GT_U16"  , "phyReg", params["phyReg"]},
			{ "IN", "GT_U8"  , "phyDev", params["phyDev"]}, --(0-31)
			{ "OUT", "GT_U16"  , "dataPtr"}
		})

		if (res==0) then print("Device :"..devices[j].."\tValue:"..string.format("\t0x%04X",val["dataPtr"]))
		else
			print("Error reading from device "..devices[j].. " returned error code:"..res)
		end
	end
end

--------------------------------------------
-- command registration: phy 10G register read
--------------------------------------------
CLI_addHelp("debug", "phy 10G", "phy 10G sub-commands")
CLI_addHelp("debug", "phy 10G register", "phy 10G register sub-commands")

CLI_addCommand("debug", "phy 10G register read", {
    func=phy10GRegisterRead,
	help="Read specified SMI Register and PHY device",
	params={
		{ type="named",
		{ format="device %devID_all",name="devID",help="The device number" } ,
		{ format="port %portNum",name="portNum", help="The port number" } ,
		{ format="register %GT_U16",name="phyReg", help="The phy register number" } ,
		{ format="phyDev %phyDev",name="phyDev", help="The phy device number (0-31)" } ,
		{ format="phyID %phyID",name="phyID", help="The phy ID number (optional, for external phy -  0-31)" } ,
		mandatory={"devID","portNum","phyReg","phyDev"},
		}
    }
})

-- ************************************************************************
--  phy10GRegisterWrite
--
--  @description write data to a phy using the smi
--
--  @param devID    - The device number
--  @param port    - The port number
--  @param phyReg    - The phy register number number
--  @param data    - The data to be written
--
--
-- ************************************************************************
local function phy10GRegisterWrite(params)
	local res,devices,j
	local val
	local useExternal
   
	if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end
	if (params["phyID"]==nil) then useExternal=false else useExternal = true end

	for j=1,#devices do

		res, val = myGenWrapper("cpssDxChPhyPort10GSmiRegisterWrite", {
			{ "IN", "GT_U8"  , "devNum", devices[j]},
			{ "IN", "GT_U32" , "portNum",  params["portNum"]},
			{ "IN", "GT_U8" , "phyId", params["phyID"]},  --(0-31)
			{ "IN", "GT_BOOL", "useExternalPhy", useExternal},
			{ "IN", "GT_U16"  , "phyReg", params["phyReg"]},
			{ "IN", "GT_U8"  , "phyDev", params["phyDev"]},  --(0-31)
			{ "IN", "GT_U16"  , "dataPtr", params["data"]}
		})


		if (res~=0) then print("Error writing to device "..devices[j].. " returned error code:"..res) end
	end
end

--------------------------------------------
-- command registration: phy 10G register write
--------------------------------------------
CLI_addCommand("debug", "phy 10G register write", {
    func=phy10GRegisterWrite,
    help="Write value to a specified SMI Register and PHY device",
    params={
        { type="named",
        { format="device %devID_all",name="devID",help="The device number" } ,
        { format="port %portNum",name="portNum", help="The port number" } ,
        { format="register %GT_U16",name="phyReg", help="The phy register number" } ,
        { format="phyDev %phyDev",name="phyDev", help="The phy device number" } ,
        { format="phyID %phyID",name="phyID", help="The phy ID number (optional)" } ,
        { format="data %GT_U16",name="data", help="The data to write" } ,
        mandatory={"devID","portNum","phyReg","phyDev","data"},
        }
    }
})



-- ************************************************************************
--  smiRegisterRead
--
--  @description reads data from phy using the smi
--
--  @param devID        - The device number
--  @param portGroup    - The portgroup number
--  @param smiInterface - The interface of the smi
--  @param phyAddr      - The address of the phy
--  @param phyReg       - The phy register number number
--
--
-- ************************************************************************

local function smiRegisterRead(params)
	local res,val,devices,j

	if (params["portGroup"]=="all")  then params["portGroup"]=0xFFFFFFFF else params["portGroup"]=2^tonumber(params["portGroup"]) end
	if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end


	for j=1,#devices do

		res, val = myGenWrapper("cpssSmiRegisterRead", {
				{ "IN", "GT_U8"  , "devNum", devices[j]},
				{ "IN", "GT_U32" , "portGroupsBmp",  params["portGroup"]},
				{ "IN", TYPE["ENUM"], "smiInterface", params["smiInterface"]},
				{ "IN", "GT_U32"  , "smiAddr", params["phyAddr"]}, -- (0-31)
				{ "IN", "GT_U32"  , "regAddr", params["phyReg"]},
				{ "OUT", "GT_U32"  , "dataPtr"}
		})

		if (res==0) then print("Device :"..devices[j].."\tValue:"..string.format("\t0x%08X",val["dataPtr"]))
		else
			print("Error reading from device "..devices[j])

		end

	end

end

--------------------------------------------
-- command registration: smi register read
--------------------------------------------
CLI_addHelp("debug", "smi", "SMI sub-commands")
CLI_addHelp("debug", "smi register", "SMI register sub-commands")

CLI_addCommand("debug", "smi register read", {
    func=smiRegisterRead,
    help="Read data using smi",
    params={
        { type="named",
        { format="device %devID_all",name="devID",help="The device number" } ,
        { format="portgroup %portGroup",name="portGroup", help="The port-group number" } ,
        { format="interface %smiInterface",name="smiInterface", help="The phy interface number" } ,
        { format="address %phyAddr",name="phyAddr", help="The phy address" } ,
        { format="register %GT_U32",name="phyReg", help="The phy register number" } ,
        mandatory={"devID","portGroup","smiInterface","phyAddr","phyReg"},
        }
    }
})

-- ************************************************************************
--  smiRegisterWrite
--
--  @description write data to a phy using the smi
--
--  @param devID    - The device number
--  @param portGroup    - The portgroup number
--  @param smiInterface    - The interface of the smi
--  @param address    - The address of the phy
--  @param phyReg    - The phy register number number
--  @param data    - The data to be written
--
--
-- ************************************************************************
local function smiRegisterWrite(params)
	local res,devices,j

	if (params["portGroup"]=="all")  then params["portGroup"]=0xFFFFFFFF else params["portGroup"]=2^tonumber(params["portGroup"]) end
	if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end


	for j=1,#devices do

		res = myGenWrapper("cpssSmiRegisterWrite", {
				{ "IN", "GT_U8"  , "devNum", devices[j]},
				{ "IN", "GT_U32" , "portGroupsBmp",  params["portGroup"]},
				{ "IN", TYPE["ENUM"], "smiInterface", params["smiInterface"]},
				{ "IN", "GT_U32"  , "smiAddr", params["phyAddr"]}, -- (0-31)
				{ "IN", "GT_U32"  , "regAddr", params["phyReg"]},
				{ "IN", "GT_U32"  , "dataPtr",params["data"]}
		})

		if (res~=0) then print("Error writing to device "..devices[j]) end

	end

end

--------------------------------------------
-- command registration: smi register write
--------------------------------------------
CLI_addCommand("debug", "smi register write", {
    func=smiRegisterWrite,
    help="Write data using smi",
    params={
        { type="named",
        { format="device %devID_all",name="devID",help="The device number" } ,
        { format="portgroup %portGroup",name="portGroup", help="The port-group number" } ,
        { format="interface %smiInterface",name="smiInterface", help="The phy interface number" } ,
        { format="address %phyAddr",name="phyAddr", help="The phy address" } ,
        { format="register %GT_U32",name="phyReg", help="The phy register number" } ,
        { format="data %GT_U16",name="data", help="The data to be written" } ,
        mandatory={"devID","portGroup","smiInterface","phyAddr","phyReg","data"},
        }
    }
})




-- ************************************************************************
--  smiRead
--
--  @description reads data from phy using the smi
--
--  @param devID        - The device number
--  @param portGroup    - The portgroup number
--  @param smiInterface - The interface of the smi
--  @param phyAddr      - The address of the phy
--  @param phyReg       - The phy register number number
--
--
-- ************************************************************************


local function smiRead(params)
	local res,val,devices,j

	if (params["portGroup"]=="all")  then params["portGroup"]=0xFFFFFFFF else params["portGroup"]=2^tonumber(params["portGroup"]) end
	if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end


	for j=1,#devices do

		res, val = myGenWrapper("cpssSmiRegisterReadShort", {
				{ "IN", "GT_U8"  , "devNum", devices[j]},
				{ "IN", "GT_U32" , "portGroupsBmp",  params["portGroup"]},
				{ "IN", TYPE["ENUM"], "smiInterface", params["smiInterface"]},
				{ "IN", "GT_U32"  , "smiAddr", params["phyAddr"]},  -- (0-31)
				{ "IN", "GT_U32"  , "regAddr", params["phyReg"]},
				{ "OUT", "GT_U16"  , "dataPtr"}
		})

		if (res==0) then print("Device :"..devices[j].."\tValue:"..string.format("\t0x%04X",val["dataPtr"]))
		else
			print("Error reading from device "..devices[j])

		end

	end

end

--------------------------------------------
-- command registration: smi read
--------------------------------------------
CLI_addCommand("debug", "smi read", {
    func=smiRead,
    help="Read data using smi",
    params={
        { type="named",
        { format="device %devID_all",name="devID",help="The device number" } ,
        { format="portgroup %portGroup",name="portGroup", help="The port-group number" } ,
        { format="interface %smiInterface",name="smiInterface", help="The phy interface number" } ,
        { format="address %phyAddr",name="phyAddr", help="The phy address" } ,
        { format="register %GT_U32",name="phyReg", help="The phy register number" } ,
        mandatory={"devID","portGroup","smiInterface","phyAddr","phyReg"},
        }
    }
})

-- ************************************************************************
--  smiWrite
--
--  @description write data to a phy using the smi
--
--  @param devID    - The device number
--  @param portGroup    - The portgroup number
--  @param smiInterface    - The interface of the smi
--  @param address    - The address of the phy
--  @param phyReg    - The phy register number number
--  @param data    - The data to be written
--
--
-- ************************************************************************
local function smiWrite(params)
	local res,devices,j

	if (params["portGroup"]=="all")  then params["portGroup"]=0xFFFFFFFF else params["portGroup"]=2^tonumber(params["portGroup"]) end
	if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end


	for j=1,#devices do

		res = myGenWrapper("cpssSmiRegisterWriteShort", {
				{ "IN", "GT_U8"  , "devNum", devices[j]},
				{ "IN", "GT_U32" , "portGroupsBmp",  params["portGroup"]},
				{ "IN", TYPE["ENUM"], "smiInterface", params["smiInterface"]},
				{ "IN", "GT_U32"  , "smiAddr", params["phyAddr"]}, -- (0-31)
				{ "IN", "GT_U32"  , "regAddr", params["phyReg"]},
				{ "IN", "GT_U16"  , "dataPtr",params["data"]}
		})

		if (res~=0) then print("Error writing to device "..devices[j]) end

	end

end

--------------------------------------------
-- command registration: smi write
--------------------------------------------
CLI_addCommand("debug", "smi write", {
    func=smiWrite,
    help="Write data using smi",
    params={
        { type="named",
        { format="device %devID_all",name="devID",help="The device number" } ,
        { format="portgroup %portGroup",name="portGroup", help="The port-group number" } ,
        { format="interface %smiInterface",name="smiInterface", help="The phy interface number" } ,
        { format="address %phyAddr",name="phyAddr", help="The phy address" } ,
        { format="register %GT_U32",name="phyReg", help="The phy register number" } ,
        { format="data %GT_U16",name="data", help="The data to be written" } ,
        mandatory={"devID","portGroup","smiInterface","phyAddr","phyReg","data"},
        }
    }
})


-- ************************************************************************
--  smiInfo
--
--  @description displays the address and interface of the chosen phy
--
--  @param devID    - The device number
--  @param port     - The port number
--
--
-- ************************************************************************
local function smiInfo(params)
	local res1,res2,val1,val2,devices,j,ports,i

	if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

	for j=1,#devices do
		if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end

		for i=1,#ports do
			res1, val1 = myGenWrapper("cpssDxChPhyPortSmiInterfaceGet", {
					{ "IN", "GT_U8"  , "devNum", devices[j]},
					{ "IN", "GT_U32" , "portNum",  ports[i]},
					{ "OUT", TYPE["ENUM"], "smiInterfacePtr"},
			})

			res2, val2 = myGenWrapper("cpssDxChPhyPortAddrGet", {
					{ "IN", "GT_U8"  , "devNum", devices[j]},
					{ "IN", "GT_U32" , "portNum",  ports[i]},
					{ "OUT","GT_U8", "phyAddFromHwPtr"},
					{ "OUT","GT_U8", "phyAddFromDbPtr"},
			})

			if (res1==0) and (res2==0) then
				print(string.format("Device:%2d  Port:%2d  Interface:%2d  HW address:0x%02X  DB address:0x%02X",devices[j],ports[i],val1["smiInterfacePtr"],val2["phyAddFromHwPtr"],val2["phyAddFromDbPtr"]))
			elseif (res1~=18) then
				print("Could not get device "..devices[j] ..  " Port "..ports[i].." data")
			end
		end
	end
end

--------------------------------------------
-- command registration: smi info
--------------------------------------------
CLI_addCommand("debug", "smi info", {
    func=smiInfo,
	help="Display port phy information",
	params={
		{ type="named",
		{ format="device %devID_all",name="devID",help="The device number" } ,
		{ format="port %portNum",name="portNum",help="The port number" } ,
		mandatory={"devID"},
		requirement={["portNum"]={"devID"}}
		}
	}
})
