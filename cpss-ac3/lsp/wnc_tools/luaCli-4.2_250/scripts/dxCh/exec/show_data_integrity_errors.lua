--********************************************************************************
--*              (c), Copyright 2014, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_data_integrity_errors.lua
--*
--* DESCRIPTION:
--*  showing of data integrity errors -  errorCounter, failedRow, failedSyndrome
--*    for specified device and type of memory
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--registering c functions

cmdLuaCLI_registerCfunction("wrlCpssDxChDiagDataIntegrityErrorInfoGet")


-- *****************************************************************************
---
--  show_data_integrity_events
--        @description  shows TODO
--
--        @param params.devID   -
--        @param params.portGroupID   -
--        @param params.evType   -
--
--        @return       true on success, otherwise false and error message
--
local function show_data_integrity_errors(params)
  local rc, tbl, val, devID
  local errorCounter, failedRow, failedSyndrome

  devID = params.devID
  
  if boardEnv == nil or boardEnv.devList == nil or boardEnv.devList[devID+1] == nil or 
     boardEnv.devList[devID+1].type ~= "CPSS_PP_FAMILY_DXCH_LION2_E" then
    printErr(" Not applicable device\n")
    return
  end
  

  -- table description
  local tableInfo = {
    { head="Error counter",   len=15, align="r", path="errorCounter",   type="number" },
    { head="Failed raw",      len=15, align="r", path="failedRow",      type="number" },
    { head="Failed syndrome", len=15, align="r", path="failedSyndrome", type="number" }
  }

  -- create table according to description
  tbl = Nice_Table(tableInfo)
  
  
  if params.memType == "regular" then
    local memType = params.memTypeEnum
    local dfxPipeId = params.dfxPipeId
    local dfxClientId = params.dfxClientId
    local dfxMemoryId = params.dfxMemoryId
    rc, errorCounter, failedRow, failedSyndrome = wrLogWrapper("wrlCpssDxChDiagDataIntegrityErrorInfoGet", 
                                                                  "(devID, memType, dfxPipeId, dfxClientId, dfxMemoryId)", 
                                                                  devID, memType, dfxPipeId, dfxClientId, dfxMemoryId)
    if rc ~= 0 then
      printErr(" wrlCpssDxChDiagDataIntegrityErrorInfoGet  failed, rc = " .. returnCodes[rc].."\n")
      return false
    end
    tbl:addRow({["errorCounter"] = errorCounter, ["failedRow"] = failedRow, ["failedSyndrome"]=failedSyndrome})
  elseif params.memType == "mppm" then
    local memType = params.memTypeEnum
    local portGroupId = params.portGroupId
    local mppmId = params.mppmId
    local bankId = params.bankId
    rc, errorCounter, failedRow, failedSyndrome = wrLogWrapper("wrlCpssDxChDiagDataIntegrityErrorInfoGet", 
                                                                  "(devID, memType, portGroupId, mppmId, bankId)", 
                                                                  devID, memType, portGroupId, mppmId, bankId)
    if rc ~= 0 then
      printErr(" wrlCpssDxChDiagDataIntegrityErrorInfoGet  failed, rc = " .. returnCodes[rc].."\n")
      return false
    end
    tbl:addRow({["errorCounter"] = errorCounter, ["failedRow"] = failedRow, ["failedSyndrome"]=failedSyndrome})
  elseif params.memType == "tcam" then
    local memType = params.memTypeEnum
    local tcamArrayType = params.tcamArrayType
    local ruleIndex = params.ruleIndex
    rc, errorCounter, failedRow, failedSyndrome = wrLogWrapper("wrlCpssDxChDiagDataIntegrityErrorInfoGet", 
                                                                  "(devID, memType, tcamArrayType, ruleIndex)", 
                                                                  devID, memType, tcamArrayType, ruleIndex)
    if rc ~= 0 then
      printErr(" wrlCpssDxChDiagDataIntegrityErrorInfoGet  failed, rc = " .. returnCodes[rc].."\n")
      return false
    end
    tbl:addRow({["errorCounter"] = errorCounter, ["failedRow"] = failedRow, ["failedSyndrome"]=failedSyndrome})  
  else
    return false
  end

  -- print table
  if #tbl.rows > 0 then
    tbl:print()
  else
    print("No errors to show")
  end
  
  return

end

-- *****************************************************************************
-- CLI_type_dict  section
--
CLI_type_dict["tcamArrayType"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "tcam array type",
    enum = {
        ["ARRAY_X"] =    { value="CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E",
                                help="Special memory location indexes for TCAM memory" },
        ["ARRAY_Y"] =       { value="CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E", 
                                help="Special memory location indexes for MPPM memory" }
    }
} 

CLI_type_dict["memTypeEnum"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT",
    enum = {
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_CONTROL_ACCESS_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_CONTROL_ACCESS_TABLE_E",
              help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_LINKED_LIST_BUFFERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_LINKED_LIST_BUFFERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_FINAL_UDB_CLEAR_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_FINAL_UDB_CLEAR_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_DIST_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_DIST_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_UC_DIST_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_UC_DIST_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CNC_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CNC_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CPFC_PFC_IND_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CPFC_PFC_IND_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CTU_DBM_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CTU_DBM_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CTU_DESC_UNUSED_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CTU_DESC_UNUSED_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_CONF_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_CONF_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_DATA_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_DATA_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_EPLR_DESC_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_EPLR_DESC_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_TCAM_DESC_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_TCAM_DESC_FIFO_E",
            help="TCAM TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_BILLING_EGRESS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_BILLING_EGRESS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_IPFIX_AGING_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_IPFIX_AGING_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_IPFIX_WRAPAROUND_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_IPFIX_WRAPAROUND_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_METERING_EGRESS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_METERING_EGRESS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_POLICER_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_POLICER_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_QOS_REMARKING_TABLE_EGRESS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPLR_QOS_REMARKING_TABLE_EGRESS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_CONF_LIMIT_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_CONF_LIMIT_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_CPU_CODE_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_CPU_CODE_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_QOS_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_QOS_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_RATE_LIMIT_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_RATE_LIMIT_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_STC_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_STC_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_TRUNK_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_TRUNK_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_VPM_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_VPM_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_MAC_SA_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_MAC_SA_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_TSARP_TUNNEL_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_TSARP_TUNNEL_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_VLAN_TRANSLATION_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HA_VLAN_TRANSLATION_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_AGING_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_AGING_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG2_DESC_RETURN_TWO_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG2_DESC_RETURN_TWO_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG4_DESC_RETURN_TWO_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG4_DESC_RETURN_TWO_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_VLAN_URPF_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_VLAN_URPF_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_PCR_REGISTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_PCR_REGISTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_UNUSED_DATA_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_UNUSED_DATA_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_UPDATE_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2I_UPDATE_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_CONFI_PROCESSOR_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_CONFI_PROCESSOR_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_GRD_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_GRD_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_DESCRIPTOR_FILE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_DESCRIPTOR_FILE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_REG_FILE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_REG_FILE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_RX_FIFO_2_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_RX_FIFO_2_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_TX_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_TX_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_WRR_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_WRR_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_PCL2MLL_UNUSED_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_PCL2MLL_UNUSED_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_REPLICATIONS_IN_USE_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_REPLICATIONS_IN_USE_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_TABLE_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_TABLE_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E",
            help="MPPM TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E",
            help="MPPM TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MT_MAC_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MT_MAC_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MT_RF_TOP_MAC_UPD_OUT_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MT_RF_TOP_MAC_UPD_OUT_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CONFIG_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CONFIG_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CRC_HASH_MASK_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_CRC_HASH_MASK_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_0_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_0_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_1_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_1_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_2_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_LOOKUP_FIFO_2_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_UNUSED_DATA_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PCL_UNUSED_DATA_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR0_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR0_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR1_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR1_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_DESCRIPTOR_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_DESCRIPTOR_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_AGING_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_AGING_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_WRAPAROUND_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_IPFIX_WRAPAROUND_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR1_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR1_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_POLICER_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_POLICER_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_QOS_REMARKING_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_QOS_REMARKING_TABLE_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ACTION_TABLE_LOWER_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ACTION_TABLE_LOWER_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ECC_TABLE_DATA_LOWER_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ECC_TABLE_DATA_LOWER_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ECC_TABLE_MASK_LOWER_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_ECC_TABLE_MASK_LOWER_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_LOWER_ANSWER_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_LOWER_LOWER_ANSWER_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_ACTION_TABLE_UPPER_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_ACTION_TABLE_UPPER_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_ECC_TABLE_DATA_UPPER_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_ECC_TABLE_DATA_UPPER_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_UPPER_ANSWER_FIFO_CL0_1_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_UPPER_ANSWER_FIFO_CL0_1_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_UPPER_ANSWER_FIFO_CL2_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCC_UPPER_UPPER_ANSWER_FIFO_CL2_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_LOOKUP_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_LOOKUP_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_PORT_PROTOCOL_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_PORT_PROTOCOL_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_PORT_VLAN_QOS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_PORT_VLAN_QOS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_TXQ_E2E_FC_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_TXQ_E2E_FC_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UDB_CFG_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UDB_CFG_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UDB_PROFILE_ID_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UDB_PROFILE_ID_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UNUSED_DATA_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_UNUSED_DATA_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_VLAN_TRANSLATION_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TTI_VLAN_TRANSLATION_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_CPU_TX_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_CPU_TX_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_DATA_FROM_MPPM_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_DATA_FROM_MPPM_E",
            help="MPPM TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_INFO_DESC_PREFETCH_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_INFO_DESC_PREFETCH_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_MEM_CLEAR_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_MEM_CLEAR_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_NEXT_LIST_BUFFERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_NEXT_LIST_BUFFERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_READ_BURST_STATE_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_READ_BURST_STATE_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_TX_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_TX_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DIST_BURST_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DIST_BURST_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_STC_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_STC_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PORT_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_DESIGNATED_TBL_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_DESIGNATED_TBL_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_0_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_0_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_1_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_1_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_NON_TRUNK_MEMBER_TBL2_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_NON_TRUNK_MEMBER_TBL2_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_NON_TRUNK_MEMBER_TBL_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_NON_TRUNK_MEMBER_TBL_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_SEC_TRG_TBL_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_SEC_TRG_TBL_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_SST_TBL_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_SST_TBL_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QHEAD_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QTAIL_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_QTAIL_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_BUFFER_FIFO_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_BUFFER_FIFO_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_BUF_LIMIT_DP0_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_BUF_LIMIT_DP0_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_DESC_LIMIT_DP0_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_DESC_LIMIT_DP0_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP0_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP0_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP12_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP12_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_0_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_0_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_1_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_1_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_2_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_2_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_3_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_3_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_4_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_4_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_5_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_5_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_6_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_6_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_7_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_7_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_0_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_0_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_1_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_1_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_2_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_2_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_3_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_3_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_4_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_4_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_5_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_5_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_6_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_6_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_7_COUNTERS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_7_COUNTERS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_XOFF_THRES_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_XOFF_THRES_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_XON_THRES_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_XON_THRES_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_SAMPLE_INTERVALS_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_SAMPLE_INTERVALS_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_SHARED_Q_LIMIT_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_SHARED_Q_LIMIT_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_SPT_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_SPT_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_VLAN_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_VLAN_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_SPT_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_SPT_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_MAP_DEV_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_MAP_DEV_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L2_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L2_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L3_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L3_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_VIDX_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_VIDX_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_WRDMA_IBUF_BANK_RAM_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_WRDMA_IBUF_BANK_RAM_E",
            help="REGULAR TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E",
            help="TCAM TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E",
            help="TCAM TYPE" },
      ["CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_TRUNK_LTT_TABLE_E"] = 
          { value="CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EQ_TRUNK_LTT_TABLE_E",
            help="REGULAR TYPE" }
    }
} 

-- *****************************************************************************
-- CLI_addCommand section
--
CLI_addCommand("exec", "show data-integrity-errors regular", {
  func = function(params)
    params.memType = "regular"
    return show_data_integrity_errors(params)
  end,
  help   = "Show data integrity errors for regular memory location indexes",
  params={{type= "named",
            {format="device %devID",        name="devID",       help="ID of the device"},
            {format="type %memTypeEnum",         name="memTypeEnum", help="type"},
            {format="dfxPipeId %GT_U32",    name="dfxPipeId",   help="DFX pipe ID"},
            {format="dfxClientId %GT_U32",  name="dfxClientId", help="DFX client ID"},
            {format="dfxMemoryId %GT_U32",  name="dfxMemoryId", help="DFX memory ID"},
            requirements={ memTypeEnum={"devID"}, dfxPipeId={"memTypeEnum"}, dfxClientId={"dfxPipeId"}, dfxMemoryId={"dfxClientId"}},
            mandatory = {"devID", "memTypeEnum", "dfxPipeId", "dfxClientId", "dfxMemoryId"}
          }}
})

-- *****************************************************************************
--
CLI_addCommand("exec", "show data-integrity-errors mppm", {
  func = function(params)
    params.memType = "mppm"
    return show_data_integrity_errors(params)
  end,
  help   = "Show data integrity errors for special memory location indexes for MPPM memory",
  params={{type= "named",
            {format="device %devID",        name="devID",       help="ID of the device"},
            {format="type %memTypeEnum",    name="memTypeEnum", help="type"},
            {format="portGroupId %GT_U32",  name="portGroupId", help="port group ID"},
            {format="mppmId %GT_U32",       name="mppmId",      help="MPPM ID"},
            {format="bankId %GT_U32",       name="bankId",      help="MPPM bank ID"},
            requirements={ ["memTypeEnum"]={"devID"}, ["portGroupId"]={"memTypeEnum"}, 
                                     ["mppmId"]={"portGroupId"}, ["bankId"]={"mppmId"}},
            mandatory = {"devID", "memTypeEnum", "portGroupId", "mppmId", "bankId"}
          }}
})

-- *****************************************************************************
--
CLI_addCommand("exec", "show data-integrity-errors tcam", {
  func = function(params)
    params.memType = "tcam"
    return show_data_integrity_errors(params)
  end,
  help   = "Show data integrity errors for special memory location indexes for TCAM memory",
  params={{type= "named",
            {format="device %devID",             name="devID",         help="ID of the device"},
            {format="type %memTypeEnum",         name="memTypeEnum",   help="type"},
            {format="arrayType %tcamArrayType",  name="tcamArrayType", help="tcam array type"},
            {format="ruleIndex %GT_U32",         name="ruleIndex",     help="index of the standard rule in the TCAM"},
            requirements={ ["memTypeEnum"]={"devID"}, ["tcamArrayType"]={"memTypeEnum"}, 
                                                        ["ruleIndex"]={"tcamArrayType"}},
            mandatory = {"devID", "memTypeEnum", "tcamArrayType", "ruleIndex"}
          }}
})
