--********************************************************************************
--*              (c), Copyright 2014, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pbr_utils.lua
--*
--* DESCRIPTION:
--*       Policy Based Routing functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- input parameter: lpmDBId
-- returns: rc and true/false or text-error-message
cmdLuaCLI_registerCfunction("wrlCpssDxChIsRoutingPolicyBased");

-- enables/disables port for PBR
-- devNum:  device number
-- portNum: port number
-- enable:  true - enable, false - disable
function pbr_port_enable(devNum, portNum, enable, pcl_id)
    local ret, val, access_mode;
    local groupKeyTypes = {};
    local lookupCfg = {};
    local interfaceInfo = {};

    -- if no PCL Based Routing on lpmDBId == 0 norhing to do
    ret, val = wrlCpssDxChIsRoutingPolicyBased(0);
    if (ret~=0) then
        print(
            "Error in wrlCpssDxChIsRoutingPolicyBased: " ..
            " ret " .. tostring(ret) .. " error: " .. tostring(val))
    end
    if not val then
        return
    end

    -- enable PCL any case
    ret,val = myGenWrapper("cpssDxChPclPortIngressPolicyEnable",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PORT_NUM","portNum",portNum},
        {"IN","GT_BOOL","enable", true},
    })
    if (ret~=0) then
        print(
            "Error in cpssDxChPclPortIngressPolicyEnable: " ..
            " dev " .. tostring(devNum) .. " port " .. tostring(portNum) ..
            " ret " .. tostring(ret))
    end

    access_mode = "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E";

    ret,val = myGenWrapper("cpssDxChPclPortLookupCfgTabAccessModeSet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PORT_NUM","portNum",portNum},
        {"IN","CPSS_PCL_DIRECTION_ENT","direction", "CPSS_PCL_DIRECTION_INGRESS_E"},
        {"IN","CPSS_PCL_LOOKUP_NUMBER_ENT","lookupNum", "CPSS_PCL_LOOKUP_1_E"},
        {"IN","GT_U32","subLookupNum", 0},
        {"IN","CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT","mode", access_mode},
    })
    if (ret~=0) then
        print(
            "Error in : cpssDxChPclPortLookupCfgTabAccessModeSet" ..
            " dev " .. tostring(devNum) .. " port " .. tostring(portNum) ..
            " ret " .. tostring(ret))
    end

    interfaceInfo.devPort = {}
    interfaceInfo.devPort.devNum    = device_to_hardware_format_convert(devNum);
    interfaceInfo.devPort.hwDevNum  = interfaceInfo.devPort.devNum;
    interfaceInfo.devPort.portNum   = portNum

    groupKeyTypes.nonIpKey = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E";
    groupKeyTypes.ipv4Key  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E";
    groupKeyTypes.ipv6Key  = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E";

    lookupCfg.enableLookup = enable;
    lookupCfg.pclId = pcl_id;
    lookupCfg.groupKeyTypes = groupKeyTypes;

    ret,val = myGenWrapper("cpssDxChPclCfgTblSet",{
        {"IN","GT_U8","devNum", devNum},
        {"IN","CPSS_INTERFACE_INFO_STC","interfaceInfoPtr",interfaceInfo},
        {"IN","CPSS_PCL_DIRECTION_ENT","direction", "CPSS_PCL_DIRECTION_INGRESS_E"},
        {"IN","CPSS_PCL_LOOKUP_NUMBER_ENT","lookupNum", "CPSS_PCL_LOOKUP_1_E"},
        {"IN","CPSS_DXCH_PCL_LOOKUP_CFG_STC","lookupCfgPtr", lookupCfg},
    })
    if (ret~=0) then
        print(
            "Error in : cpssDxChPclCfgTblSet" ..
            " dev " .. tostring(devNum) .. " port " .. tostring(portNum) ..
            " ret " .. tostring(ret))
    end
end

function pbr_set_default_cfg_tab_access_mode()
    local sysInfo = luaCLI_getDevInfo();
    local ret, val, access_mode, devNum, ports;

    if type(sysInfo) ~= "table" or
        (not next(sysInfo))
    then
        -- do nothing if called before Init System
        return;
    end

    access_mode = {
        ipclAccMode     = "CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E",
        ipclDevPortBase = "CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E",
        ipclMaxDevPorts = "CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_32_E",
        epclAccMode     = "CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E",
        epclDevPortBase = "CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E",
        epclMaxDevPorts = "CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_32_E",
    };

    for devNum, ports in pairs(sysInfo) do
        ret,val = myGenWrapper("cpssDxChPclCfgTblAccessModeSet",{
            {"IN","GT_U8","devNum", devNum},
            {"IN","CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC","accModePtr",access_mode},
        })
        if (ret~=0) then
            print(
                "Error in : cpssDxChPclCfgTblAccessModeSet" ..
                " dev " .. tostring(devNum) .. " ret " .. tostring(ret))
        end
        print("Default PCL CFG Table Access mode configured on dev: " .. tostring(devNum));
    end
end

-- called on Reload LUA files
-- configire all devices regardless to PBR using
pbr_set_default_cfg_tab_access_mode();
