--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* eom.lua
--*
--* DESCRIPTION:
--*       serdes eom commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************
cmdLuaCLI_registerCfunction("wrlPxPortSerdesEyeGet")

local function eomGetMatrix(params)
    local lowerMatrix,upperMatrix,ret,val,dfe,baudRate,devNum,portNum
    local filename

    devNum = params["devPort"]["devId"]
    portNum = params["devPort"]["portNum"]

    local x_points
    local y_points
    local matrix
    local serdesNum__ = params["serdesNum"]
    local minDwellBits_ = params["minDwellBits"]
    local maxDwellBits_ = params["maxDwellBits"]
    local noEye_ = params["noEye"]

    if(noEye_ == nill) then
        noEye_ = 0
    else
        noEye_ = 1
    end
    if(minDwellBits_ == nill) then
        minDwellBits_ = 100000
    end
    if(maxDwellBits_ == nill) then
        maxDwellBits_ = 100000000
    end

    x_points, y_points , matrix = wrLogWrapper("wrlPxPortSerdesEyeGet",
                                        "(devNum,portNum, serdesNum__, minDwellBits_, maxDwellBits_, noEye_)",
                                        devNum,portNum, serdesNum__, minDwellBits_, maxDwellBits_, noEye_)
    if(x_points ~= nil) then
        print(matrix)
    else
        print("Could not get EOM matrix")
    end
end

--------------------------------------------------------
-- command registration: eom matrix
--------------------------------------------------------
 CLI_addHelp("debug", "eom", "Retreive EOM parameters")
 CLI_addCommand("debug", "eom matrix", {
    func=eomGetMatrix,
    help="Prints the sampled Matrix",
    params={
        {   type="named",
            { format="port %dev_port", name="devPort", help="Device number" },
            { format="serdes %GT_U32", name="serdesNum", help="Local serdes number" },
            { format="min-dwell %GT_U32", name="minDwellBits", help="Minimum dwell bits. Range: 100000 - 100000000 (caelum and above)" },
            { format="max-dwell %GT_U32", name="maxDwellBits", help="Maximum dwell bits. Range: 100000 - 100000000 (caelum and above)" },
            { format="noeye", name="noEye", help="Don't print the eye on screen" },
            mandatory={"devPort","serdesNum"}
        }
    }
})






