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

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        res, val = myGenWrapper("cpssSmiRegisterReadShort", {
                { "IN", "GT_U8"  , "devNum",  devices[j]},
                { "IN", "GT_PORT_GROUPS_BMP", "portGroupsBmp", 0xFFFFFFFF},
                { "IN", "CPSS_PHY_SMI_INTERFACE_ENT", "smiInterface", "CPSS_PHY_SMI_INTERFACE_0_E"},
                { "IN", "GT_U32" , "smiAddr",  params["phyID"]},
                { "IN", "GT_U32" , "regAddr",  params["phyReg"]}, --(0-31)
                { "OUT", "GT_U16", "dataPtr"}
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
        { format="device %devID_all",name="devID",  help="The device number" },
        { format="phyID %phyID",     name="phyID",  help="The phy ID number" },
        { format="register %phyReg", name="phyReg", help="Phy register number (0..31)" } ,
        mandatory={"devID", "phyReg", "phyID"},
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

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        res, val = myGenWrapper("cpssSmiRegisterWriteShort", {
                { "IN", "GT_U8"  , "devNum",   devices[j]},
                { "IN", "GT_PORT_GROUPS_BMP", "portGroupsBmp", 0xFFFFFFFF},
                { "IN", "CPSS_PHY_SMI_INTERFACE_ENT", "smiInterface", "CPSS_PHY_SMI_INTERFACE_0_E"},
                { "IN", "GT_U32" , "smiAddr",  params["phyID"]},
                { "IN", "GT_U32" , "regAddr",  params["phyReg"]}, --(0-31)
                { "OUT", "GT_U16", "dataPtr",  params["data"]}
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
        { format="device %devID_all",name="devID",  help="The device number" },
        { format="phyID %phyID",     name="phyID",  help="The phy ID number" },
        { format="register %phyReg", name="phyReg", help="Phy register number (0..31)" },
        { format="data %GT_U16",     name="data",   help="The data to write" },
        mandatory={"devID", "phyReg", "data", "phyID"},
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

    for j=1,#devices do
        res, val = myGenWrapper("cpssXsmiPortGroupRegisterRead", {
            { "IN",  "GT_U8"             , "devNum", devices[j]},
            { "IN",  "GT_PORT_GROUPS_BMP", "portGroupsBmp", 0xFFFFFFFF},
            { "IN",  "CPSS_PHY_XSMI_INTERFACE_ENT", "xsmiInterface", "CPSS_PHY_XSMI_INTERFACE_0_E"},
            { "IN",  "GT_U32", "xsmiAddr", params["phyID"]},  --(0-31)
            { "IN",  "GT_U32", "regAddr" , params["phyReg"]},
            { "IN",  "GT_U32", "phyDev"  , params["phyDev"]}, --(0-31)
            { "OUT", "GT_U16", "dataPtr"}
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
        { format="device %devID_all",name="devID",  help="The device number" },
        { format="register %GT_U16", name="phyReg", help="The phy register number" },
        { format="phyDev %phyDev",   name="phyDev", help="The phy device number (0-31)" },
        { format="phyID %phyID",     name="phyID",  help="The phy ID number" },
        mandatory={"devID", "phyReg", "phyDev", "phyID"},
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

    for j=1,#devices do
        res, val = myGenWrapper("cpssXsmiPortGroupRegisterWrite", {
            { "IN",  "GT_U8"             , "devNum", devices[j]},
            { "IN",  "GT_PORT_GROUPS_BMP", "portGroupsBmp", 0xFFFFFFFF},
            { "IN",  "CPSS_PHY_XSMI_INTERFACE_ENT", "xsmiInterface", "CPSS_PHY_XSMI_INTERFACE_0_E"},
            { "IN",  "GT_U32", "xsmiAddr", params["phyID"]},  --(0-31)
            { "IN",  "GT_U32", "regAddr" , params["phyReg"]},
            { "IN",  "GT_U32", "phyDev"  , params["phyDev"]}, --(0-31)
            { "IN",  "GT_U16", "dataPtr" , params["data"]}
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
        { format="device %devID_all",name="devID",  help="The device number" } ,
        { format="register %GT_U16", name="phyReg", help="The phy register number" } ,
        { format="phyDev %phyDev",   name="phyDev", help="The phy device number" } ,
        { format="phyID %phyID",     name="phyID",  help="The phy ID number" } ,
        { format="data %GT_U16",     name="data",   help="The data to write" } ,
        mandatory={"devID", "phyReg", "phyDev", "phyID", "data"},
        }
    }
})
