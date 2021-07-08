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
cmdLuaCLI_registerCfunction("wrlDxChPortEomMatrixGet")
cmdLuaCLI_registerCfunction("wrlDxChPortSerdesEyeGet")

local function eomGetMatrix(params)
    local lowerMatrix,upperMatrix,ret,val,dfe,baudRate,devNum,portNum
    local filename

    devNum = params["devPort"]["devId"]
    portNum = params["devPort"]["portNum"]

    local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(devNum)
    if devSubFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E"
           or devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E"
           or devFamily == "CPSS_PP_FAMILY_DXCH_AC3X_E"
    then
        if (params["filename"] ~= nil) then
            print("Not applicabale parameter \"file-name\" for this device")
        elseif (params["samplingTime"] ~= nil) then
            print("Not applicabale parameter \"sample-time\" for this device")
        else
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

            x_points, y_points , matrix = wrLogWrapper("wrlDxChPortSerdesEyeGet",
                                               "(devNum,portNum, serdesNum__, minDwellBits_, maxDwellBits_, noEye_)",
                                              devNum,portNum, serdesNum__, minDwellBits_, maxDwellBits_, noEye_)
           if(x_points ~= nil) then
               print(matrix)
           else
               print("Could not get EOM matrix")
            end
        end
    else

        if (params["minDwellBits"] ~= nil) then
            print("Not applicabale parameter \"min-dwell\" for this device")
        elseif (params["maxDwellBits"] ~= nil) then
            print("Not applicabale parameter \"max-dwell\" for this device")
        elseif (params["noEye"] ~= nil) then
            print("Not applicabale parameter \"noeye\" for this device")
        else
            if (params["filename"] ~= nil) then
                filename = params["filename"]
            else
                filename = "EOMMatrix.xml"
            end

            ret, val = myGenWrapper(
              "cpssDxChPortEomBaudRateGet",
              {
                 { "IN", "GT_U8",   "devNum", devNum },
                 { "IN", "GT_U32",  "portNum", portNum },
                 { "IN", "GT_U32",  "serdesNum", params["serdesNum"] },
                 { "OUT", "GT_U32", "baudRate"},
              })

            if (ret~=0) then
                print("Could not get baud rate for serdes, error num ".. ret)
            else
                baudRate= val["baudRate"]
                print("Baud rate:"..baudRate)
            end


            ret, val = myGenWrapper(
              "cpssDxChPortEomDfeResGet",
              {
                 { "IN", "GT_U8",   "devNum", devNum },
                 { "IN", "GT_U32",  "portNum", portNum},
                 { "IN", "GT_U32",  "serdesNum", params["serdesNum"] },
                 { "OUT", "GT_U32", "dfeRes"}
              })

            if (ret~=0) then
                print("Could not get DFE resolution for serdes, error num ".. ret)
            else
                dfe=val["dfeRes"]
                print("DFE resolution:"..dfe)
            end


            if params["samplingTime"]==nil then params["samplingTime"]=100 end
            
            local serdesNum_ = params["serdesNum"]
            local samplingTime_ = params["samplingTime"]
            lowerMatrix,upperMatrix = wrLogWrapper("wrlDxChPortEomMatrixGet",
                                                   "(devNum,portNum, serdesNum_, samplingTime_)",
                                                   devNum,portNum, serdesNum_, samplingTime_)

            if (lowerMatrix==nil) then
                if upperMatrix~=nil then
                    print("Could not get EOM matrix, "..upperMatrix)
                else
                    print("Could not get EOM matrix, matrices returned empty, please check port link")
                end
            else
                local rootNode,rootEntry,infoEntry,matrixNode,node,i,j,str
                rootNode = xml_newXml()
                rootEntry = xml_newElement(rootNode,"ROOT")

                infoEntry = xml_newElement(rootEntry,"Info")
                xml_addAttrib(infoEntry,"Timeout",params["samplingTime"])
                xml_addAttrib(infoEntry,"BaudRate",baudRate)
                xml_addAttrib(infoEntry,"DFE",dfe)

                node = xml_newElement(infoEntry,"CpssSI")
                xml_addAttrib(node,"DevID",devNum)
                xml_addAttrib(node,"globalPortID",portNum)
                xml_addAttrib(node,"LaneID",params["serdesNum"])

                matrixNode = xml_newElement(rootEntry,"matrix")
                xml_addAttrib(matrixNode,"Rows",#lowerMatrix+#upperMatrix)
                xml_addAttrib(matrixNode,"Cols",#lowerMatrix[1])

                for i=#upperMatrix,1,-1  do
                    node = xml_newElement(matrixNode,"row")
                    str=""
                    for j=1,#upperMatrix[i] do str=str..string.format("%08X",upperMatrix[i][j]) end
                    xml_addAttrib(node,"Cells",str)
                end

                for i=1,#lowerMatrix do
                    node = xml_newElement(matrixNode,"row")
                    str=""
                    for j=1,#lowerMatrix[i] do str=str..string.format("%08X",lowerMatrix[i][j]) end
                    xml_addAttrib(node,"Cells",str)
                end

                if fs.fileexists(filename) then copy_file(filename, filename .. ".bak") end
                fs.unlink(filename)
                xml_saveFile(rootNode,filename)
                xml_delete(rootNode)
            --	printMatrix(lowerMatrix)
            end
        end
    end
end


--[[
function printMatrix(matrix,rows)
    local i,j,str

    if rows==nil or rows>#matrix then rows=#matrix end
    for i=1,rows do
        str=i..":"
        for j=1,#matrix[1] do
            str=str..string.format("%6d",matrix[i][j]).." | "
        end
        print(str)
    end
end
]]--

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
            { format="sample-time %GT_U32", name="samplingTime", help="Sampling time (lion2 only)" },
            { format="min-dwell %GT_U32", name="minDwellBits", help="Minimum dwell bits. Range: 100000 - 100000000 (caelum and above)" },
            { format="max-dwell %GT_U32", name="maxDwellBits", help="Maximum dwell bits. Range: 100000 - 100000000 (caelum and above)" },
            { format="noeye", name="noEye", help="Don't print the eye on screen" }, 
            { format="file-name %filename", name="filename", help="file name" },
            mandatory={"devPort","serdesNum"}
        }
    }
})






