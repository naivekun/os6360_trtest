--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PCLAddRule.lua
--*
--* DESCRIPTION:
--*       adding a new rule to the PCL
--*
--* FILE REVISION NUMBER:
--*       $Revision: 11 $
--*
--********************************************************************************

--includes
if not cli_C_functions_registered("wrlCpssDxChTcamUtilizationGet") then
    function wrlCpssDxChTcamUtilizationGet()
        return 0x11 -- GT_NOT_IMPLEMENTED
    end
end


--constants

-- ************************************************************************
--  addPCLRule_cheetah1
--
--  @description adds a new PCL rule
--
--  @param params - params["ruleID"]        - the rule ID number
--          params["action"]        - action to perform : permit or deny or trap
--          params["srcPort"]        - source port
--          params["isTagged"]      - is the port vlan tagged : true or false
--          params["vid"]          - vlan id
--          params["up"]          - user priority
--          params["qosProfile"]      - qos profile
--          params["macDest"]        - Mac destination address
--          params["macSrc"]        - Mac source address
--          params["dscp"]        - Differentiated Services Code Point
--          params["ipv4-src"]      - IPv4 source address
--          params["ipv6-src"]      - IPv6 source address
--          params["ipv4-dest"]      - IPv4 destination address
--          params["ipv6-dest"]      - IPv6 destination address
--
--      common_global[g_pclIDDev]   - the device number
--    @usage __global       - __global["g_pclIDDev"]: device
--    @usage __global       - __global["g_pclID"]: pcl id of the access list
--    @usage __global       - __global["g_pclDirection"]: ingress/egress
--
--      @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function addPCLRule_cheetah1(params)
    local devNum,actionSTC,rule,ruleMsk,tempMsk,devices,j, direction
    local ret, val
    
    devNum = getGlobal("g_pclIDDev")
    -- (devNum=="all") or passed device number
    -- filter Cheeta devices only
    devices = generic_getDeviceOfFamilyList(devNum, {"CPSS_PP_FAMILY_CHEETAH_E"});
    if not next(devices) then
        -- no devices to treat
        return true;
    end

    direction = getGlobal("g_pclDirection")
    if direction ~= "ingress" then
        return false,"ingress direction only supported"
    end

    -- not supported parameters of general command

    if (params["ipv6-source"] or params["ipv6-destination"]) then
        print("IPv6 packets not supported by ACL ")
        return false,"IPv6 packets not supported by ACL "
    end

    if (params["tpidIndex"] or params["tpid1Index"]) then
        print("Error: TPID Index classification isn't supported")
        return false,"Error, rule could not be added"
    end

  if((params["redirect_ethernet"]) and (params["port_channel"])) then
    print(" Simultaneous Redirect to both ethernet and port-channel not possible")
    return false, "Simultaneous Redirect to both ethernet and port-channel not possible"
  end

    if params["counters"] then
    print("For Cheetat1 use match_counter instead ")
    return false, "For Cheetat1 use match_counter instead"
    end

  --print(to_string(params))

  -- enums
  local CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E = 4
  local CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E = 1
  local CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E = 1
  local CPSS_INTERFACE_PORT_E = 0
  local CPSS_INTERFACE_TRUNK_E = 1
    local CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E = 1
    local CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E = 1

    actionSTC={mirror={cpuCode="CPSS_NET_ETH_BRIDGED_LLT_E"}}
    rule={}
    ruleMsk={}
    tempMsk={}

    rule["common"]={}
    ruleMsk["common"]={}
    rule["common"]["pclId"]=getGlobal("g_pclID")
    ruleMsk["common"]["pclId"]=0x3ff

    if params["action"]=="deny" then -- both ingress and egress
        actionSTC["pktCmd"]=3   --hard drop
    elseif params["action"]=="trap" then
        actionSTC["pktCmd"]=2   --trap to cpu (relevant for ingress only)
    elseif (params["policerID"]~=nil) then
        local policy_mode = params.policy_mode;
        if not policy_mode then
            policy_mode = "CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E";
        end
        actionSTC["policer"]={};
        actionSTC["policer"]["policerEnable"] = policy_mode;
        actionSTC["policer"]["policerId"]=params["policerID"];
    end

    actionSTC.mirror = {};
    if params["cpu_code_number"] then
        -- CPU Code CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + given HW value
        actionSTC.mirror.cpuCode = 1024 + params["cpu_code_number"];
    else
        -- default CPU Code CPSS_NET_FIRST_USER_DEFINED_E
        actionSTC.mirror.cpuCode = 500;
    end

    if params["match_counter"] ~= nil then
        actionSTC["matchCounter"] = {}
        actionSTC["matchCounter"]["enableMatchCount"] = true
        actionSTC["matchCounter"]["matchCounterIndex"] = params["match_counter"]
    end

    if params["modifyVlan0"] ~= nil then
        actionSTC["vlan"] = {}
        actionSTC["vlan"]["ingress"] = {}
        actionSTC["vlan"]["ingress"]["modifyVlan"] = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E
        actionSTC["vlan"]["ingress"]["vlanId"] = params["modifyVlan0"]
        actionSTC["vlan"]["ingress"]["precedence"]  = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E
    end
    if params["redirect_ethernet"] ~= nil then
        local redirectPort = params["redirect_ethernet"]["portNum"]
        local redirectDevNum = params["redirect_ethernet"]["devId"]
        local ret, hwDevId = device_to_hardware_format_convert(redirectDevNum)
        actionSTC["redirect"] = {}
        actionSTC["redirect"]["redirectCmd"] = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E
        actionSTC["redirect"]["data"] = {}
        actionSTC["redirect"]["data"]["outIf"] = {}
        actionSTC["redirect"]["data"]["outIf"]["outInterface"] = {}
        actionSTC["redirect"]["data"]["outIf"]["outInterface"]["type"] = CPSS_INTERFACE_PORT_E
        actionSTC["redirect"]["data"]["outIf"]["outInterface"]["devPort"] = {}
        actionSTC["redirect"]["data"]["outIf"]["outInterface"]["devPort"]["devNum"] = hwDevId  -- Use devNum in LUA table but not hwDevNum as in struct.
        actionSTC["redirect"]["data"]["outIf"]["outInterface"]["devPort"]["portNum"] = redirectPort
        actionSTC["bypassBridge"] = true -- Bypass bridge must be set to avoid redirection action override by Bridge
    end
    if params["redirect_vidx"] ~= nil then
        actionSTC.redirect = {
            redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E,
            data = {
                outIf = {
                    outInterface = {
                        type = "CPSS_INTERFACE_VIDX_E",
                        vidx = params.redirect_vidx
                    }
                }
            }
        }
        actionSTC.bypassBridge = true -- Bypass bridge must be set to avoid redirection action override by Bridge
    end
    if params["port_channel"] ~= nil then
        actionSTC["redirect"] = {}
        actionSTC["redirect"]["redirectCmd"] = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E
        actionSTC["redirect"]["data"] = {}
        actionSTC["redirect"]["data"]["outIf"] = {}
        actionSTC["redirect"]["data"]["outIf"]["outInterface"] = {}
        actionSTC["redirect"]["data"]["outIf"]["outInterface"]["type"] = CPSS_INTERFACE_TRUNK_E
        actionSTC["redirect"]["data"]["outIf"]["outInterface"]["trunkId"] = params["port_channel"]
        actionSTC["bypassBridge"] = true -- Bypass bridge must be set to avoid redirection action override by Bridge
    end

    if params["alt1"]~=nil then
        actionSTC["qos"]={}
        actionSTC["qos"]["ingress"]={}
        actionSTC["qos"]["ingress"]["profileAssignIndex"]=true

        if params["alt1"]=="set-vpt" then
            actionSTC["qos"]["ingress"]["profileIndex"]=64+8+params[params["alt1"]]
            actionSTC["qos"]["ingress"]["modifyUp"]=2
        elseif params["alt1"]=="set-dscp" then
            actionSTC["qos"]["ingress"]["profileIndex"]=params[params["alt1"]]
            actionSTC["qos"]["ingress"]["modifyDscp"]=2
        elseif params["alt1"]=="set-queue" then
            actionSTC["qos"]["ingress"]["profileIndex"]=64-1+params[params["alt1"]]
        elseif params["alt1"]=="set-qos-profile-index" then
          actionSTC["qos"]["ingress"]["profileIndex"]=params[params["alt1"]]
        end
    end

    if params["isTagged"] then
        if params["isTagged"] == true then
            rule["common"]["isTagged"]=1
        else
            rule["common"]["isTagged"]=0
        end
        ruleMsk["common"]["isTagged"]=1
    end

    if params["vid"]~=nil then
    local any = false
    if params["vid"] == "any" then -- check if "any" vlan
      any = true
    end
        if any == true then
            rule["common"]["isTagged"]=1
            ruleMsk["common"]["isTagged"]=1
        else
            rule["common"]["vid"]=tonumber(params["vid"])
            ruleMsk["common"]["vid"]=0xFFF
        end
    end

    if params["up"]~=nil then
        rule["common"]["up"]=params["up"]
        ruleMsk["common"]["up"]=0x7
    end

    if params["qosProfile"]~=nil then
        rule["common"]=params["qosProfile"]
        ruleMsk["common"]=0xFFFFFFFF
    end

    if params["dscp"]~=nil then
        rule["commonExt"]["dscp"]=params["dscp"]
        ruleMsk["commonExt"]["dscp"]=0x3F
    end

    if params["srcPort"]~=nil then
        rule["common"]["sourcePort"]=params["srcPort"]
        ruleMsk["common"]["sourcePort"]=0xFFFFFFFF
    end


    if (params["ipv4-source"] or params["ipv4-destination"]) then
        rule["common"]["isIp"]=1
        ruleMsk["common"]["isIp"]=0x1
    end

    -- rule["commonExt"]["isIpv6"]=0
    -- ruleMsk["commonExt"]["isIpv6"]=0

    --ipv4
    if params["ipv4-source"]~=nil then
    local ipv4SrcMsk = params["ipv4-source"][5]
    if ipv4SrcMsk == nil then  ipv4SrcMsk = {"255","255","255","255"} end
        rule["sip"]=prvAddAddr(params["ipv4-source"],10,4)
        ruleMsk["sip"]=prvAddAddr(ipv4SrcMsk,10,4)
        prvMaskAddress(rule["sip"],ruleMsk["sip"])
    end

    if params["ipv4-destination"]~=nil then
        local ipv4DestMsk = params["ipv4-destination"][5]
    if ipv4DestMsk == nil then ipv4DestMsk = {"255","255","255","255"} end
        rule["dip"]=prvAddAddr(params["ipv4-destination"],10,4)
        ruleMsk["dip"]=prvAddAddr(ipv4DestMsk,10,4)
        prvMaskAddress(rule["dip"],ruleMsk["dip"])
    end

    --Mac
    if params["macSrc"]~=nil then
        local macSrcMsk=params["macSrc"][7]
        if macSrcMsk == nil then macSrcMsk = {"ff","ff","ff","ff","ff","ff"} end
        rule["macSa"]=prvAddAddr(params["macSrc"],16,6)
        ruleMsk["macSa"]=prvAddAddr(macSrcMsk,16,6)
        prvMaskAddress(rule["macSa"],ruleMsk["macSa"])
    end

    if params["macDest"]~=nil then
        local macDestMsk = params["macDest"][7]
        if params["macDestMsk"]==nil then  params["macDestMsk"]={"ff","ff","ff","ff","ff","ff"} end
        rule["macDa"]=prvAddAddr(params["macDest"],16,6)
        ruleMsk["macDa"]=prvAddAddr(macDestMsk,16,6)
        prvMaskAddress(rule["macDa"],ruleMsk["macDa"])
    end

    rule["common"]["isL2Valid"]=1
    ruleMsk["common"]["isL2Valid"]=0x1

    local patternAndMaskType, ruleFormat
    patternAndMaskType = "CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC"
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E

    for j=1, #devices do
        ret = myGenWrapper("cpssDxChPclRuleSet",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN",TYPE["ENUM"],"ruleFormat",ruleFormat},  --Ext UDB
            {"IN","GT_U32","ruleIndex",params["ruleID"]},   -- index of the rule
            {"IN",TYPE["ENUM"],"ruleOptionsBmp",0}, --Rule is valid
            {"IN",patternAndMaskType,"maskPtr",ruleMsk},  -- The ext UDB rules
            {"IN",patternAndMaskType,"patternPtr",rule},  --The ext UDB mask
            {"IN","CPSS_DXCH_PCL_ACTION_STC","actionPtr",actionSTC} -- The action to be taken (permit/deny)
        })

        if ret~=0 then
            print("cpssDxChPclRuleSet() failed: "..returnCodes[ret])
            return false,"Error, rule could not be added"
        end
    end
    --print("\nRule:"..to_string(rule).."\n\nRule mask:\n"..to_string(ruleMsk).."\n\n")
    --print("Ret:\n"..to_string(ret))
    --print("Action:\n"..to_string(actionSTC))

    return true
end


-- ************************************************************************
--  addPCLRule
--
--  @description adds a new PCL rule
--
--  @param params - params["ruleID"]        - the rule ID number
--          params["action"]        - action to perform : permit or deny or trap
--          params["srcPort"]        - source port
--          params["isTagged"]      - is the port vlan tagged : true or false
--          params["vid"]          - vlan id
--          params["up"]          - user priority
--          params["qosProfile"]      - qos profile
--          params["macDest"]        - Mac destination address
--          params["macSrc"]        - Mac source address
--          params["dscp"]        - Differentiated Services Code Point
--          params["ipv4-src"]      - IPv4 source address
--          params["ipv6-src"]      - IPv6 source address
--          params["ipv4-dest"]      - IPv4 destination address
--          params["ipv6-dest"]      - IPv6 destination address
--
--      common_global[g_pclIDDev]   - the device number
--    @usage __global       - __global["g_pclIDDev"]: device
--    @usage __global       - __global["g_pclID"]: pcl id of the access list
--    @usage __global       - __global["g_pclDirection"]: ingress/egress
--
--      @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function addPCLRule(params)
    local devNum,actionSTC,rule,ruleMsk,tempMsk,devices,j, direction
    local ret, val
    
    if ((params["ipv6-source"] or params["ipv6-destination"]) and (params["ipv4-source"] or params["ipv4-destination"] or params["macSrc"] or params["macDest"]))~=nil then
        print("IPv6 cannot be used with IPv4 or Mac")
        return false,"IPv6 cannot be used with IPv4 or Mac"
    end

  if((params["redirect_ethernet"]) and (params["port_channel"]))~=nil then
    print(" Simultaneous Redirect to both ethernet and port-channel not possible")
    return false, "Simultaneous Redirect to both ethernet and port-channel not possible"
  end

  --print(to_string(params))

  -- enums
  local CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E = 1
  local CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E = 2
  local CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E = 10
  local CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E = 14
  local CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E = 1
  local CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E = 1
  local CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E = 1
  local CPSS_INTERFACE_PORT_E = 0
  local CPSS_INTERFACE_TRUNK_E = 1
    local CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E = 1
    local CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E = 1

    -- treat Cheetah devices only and check result
    local cheetah_rc, cheetah_err = addPCLRule_cheetah1(params);
    if not cheetah_rc then
        return cheetah_rc, cheetah_err;
    end

    -- treat Not Cheetah devices
    devNum = getGlobal("g_pclIDDev")
    -- (devNum=="all") or passed device number
    -- filter Not Cheetah devices only
    devices = generic_getDeviceOfNotFamilyList(devNum, {"CPSS_PP_FAMILY_CHEETAH_E"});
    if not next(devices) then
        -- no devices to treat
        return true;
    end

  direction = getGlobal("g_pclDirection")
    actionSTC={mirror={cpuCode="CPSS_NET_ETH_BRIDGED_LLT_E"}}
    rule={}
    ruleMsk={}
    tempMsk={}

  if direction == "ingress" then
    rule["commonIngrUdb"]={}
    ruleMsk["commonIngrUdb"]={}
    rule["commonIngrUdb"]["pclId"]=getGlobal("g_pclID")
    ruleMsk["commonIngrUdb"]["pclId"]=0x3ff
  elseif direction == "egress" then
    rule["common"]={}
    rule["commonExt"]={}
    ruleMsk["common"]={}
    ruleMsk["commonExt"]={}
    rule["common"]["pclId"]=getGlobal("g_pclID")
    ruleMsk["common"]["pclId"]=0x3ff
  end

    if params["action"]=="deny" then -- both ingress and egress
        actionSTC["pktCmd"]=3   --hard drop
    elseif params["action"]=="trap" then
        actionSTC["pktCmd"]=2   --trap to cpu (relevant for ingress only)
    elseif (params["policerID"]~=nil) then
        local policy_mode = params.policy_mode;
        if not policy_mode then
            policy_mode = "CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E";
        end
        actionSTC["policer"]={};
        actionSTC["policer"]["policerEnable"] = policy_mode;
        actionSTC["policer"]["policerId"]=params["policerID"];
    end

    actionSTC.mirror = {};
    if params["cpu_code_number"] then
        -- CPU Code CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + given HW value
        actionSTC.mirror.cpuCode = 1024 + params["cpu_code_number"];
    else
        -- default CPU Code CPSS_NET_FIRST_USER_DEFINED_E
        actionSTC.mirror.cpuCode = 500;
    end

  if direction == "ingress" then -- set action for CNC
    if params["counters"] ~= nil then
      actionSTC["matchCounter"] = {}
      actionSTC["matchCounter"]["enableMatchCount"] = true
      actionSTC["matchCounter"]["matchCounterIndex"] = params["ruleID"]
        end
  end

    if direction == "ingress" then -- set action redirect
        if params["modifyVlan0"] ~= nil then
            actionSTC["vlan"] = {}
            actionSTC["vlan"]["ingress"] = {}
            actionSTC["vlan"]["ingress"]["modifyVlan"] = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E
            actionSTC["vlan"]["ingress"]["vlanId"] = params["modifyVlan0"]
            actionSTC["vlan"]["ingress"]["precedence"]  = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E
        end
        if params["redirect_ethernet"] ~= nil then
            local redirectPort = params["redirect_ethernet"]["portNum"]
            local redirectDevNum = params["redirect_ethernet"]["devId"]
            local ret, hwDevId = device_to_hardware_format_convert(redirectDevNum)
            actionSTC["redirect"] = {}
            actionSTC["redirect"]["redirectCmd"] = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E
            actionSTC["redirect"]["data"] = {}
            actionSTC["redirect"]["data"]["outIf"] = {}
            actionSTC["redirect"]["data"]["outIf"]["outInterface"] = {}
            actionSTC["redirect"]["data"]["outIf"]["outInterface"]["type"] = CPSS_INTERFACE_PORT_E
            actionSTC["redirect"]["data"]["outIf"]["outInterface"]["devPort"] = {}
            actionSTC["redirect"]["data"]["outIf"]["outInterface"]["devPort"]["devNum"] = hwDevId  -- Use devNum in LUA table but not hwDevNum as in struct.
            actionSTC["redirect"]["data"]["outIf"]["outInterface"]["devPort"]["portNum"] = redirectPort
            actionSTC["bypassBridge"] = true -- Bypass bridge must be set to avoid redirection action override by Bridge
        end
        if params["redirect_vidx"] ~= nil then
            actionSTC.redirect = {
                redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E,
                data = {
                    outIf = {
                        outInterface = {
                            type = "CPSS_INTERFACE_VIDX_E",
                            vidx = params.redirect_vidx
                        }
                    }
                }
            }
            actionSTC.bypassBridge = true -- Bypass bridge must be set to avoid redirection action override by Bridge
        end
      if params["port_channel"] ~= nil then
        actionSTC["redirect"] = {}
            actionSTC["redirect"]["redirectCmd"] = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E
            actionSTC["redirect"]["data"] = {}
            actionSTC["redirect"]["data"]["outIf"] = {}
            actionSTC["redirect"]["data"]["outIf"]["outInterface"] = {}
            actionSTC["redirect"]["data"]["outIf"]["outInterface"]["type"] = CPSS_INTERFACE_TRUNK_E
            actionSTC["redirect"]["data"]["outIf"]["outInterface"]["trunkId"] = params["port_channel"]
            actionSTC["bypassBridge"] = true -- Bypass bridge must be set to avoid redirection action override by Bridge
      end

    end

  if direction=="egress" then
    actionSTC["egressPolicy"] = true
    if params["modifyVlan0"] ~= nil then
      actionSTC["vlan"] = {}
      actionSTC["vlan"]["egress"] = {}
      
      if (is_sip_5(nil)) then
        actionSTC["vlan"]["egress"]["vlanCmd"] = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E
      else
        actionSTC["vlan"]["egress"]["vlanCmd"] = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E        
      end
        
      actionSTC["vlan"]["egress"]["vlanId"] = params["modifyVlan0"]
    end
    if params["modifyVlan1"] ~= nil then
      if actionSTC["vlan"] == nil then actionSTC["vlan"] = {} end
      if actionSTC["vlan"]["egress"] == nil then actionSTC["vlan"]["egress"] = {} end
      actionSTC["vlan"]["egress"]["vlanId1ModifyEnable"] = true
      actionSTC["vlan"]["egress"]["vlanId1"] = params["modifyVlan1"]
    end
  end

  if direction == "ingress" then --Ingress QOS
    if params["alt1"]~=nil then
      actionSTC["qos"]={}
      actionSTC["qos"]["ingress"]={}
      actionSTC["qos"]["ingress"]["profileAssignIndex"]=true

      if params["alt1"]=="set-vpt" then
        actionSTC["qos"]["ingress"]["profileIndex"]=64+8+params[params["alt1"]]
        actionSTC["qos"]["ingress"]["modifyUp"]=2


       for j=1, #devices do
       
        -- update UP in qos profile begin
        ret,val = myGenWrapper("cpssDxChCosProfileEntryGet",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN","GT_U32","profileIndex",64+8+params[params["alt1"]]},   
            {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
        })

        if (ret==0) then val["cosPtr"]["userPriority"]=params[params["alt1"]]
            ret = myGenWrapper("cpssDxChCosProfileEntrySet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_U32","profileIndex",64+8+params[params["alt1"]]},    
                {"IN","CPSS_DXCH_COS_PROFILE_STC","cosPtr",val["cosPtr"]}
            })
	    
        else
            return false,"Could not set profile number "..64+8+params[params["alt1"]]
        end
        --update UP in qos profile end 
        
      end
        
      elseif params["alt1"]=="set-dscp" then
        actionSTC["qos"]["ingress"]["profileIndex"]=params[params["alt1"]]
        actionSTC["qos"]["ingress"]["modifyDscp"]=2
      elseif params["alt1"]=="set-queue" then
        actionSTC["qos"]["ingress"]["profileIndex"]=64-1+params[params["alt1"]]
      elseif params["alt1"]=="set-qos-profile-index" then
        actionSTC["qos"]["ingress"]["profileIndex"]=params[params["alt1"]]
      end
    end
  elseif direction == "egress" then --Egress QOS
    if params["alt1"]~=nil then
      actionSTC["qos"]={}
      actionSTC["qos"]["egress"]={}
      if params["alt1"]=="set-vpt" then
      
        if (is_sip_5(nil)) then
          actionSTC["qos"]["egress"]["modifyUp"] = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E
        else
          actionSTC["qos"]["egress"]["modifyUp"] = CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E       
        end        
        
        actionSTC["qos"]["egress"]["up"] = params[params["alt1"]]
        
      elseif params["alt1"]=="set-dscp" then
        actionSTC["qos"]["egress"]["up1ModifyEnable"] = true
        actionSTC["qos"]["egress"]["up1"] = params[params["alt1"]]
      end
    end
  end

    if params["isTagged"]~=nil then
    if direction == "ingress" then -- Ingress isTagged
      if params["isTagged"]==true then rule["commonIngrUdb"]["pktTagging"]=3 end
      ruleMsk["commonIngrUdb"]["pktTagging"]=0xFFFFFFFF
    elseif direction == "egress" then -- Egress isTagged
      if params["isTagged"] == true then
        rule["common"]["isTagged"] = 1
        ruleMsk["common"]["isTagged"] = 0x1
      end
    end
    end

    if params["vid"]~=nil then
    local any = false
    if params["vid"] == "any" then -- check if "any" vlan
      any = true
    end
    if direction == "ingress" then -- Ingress vid
      rule["commonIngrUdb"]["vid"]=tonumber(params["vid"])
      if any == true then
        rule["commonIngrUdb"]["pktTagging"] = 0x3 -- vlan0 exists
        ruleMsk["commonIngrUdb"]["pktTagging"] = 0xFFFFFFFF
        ruleMsk["commonIngrUdb"]["vid"]=0x0 -- any vlan -> mask == 0
      else
        ruleMsk["commonIngrUdb"]["vid"]=0xFFF -- specific vlan -> mask == 0xFFF
      end
    elseif direction == "egress" then -- Egress vid
      rule["common"]["vid"] = tonumber(params["vid"])
      if any == true then
        rule["common"]["isTagged"] = 1 -- vlan1 exists
        ruleMsk["common"]["isTagged"] = 0x1
        ruleMsk["common"]["vid"] = 0x0 -- any vlan -> mask == 0
      else
        ruleMsk["common"]["vid"] = 0xFFF -- specific vlan -> mask == 0xFFF
      end
    end
    end

    if params["vid1"]~=nil then
    local any = false
    if params["vid1"] == "any" then
      any = true
    end
    if direction == "ingress" then -- Ingress vid1
      rule["vid1"]=tonumber(params["vid1"])
      if any == true then
        rule["tag1Exist"] = 0x1 -- vlan0 exists
        ruleMsk["tag1Exist"] = 0xFFFFFFFF
        ruleMsk["vid1"]=0x0 -- any vlan -> mask == 0
      else
        ruleMsk["vid1"]=0xFFF -- specific vlan -> mask == 0xFFF
      end
    elseif direction == "egress" then -- Egress vid1
      rule["vid1"] = tonumber(params["vid1"])
      if any == true then
        rule["common"]["tag1Exist"] = 1 -- vlan1 exists
        ruleMsk["common"]["tag1Exist"] = 0x1
        ruleMsk["vid1"] = 0x0 -- any vlan -> mask == 0
      else
        ruleMsk["vid1"] = 0xFFF -- specific vlan -> mask == 0xFFF
      end
    end
    end


    if params["up"]~=nil then
    if direction == "ingress" then --Ingress up
      rule["commonIngrUdb"]["up"]=params["up"]
      ruleMsk["commonIngrUdb"]["up"]=0x7
    elseif direction == "egress" then --Egress up
      rule["common"]["up"] = params["up"]
      ruleMsk["common"]["up"] = 0x7
    end
    end

    if params["qosProfile"]~=nil then
    if direction == "ingress" then -- Ingress QOS profile
      rule["qosProfile"]=params["qosProfile"]
      ruleMsk["qosProfile"]=0xFFFFFFFF
    elseif direction == "egress" then -- Egress QOS profile
      rule["common"]["egrPacketType"] = 3 -- packet type forward
      ruleMsk["common"]["egrPacketType"] = 0x3
      rule["common"]["fwdData"]["qosProfile"] = params["qosProfile"]
      ruleMsk["common"]["fwdData"]["qosProfile"] = 0xFFFFFFFF
    end
    end

    if params["dscp"]~=nil then
    if direction == "ingress" then -- Ingress dscp
      rule["commonIngrUdb"]["dscpOrExp"]=params["dscp"]
      ruleMsk["commonIngrUdb"]["dscpOrExp"]=0x3F
    elseif direction == "egress" then -- Egress dscp
      rule["commonExt"]["dscp"] = params["dscp"]
      ruleMsk["commonExt"]["dscp"] = 0x3F
    end
    end

    if params["srcPort"]~=nil then
    if direction == "ingress" then -- Ingress srcPort
      rule["commonIngrUdb"]["sourcePort"]=params["srcPort"]
      ruleMsk["commonIngrUdb"]["sourcePort"]=0xFFFFFFFF
    elseif direction == "egress" then -- Egress srcPort
      rule["common"]["sourcePort"] = params["srcPort"]
      ruleMsk["common"]["sourcePort"] = 0xFFFFFFFF
    end
    end


    if direction == "ingress" then -- is rule related to IP (ingress)
    if (params["ipv4-source"] or params["ipv4-destination"] or params["ipv6-destination"] or params["ipv6-source"])~=nil then
      rule["commonIngrUdb"]["isIp"]=1
      ruleMsk["commonIngrUdb"]["isIp"]=0x1
    end
  end

  if direction == "ingress" then
    --ipv6
    if (params["ipv6-source"] or params["ipv6-destination"])~=nil then
      if params["ipv6-source"]~=nil then
        rule=prvAddIpv6Src(rule,params["ipv6-source"]["addr"])
        params["ipv6SrcMsk"]=params["ipv6-source"][1]

        if params["ipv6SrcMsk"]==nil then params["ipv6SrcMsk"]={} params["ipv6SrcMsk"]["addr"]={65535,65535,65535,65535,65535,65535,65535,65535} end
        ruleMsk=prvAddIpv6Src(ruleMsk,params["ipv6SrcMsk"]["addr"])
        prvMaskAddress(rule["sipBits31_0"],ruleMsk["sipBits31_0"])
        prvMaskAddress(rule["sipBits79_32orMacSa"],ruleMsk["sipBits79_32orMacSa"])
        prvMaskAddress(rule["sipBits127_80orMacDa"],ruleMsk["sipBits127_80orMacDa"])
      end


      if params["ipv6-destination"]~=nil then
        rule=prvAddIpv6Dest(rule,params["ipv6-destination"]["addr"])
        params["ipv6DestMsk"]=params["ipv6-destination"][1]

        if params["ipv6DestMsk"]==nil then params["ipv6DestMsk"]={} params["ipv6DestMsk"]["addr"]={65535,65535,65535,65535,65535,65535,65535,65535} end
        ruleMsk=prvAddIpv6Dest(ruleMsk,params["ipv6DestMsk"]["addr"])
        prvMaskAddress(rule["udb0_15"],ruleMsk["udb0_15"])
        rule["commonIngrUdb"]["isUdbValid"]=1
        ruleMsk["commonIngrUdb"]["isUdbValid"]=0x1

      end

      rule["isIpv6"]=1
      ruleMsk["isIpv6"]=0x1
    end
  else -- ipv6 is not supported
    if (params["ipv6-source"] or params["ipv6-destination"])~=nil then
      print("Error: can't support IPv6 in Egress")
      return false,"Error, rule could not be added"
    end
  end

    --ipv4
    if params["ipv4-source"]~=nil then
    params["ipv4SrcMsk"]=params["ipv4-source"][5]
    if params["ipv4SrcMsk"]==nil then  params["ipv4SrcMsk"]={"255","255","255","255"} end
    if direction == "ingress" then
      rule["sipBits31_0"]=prvAddAddr(params["ipv4-source"],10,4)
      ruleMsk["sipBits31_0"]=prvAddAddr(params["ipv4SrcMsk"],10,4)
      prvMaskAddress(rule["sipBits31_0"],ruleMsk["sipBits31_0"])
    elseif direction == "egress" then
      rule["sip"]=prvAddAddr(params["ipv4-source"],10,4)
      ruleMsk["sip"]=prvAddAddr(params["ipv4-source"],10,4)
      prvMaskAddress(rule["sip"],ruleMsk["sip"])
    end
    end

    if params["ipv4-destination"]~=nil then
        params["ipv4DestMsk"]=params["ipv4-destination"][5]
    if params["ipv4DestMsk"]==nil then params["ipv4DestMsk"]= {"255","255","255","255"} end
    if direction == "ingress" then
      rule["dipBits31_0"]=prvAddAddr(params["ipv4-destination"],10,4)
      ruleMsk["dipBits31_0"]=prvAddAddr(params["ipv4DestMsk"],10,4)
      prvMaskAddress(rule["dipBits31_0"],ruleMsk["dipBits31_0"])
    elseif direction == "egress" then
      rule["dip"]=prvAddAddr(params["ipv4-destination"],10,4)
      ruleMsk["dip"]=prvAddAddr(params["ipv4-destination"],10,4)
      prvMaskAddress(rule["dip"],ruleMsk["dip"])
    end

    end

    --Mac
    if params["macSrc"]~=nil then
        params["macSrcMsk"]=params["macSrc"][7]
        if params["macSrcMsk"]==nil then params["macSrcMsk"]={"ff","ff","ff","ff","ff","ff"} end
    if direction == "ingress" then
      rule["sipBits79_32orMacSa"]=prvAddAddr(params["macSrc"],16,6)
      ruleMsk["sipBits79_32orMacSa"]=prvAddAddr(params["macSrcMsk"],16,6)
      prvMaskAddress(rule["sipBits79_32orMacSa"],ruleMsk["sipBits79_32orMacSa"])
    elseif direction == "egress" then
      rule["macSa"]=prvAddAddr(params["macSrc"],16,6)
      ruleMsk["macSa"]=prvAddAddr(params["macSrcMsk"],16,6)
      prvMaskAddress(rule["macSa"],ruleMsk["macSa"])
    end
    end

    if params["macDest"]~=nil then
        params["macDestMsk"]=params["macDest"][7]
        if params["macDestMsk"]==nil then  params["macDestMsk"]={"ff","ff","ff","ff","ff","ff"} end
    if direction == "ingress" then
      rule["sipBits127_80orMacDa"]=prvAddAddr(params["macDest"],16,6)
      ruleMsk["sipBits127_80orMacDa"]=prvAddAddr(params["macDestMsk"],16,6)
      prvMaskAddress(rule["sipBits127_80orMacDa"],ruleMsk["sipBits127_80orMacDa"])
    elseif direction == "egress" then
      rule["macDa"]=prvAddAddr(params["macDest"],16,6)
      ruleMsk["macDa"]=prvAddAddr(params["macDestMsk"],16,6)
      prvMaskAddress(rule["macDa"],ruleMsk["macDa"])
    end
    end

    if (is_sip_5(nil)) then
        if direction == "ingress" then
            if (params["tpidIndex"] or params["tpid1Index"])~=nil then
                -- Use UDB #30 for tag0/1 TPID Index metadata
                local udb30Mask = 0
                local udb30Value = 0
                if params["tpidIndex"]~=nil then
                    udb30Value = params["tpidIndex"]
                    udb30Mask = 0x7
                end
                if params["tpid1Index"]~=nil then
                    udb30Value = udb30Value + (params["tpid1Index"] * 8)
                    udb30Mask = udb30Mask + (0x7 *8)
                end
                rule["udb23_30"] = {}
                ruleMsk["udb23_30"] = {}
                rule["udb23_30"][7]=udb30Value
                ruleMsk["udb23_30"][7]=udb30Mask
            end
        else -- egress
            if (params["tpidIndex"] or params["tpid1Index"])~=nil then
                print("Error: can't support TPID Index in Egress")
                return false,"Error, rule could not be added"
            end
        end
    else -- legacy devices like Ch/Ch2/xCat/xCat2/xCat3/Lion/Lion2
        if (params["tpidIndex"] or params["tpid1Index"])~=nil then
            print("Error: TPID Index classification isn't supported")
            return false,"Error, rule could not be added"
        end
    end

  if direction == "ingress" then
    rule["commonIngrUdb"]["isL2Valid"]=1
    ruleMsk["commonIngrUdb"]["isL2Valid"]=0x1
  end

    if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end

  local patternAndMaskType, ruleFormat
  if direction=="ingress" then
    patternAndMaskType = "CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC"
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E
  else
    patternAndMaskType = "CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC"
    ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E
  end


    for j=1,#devices do

        ret = myGenWrapper("cpssDxChPclRuleSet",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN",TYPE["ENUM"],"ruleFormat",ruleFormat},  --Ext UDB
            {"IN","GT_U32","ruleIndex",params["ruleID"]},   -- index of the rule
            {"IN",TYPE["ENUM"],"ruleOptionsBmp",0}, --Rule is valid
            {"IN",patternAndMaskType,"maskPtr",ruleMsk},  -- The ext UDB rules
            {"IN",patternAndMaskType,"patternPtr",rule},  --The ext UDB mask
            {"IN","CPSS_DXCH_PCL_ACTION_STC","actionPtr",actionSTC} -- The action to be taken (permit/deny)
        })

        if ret~=0 then
            print("cpssDxChPclRuleSet() failed: "..returnCodes[ret])
            return false,"Error, rule could not be added"
        end
    end
    --print("\nRule:"..to_string(rule).."\n\nRule mask:\n"..to_string(ruleMsk).."\n\n")
    --print("Ret:\n"..to_string(ret))
    --print("Action:\n"..to_string(actionSTC))

  if params["counters"] ~= nil then
    for j=1,#devices do
      local blockNumber = getCNCBlock("IPCL_0")
      if blockNumber == -1 then
        return false, "Wrong client name for selected device"
      end
      for i=1, #blockNumber do
        ret = myGenWrapper("cpssDxChCncBlockClientEnableSet",{
          {"IN","GT_U8","devNum",devices[j]},
          {"IN","GT_U32","blockNum",blockNumber[i]},
          {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E},
          {"IN","GT_BOOL","updateEnable",true}
        })
        if ret~=0 then
          print("cpssDxChCncBlockClientEnableSet() failed: "..returnCodes[ret])
          return false,"Error, counter could not be set"
        end
        -- local indexRangesBmp = { l={[0]=1,[1]=0}} -- table to set GT_U64
        ret, val = myGenWrapper("cpssDxChCncBlockClientRangesSet",{
          {"IN","GT_U8","devNum",devices[j]},
          {"IN","GT_U32","blockNum",blockNumber[i]},
          {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E},
          --[[{"IN","GT_U64","indexRangesBmp", indexRangesBmp}--]]
          {"IN","GT_U32","indexRangesBmp_low", 1},
          {"IN","GT_U32","indexRangesBmp_high", 0}
        })
        if ret~=0 then
          print("cpssDxChCncBlockClientRangesSet() failed: "..returnCodes[ret])
          return false,"Error, counter could not be set"
        end

        ret = myGenWrapper("cpssDxChCncCountingEnableSet",{
          {"IN","GT_U8","devNum",devices[j]},
          {"IN","CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT","cncUnit",CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E},
          {"IN","GT_BOOL","enable",true}
        })
        if ret~=0 then
          print("cpssDxChCncCountingEnableSet() failed: "..returnCodes[ret])
          return false,"Error, counter could not be set"
        end
      end

    end

    -- create IPCL_table if not exists
    local IPCL0_table = luaGlobalGet("IPCL_0")
    if IPCL0_table == nil then
      IPCL0_table = {}
    end

    -- store ruleId in table
    IPCL0_table[params["ruleID"]] = true
    globalStore("IPCL_0", IPCL0_table)
    -- globalStore("IPCL_0", params["ruleID"])
  end

    return true
end


-- ************************************************************************
--  prvMaskAddress
--
--  @description masks the address param using the mask param
--
--  @param address - The address to mask
--  @param mask - The mask
--
--
-- ************************************************************************

function prvMaskAddress(address,mask)
    local i=0

    while address[i]~=nil do
        address[i]=bit_and(address[i],mask[i])
        i=i+1
    end
end

-- ************************************************************************
--  prvAddAddr
--
--  @description  help function which sets the struct's Ipv4 and mac address
--
--  @param toAdd - Ipv4 address array of bytes
--  @param base - the base to convert from (16 for mac and 10 for ip)
--
--  @return the struct with the added fields
--
-- ************************************************************************
function prvAddAddr(toAdd,base,num)
    local i
    local ruleStc={}

    for i=1,num do
        ruleStc[i-1] = tonumber(toAdd[i],base)
    end

    return ruleStc
end

-- ************************************************************************
--  prvAddIpv6Src
--
--  @description  sets the struct's source Ipv6 address
--
--  @param  ruleStc - The struct to add the values to (either rule or ruleMsk)
--  @param  toAdd - IPv6 address array of bytes
--
--  @return the struct with the added fields
--
-- ************************************************************************
function prvAddIpv6Src(ruleStc,toAdd)
    local i

    ruleStc["sipBits31_0"]={}
    ruleStc["sipBits79_32orMacSa"]={}
    ruleStc["sipBits127_80orMacDa"]={}

    for i=0,2 do
        if i<2 then
            ruleStc["sipBits31_0"][2*i] = math.floor(toAdd[i+1]/256)
            ruleStc["sipBits31_0"][2*i+1] = toAdd[i+1]%256
        end

        ruleStc["sipBits79_32orMacSa"][2*i] = math.floor(toAdd[i+3]/256)
        ruleStc["sipBits79_32orMacSa"][2*i+1] = toAdd[i+3]%256

        ruleStc["sipBits127_80orMacDa"][2*i] = math.floor(toAdd[i+6]/256)
        ruleStc["sipBits127_80orMacDa"][2*i+1] = toAdd[i+6]%256
    end
    return ruleStc
end

-- ************************************************************************
--  prvAddIpv6Dest
--
--  @description  sets the struct's destination Ipv6 address in the UDB
--
--  @param ruleStc - The struct to add the values to (either rule or ruleMsk)
--  @param toAdd - IPv6 address array of bytes
--
--  @return the struct with the added fields
--
-- ************************************************************************
function prvAddIpv6Dest(ruleStc,toAdd)  --added to the UDB field
    local i
    ruleStc["udb0_15"]={}

    for i=0,7 do
        ruleStc["udb0_15"][2*i]=math.floor(toAdd[i+1]/256)
        ruleStc["udb0_15"][2*i+1]=toAdd[i+1]%256
    end

    return ruleStc
end



CLI_type_dict["pcl_policy_mode"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "PCL policy mode",
  enum = {
    ["meter-count"] = {value="CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E",
            help="metering and counting" },
    ["meter-only"]  = {value="CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E"       ,
             help="metering only" },
    ["count-only"]  = {value="CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E"     ,
            help="counting only" },
   }
}


--------------------------------------------
-- command registration: rule-id
--------------------------------------------

CLI_addCommand("config-acl", "rule-id", {
  func=addPCLRule,
  help = "Create an extended ACL",
  params={


    {type="values", "%ruleID"},-- name="ruleID", help="The rule's ID number" },
    {type="named",
      { format="action %pcl_action_pkt_cmd", name="action" },

      { format="set-vpt %priority", name="alt1" , help = "Set matching pattern's vpt (0-7)"},
      { format="set-dscp %dscp", name="alt1" , help = "Set matching pattern's dscp (0-63)"},
      { format="set-queue %vptQueue", name="alt1" , help = "Set matching pattern's queue (1-8)"},
      { format="set-qos-profile-index %GT_U32", name="alt1" , help = "Set QoS profile index)"},

      { format="ipv6-source %ipv6 %ipv6Mask", name="alt2" , multiplevalues=true, help = "IP version 6 source address followed by mask"},
      { format="ipv6-destination %ipv6 %ipv6Mask", name="alt3", multiplevalues=true, help = "IP version 6 destination address followed by mask" },
      { format="mac-source %mac-address %macMask", name="macSrc", multiplevalues=true, help= "The source mac followed by mask"},
      { format="mac-destination %mac-address %macMask", name="macDest", multiplevalues=true,help= "The destination mac followed by mask" },
      { format="ipv4-source %ipv4 %ipv4Mask", name="alt2" , multiplevalues=true, help = "IP version 4 source address followed by mask"},
      { format="ipv4-destination %ipv4 %ipv4Mask", name="alt3" , multiplevalues=true, help = "IP version 4 destination address followed by mask"},
      { format="dscp %dscp",help = "Differentiated Services Code Point" },
      { format="source-port %portNum",name="srcPort", help = "Source port" },
      { format="isTagged %bool", name="isTagged",help= "Is the packet vlan tagged"},
      { format="vid %vlanIdOrAny", help="The Vlan ID" },
      { format="vid1 %vlanIdOrAny", help="The Vlan1 ID" },
      { format="up %priority", help="The user priority in the range of 0-7" },
      { format="qosProfile %qos_profile" ,help="QOS profile in the range of 0-255"},
      { format="policy %policerID" ,name="policerID" ,help="The ID of the policer to use"},
      { format="policy-mode %pcl_policy_mode" ,name="policy_mode" ,help="Policy mode"},
      { format="modifyVlan0 %vlanId" ,name="modifyVlan0" ,help="set vlan0"},
      { format="modifyVlan1 %vlanId" ,name="modifyVlan1" ,help="set vlan1"},
      { format="counters" ,name="counters" ,help="The option enables centralized counters for the rule"},
      { format="redirect-ethernet %dev_port", name="redirect_ethernet",help="Redirect to output ethernet port"},
      { format="redirect-vidx %vlanIdNoCheck", name="redirect_vidx",help="Redirect to output vidx"},
      { format="redirect-port-channel %trunkID", name="port_channel",help="Redirect to output port-channel"},
      { format="tpid-index %tpid_index", name="tpidIndex",help="The selected TPID index of Tag0"},
      { format="tpid1-index %tpid_index", name="tpid1Index",help="The selected TPID index of Tag1"},
      { format="cpu-code-number %cpu_code_number", name="cpu_code_number",help="Cpu Code Number"},
      { format="match-counter %cheetah1_pcl_match_counter",
          name="match_counter",help="ACL Rule Match Counter 1-32"},

  requirements = {
        ["macDest"] = {"action"},
        ["macSrc"] = {"action"},
        ["alt1"] = {"action"},
        ["alt2"] = {"action"},
        ["alt3"] = {"action"},
        ["srcPort"] = {"action"},
    ["vid"] = {"action"},
    ["vid1"] = {"action"},
        ["isTagged"] = {"action"},
        ["qosProfile"] = {"action"},
        ["policerID"] = {"action"},
        ["policy_mode"] = {"action"},
        ["dscp"] = {"action"},
        ["up"] = {"action"},
        ["policerID"] = {"action"},
        ["modifyVlan0"] = {"action"},
        ["modifyVlan1"] = {"action"},
    ["counters"] = {"action"},
        ["redirect_ethernet"] = {"action"},
        ["redirect_vidx"] = {"action"},
      ["redirect-port-channel"] = {"action"},
        ["tpidIndex"] = {"action"},
        ["tpid1Index"] = {"action"},
        ["cpu_code_number"] = {"action"},
        ["match_counter"] = {"action"},
        ["action"] = {"ruleID"}
        },
        mandatory = {"action"}
    }
  }
})




-- ************************************************************************
--  deleteRuleID
--
--  @description disables a specific rule
--
--  @param params - params["ruleID"]  - The rule id number
--   @usage __global       - __global["g_pclIDDev"]: device number
--
--
--  @return true on success, otherwise false and error message
--
-- ************************************************************************
local function deleteRuleID(params)
    local devNum,devices,j, i
    local ret, val
    local IPCL0_table = luaGlobalGet("IPCL_0")
    local isRuleExists = false

    -- check IPCL0 table. Is ruleID exists?
    if (IPCL0_table ~= nil) and (IPCL0_table[params["ruleID"]]~=nil) then
      isRuleExists = true
      IPCL0_table[params["ruleID"]] = nil
      globalStore("IPCL_0", IPCL0_table)
    end

    local blockNumber = getCNCBlock("IPCL_0")

    devNum=getGlobal("g_pclIDDev")
    if (devNum=="all") then devices=wrLogWrapper("wrlDevList") else devices={devNum} end

    for j=1,#devices do
        ret,val = myGenWrapper("cpssDxChPclRuleInvalidate",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN",TYPE["ENUM"],"ruleSize",1}, --ext rule
            {"IN","GT_U32","ruleIndex",params["ruleID"]}   -- index of the rule
        })

        if (ret~=0) then return false,"Could not delete rule" end

        if blockNumber ~= -1 then
          if isRuleExists then -- check if need to disable CNC on this rule
            for i=1, #blockNumber do
              ret = myGenWrapper("cpssDxChCncBlockClientEnableSet",{ -- disable Cnc block for this client
                  {"IN","GT_U8","devNum",devices[j]},
                  {"IN","GT_U32","blockNum",blockNumber[i]},
                  {"IN","CPSS_DXCH_CNC_CLIENT_ENT","client",CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E},
                  {"IN","GT_BOOL","updateEnable",false}
                })
              if ret~=0 then
                return false,"Could not delete rule"
              end
            end
          end
        end
    end
    return true
end

--------------------------------------------
-- command registration: delete rule-id
--------------------------------------------

CLI_addCommand("config-acl", "delete", {
  func=deleteRuleID,
  help="Delete a specific rule",
   params={
    { type="named",
      { format="rule-id %ruleID", name="ruleID", help="The PCL ID number (0-1023)" },
      mandatory = {"ruleID"}
    }
  }
})




-- ************************************************************************
--  tcamUtilization
--
--  @description prints all the valid TCAM rules in a given range
--
--  @param params - params["from"] - the TCAM entry to start from
--          params["size"] - number of entries to print (if valid)
--
--   @usage __global       - __global["g_pclIDDev"]: device number

--  @return true on success (and prints to screen), otherwise false and error message
--
-- ************************************************************************

local function tcamUtilization(params)
    local j
    local devices
    
    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        -- Combine all clients by TCAM group
        local tcamClients = { "IPCL_0", "IPCL_1", "IPCL_2", "EPCL" }
        local c
        local result = {}
        for c = 1, #tcamClients do
            local ret, tcamGroup, entriesTotal, entriesUsed
            ret, tcamGroup, entriesTotal, entriesUsed =
                wrlCpssDxChTcamUtilizationGet(devices[j],
                "CPSS_DXCH_TCAM_"..tcamClients[c].."_E")
            if ret == 0 then
                if result[tcamGroup] == nil then
                    result[tcamGroup] = {
                            total=entriesTotal,
                            used=entriesUsed,
                            clients={tcamClients[c]}
                        }
                else
                    table.insert(result[tcamGroup].clients, tcamClients[c])
                end
            end
        end
        local r;
        print("\nPolicy TCAM utilization on device "..devices[j]..":")
        for c, r in pairs(result) do
            local s
            s = "   "..table.concat(r.clients,",")..": "
            s = s..to_string(r.used).."/"..to_string(r.total)
            if r.total ~= 0 then
                s = s .. "  ("..to_string(math.floor(r.used*100/r.total)).."%)"
            end
            print(s)
        end

    end

    return true
end


--------------------------------------------
-- command registration: show system policy-tcam utilization
--------------------------------------------
CLI_addCommand("exec", "show system policy-tcam utilization", {
  func=tcamUtilization,
  help="Displays the utilization of the TCAM",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      mandatory = {"devID"}
    }
  }
})
