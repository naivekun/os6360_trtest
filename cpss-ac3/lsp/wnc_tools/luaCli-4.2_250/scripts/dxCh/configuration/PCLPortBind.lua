--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PCLPortBind.lua
--*
--* DESCRIPTION:
--*       Binding the ports to the PCL ID
--*
--* FILE REVISION NUMBER:
--*       $Revision: 10 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssPclRuleIdtoPclIdGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChIngressExtendedUDBPclRuleParsedGet")

-- ************************************************************************
--  prvEnablePCLGlobal
--
--  @description enables the global PCL policy for a device
--
--  @param devNum    - The device number
--    @param enable    - true enables, false disables
--
--  @return true on success, otherwise false and error message
--
-- ************************************************************************
local function prvEnablePCLGlobal(devNum,enable)
    return myGenWrapper("cpssDxChPclIngressPolicyEnable",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_BOOL","enable",enable}
    })
end

-- ************************************************************************
--  prvEnablePCLPort
--
--  @description    enables the port PCL policy for a device
--
--  @param     devNum    - The device number
--    @param     portNum    - The port number
--    @param     enable    - true enables, false disables
--
--  @return true on success, otherwise false and error message
--
-- ************************************************************************
local function prvEnablePCLPort(devNum,portNum,enable)
    return myGenWrapper("cpssDxChPclPortIngressPolicyEnable",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PORT_NUM","portNum",portNum},
        {"IN","GT_BOOL","enable",enable}
    })
end

-- ************************************************************************
--  prvEnableEgressPCLGlobal
--
--  @description enables the global egress PCL policy for a device
--
--  @param devNum    - The device number
--    @param enable    - true enables, false disables
--
--  @return true on success, otherwise false and error message
--
-- ************************************************************************

local function prvEnableEgressPCLGlobal(devNum,enable)
    return myGenWrapper("cpssDxCh2PclEgressPolicyEnable",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_BOOL","enable",enable}
    })
end

-- ************************************************************************
--  prvEnableEgressPCLPort
--
--  @description    enables the port egress PCL policy for a device
--
--  @param     devNum    - The device number
--    @param     portNum    - The port number
--    @param     enable    - true enables, false disables
--
--  @return true on success, otherwise false and error message
--
-- ************************************************************************
local function prvEnableEgressPCLPort(devNum,portNum,enable)
    local CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E = 5
    return myGenWrapper("cpssDxCh2EgressPclPacketTypesSet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","port",portNum},
        {"IN","CPSS_DXCH_PCL_EGRESS_PKT_TYPE_ENT","pktType",CPSS_DXCH_PCL_EGRESS_PKT_NON_TS_E},
        {"IN","GT_BOOL","enable",enable}
    })
end


-- ************************************************************************
--  service_acl
--
--  @description  enables/disables the port PCL policy for a device
--
--  @param params - params[pclID]    - The PCL id used to bind/unbind the interface
--
--  @return true on success, otherwise false and error message
--
-- ************************************************************************
local udb_init -- forward declaration for udb_init()
local function service_acl(params)
    local command_data = Command_Data()
    local ret,val,interfaceInfo,lookupCfgPtr,lookupNum
    local tmpDevNum,portNum,devNum, hwDevNum, phisPortNum
    local direct, directNum
    local CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E = 7
    local CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E = 16

    ret = udb_init()
    if ret~=0 then
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error initialize UDB: ", returnCodes[ret])
    end

    if params["egress"] ~= nil then
        direct = "egress"
        directNum = 1
    else
        direct = "ingress"
        directNum = 0
    end

    tmpDevNum=-1
    interfaceInfo={}
    interfaceInfo["type"]=0 -- port
    interfaceInfo["devPort"]={}

    local pcl_rule_format, pcl_rule_format_ipv6
    local pcl_rule_format_cheetah, pcl_rule_format_ipv6_cheetah
    if direct == "ingress" then -- set the pcl-rule-format for ingress and egress
        pcl_rule_format = 14
        pcl_rule_format_ipv6 = pcl_rule_format
    else
        pcl_rule_format = 10
        pcl_rule_format_ipv6 = 11
    end
    -- CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E
    pcl_rule_format_cheetah = 4
    -- CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E
    pcl_rule_format_ipv6_cheetah = 5
    lookupCfgPtr={}
    lookupCfgPtr["pclId"]=params["pclID"]
    lookupCfgPtr["groupKeyTypes"]={}
    lookupCfgPtr["groupKeyTypes"]["nonIpKey"]=pcl_rule_format
    lookupCfgPtr["groupKeyTypes"]["ipv4Key"]=pcl_rule_format
    lookupCfgPtr["groupKeyTypes"]["ipv6Key"]=pcl_rule_format_ipv6
    lookupCfgPtr["enableLookup"]=true

    if (params["lookupNum"]~=1) then params["lookupNum"]=0 end

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            -- recalculate PCL key types for next device
            if tmpDevNum~=devNum then
                if generic_IsDeviceInFamilyList(devNum, {"CPSS_PP_FAMILY_CHEETAH_E"}) then
                    lookupCfgPtr["groupKeyTypes"]["nonIpKey"]=pcl_rule_format_cheetah
                    lookupCfgPtr["groupKeyTypes"]["ipv4Key"]=pcl_rule_format_cheetah
                    lookupCfgPtr["groupKeyTypes"]["ipv6Key"]=pcl_rule_format_ipv6_cheetah
                else
                    lookupCfgPtr["groupKeyTypes"]["nonIpKey"]=pcl_rule_format
                    lookupCfgPtr["groupKeyTypes"]["ipv4Key"]=pcl_rule_format
                    lookupCfgPtr["groupKeyTypes"]["ipv6Key"]=pcl_rule_format_ipv6
                end
            end

            -- enable global PCL
            if direct == "ingress" then
                if tmpDevNum~=devNum then
                    tmpDevNum=devNum
                    prvEnablePCLGlobal(devNum,true)
                    if (is_sip_5(devNum)) then
                        -- configure the UDBs needed for the match
                        -- UDB 30, Metadata TPID Index - offset 29
                        -- bits 0..5:  bits 0..2 --> Tag0 TPID Index, bits 3..5 --> Tag1 TPID Index
                        for pclPacketType = 0, CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E do
                            if (pclPacketType ~= CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E) then
                                ret,val =  myGenWrapper(
                                    "cpssDxChPclUserDefinedByteSet", {
                                    { "IN",     "GT_U8",                                "devNum",     devNum },
                                    { "IN",     "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT",   "ruleFormat", pcl_rule_format},
                                    { "IN",     "CPSS_DXCH_PCL_PACKET_TYPE_ENT",        "packetType", pclPacketType},
                                    { "IN",     "CPSS_PCL_DIRECTION_ENT",               "direction",  "CPSS_PCL_DIRECTION_INGRESS_E"},
                                    { "IN",     "GT_U32",                               "udbIndex",   30 },
                                    { "IN",     "CPSS_DXCH_PCL_OFFSET_TYPE_ENT",        "offsetType", "CPSS_DXCH_PCL_OFFSET_METADATA_E" },
                                    { "IN",     "GT_U8",                                "offset",     29}
                                })
                                if ret~=0 then
                                    command_data:setFailPortAndLocalStatus()
                                    command_data:addError("Error setting UDB at device %d packet type %d: %s", devNum, pclPacketType, returnCodes[ret])
                                end
                            end
                        end
                    end
                end
            else
                if tmpDevNum~=devNum then tmpDevNum=devNum prvEnableEgressPCLGlobal(devNum,true) end
            end

            --Enable the port
            if direct == "ingress" then
                ret,val = prvEnablePCLPort(devNum,portNum,true)
            else
                ret,val = prvEnableEgressPCLPort(devNum,portNum,true)
            end


            if ret~=0 then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error enabling ingress policy at device %d port %d: ", devNum, portNum,returnCodes[ret])
            else
                local physicalInfo = {}
                local OUT_values,result,did_error,apiName

                if is_sip_5(devNum) then
                    apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet"
                    did_error , result , OUT_values = genericCpssApiWithErrorHandler(command_data,
                        apiName, {
                            { "IN", "GT_U8", "devNum", devNum},
                            { "IN", "GT_PORT_NUM", "portNum", portNum},
                            { "OUT", "CPSS_INTERFACE_INFO_STC", "physicalInfo"}
                        })

                    physicalInfo = OUT_values.physicalInfo;
                    hwDevNum    = physicalInfo.devPort.hwDevNum
                    phisPortNum = physicalInfo.devPort.portNum;
                else
                    local result, values = myGenWrapper(
                    "cpssDxChCfgHwDevNumGet", {
                        { "IN", "GT_U8" , "dev", devNum},
                        { "OUT","GT_U8" , "hwDevNumPtr"},
                    });
                    hwDevNum    = values.hwDevNumPtr;
                    phisPortNum = portNum;
                end

                ret,val = cpssAPIcall("cpssDxChPclPortLookupCfgTabAccessModeSet",{
                        devNum = devNum,
                        portNum = phisPortNum,
                        direction = directNum,
                        lookupNum = params["lookupNum"],
                        subLookupNum = 0,
                        mode = 0,
                        entryIndex = 0 })

                if ret~=0 then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error setting lookup config table acces mode at device %d port %d: ", devNum, portNum,returnCodes[ret])
                else
                    interfaceInfo["devPort"]["devNum"]  = hwDevNum
                    interfaceInfo["devPort"]["portNum"] = phisPortNum

                    ret,val = myGenWrapper("cpssDxChPclCfgTblSet",{
                        {"IN","GT_U8","devNum", devNum},
                        {"IN","CPSS_INTERFACE_INFO_STC","interfaceInfoPtr",interfaceInfo},
                        {"IN",TYPE["ENUM"],"direction",directNum},
                        {"IN",TYPE["ENUM"],"lookupNum",params["lookupNum"]},
                        {"IN","CPSS_DXCH_PCL_LOOKUP_CFG_STC","lookupCfgPtr",lookupCfgPtr}
                    })

                    if ret~=0 then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error configuring config table at device %d port %d: ", devNum, portNum,returnCodes[ret])
                    end
                end
            end

            --print("ret:"..to_string(ret).."    val:"..to_string(val))
            command_data:updateStatus()
            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts("Can not set rule for all processed ports.")
    end

    -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: service-acl
--------------------------------------------

CLI_addCommand({"interface", "interface_eport"}, "service-acl", {
  func=service_acl,
  help="Apply an ACL to particular interface",
   params={
    { type="named",
      { format="pcl-ID %pclID", name="pclID", help="The PCL ID number (0-1023)" },
      { format="egress", name="egress", help="direction: Egress (optional)" },
      { format="lookup %lookupNum", name="lookupNum", help="The lookup number to use (0 or 1) default is 0" },
        requirements = {["egress"] = { "pclID" }},
        mandatory = {"pclID", "lookupNum"}

    }
  }
})



local function no_service_acl(params)
    local command_data = Command_Data()
    local ret,val,interfaceInfo,lookupCfgPtr,lookupNum
    local portNum,devNum,i
    local direct, directNum

    if params["egress"] ~= nil then
        direct = "egress"
        directNum = 1
    else
        direct = "ingress"
        directNum = 0
    end

    interfaceInfo={}
    interfaceInfo["type"]=0
    interfaceInfo["devPort"]={}

    lookupCfgPtr={}
    lookupCfgPtr["pclId"]=params["pclID"]
    lookupCfgPtr["groupKeyTypes"]={}
    -- these values used only as contents of disabled entry and supported by any PP
    -- CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E == 4
    -- CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E  == 5
    -- CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E  == 10
    -- CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E   == 11
    if direct == "ingress" then
        lookupCfgPtr["groupKeyTypes"]["nonIpKey"] = 4
        lookupCfgPtr["groupKeyTypes"]["ipv4Key"]  = 4
        lookupCfgPtr["groupKeyTypes"]["ipv6Key"]  = 5
        lookupCfgPtr["enableLookup"]=0
    else
        lookupCfgPtr["groupKeyTypes"]["nonIpKey"] = 10
        lookupCfgPtr["groupKeyTypes"]["ipv4Key"]  = 10
        lookupCfgPtr["groupKeyTypes"]["ipv6Key"]  = 11
        lookupCfgPtr["enableLookup"]=0
    end


    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            interfaceInfo["devPort"]["devNum"]=devNum
            interfaceInfo["devPort"]["portNum"]=portNum

            for i=0,1 do
                --check if this is the pcl to remove
                ret,val = myGenWrapper("cpssDxChPclCfgTblGet",{   --Always ingress and port mode
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","CPSS_INTERFACE_INFO_STC","interfaceInfoPtr",interfaceInfo},
                    {"IN",TYPE["ENUM"],"direction",directNum},
                    {"IN",TYPE["ENUM"],"lookupNum",i},
                    {"OUT","CPSS_DXCH_PCL_LOOKUP_CFG_STC","lookupCfgPtr"}
                    })

                if (ret==0) and (val["lookupCfgPtr"]["pclId"]==params["pclID"]) then
                    ret,val = myGenWrapper("cpssDxChPclCfgTblSet",{   --Always ingress and port mode
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","CPSS_INTERFACE_INFO_STC","interfaceInfoPtr",interfaceInfo},
                        {"IN",TYPE["ENUM"],"direction",directNum},
                        {"IN",TYPE["ENUM"],"lookupNum",i},
                        {"IN","CPSS_DXCH_PCL_LOOKUP_CFG_STC","lookupCfgPtr",lookupCfgPtr}
                    })

                    if ret~=0 then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error configuring config table at device %d port %d: ", devNum, portNum,returnCodes[ret])
                    end
                end
            end

            --Disable the port
            if direct == "ingress" then
                ret,val = prvEnablePCLPort(devNum,portNum,false)
            else
                ret,val = prvEnableEgressPCLPort(devNum,portNum,false)
            end


            if ret~=0 then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error disabling ingress policy at device %d port %d: ", devNum, portNum,returnCodes[ret])
            end

            --print("ret:"..to_string(ret).."    val:"..to_string(val))
            command_data:updateStatus()
            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts("Can not set rule for all processed ports.")
    end
    -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: no service-acl
--------------------------------------------

CLI_addCommand({"interface", "interface_eport"}, "no service-acl", {
  func=no_service_acl,
  help="Apply an ACL to particular interface",
     params={
    { type="named",
      { format="pcl-ID %pclID", name="pclID", help="The PCL ID number (0-1023)" },
      { format="egress", name="egress", help="direction: Egress (optional)" },
      requirements = {["egress"] = { "pclID" }},
      mandatory = {"pclID"}
    }
  }
})

-- ************************************************************************
--  delete_service_acl
--
--  @description disable all ports on the device connected to the PCL-ID and all TCAM associated with the PCL-ID
--
--  @param params params[pclID]    - The PCL id to bind the interface to
--                  params[devID]    - The device number
--
--  @return true on success, otherwise false and error message
--
-- ************************************************************************

local function delete_service_acl(params)
    local ports,i,ret,val,numRules,devices,j


    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    ports=luaCLI_getDevInfo()

    local interfaceInfo={}
    interfaceInfo["type"]=0
    interfaceInfo["devPort"]={}

    local lookupCfgPtr={}
    lookupCfgPtr["pclId"]=params["pclID"]
    lookupCfgPtr["groupKeyTypes"]={}
    lookupCfgPtr["groupKeyTypes"]["nonIpKey"]=14  --Ext UDB
    lookupCfgPtr["groupKeyTypes"]["ipv4Key"]=14  --Ext UDB
    lookupCfgPtr["groupKeyTypes"]["ipv6Key"]=14  --Ext UDB
    lookupCfgPtr["enableLookup"]=0


    for j=1,#devices do
        i=1
        interfaceInfo["devPort"]["devNum"]=devices[j]


        --disable all ports that use the PCL-ID
        while(ports[devices[j]][i]~=nil) do
            interfaceInfo["devPort"]["portNum"]=ports[devices[j]][i]


            ret,val = myGenWrapper("cpssDxChPclCfgTblGet",{   --Always ingress, lookup 0 and port mode
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","CPSS_INTERFACE_INFO_STC","interfaceInfoPtr",interfaceInfo},
                {"IN",TYPE["ENUM"],"direction",0},
                {"IN",TYPE["ENUM"],"lookupNum",0},
                {"OUT","CPSS_DXCH_PCL_LOOKUP_CFG_STC","lookupCfgPtr"}
            })

            if (ret==0) and (val["lookupCfgPtr"]["pclId"]==params["pclID"]) then
                ret,val = myGenWrapper("cpssDxChPclCfgTblSet",{   --Always ingress, lookup 0 and port mode
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","CPSS_INTERFACE_INFO_STC","interfaceInfoPtr",interfaceInfo},
                    {"IN",TYPE["ENUM"],"direction",0},
                    {"IN",TYPE["ENUM"],"lookupNum",0},
                    {"IN","CPSS_DXCH_PCL_LOOKUP_CFG_STC","lookupCfgPtr",lookupCfgPtr}
                })

                if (ret~=0) then printf("Error:"..to_string(val)) end
            end
            i=i+1
        end


        --Delete all rules that belong to the PCL-ID
        ret,val = myGenWrapper("cpssDxChCfgTableNumEntriesGet",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN",TYPE["ENUM"],"table",3}, --ext rule
            {"OUT","GT_U32","numEntriesPtr"}   -- index of the rule
        })

        if (ret~=0) then return false,"Could not find max rule-iD size" end

        numRules=(val["numEntriesPtr"]/2) -1

        for i=0,numRules do

            ret,val = myGenWrapper("cpssDxChPclPortGroupRuleAnyStateGet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_U32","portGroupsBmp",0xFFFFFFFF},
                {"IN",TYPE["ENUM"],"ruleSize",1}, --ext rule
                {"IN","GT_U32","ruleIndex",i},   -- index of the rule
                {"OUT","GT_BOOL","validPtr"},   -- index of the rule
                {"OUT",TYPE["ENUM"],"ruleSizePtr"}   -- index of the rule
            })

            if (ret==0) and (val["validPtr"]==true) then            --if pclid matches the one we want
                local dev_ = devices[j]
                local rule_size = 1; -- extended rule
                val = wrLogWrapper(
                    "wrlCpssPclRuleIdtoPclIdGet",
                    "(dev_, i, rule_size, direction)",
                    dev_,i, rule_size, direction);

                if (val==params["pclID"]) then

                    ret,val = myGenWrapper("cpssDxChPclRuleInvalidate",{
                        {"IN","GT_U8","devNum",devices[j]},
                        {"IN",TYPE["ENUM"],"ruleSize",1}, --ext rule
                        {"IN","GT_U32","ruleIndex",i}   -- index of the rule
                    })
                end
            end
        end
    end
end





--------------------------------------------
-- command registration: delete access-list
--------------------------------------------
CLI_addHelp("config", "delete", "Delete sub-commands")

CLI_addCommand("config", "delete access-list", {
  func=delete_service_acl,
  help="Delete an extended ACL",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="pcl-id %pclID", name="pclID", help="The PCL ID number (0-1023)" },
        requirements = {["pclID"]={"devID"}},
      mandatory = {"pclID"}
    }
  }
})



local function prvParseAction(action)
    local ret = "\nAction:\t"
        if (action["pktCmd"]=="CPSS_PACKET_CMD_FORWARD_E") then
            ret=ret.."Permit"
        elseif (action["pktCmd"]=="CPSS_PACKET_CMD_MIRROR_TO_CPU_E") then
            ret=ret.."MirrorToCpu"
        elseif (action["pktCmd"]=="CPSS_PACKET_CMD_TRAP_TO_CPU_E") then
            ret=ret.."TrapToCpu"
        elseif (action["pktCmd"]=="CPSS_PACKET_CMD_DROP_HARD_E") then
            ret=ret.."Deny"
        elseif (action["pktCmd"]=="CPSS_PACKET_CMD_DROP_SOFT_E") then
            ret=ret.."DenySoft"
        elseif (action["pktCmd"]=="CPSS_PACKET_CMD_ROUTE_E") then
            ret=ret.."Route"
        elseif (action["pktCmd"]=="CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E") then
            ret=ret.."RouteAndMirror"
        elseif (action["pktCmd"]=="CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E") then
            ret=ret.."BridgeAndMirror"
        elseif (action["pktCmd"]=="CPSS_PACKET_CMD_BRIDGE_E") then
            ret=ret.."Bridge"
        elseif (action["pktCmd"]=="CPSS_PACKET_CMD_NONE_E") then
            ret=ret.."None"
        elseif (action["pktCmd"]=="CPSS_PACKET_CMD_LOOPBACK_E") then
            ret=ret.."Loopback"
        end

        if (action["policer"]~=nil) and (action["policer"]["policerEnable"]=="CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E") then
            ret=ret.."\nPolicer id:\t"..action["policer"]["policerId"]
        end

        if (action["qos"] ~= nil                        and
            action["qos"]["ingress"] ~= nil             and
            action["qos"]["ingress"]["profileAssignIndex"]==true) then

            if action["qos"]["ingress"]["modifyUp"]=="CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E" and action["qos"]["ingress"]["modifyDscp"]=="CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E" then
                ret=ret.."\nQOS action : Set-vpt "..(action["qos"]["ingress"]["profileIndex"]-64-8)
            elseif action["qos"]["ingress"]["modifyDscp"]=="CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E" and action["qos"]["ingress"]["modifyUp"]=="CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E" then
                ret=ret.."\nQOS action : Set-dscp "..(action["qos"]["ingress"]["profileIndex"])
            elseif action["qos"]["ingress"]["modifyDscp"]=="CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E" and action["qos"]["ingress"]["modifyUp"]=="CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E" then
                ret=ret.."\nQOS action : Set-queue "..(action["qos"]["ingress"]["profileIndex"]-64+1)
            end
        end

        if (action["vlan"] ~= nil and action["vlan"]["egress"] ~= nil and action["vlan"]["egress"]["vlanCmd"] == "CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E") then
            ret = ret .. "\n"
            ret = ret .. "vlan action : vlanCmd: modifyTag0"
            ret = ret .. "\n"
            ret = ret .. "vlan action : vlanId: " .. action["vlan"]["egress"]["vlanId"]
        end

        if (action["vlan"] ~= nil and action["vlan"]["egress"] ~= nil and action["vlan"]["egress"]["vlanId1ModifyEnable"] == true) then
            ret = ret .. "\n"
            ret = ret .. "vlan action : vlanCmd: modifyTag1"
            ret = ret .. "\n"
            ret = ret .. "vlan action : vlanId1: " .. action["vlan"]["egress"]["vlanId1"]
        end

        if (action["redirect"] ~= nil and action["redirect"]["redirectCmd"] == "CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E") then
            local entry_interface_string  =  interfaceInfoStrGet(action["redirect"]["data"]["outIf"]["outInterface"])
            ret = ret .. "\n"
            ret = ret .. "Redirect action : "..entry_interface_string
        end

    return ret
end

--concatenate two tables
local function tconcat(tbl1,tbl2)
    local i, t
    t = {}
    for i=1,#tbl1 do
        t[i] = tbl1[i]
    end
    for i=1,#tbl2 do
        t[#tbl1+i] = tbl2[i]
    end
    return t
end

local table_common = {
    {head = "Pcl Id",        path = "ingressIpCommon.pclId",        type ="number"},
    {head = "Mac to me",     path = "ingressIpCommon.macToMe",      type ="number"},
    {head = "Source port",   path = "ingressIpCommon.sourcePort",   type = "number"},
    {head = "Source device", path = "ingressIpCommon.sourceDevice", type ="number"},
    {head = "Port list bmp", path = "ingressIpCommon.portListBmp",  type ="array"},
    {head = "Is tagged",     path = "ingressIpCommon.isTagged",     type ="bool"},
    {head = "Vid",           path = "ingressIpCommon.vid",          type ="number"},
    {head = "Up",            path = "ingressIpCommon.up",           type ="number"},
    {head = "Tos",           path = "ingressIpCommon.tos",          type ="number"},
    {head = "Ip protocol",   path = "ingressIpCommon.ipProtocol",   type = "number"},
    {head = "Is L4 Valid",   path = "ingressIpCommon.isL4Valid",    type ="bool"},
    {head = "L4 Byte0",      path = "ingressIpCommon.l4Byte0",      type = "number"},
    {head = "L4 Byte1",      path = "ingressIpCommon.l4Byte1",      type = "number"},
    {head = "L4 Byte2",      path = "ingressIpCommon.l4Byte2",      type = "number"},
    {head = "L4 Byte3",      path = "ingressIpCommon.l4Byte3",      type = "number"},
    {head = "L4 Byte13",     path = "ingressIpCommon.l4Byte13",     type = "number"}
}

local table_common_ingr_udb = {
    {head = "Pcl Id",             path = "commonIngrUdb.pclId",             type ="number"},
    {head = "Mac to me",          path = "commonIngrUdb.macToMe",           type ="number"},
    {head = "Source port",        path = "commonIngrUdb.sourcePort",        type ="number"},
    {head = "Source device",      path = "commonIngrUdb.sourceDevice",      type ="number"},
    {head = "Ports",              path = "commonIngrUdb.portListBmp.ports", type ="array"},
    {head = "Vid",                path = "commonIngrUdb.vid",               type ="number"},
    {head = "Up",                 path = "commonIngrUdb.up",                type ="number"},
    {head = "Is Ip",              path = "commonIngrUdb.isIp",              type ="bool"},
    {head = "Dscp Or Exp",        path = "commonIngrUdb.dscpOrExp",         type ="number"},
    {head = "Is L2 Valid",        path = "commonIngrUdb.isL2Valid",         type ="bool"},
    {head = "Is Udb Valid",       path = "commonIngrUdb.isUdbValid",        type ="bool"},
    {head = "Pkt Tagging",        path = "commonIngrUdb.pktTagging",        type ="number"},
    {head = "L3 offset invalid",  path = "commonIngrUdb.l3OffsetInvalid",   type ="number"},
    {head = "L4 protocol type",   path = "commonIngrUdb.l4ProtocolType",    type ="number"},
    {head = "Pkt type",           path = "commonIngrUdb.pktType",           type ="number"},
    {head = "Ip header ok",       path = "commonIngrUdb.ipHeaderOk",        type ="number"},
    {head = "Mac Da type",        path = "commonIngrUdb.macDaType",         type ="number"},
    {head = "L4 Offset Invalid",  path = "commonIngrUdb.l4OffsetInvalid",   type ="number"},
    {head = "L2 encapsulation",   path = "commonIngrUdb.l2Encapsulation",   type ="number"},
    {head = "Is Ipv6Eh",          path = "commonIngrUdb.isIpv6Eh",          type ="bool"},
    {head = "Is Ipv6 Hop By Hop", path = "commonIngrUdb.isIpv6HopByHop",    type ="bool"}
}

local table_common_std_ip = {
    {head = "Is Ipv4",         path = "commonStdIp.isIpv4",         type ="bool"},
    {head = "Ip protocol",     path = "commonStdIp.ipProtocol",     type ="number"},
    {head = "Dscp",            path = "commonStdIp.dscp",           type ="number"},
    {head = "Is L4 Valid",     path = "commonStdIp.isL4Valid",      type ="bool"},
    {head = "L4 Byte2",        path = "commonStdIp.l4Byte2",        type ="number"},
    {head = "L4 Byte3",        path = "commonStdIp.l4Byte3",        type ="number"},
    {head = "Ip Header Ok",    path = "commonStdIp.ipHeaderOk",     type ="number"},
    {head = "Ipv4 fragmented", path = "commonStdIp.ipv4Fragmented", type ="number"}
}

local table_common_ext = {
    {head = "Is Ipv6",      path = "commonExt.isIpv6",     type ="bool"},
    {head = "Ip Protocol",  path = "commonExt.ipProtocol", type ="number"},
    {head = "Dscp",         path = "commonExt.dscp",       type ="number"},
    {head = "Is L4 Valid",  path = "commonExt.isL4Valid",  type ="bool"},
    {head = "L4 Byte0",     path = "commonExt.l4Byte0",    type ="number"},
    {head = "L4 Byte1",     path = "commonExt.l4Byte1",    type ="number"},
    {head = "L4 Byte2",     path = "commonExt.l4Byte2",    type ="number"},
    {head = "L4 Byte3",     path = "commonExt.l4Byte3",    type ="number"},
    {head = "L4 Byte13",    path = "commonExt.l4Byte13",   type ="number"},
    {head = "Ip header Ok", path = "commonExt.ipHeaderOk", type ="number"}
}

local table_ingress_ip_common = {
    {head = "Pcl Id",        path = "ingressIpCommon.pclId",        type ="number"},
    {head = "Mac To Me",     path = "ingressIpCommon.macToMe",      type ="number"},
    {head = "Source port",   path = "ingressIpCommon.sourcePort",   type ="number"},
    {head = "Source device", path = "ingressIpCommon.sourceDevice", type ="number"},
    {head = "Port List Bmp", path = "ingressIpCommon.portListBmp",  type ="number"},
    {head = "Is tagged",     path = "ingressIpCommon.isTagged",     type ="bool"},
    {head = "Vid",           path = "ingressIpCommon.vid",          type ="number"},
    {head = "Up",            path = "ingressIpCommon.up",           type ="number"},
    {head = "Tos",           path = "ingressIpCommon.tos",          type ="number"},
    {head = "Ip Protocol",   path = "ingressIpCommon.ipProtocol",   type ="number"},
    {head = "Is L4 Valid",   path = "ingressIpCommon.isL4Valid",    type ="number"},
    {head = "L4 Byte0",      path = "ingressIpCommon.l4Byte0",      type ="number"},
    {head = "L4 Byte1",      path = "ingressIpCommon.l4Byte1",      type ="number"},
    {head = "L4 Byte2",      path = "ingressIpCommon.l4Byte2",      type ="number"},
    {head = "L4 Byte3",      path = "ingressIpCommon.l4Byte3",      type ="number"},
    {head = "L4 Byte13",     path = "ingressIpCommon.l4Byte13",     type ="number"}
}


local table_egress_ip_common = {
    {head = "Pcl Id",            path = "egressIpCommon.pclId",         type ="number"},
    {head = "Source Port",       path = "egressIpCommon.sourcePort",    type ="number"},
    {head = "Qos Profile",       path = "egressIpCommon.qosProfile",    type ="number"},
    {head = "IsL2Valid",        path = "egressIpCommon.isL2Valid",      type ="bool"},
    {head = "OriginalVid",      path = "egressIpCommon.originalVid",    type ="number"},
    {head = "IsSrcTrunk",       path = "egressIpCommon.isSrcTrunk",     type ="bool"},
    {head = "SrcDevOrTrunkId",  path = "egressIpCommon.srcDevOrTrunkId", type ="number"},
    {head = "SrcDev",           path = "egressIpCommon.srcDev",         type ="number"},
    {head = "IsIp",             path = "egressIpCommon.isIp",           type ="bool"},
    {head = "IsArp",            path = "egressIpCommon.isArp",          type ="bool"},
    {head = "MacDa",            path = "egressIpCommon.macDa",          type ="macAddr"},
    {head = "MacSa",            path = "egressIpCommon.macSa",          type ="macAddr"},
    {head = "Tag1Exist",        path = "egressIpCommon.tag1Exist",      type ="number"},
    {head = "SourceId",         path = "egressIpCommon.sourceId",       type ="number"},
    {head = "Tos",              path = "egressIpCommon.tos",            type ="number"},
    {head = "IpProtocol",       path = "egressIpCommon.ipProtocol",     type ="number"},
    {head = "Ttl",              path = "egressIpCommon.ttl",            type ="number"},
    {head = "IsL4Valid",        path = "egressIpCommon.isL4Valid",      type ="bool"},
    {head = "L4Byte0",          path = "egressIpCommon.l4Byte0",        type ="number"},
    {head = "L4Byte1",          path = "egressIpCommon.l4Byte1",        type ="number"},
    {head = "L4Byte2",          path = "egressIpCommon.l4Byte2",        type ="number"},
    {head = "L4Byte3",          path = "egressIpCommon.l4Byte3",        type ="number"},
    {head = "L4Byte13",         path = "egressIpCommon.l4Byte13",       type ="number"},
    {head = "TcpUdpPortComparators", path = "egressIpCommon.tcpUdpPortComparators", type ="number"},
    {head = "Tc",               path = "egressIpCommon.tc",             type ="number"},
    {head = "Dp",               path = "egressIpCommon.dp",             type ="number"},
    {head = "EgrPacketType",    path = "egressIpCommon.egrPacketType",  type ="number"},
    {head = "SrcTrgOrTxMirror", path = "egressIpCommon.srcTrgOrTxMirror", type ="number"},
    {head = "AssignedUp",       path = "egressIpCommon.assignedUp",     type ="number"},
    {head = "TrgPhysicalPort",  path = "egressIpCommon.trgPhysicalPort", type ="number"},
    {head = "RxSniff",          path = "egressIpCommon.rxSniff",        type ="number"},
    {head = "IsRouted",         path = "egressIpCommon.isRouted",       type ="bool"},
    {head = "IsIpv6",           path = "egressIpCommon.isIpv6",         type ="bool"},
    {head = "CpuCode",          path = "egressIpCommon.cpuCode",        type ="number"},
    {head = "SrcTrg",           path = "egressIpCommon.srcTrg",         type ="number"},
    {head = "EgrFilterEnable",  path = "egressIpCommon.egrFilterEnable", type ="number"}
}

local table_replaced_fld_ingress = {
    {head = "pclId",       path = "replacedFld.pclId",       type ="number"},
    {head = "isUdbValid",  path = "replacedFld.isUdbValid",  type ="bool"},
    {head = "vid",         path = "replacedFld.vid",         type ="number"},
    {head = "srcPort",     path = "replacedFld.srcPort",     type ="number"},
    {head = "srcDevIsOwn", path = "replacedFld.srcDevIsOwn", type ="number"},
    {head = "vid1",        path = "replacedFld.vid1",        type ="number"},
    {head = "up1",         path = "replacedFld.up1",         type ="number"},
    {head = "macToMe",     path = "replacedFld.macToMe",     type ="number"},
    {head = "qosProfile",  path = "replacedFld.qosProfile",  type ="number"},
    {head = "flowId",      path = "replacedFld.flowId",      type ="number"}
}

local table_udb60_fixed_fld_ingress = {
    {head = "isUdbValid",  path = "udb60FixedFld.isUdbValid",   type ="bool"},
    {head = "pclId",       path = "udb60FixedFld.pclId",        type ="number"},
    {head = "vid",         path = "udb60FixedFld.vid",          type ="number"},
    {head = "srcPort",     path = "udb60FixedFld.srcPort",      type ="number"},
    {head = "srcDevIsOwn", path = "udb60FixedFld.srcDevIsOwn",  type ="number"},
    {head = "vid1",        path = "udb60FixedFld.vid1",         type ="number"},
    {head = "up1",         path = "udb60FixedFld.up1",          type ="number"},
    {head = "macToMe",     path = "udb60FixedFld.macToMe",      type ="number"},
    {head = "qosProfile",  path = "udb60FixedFld.qosProfile",   type ="number"},
    {head = "flowId",      path = "udb60FixedFld.flowId",       type ="number"}
}

local table_replaced_fld_egress = {
    {head = "Pcl Id",       path = "replacedFld.pclId",         type ="number"},
    {head = "Is Udb Valid", path = "replacedFld.isUdbValid",    type ="bool"}
}

local table_udb60_fixed_fld_egress = {
    {head = "pclId",               path = "udb60FixedFld.pclId",      type ="number"},
    {head = "isUdbValid",          path = "udb60FixedFld.isUdbValid", type ="bool"},
    {head = "vid",                 path = "udb60FixedFld.vid",        type ="number"},
    {head = "srcPort",             path = "udb60FixedFld.srcPort",    type ="number"},
    {head = "trgPort",             path = "udb60FixedFld.trgPort",    type ="number"},
    {head = "srcDev",              path = "udb60FixedFld.srcDev",     type ="number"},
    {head = "trgDev",              path = "udb60FixedFld.trgDev",     type ="number"},
    {head = "localDevTrgPhyPort",  path = "udb60FixedFld.localDevTrgPhyPort", type ="number"}
}

local table_info = {
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E = tconcat(table_common, {
        {head = "Is Ipv4",      path = "isIpv4",    type = "bool"},
        {head = "Ether Type",   path = "etherType", type = "hex"},
        {head = "Is Arp",       path = "isArp",     type = "bool"},
        {head = "L2 encap",     path = "l2Encap",   type = "number"},
        {head = "Mac Da",       path = "macDa",     type = "macAddr"},
        {head = "Mac Sa",       path = "macSa",     type = "macAddr"},
        {head = "Udb15_17",     path = "udb15_17",  type = "array"},
        {head = "Udb23_26",     path = "udb23_26",  type = "array"},
        {head = "VrfId",        path = "vrfId",     type = "number"},
        {head = "Trunk Hash",   path = "trunkHash", type = "number"},
        {head = "Tag1 Exist",   path = "tag1Exist", type = "number"},
        {head = "Vid1",         path = "vid1",      type = "number"},
        {head = "Up1",          path = "up1",       type = "number"},
        {head = "Cfi1",         path = "cfi1",      type = "number"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E = tconcat(
        tconcat(table_common, table_common_std_ip), {
        {head = "Is Arp",            path = "isArp",             type = "bool"},
        {head = "IsIpv6ExtHdrExist", path = "isIpv6ExtHdrExist", type = "bool"},
        {head = "IsIpv6HopByHop",    path = "isIpv6HopByHop",    type = "bool"},
        {head = "Mac Da",            path = "macDa",             type = "macAddr"},
        {head = "Mac Sa",            path = "macSa",             type = "macAddr"},
        {head = "Udb18_19",          path = "udb18_19",          type = "array"},
        {head = "VrfId",             path = "vrfId",             type = "number"},
        {head = "Udb27_30",          path = "udb27_30",          type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E = tconcat(
        tconcat(table_common, table_common_std_ip), {
        {head = "Is Arp",     path = "isArp",     type = "bool"},
        {head = "Is Bc",      path = "isBc",      type = "bool"},
        {head = "Sip",        path = "sip",       type = "ipAddr"},
        {head = "Dip",        path = "dip",       type = "ipAddr"},
        {head = "L4 Byte0",   path = "l4Byte0",   type = "number"},
        {head = "L4 Byte1",   path = "l4Byte1",   type = "number"},
        {head = "L4 Byte13",  path = "l4Byte13",  type = "number"},
        {head = "Udb20_22",   path = "udb20_22",  type = "array"},
        {head = "VrfId",      path = "vrfId",     type = "number"},
        {head = "Trunk Hash", path = "trunkHash", type = "number"},
        {head = "Udb31_34",   path = "udb31_34",  type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E = tconcat(
        tconcat(table_common, table_common_std_ip), {
        {head = "Is Arp",            path = "isArp",             type = "bool"},
        {head = "IsIpv6ExtHdrExist", path = "isIpv6ExtHdrExist", type = "bool"},
        {head = "IsIpv6HopByHop",    path = "isIpv6HopByHop",    type = "bool"},
        {head = "Dip",               path = "dip",               type = "ipAddr"},
        {head = "Udb47_49",          path = "udb47_49",          type = "array"},
        {head = "Udb0",              path = "udb0",              type = "number"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E = tconcat(
        tconcat(table_common, table_common_ext), {
        {head = "Sip",          path = "sip",           type = "ipAddr"},
        {head = "Dip",          path = "dip",           type = "ipAddr"},
        {head = "EtherType",    path = "etherType",     type = "hex"},
        {head = "L2 Encap",     path = "l2Encap",       type = "number"},
        {head = "Mac Da",       path = "macDa",         type = "macAddr"},
        {head = "Mac Sa",       path = "macSa",         type = "macAddr"},
        {head = "Ipv4 Fragmented",   path = "ipv4Fragmented", type = "number"},
        {head = "Udb0_5",       path = "udb0_5",        type = "array"},
        {head = "Tag1 exist",   path = "tag1Exist",     type = "number"},
        {head = "Vid1",         path = "vid1",          type = "number"},
        {head = "Up1",          path = "up1",           type = "number"},
        {head = "Cfi1",         path = "cfi1",          type = "number"},
        {head = "VrfId",        path = "vrfId",         type = "number"},
        {head = "Trunk hash",   path = "trunkHash",     type = "number"},
        {head = "Udb39_46",     path = "udb39_46",      type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E = tconcat(
        tconcat(table_common, table_common_ext), {
        {head = "Sip",   path = "sip", type = "ipAddr"},
        {head = "DipBits127to120",   path = "dipBits127to120", type = "number"},
        {head = "IsIpv6ExtHdrExist",   path = "isIpv6ExtHdrExist", type = "bool"},
        {head = "IsIpv6HopByHop",   path = "isIpv6HopByHop", type = "bool"},
        {head = "Mac Da",   path = "macDa", type = "macAddr"},
        {head = "Mac Sa",   path = "macSa", type = "macAddr"},
        {head = "Udb6_11",   path = "udb6_11", type = "array"},
        {head = "Tag1 exist",   path = "tag1Exist", type = "number"},
        {head = "Vid1",   path = "vid1", type = "number"},
        {head = "Up1",   path = "up1", type = "number"},
        {head = "VrfId",   path = "vrfId", type = "number"},
        {head = "Trunk hash",   path = "trunkHash", type = "number"},
        {head = "Udb47_49",   path = "udb47_49", type = "array"},
        {head = "Udb0_4",   path = "udb0_4", type = "array"},
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E = tconcat(
        tconcat(table_common, table_common_ext), {
        {head = "Sip",   path = "sip", type = "ipAddr"},
        {head = "Dip",   path = "dip", type = "ipAddr"},
        {head = "IsIpv6ExtHdrExist",   path = "isIpv6ExtHdrExist", type = "bool"},
        {head = "IsIpv6HopByHop",   path = "isIpv6HopByHop", type = "bool"},
        {head = "Udb12_14",   path = "udb12_14", type = "array"},
        {head = "Tag1 exist",   path = "tag1Exist", type = "number"},
        {head = "Vid1",   path = "vid1", type = "number"},
        {head = "Up1",   path = "up1", type = "number"},
        {head = "VrfId",   path = "vrfId", type = "number"},
        {head = "Trunk hash",   path = "trunkHash", type = "number"},
        {head = "Udb15_22",   path = "udb15_22", type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E = tconcat(table_common, {
        {head = "Is Ipv4",   path = "isIpv4", type = "bool"},
        {head = "Ether type",   path = "etherType", type = "hex"},
        {head = "Is arp",   path = "isArp", type = "bool"},
        {head = "L2 encap",   path = "l2Encap", type = "number"},
        {head = "Mac Da",   path = "macDa", type = "macAddr"},
        {head = "Mac Sa",   path = "macSa", type = "macAddr"},
        {head = "Vid1",   path = "vid1", type = "number"},
        {head = "Up1",   path = "up1", type = "number"},
        {head = "Cfi1",   path = "cfi1", type = "number"},
        {head = "Udb0_3",   path = "udb0_3", type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E = tconcat(
        tconcat(table_common, table_common_std_ip), {
        {head = "Is Arp",   path = "isArp", type = "bool"},
        {head = "DipBits0to31",   path = "dipBits0to31", type = "number"},
        {head = "L4 Byte13",   path = "l4Byte13", type = "number"},
        {head = "Mac Da",   path = "macDa", type = "macAddr"},
        {head = "Udb4_7",   path = "udb4_7", type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E = tconcat(
        tconcat(table_common, table_common_std_ip), {
        {head = "Is Arp",   path = "isArp", type = "bool"},
        {head = "Is Bc",   path = "isBc", type = "bool"},
        {head = "Sip",   path = "sip", type = "ipAddr"},
        {head = "Dip",   path = "dip", type = "ipAddr"},
        {head = "L4 Byte0",   path = "l4Byte0", type = "number"},
        {head = "L4 Byte1",   path = "l4Byte1", type = "number"},
        {head = "L4 Byte13",   path = "l4Byte13", type = "number"},
        {head = "Udb8_11",   path = "udb8_11", type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E = tconcat(
        tconcat(table_common, table_common_ext), {
        {head = "Sip",   path = "sip", type = "ipAddr"},
        {head = "Dip",   path = "dip", type = "ipAddr"},
        {head = "Ether type",   path = "etherType", type = "hex"},
        {head = "L2 encap",   path = "l2Encap", type = "number"},
        {head = "Mac Da",   path = "macDa", type = "macAddr"},
        {head = "Mac Sa",   path = "macSa", type = "macAddr"},
        {head = "Ipv4Fragmented",   path = "ipv4Fragmented", type = "number"},
        {head = "Vid1",   path = "vid1", type = "number"},
        {head = "Up1",   path = "up1", type = "number"},
        {head = "Cfi1",   path = "cfi1", type = "number"},
        {head = "Is Mpls",   path = "isMpls", type = "bool"},
        {head = "NumOfMplsLabels",   path = "numOfMplsLabels", type = "number"},
        {head = "ProtocolTypeAfterMpls",   path = "protocolTypeAfterMpls", type = "number"},
        {head = "Mpls Label0",   path = "mplsLabel0", type = "number"},
        {head = "Mpls Exp0",   path = "mplsExp0", type = "number"},
        {head = "Mpls Label1",   path = "mplsLabel1", type = "number"},
        {head = "Mpls Exp1",   path = "mplsExp1", type = "number"},
        {head = "Udb12_19",   path = "udb12_19", type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E = tconcat(
        tconcat(table_common, table_common_ext), {
        {head = "Sip",   path = "sip", type = "ipAddr"},
        {head = "DipBits127to120",   path = "dipBits127to120", type = "number"},
        {head = "Mac Da",   path = "macDa", type = "macAddr"},
        {head = "Mac Sa",   path = "macSa", type = "macAddr"},
        {head = "Vid1",   path = "vid1", type = "number"},
        {head = "Up1",   path = "up1", type = "number"},
        {head = "Cfi1",   path = "cfi1", type = "number"},
        {head = "IsIpv6ExtHdrExist",   path = "isIpv6ExtHdrExist", type = "bool"},
        {head = "IsIpv6HopByHop",   path = "isIpv6HopByHop", type = "bool"},
        {head = "Udb20_27",   path = "udb20_27", type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E = tconcat(
        tconcat(table_common, table_common_ext), {
        {head = "Sip",   path = "sip", type = "ipAddr"},
        {head = "Dip",   path = "dip", type = "ipAddr"},
        {head = "IsIpv6ExtHdrExist",   path = "isIpv6ExtHdrExist", type = "bool"},
        {head = "IsIpv6HopByHop",   path = "isIpv6HopByHop", type = "bool"},
        {head = "Udb28_35",   path = "udb28_35", type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E = tconcat(table_common_ingr_udb, {
        {head = "Is Ipv4",   path = "isIpv4", type = "bool"},
        {head = "Udb0_15",   path = "udb0_15", type = "array"},
        {head = "VrfId",   path = "vrfId", type = "number"},
        {head = "Qos Profile",   path = "qosProfile", type = "number"},
        {head = "Trunk hash",   path = "trunkHash", type = "number"},
        {head = "Udb35_38",   path = "udb35_38", type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E = tconcat(table_common_ingr_udb, {
        {head = "Is Ipv6",   path = "isIpv6", type = "bool"},
        {head = "Ip protocol",   path = "ipProtocol", type = "number"},
        {head = "Sip Bits31_0",   path = "sipBits31_0", type = "array"},
        {head = "Sip Bits79_32orMacSa",   path = "sipBits79_32orMacSa", type = "array"},
        {head = "Sip Bits127_80orMacDa",   path = "sipBits127_80orMacDa", type = "array"},
        {head = "Dip Bits127_112",   path = "dipBits127_112", type = "array"},
        {head = "Dip Bits31_0",   path = "dipBits31_0", type = "array"},
        {head = "Udb0_15",   path = "udb0_15", type = "array"},
        {head = "Tag1 Exist",   path = "tag1Exist", type = "number"},
        {head = "Vid1",   path = "vid1", type = "number"},
        {head = "Up1",   path = "up1", type = "number"},
        {head = "Cfi1",   path = "cfi1", type = "number"},
        {head = "VrfId",   path = "vrfId", type = "number"},
        {head = "Qos Profile",   path = "qosProfile", type = "number"},
        {head = "Trunk Hash",   path = "trunkHash", type = "number"},
        {head = "Udb23_30",   path = "udb23_30", type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E = tconcat(table_common, {
        {head = "Packet type",   path = "pktType", type = "number"},
        {head = "Ip fragmented", path = "ipFragmented", type = "number"},
        {head = "Ip header info",    path = "ipHeaderInfo", type = "number"},
        {head = "Ip packet length",  path = "ipPacketLength", type = "number"},
        {head = "TTL",           path = "ttl", type = "number"},
        {head = "SIP",           path = "sip", type = "ipAddr"},
        {head = "DIP",           path = "dip", type = "ipAddr"},
        {head = "VRF Id", path = "vrfId", type = "number"},
        {head = "TcpUdp port comparators", path = "tcpUdpPortComparators", type = "number"},
        {head = "Is Udb valid",  path = "isUdbValid", type = "bool"},
        {head = "Udb5",          path = "udb5", type = "number"},
        {head = "Udb41_44:", path = "udb41_44", type = "array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E = tconcat(table_ingress_ip_common, {
        {head = "Pcl Id", path = "pclId", type ="number"},
        {head = "Is L2 Valid", path = "isL2Valid", type ="bool"},
        {head = "Is Bc", path = "isBc", type ="bool"},
        {head = "Is Ip", path = "isIp", type ="bool"},
        {head = "Is Arp", path = "isArp", type ="bool"},
        {head = "L2 encap", path = "l2Encap", type ="number"},
        {head = "Ether type", path = "etherType", type ="hex"},
        {head = "Mac Da", path = "macDa", type ="macAddr"},
        {head = "Mac Sa", path = "macSa", type ="macAddr"},
        {head = "Tag1 exist", path = "tag1Exist", type ="number"},
        {head = "Vid1", path = "vid1", type ="number"},
        {head = "Up1", path = "up1", type ="number"},
        {head = "Cfi1", path = "cfi1", type ="number"},
        {head = "Ip fragmented", path = "ipFragmented", type ="number"},
        {head = "Ip headerInfo", path = "ipHeaderInfo", type ="number"},
        {head = "Sip", path = "sip", type ="ipAddr"},
        {head = "Dip", path = "dip", type ="ipAddr"},
        {head = "Vrf Id", path = "vrfId", type ="number"},
        {head = "Trunk hash", path = "trunkHash", type ="number"},
        {head = "Is Udb Valid", path = "isUdbValid", type ="bool"},
        {head = "UdbStdIpL2Qos", path = "UdbStdIpL2Qos", type ="array"},
        {head = "UdbStdIpV4L4", path = "UdbStdIpV4L4", type ="array"},
        {head = "UdbExtIpv6L2", path = "UdbExtIpv6L2", type ="array"},
        {head = "UdbExtIpv6L4", path = "UdbExtIpv6L4", type ="array"},
        {head = "Udb5_16", path = "udb5_16", type ="array"},
        {head = "Udb31_38", path = "udb31_38", type ="array"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E = tconcat(table_ingress_ip_common, {
        {head = "Is L2 Valid", path = "isL2Valid", type ="bool"},
        {head = "Is Nd", path = "isNd", type ="bool"},
        {head = "Is Bc", path = "isBc", type ="bool"},
        {head = "Is Ip", path = "isIp", type ="bool"},
        {head = "L2 encap", path = "l2Encap", type ="number"},
        {head = "Ether type", path = "etherType", type ="hex"},
        {head = "Mac Da", path = "macDa", type ="macAddr"},
        {head = "Mac Sa", path = "macSa", type ="macAddr"},
        {head = "Tag1 exist", path = "tag1Exist", type ="number"},
        {head = "Vid1", path = "vid1", type ="number"},
        {head = "Up1", path = "up1", type ="number"},
        {head = "Cfi1", path = "cfi1", type ="number"},
        {head = "Sip", path = "sip", type ="ipAddr"},
        {head = "Dip", path = "dip", type ="ipAddr"},
        {head = "IsIpv6ExtHdrExist", path = "isIpv6ExtHdrExist", type ="bool"},
        {head = "IsIpv6HopByHop", path = "isIpv6HopByHop", type ="bool"},
        {head = "Ip header Ok", path = "ipHeaderOk", type ="number"},
        {head = "Vrf Id", path = "vrfId", type ="number"},
        {head = "Is Udb Valid", path = "isUdbValid", type ="bool"},
        {head = "UdbExtNotIpv6", path = "UdbExtNotIpv6", type ="array"},
        {head = "UdbExtIpv6L2", path = "UdbExtIpv6L2", type ="array"},
        {head = "UdbExtIpv6L4", path = "UdbExtIpv6L4", type ="array"},
        {head = "Udb0_11", path = "udb0_11", type ="array"},
        {head = "udb12", path = "udb12", type ="number"},
        {head = "udb39_40", path = "udb39_40", type ="array"},
        {head = "srcPortOrTrunk", path = "srcPortOrTrunk", type ="number"},
        {head = "srcIsTrunk", path = "srcIsTrunk", type ="number"},
        {head = "trunkHash", path = "trunkHash", type ="number"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E = tconcat(table_ingress_ip_common, {
        {head = "Pkt type",   path = "pktType", type = "number"},
        {head = "Is Nd",   path = "isNd", type = "bool"},
        {head = "Tag1 Exist",   path = "tag1Exist", type = "number"},
        {head = "Vid1",   path = "vid1", type = "number"},
        {head = "Up1",   path = "up1", type = "number"},
        {head = "Cfi1",   path = "cfi1", type = "number"},
        {head = "MplsOuterLabel",   path = "mplsOuterLabel", type = "number"},
        {head = "MplsOuterLabExp",   path = "mplsOuterLabExp", type = "number"},
        {head = "MplsOuterLabSBit",   path = "mplsOuterLabSBit", type = "number"},
        {head = "IpPacketLength",   path = "ipPacketLength", type = "number"},
        {head = "Ipv6HdrFlowLabel",   path = "ipv6HdrFlowLabel", type = "number"},
        {head = "Ttl",   path = "ttl", type = "number"},
        {head = "Sip", path = "sip", type ="ipAddr"},
        {head = "Dip", path = "dip", type ="ipAddr"},
        {head = "IsIpv6ExtHdrExist",   path = "isIpv6ExtHdrExist", type = "bool"},
        {head = "IsIpv6HopByHop",   path = "isIpv6HopByHop", type = "bool"},
        {head = "IsIpv6LinkLocal",   path = "isIpv6LinkLocal", type = "bool"},
        {head = "IsIpv6Mld",   path = "isIpv6Mld", type = "bool"},
        {head = "IpHeaderOk",   path = "ipHeaderOk", type = "number"},
        {head = "VrfId",   path = "vrfId", type = "number"},
        {head = "IsUdbValid",   path = "isUdbValid", type = "bool"},
        {head = "UdbStdNotIp",   path = "UdbStdNotIp", type = "array"},
        {head = "UdbStdIpL2Qos",   path = "UdbStdIpL2Qos", type = "array"},
        {head = "UdbStdIpV4L4",   path = "UdbStdIpV4L4", type = "array"},
        {head = "UdbExtNotIpv6",   path = "UdbExtNotIpv6", type = "array"},
        {head = "UdbExtIpv6L2",   path = "UdbExtIpv6L2", type = "array"},
        {head = "UdbExtIpv6L4",   path = "UdbExtIpv6L4", type = "array"},
        {head = "Udb0_11",   path = "udb0_11", type = "array"},
        {head = "Udb17_22",   path = "udb17_22", type = "array"},
        {head = "Udb45_46",   path = "udb45_46", type = "array"},
        {head = "SrcPortOrTrunk",   path = "srcPortOrTrunk", type = "number"},
        {head = "SrcIsTrunk",   path = "srcIsTrunk", type = "number"},
        {head = "TrunkHash",   path = "trunkHash", type = "number"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E = tconcat(table_egress_ip_common, {
        {head = "Sip", path = "sip", type ="ipAddr"},
        {head = "Dip", path = "dip", type ="ipAddr"},
        {head = "ipv4Options", path = "ipv4Options", type = "number"},
        {head = "isVidx", path = "isVidx", type = "bool"},
        {head = "vid1", path = "vid1", type = "number"},
        {head = "up1", path = "up1", type = "number"},
        {head = "cfi1", path = "cfi1", type = "number"},
        {head = "isUdbValid", path = "isUdbValid", type = "bool"},
        {head = "udb36_49", path = "udb36_49", type = "array"},
        {head = "udb0", path = "udb0", type = "number"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E = tconcat(table_egress_ip_common, {
        {head = "Sip", path = "sip", type ="ipAddr"},
        {head = "Dip", path = "dip", type ="ipAddr"},
        {head = "isNd", path = "isNd", type = "bool"},
        {head = "isIpv6ExtHdrExist", path = "isIpv6ExtHdrExist", type = "bool"},
        {head = "isIpv6HopByHop", path = "isIpv6HopByHop", type = "bool"},
        {head = "isVidx", path = "isVidx", type = "bool"},
        {head = "vid1", path = "vid1", type = "number"},
        {head = "up1", path = "up1", type = "number"},
        {head = "cfi1", path = "cfi1", type = "number"},
        {head = "srcPort", path = "srcPort", type = "number"},
        {head = "trgPort", path = "trgPort", type = "number"},
        {head = "isUdbValid", path = "isUdbValid", type = "bool"},
        {head = "udb1_4", path = "udb1_4", type = "number"}
    }),

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E = tconcat(
        {
            {head = "udb", path = "udb", type = "array"}
        },
        tconcat(table_replaced_fld_ingress, table_udb60_fixed_fld_ingress)
    ),

    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E = tconcat(
        {
            {head = "udb", path = "udb", type = "array"}
        },
        tconcat(table_replaced_fld_egress, table_udb60_fixed_fld_egress)
    ),
}
table_info["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E"] = table_info["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E"]
table_info["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E"] = table_info["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E"]
table_info["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E"] = table_info["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E"]
table_info["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E"] = table_info["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E"]
table_info["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E"] = table_info["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E"]

table_info["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E"] = table_info["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E"]
table_info["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E"] = table_info["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E"]
table_info["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E"] = table_info["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E"]
table_info["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E"] = table_info["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E"]
table_info["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E"] = table_info["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E"]

local function getInfo(data, enumType)
    local ret = ""
    local pattern, mask
    local maskVal
    local patternVal
    local col

    if (data~=nil) then
        pattern=data["patternPtr"]
        mask=data["maskPtr"]
    end

    local headlen = 1
    for i = 1, #table_info[enumType] do
        col = table_info[enumType][i]
        local s = col.head
        if s == "" or s == nil then
            s = col.path
        end
        if string.len(s)+6 > headlen then
            headlen = string.len(s)+6
        end
    end
    for i = 1, #table_info[enumType] do
        col = table_info[enumType][i]

        maskVal = get_item_by_path(mask,col.path)

        patternVal = get_item_by_path(pattern,col.path)
        local val = nil
        local mask = nil
        if maskVal ~= nil then
            if col.type == "number" then
                if maskVal ~= 0 then
                    val = patternVal
                end
            elseif col.type == "hex" then
                if maskVal ~= 0 then
                    val = string.format("0x%x",patternVal)
                end
            elseif col.type == "bool" then
                if maskVal ~= 0 then
                    if patternVal ~= 0 then
                        val = "true"
                    else
                        val = "false"
                    end
                end
            elseif col.type == "ipAddr" then
                if  maskVal ~= "0.0.0.0" then
                    val = patternVal
                    mask = maskVal
                end
            elseif col.type == "macAddr" then
                if  maskVal ~= "00:00:00:00:00:00" then
                    val = patternVal
                    mask = maskVal
                end
            elseif col.type == "array" then
                mask = ""
                val = ""
                local fl = false

                for i=0,#maskVal do
                    mask = mask ..string.format(" 0x%02x", maskVal[i])
                    val = val .. string.format(" 0x%02x", patternVal[i])

                    if maskVal[i] ~= 0 then
                        fl = true
                    end
                end
                if not fl then
                    val = nil
                end

            end
        end

        if val ~= nil then
            local s = col.head
            if s == "" or s == nil then
                s = col.path
            end

            ret = ret .. string.format("%-"..tostring(headlen).."s: ",s) .. val .. "\n"
            if mask ~= nil then
                ret = ret .. string.format("%-"..tostring(headlen).."s: ",s.."(mask)") .. mask .. "\n"
            end
        end
    end
    return ret
end


local formatTable = {
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E = "ruleStdNotIp",

    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E = "ruleStdIpL2Qos",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E = "ruleStdIpv4L4",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E = "ruleStdIpv6Dip",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E = "ruleIngrStdUdb",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E = "ruleExtNotIpv6",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E = "ruleExtIpv6L2",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E = "ruleExtIpv6L4",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E = "ruleIngrExtUdb",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E = "ruleEgrStdNotIp",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E = "ruleEgrStdIpL2Qos",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E = "ruleEgrStdIpv4L4",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E = "ruleEgrExtNotIpv6",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E = "ruleEgrExtIpv6L2",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E = "ruleEgrExtIpv6L4",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E = "ruleStdIpv4RoutedAclQos",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E = "ruleExtIpv4PortVlanQos",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E = "ruleUltraIpv6PortVlanQos",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E = "ruleUltraIpv6RoutedAclQos",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E = "ruleEgrExtIpv4RaclVacl",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E = "ruleEgrUltraIpv6RaclVacl",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E = "ruleIngrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E = "ruleIngrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E = "ruleIngrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E = "ruleIngrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E = "ruleIngrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E = "ruleIngrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E = "ruleEgrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E = "ruleEgrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E = "ruleEgrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E = "ruleEgrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E = "ruleEgrUdbOnly",
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E = "ruleEgrUdbOnly"
}

-- 0 - ingress, 1 - egress
local formatToDirection =
{
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E = 1
}

--typedef enum
--{
--    0 CPSS_PCL_RULE_SIZE_STD_E,
--    0 CPSS_PCL_RULE_SIZE_30_BYTES_E = CPSS_PCL_RULE_SIZE_STD_E,
--    1 CPSS_PCL_RULE_SIZE_EXT_E,
--    1 CPSS_PCL_RULE_SIZE_60_BYTES_E = CPSS_PCL_RULE_SIZE_EXT_E,
--    2 CPSS_PCL_RULE_SIZE_ULTRA_E,
--    2 CPSS_PCL_RULE_SIZE_80_BYTES_E = CPSS_PCL_RULE_SIZE_ULTRA_E,
--    3 CPSS_PCL_RULE_SIZE_10_BYTES_E,
--    4 CPSS_PCL_RULE_SIZE_20_BYTES_E,
--    5 CPSS_PCL_RULE_SIZE_40_BYTES_E,
--    6 CPSS_PCL_RULE_SIZE_50_BYTES_E
--} CPSS_PCL_RULE_SIZE_ENT;
local formatToRuleSize =
{
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E = 2,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E = 2,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E = 2,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E = 3,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E = 4,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E = 5,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E = 6,
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E = 1,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E = 3,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E = 4,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E = 0,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E = 5,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E = 6,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E = 1
}

local ruleSizeToSip5RuleIndexStep = {
   [0] = 3,  --    CPSS_PCL_RULE_SIZE_STD_E (== CPSS_PCL_RULE_SIZE_30_BYTES_E),
   [1] = 6,  --    CPSS_PCL_RULE_SIZE_EXT_E (== CPSS_PCL_RULE_SIZE_60_BYTES_E),
   [2] = 12, --    CPSS_PCL_RULE_SIZE_ULTRA_E (== CPSS_PCL_RULE_SIZE_80_BYTES_E),
   [3] = 1,  --    CPSS_PCL_RULE_SIZE_10_BYTES_E,
   [4] = 2,  --    CPSS_PCL_RULE_SIZE_20_BYTES_E,
   [5] = 6,  --    CPSS_PCL_RULE_SIZE_40_BYTES_E,
   [6] = 6   --    CPSS_PCL_RULE_SIZE_50_BYTES_E
}

local ruleSizeToLegacyStdRulesAmount = {
    [0] = 1,  --    CPSS_PCL_RULE_SIZE_STD_E (== CPSS_PCL_RULE_SIZE_30_BYTES_E),
    [1] = 2,  --    CPSS_PCL_RULE_SIZE_EXT_E (== CPSS_PCL_RULE_SIZE_60_BYTES_E),
    [2] = 3   --    CPSS_PCL_RULE_SIZE_ULTRA_E (== CPSS_PCL_RULE_SIZE_80_BYTES_E),
}

-- ************************************************************************
--  show_access_list
--
--  @description
--
--  @param params    - params[devID] - The device number
--                      params[pclID] - The PCL ID (null for all)
--
--  @return true if successful, false with an error message if not
--
-- ************************************************************************
local function show_access_list(params)
    local index,value,i,v,ret,val,numRules,rules,tempPclID,devices,k
    local pclFrm = params["formatID"]
    local ruleSizeFromRule;
    local direction = formatToDirection[pclFrm];
    local ruleSize = formatToRuleSize[pclFrm];
    local ruleIndexStep;
    local isSip5Device;
    local tcamSizeDivider;
    local pclIdsRangeTab;

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end
    if pclFrm == nil then
        return false, "Pcl format is incorrect"
    end

    for k=1,#devices do
        local ruleIndexes={}

        isSip5Device = is_sip_5(devices[k]);
        if isSip5Device then
            ruleIndexStep = ruleSizeToSip5RuleIndexStep[ruleSize];
            if ruleIndexStep == nil then
                return false,"not supported rule format (sip5)";
            end
            tcamSizeDivider = ruleIndexStep;
        else -- for legacy devices
            ruleIndexStep = 1;
            tcamSizeDivider = ruleSizeToLegacyStdRulesAmount[ruleSize];
            if tcamSizeDivider == nil then
                return false,"not supported rule format (legacy)";
            end
        end

        ret,val = myGenWrapper("cpssDxChCfgTableNumEntriesGet",{
            {"IN","GT_U8","devNum",devices[k]},
            {"IN",TYPE["ENUM"],"table",3}, --ext rule
            {"OUT","GT_U32","numEntriesPtr"}   -- index of the rule
        })

        if (ret~=0) then return false,"Could not find max rule-iD size" end

        numRules=(val["numEntriesPtr"]/tcamSizeDivider) -1

        -- if params["ruleIdx"] == "all" or params["ruleIdx"] == nil then
        if type(params["ruleIdx"]) ~= "table" then
            pclIdsRangeTab = nil;
        else
            local _ndx, _id;
            pclIdsRangeTab = {};
            for _ndx, _id in pairs(params["ruleIdx"]) do
                pclIdsRangeTab[_id] = {};
            end
        end

        for i=0, numRules do
            local currentIndx = (i * ruleIndexStep);

            ret,val = myGenWrapper("cpssDxChPclPortGroupRuleAnyStateGet",{
                {"IN","GT_U8","devNum",devices[k]},
                {"IN","GT_U32","portGroupsBmp",0xFFFFFFFF},
                {"IN",TYPE["ENUM"],"ruleSize", ruleSize},
                {"IN","GT_U32","ruleIndex",currentIndx},   -- index of the rule
                {"OUT","GT_BOOL","valid"},
                {"OUT","CPSS_PCL_RULE_SIZE_ENT","ruleSize"}
            })

            if (ret==0) and (val.valid==true) then            --if pclid matches the one we want
                ruleSizeFromRule = cmdLuaCLI_callCfunction(
                    "mgmType_to_c_CPSS_PCL_RULE_SIZE_ENT",val.ruleSize)
                if ruleSizeFromRule == ruleSize then
                    local dev_ = devices[k]
                    tempPclID = wrLogWrapper(
                        "wrlCpssPclRuleIdtoPclIdGet",
                        "(dev_, currentIndx, ruleSize, direction)",
                        dev_, currentIndx, ruleSize, direction)

                    if tempPclID~=nil then
                        if (not pclIdsRangeTab) or (pclIdsRangeTab[tempPclID]) then
                            if (rules==nil) then
                                rules={}
                                rules[tempPclID]={}
                            else
                                if (rules[tempPclID]==nil) then
                                    rules[tempPclID]={}
                                end
                            end
                            table.insert(rules[tempPclID], currentIndx)
                        end
                    end
                end
            end
        end

        if rules==nil then
            print("No valid entries were found")
            return true
        end

        ------------------------------------------------------------------

        for index,value in pairs(rules) do
            local res_info = ""

            res_info = "\n\nDevice:"..devices[k].."\tPCL ID:"..index
            print(res_info)

            for i,v in ipairs(value) do
                res_info = "\nRule Index:"..v

                ret, val = myGenWrapper("cpssDxChPclRuleParsedGet", {
                    { "IN", "GT_U8", "devNum", devices[k] },
                    { "IN", "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT", "ruleFormat", pclFrm},
                    { "IN", "GT_U32", "ruleIndex", v },
                    { "IN", "CPSS_DXCH_PCL_RULE_OPTION_ENT", "ruleOptionsBmp" },
                    { "OUT","GT_BOOL", "isRuleValid" },
                    { "OUT","CPSS_DXCH_PCL_RULE_FORMAT_UNT_"..formatTable[pclFrm], "mask" },
                    { "OUT","CPSS_DXCH_PCL_RULE_FORMAT_UNT_"..formatTable[pclFrm], "pattern" },
                    { "OUT","CPSS_DXCH_PCL_ACTION_STC", "action" }})


                if ret == 0 and val.isRuleValid == true then
                    res_info = res_info..prvParseAction(val.action).."\n"
                    local rule_info =
                    {
                        maskPtr = val.mask[formatTable[pclFrm]],
                        patternPtr = val.pattern[formatTable[pclFrm]]
                    };
                    res_info = res_info..getInfo(rule_info, pclFrm);
                    if (type(autocheckDataCallback) == "function") then
                        autocheckDataCallback("pclRule",
                        {
                            ruleIndex   = v,
                            ruleFormat  = pclFrm,
                            ruleAction  = val.action,
                            ruleMask    = val.mask[formatTable[pclFrm]],
                            rulePattern = val.pattern[formatTable[pclFrm]]
                        });
                    end
                end
                print(res_info)
            end
        end
    end
    return true
end

-- ************************************************************************
--  show_pcl
--
--  @description
--
--  @param params    - params[devID] - The device number
--                      params[pclID] - The PCL ID (null for all)
--
--  @return true if successful, false with an error message if not
--
-- ************************************************************************
local function show_pcl(params)
    local index,value,i,v,ret,val,numRules,rules,tempPclID,devices,k
    local pclFrm = params["formatID"]
    local ruleSizeFromRule;
    local direction = formatToDirection[pclFrm];
    local ruleSize = formatToRuleSize[pclFrm];
    local ruleIndexStep;
    local isSip5Device;
    local tcamSizeDivider;
    local pclIndexesRangeTab;

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end
    if pclFrm == nil then
        return false, "Pcl format is incorrect"
    end

    for k=1,#devices do
        local ruleIndexes={}

        print("Rules on device " .. devices[k] .. "\n");

        isSip5Device = is_sip_5(devices[k]);
        if isSip5Device then
            ruleIndexStep = ruleSizeToSip5RuleIndexStep[ruleSize];
            if ruleIndexStep == nil then
                return false,"not supported rule format (sip5)";
            end
            tcamSizeDivider = ruleIndexStep;
        else -- for legacy devices
            ruleIndexStep = 1;
            tcamSizeDivider = ruleSizeToLegacyStdRulesAmount[ruleSize];
            if tcamSizeDivider == nil then
                return false,"not supported rule format (legacy)";
            end
        end

        ret,val = myGenWrapper("cpssDxChCfgTableNumEntriesGet",{
            {"IN","GT_U8","devNum",devices[k]},
            {"IN",TYPE["ENUM"],"table",3}, --ext rule
            {"OUT","GT_U32","numEntriesPtr"}   -- index of the rule
        })

        if (ret~=0) then return false,"Could not find max rule-iD size" end

        numRules=(val["numEntriesPtr"]/tcamSizeDivider) -1

        -- if params["ruleIdx"] == "all" or params["ruleIdx"] == nil then
        if type(params["ruleIdx"]) ~= "table" then
            pclIndexesRangeTab = nil;
        else
            local aligned_index;
            pclIndexesRangeTab = {};
            for _ndx, _ruleIdx in pairs(params["ruleIdx"]) do
                aligned_index = _ruleIdx - (_ruleIdx % ruleIndexStep);
                pclIndexesRangeTab[aligned_index] = {};
            end
        end

        for i=0, numRules do
            local currentIndx = (i * ruleIndexStep);

            if (not pclIndexesRangeTab) or pclIndexesRangeTab[currentIndx] then

                ret,val = myGenWrapper("cpssDxChPclPortGroupRuleAnyStateGet",{
                    {"IN","GT_U8","devNum",devices[k]},
                    {"IN","GT_U32","portGroupsBmp",0xFFFFFFFF},
                    {"IN",TYPE["ENUM"],"ruleSize", ruleSize},
                    {"IN","GT_U32","ruleIndex",currentIndx},   -- index of the rule
                    {"OUT","GT_BOOL","valid"},
                    {"OUT","CPSS_PCL_RULE_SIZE_ENT","ruleSize"}
                })

                if (ret==0) and (val.valid==true) then            --if pclid matches the one we want
                    ruleSizeFromRule = cmdLuaCLI_callCfunction(
                        "mgmType_to_c_CPSS_PCL_RULE_SIZE_ENT",val.ruleSize)
                    if ruleSizeFromRule == ruleSize then

                        ret, val = myGenWrapper("cpssDxChPclRuleParsedGet", {
                            { "IN", "GT_U8", "devNum", devices[k] },
                            { "IN", "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT", "ruleFormat", pclFrm},
                            { "IN", "GT_U32", "ruleIndex", currentIndx },
                            { "IN", "CPSS_DXCH_PCL_RULE_OPTION_ENT", "ruleOptionsBmp" },
                            { "OUT","GT_BOOL", "isRuleValid" },
                            { "OUT","CPSS_DXCH_PCL_RULE_FORMAT_UNT_"..formatTable[pclFrm], "mask" },
                            { "OUT","CPSS_DXCH_PCL_RULE_FORMAT_UNT_"..formatTable[pclFrm], "pattern" },
                            { "OUT","CPSS_DXCH_PCL_ACTION_STC", "action" }})


                        if ret == 0 and val.isRuleValid == true then
                            res_info = "\nRule Index:"..currentIndx;
                            res_info = res_info..prvParseAction(val.action).."\n";
                            local rule_info =
                            {
                                maskPtr = val.mask[formatTable[pclFrm]],
                                patternPtr = val.pattern[formatTable[pclFrm]]
                            };
                            res_info = res_info..getInfo(rule_info, pclFrm);
                            print(res_info);
                            if (type(autocheckDataCallback) == "function") then
                                autocheckDataCallback("pclRule",
                                {
                                    ruleIndex   = currentIndx,
                                    ruleFormat  = pclFrm,
                                    ruleAction  = val.action,
                                    ruleMask    = val.mask[formatTable[pclFrm]],
                                    rulePattern = val.pattern[formatTable[pclFrm]]
                                });
                            end
                        end
                    end
                end
            end
        end
    end
    print("End of rules \n");
    return true
end

local function CLI_check_param_ruleid_range(param, data)
    local n = tonumber(param)
    local ret = {}

    if param == "all" then
        return true, "all"
    end
    if n ~= nil then
        if n < 0 or n > 1023 then
            return false, "out of range"
        end
        return true, { n }
    end
    for w in string.gmatch(param, "[^,]+") do
        --
        if (tonumber(w) ~= nil) then
            table.insert(ret, tonumber(w))
        else
            local a, b, i
            a,b = string.match(w, "^(%d+)-(%d+)$")
            a = tonumber(a)
            b = tonumber(b)
            if (b ~= nil) and (a < b) then
                for i = a, b do
                    if i < 0 or i > 1023 then
                        return false, "invalid range"
                    end
                    table.insert(ret, i)
                end
            else
                return false, "invalid range"
            end
        end
     end
     return true, ret
end

--------------------------------------------
-- command registration: show access-list
--------------------------------------------

CLI_type_dict["pclRuleFormat"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "PCL rule format\n",
    enum = {
        ["ingress_std_not_ip"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E", help="Standard key used for non-IP packets"},
        ["ingress_std_ip_l2_qos"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E", help="Standard key used for IPv6 packets and one of the two standard keys used forIPv4 packets and ARP packets"},
        ["ingress_std_ipv4_l4"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E", help="One of the two standard keys used for IPv4 or ARP packets"},
        ["ingress_std_ipv6_dip"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E", help="DxCh specific IPV6 DIP (used for routing)"},
        ["ingress_ext_not_ipv6"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E", help="Extended key used for IPv4 packets, ARP packets, and non-IP packets"},
        ["ingress_ext_ipv6_l2"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E", help="One of the two extended keys used for IPv6 packets"},
        ["ingress_ext_ipv6_l4"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E", help="One of the two extended keys used for IPv6 packets"},
        ["egress_std_not_ip"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E", help="Standard (24B) L2"},
        ["egress_std_ip_l2_qos"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E", help="Standard key used for IPv6 packets and one of the two standard keys used for IPv4 packets and ARP packets"},
        ["egress_std_ipv4_l4"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E", help="One of the two standard keys used for IPv4 or ARP packets"},
        ["egress_ext_not_ipv6"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E", help="Extended key used for IPv4 packets, ARP packets, and non-IP packets"},
        ["egress_ext_ipv6_l2"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E", help="One of the two extended keys used for IPv6 packets (L2+IPv6)"},
        ["egress_ext_ipv6_l4"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E", help="One of the two extended keys used for IPv6 packets (L4+IPv6)"},
        ["ingress_std_udb"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E", help="Standard key with 20 User Defined Bytes"},
        ["ingress_ext_udb"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E", help="Extended key with 24 Used Defined Bytes"},
        ["ingress_std_ipv4_routed_acl_qos"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E", help="Custom key used for IPv4 packets"},
        ["ingress_ext_ipv4_port_vlan_qos"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E", help="Custom key used for IPv4 packets"},
        ["ingress_ultra_ipv6_port_vlan_qos"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E", help="Custom key used for IPv6 packets"},
        ["ingress_ultra_ipv6_routed_acl_qos"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E", help="Custom key used for IPv6 packets"},
        ["egress_ext_ipv4_racl_vacl"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E", help="Custom key used for IPv4 packets"},
        ["egress_ultra_ipv6_racl_vacl"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E", help="Custom key used for IPv6 packets"},
        ["ingress_udb_10"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E", help="Contains 10 User Defined Bytes"},
        ["ingress_udb_20"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E", help="Contains 20 User Defined Bytes"},
        ["ingress_udb_30"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E", help="Contains 30 User Defined Bytes"},
        ["ingress_udb_40"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E", help="Contains 40 User Defined Bytes"},
        ["ingress_udb_50"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E", help="Contains 50 User Defined Bytes"},
        ["ingress_udb_60"] = { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E", help="Contains 50 User Defined Bytes with fixed fields from the packet meta data"},
        ["egress_udb_10"] =  { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E",  help="Contains 10 User Defined Bytes"},
        ["egress_udb_20"] =  { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E",  help="Contains 20 User Defined Bytes"},
        ["egress_udb_30"] =  { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E",  help="Contains 30 User Defined Bytes"},
        ["egress_udb_40"] =  { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E",  help="Contains 40 User Defined Bytes"},
        ["egress_udb_50"] =  { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E",  help="Contains 50 User Defined Bytes"},
        ["egress_udb_60"] =  { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E",  help="Contains 50 User Defined Bytes with fixed fields from the packet meta data"}
    }
}


CLI_type_dict["ruleRange"] = {
    checker = CLI_check_param_ruleid_range,
    help="Pcl Id range (all, 0-5, 10)"
}

CLI_addCommand("exec", "show access-list", {
  func=show_access_list,
  help="Show a device's PCL-ID access-list",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="pcl-id %ruleRange", name="ruleIdx", help="Range of the pclIds (0-1023)" },
      { format="format %pclRuleFormat", name="formatID", help="Rule format" },
      mandatory = {"devID","ruleIdx", "formatID"},
    }
  }
})

CLI_addCommand("exec", "show pcl", {
  func=show_pcl,
  help="Show a device's pcl rule",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="rule-id %ruleRange", name="ruleIdx", help="Range of the rule indexes" },
      { format="format %pclRuleFormat", name="formatID", help="Rule format" },
      mandatory = {"devID","ruleIdx", "formatID"},
    }
  }
})

--------------------------------------------
--initialize the User Defined Bytes to ipv6 dest address
--------------------------------------------
function udb_init()
    if luaGlobalGet("udb_init")==nil then
        local packetType

        if (is_sip_5(nil)) then
            packetType = 12 -- CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E
        else
            packetType = 7 -- CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E
        end

        local all_devices = wrLogWrapper("wrlDevList")
        local i,j
        local ret, val
        for j=1, #all_devices do
            for i=0,15 do
                ret,val = myGenWrapper("cpssDxChPclUserDefinedByteSet",{
                    {"IN","GT_U8","devNum",all_devices[j]},
                    {"IN",TYPE["ENUM"],"ruleFormat",3},  --begins at ipv6 dest address
                    {"IN",TYPE["ENUM"],"packetType",packetType},
                    {"IN",TYPE["ENUM"],"direction",0},   -- ingress
                    {"IN","GT_U32","udbIndex",i},
                    {"IN",TYPE["ENUM"],"offsetType",5},   -- ipv6 header offset
                    {"IN","GT_U8","offset",26+i} --begining (the destination address)
                })
                if ret ~= 0 then
                    return ret
                end
            end
        end
        luaGlobalStore("udb_init", true)
    end
    return 0
end
