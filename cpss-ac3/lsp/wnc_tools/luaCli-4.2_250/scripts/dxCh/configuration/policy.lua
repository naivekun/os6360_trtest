--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* policy.lua
--*
--* DESCRIPTION:
--*       policy rules related commands
--*
--*    contains the commands:
--*      policy
--*      no policy
--*      show policy
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes


--constants

cmdLuaCLI_registerCfunction("wrlCpssDxChPolicerMeteringEntryEnvelopeSet");
cmdLuaCLI_registerCfunction("wrlCpssDxChPolicerMeteringEntryEnvelopeGet");

function dropPrecedenceQosProfile(devices)
  local ret, val, j, qosProfileIndex, cosPtr

  local numOfProfiles = 72
  local CPSS_DXCH_POLICER_STAGE_INGRESS_0_E = 0
  local CPSS_DP_YELLOW_E = 1
  local CPSS_DP_RED_E = 2

  -- main devices loop
  for j=1,#devices do
    for qosProfileIndex=0, numOfProfiles-1 do -- loop over the QoS profiles

      ret,val = myGenWrapper("cpssDxCh3PolicerQosRemarkingEntrySet",{
        {"IN","GT_U8","devNum",devices[j]},
        {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",CPSS_DXCH_POLICER_STAGE_INGRESS_0_E},   --stage 0 ingress
        {"IN","GT_U32","qosProfileIndex",qosProfileIndex},
        {"IN","GT_U32","greenQosTableRemarkIndex",qosProfileIndex},
        {"IN","GT_U32","yellowQosTableRemarkIndex",qosProfileIndex+numOfProfiles},
        {"IN","GT_U32","redQosTableRemarkIndex",qosProfileIndex+(2*numOfProfiles)}
      })

      if ret ~= 0 then
        return false
      end

    end

    for qosProfileIndex=numOfProfiles, (2*numOfProfiles)-1 do -- yellows
      ret,val = myGenWrapper("cpssDxChCosProfileEntryGet",{
        {"IN","GT_U8","devNum",devices[j]},
        {"IN","GT_U32","profileIndex",qosProfileIndex},
        {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
      })

      if ret ~= 0 then
        return false
      end
      cosPtr = val["cosPtr"]
      cosPtr["dropPrecedence"] = CPSS_DP_YELLOW_E -- change packet color

      ret,val = myGenWrapper("cpssDxChCosProfileEntrySet",{
        {"IN","GT_U8","devNum",devices[j]},
        {"IN","GT_U32","profileIndex",qosProfileIndex},
        {"IN","CPSS_DXCH_COS_PROFILE_STC","cosPtr",cosPtr}
      })

      if ret ~= 0 then
        return false
      end

    end

    for qosProfileIndex=(2*numOfProfiles), (3*numOfProfiles)-1 do -- reds
      ret,val = myGenWrapper("cpssDxChCosProfileEntryGet",{
        {"IN","GT_U8","devNum",devices[j]},
        {"IN","GT_U32","profileIndex",qosProfileIndex},
        {"OUT","CPSS_DXCH_COS_PROFILE_STC","cosPtr"}
      })

      if ret ~= 0 then
        return false
      end
      cosPtr = val["cosPtr"]
      cosPtr["dropPrecedence"] = CPSS_DP_RED_E -- change packet color

      ret,val = myGenWrapper("cpssDxChCosProfileEntrySet",{
        {"IN","GT_U8","devNum",devices[j]},
        {"IN","GT_U32","profileIndex",qosProfileIndex},
        {"IN","CPSS_DXCH_COS_PROFILE_STC","cosPtr",cosPtr}
      })

      if ret ~= 0 then
        return false
      end

    end

  end

  return true
end


-- ************************************************************************
--  setPolicyID
--
--  @description adds a new policy rule
--
--  @param params - params["devID"]        - the device ID number
--          params["policyID"]      - the policy ID number
--          params["action"]      - the action to take, either drop or tansmit
--          params["commitedRate"]    - The commited rate in Kbps
--          params["commitedBurst"]    - Thecommited burst size in bytes
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************



local function setPolicyID(params)
    local policer_stc,ret,val,devices,j

  local CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E = 0
  local CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E = 1

  if (params["peakRateKbps"] ~= nil and params["peakBurstByte"] == nil) then
    print("Could not set policyId, pbs parameter is missing")
    return false
  end

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    policer_stc={
        packetSizeMode = "CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E",
        tunnelTerminationPacketSizeMode = "CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E",
        dsaTagCountingMode = "CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E",
        timeStampCountingMode = "CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E",
        yellowEcnMarkingEnable = "GT_FALSE"
    }

    if params.flagNo~=true then
        policer_stc["policerEnable"]=true
    if (params["peakRateKbps"] ~= nil and params["peakBurstByte"] ~= nil) then -- trTcmParams
      policer_stc["tokenBucketParams"]={
        trTcmParams = {
          cir = params["committedRate"],
          cbs = params["committedBurst"],
          pir = params["peakRateKbps"],
          pbs = params["peakBurstByte"]
        }
      }
      policer_stc["meterMode"]= CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E
    else -- srTcmParams
      policer_stc["tokenBucketParams"]={
        srTcmParams = {
          cir = params["committedRate"],
          cbs = params["committedBurst"],
          ebs = params["committedBurst"]
        }
      }
      policer_stc["meterMode"]= CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E
    end
        policer_stc["modifyDscp"]="CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E"

        if (params["action"]=="transmit") then
            policer_stc["redPcktCmd"]="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E"
        elseif (params["action"]=="precedence") then
      policer_stc["redPcktCmd"]="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E"
      policer_stc["yellowPcktCmd"]="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E"
      if (dropPrecedenceQosProfile(devices)==false) then
        print("Could not set policy id " .. params["policerID"])
        return false
      end
    elseif (params["action"]=="drop") then
            policer_stc["redPcktCmd"]="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E"
        end
    end

    for j=1,#devices do
        ret,val = myGenWrapper("cpssDxChPolicerStageMeterModeSet",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN",TYPE["ENUM"],"stage",0},
            {"IN",TYPE["ENUM"],"mode",1},  --flow mode
        })
        if ret ~= 0 then
            print("Could not set policy id " .. params["policerID"])
            return false
        end

        ret,val = myGenWrapper("cpssDxCh3PolicerMeteringEnableSet",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN",TYPE["ENUM"],"stage",0},
            {"IN","GT_BOOL","enable",true}
        })
        if ret ~= 0 then
            print("Could not set policy id " .. params["policerID"])
            return false
        end

        ret,val = myGenWrapper("cpssDxCh3PolicerMeteringEntrySet",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage","CPSS_DXCH_POLICER_STAGE_INGRESS_0_E"},   --stage 0 ingress
            {"IN","GT_U32","entryIndex",params["policerID"]},
            {"IN","CPSS_DXCH3_POLICER_METERING_ENTRY_STC","entryPtr",policer_stc},
            {"OUT","CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT_srTcmParams","tbParamsPtr"}
        })
        if ret ~= 0 then
            print("Could not set policy id " .. params["policerID"])
            return false
        end
    end

    return true
end

--------------------------------------------
-- command registration: policy-id
--------------------------------------------

CLI_addCommand("config", "policy-id", {
  func=setPolicyID,
  help="Sets a new policy rule",
   params={
    {type="values", "%policerID"},
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="police %commitedRateKbps", name="committedRate", help="The committed rate in Kbps" },
      { format="burst %commitedBurstByte", name="committedBurst", help="The committed burst size in bytes" },
      { format="pir %peakRateKbps", name="peakRateKbps", help="The Peak Information Rate in Kbps" },
      { format="pbs %peakBurstByte", name="peakBurstByte", help="The Peak Burst Size in bytes" },
      { format="exceed-action %exceedAction", name="action", help="Action to take if the packet exceeds the rate" },
      requirements={
        ["devID"] = {"policerID"},
        ["committedRate"] = {"devID"},
        ["committedBurst"] = {"committedRate"},
    ["peakRateKbps"] = {"committedBurst"},
    ["peakBurstByte"] = {"peakRateKbps"},
        ["action"] = {"committedBurst"}
      },
          mandatory = {"action"}
    }
  }
})




--------------------------------------------
-- command registration: no policy
--------------------------------------------

CLI_addCommand("config", "no policy", {
   func=function(params)
      params.flagNo=true
      return setPolicyID(params) end,
  help="Invalidates the specified policer-ID",
   params={
    { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="id %policerID", name="policerID", help="The policy ID number" },
          requirements={["policerID"] = {"devID"}},
        mandatory = {"policerID"}
    }
  }
})



-- ************************************************************************
--  showPolicyID
--
--  @description displays a policy-id
--
--  @param params - params["devID"]        - the device ID number
--          params["policyID"]      - the policy id number
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************

local function showPolicyID(params)
    local ret,val,numPolicyID,temp,action,devices,j,k,startFor,endFor,displayTable
  local skip;


    displayTable=false
    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end


    for j=1,#devices do

        if params["policyID"]~=nil then
            startFor=params["policyID"]
            endFor=params["policyID"]
        else
            ret,val = myGenWrapper("cpssDxChCfgTableNumEntriesGet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN",TYPE["ENUM"],"table",8}, --policer id
                {"OUT","GT_U32","numEntriesPtr"}
            })

            if (ret~=0) then return false,"Cannot find maximum number of policy rules" end

            startFor=0
            endFor=val["numEntriesPtr"]-1
        end


        for k=startFor,endFor do
          ret,val = myGenWrapper("cpssDxCh3PolicerMeteringEntryGet",{
                  {"IN","GT_U8","devNum",devices[j]},
                  {"IN",TYPE["ENUM"],"stage",0},   --stage 0 ingress
                  {"IN","GT_U32","entryIndex",k},
                  {"OUT","CPSS_DXCH3_POLICER_METERING_ENTRY_STC","entryPtr"},
              })

          if (ret==4) then break end

          if ret == 0 then
            temp=val["entryPtr"]["tokenBucketParams"]["srTcmParams"]
            if temp == nil then
              temp=val["entryPtr"]["tokenBucketParams"]["trTcmParams"]
            end
            if val["entryPtr"]["redPcktCmd"]=="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E" then     action="Drop"
            elseif val["entryPtr"]["redPcktCmd"]=="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E" then action="No change"
            else action="Transmit" end


            if temp["cir"]~=0 then
              if displayTable==false then displayTable=true print("\nDev\tPolicy-id\tCommited-rate (kbps)\tCommited burst\tAction\n----\t---------\t---------------------\t--------------\t-------") end
              print(string.format("%2d%13d%22d%20d\t%-10s",devices[j],k,temp["cir"],temp["cbs"],action))
            end
          else
            if ret==7 then
              -- skip "BAD STATE" entries. Envelope Entris are also "BAD STATE"
            else
              print("Could not display policy-id ".. tostring(k)) 
              return false
            end
          end
        end
    end

    if displayTable==false then print("No valid entries were found") end
    return true

end





--------------------------------------------
-- command registration: show policy
--------------------------------------------

CLI_addCommand("exec", "show policy", {
  func=showPolicyID,
  help="Shows a policy rule list",
   params={
     { type="named",
      { format="device %devID_all", name="devID", help="The device number" },
      { format="policy-id %policerID", name="policyID", help="The policy ID number" },
      requirements={["policyID"] = {"devID"}},
      mandatory = {"devID"}
    }
  }
})

--------------------------------------------
-- policy meter entries in CPU memory
--------------------------------------------

CLI_type_dict["policy_meter_defaults"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Policy meter defaults",
    enum = {
        ["ingress"] = { value="ingress", help="Ingress" },
        ["egress"] =  { value="egress", help="Egress" }
   }
}

CLI_type_dict["policer_cmd"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Policy meter packet command",
    enum = {
        ["no-change"]       =
      { value="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E",
        help="No change" },
        ["drop"]            =
      { value="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E",
        help="Drop" },
        ["remark"]          =
      { value="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_E",
        help="Remark by table" },
        ["remark-by-entry"] =
      { value="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_REMARK_BY_ENTRY_E",
        help="Remark by entry" },
   }
}

CLI_type_dict["policy_meter_mng_cnt_set"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Policy meter management conter set",
    enum = {
        ["set0"]    = { value="CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E",     help="Manage Counters set0" },
        ["set1"]    = { value="CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E",     help="Manage Counters set1" },
        ["set2"]    = { value="CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E",     help="Manage Counters set2" },
        ["disable"] = { value="CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E", help="Disable conting" },
   }
}

CLI_type_dict["policy_mng_cnt_set_cfg"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Policy meter management conter set",
    enum = {
        ["0"]    = { value="CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E",     help="Manage Counters set0" },
        ["1"]    = { value="CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E",     help="Manage Counters set1" },
        ["2"]    = { value="CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E",     help="Manage Counters set2" },
   }
}

CLI_type_dict["policy_meter_modify_up"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Policy meter modify UP",
    enum = {
        ["previous"]    = { value="CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E", help="Keep previous" },
        ["disable"]     = { value="CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E",       help="Disable modification" },
        ["enable"]      = { value="CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_E",        help="Modify" },
        ["enable-tag0"] = { value="CPSS_DXCH_POLICER_MODIFY_UP_ENABLE_TAG0_E",   help="Modify Tag0" },
   }
}

CLI_type_dict["policy_meter_modify_dscp"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Policy meter modify DSCP",
    enum = {
        ["previous"]     = { value="CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E", help="Keep previous" },
        ["disable"]      = { value="CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E",       help="Disable modification" },
        ["enable"]       = { value="CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_E",        help="Modify" },
        ["enable-inner"] = { value="CPSS_DXCH_POLICER_MODIFY_DSCP_ENABLE_INNER_E",  help="Modify Inner Tag" },
   }
}

CLI_type_dict["policy_meter_modify"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Policy meter modify",
    enum = {
        ["previous"] = { value="CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E", help="Keep previous" },
        ["disable"]  = { value="CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E",       help="Disable modification" },
        ["enable"]   = { value="CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E",        help="Modify" },
   }
}

CLI_type_dict["policy_meter_remark_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Policy meter remark mode",
    enum = {
        ["l2"] = { value="CPSS_DXCH_POLICER_REMARK_MODE_L2_E", help="Remark by L2 header" },
        ["l3"] = { value="CPSS_DXCH_POLICER_REMARK_MODE_L3_E", help="Remark by L3 header" },
   }
}

CLI_type_dict["policy_meter_conformance_type"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Policy meter conformance type",
    enum = {
        ["sr-tcm"]  = { value="CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E", help="Single Rate TCM" },
        ["tr-tcm"]  = { value="CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E", help="Two Rates TCM" },
        ["mef0"]    = { value="CPSS_DXCH3_POLICER_METER_MODE_MEF0_E", help="MEF0" },
        ["mef1"]    = { value="CPSS_DXCH3_POLICER_METER_MODE_MEF1_E", help="MEF1" },
        ["mef10-3"] = { value="CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E", help="MEF10.3" },
   }
}

CLI_type_dict["policy_meter_color_mode"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Policy meter color mode",
  enum = {
    ["blind"] = { value="CPSS_POLICER_COLOR_BLIND_E", help="Color blind" },
    ["aware"] = { value="CPSS_POLICER_COLOR_AWARE_E", help="Color aware" },
   }
}

CLI_type_dict["policy_meter_byte_or_packet"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Policy meter byte or packet",
  enum = {
    ["byte"] = { value="CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E", help="Bytes" },
    ["packet"] = { value="CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E", help="Packets" },
   }
}

CLI_type_dict["policy_meter_packet_size"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Policy meter packet size",
  enum = {
    ["passenger"] = { value="CPSS_POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E", help="Passenger" },
    ["l3"]        = { value="CPSS_POLICER_PACKET_SIZE_L3_ONLY_E",          help="L3" },
    ["l2"]        = { value="CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E",       help="L2" },
    ["l1"]        = { value="CPSS_POLICER_PACKET_SIZE_L1_INCLUDE_E",       help="L1" },
   }
}

CLI_type_dict["policy_meter_tt_size"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Policy meter tunnel termination size",
  enum = {
    ["passenger"] = { value="CPSS_DXCH3_POLICER_TT_PACKET_SIZE_PASSENGER_E", help="Passenger" },
    ["regular"]   = { value="CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E",   help="Regular" },
   }
}

CLI_type_dict["policy_meter_dsa_mode"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Policy meter DSA Tag mode",
  enum = {
    ["include"] = { value="CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E",    help="Include" },
    ["exclude"] = { value="CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E", help="Exclude" },
   }
}

CLI_type_dict["policy_meter_ts_mode"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Policy meter Time Stamp mode",
  enum = {
    ["include"] = { value="CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E", help="Include" },
    ["exclude"] = { value="CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_EXCLUDE_E", help="Exclude" },
   }
}

CLI_type_dict["policy_meter_max_size"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Policy meter envelope maximal size",
  enum = {
    ["2-entries"] = { value="CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_2_E", help="2 entries" },
    ["4-entries"] = { value="CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_4_E", help="4 entries" },
    ["8-entries"] = { value="CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_8_E", help="8 entries" },
   }
}

CLI_type_dict["policy_meter_envelope_size"] = {
    checker = CLI_check_param_number,
    min=2,
    max=8,
    help = "Policy meter enveope size",
}

CLI_type_dict["policy_billing_count_mode"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Policy billing count mode",
  enum = {
    ["1-byte"]   = { value="CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E",   help="unit is 1 byte" },
    ["16-bytes"] = { value="CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E", help="unit is 16 bytes" },
    ["packet"]   = { value="CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E",   help="unit is packet" },
   }
}

CLI_type_dict["policy_global_counting_mode"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Policy global counting mode",
  enum = {
    ["disable"] = { value="CPSS_DXCH_POLICER_COUNTING_DISABLE_E",       help="Disable mode" },
    ["billing"] = { value="CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E", help="Billing mode" },
    ["policy"]  = { value="CPSS_DXCH_POLICER_COUNTING_POLICY_E",        help="Policy mode" },
    ["vlan"]    = { value="CPSS_DXCH_POLICER_COUNTING_VLAN_E",          help="Vlan mode" },
   }
}

CLI_type_dict["policy_stage_meter_mode"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Policy stage meter mode",
  enum = {
    ["port"] = { value="CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E", help="Port mode" },
    ["flow"] = { value="CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E", help="Flow mode" },
   }
}

local policy_metering_default_ingress =
{
  countingEntryIndex=0,
  mngCounterSet="CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E",
  meterColorMode="CPSS_POLICER_COLOR_BLIND_E",
  meterMode="CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E",
  modifyUp="CPSS_DXCH_POLICER_MODIFY_UP_KEEP_PREVIOUS_E",
  modifyDscp="CPSS_DXCH_POLICER_MODIFY_DSCP_KEEP_PREVIOUS_E",
  modifyDp="CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E",
  modifyExp="CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E",
  modifyTc="CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E",
  greenPcktCmd="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E",
  yellowPcktCmd="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E",
  redPcktCmd="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E",
  qosProfile=0,
  remarkMode="CPSS_DXCH_POLICER_REMARK_MODE_L2_E",
  byteOrPacketCountingMode="CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E",
  packetSizeMode="CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E",
  tunnelTerminationPacketSizeMode="CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E",
  dsaTagCountingMode="CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E",
  timeStampCountingMode="CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E",
  yellowEcnMarkingEnable=false,
    couplingFlag=false,
    maxRateIndex=0,
  tokenBucketParams={}
};

local policy_metering_default_egress =
{
  countingEntryIndex=0,
  mngCounterSet="CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E",
  meterColorMode="CPSS_POLICER_COLOR_BLIND_E",
  meterMode="CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E",
  modifyUp="CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E",
  modifyDscp="CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E",
  modifyDp="CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E",
  modifyExp="CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E",
  modifyTc="CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E",
  greenPcktCmd="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E",
  yellowPcktCmd="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E",
  redPcktCmd="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E",
  qosProfile=0,
  remarkMode="CPSS_DXCH_POLICER_REMARK_MODE_L2_E",
  byteOrPacketCountingMode="CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E",
  packetSizeMode="CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E",
  tunnelTerminationPacketSizeMode="CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E",
  dsaTagCountingMode="CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_INCLUDED_E",
  timeStampCountingMode="CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E",
  yellowEcnMarkingEnable=false,
    couplingFlag=false,
    maxRateIndex=0,
  tokenBucketParams={}
};

-- DB of entries
local policy_metering_memo_entries = {};

local function policy_meter_prepare(params)
  local defaults, range, iterator, index;
  if params.defaults == "ingress" then
    defaults = policy_metering_default_ingress;
  else
    defaults = policy_metering_default_egress;
  end
  range = params.range;
  for iterator, index in number_range_iterator(range) do
    if not policy_metering_memo_entries[index] then
      policy_metering_memo_entries[index] = deepcopy(defaults);
      end
  end
  --switch to "policy-meter" context
  setGlobal("policy-meter-CPU-memory-range", range);
  CLI_prompts["policy-meter"] = "(policy-meter)#";
  CLI_change_mode_push("policy-meter");
end

CLI_addCommand("config", "policy-meter prepare", {
   func = policy_meter_prepare,
   help="prepare policy-meter entries in CPU memory",
   params={
    { type="values",
      { format = "%policy_meter_defaults", name = "defaults", help = "Defaults"},
    { format = "%number_range", name = "range", help = "Entries range"},
        mandatory = {"defaults", "range"}
    }
  }
})

local function policy_meter_remove(params)
  local range, iterator, index;
  range = params.range;
  for iterator, index in number_range_iterator(range) do
    policy_metering_memo_entries[index] = nil;
  end
end

CLI_addCommand("config", "no policy-meter", {
   func = policy_meter_remove,
   help="remove policy-meter entries from CPU memory",
   params={
    { type="values",
    { format = "%number_range", name = "range", help = "Entries range"},
        mandatory = {"range"}
    }
  }
})

local function policy_meter_set(params)
  local devices, ret, val, i;
  local conformance_type, buckets_type;
    if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
    else
    devices={params["devID"]}
    end

  local memo_entry = policy_metering_memo_entries[params.prepared_index];
  if not memo_entry then
    print("No CPU memory with given index");
    return;
    end

  conformance_type = memo_entry.meterMode;
  if (conformance_type == "CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E") then
    buckets_type = "CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT_srTcmParams"
  elseif (conformance_type == "CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E")
      or (conformance_type == "CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E") then
    buckets_type = "CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT_envelope"
  else
    buckets_type = "CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT_trTcmParams"
  end

    for i=1,#devices do
    ret,val = myGenWrapper("cpssDxCh3PolicerMeteringEntrySet",{
      {"IN","GT_U8","devNum",devices[i]},
      {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",params.stage},
      {"IN","GT_U32","entryIndex",params.entry_index},
      {"IN","CPSS_DXCH3_POLICER_METERING_ENTRY_STC","entryPtr",memo_entry},
            {"OUT",buckets_type,"tbParamsPtr"}
      });
    if ret ~= 0 then
      print("Could not set Metering entry");
    end
  end
end

CLI_addCommand("config", "policy-meter set", {
   func = policy_meter_set,
   help="set policy-meter entry from CPU memory to HW",
   params={
    { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
      { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
      { format = "prepared-index %GT_U32", name = "prepared_index", help = "Index in CPU memort"},
      { format = "entry-index %GT_U32", name = "entry_index", help = "Index in HW"},
        mandatory = {"devID", "stage", "prepared_index", "entry_index"},
    requirements={
      ["stage"] = {"devID"},
      ["prepared_index"] = {"stage"},
      ["entry_index"] = {"prepared_index"},
    },
    }
  }
})

local function policy_meter_free_envelope_max_rates(params)
  local devices, j;
  if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
  else
    devices={params["devID"]}
  end
  for j=1,#devices do
    policerDbMaxRateTableEntryEnvelopeFree(
      devices[j], params.stage,
      params.entry_index, params.envelope_size)
  end
end

CLI_addCommand("config", "policy-meter free-envelope-max-rates", {
   func = policy_meter_free_envelope_max_rates,
   help="free policy-meter envelope max rates from CPU memory",
   params={
    { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
      { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
      { format = "entry-index %GT_U32", name = "entry_index", help = "Index in HW"},
      { format = "envelope-size %policy_meter_envelope_size",
      name = "envelope_size", help = "Envelope size"},
        mandatory = {"devID", "stage", "entry_index", "envelope_size"},
    requirements={
      ["stage"] = {"devID"},
      ["entry_index"] = {"stage"},
      ["envelope_size"] = {"entry_index"},
    },
    }
  }
})

local function policer_stage_string_to_num(str)
  local convert = {
    ["CPSS_DXCH_POLICER_STAGE_INGRESS_0_E"] = 0,
    ["CPSS_DXCH_POLICER_STAGE_INGRESS_1_E"] = 1,
    ["CPSS_DXCH_POLICER_STAGE_EGRESS_E"] = 2,
    ["ingress0"] = 0,
    ["ingress1"] = 1,
    ["egress"] = 2,
    [0] = 0, [1] = 1, [2] = 2,
  };
  local s = convert[str];
  if not s then
    print("policer_stage_string_to_num error, received " .. to_string(str));
  end
  return s;
end

local function policy_meter_set_envelope(params)
  local devices, ret, val, j, meterMode;
  local bucketData, tb, maxRateData;

  if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
  else
    devices={params["devID"]}
  end

  local memo_entry_arr = {};
  meterMode = "CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E";
  for j = 0,(params.envelope_size - 1) do
    memo_entry_arr[j] = deepcopy(
      policy_metering_memo_entries[params.prepared_index + j]);
    if not memo_entry_arr[j] then
      print("No CPU memory with given index: " .. tostring(j));
      return;
    end
    memo_entry_arr[j].meterMode = meterMode;
    -- any not first
    meterMode = "CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E";
  end

  -- build bucketData
  bucketData = {};
  for j = 0,(params.envelope_size - 1) do
    if not memo_entry_arr[j].tokenBucketParams
      or not memo_entry_arr[j].tokenBucketParams.envelope then
      print("Missing tokenBucketParams.envelope data: " .. tostring(j));
      return;
    end;
    tb = memo_entry_arr[j].tokenBucketParams.envelope;
    bucketData[j] = {
      bucket0 = {
        max_rate = tb.maxCir,
        rate = tb.cir,
        size = tb.cbs,
      },
      bucket1 = {
        max_rate = tb.maxEir,
        rate = tb.eir,
        size = tb.ebs,
      },
    };
  end

  --check that bucket data is full
  for j = 0,(params.envelope_size - 1) do
    local k0, k1;
    local n0 = {"bucket0", "bucket1"};
    local n1 = {"max_rate", "rate", "size"};
    for k0 = 1,2 do
      for k1 = 1,3 do
        if not bucketData[j][n0[k0]][n1[k1]] then
          print("Missing tokenBucketParams.envelope data");
          return;
        end
      end
    end
  end

  -- calculate and confugure max rates table
  for j=1,#devices do
    maxRateData = policerDbMaxRateTableEntryEnvelopeAlloc(
      devices[j], params.stage,
      params.entry_index, params.coupling_flag0,
      bucketData --[[entry_list--]]);
    if not maxRateData then
      policy_meter_free_envelope_max_rates(params);
      print("Max rate Table allocation failed");
      return;
    end
    ret = policerDbMaxRateTableEntryEnvelopeStoreToHw(
      devices[j], params.stage, maxRateData);
    if not ret then
      policy_meter_free_envelope_max_rates(params);
      print("Max rate Table configuration failed");
      return;
    end
  end

  for j=1,#devices do
  
    local dev_ =  devices[j]
    local policer_stage_ = policer_stage_string_to_num(params.stage) 
    local entry_index_ = params.entry_index
    local envelope_size_ = params.envelope_size 
    local coupling_flag0_ = params.coupling_flag0 
    
    ret,val = wrLogWrapper("wrlCpssDxChPolicerMeteringEntryEnvelopeSet", 
              "(dev_, policer_stage_, entry_index_, envelope_size_, coupling_flag0_, memo_entry_arr)", 
              dev_, policer_stage_, entry_index_, envelope_size_, coupling_flag0_, memo_entry_arr)
    if ret ~= 0 then
      policy_meter_free_envelope_max_rates(params);
      print("Could not set Metering Envelope");
    end
  end
end

CLI_addCommand("config", "policy-meter set-envelope", {
   func = policy_meter_set_envelope,
   help="set policy-meter envelope from CPU memory to HW",
   params={
    { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
      { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
      { format = "prepared-index %GT_U32", name = "prepared_index", help = "Index in CPU memort"},
      { format = "envelope-size %policy_meter_envelope_size",
      name = "envelope_size", help = "Envelope size"},
      { format = "coupling-flag0 %bool", name = "coupling_flag0", help = "Coupling Flag0"},
      { format = "entry-index %GT_U32", name = "entry_index", help = "Index in HW"},
        mandatory = {"devID", "stage", "prepared_index",
      "envelope_size", "coupling_flag0", "entry_index"},
    requirements={
      ["stage"] = {"devID"},
      ["prepared_index"] = {"stage"},
      ["envelope_size"] = {"prepared_index"},
      ["coupling_flag0"] = {"envelope_size"},
      ["entry_index"] = {"coupling_flag0"},
    },
    }
  }
})

local function prv_policy_meter_show_entry(entry)
  -- temporary debug version
  print(to_string(entry));
end

local function policy_meter_show_db(params)
  local range, iterator, index;
  range = params.range;
  for iterator, index in number_range_iterator(range) do
    print("CPU memo entry index: " .. tostring(params.entry_index));
    prv_policy_meter_show_entry(policy_metering_memo_entries[index])
  end
end

CLI_addCommand("exec", "show policy-meter-db", {
   func = policy_meter_show_db,
   help="show policy-meter entry from CPU memory",
   params={
    { type="values",
    { format = "%number_range", name = "range", help = "Entries range"},
    }
  }
})

local function policy_meter_show_entry(params)
  local devices, ret, val, i;
  local memo_entry;
    if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
    else
    devices={params["devID"]}
    end

    for i=1,#devices do
    ret,val = myGenWrapper("cpssDxCh3PolicerMeteringEntryGet",{
      {"IN","GT_U8","devNum",devices[i]},
      {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",params.stage},
      {"IN","GT_U32","entryIndex",params.entry_index},
      {"OUT","CPSS_DXCH3_POLICER_METERING_ENTRY_STC","entryPtr"},
      });
    if ret ~= 0 then
      print("Could not get Metering entry");
    end
    memo_entry = val;

    print("dev: " .. tostring(devices[i]) .. " index " .. tostring(params.entry_index));
    prv_policy_meter_show_entry(memo_entry);
  end
end

CLI_addCommand("exec", "show policy-meter-hw", {
   func = policy_meter_show_entry,
   help="show policy-meter entry from device",
   params={
    { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
      { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
      { format = "entry-index %GT_U32", name = "entry_index", help = "Index in HW"},
        mandatory = {"devID", "stage", "entry_index"},
    requirements={
      ["stage"] = {"devID"},
      ["entry_index"] = {"stage"},
    },
    }
  }
})

-- returns nil on fail or
-- {[index] = {
--       max_rate_index = max_rate_index,
--       rate0 = max_rate0, rate1 = max_rate1}
--       ... }
-- function policerDbMaxRateTableEntryEnvelopeGet(
--     devNum, stage, hw_metring_entry_index, coupling_flag0, envelope_size)
--local memo_entry, coupling_flag0, envelope_size;



local function policy_meter_show_envelope(params)
  local devices, ret, val, i,j;
  local envelope_size, coupling_flag0, memo_entry_arr;
  local max_rate_arr;
  if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
  else
    devices={params["devID"]}
    end

    for i=1,#devices do
        local dev_ =  devices[i]
        local policer_stage_ = policer_stage_string_to_num(params.stage) 
        local entry_index_ = params.entry_index
        local envelope_size_ = 8 
    
    ret, envelope_size, coupling_flag0, memo_entry_arr =
            wrLogWrapper("wrlCpssDxChPolicerMeteringEntryEnvelopeGet", 
            "(dev_, policer_stage_, entry_index_, envelope_size_)", 
            dev_, policer_stage_, entry_index_, envelope_size_)
    if ret ~= 0 then
      print("Could not get Metering Envelope");
      return;
    end

    max_rate_arr = policerDbMaxRateTableEntryEnvelopeGet(
      devices[i], params.stage, params.entry_index, coupling_flag0, envelope_size);
    if not max_rate_arr then
      print("Could not retieve Metering Envelope Maximal rates from DB");
      return;
    end
    for j = 0,(envelope_size - 1) do
      memo_entry_arr[j].max_cir = max_rate_arr[j].rate0;
      memo_entry_arr[j].max_eir = max_rate_arr[j].rate1;
    end

    print("dev: " .. tostring(devices[i]) .. " index " .. tostring(params.entry_index)
        .. " envelope-size " .. tostring(envelope_size)
        .. " coupling-flag0 " .. tostring(coupling_flag0));
    for j = 0,(envelope_size - 1) do
      prv_policy_meter_show_entry(memo_entry_arr[j]);
    end
  end
end

CLI_addCommand("exec", "show policy-meter-envelope-hw", {
   func = policy_meter_show_envelope,
   help="show policy-meter envelope from device",
   params={
    { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
      { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
      { format = "entry-index %GT_U32", name = "entry_index", help = "Index in HW"},
        mandatory = {"devID", "stage", "entry_index"},
    requirements={
      ["stage"] = {"devID"},
      ["entry_index"] = {"stage"},
    },
    }
  }
})

local function policy_meter_edit_command(params)
  local range, iterator, index, entry;
  range = getGlobal("policy-meter-CPU-memory-range");
  for iterator, index in number_range_iterator(range) do
    entry = policy_metering_memo_entries[index];
    if params.green_cmd then
      entry.greenPcktCmd = params.green_cmd;
    end
    if params.yellow_cmd then
      entry.yellowPcktCmd = params.yellow_cmd;
    end
    if params.red_cmd then
      entry.redPcktCmd = params.red_cmd;
    end
  end
end

CLI_addCommand("policy-meter", "command", {
   func = policy_meter_edit_command,
   help="set command in policy-meter entries",
   params={
    { type="named",
    { format="green-cmd %policer_cmd",  name="green_cmd",  help="Green packet command" },
    { format="yellow-cmd %policer_cmd", name="yellow_cmd", help="Yellow packet command" },
    { format="red-cmd %policer_cmd",    name="red_cmd",    help="Red packet command" },
    }
  }
})

local function policy_meter_edit_counting(params)
  local range, iterator, index, entry;
  range = getGlobal("policy-meter-CPU-memory-range");
  for iterator, index in number_range_iterator(range) do
    entry = policy_metering_memo_entries[index];
    if params.counting_index then
      entry.countingEntryIndex = params.counting_index;
    end
    if params.mng_cnt_set then
      entry.mngCounterSet = params.mng_cnt_set;
    end
  end
end

CLI_addCommand("policy-meter", "counting", {
   func = policy_meter_edit_counting,
   help="set counting in policy-meter entries",
   params={
    { type="named",
    { format="counting-index %GT_U32", name="counting_index",
      help="Counting entry index" },
    { format="mng-cnt-set %policy_meter_mng_cnt_set",
      name="mng_cnt_set",  help="Management Counters set" },
    }
  }
})

local function policy_meter_edit_qos(params)
  local range, iterator, index, entry;
  range = getGlobal("policy-meter-CPU-memory-range");
  for iterator, index in number_range_iterator(range) do
    entry = policy_metering_memo_entries[index];
    if params.modify_up then
      entry.modifyUp = params.modify_up;
    end
    if params.modify_dscp then
      entry.modifyDscp = params.modify_dscp;
    end
    if params.modify_dp then
      entry.modifyDp = params.modify_dp;
    end
    if params.modify_exp then
      entry.modifyExp = params.modify_exp;
    end
    if params.modify_tc then
      entry.modifyTc = params.modify_tc;
    end
    if params.qos_profile then
      entry.qosProfile = params.qos_profile;
    end
    if params.remark_mode then
      entry.remarkMode = params.remark_mode;
    end
    if params.yellow_ecn_mark then
      entry.yellowEcnMarkingEnable = params.yellow_ecn_mark;
    end
  end
end

CLI_addCommand("policy-meter", "qos", {
   func = policy_meter_edit_qos,
   help="set qos in policy-meter entries",
   params={
    { type="named",
    { format="modify-up %policy_meter_modify_up",     name="modify_up",   help="Modify UP" },
    { format="modify-dscp %policy_meter_modify_dscp", name="modify_dscp", help="Modify DSCP" },
    { format="modify-dp %policy_meter_modify",        name="modify_dp",   help="Modify DP" },
    { format="modify-exp %policy_meter_modify",       name="modify_exp",  help="Modify EXP" },
    { format="modify-tc %policy_meter_modify",        name="modify_tc",   help="Modify TC" },
    { format="qos-profile %GT_U32",                   name="qos_profile", help="Qos Profile Index" },
    { format="remark-mode %policy_meter_remark_mode", name="remark_mode", help="Remark Mode" },
    { format="yellow-ecn-mark %bool",                 name="yellow_ecn_mark", help="Remark ECN Field" },
    }
  }
})

local function policy_meter_edit_conformance(params)
  local range, iterator, index, entry, tbHeader, tb;
  range = getGlobal("policy-meter-CPU-memory-range");
  for iterator, index in number_range_iterator(range) do
    entry = policy_metering_memo_entries[index];
    -- mandatory conformance_type
    entry.meterMode = params.conformance_type;
    if params.color_mode then
      entry.meterColorMode = params.color_mode;
    end
    if not entry.tokenBucketParams then
      entry.tokenBucketParams = {};
    end
    tbHeader = entry.tokenBucketParams;
    if params.conformance_type == "CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E" then
      -- srTcmParams (sr-tcm)
      if not tbHeader.srTcmParams then
        tbHeader.srTcmParams = {};
      end
      tb = tbHeader.srTcmParams;
      if params.pbs then
        tb.ebs = params.pbs;
      end
    elseif params.conformance_type
        == "CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E" then
      -- envelope (MEF10.3))
      if not tbHeader.envelope then
        tbHeader.envelope = {};
      end
      tb = tbHeader.envelope;
      if params.eir then
        tb.eir = params.eir;
      end
      if params.ebs then
        tb.ebs = params.ebs;
      end
      if params.max_cir then
        tb.maxCir = params.max_cir;
      end
      if params.max_eir then
        tb.maxEir = params.max_eir;
      end
      if params.coupling_flag then
        entry.couplingFlag = params.coupling_flag;
      end
    else
      -- trTcmParams (tr-tcm, mef0, mef1)
      if not tbHeader.trTcmParams then
        tbHeader.trTcmParams = {};
      end
      tb = tbHeader.trTcmParams;
      if params.pir then
        tb.pir = params.pir;
      end
      if params.pbs then
        tb.pbs = params.pbs;
      end
    end
    -- common for all conformance types
    if params.cir then
      tb.cir = params.cir;
    end
    if params.cbs then
      tb.cbs = params.cbs;
    end
  end
end

CLI_addCommand("policy-meter", "conformance", {
   func = policy_meter_edit_conformance,
   help="set conformance in policy-meter entries",
   params={
  {type="values",
    {format="%policy_meter_conformance_type", name = "conformance_type"}
  },
  { type="named",
    { format="meter-color-mode %policy_meter_color_mode",  name="color_mode",   help="Color mode" },
    { format="cir %GT_U32", name="cir", help="CIR" },
    { format="cbs %GT_U32", name="cbs", help="CBS" },
    { format="pir %GT_U32", name="pir", help="PIR" },
    { format="pbs %GT_U32", name="pbs", help="PBS" },
    { format="eir %GT_U32", name="eir", help="Mef10.3 EIR" },
    { format="ebs %GT_U32", name="ebs", help="Mef10.3 EBS" },
    { format="coupling-flag %bool", name="coupling_flag", help="Coupling flag" },
    { format="max-cir %GT_U32", name="max_cir", help="Mef10.3 Max CIR" },
    { format="max-eir %GT_U32", name="max_eir", help="Mef10.3 Max EIR" },
  }
  }
})

local function policy_meter_edit_byte_counting(params)
  local range, iterator, index, entry, tb;
  range = getGlobal("policy-meter-CPU-memory-range");
  for iterator, index in number_range_iterator(range) do
    entry = policy_metering_memo_entries[index];
    if params.byte_or_packet then
      entry.byteOrPacketCountingMode = params.byte_or_packet;
    end
    if params.packet_size then
      entry.packetSizeMode = params.packet_size;
    end
    if params.tt_size then
      entry.tunnelTerminationPacketSizeMode = params.tt_size;
    end
    if params.dsa_mode then
      entry.dsaTagCountingMode = params.dsa_mode;
    end
    if params.ts_mode then
      entry.timeStampCountingMode = params.ts_mode;
    end
  end
end

CLI_addCommand("policy-meter", "byte-counting", {
   func = policy_meter_edit_byte_counting,
   help="set byte counting in policy-meter entries",
   params={
  { type="named",
    { format="byte-or-packet %policy_meter_byte_or_packet",  name="byte_or_packet",
      help="Byte or Packet" },
    { format="packet-size %policy_meter_packet_size", name="packet_size", help="Packet size" },
    { format="tunnel-termination-size %policy_meter_tt_size",
      name="tt_size", help="Tunnel Termination size" },
    { format="dsa-tag %policy_meter_dsa_mode", name="dsa_mode", help="DSA Tag mode" },
    { format="time-stamp %policy_meter_ts_mode", name="ts_mode", help="Time Stamp mode" },
  }
  }
})

local function policy_meter_flow_based_cfg(params)
  local devices, ret, val, i;
  if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
  else
    devices={params["devID"]}
  end

  for i=1,#devices do
    ret,val = myGenWrapper("cpssDxChPolicerMeterTableFlowBasedIndexConfigSet",{
      {"IN","GT_U8","devNum",devices[i]},
      {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",params.stage},
      {"IN","GT_U32","threshold",params.threshold},
      {"IN","CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT","maxSize",params.max_size},
    });
    if ret ~= 0 then
      print("Could not set Flow Based Access Configuration");
    end
  end
end

CLI_addCommand("config", "policy-meter flow-based-cfg", {
   func = policy_meter_flow_based_cfg,
   help="set policy-meter flow based access configuration",
   params={
  { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
    { format = "threshold %GT_U32", name = "threshold", help = "Policer pointer threshold"},
    { format = "max-size %policy_meter_max_size", name = "max_size", help = "Maximal Envelope size"},
    mandatory = {"devID", "stage", "threshold", "max_size"},
    requirements={
      ["stage"] = {"devID"},
      ["threshold"] = {"stage"},
      ["max_size"] = {"threshold"},
    },
  }
  }
})

local function policy_meter_flow_based_cfg_show(params)
  local devices, ret, val, i;
  if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
  else
    devices={params["devID"]}
  end

  for i=1,#devices do
    ret,val = myGenWrapper("cpssDxChPolicerMeterTableFlowBasedIndexConfigGet",{
      {"IN", "GT_U8","devNum",devices[i]},
      {"IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",params.stage},
      {"OUT","GT_U32","thresholdPtr",},
      {"OUT","CPSS_DXCH_POLICER_ENVELOPE_MAX_SIZE_ENT","maxSizePtr"},
    });
    if ret ~= 0 then
      print("Could not get Flow Based Access Configuration");
    end
    print(
      "dev " .. tostring(devices[i]) ..
      " threshold " .. tostring(val.thresholdPtr) ..
      " max_size " .. val.maxSizePtr);
  end
end

CLI_addCommand("exec", "show policy-meter flow-based-cfg", {
   func = policy_meter_flow_based_cfg_show,
   help="show policy-meter flow based access configuration",
   params={
  { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
    mandatory = {"devID", "stage"},
    requirements={
      ["stage"] = {"devID"},
    },
  }
  }
})

local function policy_meter_qos_profile_to_priority_map(params)
  local devices, ret, val, i, range, iterator, index;
    if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
    else
    devices={params["devID"]}
    end

  range = params.qos_profile_range;
    for i=1,#devices do
    for iterator, index in number_range_iterator(range) do
      ret,val = myGenWrapper("cpssDxChPolicerQosProfileToPriorityMapSet",{
        {"IN","GT_U8","devNum",devices[i]},
        {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",params.stage},
        {"IN","GT_U32","qosProfileIndex",index},
        {"IN","GT_U32","priority",params.priority},
      });
      if ret ~= 0 then
        print("Could not set Qos Profile to Priority Map");
      end
    end
  end
end

CLI_addCommand("config", "policy-meter qos-profile-to-priority", {
   func = policy_meter_qos_profile_to_priority_map,
   help="set policy-meter map QOS profiles range to priority",
   params={
  { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
    { format = "qos-profile-index %number_range", name = "qos_profile_range",
      help = "QOS profile index range"},
    { format = "priority %GT_U32", name = "priority", help = "Priority"},
    mandatory = {"devID", "stage", "qos_profile_range", "priority"},
    requirements={
      ["stage"] = {"devID"},
      ["qos_profile_range"] = {"stage"},
      ["priority"] = {"qos_profile_range"},
    },
  }
  }
})

local function policy_meter_qos_profile_to_priority_map_show(params)
  local devices, ret, val, i, range, iterator, index;
    if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
    else
    devices={params["devID"]}
    end

  range = params.qos_profile_range;
    for i=1,#devices do
    for iterator, index in number_range_iterator(range) do
      ret,val = myGenWrapper("cpssDxChPolicerQosProfileToPriorityMapGet",{
        {"IN","GT_U8","devNum",devices[i]},
        {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",params.stage},
        {"IN","GT_U32","qosProfileIndex",index},
        {"OUT","GT_U32","priorityPtr"},
      });
      if ret ~= 0 then
        print("Could not get Qos Profile to Priority Map");
      end
      print(
        "dev " .. tostring(devices[i]) ..
        " qos-profile-index " .. tostring(index) ..
        " max_size " .. val.priorityPtr);
    end
  end
end

CLI_addCommand("exec", "show policy-meter qos-profile-to-priority", {
   func = policy_meter_qos_profile_to_priority_map_show,
   help="show policy-meter map QOS profiles range to priority",
   params={
  { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
    { format = "qos-profile-index %number_range", name = "qos_profile_range",
      help = "QOS profile index range"},
    mandatory = {"devID", "stage", "qos_profile_range"},
    requirements={
      ["stage"] = {"devID"},
      ["qos_profile_range"] = {"stage"},
    },
  }
  }
})

local function policy_meter_set_mru(params)
  local devices, did_error, ret, val, i;
    if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
    else
    devices={params["devID"]}
    end
  ret = true;
    for i=1,#devices do
    did_error = generic_policer_mru_set(devices[i], params.stage, params.mru);
    ret = ret and (not did_error);
  end
  return ret;
end

CLI_addCommand("config", "policy-meter set-mru", {
   func = policy_meter_set_mru,
   help="set policy-meter MRU",
   params={
  { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
    { format = "mru %GT_U32", name = "mru", help = "policy-meter MRU"},
    mandatory = {"devID", "stage", "mru"},
    requirements={
      ["stage"] = {"devID"},
      ["mru"] = {"stage"},
    },
  }
  }
})

local function policy_meter_show_mru(params)
  local devices, did_error, mru, ret, val, i;
    if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
    else
    devices={params["devID"]}
    end
  ret = true;
    for i=1,#devices do
    did_error, mru = generic_policer_mru_get(devices[i], params.stage);
    ret = ret and (not did_error);
    if not did_error then
      print("dev " .. tostring(devices[i]) .. " MRU " .. tostring(mru));
    end
  end
  return ret;
end

CLI_addCommand("exec", "show policy-meter mru", {
   func = policy_meter_show_mru,
   help="show policy-meter MRU",
   params={
  { type="named",
    { format="device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
    mandatory = {"devID", "stage"},
    requirements={
      ["stage"] = {"devID"},
    },
  }
  }
})

local function policy_billing_reset(params)
    local devices,j, entry;
    local did_error, result, OUT_values, billingCntr;
    local command_data = Command_Data();
  local configure_required, iterator, index;

    if (params["devID"]=="all") then
        devices=wrLogWrapper("wrlDevList")
    else
        devices={params["devID"]};
    end

  configure_required = false;
  if params.count_mode then
    configure_required = true;
  end
  if params.all_as_green then
    configure_required = true;
  end
  if params.packet_size then
    configure_required = true;
  end
  if params.tt_size then
    configure_required = true;
  end
  if params.dsa_mode then
    configure_required = true;
  end
  if params.ts_mode then
    configure_required = true;
  end
    for j=1,#devices do
    for iterator, index in number_range_iterator(params.entry_index) do
      did_error, result, billingCntr = policy_billing_get(
        command_data, devices[j], params.stage,
        index, true --[[reset--]]);
      if did_error then
        -- ignore GT_BAD_STATE. GT_BAD_STATE means that entry was not set yet.
        -- The code below will set it first time.
        if result ~= 7 then
           return false;
        end
      end

      if configure_required then
        entry = deepcopy(billingCntr);
        if did_error then
          -- fix packet size mode to be like 0 in HW
          entry.packetSizeMode = "CPSS_POLICER_PACKET_SIZE_L3_ONLY_E"
          end
        entry.greenCntr  = {l = {[0] = 0, [1] = 0}};
        entry.yellowCntr = {l = {[0] = 0, [1] = 0}};
        entry.redCntr    = {l = {[0] = 0, [1] = 0}};
        if params.count_mode then
          entry.billingCntrMode = params.count_mode;
        end
        if params.all_as_green then
          entry.billingCntrAllEnable = params.all_as_green;
        end
        if params.packet_size then
          entry.packetSizeMode = params.packet_size;
        end
        if params.tt_size then
          entry.tunnelTerminationPacketSizeMode = params.tt_size;
        end
        if params.dsa_mode then
          entry.dsaTagCountingMode = params.dsa_mode;
        end
        if params.ts_mode then
          entry.timeStampCountingMode = params.ts_mode;
        end

        -- always flush cache before Set entry
        did_error, result, OUT_values =
          genericCpssApiWithErrorHandler(
            command_data, "cpssDxChPolicerCountingWriteBackCacheFlush",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage", params.stage},
          });
        if did_error then
          return false;
        end

        did_error, result, OUT_values =
          genericCpssApiWithErrorHandler(
            command_data, "cpssDxCh3PolicerBillingEntrySet",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage", params.stage},
            {"IN","GT_U32","entryIndex",index},
            {"IN","CPSS_DXCH3_POLICER_BILLING_ENTRY_STC","entry", entry}
          });
        if did_error then
          return false;
        end
      end
    end
    end
    return true;
end

CLI_addCommand("config", "policy-billing reset", {
   func = policy_billing_reset,
   help="reset and configure policy-billing entry",
   params={
  { type="named",
    { format= "device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
      { format = "entry-index %number_range", name = "entry_index", help = "range of indexes in HW"},
    { format = "all-as-green %bool", name = "all_as_green", help = "Count all packets as green"},
    { format = "count-mode %policy_billing_count_mode", name = "count_mode", help = "Counting mode"},
    { format = "dsa-tag %policy_meter_dsa_mode", name="dsa_mode", help="DSA Tag mode" },
    { format = "packet-size %policy_meter_packet_size", name="packet_size", help="Packet size" },
    { format = "time-stamp %policy_meter_ts_mode", name="ts_mode", help="Time Stamp mode" },
    { format = "tunnel-termination-size %policy_meter_tt_size",
      name = "tt_size", help="Tunnel Termination size" },
    mandatory = {"devID", "stage", "entry_index"},
    requirements={
      ["stage"] = {"devID"},
      ["entry_index"] = {"stage"},
    },
  }
  }
})

-- global function to use also in tests for checking
-- returns did_error, result
function policy_billing_get(command_data, dev_num, stage, index, reset)
  local did_error, result, OUT_values =
    genericCpssApiWithErrorHandler(
      command_data, "cpssDxCh3PolicerBillingEntryGet",{
      {"IN", "GT_U8", "devNum", dev_num},
      {"IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", stage},
      {"IN", "GT_U32", "entryIndex", index},
      {"IN", "GT_BOOL", "reset", reset},
      {"OUT","CPSS_DXCH3_POLICER_BILLING_ENTRY_STC", "billingCntrPtr"}
    });
  return did_error, result, OUT_values.billingCntrPtr;
end

local function policy_billing_show(params)
    local devices,j, iterator, index;
    local did_error, result, billingCntr;
    local command_data = Command_Data();

    if (params["devID"]=="all") then
        devices=wrLogWrapper("wrlDevList")
    else
        devices={params["devID"]};
    end

    for j=1,#devices do
    for iterator, index in number_range_iterator(params.entry_index) do
      did_error, result, billingCntr = policy_billing_get(
          command_data, devices[j], params.stage,
          index, false --[[reset--]]);
      if did_error then
        return false;
      end

      print(
        "dev "     .. tostring(devices[j]) ..
        " stage "  .. tostring(params.stage) ..
        " index "  .. tostring(index) ..
        " low,high: " ..
        " green "  .. tostring(billingCntr.greenCntr.l[0])  ..
        " " .. tostring(billingCntr.greenCntr.l[1]) ..
        " yellow " .. tostring(billingCntr.yellowCntr.l[0]) ..
        " " .. tostring(billingCntr.yellowCntr.l[1]) ..
        " red "    .. tostring(billingCntr.redCntr.l[0])    ..
        " " .. tostring(billingCntr.redCntr.l[1]));
    end
  end
    return true;
end

CLI_addCommand("exec", "show policy-billing", {
   func = policy_billing_show,
   help="show policy-billing entry",
   params={
  { type="named",
    { format= "device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
      { format = "entry-index %number_range", name = "entry_index", help = "range of indexes in HW"},
    mandatory = {"devID", "stage", "entry_index"},
    requirements={
      ["stage"] = {"devID"},
      ["entry_index"] = {"stage"},
    },
  }
  }
})

local function policy_mng_counters_reset(params)
  local devices, j, k, s_type, entry;
  local did_error, result, OUT_values;
  local command_data = Command_Data();

  if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
  else
    devices={params["devID"]};
  end

  local set_types = {
    "CPSS_DXCH3_POLICER_MNG_CNTR_GREEN_E",
    "CPSS_DXCH3_POLICER_MNG_CNTR_YELLOW_E",
    "CPSS_DXCH3_POLICER_MNG_CNTR_RED_E",
    "CPSS_DXCH3_POLICER_MNG_CNTR_DROP_E",
  };

  entry = {duMngCntr = {l = {[0]=0,[1]=0}}, packetMngCntr = 0};
  for j=1,#devices do
    for k,s_type in pairs(set_types) do
      did_error, result, OUT_values =
        genericCpssApiWithErrorHandler(
          command_data, "cpssDxCh3PolicerManagementCountersSet",{
          {"IN", "GT_U8", "devNum", devices[j]},
          {"IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", params.stage},
          {"IN", "CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT", "mngCntrSet", params.set},
          {"IN", "CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT", "mngCntrType", s_type},
          {"IN", "CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC", "mngCntrPtr", entry},
        });
      if did_error then
        return false;
      end
    end
  end
end

CLI_addCommand("config", "policy-mng-conters reset", {
   func = policy_mng_counters_reset,
   help="reset policy-management counter set",
   params={
  { type="named",
    { format= "device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
    { format = "set %policy_mng_cnt_set_cfg", name = "set", help = "Counter set"},
    mandatory = {"devID", "stage", "set"},
    requirements={
      ["stage"] = {"devID"},
      ["set"] = {"stage"},
    },
  }
  }
})

-- global function to use also in tests for checking
-- returns did_error, result - all 4 colors couners
function policy_mng_counters_get(command_data, dev_num, stage, set)
  local k, s_type, res;
  local did_error, result, OUT_values;
  local set_ids = {
    [0] = "CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E",
    [1] = "CPSS_DXCH3_POLICER_MNG_CNTR_SET1_E",
    [2] = "CPSS_DXCH3_POLICER_MNG_CNTR_SET2_E",
  };
  if set_ids[set] then
    set = set_ids[set];
  end
  local set_types = {
    {"CPSS_DXCH3_POLICER_MNG_CNTR_GREEN_E",  "green"},
    {"CPSS_DXCH3_POLICER_MNG_CNTR_YELLOW_E", "yellow"},
    {"CPSS_DXCH3_POLICER_MNG_CNTR_RED_E",    "red"},
    {"CPSS_DXCH3_POLICER_MNG_CNTR_DROP_E",   "drop"},
  };
  res = {};
  for k,s_type in pairs(set_types) do
    did_error, result, OUT_values =
      genericCpssApiWithErrorHandler(
        command_data, "cpssDxCh3PolicerManagementCountersGet",{
        {"IN", "GT_U8", "devNum", dev_num},
        {"IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", stage},
        {"IN", "CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT", "mngCntrSet", set},
        {"IN", "CPSS_DXCH3_POLICER_MNG_CNTR_TYPE_ENT", "mngCntrType", s_type[1]},
        {"OUT","CPSS_DXCH3_POLICER_MNG_CNTR_ENTRY_STC", "mngCntrPtr"},
      });
    if did_error then
      print("cpssDxCh3PolicerManagementCountersGet failed");
      return true;
    end
    res[s_type[2]] = OUT_values.mngCntrPtr;
  end
  return false, res;
end

local function policy_mng_counters_show(params)
  local devices, j, k, s, mng_cntr, did_error;
  local command_data = Command_Data();

  if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
  else
    devices={params["devID"]};
  end

  for j=1,#devices do
    print("dev " .. tostring(devices[j]) .. " stage " .. params.stage);
    did_error, mng_cntr = policy_mng_counters_get(
      command_data, devices[j], params.stage, params.set);
    if did_error then
      return false;
    end
    for k,s in pairs(mng_cntr) do
      print(
        string.format("%-10.10s",k) --alignment
        .. " packets " .. tostring(s.packetMngCntr)
        .. " bytes high " .. tostring(s.duMngCntr.l[1])
        .. " low " .. tostring(s.duMngCntr.l[0]));
    end
  end
  return true;
end

CLI_addCommand("exec", "show policy-mng-conters", {
   func = policy_mng_counters_show,
   help="reset policy-management counter set",
   params={
  { type="named",
    { format= "device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", name = "stage", help = "Policer stage"},
    { format = "set %policy_mng_cnt_set_cfg", name = "set", help = "Counter set"},
    mandatory = {"devID", "stage", "set"},
    requirements={
      ["stage"] = {"devID"},
      ["set"] = {"stage"},
    },
  }
  }
})

local function policy_meter_stage_enable(params)
  local mode, counting;
  local devices, j, did_error, result, OUT_values;
  local command_data = Command_Data();
  if params.enable then
    mode     = params.mode;
    counting = params.counting;
    if not mode then
      mode = "CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E";
    end
    if not counting then
      counting = "CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E";
    end
  else
    mode     = "CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E";
    counting = "CPSS_DXCH_POLICER_COUNTING_DISABLE_E";
  end

  if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
  else
    devices={params["devID"]};
  end

  for j=1,#devices do
    did_error, result, OUT_values =
      genericCpssApiWithErrorHandler(
        command_data, "cpssDxCh3PolicerMeteringEnableSet",{
        {"IN", "GT_U8", "devNum", devices[j]},
        {"IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", params.stage},
        {"IN", "GT_BOOL","enable",params.enable},
      });
    if did_error then
      print("cpssDxCh3PolicerMeteringEnableSet failed");
      return false;
    end

    did_error, result, OUT_values =
      genericCpssApiWithErrorHandler(
        command_data, "cpssDxChPolicerStageMeterModeSet",{
        {"IN", "GT_U8", "devNum", devices[j]},
        {"IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", params.stage},
        {"IN","CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT","mode",mode},
      });
    if did_error then
      print("cpssDxChPolicerStageMeterModeSet failed");
      return false;
    end

    did_error, result, OUT_values =
      genericCpssApiWithErrorHandler(
        command_data, "cpssDxChPolicerCountingModeSet",{
        {"IN", "GT_U8", "devNum", devices[j]},
        {"IN", "CPSS_DXCH_POLICER_STAGE_TYPE_ENT", "stage", params.stage},
        {"IN","CPSS_DXCH_POLICER_COUNTING_MODE_ENT","mode", counting},
      });
    if did_error then
      print("cpssDxChPolicerCountingModeSet failed");
      return false;
    end
  end
  return true;
end

CLI_addCommand("config", "policy-meter stage-enable", {
   func = function(params)
     params.enable = true;
     return policy_meter_stage_enable(params);
   end,
   help="enable and configure policer stage",
   params={
  { type="named",
    { format= "device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", help = "Policer stage"},
    { format = "mode %policy_stage_meter_mode", help = "Port or Flow mode"},
    { format = "counting %policy_global_counting_mode", help = "Counting mode"},
    mandatory = {"devID", "stage"},
    requirements={
      ["stage"]    = {"devID"},
      ["mode"]     = {"stage"},
      ["counting"] = {"mode"},
    },
  }
  }
})

CLI_addCommand("config", "no policy-meter stage-enable", {
   func = function(params)
     params.enable = false;
     return policy_meter_stage_enable(params);
   end,
   help="disable policer stage",
   params={
  { type="named",
    { format= "device %devID_all", name="devID", help="The device number" },
    { format = "stage %policer_stage", help = "Policer stage"},
    mandatory = {"devID", "stage"},
    requirements={
      ["stage"]    = {"devID"},
    },
  }
  }
})

local function policy_meter_hierachical_map(params)
  local devices, j, did_error, result, OUT_values;
  local hear_entry, index;
  local command_data = Command_Data();

  if (params["devID"]=="all") then
    devices=wrLogWrapper("wrlDevList")
  else
    devices={params["devID"]};
  end
  for j=1,#devices do
    for index = 0,(params.size - 1) do
      hear_entry = {
        countingEnable = params.counting,
        meteringEnable = params.metering,
        policerPointer = (params["base-map"] + (params.increment * index)),
      };
      did_error, result, OUT_values =
        genericCpssApiWithErrorHandler(
          command_data, "cpssDxChPolicerHierarchicalTableEntrySet",{
          {"IN", "GT_U8", "devNum", devices[j]},
          {"IN", "GT_U32", "index", (params.base + index)},
          {"IN","CPSS_DXCH_POLICER_HIERARCHICAL_TABLE_ENTRY_STC","entryPtr",hear_entry},
        });
      if did_error then
        print("cpssDxChPolicerHierarchicalTableEntrySet failed");
        return false;
      end
    end
  end
  return true;
end

CLI_addCommand("config", "policy-meter hierachical-map", {
   func = policy_meter_hierachical_map,
   help="configure ingress policy meter hierachical map",
   params={
  { type="named",
    { format= "device %devID_all", name="devID", help="The device number" },
    { format = "base %GT_U32", help = "Base index of configured range"},
    { format = "size %GT_U32", help = "Size of configured range"},
    { format = "base-map %GT_U32", help = "The mapped value for the base"},
    { format = "increment %GT_U32", help = "The increment of mapped value (typically 0,1 or -1)"},
    { format = "metering %bool", help = "metering enable/disable"},
    { format = "counting %bool", help = "counting enable/disable"},
    mandatory = {"devID", "base", "size", "base-map", "increment", "metering", "counting"},
    requirements={
      ["base"]      = {"devID"},
      ["size"]      = {"base"},
      ["base-map"]  = {"size"},
      ["increment"] = {"base-map"},
      ["metering"]  = {"increment"},
      ["counting"]  = {"metering"},
    },
  }
  }
})

