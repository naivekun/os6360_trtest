--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpssAPI.lua
--*
--* DESCRIPTION:
--* DX related code
--*
--* FILE REVISION NUMBER:
--*       $Revision: 26 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- cpssAPI_desc        - Cpss API name and input parameters
--


--  ****************************************************************************
-- 
-- Start of <<direct call>> section


-- find structure in union by keyValue_________________________________
--
  local function parseUNT(keyValue, untName)
    local tmpMember
    local unionMemberName, unionMemberType
    local stc = cpssAPI_getstruct(untName)
    
    tmpMember = nil
    unionMemberName = nil
    unionMemberType = nil
    repeat
      tmpMember = xml_getchild(stc, tmpMember)
      if tmpMember == nil then
        break
      end
      -- is member key equal to keyValue ?
      local mkey = xml_get_attr(tmpMember, "key")
      if mkey == keyValue then
        unionMemberName = xml_get_attr(tmpMember,"name")
        unionMemberType = xml_get_attr(tmpMember,"type")
        break
      end
    until tmpMember == nil
    return unionMemberName, unionMemberType
  end 




-- direct call of cpssDxChTtiRuleSet ____________________________________________
--

  local function direct_call_cpssDxChTtiRuleSet(params)
    local ret, val, i
    local patternPtr = {}
    local maskPtr = {}
    local actionPtr = {}
    local pattern, mask, action 
    
    local patternStructName, maskStructName, actionStructName
    
    if params.ttiRuleType == "CPSS_DXCH_TTI_RULE_IPV4_E" then
    
      patternStructName = "CPSS_DXCH_TTI_RULE_UNT_ipv4"
      patternPtr.ipv4 = {}
      patternPtr.ipv4 = readStructParam("CPSS_DXCH_TTI_IPV4_RULE_STC", "pattern.ipv4")
    
      maskStructName = "CPSS_DXCH_TTI_RULE_UNT_ipv4"
      maskPtr.ipv4 = {}
      maskPtr.ipv4  = readStructParam("CPSS_DXCH_TTI_IPV4_RULE_STC", "mask.ipv4")
    
    elseif params.ttiRuleType == "CPSS_DXCH_TTI_RULE_MPLS_E" then
    
      patternStructName = "CPSS_DXCH_TTI_RULE_UNT_mpls"
      patternPtr.mpls = {}
      patternPtr.mpls = readStructParam("CPSS_DXCH_TTI_MPLS_RULE_STC", "pattern.mpls")
    
      maskStructName = "CPSS_DXCH_TTI_RULE_UNT_mpls"
      maskPtr.mpls = {}
      maskPtr.mpls  = readStructParam("CPSS_DXCH_TTI_MPLS_RULE_STC", "mask.mpls")
    
    elseif params.ttiRuleType == "CPSS_DXCH_TTI_RULE_ETH_E" then
    
      patternStructName = "CPSS_DXCH_TTI_RULE_UNT_eth"
      patternPtr.eth = {}
      patternPtr.eth = readStructParam("CPSS_DXCH_TTI_ETH_RULE_STC", "pattern.eth")
    
      maskStructName = "CPSS_DXCH_TTI_RULE_UNT_eth"
      maskPtr.eth = {}
      maskPtr.eth  = readStructParam("CPSS_DXCH_TTI_ETH_RULE_STC", "mask.eth")

    elseif params.ttiRuleType == "CPSS_DXCH_TTI_RULE_MIM_E" then
    
      patternStructName = "CPSS_DXCH_TTI_RULE_UNT_mim"
      patternPtr.mim = {}
      patternPtr.mim = readStructParam("CPSS_DXCH_TTI_MIM_RULE_STC", "pattern.mim")
    
      maskStructName = "CPSS_DXCH_TTI_RULE_UNT_mim"
      maskPtr.mim = {}
      maskPtr.mim  = readStructParam("CPSS_DXCH_TTI_MIM_RULE_STC", "mask.mim")

    elseif (params.ttiRuleType == "CPSS_DXCH_TTI_RULE_UDB_10_E") or  
           (params.ttiRuleType == "CPSS_DXCH_TTI_RULE_UDB_20_E") or
           (params.ttiRuleType == "CPSS_DXCH_TTI_RULE_UDB_30_E") then
           
      patternStructName = "CPSS_DXCH_TTI_RULE_UNT_udbArray"
      patternPtr.udbArray = {}
      for i = 0, 29 do
        patternPtr.udbArray[i] = readStructMember("pattern",string.format("%s[%d]","udbArray",i),"GT_U8","int", 0)
        if patternPtr.udbArray[i] == nil then
          printErr("Input error of patternPtr.udbArray")
          return false
        end
      end
    
      maskStructName = "CPSS_DXCH_TTI_RULE_UNT_udbArray"
      maskPtr.udbArray = {}
      for i = 0, 29 do
        maskPtr.udbArray[i] = readStructMember("mask",string.format("%s[%d]","udbArray",i),"GT_U8","int", 0)
        if maskPtr.udbArray[i] == nil then
          printErr("Input error of maskPtr.udbArray")
          return false
        end
       end

    else
      printErr("Wrong tti rule type.")
      return false   
    end

    
    if params.ttiActionType == "CPSS_DXCH_TTI_ACTION_TYPE1_ENT" then
    
      actionStructName = "CPSS_DXCH_TTI_ACTION_UNT_type1"
      actionPtr.type1 = {}
      actionPtr.type1 = readStructParam("CPSS_DXCH_TTI_ACTION_STC", "actionType.type1")
    
    elseif params.ttiActionType == "CPSS_DXCH_TTI_ACTION_TYPE2_ENT" then
    
      actionStructName = "CPSS_DXCH_TTI_ACTION_UNT_type2"
      actionPtr.type2 = {}
      actionPtr.type2 = readStructParam("CPSS_DXCH_TTI_ACTION_2_STC", "actionType.type2")
    
    else
      printErr("Wrong tti action type.")
      return false   
    end

    ret, val = myGenWrapper("cpssDxChTtiRuleSet", {
                    { "IN", "GT_U8",                          "devNum",       params.devID },
                    { "IN", "GT_U32",                         "index",        params.index },
                    { "IN", "CPSS_DXCH_TTI_RULE_TYPE_ENT",    "ruleType",     params.ttiRuleType },
                    { "IN", patternStructName,                "patternPtr",   patternPtr },
                    { "IN", maskStructName,                   "maskPtr",      maskPtr },
                    { "IN", "CPSS_DXCH_TTI_ACTION_TYPE_ENT",  "actionType",   params.ttiActionType },
                    { "IN", actionStructName,                 "actionPtr",    actionPtr }
                })
                
    if (ret ~= 0) then
      printErr("cpssDxChTtiRuleSet() failed: " .. returnCodes[ret])
      return false, "Error in cpssDxChTtiRuleSet()"
    end

    return true
  end

-- CPSS_DXCH_TTI_RULE_TYPE_ENT
  CLI_type_dict["ttiRuleTypeEnum"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "ttiRuleType",
    enum = {
        ["CPSS_DXCH_TTI_RULE_IPV4_E"] =        { value="CPSS_DXCH_TTI_RULE_IPV4_E",     help="CPSS_DXCH_TTI_RULE_IPV4_E" },
        ["CPSS_DXCH_TTI_RULE_MPLS_E"] =        { value="CPSS_DXCH_TTI_RULE_MPLS_E",     help="CPSS_DXCH_TTI_RULE_MPLS_E" },
        ["CPSS_DXCH_TTI_RULE_ETH_E"]  =        { value="CPSS_DXCH_TTI_RULE_ETH_E",      help="CPSS_DXCH_TTI_RULE_ETH_E" },
        ["CPSS_DXCH_TTI_RULE_MIM_E"]  =        { value="CPSS_DXCH_TTI_RULE_MIM_E",      help="CPSS_DXCH_TTI_RULE_MIM_E" },
        ["CPSS_DXCH_TTI_RULE_UDB_10_E"] =      { value="CPSS_DXCH_TTI_RULE_UDB_10_E",   help="CPSS_DXCH_TTI_RULE_UDB_10_E" },
        ["CPSS_DXCH_TTI_RULE_UDB_20_E"] =      { value="CPSS_DXCH_TTI_RULE_UDB_20_E",   help="CPSS_DXCH_TTI_RULE_UDB_20_E" },
        ["CPSS_DXCH_TTI_RULE_UDB_30_E"] =      { value="CPSS_DXCH_TTI_RULE_UDB_30_E",   help="CPSS_DXCH_TTI_RULE_UDB_30_E" }
    }
  } 

-- CPSS_DXCH_TTI_ACTION_TYPE_ENT
  CLI_type_dict["ttiActionTypeEnum"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "ttiActionType",
    enum = {
        ["CPSS_DXCH_TTI_ACTION_TYPE1_ENT"] =        { value="CPSS_DXCH_TTI_ACTION_TYPE1_ENT",     help="CPSS_DXCH_TTI_ACTION_TYPE1_ENT" },
        ["CPSS_DXCH_TTI_ACTION_TYPE2_ENT"] =        { value="CPSS_DXCH_TTI_ACTION_TYPE2_ENT",     help="CPSS_DXCH_TTI_ACTION_TYPE2_ENT" }
    }
  } 

  -- direct call of cpss api cpssDxChTtiRuleSet
  CLI_addCommand("exec", "cpss-api call cpssDxChTtiRuleSet", 
  {
    help = "Direct call of function cpssDxChTtiRuleSet",
    func = direct_call_cpssDxChTtiRuleSet,
    params={{type= "named",
              {format="devNum %devID",                  name="devID",         help="ID of the device"},
              {format="index  %GT_U32",                 name="index",         help="Global index of the rule in the TCAM"},
              {format="ttiRuleType %ttiRuleTypeEnum",   name="ttiRuleType",   help="TTI rule type"},
              {format="actionType %ttiActionTypeEnum",  name="ttiActionType", help="type of the action to use"},
              requirements={ ["index"]={"devID"}, ["ttiRuleType"]={"index"}, ["ttiActionType"]={"ttiRuleType"},},
              mandatory = {"devID", "index", "ttiRuleType", "ttiActionType"}
           }}
  })
  
-- end of cpssDxChTtiRuleSet()__________________________________________________

-- direct call of cpssDxChCncBlockClientRangesSet_______________________________
--
  local function direct_call_cpssDxChCncBlockClientRangesSet(params)
    local ret, val, i

    -- cpss-api call cpssDxChCncBlockClientRangesSet devNum 0 blockNum 0 client CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E

    -- GT_U64 indexRangesBmp.l[0] and indexRangesBmp.l[1]
    local indexRangesBmp0 = readStructMember("indexRangesBmp.l", "0" , "GT_U32", "int", 0 )
    local indexRangesBmp1 = readStructMember("indexRangesBmp.l", "1" , "GT_U32", "int", 0 )

    ret, val = myGenWrapper("cpssDxChCncBlockClientRangesSet", {
                    { "IN", "GT_U8",                         "devNum",              params.devID },
                    { "IN", "GT_U32",                        "blockNum",            params.blockNum },
                    { "IN", "CPSS_DXCH_CNC_CLIENT_ENT",      "client",              params.client },
                    { "IN", "GT_U32",                        "indexRangesBmp_low",  indexRangesBmp0},
                    { "IN", "GT_U32",                        "indexRangesBmp_high", indexRangesBmp1}
                })

    if (ret ~= 0) then
      printErr("cpssDxChCncBlockClientRangesSet() failed: " .. returnCodes[ret])
      return false, "Error in cpssDxChCncBlockClientRangesSet()"
    else
      print("")
      print("result="..returnCodes[ret])
      print("values="..to_string(val))
    end
    
    CLI_dont_add_to_running_config = true
    table.insert(CLI_running_Cfg_Tbl, "call cpss-api cpssDxChCncBlockClientRangesSet devNum " .. tostring(params.devID) ..
                  "  blockNum " .. tostring(params.blockNum) .. "  client " .. tostring(params.client) .. 
                  "  indexRangesBmp.l[0] " .. tostring(indexRangesBmp0) .. "  indexRangesBmp.l[1] " .. tostring(indexRangesBmp1))
    
    return true
  end


  -- CPSS_DXCH_CNC_CLIENT_ENT;
  CLI_type_dict["cncClientEnum"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "CNC clients",
    enum = {


    ["CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E"]                        =  { value="CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E",  help="CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E"]                     =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E"]                   =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E"]                     =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E"]                   =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E" },
    ["CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E"]                    =  { value="CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E",  help="CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E" },
    ["CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E"]                   =  { value="CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E",  help="CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E" },
    ["CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E"]                               =  { value="CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E",  help="CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E" },
    ["CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E"]                         =  { value="CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E",  help="CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E" },
    ["CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E"]                             =  { value="CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E",  help="CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E" },
    ["CPSS_DXCH_CNC_CLIENT_TTI_E"]                                      =  { value="CPSS_DXCH_CNC_CLIENT_TTI_E",  help="CPSS_DXCH_CNC_CLIENT_TTI_E" },

    ["CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E"]                        =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E" },
    ["CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E"]                         =  { value="CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E",  help="CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E" },
    ["CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E"]                           =  { value="CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E",  help="CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E" },
    ["CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E"]                           =  { value="CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E",  help="CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E" },
    ["CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E"]          =  { value="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E",  help="CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E" },
    ["CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E"]                    =  { value="CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E",  help="CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E" },
    ["CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E"]                    =  { value="CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E",  help="CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E" },
    ["CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E"]                    =  { value="CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E",  help="CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E" },
    ["CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E"]                    =  { value="CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E",  help="CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E" },
    ["CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E"]                    =  { value="CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E",  help="CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E" },
    ["CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E"]                             =  { value="CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E",  help="CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E" },

    ["CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E"]                           =  { value="CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E",  help="CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E" },
    ["CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E"]                           =  { value="CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E",  help="CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E" },

    ["CPSS_DXCH_CNC_CLIENT_LAST_E"]                                     =  { value="CPSS_DXCH_CNC_CLIENT_LAST_E",  help="CPSS_DXCH_CNC_CLIENT_LAST_E" }
    }
  }

  -- cpss-api call cpssDxChCncBlockClientRangesSet devNum 0 blockNum 0 client CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E

  -- direct call of cpss api cpssDxChCncBlockClientRangesSet
  CLI_addCommand("exec", "cpss-api call cpssDxChCncBlockClientRangesSet",
  {
    help = "Direct call of function cpssDxChCncBlockClientRangesSet",
    func = direct_call_cpssDxChCncBlockClientRangesSet,
    params={{type= "named",
              {format="devNum %devID",                       name="devID",        help="ID of the device"},
              {format="blockNum %GT_U32",                    name="blockNum",     help="CNC block number"},
              {format="client %cncClientEnum",               name="client",       help="CNC client"},
              requirements={ ["blockNum"]={"devID"}, ["client"]={"blockNum"}},
              mandatory = {"devID", "blockNum", "client"}
           }}
  })

-- end of cpssDxChCncBlockClientRangesSet()__________________________________________________

-- direct call of cpssDxChTrunkTableEntryGet____________________________________
  local function direct_call_cpssDxChTrunkTableEntryGet(params)
    local ret, val
    local devNum = params.devID
    local trunkId = params.trunkId
    
    ret, val = wrlCpssDxChTrunkDevicesAndPortsList(devNum, trunkId , 
                1 --[[get real order in format of CPSS_TRUNK_MEMBERS_AND_NUM_STC ]])

    if (ret ~= 0 or 
        val == nil or 
        val.numMembers == nil or
        val.members == nil) 
    then
        if(ret ~= 0) then
            print("trunkId[".. tostring(trunkId).. "] got error " .. to_string(returnCodes[ret]))
        else
            print("trunkId[".. tostring(trunkId).. "] unknown error ")
        end
    
      -- did not get proper format of info 
      return false
    end

    local numMembers = val.numMembers
    local members = val.members
    local NULL_Port = 62
    local is_NULL_Port = false

    if (numMembers == 0) then
        print("empty trunk (no ports)")
        return true
    end

    for index = 0, numMembers-1 do
        local member = members[index]
        if(member.port == NULL_Port) then 
            is_NULL_Port = true
        end
    end

    if (numMembers == 1 and is_NULL_Port) then
        print("trunkId[".. tostring(trunkId).. "] is empty (only NULL port)")
        return true
    end
    
    print("trunkId[".. tostring(trunkId).. "] , numMembers= " .. tostring(numMembers))
    for index = 0, numMembers-1 do
        local member = members[index]
        print(string.format("[%d]={device=[%d],port=[%d]}",index,member.device,member.port))
    end
    
    return true
  end
  
  
  -- direct call of cpss api cpssDxChTrunkTableEntryGet
  CLI_addCommand("exec", "cpss-api call cpssDxChTrunkTableEntryGet", 
  {
    help = "Direct call of function cpssDxChTrunkTableEntryGet",
    func = direct_call_cpssDxChTrunkTableEntryGet,
    params={{type= "named",
              {format="devNum %devID",              name="devID",       help="The device number"},
              {format="trunkId  %GT_U32",           name="trunkId",     help="Trunk ID"},
              requirements={ ["trunkId"]={"devID"}},
              mandatory = {"devID", "trunkId"}
           }}
  })

-- end of cpssDxChTrunkTableEntryGet()__________________________________________


-- direct call of cpssDxChTrunkTableEntrySet ____________________________________
  local function direct_call_cpssDxChTrunkTableEntrySet(params)
    local i, ret 
    local members = {}
    local maxMembers = 8
    
    if(is_sip_5(params.devID)) then
        -- let the cpss to check it
        maxMembers = 4096
    end
    
    if params.numMembers > maxMembers then
        printErr("Input error of numMembers")
        return false
    end

    if params.numMembers > 0 then
        for i = 0, params.numMembers-1 do
            members[i] = readStructParam("CPSS_TRUNK_MEMBER_STC", string.format("%s[%d]","members",i))
            if members[i] == nil then
              printErr("Input error of membersArray")
              return false
            end     
        end
    end

    --print("members",to_string(members))
    
    local membersArrayAndNum = 
        {numMembers = params.numMembers,
         members = members}
    
    ret = wrlCpssDxChTrunkTableEntrySet(
        params.devID,
        params.trunkId,
        membersArrayAndNum)

    if (ret ~= 0) then
      printErr("cpssDxChTrunkTableEntrySet() failed: " .. returnCodes[ret])
      return false, "Error in cpssDxChTrunkTableEntrySet()"
    end

    return true
  end
  
  
  -- direct call of cpss api cpssDxChTrunkTableEntrySet
  CLI_addCommand("exec", "cpss-api call cpssDxChTrunkTableEntrySet", 
  {
    help = "Direct call of function cpssDxChTrunkTableEntrySet",
    func = direct_call_cpssDxChTrunkTableEntrySet,
    params={{type= "named",
              {format="devNum %devID",              name="devID",       help="The device number"},
              {format="trunkId  %GT_U32",           name="trunkId",     help="Trunk ID"},
              {format="numMembers  %GT_U32",        name="numMembers",     help="Num of enabled members"},
              requirements={ ["trunkId"]={"devID"}, ["numMembers"]={"trunkId"} },
              mandatory = {"devID", "trunkId", "numMembers"}
           }}
  })

-- end of cpssDxChTrunkTableEntrySet()_________________________________________

-- direct call of cpssDxChPclRuleSet____________________________________________
--
  local function direct_call_cpssDxChPclRuleSet(params)
    local ret, val, i
    local maskPtr = {}
    local patternPtr = {}
    local actionPtr = {}
    local pattern, mask, action 
    local tmpPrompt
    local tmpEnum
    local maskStructName, patternStructName, actionStructName

    -- print("DBG:")
    -- print("params.devID = " .. to_string(params.devID))
    -- print("params.ruleFormat = " .. to_string(params.ruleFormat))
    -- print("params.ruleIndex = " .. to_string(params.ruleIndex))
    -- print("params.ruleOptions = " .. to_string(params.ruleOptions))
    
    local untMemberName, untMemberType
    
    local keyValue = params.ruleFormat
    local untName = "CPSS_DXCH_PCL_RULE_FORMAT_UNT"
    
    untMemberName, untMemberType = parseUNT(keyValue, untName)
    
    if ((untMemberName == nil)  or (untMemberType == nil)) then
       printErr("Error: Wrong key value " .. keyValue .. " for union " .. untName)
       return false
    end
    
--    print("DBG: memberName = " .. untMemberName)
--    print("DBG: memberType = " .. untMemberType)
    
    maskStructName = untName .. "_" .. untMemberName
    maskPtr[untMemberName] = {}
    tmpPrompt = "mask." .. untMemberName
    maskPtr[untMemberName] = readStructParam(untMemberType, tmpPrompt)
    
    patternStructName = untName .. "_" .. untMemberName  
    patternPtr[untMemberName] = {}
    tmpPrompt = "pattern." .. untMemberName
    patternPtr[untMemberName] = readStructParam(untMemberType, tmpPrompt)
    
    
    -- start with action struct
    actionPtr = {}
    
    -- read enum      CPSS_PACKET_CMD_ENT                    pktCmd;
    actionPtr["pktCmd"] = readStructMember("action", "pktCmd" , "CPSS_PACKET_CMD_ENT", "enum", "CPSS_PACKET_CMD_FORWARD_E" )
    -- GT_BOOL                                actionStop;
    actionPtr["actionStop"] = readStructMember("action", "actionStop" , "GT_BOOL", "bool", GT_FALSE )
    -- GT_BOOL                                bypassBridge;
    actionPtr["bypassBridge"] = readStructMember("action", "bypassBridge" , "GT_BOOL", "bool", GT_FALSE )
    -- GT_BOOL                                bypassIngressPipe;
    actionPtr["bypassIngressPipe"] = readStructMember("action", "bypassIngressPipe" , "GT_BOOL", "bool", GT_FALSE )
    -- GT_BOOL                                egressPolicy;
    actionPtr["egressPolicy"] = readStructMember("action", "egressPolicy" , "GT_BOOL", "bool", GT_FALSE )
    -- CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC lookupConfig;
    actionPtr["lookupConfig"] = readStructParam("CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC", "action.lookupConfig")
    -- CPSS_DXCH_PCL_ACTION_MIRROR_STC        mirror;
    actionPtr["mirror"] = readStructParam("CPSS_DXCH_PCL_ACTION_MIRROR_STC", "action.mirror")
    -- CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC matchCounter;
    actionPtr["matchCounter"] = readStructParam("CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC", "action.matchCounter")

   -- CPSS_DXCH_PCL_ACTION_QOS_UNT           qos;
    if actionPtr["egressPolicy"] == true then
      actionPtr["qos"] = {}
      actionPtr["qos"]["egress"] = {}
      --  CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT  modifyDscp;
      actionPtr["qos"]["egress"]["modifyDscp"] = readStructMember("action.qos.egress", "modifyDscp" , "CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT", "enum", "CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E" )
      --  GT_U32                                        dscp;
      actionPtr["qos"]["egress"]["dscp"] = readStructMember("action.qos.egress", "dscp" , "GT_U32", "int", 0 )
      --  CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT      modifyUp;
      actionPtr["qos"]["egress"]["modifyUp"] = readStructMember("action.qos.egress", "modifyUp" , "CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT", "enum", "CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E" )
      --  GT_U32                                        up;
      actionPtr["qos"]["egress"]["up"] = readStructMember("action.qos.egress", "up" , "GT_U32", "int", 0 )
      --  GT_BOOL                                       up1ModifyEnable;
      actionPtr["qos"]["egress"]["up1ModifyEnable"] = readStructMember("action.qos.egress", "up1ModifyEnable" , "GT_BOOL", "bool", GT_FALSE )
      --  GT_U32                                        up1;
      actionPtr["qos"]["egress"]["up1"] = readStructMember("action.qos.egress", "up1" , "GT_U32", "int", 0 )
    else
      actionPtr["qos"] = {}
      actionPtr["qos"]["ingress"] = {}
      -- CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT         modifyDscp;
      actionPtr["qos"]["ingress"]["modifyDscp"] = readStructMember("action.qos.ingress", "modifyDscp" , "CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT", "enum", "CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E" )
      -- CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT         modifyUp;
      actionPtr["qos"]["ingress"]["modifyUp"] = readStructMember("action.qos.ingress", "modifyUp" , "CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT", "enum", "CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E" )
      -- GT_U32                                        profileIndex;
      actionPtr["qos"]["ingress"]["profileIndex"] = readStructMember("action.qos.ingress", "profileIndex" , "GT_U32", "int", 0 )
      -- GT_BOOL                                       profileAssignIndex;
      actionPtr["qos"]["ingress"]["profileAssignIndex"] = readStructMember("action.qos.ingress", "profileAssignIndex" , "GT_BOOL", "bool", GT_FALSE )
      -- CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT   profilePrecedence;
      actionPtr["qos"]["ingress"]["profilePrecedence"] = readStructMember("action.qos.ingress", "profilePrecedence" , "CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT", "enum", "CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E" )
      -- CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT      up1Cmd;
      actionPtr["qos"]["ingress"]["up1Cmd"] = readStructMember("action.qos.ingress", "up1Cmd" , "CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT", "enum", "CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E" )
      -- GT_U32                                        up1;
      actionPtr["qos"]["ingress"]["up1"] = readStructMember("action.qos.ingress", "up1" , "GT_U32", "int", 0 )
    end

    -- CPSS_DXCH_PCL_ACTION_REDIRECT_STC      redirect;
    actionPtr["redirect"] = readStructParam("CPSS_DXCH_PCL_ACTION_REDIRECT_STC", "action.redirect")
    -- CPSS_DXCH_PCL_ACTION_POLICER_STC       policer;
    actionPtr["policer"] = readStructParam("CPSS_DXCH_PCL_ACTION_POLICER_STC", "action.policer")

    -- CPSS_DXCH_PCL_ACTION_VLAN_UNT          vlan;
    if actionPtr["egressPolicy"] == true then
      actionPtr["vlan"] = {}
      actionPtr["vlan"]["egress"] = {}
      -- CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT       vlanCmd;
      actionPtr["vlan"]["egress"]["vlanCmd"] = readStructMember("action.vlan.egress", "vlanCmd" , "CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT", "enum", "CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E" )
      -- GT_U32                                         vlanId;
      actionPtr["vlan"]["egress"]["vlanId"] = readStructMember("action.vlan.egress", "vlanId" , "GT_U32", "int", 0 )
      -- GT_BOOL                                        vlanId1ModifyEnable;
      actionPtr["vlan"]["egress"]["vlanId1ModifyEnable"] = readStructMember("action.vlan.egress", "vlanId1ModifyEnable" , "GT_BOOL", "bool", GT_FALSE )
      -- GT_U32                                         vlanId1;
      actionPtr["vlan"]["egress"]["vlanId1"] = readStructMember("action.vlan.egress", "vlanId1" , "GT_U32", "int", 0 )
    else
      actionPtr["vlan"] = {}
      actionPtr["vlan"]["ingress"] = {}
      -- CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT           modifyVlan;
      actionPtr["vlan"]["ingress"]["modifyVlan"] = readStructMember("action.vlan.ingress", "modifyVlan" , "CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT", "enum", "CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E" )
      -- GT_BOOL                                        nestedVlan;
      actionPtr["vlan"]["ingress"]["nestedVlan"] = readStructMember("action.vlan.ingress", "nestedVlan" , "GT_BOOL", "bool", GT_FALSE )
      -- GT_U32                                         vlanId;
      actionPtr["vlan"]["ingress"]["vlanId"] = readStructMember("action.vlan.ingress", "vlanId" , "GT_U32", "int", 0 )
      -- CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT    precedence;
      actionPtr["vlan"]["ingress"]["precedence"] = readStructMember("action.vlan.ingress", "precedence" , "CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT", "enum", "CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E" )
      -- CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT  vlanId1Cmd;
      actionPtr["vlan"]["ingress"]["vlanId1Cmd"] = readStructMember("action.vlan.ingress", "vlanId1Cmd" , "CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT", "enum", "CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E" )
      -- GT_U32                                         vlanId1;
      actionPtr["vlan"]["ingress"]["vlanId1"] = readStructMember("action.vlan.ingress", "vlanId1" , "GT_U32", "int", 0 )
    end
    
    -- CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC   ipUcRoute;
    actionPtr["ipUcRoute"] = readStructParam("CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC", "action.ipUcRoute")
    -- CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC     sourceId;
    actionPtr["sourceId"] = readStructParam("CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC", "action.sourceId")
    -- CPSS_DXCH_PCL_ACTION_OAM_STC           oam;
    actionPtr["oam"] = readStructParam("CPSS_DXCH_PCL_ACTION_OAM_STC", "action.oam")
    -- GT_U32                                 flowId;
    actionPtr["flowId"] = readStructMember("action", "flowId" , "GT_U32", "int", 0 )
    -- CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC   sourcePort;
    actionPtr["sourcePort"] = readStructParam("CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC", "action.sourcePort")
    -- GT_BOOL                                setMacToMe;
    actionPtr["setMacToMe"] = readStructMember("action", "setMacToMe" , "GT_BOOL", "bool", GT_FALSE )
    -- GT_BOOL                                channelTypeToOpcodeMapEnable;
    actionPtr["channelTypeToOpcodeMapEnable"] = readStructMember("action", "channelTypeToOpcodeMapEnable" , "GT_BOOL", "bool", GT_FALSE )
    -- GT_U32                                 tmQueueId;
    actionPtr["tmQueueId"] = readStructMember("action", "tmQueueId" , "GT_U32", "int", 0 )
    -- GT_BOOL                                unknownSaCommandEnable;
    actionPtr["unknownSaCommandEnable"] = readStructMember("action", "unknownSaCommandEnable" , "GT_BOOL", "bool", GT_FALSE )
    -- CPSS_PACKET_CMD_ENT                    unknownSaCommand;
    actionPtr["unknownSaCommand"] = readStructMember("action", "unknownSaCommand" , "CPSS_PACKET_CMD_ENT", "enum", "CPSS_PACKET_CMD_FORWARD_E" )

    --[[
    print("DBG: maskPtr  ===================================================================")
    print(to_string(maskPtr))
    print("DBG: patternPtr===================================================================")
    print(to_string(patternPtr))
    print("DBG: actionPtr====================================================================")
    print(to_string(actionPtr))
    print("==================================================================================")
    ]]--
    ret, val = myGenWrapper("cpssDxChPclRuleSet", {
                    { "IN", "GT_U8",                              "devNum",         params.devID },
                    { "IN", "CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT", "ruleFormat",     params.ruleFormat },
                    { "IN", "GT_U32",                             "ruleIndex",      params.ruleIndex },
                    { "IN", "CPSS_DXCH_PCL_RULE_OPTION_ENT",      "ruleOptionsBmp", params.ruleOptions },
                    { "IN", maskStructName,                       "maskPtr",        maskPtr },
                    { "IN", patternStructName,                    "patternPtr",     patternPtr },
                    { "IN", "CPSS_DXCH_PCL_ACTION_STC",           "actionPtr",      actionPtr }
                })
                
    if (ret ~= 0) then
      printErr("cpssDxChPclRuleSet() failed: " .. returnCodes[ret])
      return false, "Error in cpssDxChPclRuleSet()"
    else 
      print("")
      print("result="..returnCodes[ret])
      print("values="..to_string(val))
    end

    return true
  end


-- CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT
  CLI_type_dict["PclRuleFormatTypeEnum"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "PclRuleFormatType",
    enum = {
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E"]  =                 { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E",                 help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E" },     --/*0*/   ,
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E"]  =              { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E",              help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E" },  --/*1*/   ,
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E"]  =                { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E",                help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E"]  =               { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E",               help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E"]  =               { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E",               help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E"]  =                { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E",                help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E"]  =                { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E",                help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E"]  =                  { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E",                  help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E"]  =               { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E",               help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E"]  =                 { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E",                 help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E"]  =                { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E",                help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E"]  =                 { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E",                 help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E"]  =                 { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E",                 help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E"]  =                    { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E",                    help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E"]  =                    { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E",                    help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E"]  =    { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E",    help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E"]  =     { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E",     help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E"]  =   { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E",   help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E"]  =  { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E",  help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E"]  =          { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E",          help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E"]  =        { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E",        help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E"]  =                     { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E",                     help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E"]  =                     { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E",                     help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E"]  =                     { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E",                     help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E"]  =                     { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E",                     help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E"]  =                     { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E",                     help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E"]  =                     { value="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E",                     help="CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E"]  =                      { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E",                      help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E"]  =                      { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E",                      help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E"]  =                      { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E",                      help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E"]  =                      { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E",                      help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E"]  =                      { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E",                      help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E"]  =                      { value="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E",                      help="CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E" },
    ["CPSS_DXCH_PCL_RULE_FORMAT_LAST_E"]  =                               { value="CPSS_DXCH_PCL_RULE_FORMAT_LAST_E",                               help="CPSS_DXCH_PCL_RULE_FORMAT_LAST_E" }
    }
  }

-- CPSS_DXCH_PCL_RULE_OPTION_ENT
  CLI_type_dict["PclRuleOptionEnum"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "PclRuleOption",
    enum = {
      ["CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E"]  =  { value=0,  help="CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E" }
    }
  }

-- CPSS_PACKET_CMD_ENT;
  CLI_type_dict["packetCmdEnum"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "packet command",
    enum = {
      ["CPSS_PACKET_CMD_FORWARD_E"]           =  { value="CPSS_PACKET_CMD_FORWARD_E",  help="CPSS_PACKET_CMD_FORWARD_E" },
      ["CPSS_PACKET_CMD_MIRROR_TO_CPU_E"]     =  { value="CPSS_PACKET_CMD_MIRROR_TO_CPU_E",  help="CPSS_PACKET_CMD_MIRROR_TO_CPU_E" },
      ["CPSS_PACKET_CMD_TRAP_TO_CPU_E"]       =  { value="CPSS_PACKET_CMD_TRAP_TO_CPU_E",  help="CPSS_PACKET_CMD_TRAP_TO_CPU_E" },
      ["CPSS_PACKET_CMD_DROP_HARD_E"]         =  { value="CPSS_PACKET_CMD_DROP_HARD_E",  help="CPSS_PACKET_CMD_DROP_HARD_E" },
      ["CPSS_PACKET_CMD_DROP_SOFT_E"]         =  { value="CPSS_PACKET_CMD_DROP_SOFT_E",  help="CPSS_PACKET_CMD_DROP_SOFT_E" },
      ["CPSS_PACKET_CMD_ROUTE_E"]             =  { value="CPSS_PACKET_CMD_ROUTE_E",  help="CPSS_PACKET_CMD_ROUTE_E" },
      ["CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E"]  =  { value="CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E",  help="CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E" },
      ["CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E"] =  { value="CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E",  help="CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E" },
      ["CPSS_PACKET_CMD_BRIDGE_E"]            =  { value="CPSS_PACKET_CMD_BRIDGE_E",  help="CPSS_PACKET_CMD_BRIDGE_E" },
      ["CPSS_PACKET_CMD_NONE_E"]              =  { value="CPSS_PACKET_CMD_NONE_E",  help="CPSS_PACKET_CMD_NONE_E" },
      ["CPSS_PACKET_CMD_LOOPBACK_E"]          =  { value="CPSS_PACKET_CMD_LOOPBACK_E",  help="CPSS_PACKET_CMD_LOOPBACK_E" }
    }
  }

  -- cpss-api call cpssDxChPclRuleSet devNum 0 ruleFormat CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E ruleIndex 0 ruleOptionsBmp CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E

  -- direct call of cpss api cpssDxChTtiRuleSet
  CLI_addCommand("exec", "cpss-api call cpssDxChPclRuleSet", 
  {
    help = "Direct call of function cpssDxChPclRuleSet",
    func = direct_call_cpssDxChPclRuleSet,
    params={{type= "named",
              {format="devNum %devID",                       name="devID",        help="ID of the device"},
              {format="ruleFormat %PclRuleFormatTypeEnum",   name="ruleFormat",   help="PCL rule format type"},
              {format="ruleIndex  %GT_U32",                  name="ruleIndex",    help="PCL rule index"},
              {format="ruleOptionsBmp %PclRuleOptionEnum",   name="ruleOptions",  help="PCL rule options bmp"},
              requirements={ ["ruleFormat"]={"devID"}, ["ruleIndex"]={"ruleFormat"}, ["ruleOptions"]={"ruleIndex"},},
              mandatory = {"devID", "ruleFormat", "ruleIndex", "ruleOptions"}
           }}
  })

-- end of cpssDxChPclRuleSet()__________________________________________________

-- direct call of cpssDxChPortModeSpeedSet ____________________________________

    local function direct_call_cpssDxChPortModeSpeedSet(params)
        local ret

      ret = wrlDxChPortModeSpeedSet(
          params.devID,
          params.port,
          params.powerUp,
          params.ifMode,
          params.speed)

      if (ret ~= 0) then
        printErr("cpssDxChPortModeSpeedSet() failed: " .. returnCodes[ret])
        return false, "Error in cpssDxChPortModeSpeedSet()"
      end
      return true
    end

    -- direct call of cpss api cpssDxChPortModeSpeedSet
    CLI_addCommand("exec", "cpss-api call cpssDxChPortModeSpeedSet",
    {
      help = "Direct call of function cpssDxChPortModeSpeedSet",
      func = direct_call_cpssDxChPortModeSpeedSet,
      params={{type= "named",
                {format="devNum %devID",                  name="devID",   help="The device number"},
                {format="portNum %GT_U32",                name="port",   help="The port number"},
                {format="powerUp  %bool",              name="powerUp", help="Serdes power up"},
                {format="ifMode  %port_interface_mode", name="ifMode",  help="Interface mode"},
                {format="speed  %port_speed",          name="speed",   help="Port speed"},
                requirements={ ["port"]={"devID"}, ["powerUp"]={"port"}, ["ifMode"]={"powerUp"}, ["speed"]={"ifMode"} },
                mandatory = {"devID", "port", "powerUp", "ifMode", "speed"}
             }}
    })

  -- end of cpssDxChPortModeSpeedSet()_________________________________________
