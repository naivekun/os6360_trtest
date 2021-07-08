--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* policyIf.lua
--*
--* DESCRIPTION:
--*       policy rules related commands for vlan and ports
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
--  setVlanPolicyID
--
--  @description Sets the vlan to a policy rule
--
--  @param params - params["policyID"]      - the policy ID number
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************



local function setVlanPolicyID(params)
    local policer_stc,ret,val
  local mode
  local command_data = Command_Data()
  
  local CPSS_DXCH_POLICER_STAGE_INGRESS_0_E = 0
  local CPSS_DXCH_POLICER_E_ATTR_METER_MODE_DISABLED_E = 0
  local CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E = 2
    -- Common variables initialization
    command_data:initInterfaceDevVlanRange()
    command_data:initInterfaceDeviceRange("dev_vlan_range")
    command_data:initDevVlanInfoIterator()
  
  if params.flagNo~=true then 
    policer_stc={
      meteringEnable = true,
      countingEnable = true,
      ucKnownEnable = true,
      ucUnknownEnable = true,
      mcRegisteredEnable = true,
      mcUnregisteredEnable = true,
      bcEnable = true,
      tcpSynEnable = true,
      policerIndex = params["policerID"]
    }
    mode = CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EVLAN_ENABLED_E -- eVlan enable
  else 
    mode = CPSS_DXCH_POLICER_E_ATTR_METER_MODE_DISABLED_E -- disable
  end
  
  -- Main vlan handling cycle
  if true == command_data["status"] then
    for iterator, devNum, vlanId, vlan_info in 
                  command_data:getDeviceVlanInfoIterator() do  
      ret,val = myGenWrapper("cpssDxChPolicerEAttributesMeteringModeSet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",CPSS_DXCH_POLICER_STAGE_INGRESS_0_E},
        {"IN","CPSS_DXCH_POLICER_E_ATTR_METER_MODE_ENT","mode",mode}  --eVlan enabled
      })
      if ret ~= 0 then
        print("Could not set policy id " .. params["policerID"])
        return false
      end    
        
      ret,val = myGenWrapper("cpssDxChPolicerTriggerEntrySet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",CPSS_DXCH_POLICER_STAGE_INGRESS_0_E},   --stage 0 ingress
        {"IN","GT_U32","entryIndex",vlanId},
        {"IN","CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC","entryPtr",policer_stc}
      }) 
      if ret ~= 0 then
        print("Could not set policy id " .. params["policerID"])
        return false
      end      
    end
  end
  
    return true
end

--------------------------------------------
-- command registration: policy-id
--------------------------------------------

CLI_addCommand("vlan_configuration", "policy-id", {
  func=setVlanPolicyID,
  help="Sets the vlan to a policy rule",
   params={
    {type="values", "%policerID"}
  }
})


--------------------------------------------
-- command registration: no policy
--------------------------------------------

CLI_addCommand("vlan_configuration", "no policy", {
   func=function(params)
      params.flagNo=true
      return setVlanPolicyID(params) end,
  help="Invalidates the specified policer-ID",
   params={
    {type="values", "%policerID"}
  }
})



-- ************************************************************************
--  setPortPolicyID
--
--  @description adds a new policy rule for a port
--
--  @param params - params["action"]      - the action to take, either drop or tansmit
--          params["commitedRate"]    - The commited rate in Kbps
--          params["commitedBurst"]    - Thecommited burst size in bytes
--          params["flagNo"]       - disable the policy
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************



local function setPortPolicyID(params)
    local policer_stc,ret,val,devices,j
  local enable
  local command_data = Command_Data()
  
  local CPSS_DXCH_POLICER_STAGE_INGRESS_0_E = 0
  local CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E = 0
  local CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E = 1
  
  if (params["peakRateKbps"] ~= nil and params["peakBurstByte"] == nil) then
    print("Could not set policyId, pbs parameter is missing")
    return false
  end
    
    -- Common variables initialization.    
    command_data:initInterfaceRangeIterator()   
    command_data:initInterfaceDeviceRange()

    policer_stc={
        packetSizeMode = "CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E",
        tunnelTerminationPacketSizeMode = "CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E",
        dsaTagCountingMode = "CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_COMPENSATED_E",
        timeStampCountingMode = "CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_INCLUDE_E",
        yellowEcnMarkingEnable = "GT_FALSE"
    }
  
  if params.flagNo~=true then
        policer_stc["policerEnable"]=true
    if (params["peakRateKbps"] ~= nil and params["peakBurstByte"] ~= nil) then
      policer_stc["tokenBucketParams"]={
        trTcmParams = {
          cir = params["committedRate"],
          cbs = params["committedBurst"],
          pir = params["peakRateKbps"],
          pbs = params["peakBurstByte"]
        }
      }
      policer_stc["meterMode"]= CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E      
    else
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
      devices = {}
      
      for iterator, devNum, portNum in command_data:getDevicesIterator() do -- get list of the devices
        table.insert(devices, devNum)
      end
      
      if (dropPrecedenceQosProfile(devices)==false) then
        print("Could not set policy ids ")
        return false
      end
    elseif (params["action"]=="drop") then
            policer_stc["redPcktCmd"]="CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E"
        end
    enable = true -- enable
  else
    enable = false --  disable
  end
  
  -- Main interface handling cycle
  if true == command_data["status"] then
    for iterator, devNum, portNum in command_data:getPortIterator() do
      
      ret,val = myGenWrapper("cpssDxCh3PolicerPortMeteringEnableSet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",CPSS_DXCH_POLICER_STAGE_INGRESS_0_E},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"IN","GT_BOOL","enable",enable}  -- enable/disable
      })
      if ret ~= 0 then
        print("Could not set policy id " .. portNum)
        return false
      end    
        ret,val = myGenWrapper("cpssDxCh3PolicerMeteringEntrySet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","CPSS_DXCH_POLICER_STAGE_TYPE_ENT","stage",CPSS_DXCH_POLICER_STAGE_INGRESS_0_E},   --stage 0 ingress
            {"IN","GT_U32","entryIndex",portNum},
            {"IN","CPSS_DXCH3_POLICER_METERING_ENTRY_STC","entryPtr",policer_stc},
            {"OUT","CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT_srTcmParams","tbParamsPtr"}
        }) 
        if ret ~= 0 then
            print("Could not set policy id " .. portNum)
            return false
        end  
      
    end
  end
  
    return true
end

--------------------------------------------
-- command registration: policy-id
--------------------------------------------

CLI_addCommand("interface", "policy-id", {
  func=setPortPolicyID,
  help="Sets the port to a policy rule",
   params={
    { type="named",
      { format="police %commitedRateKbps", name="committedRate", help="The committed rate in Kbps" },
      { format="burst %commitedBurstByte", name="committedBurst", help="The committed burst size in bytes" },
      { format="pir %peakRateKbps", name="peakRateKbps", help="The Peak Information Rate in Kbps" },
      { format="pbs %peakBurstByte", name="peakBurstByte", help="The Peak Burst Size in bytes" },
      { format="exceed-action %exceedAction", name="action", help="Action to take if the packet exceeds the rate" },
      requirements={
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
CLI_addCommand("interface", "no policy", {
   func=function(params)
      params.flagNo=true
      return setPortPolicyID(params) end,
  help="Invalidates the specified policer-ID",
   params={
  }
})
