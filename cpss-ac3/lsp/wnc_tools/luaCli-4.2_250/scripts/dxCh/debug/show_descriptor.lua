--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_descriptor.lua
--*
--* DESCRIPTION:
--*       showing of the descriptor information
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants



-- ************************************************************************
---
-- The array "DESCRIPTOR_FIELDS_ARRAY" was replicated from the ENUM
--   "CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_ENT"   in the file 
--   mainPpDrv\h\cpss\dxCh\dxChxGen\diag\cpssDxChDiagDescriptor.h. 
--   
--   Note: in case the ENUM CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT changes this
--         file must be updated as well.
--  
  local DESCRIPTOR_FIELDS_ARRAY = {
    [ 1] =" AppSpecificCPUCode ",
    [ 2] =" ARPPtr ",
    [ 3] =" BadIPxBC ",
    [ 4] =" Bank ",
    [ 5] =" BillingEn ",
    [ 6] =" BufReuse ",
    [ 7] =" BypassBridge_Dup ",
    [ 8] =" BypassBridge ",
    [ 9] =" BypassIngressPipe ",
    [10] =" ByteCount ",
    [11] =" CCFC ",
    [12] =" CFI0 ",
    [13] =" CFI1 ",
    [14] =" CFI ",
    [15] =" CFMPkt ",
    [16] =" CongestedPortNumber ",
    [17] =" CongestedQueueNumber ",
    [18] =" ControlTC ",
    [19] =" CopyTTL ",
    [20] =" CPIDMatch ",
    [21] =" CPU2CPUMirror ",
    [22] =" CPUCode ",
    [23] =" CPUMailToNeigborCPU ",
    [24] =" CPUTrgDev ",
    [25] =" DecTTL ",
    [26] =" DIP bits 0..31 ",
    [27] =" DIP bits 32..63 ",
    [28] =" DIP bits 64..95 ",
    [29] =" DIP bits 96..127 ",
    [30] =" DIP ",
    [31] =" DisableIPCL1_for_RoutedPackets ",
    [32] =" DoRouteHa ",
    [33] =" DoRoute ",
    [34] =" DP ",
    [35] =" DropCode ",
    [36] =" DropOnSource ",
    [37] =" DropPkt ",
    [38] =" DSCP ",
    [39] =" EgressFilterEn ",
    [40] =" EgressFilterRegistered ",
    [41] =" EgressTrgDev ",
    [42] =" EgressTrgPort_dup_2_0 ",
    [43] =" EgressTrgPort ",
    [44] =" EnableBridgeLogFlow ",
    [45] =" EnableBridge ",
    [46] =" EnableDsaTagManipulation ",
    [47] =" EnableRouterLogFlow ",
    [48] =" EnableSourceLocalFiltering ",
    [49] =" Enable_IPCL0_lookup ",
    [50] =" Enable_Policy ",
    [51] =" EtherType ",
    [52] =" ExcludedDevIsLocal ",
    [53] =" ExcludedPort ",
    [54] =" ExcludedTrunk ",
    [55] =" ExcludeIsTrunk ",
    [56] =" EXP ",
    [57] =" FirstBuffer ",
    [58] =" FloodMLLPointer ",
    [59] =" FlowID ",
    [60] =" FlowSubTemplate ",
    [61] =" Force_IPCL1_configurations ",
    [62] =" Fragmented ",
    [63] =" FromCPUDP ",
    [64] =" FromCPUTC ",
    [65] =" HashMaskIndex ",
    [66] =" IGMPNonQuery ",
    [67] =" IGMPQuery ",
    [68] =" InLIF_Mode ",
    [69] =" InnerIsLLCNonSnap ",
    [70] =" InnerL3Offset1_dup ",
    [71] =" InnerL3Offset ",
    [72] =" InnerPktType ",
    [73] =" IPCL_Profile_Index ",
    [74] =" IPLegal ",
    [75] =" IPMgmntEn ",
    [76] =" IPModifyDSCP ",
    [77] =" IPModifyUP ",
    [78] =" IPM ",
    [79] =" IPQoSProfile ",
    [80] =" Ipv4MulticastEn ",
    [81] =" IPv4OptionField ",
    [82] =" IPv4RIPv1 ",
    [83] =" Ipv4UnicastEn ",
    [84] =" IPv4_Bad_GRE_Header ",
    [85] =" IPv4_DF ",
    [86] =" IPv6FlowLabel ",
    [87] =" IPv6HBHExt ",
    [88] =" IPv6HBHOptions ",
    [89] =" Ipv6ICMPType ",
    [90] =" IPv6ICMP ",
    [91] =" Ipv6MulticastEn ",
    [92] =" IPv6NonHBHExt ",
    [93] =" IPv6SiteID ",
    [94] =" Ipv6UnicastEn ",
    [95] =" IPv6_EH ",
    [96] =" IPxHeaderTotalLength ",
    [97] =" IPxMACMcMismatch ",
    [98] =" IPxMcLinkLocalProt ",
    [99] =" IPxProtocol ",
    [100] =" IP_Fragmented ",
    [101] =" IP_Header_Info ",
    [102] =" IsARPReply ",
    [103] =" IsARP ",
    [104] =" IsBC ",
    [105] =" IsIPv4 ",
    [106] =" IsIPv4_ForKey ",
    [107] =" IsIPv6LinkLocal ",
    [108] =" IsIPv6MLD ",
    [109] =" IsIPv6 ",
    [110] =" IsIPv6_ForKey ",
    [111] =" IsIP ",
    [112] =" IsIP_ForKey ",
    [113] =" IsMPLS ",
    [114] =" IsND ",
    [115] =" IsPTP ",
    [116] =" IsSYN ",
    [117] =" L2Echo ",
    [118] =" L2Encapsulation ",
    [119] =" L2_Valid ",
    [120] =" L3OffsetInvalid ",
    [121] =" L3TotalOffset_or_InnerHeaderTotalOffset ",
    [122] =" L4Byte13 ",
    [123] =" L4Bytes3_0 ",
    [124] =" L4OffsetInvalid ",
    [125] =" L4Valid ",
    [126] =" LLT ",
    [127] =" LocalDeviceSrcTrunkID ",
    [128] =" LocalDevSrcPort ",
    [129] =" Lookup0_PCL_mode ",
    [130] =" Lookup0_UseIndexFromDesc ",
    [131] =" Lookup1_PCL_mode ",
    [132] =" Lookup1_UseIndexFromDesc ",
    [133] =" Lookup2_PCL_mode ",
    [134] =" Lookup2_UseIndexFromDesc ",
    [135] =" mac2me ",
    [136] =" MacDaType ",
    [137] =" MACDA bits 0..31 ",
    [138] =" MACDA bits 32..48 ",
    [139] =" MacSAArpSAMismatch ",
    [140] =" MACSA bits 0..31 ",
    [141] =" MACSA bits 32..48 ",
    [142] =" MailBoxToNeighborCPU ",
    [143] =" MarvellTaggedExtended ",
    [144] =" MarvellTagged ",
    [145] =" MCRPFFail ",
    [146] =" MDB ",
    [147] =" MeteringEn ",
    [148] =" MLLEn ",
    [149] =" MLLLast ",
    [150] =" MLLPtr ",
    [151] =" MLL ",
    [152] =" MLL_Pointer ",
    [153] =" ModifyDP ",
    [154] =" ModifyDSCP ",
    [155] =" ModifyDSCP_EXP ",
    [156] =" ModifyEXP ",
    [157] =" ModifyTC ",
    [158] =" ModifyUP ",
    [159] =" MPLSCmd ",
    [160] =" MPLSLabel ",
    [161] =" MPLS_Label0 ",
    [162] =" MPLS_Label1 ",
    [163] =" MPLS_Label2 ",
    [164] =" MPLS_OuterEXP ",
    [165] =" MPLS_OuterLabel ",
    [166] =" MPLS_OuterS_Bit ",
    [167] =" NestedVlanAccessPort ",
    [168] =" NHEMCRPFFailCmd ",
    [169] =" Num_of_IngressTags ",
    [170] =" OrigIsTrunk ",
    [171] =" OrigQCNPrio ",
    [172] =" OrigSrcDev ",
    [173] =" OrigSrcPort ",
    [174] =" OrigSrcTrunk ",
    [175] =" OrigVID ",
    [176] =" OuterIsLLCNonSnap ",
    [177] =" OuterL3TotalOffset ",
    [178] =" OuterSrcTag ",
    [179] =" OutGoingMtagCmd ",
    [180] =" OutLIF_Type ",
    [181] =" PacketCmd ",
    [182] =" PacketIsCNTagged ",
    [183] =" PacketIsLLCSnap ",
    [184] =" PacketReject ",
    [185] =" PacketTOS ",
    [186] =" PacketType ",
    [187] =" PacketUP ",
    [188] =" PCL1_IPv4_Key_Type ",
    [189] =" PCL1_IPv6_Key_Type ",
    [190] =" PCL1_Lookup_mode ",
    [191] =" PCL1_Lookup_Type ",
    [192] =" PCL1_NonIP_Key_Type ",
    [193] =" PCL1_PCL_ID ",
    [194] =" PCLAssignedSSTID ",
    [195] =" PktCmd ",
    [196] =" PktHash ",
    [197] =" PktIsLooped ",
    [198] =" PktTypeForKey ",
    [199] =" PolicerEn ",
    [200] =" PolicerPtr ",
    [201] =" PolicyBasedRouted ",
    [202] =" PolicyRTTIndex ",
    [203] =" PortSpeedIndex_or_OrigQCNPrio ",
    [204] =" PrioTagged ",
    [205] =" PseudoWireID ",
    [206] =" PTPTriggerType ",
    [207] =" QCNPacket ",
    [208] =" QCNqDelta ",
    [209] =" QCNqFb ",
    [210] =" QCNqOffset ",
    [211] =" QCNqSaturated ",
    [212] =" QCN ",
    [213] =" QinQ ",
    [214] =" QosProfilePrecedence ",
    [215] =" QosProfile ",
    [216] =" RecalcCRC ",
    [217] =" RedirectCmd ",
    [218] =" Reserved bits 0..31 ",
    [219] =" Reserved bits 32..63 ",
    [220] =" Reserved bits 64..65 ",
    [221] =" Reserved bits 64..90 ",
    [222] =" Reserved ",
    [223] =" RGARPIndex ",
    [224] =" RGBypassTTLandOptionChecks ",
    [225] =" RGDecTTL ",
    [226] =" RGEgressInterface ",
    [227] =" RGFwdCmd ",
    [228] =" RGICMP_RedirectCheckEn ",
    [229] =" RGMirrorToIngressAnalyzerPort ",
    [230] =" RGModifyDSCP ",
    [231] =" RGModifyUP ",
    [232] =" RGNHEVID ",
    [233] =" RGPCERouteEntry ",
    [234] =" RGQoSProfile ",
    [235] =" RGQoS_Precedence ",
    [236] =" RGQoS_ProfileMarkingEn ",
    [237] =" Routed ",
    [238] =" RouteToEntry ",
    [239] =" RsrvdCopy ",
    [240] =" RxSniff ",
    [241] =" SIP bits 0..31 ",
    [242] =" SIP bits 32..63 ",
    [243] =" SIP bits 64..95 ",
    [244] =" SIP bits 96..127 ",
    [245] =" SniffTrgDev ",
    [246] =" SniffTrgPort ",
    [247] =" SniffVIDX ",
    [248] =" SolicitationMulticastMessage ",
    [249] =" SrcDevIsOwn ",
    [250] =" SrcDev ",
    [251] =" SrcIsLoopback ",
    [252] =" SrcPort ",
    [253] =" SrcTagged_dup ",
    [254] =" SrcTagged ",
    [255] =" SrcTCPUDPPortIs0 ",
    [256] =" SrcTrg ",
    [257] =" Src_is_PE ",
    [258] =" SST_ID ",
    [259] =" SYNWithData ",
    [260] =" TargetIsTrunk ",
    [261] =" TCPFlags ",
    [262] =" TCP_UDPDestPort ",
    [263] =" TCP_UDP_SrcPort ",
    [264] =" TC ",
    [265] =" TrgDevice ",
    [266] =" TrgDevice_VirtualSrcDev ",
    [267] =" TrgDev ",
    [268] =" TrgIsTrunk ",
    [269] =" TrgPort ",
    [270] =" TrgPort_VirtualSrcPort ",
    [271] =" TrgTagged_dup ",
    [272] =" TrgTagged ",
    [273] =" TrgTrunkID ",
    [274] =" Truncated ",
    [275] =" TS_Passenger_Type ",
    [276] =" TTL ",
    [277] =" TunnelPtr ",
    [278] =" TunnelStart ",
    [279] =" TunnelTerminated ",
    [280] =" TunnelType ",
    [281] =" TxSampled ",
    [282] =" UDB0 ",
    [283] =" UDB10 ",
    [284] =" UDB11 ",
    [285] =" UDB12 ",
    [286] =" UDB13 ",
    [287] =" UDB14 ",
    [288] =" UDB15 ",
    [289] =" UDB16 ",
    [290] =" UDB17 ",
    [291] =" UDB18 ",
    [292] =" UDB19 ",
    [293] =" UDB1 ",
    [294] =" UDB20 ",
    [295] =" UDB21 ",
    [296] =" UDB22 ",
    [297] =" UDB2 ",
    [298] =" UDB3 ",
    [299] =" UDB4 ",
    [300] =" UDB5 ",
    [301] =" UDB6 ",
    [302] =" UDB7 ",
    [303] =" UDB8 ",
    [304] =" UDB9 ",
    [305] =" UDB_BytesValid ",
    [306] =" UP1 ",
    [307] =" UP ",
    [308] =" UseInterfaceBasedIndex ",
    [309] =" UseVIDX ",
    [310] =" VID1 ",
    [311] =" VIDModified ",
    [312] =" VIDX ",
    [313] =" VID ",
    [314] =" VID_or_InLIF_ID ",
    [315] =" VID_Precedence ",
    [316] =" VirtualInterface ",
    [317] =" VLAN1_Exist ",
    [318] =" VRF_ID ",
    [319] =" VR_ID ",
    [320] =" SrcOwnDev0 ",
    [321] =" CongestedQueueFillLevel ",
    [322] ="LAST_E"
}  

-- ************************************************************************
---
--  showDescriptor
--        @description  The array "DESCRIPTOR_FIELDS_ARRAY" was replicated
--                      from the ENUM
--                              "CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_ENT"
--                      in the file mainPpDrv\h\cpss\dxCh\dxChxGen\diag\
--                      cpssDxChDiagDescriptor.h. Note: in case the ENUM
--                      CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT changes this
--                      file must be updated as well.
--
--        @param params         - The parameters
--
local function showDescriptor(params)
  local result, values, i, v, devices, j

  if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

  for j=1,#devices do
    result, values = myGenWrapper(
      "cpssDxChDiagDescriptorGet", {
        { "IN", "GT_U8"  , "devNum", devices[j]},  -- devNum
        { "IN", "GT_U32" , "descriptorType",  params["descType"]},
        { "OUT", "CPSS_DXCH_DIAG_DESCRIPTOR_STC", "descriptorPtr"},
      }
    )

    print("Device:".. devices[j])
    if (result==0) then
      for i,v in pairs(values["descriptorPtr"]["fieldValueValid"]) do
        if v==true then 
          local fieldName = DESCRIPTOR_FIELDS_ARRAY[i+1]
          if fieldName == nil then fieldName = "unknown" end
          print(string.format("%-46s0x%08X", fieldName, values["descriptorPtr"]["fieldValue"][i])   ) 
        end
        
      end
    elseif params["devID"]=="all" then
      print("Descriptor type #" .. tostring(params["descType"]) .. " possibly not supported on this device type.")
    else
      print("retCode =" .. tostring(result) .. ". Descriptor type #" .. tostring(params["descType"]))
      return false, "Error"
    end
  end

end

-- ************************************************************************
---
--  showDescriptors
--        @description  Show "DESCRIPTOR_FIELDS_ARRAY" for all or single 
--                      "CPSS_DXCH_DIAG_DESCRIPTOR_FIELD_TYPE_ENT" entries
--
--        @param params         - The parameters ( descType == -1 for ALL)
--
local function showDescriptors(params)
  local k, v
  if params.descType == nil then return false end
  if params.descType ~= -1 then return showDescriptor(params) end

  print("-------- SHOW ALL DESCRIPTORS --------")

  for k, v in pairs(CLI_type_dict["descType"]["enum"]) do
    if v.value ~= -1 then

      print ("------- DESCRIPTOR: " .. tostring(k))
      params.descType = tonumber(v.value)
      showDescriptor(params)
    end
  end
end



-- ************************************************************************
---
--  showDescriptorRegister
--        @description  Callback. Add <show descriptor> device-depend commands 
--                      to Lua CLI after cpssInitSystem
--
--        @param params  - none
--
function showDescriptorRegister()

   -- if boardEnv.devList[0].type == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E"
  if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_BOBCAT2_E"] == nil then
    -- add the suitable command for all devices except bobCat2
    CLI_addCommand("debug", "show descriptor", {
      func = showDescriptors,
      help = "Show descriptor information",
      params={
        { type="named",
          { format="device %devID_all",  name="devID",    help="The device number\n" } ,
          { format="descType %descTypeAll", name="descType", help="The type of the descriptor\n" },
          mandatory={"devID","descType"},
          requirements={descType={"devID"}}
        }
      }
    })
  else

    -- idebug: configure recipe - last descriptor
    CLI_addCommand("debug", "show descriptor", {
        help="show last-descriptor",
        func=function(params)
          return last_descriptor(params)
        end,
        params={
          {  type="values", 
            { format="@descriptor", help="descriptor name" }
          }
        }
    })
    -- idebug: configure recipe - last descriptor
    CLI_addCommand("debug", "show descriptor PORT2MEM", {
        help="(descriptor)",
        func=function(params)
          params["descriptor"] = "PORT2MEM"
          return last_descriptor(params)
        end,
        params={
          { type="named", 
            { format="port %port", name="port", help="port number"},
            mandatory={"port"}
          }
        }
    })
    -- idebug: configure recipe - last descriptor
    CLI_addCommand("debug", "show descriptor all", {
        help="show all descriptors",
        func=function(params)
          return last_descriptor_all(params)
        end,
        params={
          {  type="named",
            { format="port %port", name="port", help="port number"},
            { format="to %ipv4",   name = "ip", help = "The tftp's ip to upload the log file"},
            requirements={ip={"port"}},
            mandatory={"port"}
          }
        }
    })
  end
end

-- insert showDescriptorRegister() callback to global list
table.insert(typeCallBacksList, showDescriptorRegister)
