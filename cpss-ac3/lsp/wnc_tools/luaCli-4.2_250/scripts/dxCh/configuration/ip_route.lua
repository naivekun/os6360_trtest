--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_route.lua
--*
--* DESCRIPTION:
--*       creating/destroing of IP Routes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChIpUcRouteEntryValidityCheck")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixExist")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixAdd")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixDel")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixAddEcmpRange")
cmdLuaCLI_registerCfunction("wrlCpssDxChIpLpmIpUcPrefixDelEcmpRange")

--constants

CLI_type_dict["next_hop_routeType"] = {

    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,

    help = "ECMP, QOS or REGULAR",
    enum = {
        ["ECMP"] = { value="ECMP", help="Equal-Cost-MultiPath block,where traffic is split among the route entries according to a hash function" },
        ["QOS"] = { value="QOS", help="Quality-of-Service block, where the route entry is according to the QoSProfile-to-Route-Block-Offset Table" },
        ["REGULAR"] = { value="REGULAR", help="Regular next hop" },
    }
}

-- ************************************************************************
---
--  ip_route_func
--        @description  creates IP Routes
--
--        @param params             - params["ipv4"]: prefix ip v4 address;
--                                    params["ipv4-mask"]: prefix ip v4
--                                    mask;
--                                    params["ipv6"]: prefix ip v6 address;
--                                    params["ipv6-mask"]: prefix ip v6
--                                    mask;
--                                    params["next_hop"]: next-hop entry
--                                    index
--                                    params["vrf_id"]: Virtual Router ID
--                                    params["next_hop_routeType"]: ECMP or REGULAR, optional parameter
--                                    params["num_of_paths"]: The number of ECMP paths, optional parameter
--                                    params["ecmp_ip_entry_index"]:  the index of the entry in the ECMP table, optional parameter
--                                    params["random_enable"]:  Whether to do random selection of next hop, optional parameter
--
--        @return       true on success, otherwise false and error message
--
function ip_route_func(params)
    -- Common variables declaration
    local result, values
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local ip_protocol, ip_address, mask_length, next_hop_enrty_index, vrf_id
    local devNum = devEnv.dev
    local  next_hop_routeType, num_of_paths, ecmp_ip_entry_index, random_enable

    -- Command specific variables initialization.
    ip_protocol             = params["ip_protocol"]
    ip_address              = getFirstDefinedValue(params["ipv4"],
                                                   params["ipv6"])
    mask_length             = getFirstDefinedValue(params["ipv4-mask"],
                                                   params["ipv6-mask"])
    next_hop_enrty_index    = params["next_hop"]
    vrf_id                  = params["vrf_id"] or 0 -- set 0 if nil

    -- check optional params
    if (((params.next_hop_routeType ~= nil)and(params.next_hop_routeType=="ECMP")) and
         (params.num_of_paths == nil or params.ecmp_ip_entry_index == nil)) then
        print("ip route command error in optional params for routeType=ECMP,num_of_paths and ecmp_ip_entry_index can not be nil")
        return nil
    else
        if  (params.next_hop_routeType ~= nil)and(params.next_hop_routeType=="QOS") then
            print("ip route command error in optional params for routeType=QOS not supported")
            return nil
        else
            if ((params.next_hop_routeType == "REGULAR")and
                ((params.num_of_paths ~= nil or params.ecmp_ip_entry_index ~= nil or params.random_enable ~= nil)))then
                print("ip route command error in optional params for routeType=REGULAR - Should not configure ECMP parameters")
                return nil
            end
        end
    end

     -- if optional params are unset
    if (params.next_hop_routeType == nil or params.next_hop_routeType == "REGULAR")then

        -- Next-hot entry initialization setting.
        if true == command_data["status"]                       then
            result, values = wrLogWrapper("wrlCpssDxChIpUcRouteEntryValidityCheck", "(next_hop_enrty_index)", next_hop_enrty_index)
            if     (0 == result) and (false == values) then
                command_data:setFailStatus()
                command_data:addWarning("Next-hop entry %d is not initialized.",
                                        next_hop_enrty_index)
            elseif 0x10 == result then
                command_data:setFailStatus()
                command_data:addWarning("It is not allowed to check %d next-hop " ..
                                        "entry initialization.",
                                        next_hop_enrty_index)
            elseif 0 ~= result then
                command_data:setFailStatus()
                command_data:addError("Error at checking of %d next-hop entry " ..
                                      "initialization.", next_hop_enrty_index)
            end
        end

        -- Ip prefix adding.
        if true == command_data["status"]   then
            local is_sip_5_ = is_sip_5(devNum)
            result, values = wrLogWrapper("wrlCpssDxChIpLpmIpUcPrefixAdd",
                             "(vrf_id, ip_protocol, ip_address, mask_length, next_hop_enrty_index, is_sip_5_)",
                             vrf_id, ip_protocol, ip_address, mask_length, next_hop_enrty_index, is_sip_5_)
            if     0x10 == result then
                command_data:setFailStatus()
                command_data:addWarning("It is not allowed to write ip prefix %s.",
                                        ipPrefixStrGet(ip_protocol, ip_address,
                                                       mask_length))
            elseif -1 == result then
                command_data:setFailStatus()
                command_data:addError("Error at writing of ip prefix %s. "..
                                      " Check a Virtual Router (VRF-ID=%d) exists.",
                                      ipPrefixStrGet(ip_protocol, ip_address,
                                                     mask_length), vrf_id)
            elseif 0 ~= result then
                command_data:setFailStatus()
                command_data:addError("Error at writing of ip prefix %s: %s.",
                                      ipPrefixStrGet(ip_protocol, ip_address,
                                                     mask_length), returnCodes[result])
            end
        end

    else
        -- configuration is for ECMP
        next_hop_routeType = params.next_hop_routeType
        num_of_paths = tonumber(params.num_of_paths)
        ecmp_ip_entry_index = tonumber(params.ecmp_ip_entry_index)
        if(params.random_enable==nil)then
            random_enable=false
        else
            random_enable=params.random_enable
        end

        local result, values = wrLogWrapper(
            "wrlCpssDxChIpLpmIpUcPrefixAddEcmpRange",
            ("(vrf_id, ip_protocol, ip_address, mask_length, "
            .. "ecmp_ip_entry_index, next_hop_enrty_index, num_of_paths, random_enable)"),
            vrf_id, ip_protocol, ip_address, mask_length,
            ecmp_ip_entry_index, next_hop_enrty_index, num_of_paths, random_enable)
        if (result ~= 0) then
            printLog ("Error in wrlCpssDxChIpLpmIpUcPrefixAddEcmpRange")
            print("Error in wrlCpssDxChIpLpmIpUcPrefixAddEcmpRange");
            setFailState()
    end

    end
    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  no_ip_route_func
--        @description  destroes IP Routes
--
--        @param params             - params["ipv4"]: prefix ip v4 address;
--                                    params["ipv4-mask"]: prefix ip v4
--                                    mask;
--                                    params["ipv6"]: prefix ip v6 address;
--                                    params["ipv6-mask"]: prefix ip v6
--                                    mask;
--                                    params["vrf_id"]: Virtual Router ID
--
--        @return       true on success, otherwise false and error message
--
function no_ip_route_func(params)
    -- Common variables declaration
    local result, values, isIpV4PrefixExist, routeType, ipUcRouteEntryIndex
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local ip_protocol, ip_address, mask_length, next_hop_enrty_index, vrf_id

    -- Command specific variables initialization.
    ip_protocol             = params["ip_protocol"]
    ip_address              = getFirstDefinedValue(params["ipv4"],
                                                   params["ipv6"])
    mask_length             = getFirstDefinedValue(params["ipv4-mask"],
                                                   params["ipv6-mask"])
    vrf_id                  = params["vrf_id"] or 0 -- set 0 if nil

     -- Ip v4 prefix existance cheking.
    if true == command_data["status"]                       then
        result, isIpV4PrefixExist, routeType, ipUcRouteEntryIndex =
                                    wrLogWrapper("wrlCpssDxChIpLpmIpUcPrefixExist",
                                      "(vrf_id, ip_protocol, ip_address, mask_length)",
                                       vrf_id, ip_protocol, ip_address, mask_length)

      if  (0 == result) and (false == isIpV4PrefixExist) then
            command_data:setFailStatus()
            command_data:addWarning("Ip prefix %s not exist.",
                                    ipPrefixStrGet(ip_protocol, ip_address,
                                                   mask_length))
        elseif 0x10 == result then
            command_data:setFailStatus()
            command_data:addWarning("It is not allowed to check %s ip " ..
                                    "prefix existance.",
                                    ipPrefixStrGet(ip_protocol,
                                                   ip_address, mask_length))
        elseif 0 ~= result then
            command_data:setFailStatus()
            command_data:addError("Error at checking of %s ip v4 prefix " ..
                                  "existance", ipPrefixStrGet(ip_protocol,
                                                              ip_address,
                                                              mask_length))
        end
    end

    if(routeType=="REGULAR") then
        -- Ip v4 prefix deleting.
        if (true == command_data["status"])and("CPSS_IP_PROTOCOL_IPV4_E" == ip_protocol) then
            result, values =
                myGenWrapper("cpssDxChIpLpmIpv4UcPrefixDel", {
                             { "IN", "GT_U32",      "lpmDBId",      0           },
                             { "IN", "GT_U32",      "vrfId",        vrf_id      },
                             { "IN", "GT_IPADDR",   "ipAddr",       ip_address  },
                             { "IN", "GT_U32",      "prefixLen",    mask_length }})
            if     0x10 == result then
                command_data:setFailStatus()
                command_data:addWarning("It is not allowed to delete ipv4 prefix %s.",
                                        ipPrefixStrGet(ip_protocol, ip_address,
                                                       mask_length))
            elseif -1 == result then
                command_data:setFailStatus()
                command_data:addWarning("Error at deleting of ip prefix %s. "..
                                        " Check a Virtual Router (VRF-ID=%d) exists.",
                                        ipPrefixStrGet(ip_protocol, ip_address,
                                                       mask_length), vrf_id)
            elseif 0 ~= result then
                command_data:setFailStatus()
                command_data:addError("Error at deleting of ipv4 prefix %s: %s.",
                                      ipPrefixStrGet(ip_protocol, ip_address,
                                                     mask_length),
                                      returnCodes[result])
            end
        end

        -- Ip v6 prefix deleting.
        if (true == command_data["status"])and ("CPSS_IP_PROTOCOL_IPV6_E" == ip_protocol)then
            result, values = wrLogWrapper("wrlCpssDxChIpLpmIpUcPrefixDel",
                                          "(vrf_id, ip_protocol, ip_address, mask_length)",
                                             vrf_id, ip_protocol, ip_address, mask_length)
            if     0x10 == result then
                command_data:setFailStatus()
                command_data:addWarning("It is not allowed to delete ipv6 prefix %s.",
                                        ipPrefixStrGet(ip_protocol, ip_address,
                                                       mask_length))
            elseif 0 ~= result then
                command_data:setFailStatus()
                command_data:addError("Error at deleting of ipv6 prefix %s: %s.",
                                      ipPrefixStrGet(ip_protocol, ip_address,
                                                     mask_length), returnCodes[result])
            end
        end
    else

        if (true == command_data["status"])then
            ecmp_ip_entry_index = ipUcRouteEntryIndex
            result, values = wrLogWrapper("wrlCpssDxChIpLpmIpUcPrefixDelEcmpRange",
                                          "(vrf_id, ip_protocol, ip_address, mask_length,ecmp_ip_entry_index)",
                                             vrf_id, ip_protocol, ip_address, mask_length,ecmp_ip_entry_index)
            if     0x10 == result then
                command_data:setFailStatus()
                command_data:addWarning("It is not allowed to delete prefix %s.",
                                        ipPrefixStrGet(ip_protocol, ip_address,
                                                       mask_length))
            elseif 0 ~= result then
                command_data:setFailStatus()
                command_data:addError("Error at deleting of prefix %s: %s.",
                                      ipPrefixStrGet(ip_protocol, ip_address,
                                                     mask_length), returnCodes[result])
            end
        end
    end


    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: ip route
--------------------------------------------------------------------------------
CLI_addCommand("config", "ip route", {
  func   = function(params)
               params.ip_protocol = "CPSS_IP_PROTOCOL_IPV4_E"
               return ip_route_func(params)
           end,
  help   = "Creating of IP Routes",
  params = {
      { type = "values",
          "%ipv4",
          "%ipv4-mask"
      },
      { type="named",
        "#next_hop",
        {format="vrf-id %vrf_id", name="vrf_id", help = "a Virtual Router Id. The default is 0" },
        { format = "next-hop-routeType %next_hop_routeType", name="next_hop_routeType", help = "ECMP or REGULAR, optional parameter"},
        { format = "num-of-paths %num_of_paths", name="num_of_paths", help="The number of ECMP paths, optional parameter"},
        { format = "ecmp-ip-entry-index %ecmp_ip_entry_index", name="ecmp_ip_entry_index", help="the index of the entry in the ECMP table, optional parameter"},
        { format = "random-enable %bool", name="random_enable", help="Whether to do random selection of next hop"},
        requirements={
            ["num_of_paths"]={"next_hop_routeType"},
            ["ecmp_ip_entry_index"]={"num_of_paths"},
            ["random_enable"]={"ecmp_ip_entry_index"},
            mandatory = { "next_hop", "vrf_id" }
       }
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no ip route
--------------------------------------------------------------------------------
CLI_addCommand("config", "no ip route", {
  func   = function(params)
               params.ip_protocol = "CPSS_IP_PROTOCOL_IPV4_E"
               return no_ip_route_func(params)
           end,
  help   = "Destroing of IP Routes",
  params = {
      { type = "values",
          "%ipv4",
          "%ipv4-mask"
      },
      {type = "named",
       {format="vrf-id %vrf_id", name="vrf_id", help = "a Virtual Router Id. The default is 0." },
       mandtory = "vrf_id"
      }
  }
})
