cpssAPIs["cpssDxChCosDscpToProfileMapSet"] = {
  { "IN", "GT_U8", "devNum" },
  { "IN", "GT_U8", "dscp" },
  { "IN", "GT_U32", "profileIndex" }
}
cpssAPIs["cpssDxChCosUpCfiDeiToProfileMapSet"] = {
  { "IN", "GT_U8", "devNum" },
  { "IN", "GT_U32", "upProfileIndex" },
  { "IN", "GT_U8", "up" },
  { "IN", "GT_U8", "cfiDeiBit" },
  { "IN", "GT_U32", "profileIndex" }
}
cpssAPIs["cpssDxChCosUpCfiDeiToProfileMapGet"] = {
  { "IN", "GT_U8", "devNum" },
  { "IN", "GT_U32", "upProfileIndex" },
  { "IN", "GT_U8", "up" },
  { "IN", "GT_U8", "cfiDeiBit" },
  { "OUT", "GT_U32", "profileIndexPtr" }
}
cpssAPIs["cpssDxChBrgVlanPortVidGet"] = {
  { "IN", "GT_U8", "devNum" },
  { "IN", "GT_PORT_NUM", "portNum" },
  { "OUT", "GT_U16", "vid" }
}
cpssAPIs["cpssDxChBrgVlanPortVidSet"] = {
  { "IN", "GT_U8", "devNum" },
  { "IN", "GT_PORT_NUM", "portNum" },
  { "IN", "GT_U16", "vlanId" }
}
cpssAPIs["cpssDxChPclPortLookupCfgTabAccessModeSet"] = {
  { "IN", "GT_U8", "devNum" },
  { "IN", "GT_PORT_NUM", "portNum" },
  { "IN", "CPSS_PCL_DIRECTION_ENT", "direction" },
  { "IN", "CPSS_PCL_LOOKUP_NUMBER_ENT", "lookupNum" },
  { "IN", "GT_U32", "subLookupNum" },
  { "IN", "CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT", "mode" },
  { "IN", "GT_U32", "entryIndex", 0 }
}
