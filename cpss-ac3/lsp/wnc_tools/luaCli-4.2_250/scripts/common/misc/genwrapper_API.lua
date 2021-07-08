--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* genwrapper_API.lua
--*
--* DESCRIPTION:
--*       general lua wrapper functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes


--constants


--
-- CPSS API descriptions table
--
-- Format:
--      cpssAPIs[cpssAPIname] = paramsDescription
--                          See myGenWrapper() description for details
--                          Only three columns required
cpssAPIs={}

--
-- Map common function to cpss API by device type
-- Format:
--      cpssAPICommon[deviceFamily][name] = cpssAPIname
cpssAPICommon={}
cpssAPICommon["DX"]={}
cpssAPICommon["SX"]={}
cpssAPICommon["EX"]={}
cpssAPICommon["EXMXPM"]={}

--
-- Map deviceId to family
cpssDeviceFamily={}


-- ************************************************************************
---
--  cpssAPIcall
--        @description  Call CPSS API function. This function search for 
--                      parameter description table in cpssAPIs table.
--                      Example: 
--                          result, values = cpssAPIcall(
--                              "cpssDxChBrgFdbMacEntryRead", 
--                          {
--                              devNum = 0,
--                              index = indexCnt
--                          })
--
--        @param cpssAPIname    - Fx: "prvCpssDrvHwPpReadRegister"
--        @param params         - table: parameter name -> value
--        @param strict         - when this parameter is true then raise 
--                                error when IN or INOUT parameter is not 
--                                defined
--
--        @return       GT_STATUS, array of output values string if error
--
function cpssAPIcall(cpssAPIname, params, strict)
    if type(params) ~= "table" then
        error("cpssAPIcall(): the second parameter must be table", 2)
    end
    -- map common function to family specific (optional)
    if params["devNum"] ~= nil then
        local family = cpssDeviceFamily[params["devNum"]]
        if family ~= nil then
            if cpssAPICommon[family] ~= nil then
                if cpssAPICommon[family][cpssAPIname] ~= nil then
                    cpssAPIname = cpssAPICommon[family][cpssAPIname]
                end
            end
        end
    end
    local desc = cpssAPIs[cpssAPIname]
    if desc == nil then
        error("cpssAPIcall(): no description for "..cpssAPIname, 2)
    end
    if type(desc) ~= "table" then
        error("cpssAPIcall(): wrong description for "..cpssAPIname, 2)
    end
    local cparams = {}
    local n
    for n = 1, #desc do
        if type(desc[n]) ~= "table" then
            error("cpssAPIcall(): wrong description for "..cpssAPIname, 2)
        end
        cparams[n] = duplicate(desc[n])
        if desc[n][1] == "IN" or desc[n][1] == "INOUT" then
            if params[desc[n][3]] ~= nil then
                cparams[n][4] = params[desc[n][3]]
            end
            if cparams[n][4] == nil and strict then
                error("cpssAPIcall(): parameter '"..desc[n][3].."' not defined for "..cpssAPIname, 2)
            end
        end
    end
    return myGenWrapper(cpssAPIname, cparams)
end


-- ************************************************************************
---
--  cpssPerDeviceParamGet
--        @description  Call one of CPSS API function with common API. The
--                      called function receive device number as parameter
--                      and return one parameter.
--                      Example: 
--                      result, values = cpssPerPortParamGet(
--                          "cpssDxChPortTxQueueEnableGet", 0)
--                      print("result=",result,"parameter=",values.param)
--
--        @param cpssAPIname    - Fx: "cpssDxChPortTxQueueEnableGet"
--        @param devNum         - device ID
--        @param paramName      - Fx: "enable", default "param"
--        @param paramType      - Fx: "GT_BOOL", default is "GT_U32"
--
--        @return       GT_STATUS, array of output values string if error
--
function cpssPerDeviceParamGet(cpssAPIname, devNum, paramName, paramType)
    if paramName == nil then
        paramName = "param"
    end
    if paramType == nil then
        paramType = "GT_U32"
    end
    return myGenWrapper(cpssAPIname, {
        { "IN", "GT_U8", "devNum", devNum },
        { "OUT", paramType, paramName }
    })
end


-- ************************************************************************
---
--  cpssPerDeviceParamSet
--        @description  Call one of CPSS API function with common API. The
--                      called function receive device number as parameter
--                      and set one parameter.
--                      Example:
--                      result, err = cpssPerPortParamGet(
--                          "cpssDxChPortTxQueueEnableSet", 0, 1)
--                      print("result=",result)
--
--        @param cpssAPIname    - Fx: "cpssDxChPortTxQueueEnableSet"
--        @param devNum         - device ID
--        @param param          - per device parameter to set
--        @param paramName      - Fx: "enable", default "param"
--        @param paramType      - Fx: "GT_BOOL", default is "GT_U32"
--
--        @return       GT_STATUS, array of output values string if error
--
function cpssPerDeviceParamSet(cpssAPIname, devNum, param, paramName, paramType)
    if paramName == nil then
        paramName = "param"
    end
    if paramType == nil then
        paramType = "GT_U32"
    end
    return myGenWrapper(cpssAPIname, {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", paramType, paramName, param }
    })
end


-- ************************************************************************
---
--  cpssPerPortParamGet
--        @description  Call one of CPSS API function with common API. The
--                      called function receive device and port numbers as 
--                      parameters and return one parameter.
--                      result, values = cpssPerPortParamGet(
--                          "cpssDxChPortTxShaperEnableGet", 0, 0)
--                      print("result=",result,"parameter=",values.param)
--
--        @param cpssAPIname    - Fx: "cpssDxChPortTxShaperEnableGet"
--        @param devNum         - device ID
--        @param portNum        - port number
--        @param paramName      - Fx: "enable", default "param"
--        @param paramType      - Fx: "GT_BOOL", default is "GT_U32"
--
--        @return       GT_STATUS, array of output values string if error
--
function cpssPerPortParamGet(cpssAPIname, devNum, portNum, paramName, paramType)
    if cpssAPIs[cpssAPIname] ~= nil then
        return cpssAPIcall(cpssAPIname, {
            devNum = devNum,
            portNum = portNum
        })
    end
    if paramName == nil then
        paramName = "param"
    end
    if paramType == nil then
        paramType = "GT_U32"
    end
    return myGenWrapper(cpssAPIname, {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "portNum", portNum },
        { "OUT", paramType, paramName }
    })
end


-- ************************************************************************
---
--  cpssPerPortParamSet
--        @description  Call one of CPSS API function with common API. The
--                      called function receive device and port numbers as 
--                      parameters and set one parameter
--                      result, err = cpssPerPortParamGet(
--                          "cpssDxChPortTxShaperEnableSet", 0, 0, 1)
--                      print("result=",result)
--
--        @param cpssAPIname    - Fx: "cpssDxChPortTxShaperEnableSet"
--        @param devNum         - device ID
--        @param portNum        - port number
--        @param param          - per device parameter to set
--        @param paramName      - Fx: "enable", default "param"
--        @param paramType      - Fx: "GT_BOOL", default is "GT_U32"
--
--        @return       GT_STATUS, array of output values string if error
--
function cpssPerPortParamSet(cpssAPIname, devNum, portNum, param, paramName, paramType)
    if paramName == nil then
        paramName = "param"
    end
    if cpssAPIs[cpssAPIname] ~= nil then
        return cpssAPIcall(cpssAPIname, {
            devNum = devNum,
            portNum = portNum,
            [paramName] = param
        })
    end
    if paramType == nil then
        paramType = "GT_U32"
    end
    return myGenWrapper(cpssAPIname, {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U32", "portNum", portNum },
        { "IN", paramType, paramName, param }
    })
end


-- ************************************************************************
---
--  cpssPerVlanParamSet
--        @description  Call one of CPSS API function with common API.
--                      The called function receive device and vlan 
--                      id as parameters and set one parameter. 
--                      result, err = 
--                          cpssPerTrunkParamSet("cpssDxChTrunkMemberAdd", 
--                                               devNum, vlanId, param, 
--                                               "member", 
--                                               "CPSS_TRUNK_MEMBER_STC")
--                      print("result=", result)
--
--        @param cpssAPIname        - Fx: "cpssDxChTrunkMemberAdd"
--        @param vlanId             - vlan id
--        @param param              - per device parameter to set
--        @param paramName          - Fx: "enable", default "param"
--        @param paramType          - Fx: "GT_BOOL", default is "GT_U32"
--
--        @return       GT_STATUS on success, otherwise false and error 
--                      message
--
function cpssPerVlanParamSet(cpssAPIname, devNum, vlanId, param, paramName, paramType)
    if paramName == nil then
        paramName = "param"
    end
    if paramType == nil then
        paramType = "GT_U32"
    end
    return myGenWrapper(cpssAPIname, {
        { "IN", "GT_U8", "devNum", devNum },
        { "IN", "GT_U16", "vlanId", vlanId },
        { "IN", paramType, paramName, param }
    })
end


-- ************************************************************************
---
--  cpssPerTrunkParamSet
--        @description  Call one of CPSS API function with common API. The
--                      called function receive device and trunk number as 
--                      parameters and set one parameter.
--                      Example:
--                      result, err = cpssPerTrunkParamSet(
--                          "cpssDxChTrunkMemberAdd", devNum, trunkId, 
--                          param, "member", "CPSS_TRUNK_MEMBER_STC")
--                      print("result=", result)
--
--        @param cpssAPIname    - Fx: "cpssDxChTrunkMemberAdd"
--        @param trunkId        - trunk ID
--        @param param          - per device parameter to set
--        @param paramName      - Fx: "enable", default "param"
--        @param paramType      - Fx: "GT_BOOL", default is "GT_U32"
--
--        @return       GT_STATUS on success, otherwise false and error message
--
function cpssPerTrunkParamSet(cpssAPIname, devNum, trunkId, param, paramName, paramType)
    if paramName == nil then
        paramName = "param"
    end
    if paramType == nil then
        paramType = "GT_U32"
    end
    return myGenWrapper(cpssAPIname, {
        { "IN", "GT_U8",        "devNum",   devNum  },
        { "IN", "GT_TRUNK_ID",  "trunkId",  trunkId },
        { "IN", paramType,      paramName,  param   }
    })
end

do

cmdLuaCLI_registerCfunction("wrlCpssCommonCpssVersionGet")
local result, version = wrlCpssCommonCpssVersionGet()
local vers="42"
if result == 0 then
    if string.match(version, "^CPSS_4.0_") ~= nil then
        vers="40"
    elseif string.match(version, "^CPSS_4.1_") ~= nil then
        vers="41"
    elseif string.match(version, "^CPSS_4.2_") ~= nil then
        vers="42"
    end
end
-- load debug/cpssAPIdata_$vers.lua first
if vers ~= nil and fs.stat("common/misc/cpssAPIdata_"..vers..".lua") ~= nil then
    require("common/misc/cpssAPIdata_"..vers)
end
-- load debug/cpssAPIdata.lua
if fs.stat("misc/cpssAPIdata.lua") ~= nil then
    require("common/misc/cpssAPIdata")
end


end
