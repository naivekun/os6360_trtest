--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_utils.lua
--*
--* DESCRIPTION:
--*       serdes utils
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--[[
the functions are :

]]--
-- ************************************************************************

function PxGetNuberSerdesOfPort(devNum,portNum)

    local ret,val
    local ifMode
    local port_interface_mode
    local numOfSerdesLanesPtr=0



    ret,val = cpssPerPortParamGet("cpssPxPortInterfaceModeGet",
                                         devNum, portNum, "mode",
                                         "CPSS_PORT_INTERFACE_MODE_ENT")
    if 0 == ret then

        port_interface_mode = val["mode"]
        if port_interface_mode ~= "CPSS_PORT_INTERFACE_MODE_NA_E" then

            --print("port_interface_mode " .. port_interface_mode .. " of device ".. tostring(devNum) .. " port " .. tostring(portNum))

            ret, val = myGenWrapper("cpssPxPortInterfaceModeGet",{
                {"IN","GT_SW_DEV_NUM","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})

            if(ret~=0) then
                --print(tostring(devNum).."/"..tostring(portNum).." Error cpssPxPortInterfaceModeGet ret "..to_string(ret).." = "..returnCodes[ret])
                return ret,0
            end

            ifMode=val["ifModePtr"]

            ret, val = myGenWrapper("prvCpssPxPortIfModeSerdesNumGet",{
                {"IN","GT_SW_DEV_NUM","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                {"OUT","GT_U32","startSerdesPtr"},
                {"OUT","GT_U32","numOfSerdesLanesPtr"}})

            if(ret~=0) then
                --print(tostring(devNum).."/"..tostring(portNum).." Error prvCpssPxPortIfModeSerdesNumGet ret "..to_string(ret).." = "..returnCodes[ret])
                return ret,0
            end

            startSerdesPtr=val["startSerdesPtr"]
            numOfSerdesLanesPtr=val["numOfSerdesLanesPtr"]


        end --if val["mode"] ~= "CPSS_PORT_INTERFACE_MODE_QSGMII_Eof"

    else
        --print("Error at interface mode getting cpssPxPortInterfaceModeGet " ..
        --                      "of device %d port %d: %s", devNum,
        --                      portNum, returnCodes[ret])
    end


    if(ret==0 and numOfSerdesLanesPtr ~=nil and numOfSerdesLanesPtr > 0  ) then
        return ret,numOfSerdesLanesPtr
    else
        return ret,0
    end

end

