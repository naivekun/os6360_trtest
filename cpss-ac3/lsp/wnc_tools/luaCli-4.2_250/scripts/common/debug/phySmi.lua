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
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

CLI_type_dict["phyReg"] = {
    checker = CLI_check_param_number,
    min=0,
    max=31,
    help="Phy register number"
}

CLI_type_dict["phyID"] = {
    checker = CLI_check_param_number,
    min=0,
    max=31,
    help="Phy ID number"
}

CLI_type_dict["phyDev"] = {
    checker = CLI_check_param_number,
    min=0,
    max=31,
    help="Phy device number"
}

CLI_type_dict["phyAddr"] = {
    checker = CLI_check_param_number,
    min=0,
    max=31,
    help="Phy address"
}

--constants


-- ************************************************************************
--  xsmiRead
--
--  @description Read value from a specified XSMI Register
--
--  @param devID     - The device number
--  @param portGroup - The portgroup number
--  @param xsmiInterface - The XSMI interface instance
--  @param xsmiAddr  - Address of register of configured device
--  @param regAddr   - The phy register number
--  @param phyDev    - Address of configured device on XSMI
--
--
-- ************************************************************************
local function xsmiRead(params)
    local res,val,devices,j

    if (params["portGroup"]=="all")  then params["portGroup"]=0xFFFFFFFF else params["portGroup"]=2^tonumber(params["portGroup"]) end
    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        res, val = myGenWrapper("cpssXsmiPortGroupRegisterRead", {
                { "IN", "GT_U8"  , "devNum", devices[j]},
                { "IN", "GT_U32" , "portGroupsBmp",  params["portGroup"]},
                { "IN", TYPE["ENUM"], "xsmiInterface", params["xsmiInterface"]},
                { "IN", "GT_U32"  , "xsmiAddr", params["xsmiAddr"]}, -- (0-31)
                { "IN", "GT_U32"  , "regAddr", params["regAddr"]},
                { "IN", "GT_U32"  , "phyDev", params["phyDev"]}, -- (0-31)
                { "OUT", "GT_U16"  , "dataPtr"}
        })

        if (res==0) then
            print("Device :"..devices[j].."\tValue:"..string.format("\t0x%04X",val["dataPtr"]))
        else
            print("Error reading from device "..devices[j])
        end
    end
end


--instead 0 should be "CPSS_PHY_XSMI_INTERFACE_0_E"
CLI_type_dict["xsmiInterface"] = {
    help = "Xsmi Interface\n",
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    enum = {
        ["0"] = { value=0, help="XSMI0 master interface number" },
        ["1"] = { value=1, help="XSMI1 master interface number" }
   }
}
CLI_type_dict["xsmiPhyDev"] = {
    help="The PHY device",
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=0,
    max=31
}

--------------------------------------------
-- command registration: xsmi register read
--------------------------------------------
CLI_addHelp("debug", "xsmi", "XSMI sub-commands")
CLI_addHelp("debug", "xsmi register", "XSMI register sub-commands")

CLI_addCommand("debug", "xsmi register read", {
    func=xsmiRead,
    help="Read data using xsmi",
    params={
        { type="named",
        { format="device %devID_all",name="devID",help="The device number" } ,
        { format="portgroup %portGroup",name="portGroup", help="The port-group number" } ,
        { format="interface %xsmiInterface",name="xsmiInterface", help="The XSMI interface number" },
        { format="address %phyAddr",name="xsmiAddr", help="The phy address" } ,
        { format="register %GT_U32",name="regAddr", help="The phy register number" } ,
        { format="phydev %xsmiPhyDev",name="phyDev", help="The phy device to read from" } ,
        mandatory={"devID","portGroup", "xsmiInterface", "xsmiAddr","regAddr","phyDev"},
        }
    }
})


-- ************************************************************************
--  xsmiWrite
--
--  @description Write value to a specified XSMI Register
--
--  @param devID     - The device number
--  @param portGroup - The portgroup number
--  @param xsmiInterface - The XSMI interface instance
--  @param xsmiAddr  - Address of register of configured device
--  @param regAddr   - The phy register number
--  @param phyDev    - Address of configured device on XSMI
--  @param data      - The data to write
--
--
-- ************************************************************************
local function xsmiWrite(params)
	local res,val,devices,j

	if (params["portGroup"]=="all")  then params["portGroup"]=0xFFFFFFFF else params["portGroup"]=2^tonumber(params["portGroup"]) end
	if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

        for j=1,#devices do
            res, val = myGenWrapper("cpssXsmiPortGroupRegisterWrite", {
                    { "IN", "GT_U8"  , "devNum", devices[j]},
                    { "IN", "GT_U32" , "portGroupsBmp",  params["portGroup"]},
                    { "IN", TYPE["ENUM"], "xsmiInterface", params["xsmiInterface"]},
                    { "IN", "GT_U32"  , "xsmiAddr", params["xsmiAddr"]},  -- (0-31)
                    { "IN", "GT_U32"  , "regAddr", params["regAddr"]},
                    { "IN", "GT_U32"  , "phyDev", params["phyDev"]},  -- (0-31)
                    { "IN", "GT_U16"  , "data",params["data"]}
            })
            if (res~=0) then print("Error reading from device "..devices[j]) end
        end
end

--------------------------------------------
-- command registration: xsmi register write
--------------------------------------------
CLI_addCommand("debug", "xsmi register write", {
    func=xsmiWrite,
    help="Write data using xsmi",
    params={
        { type="named",
        { format="device %devID_all",name="devID",help="The device number" } ,
        { format="portgroup %portGroup",name="portGroup", help="The port-group number" } ,
        { format="interface %xsmiInterface",name="xsmiInterface", help="The XSMI interface number" },
        { format="address %phyAddr",name="xsmiAddr", help="The phy address" } ,
        { format="register %GT_U32",name="regAddr", help="The phy register number" } ,
        { format="phydev %xsmiPhyDev",name="phyDev", help="The phy device to read from" } ,
        { format="data %GT_U16",name="data", help="The data to be written" } ,
        mandatory={"devID","portGroup", "xsmiInterface", "xsmiAddr","regAddr","phyDev","data"},
        }
    }
})

if dxCh_family == true then
    require_safe_dx("debug/phySmi")
end

if px_family == true then
    require_safe_px("debug/phySmi")
end
