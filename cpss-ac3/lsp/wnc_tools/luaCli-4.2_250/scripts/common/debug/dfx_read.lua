--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dfx_read.lua
--*
--* DESCRIPTION:
--*       reading of a dfx
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  dfx_read
--        @description  Reads from a dfx
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--
local function dfx_read(params)
    local result, values,j,devices,i,portGroups
    local status, err
    local step, num
    local k

    step = 0x0
    num  = 0x1

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end
    if (params["step"] ~= nil) then step = params["step"] end
    if (params["num"]  ~= nil) then num  = params["num"] end

    for j=1,#devices do
        local offset
        offset = params["offset"]
        for k=1, num do
	    status = true
	    --is it working on the right interface type, (not vlan or anything)
	    result, values = myGenWrapper(
	    	"cpssDrvHwPpResetAndInitControllerReadReg", {
		{ "IN", "GT_U8"  , "dev", devices[j]},    -- devNum
		{ "IN", "GT_U32" , "address",  offset},
		{ "OUT", "GT_U32", "regValue"},
		}
	    )
	    if (result ~= 0) then
		status = false
		err = returnCodes[result]
	    end
	    print(string.format("Device:%d\t Offset:0x%X\t value=0x%.8x", devices[j] , offset, values["regValue"]))
            offset = offset + step;
        end
    end

    return status, err
end

-- *debug*
-- dfx read device %devID offset %GT_U32_hex mask %GT_U32_hex
CLI_addHelp("debug", "dfx-register", "dfx related command")
CLI_addCommand("debug", "dfx-register read", {
    func = dfx_read,
    help = "Read a dfx",
    params={
         { type="named",
         { format="device %devID_all",name="devID", help="The device number" },
         { format="offset %register_address", help="The address of the data to read (hex)" },
         { format="step %GT_U32_hex", help="The step of shifting offset"},
         { format="num %GT_U32_hex",  help="Number of iterations"},
           requirements = {
            ["offset"] = {"devID"},
            ["step"] = {"offset"},
            ["num"] =  {"step"},
         },
         mandatory = {"offset"}
        }
    }
})
