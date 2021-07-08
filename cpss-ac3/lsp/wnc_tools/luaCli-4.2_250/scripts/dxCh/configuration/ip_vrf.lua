--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_vrf.lua
--*
--* DESCRIPTION:
--*       adding/deleting virtual routers
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmVirtualRouterAdd") 

-- default route entries info. Are used when Virtual Router creating.
local defaultRoutesInfo = {
   -- CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
   ipv4Uc = {
      lttEntry = {
         routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
         routeEntryBaseIndex = 0,
         ucRPFCheckEnable = false,
         sipSaCheckMismatchEnable = false,
         ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",
		 priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_EXACT_MATCH_E"  
   }},
	ipv4Mc = {
	   routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
	   numOfPaths = 0,
	   routeEntryBaseIndex = 1,
	   ucRPFCheckEnable = false,
	   sipSaCheckMismatchEnable = false,
	   ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",
	   priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_EXACT_MATCH_E"  
	},

   -- CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
   ipv6Uc = {
      lttEntry = {
         routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
         numOfPaths = 0,
         routeEntryBaseIndex = 0,
         ucRPFCheckEnable = false,
         sipSaCheckMismatchEnable = false,
         ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",
		 priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_EXACT_MATCH_E"  

   }},
   ipv6Mc = {
      routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
      numOfPaths = 0,
      routeEntryBaseIndex = 1,
      ucRPFCheckEnable = false,
      sipSaCheckMismatchEnable = false,
      ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",
	  priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_EXACT_MATCH_E"  
   },
   fcoe = {
	   lttEntry = {
		  routeType = "CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E",
		  routeEntryBaseIndex = 0,
		  ucRPFCheckEnable = false,
		  sipSaCheckMismatchEnable = false,
		  ipv6MCGroupScopeLevel = "CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E",
		  priority = "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_EXACT_MATCH_E"  
	}}
}

-- *****************************************************************************
--
--  deleteVrf
--  @description  delete virtual router
--
--  @param params - parameters
--     - params["vrfId"]   - a virtual router Id (1-4095)
--
--  @return       true on success, otherwise false and error message
--
local function deleteVrf(params)
    -- Common variables declaration
    local result, values
    local command_data = Command_Data()

    -- Command specific variables declaration.
    local vrfId = params["vrf_id"]

    command_data:clearLocalStatus()

    local rc = myGenWrapper(
       "cpssDxChIpLpmVirtualRouterDel",
       {
          {"IN", "GT_U32", "lpmDbId", 0},
          {"IN", "GT_U32", "vrId", vrfId}
       }
    )

    if rc ~= 0 then
       command_data:setFailLocalStatus()
       if rc == 0x0B then -- GT_NOT_FOUND
          command_data:addWarning("The virual router %d doesn't exists", vrfId)
       else
          command_data:addError("Error deleting virtual router %d: %s",
                                vrfId, returnCodes[rc])
       end
    end
    command_data:updateStatus()

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end



-- *****************************************************************************
--
--  createVrf
--  @description  create virtual router
--
--  @param params - parameters
--     - params["vrfId"]   - a virtual router Id (1-4095)
--
--  @return       true on success, otherwise false and error message
--
local function createVrf(params)
    -- Common variables declaration
    local result, values
    local command_data = Command_Data()

    -- Command specific variables declaration.
    local vrfId = params["vrf_id"]

    local vrConfig = {
       supportIpv4Uc = true,
       defIpv4UcNextHopInfo   = defaultRoutesInfo.ipv4Uc,
       supportIpv4Mc = true,
       defIpv4McRouteLttEntry = defaultRoutesInfo.ipv4Mc,
       supportIpv6Uc = true,
       defIpv6UcNextHopInfo   = defaultRoutesInfo.ipv6Uc,
       supportIpv6Mc = true,
       defIpv6McRouteLttEntry = defaultRoutesInfo.ipv6Mc,
	   supportFcoe = false,
	   defFcoeNextHopInfo   = defaultRoutesInfo.Fcoe
    }
	print("routeType: ") 
	print(defaultRoutesInfo.ipv4Uc.lttEntry.routeType)
	print("routeType vrfConfig: ")
	print(vrConfig.defIpv4UcNextHopInfo.lttEntry.routeType)
	 
    command_data:clearLocalStatus()
    result, values = wrlCpssDxChIpLpmVirtualRouterAdd(0,vrfId,
													  vrConfig.supportIpv4Uc,defaultRoutesInfo.ipv4Uc.lttEntry,
													  vrConfig.supportIpv4Mc,defaultRoutesInfo.ipv4Mc,
													  vrConfig.supportIpv6Uc,defaultRoutesInfo.ipv6Uc.lttEntry,
													  vrConfig.supportIpv6Mc,defaultRoutesInfo.ipv6Mc,
													  vrConfig.supportFcoe,defaultRoutesInfo.Fcoe) 
    if result == 0x1B  then -- GT_ALREADY_EXIST
       command_data:addWarning("The virual router %d exists already", vrfId)
    elseif result ~= 0 then
       command_data:setFailLocalStatus()
       command_data:addError("Error adding virtual router %d: %s",
                             vrfId, returnCodes[result])
    end
    command_data:updateStatus()

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end



-- parameters of CLI commands "[no] ip vrf"
local ipVrfCommandParams = {
      { type="values", {format="%vrf_id_without_0", name = "vrf_id"}}
}

--------------------------------------------------------------------------------
-- command registration: ip vrf
--------------------------------------------------------------------------------
CLI_addCommand("config", "ip vrf-id", {
  func = function(params)
     return createVrf(params)
  end,
  help   = "Create a Virtual Router",
  params = ipVrfCommandParams
})


--------------------------------------------------------------------------------
-- command registration: no ip vrf
--------------------------------------------------------------------------------
CLI_addCommand("config", "no ip vrf-id", {
  func = function(params)
     return deleteVrf(params)
  end,
  help   = "Delete a Virtual Router",
  params = ipVrfCommandParams
})
