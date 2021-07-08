--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port-manager.lua
--*
--* DESCRIPTION:
--*     Port manager.
--*
--*
--*
--*
--********************************************************************************

--includes


--constants
function bit(p)
  return 2 ^ (p)  -- 1-based indexing
end

-- Typical call:  if hasbit(x, bit(3)) then ...
function hasbit(x, p)
  return x % (bit(p) + bit(p)) >= bit(p)
end

function shiftRight(x, p)
    return math.floor(x / 2 ^ p)
end

function bitand(a, b)
    local result = 0
    local bitval = 1
    while a > 0 and b > 0 do
      if a % 2 == 1 and b % 2 == 1 then -- test the rightmost bits
          result = result + bitval      -- set the current bit
      end
      bitval = bitval * 2 -- shift left
      a = math.floor(a/2) -- shift right
      b = math.floor(b/2)
    end
    return result
end

function stringToEnum_portParams(str)
    if str == "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E" then
        return 0
    elseif str == "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E" then
        return 1
    elseif str == "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E" then
        return 2
    elseif str == "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_ABILITY_E" then
        return 3
    elseif str == "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_REQUESTED_E" then
        return 4
    else
        print("error at port parameter type")
    end
end

function stringToEnum_portAttribute(str)
    if str == "CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_TUNE_MODE_E" then
        return 0
    elseif str == "CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_ADAPTIVE_RX_TUNE_SUPPORTED_E" then
        return 1
    elseif str == "CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_EDGE_DETECT_SUPPORTED_E" then
        return 2
    elseif str == "CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_LOOPBACK_E" then
        return 3
    else
        print("error at port attribute type")
    end
end

function stringToEnum_apPportAttribute(str)
    if str == "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FEC_REQUIRED_E" then
        return 0
    elseif str == "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FEC_SUPPORTED_E" then
        return 1
    elseif str == "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_NONCE_DISABLED_E" then
        return 2
    elseif str == "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FC_PAUSE_E" then
        return 3
    elseif str == "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FC_ASM_DIR_E" then
        return 4
    elseif str == "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_LANE_NUM_E" then
        return 5
        else
            print("error at ap port attribute type")
    end
end

function stringToEnum_event(str)
    if str == "CPSS_PORT_MANAGER_EVENT_CREATE_E" then
        return 0
    elseif str == "CPSS_PORT_MANAGER_EVENT_DELETE_E" then
        return 1
    elseif str == "CPSS_PORT_MANAGER_EVENT_ENABLE_E" then
        return 2
    elseif str == "CPSS_PORT_MANAGER_EVENT_DISABLE_E" then
        return 3
    elseif str == "PORT_MANAGER_EVENT_DELETE_E" then
        return 4
    elseif str == "CPSS_PORT_MANAGER_EVENT_INIT_E" then
        return 5
    elseif str == "CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E" then
        return 6
    elseif str == "CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E" then
        return 7
    elseif str == "CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E" then
        return 8
    else
        print("error at port event")
    end
end

function fecToVal(fec)
      if fec ==  "CPSS_DXCH_PORT_FEC_MODE_ENABLED_E" then
          return 0
      elseif fec ==  "CPSS_DXCH_PORT_FEC_MODE_DISABLED_E" or fec == nil then
          return 1
      elseif fec ==  "CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E" then
          return 2
      elseif fec ==  "CPSS_DXCH_PORT_BOTH_FEC_MODE_ENABLED_E" then
          return 3
      else
          print("error at fec type",fec)
      end
end

function fcToVal(fec)
      if fec ==  "CPSS_DXCH_PORT_AP_FLOW_CONTROL_SYMMETRIC_E" or fec ==  "CPSS_PORT_AP_FLOW_CONTROL_SYMMETRIC_E" then
          return 0
      elseif fec ==  "CPSS_DXCH_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E" or fec == "CPSS_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E" then
          return 1
      else
          print("error at flow control type",fec)
      end
end

function portTypeStrGet(portTypeFullStr)
    if  "CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E"    == portTypeFullStr      then
        return "AP"
    elseif "CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E"   == portTypeFullStr      then
        return "Regular"
    else
        print("error at portType string type ",portTypeFullStr)
    end
end


function fecStrGet(fecFullStr)
    if  "CPSS_PORT_FEC_MODE_ENABLED_E"    == fecFullStr      then
        return "FC"
    elseif "CPSS_PORT_FEC_MODE_DISABLED_E"   == fecFullStr      then
        return "Disabled"
    elseif "CPSS_PORT_RS_FEC_MODE_ENABLED_E"   == fecFullStr      then
        return "RS"
    elseif "CPSS_PORT_BOTH_FEC_MODE_ENABLED_E"   == fecFullStr      then
        return "Both"
    else
        print("error at fec string type ",fecFullStr)
    end
end
function tuneStrGet(tuneFullStr)
    if  "CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E"    == tuneFullStr      then
        return "Tx cfg"
    elseif  "CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E"    == tuneFullStr      then
        return "Tx start"
    elseif  "CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E"    == tuneFullStr      then
        return "Tx status"
    elseif  "CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E"    == tuneFullStr      then
        return "Rx"
    elseif  "CPSS_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E"    == tuneFullStr      then
        return "stop"
    elseif  "CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E"    == tuneFullStr      then
        return "adaptive start"
    elseif  "CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E"    == tuneFullStr      then
        return "adaptive stop"
    elseif  "CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENHANCE_E"    == tuneFullStr      then
        return "enhance"
    elseif  "CPSS_PORT_SERDES_AUTO_TUNE_MODE_EDGE_DETECT_E"    == tuneFullStr      then
        return "edge-detect"
    elseif  "CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_WAIT_FOR_FINISH_E"    == tuneFullStr      then
        return "Rx wait-for-finish"
    else
        print("error at tune string type ",tuneFullStr)
    end
end

function loopbackStrGet(lbFullStr)
    if  "CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E"    == lbFullStr      then
        return "MAC"
    elseif "CPSS_PORT_MANAGER_LOOPBACK_TYPE_PCS_E"   == lbFullStr      then
        return "PCS"
    elseif "CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E"   == lbFullStr      then
        return "SERDES"
    else
        return "NONE"
    end
end

-- ************************************************************************
---
--  ap_func
--        @description  enable/disable ap process on port
--
--
--
--        @return       error message if fails
--
local function port_manager_parameters_set_func(params)
  local command_data = Command_Data()
  local devNum, portNum, ret, enable
  local GT_OK=0
  local apParamsPtr={}
  local fecA
  local fecR
  local portParamsStcPtr={}
  local laneIdx, modeParamsIdx, attrIdx, bit

--  portManagerParams={}

  portParamsStcPtr.portType=0
  portParamsStcPtr.portParams={}
  portParamsStcPtr.portParams.port={}
  portParamsStcPtr.portParams.port.portModeParams={}
  portParamsStcPtr.portParams.port.portModeParams.modeMandatoryParams={}

  -- mandatory params
  portParamsStcPtr.portParams.port.portModeParams.modeMandatoryParams.ifMode = params.ifMode
  portParamsStcPtr.portParams.port.portModeParams.modeMandatoryParams.speed =  params.speed

  --print("PARAMS1:",params.ifMode," ", params.speed)
  --print("PARAMS2:",portParamsStcPtr.portParams.port.portModeParams.modeMandatoryParams.ifMode,
  --      " ", portParamsStcPtr.portParams.port.portModeParams.modeMandatoryParams.speed)

  portParamsStcPtr.portParams.port.portModeParams.modeParamsArr={}
  portParamsStcPtr.portParams.port.portModeParams.modeParamsArrSize = 0
  portParamsStcPtr.portParams.port.portAttributesArr = {}
  portParamsStcPtr.portParams.port.portAttributesArrSize = 0

  modeParamsIdx = 0
  laneIdx = 0

  --print("packed bitmap #1- ",params.rx_bitmap_packed)
  --  print("params #1- ",params)
  if params.rx_bitmap_packed ~= nil then

      if params.rx_squelch_packed == nil then
          params.rx_squelch_packed = 0
      end

      if params.rx_LB_packed == nil then
          params.rx_LB_packed = 0
      end

      if params.rx_HF_packed == nil then
          params.rx_HF_packed = 0
      end

      if params.rx_LF_packed == nil then
          params.rx_LF_packed = 0
      end

      if params.rx_dcGain_packed == nil then
          params.rx_dcGain_packed = 0
      end


      --print("packed bitmap- ",params.rx_bitmap_packed)
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx] = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.totalLanes = 0
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].paramType =
        stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")
      for bit=0,3 do
          --print("packed bitmap- with bit- ",bit)
          if hasbit(params.rx_bitmap_packed,bit) then
              --print("packed bitmap- with bit- ",bit, " has bit! ")
              -- increase total lanes
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.totalLanes =
                portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.totalLanes+1
              -- squelch
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx] = {}
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams = {}
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams.sqlch =
                bitand(shiftRight((params.rx_squelch_packed),bit*8),255)
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].laneType = 1
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].lane = bit
              -- LB
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams.loopBandwidth =
                bitand(shiftRight((params.rx_LB_packed),bit*8),255)
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].laneType = 1
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].lane = bit
              -- HF
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams.ffeCap =
                bitand(shiftRight((params.rx_HF_packed),bit*8),255)
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].laneType = 1
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].lane = bit
              -- LF
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams.ffeRes =
                bitand(shiftRight((params.rx_LF_packed),bit*8),255)
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].laneType = 1
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].lane = bit
              -- dc gain
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams.dcGain =
                bitand(shiftRight((params.rx_dcGain_packed),bit*8),255)
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].laneType = 1
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].lane = bit


              --print("[calculated] packed bitmap- with bit- ",bit, " has bit! - squelch- ",bitand(shiftRight((params.rx_squelch_packed),bit*8),255)," LB- ",
              --      bitand(shiftRight((params.rx_LB_packed),bit*8),255)," HF- ",  bitand(shiftRight((params.rx_HF_packed),bit*8),255), " LF- ",
              --      bitand(shiftRight((params.rx_LF_packed),bit*8),255), " GAIN- ",bitand(shiftRight((params.rx_dcGain_packed),bit*8),255))

              --print("packed bitmap- with bit- ",bit, " has bit! - squelch- ",params.rx_squelch_packed," LB- ",
               --     params.rx_LB_packed," HF- ", params.rx_HF_packed, " LF- ", params.rx_LF_packed, " GAIN- ",params.rx_dcGain_packed)

              laneIdx = laneIdx + 1
           end
      end
      modeParamsIdx = modeParamsIdx + 1
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArrSize = portParamsStcPtr.portParams.port.portModeParams.modeParamsArrSize + 1
  end

  --print("AFTER. LF- ",portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[0].param.serdesRxParams.rxCfg[0].rxParams.lowFrequency ," "
  --,"AFTER. HF- ", portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[0].param.serdesRxParams.rxCfg[0].rxParams.highFrequency )

  laneIdx = 0
  if params.tx_bitmap_packed ~= nil then

      if params.tx_amp_packed == nil then
          params.tx_amp_packed = 0
      end

      if params.tx_emph0_packed == nil then
          params.tx_emph0_packed = 0
      end

      if params.tx_emph1_packed == nil then
          params.tx_emph1_packed = 0
      end

      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx] = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.totalLanes = 0
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].paramType =
        stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")
      for bit=0,3 do
          if hasbit(params.tx_bitmap_packed,bit) then
              -- increase total lanes
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.totalLanes =
                portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.totalLanes+1
              -- tx_amp_packed
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx] = {}
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].txParams = {}
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].txParams.txAmp =
                bitand(shiftRight((params.tx_amp_packed),bit*8),255)
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].laneType = 1
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].lane = bit
              -- tx_emph0_packed
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].txParams.emph0 =
                bitand(shiftRight((params.tx_emph0_packed),bit*8),255)
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].laneType = 1
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].lane = bit
              -- tx_emph1_packed
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].txParams.emph1 =
                bitand(shiftRight((params.tx_emph1_packed),bit*8),255)
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].laneType = 1
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].lane = bit
              laneIdx = laneIdx + 1
           end
      end
      modeParamsId= modeParamsIdx + 1
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArrSize = portParamsStcPtr.portParams.port.portModeParams.modeParamsArrSize + 1
  end

  laneIdx = 0
  if params.polarity_rx_packed  ~= nil then
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx] = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity.totalLanes = 0
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].paramType =
        stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")
      for bit=0,3 do
          if hasbit(params.polarity_rx_packed,bit) then
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity = {}
              -- increase total lanes
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity.totalLanes =
                portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity.totalLanes+1
              -- polarity_rx_packed
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg[laneIdx] = {}
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg[laneIdx].rxPolarity =
                bitand(shiftRight((params.polarity_rx_packed),bit),1)
              -- polarity_tx_packed
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg[laneIdx].txPolarity =
                bitand(shiftRight((params.polarity_tx_packed),bit),1)

              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg[laneIdx].laneType = 1
              portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg[laneIdx].lane = bit

              laneIdx = laneIdx + 1
           end
      end
      modeParamsIdx = modeParamsIdx + 1
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArrSize = portParamsStcPtr.portParams.port.portModeParams.modeParamsArrSize + 1
  end

  if params.fec_ability  ~= nil then
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx] = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].paramType =
        stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_ABILITY_E")
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param = {}
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArr[modeParamsIdx].param.fecAbility = params.fec_ability
      portParamsStcPtr.portParams.port.portModeParams.modeParamsArrSize = portParamsStcPtr.portParams.port.portModeParams.modeParamsArrSize + 1
  end

  attrIdx = 0
  if params.tune_mode  ~= nil then
      portParamsStcPtr.portParams.port.portAttributesArrSize = portParamsStcPtr.portParams.port.portAttributesArrSize + 1
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx] = {}
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx].attributeType =
        stringToEnum_portAttribute("CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_TUNE_MODE_E")
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx].attribute = {}
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx].attribute.trainMode = params.tune_mode
      attrIdx = attrIdx + 1
  end

  if params.adaptive_rx_supported  ~= nil then
      portParamsStcPtr.portParams.port.portAttributesArrSize = portParamsStcPtr.portParams.port.portAttributesArrSize + 1
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx] = {}
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx].attributeType =
        stringToEnum_portAttribute("CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_ADAPTIVE_RX_TUNE_SUPPORTED_E")
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx].attribute = {}
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx].attribute.adaptiveRxTrainSupported = params.tune_mode
      attrIdx = attrIdx + 1
  end

  if params.edge_detect_supported  ~= nil then
      portParamsStcPtr.portParams.port.portAttributesArrSize = portParamsStcPtr.portParams.port.portAttributesArrSize + 1
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx] = {}
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx].attributeType =
        stringToEnum_portAttribute("CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_EDGE_DETECT_SUPPORTED_E")
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx].attribute = {}
      portParamsStcPtr.portParams.port.portAttributesArr[attrIdx].attribute.edgeDetectSupported = params.tune_mode
      attrIdx = attrIdx + 1
  end

--  portManagerParams.modeParams.modeParamsArr[0].param.serdesRxParams.totalLanes = 1
--  portManagerParams.modeParams.modeParamsArr[0].param.serdesRxParams.rxCfg[0].lane = 0
--  portManagerParams.modeParams.modeParamsArr[0].param.serdesRxParams.rxCfg[0].rxParams.squelch = params.lane0_squelch
--  portManagerParams.modeParams.modeParamsArr[0].param.serdesRxParams.rxCfg[0].rxParams.LF = params.lane0_LF
--  portManagerParams.modeParams.modeParamsArr[0].param.serdesRxParams.rxCfg[0].laneType = 1;

  local iterator
  for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
    command_data:clearPortStatus()
    command_data:clearLocalStatus()
    if(nil~=devNum) then
      ret = myGenWrapper("prvCpssPortManagerPortParamsSet_L",{
      {"IN","GT_U8","devNum",devNum},
      {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
      {"IN","PRV_CPSS_PORT_MANAGER_GENERIC_PARAMS_STC_L","portParamsStcPtr",portParamsStcPtr}})
      if(ret~=GT_OK) then
        print("Error at command")
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
      end
    end
  end

  -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function port_manager_ap_port_parameters_set_func(params)
  local command_data = Command_Data()
  local devNum, portNum, ret, enable
  local GT_OK=0
  local apParamsPtr={}
  local fecA
  local fecR
  local portParamsStcPtr={}
  local laneIdx, modeParamsIdx, attrIdx, bit
  local advIdx, tmpParamsStc

  advIdx = 0

  tmpParamsStc = {}
  tmpParamsStc.ifMode = {}
  tmpParamsStc.speed = {}
  tmpParamsStc.rx_squelch_packed = {}
  tmpParamsStc.rx_dcGain_packed = {}
  tmpParamsStc.rx_LB_packed = {}
  tmpParamsStc.rx_HF_packed = {}
  tmpParamsStc.rx_LF_packed = {}
  tmpParamsStc.rx_bitmap_packed = {}
  tmpParamsStc.tx_amp_packed = {}
  tmpParamsStc.tx_emph0_packed = {}
  tmpParamsStc.tx_emph1_packed = {}
  tmpParamsStc.tx_bitmap_packed = {}
  tmpParamsStc.polarity_rx_packed = {}
  tmpParamsStc.polarity_tx_packed = {}
  tmpParamsStc.fec_ability = {}
  tmpParamsStc.fec_requested = {}

  tmpParamsStc.ifMode[0] = params.mode1
  tmpParamsStc.speed[0] = params.speed1
  --tmpParamsStc.rx_squelch_packed[0] = params.rx_squelch_packed0
  --tmpParamsStc.rx_dcGain_packed[0] = params.rx_dcGain_packed0
  --tmpParamsStc.rx_LB_packed[0] = params.rx_LB_packed0
  --tmpParamsStc.rx_HF_packed[0] = params.rx_HF_packed0
  --tmpParamsStc.rx_LF_packed[0] = params.rx_LF_packed0
  --tmpParamsStc.rx_bitmap_packed[0] = params.rx_bitmap_packed0
  --tmpParamsStc.tx_amp_packed[0] = params.tx_amp_packed0
  --tmpParamsStc.tx_emph0_packed[0] = params.tx_emph0_packed0
  --tmpParamsStc.tx_emph1_packed[0] = params.tx_emph1_packed0
  --tmpParamsStc.tx_bitmap_packed[0] = params.tx_bitmap_packed0
  --tmpParamsStc.polarity_rx_packed[0] = params.polarity_rx_packed0
  --tmpParamsStc.polarity_tx_packed[0] = params.polarity_tx_packed0
  tmpParamsStc.fec_ability[0] = params.fec_ability1
  tmpParamsStc.fec_requested[0] = params.fec_requested1

  tmpParamsStc.ifMode[1] = params.mode2
  tmpParamsStc.speed[1] = params.speed2
  --tmpParamsStc.rx_squelch_packed[1] = params.rx_squelch_packed1
  --tmpParamsStc.rx_dcGain_packed[1] = params.rx_dcGain_packed1
  --tmpParamsStc.rx_LB_packed[1] = params.rx_LB_packed1
  --tmpParamsStc.rx_HF_packed[1] = params.rx_HF_packed1
  --tmpParamsStc.rx_LF_packed[1] = params.rx_LF_packed1
  --tmpParamsStc.rx_bitmap_packed[1] = params.rx_bitmap_packed1
  --tmpParamsStc.tx_amp_packed[1] = params.tx_amp_packed1
  --tmpParamsStc.tx_emph0_packed[1] = params.tx_emph0_packed1
  --tmpParamsStc.tx_emph1_packed[1] = params.tx_emph1_packed1
  --tmpParamsStc.tx_bitmap_packed[1] = params.tx_bitmap_packed1
  --tmpParamsStc.polarity_rx_packed[1] = params.polarity_rx_packed1
  --tmpParamsStc.polarity_tx_packed[1] = params.polarity_tx_packed1
  tmpParamsStc.fec_ability[1] = params.fec_ability2
  tmpParamsStc.fec_requested[1] = params.fec_requested2

  tmpParamsStc.ifMode[2] = params.mode3
  tmpParamsStc.speed[2] = params.speed3
  --tmpParamsStc.rx_squelch_packed[2] = params.rx_squelch_packed2
  --tmpParamsStc.rx_dcGain_packed[2] = params.rx_dcGain_packed2
  --tmpParamsStc.rx_LB_packed[2] = params.rx_LB_packed2
  --tmpParamsStc.rx_HF_packed[2] = params.rx_HF_packed2
  --tmpParamsStc.rx_LF_packed[2] = params.rx_LF_packed2
  --tmpParamsStc.rx_bitmap_packed[2] = params.rx_bitmap_packed2
  --tmpParamsStc.tx_amp_packed[2] = params.tx_amp_packed2
  --tmpParamsStc.tx_emph0_packed[2] = params.tx_emph0_packed2
  --tmpParamsStc.tx_emph1_packed[2] = params.tx_emph1_packed2
  --tmpParamsStc.tx_bitmap_packed[2] = params.tx_bitmap_packed2
  --tmpParamsStc.polarity_rx_packed[2] = params.polarity_rx_packed2
  --tmpParamsStc.polarity_tx_packed[2] = params.polarity_tx_packed2
  tmpParamsStc.fec_ability[2] = params.fec_ability3
  tmpParamsStc.fec_requested[2] = params.fec_requested3

  tmpParamsStc.ifMode[3] = params.mode4
  tmpParamsStc.speed[3] = params.speed4
  --tmpParamsStc.rx_squelch_packed[3] = params.rx_squelch_packed3
  --tmpParamsStc.rx_dcGain_packed[3] = params.rx_dcGain_packed3
  --tmpParamsStc.rx_LB_packed[3] = params.rx_LB_packed3
  --tmpParamsStc.rx_HF_packed[3] = params.rx_HF_packed3
  --tmpParamsStc.rx_LF_packed[3] = params.rx_LF_packed3
  --tmpParamsStc.rx_bitmap_packed[3] = params.rx_bitmap_packed3
  --tmpParamsStc.tx_amp_packed[3] = params.tx_amp_packed3
  --tmpParamsStc.tx_emph0_packed[3] = params.tx_emph0_packed3
  --tmpParamsStc.tx_emph1_packed[3] = params.tx_emph1_packed3
  --tmpParamsStc.tx_bitmap_packed[3] = params.tx_bitmap_packed3
  --tmpParamsStc.polarity_rx_packed[3] = params.polarity_rx_packed3
  --tmpParamsStc.polarity_tx_packed[3] = params.polarity_tx_packed3
  tmpParamsStc.fec_ability[3] = params.fec_ability4
  tmpParamsStc.fec_requested[3] = params.fec_requested4

  tmpParamsStc.ifMode[4] = params.mode5
  tmpParamsStc.speed[4] = params.speed5
  --tmpParamsStc.rx_squelch_packed[4] = params.rx_squelch_packed4
  --tmpParamsStc.rx_dcGain_packed[4] = params.rx_dcGain_packed4
  --tmpParamsStc.rx_LB_packed[4] = params.rx_LB_packed4
  --tmpParamsStc.rx_HF_packed[4] = params.rx_HF_packed4
  --tmpParamsStc.rx_LF_packed[4] = params.rx_LF_packed4
  --tmpParamsStc.rx_bitmap_packed[4] = params.rx_bitmap_packed4
  --tmpParamsStc.tx_amp_packed[4] = params.tx_amp_packed4
  --tmpParamsStc.tx_emph0_packed[4] = params.tx_emph0_packed4
  --tmpParamsStc.tx_emph1_packed[4] = params.tx_emph1_packed4
  --tmpParamsStc.tx_bitmap_packed[4] = params.tx_bitmap_packed4
  --tmpParamsStc.polarity_rx_packed[4] = params.polarity_rx_packed4
  --tmpParamsStc.polarity_tx_packed[4] = params.polarity_tx_packed4
  tmpParamsStc.fec_ability[4] = params.fec_ability5
  tmpParamsStc.fec_requested[4] = params.fec_requested5

  tmpParamsStc.ifMode[5] = params.mode6
  tmpParamsStc.speed[5] = params.speed6
  --tmpParamsStc.rx_squelch_packed[5] = params.rx_squelch_packed5
  --tmpParamsStc.rx_dcGain_packed[5] = params.rx_dcGain_packed5
  --tmpParamsStc.rx_LB_packed[5] = params.rx_LB_packed5
  --tmpParamsStc.rx_HF_packed[5] = params.rx_HF_packed5
  --tmpParamsStc.rx_LF_packed[5] = params.rx_LF_packed5
  --tmpParamsStc.rx_bitmap_packed[5] = params.rx_bitmap_packed5
  --tmpParamsStc.tx_amp_packed[5] = params.tx_amp_packed5
  --tmpParamsStc.tx_emph0_packed[5] = params.tx_emph0_packed5
  --tmpParamsStc.tx_emph1_packed[5] = params.tx_emph1_packed5
  --tmpParamsStc.tx_bitmap_packed[5] = params.tx_bitmap_packed5
  --tmpParamsStc.polarity_rx_packed[5] = params.polarity_rx_packed5
  --tmpParamsStc.polarity_tx_packed[5] = params.polarity_tx_packed5
  tmpParamsStc.fec_ability[5] = params.fec_ability6
  tmpParamsStc.fec_requested[5] = params.fec_requested6

  tmpParamsStc.ifMode[6] = params.mode7
  tmpParamsStc.speed[6] = params.speed7
  --tmpParamsStc.rx_squelch_packed[6] = params.rx_squelch_packed6
  --tmpParamsStc.rx_dcGain_packed[6] = params.rx_dcGain_packed6
  --tmpParamsStc.rx_LB_packed[6] = params.rx_LB_packed6
  --tmpParamsStc.rx_HF_packed[6] = params.rx_HF_packed6
  --tmpParamsStc.rx_LF_packed[6] = params.rx_LF_packed6
  --tmpParamsStc.rx_bitmap_packed[6] = params.rx_bitmap_packed6
  --tmpParamsStc.tx_amp_packed[6] = params.tx_amp_packed6
  --tmpParamsStc.tx_emph0_packed[6] = params.tx_emph0_packed6
  --tmpParamsStc.tx_emph1_packed[6] = params.tx_emph1_packed6
  --tmpParamsStc.tx_bitmap_packed[6] = params.tx_bitmap_packed6
  --tmpParamsStc.polarity_rx_packed[6] = params.polarity_rx_packed6
  --tmpParamsStc.polarity_tx_packed[6] = params.polarity_tx_packed6
  tmpParamsStc.fec_ability[6] = params.fec_ability7
  tmpParamsStc.fec_requested[6] = params.fec_requested7

  tmpParamsStc.ifMode[7] = params.mode8
  tmpParamsStc.speed[7] = params.speed8
  --tmpParamsStc.rx_squelch_packed[7] = params.rx_squelch_packed7
  --tmpParamsStc.rx_dcGain_packed[7] = params.rx_dcGain_packed7
  --tmpParamsStc.rx_LB_packed[7] = params.rx_LB_packed7
  --tmpParamsStc.rx_HF_packed[7] = params.rx_HF_packed7
  --tmpParamsStc.rx_LF_packed[7] = params.rx_LF_packed7
  --tmpParamsStc.rx_bitmap_packed[7] = params.rx_bitmap_packed7
  --tmpParamsStc.tx_amp_packed[7] = params.tx_amp_packed7
  --tmpParamsStc.tx_emph0_packed[7] = params.tx_emph0_packed7
  --tmpParamsStc.tx_emph1_packed[7] = params.tx_emph1_packed7
  --tmpParamsStc.tx_bitmap_packed[7] = params.tx_bitmap_packed7
  --tmpParamsStc.polarity_rx_packed[7] = params.polarity_rx_packed7
  --tmpParamsStc.polarity_tx_packed[7] = params.polarity_tx_packed7
  tmpParamsStc.fec_ability[7] = params.fec_ability8
  tmpParamsStc.fec_requested[7] = params.fec_requested8

  tmpParamsStc.ifMode[8] = params.mode9
  tmpParamsStc.speed[8] = params.speed9
  --tmpParamsStc.rx_squelch_packed[8] = params.rx_squelch_packed8
  --tmpParamsStc.rx_dcGain_packed[8] = params.rx_dcGain_packed8
  --tmpParamsStc.rx_LB_packed[8] = params.rx_LB_packed8
  --tmpParamsStc.rx_HF_packed[8] = params.rx_HF_packed8
  --tmpParamsStc.rx_LF_packed[8] = params.rx_LF_packed8
  --tmpParamsStc.rx_bitmap_packed[8] = params.rx_bitmap_packed8
  --tmpParamsStc.tx_amp_packed[8] = params.tx_amp_packed8
  --tmpParamsStc.tx_emph0_packed[8] = params.tx_emph0_packed8
  --tmpParamsStc.tx_emph1_packed[8] = params.tx_emph1_packed8
  --tmpParamsStc.tx_bitmap_packed[8] = params.tx_bitmap_packed8
  --tmpParamsStc.polarity_rx_packed[8] = params.polarity_rx_packed8
  --tmpParamsStc.polarity_tx_packed[8] = params.polarity_tx_packed8
  tmpParamsStc.fec_ability[8] = params.fec_ability9
  tmpParamsStc.fec_requested[8] = params.fec_requested9

  tmpParamsStc.ifMode[9] = params.mode10
  tmpParamsStc.speed[9] = params.speed10
  --tmpParamsStc.rx_squelch_packed[9] = params.rx_squelch_packed9
  --tmpParamsStc.rx_dcGain_packed[9] = params.rx_dcGain_packed9
  --tmpParamsStc.rx_LB_packed[9] = params.rx_LB_packed9
  --tmpParamsStc.rx_HF_packed[9] = params.rx_HF_packed9
  --tmpParamsStc.rx_LF_packed[9] = params.rx_LF_packed9
  --tmpParamsStc.rx_bitmap_packed[9] = params.rx_bitmap_packed9
  --tmpParamsStc.tx_amp_packed[9] = params.tx_amp_packed9
  --tmpParamsStc.tx_emph0_packed[9] = params.tx_emph0_packed9
  --tmpParamsStc.tx_emph1_packed[9] = params.tx_emph1_packed9
  --tmpParamsStc.tx_bitmap_packed[9] = params.tx_bitmap_packed9
  --tmpParamsStc.polarity_rx_packed[9] = params.polarity_rx_packed9
  --tmpParamsStc.polarity_tx_packed[9] = params.polarity_tx_packed9
  tmpParamsStc.fec_ability[9] = params.fec_ability10
  tmpParamsStc.fec_requested[9] = params.fec_requested10

  portParamsStcPtr.portType=1
  portParamsStcPtr.portParams = {}
  portParamsStcPtr.portParams.apPort = {}
  portParamsStcPtr.portParams.apPort.advertiseModesArr = {}
  portParamsStcPtr.portParams.apPort.advertiseModesArrSize = 0
  portParamsStcPtr.portParams.apPort.apPortAttributesArr = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArrSize = 0

  for advIdx=0,9 do

      portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx]={}
      portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeMandatoryParams={}

      -- mandatory params
      portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeMandatoryParams.ifMode = tmpParamsStc.ifMode[advIdx]
      portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeMandatoryParams.speed =  tmpParamsStc.speed[advIdx]

      --print("PARAMS1:",tmpParamsStc.ifMode[advIdx]," ", tmpParamsStc.speed[advIdx])
      --print("PARAMS2:",portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeMandatoryParams.ifMode,
      --      " ", portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeMandatoryParams.speed)

      if tmpParamsStc.ifMode[advIdx] ~= nil then
          portParamsStcPtr.portParams.apPort.advertiseModesArrSize = portParamsStcPtr.portParams.apPort.advertiseModesArrSize + 1
      end

      portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr={}
      portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArrSize = 2

      modeParamsIdx = 0
      laneIdx = 0

     -- if params.rx_bitmap_packed[advIdx] ~= nil then
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx] = {}
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param = {}
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams = {}
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.totalLanes = 0
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].paramType =
     --       stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")
     --     for bit=0,3 do
     --         if hasbit(params.rx_bitmap_packed[advIdx],bit) then
     --             -- increase total lanes
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.totalLanes =
     --               portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.totalLanes+1
     --             -- squelch
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx] = {}
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams.squelch =
     --               bitand(shiftRight((params.rx_squelch_packed[advIdx]),bit),255)
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].laneType = 1
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].lane = bit
     --             -- LB
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams.squelch =
     --               bitand(shiftRight((params.rx_LB_packed[advIdx]),bit),255)
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].laneType = 1
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].lane = bit
     --             -- HF
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams.squelch =
     --               bitand(shiftRight((params.rx_HF_packed[advIdx]),bit),255)
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].laneType = 1
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].lane = bit
     --             -- LF
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams.squelch =
     --               bitand(shiftRight((params.rx_LF_packed[advIdx]),bit),255)
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].laneType = 1
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].lane = bit
     --             -- dc gain
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].rxParams.squelch =
     --               bitand(shiftRight((params.rx_dcGain_packed[advIdx]),bit),255)
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].laneType = 1
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesRxParams.rxCfg[laneIdx].lane = bit
     --
     --             laneIdx = laneIdx + 1
     --          end
     --     end
     --     modeParamsIdx = modeParamsIdx + 1
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArrSize = portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArrSize + 1
     -- end
     --
     -- laneIdx = 0
     -- if params.tx_bitmap_packed ~= nil then
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx] = {}
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param = {}
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams = {}
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.totalLanes = 0
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].paramType =
     --       stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")
     --     for bit=0,3 do
     --         if hasbit(params.tx_bitmap_packed[advIdx],bit) then
     --             -- increase total lanes
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.totalLanes =
     --               portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.totalLanes+1
     --             -- tx_amp_packed
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx] = {}
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].txParams = {}
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].txParams.squelch =
     --               bitand(shiftRight((params.tx_amp_packed[advIdx]),bit),255)
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].laneType = 1
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].lane = bit
     --             -- tx_emph0_packed
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].txParams.emph0 =
     --               bitand(shiftRight((params.tx_emph0_packed[advIdx]),bit),255)
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].laneType = 1
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].lane = bit
     --             -- tx_emph1_packed
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].txParams.emph1 =
     --               bitand(shiftRight((params.tx_emph1_packed[advIdx]),bit),255)
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].laneType = 1
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesTxParams.txCfg[laneIdx].lane = bit
     --             laneIdx = laneIdx + 1
     --          end
     --     end
     --     modeParamsId= modeParamsIdx + 1
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArrSize = portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArrSize + 1
     -- end
     --
     -- laneIdx = 0
     -- if params.polarity_rx_packed[advIdx]  ~= nil then
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx] = {}
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param = {}
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesPolarity = {}
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesPolarity.totalLanes = 0
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].paramType =
     --       stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")
     --     for bit=0,3 do
     --         if hasbit(params.polarity_rx_packed[advIdx],bit) then
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesPolarity = {}
     --             -- increase total lanes
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesPolarity.totalLanes =
     --               portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesPolarity.totalLanes+1
     --             -- polarity_rx_packed
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg[laneIdx] = {}
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg[laneIdx].rxPolarity =
     --               bitand(shiftRight((params.polarity_rx_packed[advIdx]),bit),1)
     --             -- polarity_tx_packed
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg[laneIdx].txPolarity =
     --               bitand(shiftRight((params.polarity_tx_packed[advIdx]),bit),1)
     --
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg[laneIdx].laneType = 1
     --             portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[modeParamsIdx].param.serdesPolarity.polarityCfg[laneIdx].lane = bit
     --
     --             laneIdx = laneIdx + 1
     --          end
     --     end
     --     modeParamsIdx = modeParamsIdx + 1
     --     portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArrSize = portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArrSize + 1
     -- end

     -- if tmpParamsStc.fec_ability[advIdx]  ~= nil then
          portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[0] = {}
          portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[0].paramType =
            stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_ABILITY_E")
          portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[0].param = {}
          portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[0].param.fecAbility = fecToVal(tmpParamsStc.fec_ability[advIdx])
    --  end

    --  if tmpParamsStc.fec_requested[advIdx]  ~= nil then
          portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[1] = {}
          portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[1].paramType =
            stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_REQUESTED_E")
          portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[1].param = {}
          portParamsStcPtr.portParams.apPort.advertiseModesArr[advIdx].modeParamsArr[1].param.fecRequested = fecToVal(tmpParamsStc.fec_requested[advIdx])
    -- end
  end

  attrIdx = 0

  portParamsStcPtr.portParams.apPort.apPortAttributesArrSize = portParamsStcPtr.portParams.apPort.apPortAttributesArrSize + 1
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx] = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attributeType =
    stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FEC_REQUIRED_E")
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute = {}
  if params.fecRequired == CPSS_PORT_FEC_MODE_ENABLED_E  then
      portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute.fecRequired = true
  else
      portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute.fecRequired = false
  end
  attrIdx = attrIdx + 1


  portParamsStcPtr.portParams.apPort.apPortAttributesArrSize = portParamsStcPtr.portParams.apPort.apPortAttributesArrSize + 1
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx] = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attributeType =
    stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FEC_SUPPORTED_E")
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute = {}
  if params.fecSupported == CPSS_PORT_FEC_MODE_ENABLED_E then
      portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute.fecSupported = true
  else
      portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute.fecSupported = false
  end
  attrIdx = attrIdx + 1


  portParamsStcPtr.portParams.apPort.apPortAttributesArrSize = portParamsStcPtr.portParams.apPort.apPortAttributesArrSize + 1
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx] = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attributeType =
    stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_NONCE_DISABLED_E")
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute.nonceDisable = params.nonceDisable
  --print(" nonce ",params.nonceDisable)
  attrIdx = attrIdx + 1


  portParamsStcPtr.portParams.apPort.apPortAttributesArrSize = portParamsStcPtr.portParams.apPort.apPortAttributesArrSize + 1
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx] = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attributeType =
    stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FC_PAUSE_E")
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute.fcPause = params.fcPause
  --print(" fcPause ",params.fcPause)
  attrIdx = attrIdx + 1


  portParamsStcPtr.portParams.apPort.apPortAttributesArrSize = portParamsStcPtr.portParams.apPort.apPortAttributesArrSize + 1
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx] = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attributeType =
    stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FC_ASM_DIR_E")
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute.fcAsmDir = fcToVal(params.fcAsmDir)
  --print(" fcAsmDir ",params.fcAsmDir)
  attrIdx = attrIdx + 1


  portParamsStcPtr.portParams.apPort.apPortAttributesArrSize = portParamsStcPtr.portParams.apPort.apPortAttributesArrSize + 1
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx] = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attributeType =
    stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_LANE_NUM_E")
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute = {}
  portParamsStcPtr.portParams.apPort.apPortAttributesArr[attrIdx].attribute.laneNum = params.laneNum
  --print(" laneNum ",params.laneNum)
  attrIdx = attrIdx + 1


  local iterator
  for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
    command_data:clearPortStatus()
    command_data:clearLocalStatus()
    if(nil~=devNum) then
      ret = myGenWrapper("prvCpssPortManagerPortParamsSet_L",{
      {"IN","GT_U8","devNum",devNum},
      {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
      {"IN","PRV_CPSS_PORT_MANAGER_GENERIC_PARAMS_STC_L","portParamsStcPtr",portParamsStcPtr}})
      if(ret~=GT_OK) then
        print("Error at command")
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
      end
    end
  end

  -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


local function port_manager_event_set_func(params)
  local command_data = Command_Data()
  local devNum, portNum, ret, enable
  local GT_OK=0
  local apParamsPtr={}
  local fecA
  local fecR
  local portEventStcPtr={}

  portEventStcPtr.portEvent = stringToEnum_event(params.event)

  local iterator
  for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
    command_data:clearPortStatus()
    command_data:clearLocalStatus()
    if(nil~=devNum) then
      ret = myGenWrapper("cpssDxChPortManagerEventSet",{
      {"IN","GT_U8","devNum",devNum},
      {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
      {"IN","CPSS_PORT_MANAGER_STC","portEventStcPtr",portEventStcPtr}})
      if(ret~=GT_OK) then
        print("Error at command")
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
      end
    end
  end

  -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


local function statusShowFunc(params)
  local command_data = Command_Data()
  local devNum, portNum, ret, enable
  local GT_OK=0
  local portStagePtr={}
  local localStatusPtr={}
  local flagPrintHeaderOnce = 0

  local iterator
  for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
    command_data:clearPortStatus()
    command_data:clearLocalStatus()
    if(nil~=devNum) then
      ret, localStatusPtr = myGenWrapper("cpssDxChPortManagerStatusGet",{
      {"IN","GT_U8","devNum",devNum},
      {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
      {"OUT","CPSS_PORT_MANAGER_STATUS_STC","portStagePtr"}})
      if(ret~=GT_OK) then
        print("Port ", portNum, "Error at command")
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
      end

      if flagPrintHeaderOnce == 0 then
        --print("                   state          oper        failure", "\n")
        print(string.format("%-11s%-40s%-7s%-7s \n", "dev/port","state","oper","failure"))
        flagPrintHeaderOnce = 1
      end

      --  print(localStatusPtr["portStagePtr"].portState,"  ",
      --          localStatusPtr["portStagePtr"].portUnderOperDisable ,"  ",
      --              localStatusPtr["portStagePtr"].failure,"\n")
      print(string.format("%-11s%-40s%-7s%-7s \n", tostring(devNum).."/"..tostring(portNum),
                                  alignLeftToCenterStr(localStatusPtr["portStagePtr"].portState,
                                                       16),
                                  alignLeftToCenterStr(tostring(localStatusPtr["portStagePtr"].portUnderOperDisable), 5),
                                  alignLeftToCenterStr(localStatusPtr["portStagePtr"].failure, 5)))
    end
  end

  -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


local function parametersShowFunc(params)
  local command_data = Command_Data()
  local devNum, portNum, ret, enable, index1, index2, laneIndex
  local GT_OK=0
  local portStagePtr={}
  local localParametersPtr={}
  local localParametersPtr2={}
  local portParamsStcPtr={}
  local portParamsStcPtr2={}
  local zzz = {}


  local iterator
  for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
    command_data:clearPortStatus()
    command_data:clearLocalStatus()
    if(nil~=devNum) then

      ret , zzz = myGenWrapper("prvCpssPortManagerPortParamsGet_L",{
      {"IN","GT_U8","devNum",devNum},
      {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
      {"INOUT","PRV_CPSS_PORT_MANAGER_GENERIC_PARAMS_STC_L","portParamsStcPtr", portParamsStcPtr2}})
      if(ret~=GT_OK) then
        print("Port ", portNum, "Error at command")
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
      end

      if zzz["portParamsStcPtr"].portType == "CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E" then

          -- port header
          print(string.format("%-11s%-10s%-15s%-10s%-15s%-20s%-22s%-18s%-10s \n", "dev/port","type","ifMode","speed", "fecAbility",
                              "trainMode","adaptiveRxSupported","edgeDetSupported","lbType"))

          -- port data
          print(string.format("%-11s%-10s%-15s%-10s%-15s%-20s%-22s%-18s%-10s \n", tostring(devNum).."/"..tostring(portNum),
                              portTypeStrGet(zzz["portParamsStcPtr"].portType),
                              interfaceStrGet(zzz["portParamsStcPtr"].portParams.port.portModeParams.modeMandatoryParams.ifMode),
                              speedStrGet(zzz["portParamsStcPtr"].portParams.port.portModeParams.modeMandatoryParams.speed),
                              fecStrGet(zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_ABILITY_E")].param.fecAbility),
                              tuneStrGet(zzz["portParamsStcPtr"].portParams.port.portAttributesArr[stringToEnum_portAttribute("CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_TUNE_MODE_E")].attribute.trainMode),
                              tostring(zzz["portParamsStcPtr"].portParams.port.portAttributesArr[stringToEnum_portAttribute("CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_ADAPTIVE_RX_TUNE_SUPPORTED_E")].attribute.adaptiveRxTrainSupported),
                              tostring(zzz["portParamsStcPtr"].portParams.port.portAttributesArr[stringToEnum_portAttribute("CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_EDGE_DETECT_SUPPORTED_E")].attribute.edgeDetectSupported),
                              loopbackStrGet(zzz["portParamsStcPtr"].portParams.port.portAttributesArr[stringToEnum_portAttribute("CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_LOOPBACK_E")].attribute.loopback.loopbackType)))

           -- lanes header
           print(string.format("%-10s%-10s%-12s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-15s%-12s%-10s \n", "lane","txAmp","txAmpAdjEn","emph0", "emph1","txAmpShft",
                              "sqlch","ffeRes","ffeCap","dcGain","bandWidth","loopBandwidth","txPolarity","rxPolarity"))
          --for index1=0,5 do
              for laneIndex=0,zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")].param.serdesPolarity.totalLanes-1 do
              -- lane data
              print(string.format("%-10s%-10s%-12s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-15s%-12s%-10s \n", laneIndex,
                    zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.txAmp,
                    tostring(zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpAdjEn),
                    zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.emph0,
                    zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.emph1,
                    tostring(zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpShft),
                    zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.sqlch,
                    zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeRes,
                    zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeCap,
                    zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.dcGain,
                    zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.bandWidth,
                    zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.loopBandwidth,
                    tostring(zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")].param.serdesPolarity.polarityCfg[laneIndex].txPolarity),
                    tostring(zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")].param.serdesPolarity.polarityCfg[laneIndex].rxPolarity)))
             end
             -- if zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].paramType ==
             --  "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E" then
             --      print("total lanes: ",zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesTxParams.totalLanes)
             --      for laneIndex=0,zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesTxParams.totalLanes-1 do
             --          print("txAmp: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesTxParams.txCfg[laneIndex].txParams.txAmp)
             --          print("txAmpAdjEn: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpAdjEn)
             --          print("emph0: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesTxParams.txCfg[laneIndex].txParams.emph0)
             --          print("emph1: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesTxParams.txCfg[laneIndex].txParams.emph1)
             --          print("txAmpShft: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpShft)
             --      end
             -- elseif zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].paramType ==
             --  "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E" then
             --      for laneIndex=0,zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesRxParams.totalLanes-1 do
             --          print("sqlch: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.sqlch)
             --          print("ffeRes: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeRes)
             --          print("ffeCap: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeCap)
             --          --print("align90: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.align90)
             --          print("dcGain: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.dcGain)
             --          print("bandWidth: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.bandWidth)
             --          print("loopBandwidth: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.loopBandwidth)
             --      end
             -- elseif zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].paramType ==
             --  "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E" then
             --      for laneIndex=0,zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesPolarity.totalLanes-1 do
             --          print("txPolarity: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesPolarity.polarityCfg[laneIndex].txPolarity)
             --          print("rxPolarity: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.serdesPolarity.polarityCfg[laneIndex].rxPolarity)
             --      end
             -- --elseif zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].paramType ==
             -- -- "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_ABILITY_E" then
             -- --     print("fecAbility: ", zzz["portParamsStcPtr"].portParams.port.portModeParams.modeParamsArr[index1].param.fecAbility, " ")
             -- --end
          --end

         -- for index1=0,3 do
         --     if zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attributeType ==
         --     "CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_TUNE_MODE_E" then
         --         print("trainMode: ",zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attribute.trainMode)
         --     end
         --
         --     if zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attributeType ==
         --     "CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_ADAPTIVE_RX_TUNE_SUPPORTED_E" then
         --         print("adaptiveRxTrainSupported: ",zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attribute.adaptiveRxTrainSupported)
         --     end
         --     if zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attributeType ==
         --     "CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_EDGE_DETECT_SUPPORTED_E" then
         --         print("edgeDetectSupported: ",zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attribute.edgeDetectSupported)
         --     end
         --     if zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attributeType ==
         --     "CPSS_PORT_MANAGER_ATTRIBUTE_TYPE_LOOPBACK_E" then
         --         print("loopbackType: ",zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attribute.loopback.loopbackType,
         --               " ")
         --         if zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attribute.loopback.loopbackType ==
         --         "CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E" then
         --             print(zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attribute.loopback.macLoopbackEnable)
         --         end
         --         if zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attribute.loopback.loopbackType ==
         --         "CPSS_PORT_MANAGER_LOOPBACK_TYPE_PCS_E" then
         --             print(zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attribute.loopback.pcsLoopbackMode)
         --         end
         --         if zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attribute.loopback.loopbackType ==
         --         "CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E" then
         --             print(zzz["portParamsStcPtr"].portParams.port.portAttributesArr[index1].attribute.loopback.serdesLoopbackMode)
         --         end
         --     end
         -- end
      else

          -- global ap port attributes header
          print(string.format("%-11s%-10s%-15s%-10s%-20s%-22s%-18s \n", "dev/port","type","fecRequired","fecSupported",
                              "fcPause", "fcAsmDir","laneNum"))

          -- global ap port attributes data header
          print(string.format("%-11s%-10s%-15s%-10s%-20s%-22s%-18s \n", tostring(devNum).."/"..tostring(portNum),
                              portTypeStrGet(zzz["portParamsStcPtr"].portType),
                              tostring(zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FEC_REQUIRED_E")].attribute.fecRequired),
                              tostring(zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FEC_SUPPORTED_E")].attribute.fecSupported),
                              tostring(zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FC_PAUSE_E")].attribute.fcPause),
                              fcToVal(zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FC_ASM_DIR_E")].attribute.fcAsmDir),
                              tostring(zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[stringToEnum_apPportAttribute("CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_LANE_NUM_E")].attribute.laneNum)))


          -- we will print all advertisement modes only if no resolution exist. if resolution exist, print only resolution
          if zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeMandatoryParams.speed == "CPSS_PORT_SPEED_NA_E"
          or  zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeMandatoryParams.speed == "CPSS_PORT_SPEED_10_E" then

              for index1=0,9 do
                  if zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeMandatoryParams.speed ~= "CPSS_PORT_SPEED_10_E"
                  and zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeMandatoryParams.speed ~= "CPSS_PORT_SPEED_NA_E" then
                      --print("ifMode[",index1,"]: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeMandatoryParams.ifMode)
                      --print("speed[",index1,"]: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeMandatoryParams.speed)

                      -- ifMode and speed for this advMode
                      print(string.format("%-10s%-10s%-10s  \n", "advIdx", "ifMode","speed"))

                      -- ifMode and speed for this advMode
                      print(string.format("%-10s%-10s%-10s \n", index1,
                            interfaceStrGet(zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeMandatoryParams.ifMode),
                            speedStrGet(zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeMandatoryParams.speed)))

                      -- lanes header
                      print(string.format("%-10s%-10s%-12s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-15s%-12s%-10s \n", "lane","txAmp","txAmpAdjEn","emph0", "emph1","txAmpShft",
                                         "sqlch","ffeRes","ffeCap","dcGain","bandWidth","loopBandwidth","txPolarity","rxPolarity"))
                      for laneIndex=0,zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")].param.serdesPolarity.totalLanes-1 do
                          -- lane data
                          print(string.format("%-10s%-10s%-12s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-15s%-12s%-10s \n", laneIndex,
                                zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.txAmp,
                                tostring(zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpAdjEn),
                                zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.emph0,
                                zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.emph1,
                                tostring(zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpShft),
                                zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.sqlch,
                                zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeRes,
                                zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeCap,
                                zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.dcGain,
                                zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.bandWidth,
                                zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.loopBandwidth,
                                tostring(zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")].param.serdesPolarity.polarityCfg[laneIndex].txPolarity),
                                tostring(zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")].param.serdesPolarity.polarityCfg[laneIndex].rxPolarity)))
                      end

                      --for index2=0,5 do
                      --    if zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].paramType ==
                      --     "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E" then
                      --         print("total lanes: ",zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesTxParams.totalLanes)
                      --         for laneIndex=0,zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesTxParams.totalLanes-1 do
                      --             print("txAmp: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesTxParams.txCfg[laneIndex].txParams.txAmp)
                      --             print("txAmpAdjEn: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpAdjEn)
                      --             print("emph0: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesTxParams.txCfg[laneIndex].txParams.emph0)
                      --             print("emph1: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesTxParams.txCfg[laneIndex].txParams.emph1)
                      --             print("txAmpShft: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpShft)
                      --         end
                      --    elseif zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].paramType ==
                      --     "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E" then
                      --         for laneIndex=0,zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesRxParams.totalLanes-1 do
                      --             print("sqlch: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesRxParams.rxCfg[laneIndex].rxParams.sqlch)
                      --             print("ffeRes: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeRes)
                      --             print("ffeCap: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeCap)
                      --             --print("align90: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesRxParams.rxCfg[laneIndex].rxParams.align90)
                      --             print("dcGain: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesRxParams.rxCfg[laneIndex].rxParams.dcGain)
                      --             print("bandWidth: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesRxParams.rxCfg[laneIndex].rxParams.bandWidth)
                      --             print("loopBandwidth: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesRxParams.rxCfg[laneIndex].rxParams.loopBandwidth)
                      --         end
                      --    elseif zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].paramType ==
                      --     "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E" then
                      --         for laneIndex=0,zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesPolarity.totalLanes-1 do
                      --             print("txPolarity: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesPolarity.polarityCfg[laneIndex].txPolarity)
                      --             print("rxPolarity: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.serdesPolarity.polarityCfg[laneIndex].rxPolarity)
                      --         end
                      --    elseif zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].paramType ==
                      --     "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_ABILITY_E" then
                      --         print("fecAbility: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.fecAbility, " ")
                      --    elseif zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].paramType ==
                      --     "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_REQUESTED_E" or zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].paramType ==
                      --     "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_NON_AP_PORT_LAST_E" then
                      --         print("fecAbility: ", zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].param.fecRequested, " ")
                      --    --else
                      --    --    print( zzz["portParamsStcPtr"].portParams.apPort.advertiseModesArr[index1].modeParamsArr[index2].paramType, " ")
                      --    end
                      --end
                  end
              end
          end

          --for index1=0,5 do
          --    if zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attributeType ==
          --    "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FEC_REQUIRED_E" then
          --        print("fecRequired: ",zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attribute.fecRequired)
          --    end
          --
          --    if zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attributeType ==
          --    "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FEC_SUPPORTED_E" then
          --        print("fecSupported: ",zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attribute.fecSupported)
          --    end
          --    if zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attributeType ==
          --    "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_NONCE_DISABLED_E" then
          --        print("nonceDisable: ",zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attribute.nonceDisable)
          --    end
          --    if zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attributeType ==
          --    "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FC_PAUSE_E" then
          --        print("fcPause: ",zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attribute.fcPause)
          --    end
          --    if zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attributeType ==
          --    "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_FC_ASM_DIR_E" then
          --        print("fcAsmDir: ",zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attribute.fcAsmDir)
          --    end
          --    if zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attributeType ==
          --    "CPSS_PORT_MANAGER_AP_ATTRIBUTE_TYPE_LANE_NUM_E" then
          --        print("laneNum: ",zzz["portParamsStcPtr"].portParams.apPort.apPortAttributesArr[index1].attribute.laneNum)
          --    end
          --end

          if zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeMandatoryParams.speed ~= "CPSS_PORT_SPEED_NA_E"
          and  zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeMandatoryParams.speed ~= "CPSS_PORT_SPEED_10_E" then
              --print("resolution parameters: ")
              --print("ifMode: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeMandatoryParams.ifMode,
              --       "speed: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeMandatoryParams.speed)

              print("Resolution params: \n")

              -- ifMode and speed for this advMode
              print(string.format("%-10s%-10s  \n", "ifMode","speed"))

              -- ifMode and speed for this advMode
              print(string.format("%-10s%-10s \n",
                        interfaceStrGet(zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeMandatoryParams.ifMode),
                        speedStrGet(zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeMandatoryParams.speed)))

              -- lanes header
              print(string.format("%-10s%-10s%-12s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-15s%-12s%-10s \n", "lane","txAmp","txAmpAdjEn","emph0", "emph1","txAmpShft",
                        "sqlch","ffeRes","ffeCap","dcGain","bandWidth","loopBandwidth","txPolarity","rxPolarity"))

              for laneIndex=0,zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")].param.serdesPolarity.totalLanes-1 do
                  -- lane data
                  print(string.format("%-10s%-10s%-12s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-15s%-12s%-10s \n", laneIndex,
                        zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.txAmp,
                        tostring(zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpAdjEn),
                        zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.emph0,
                        zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.emph1,
                        tostring(zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E")].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpShft),
                        zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.sqlch,
                        zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeRes,
                        zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeCap,
                        zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.dcGain,
                        zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.bandWidth,
                        zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E")].param.serdesRxParams.rxCfg[laneIndex].rxParams.loopBandwidth,
                        tostring(zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")].param.serdesPolarity.polarityCfg[laneIndex].txPolarity),
                        tostring(zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[stringToEnum_portParams("CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E")].param.serdesPolarity.polarityCfg[laneIndex].rxPolarity)))
              end

             -- for index1=0,5 do
             --     if zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].paramType ==
             --      "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_TX_E" then
             --          print("total lanes: ",zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesTxParams.totalLanes)
             --          for laneIndex=0,zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesTxParams.totalLanes-1 do
             --              print("txAmp: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesTxParams.txCfg[laneIndex].txParams.txAmp)
             --              print("txAmpAdjEn: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpAdjEn)
             --              print("emph0: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesTxParams.txCfg[laneIndex].txParams.emph0)
             --              print("emph1: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesTxParams.txCfg[laneIndex].txParams.emph1)
             --              print("txAmpShft: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesTxParams.txCfg[laneIndex].txParams.txAmpShft)
             --          end
             --     elseif zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].paramType ==
             --      "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_RX_E" then
             --          for laneIndex=0,zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesRxParams.totalLanes-1 do
             --              print("sqlch: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.sqlch)
             --              print("ffeRes: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeRes)
             --              print("ffeCap: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.ffeCap)
             --              --print("align90: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.align90)
             --              print("dcGain: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.dcGain)
             --              print("bandWidth: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.bandWidth)
             --              print("loopBandwidth: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesRxParams.rxCfg[laneIndex].rxParams.loopBandwidth)
             --          end
             --     elseif zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].paramType ==
             --      "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_SERDES_POLARITY_E" then
             --          for laneIndex=0,zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesPolarity.totalLanes-1 do
             --              print("txPolarity: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesPolarity.polarityCfg[laneIndex].txPolarity)
             --              print("rxPolarity: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.serdesPolarity.polarityCfg[laneIndex].rxPolarity)
             --          end
             --     elseif zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].paramType ==
             --      "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_ABILITY_E" then
             --          print("fecAbility: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.fecAbility, " ")
             --     elseif zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].paramType ==
             --      "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_FEC_REQUESTED_E" or zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].paramType ==
             --      "CPSS_PORT_MANAGER_MODE_PARAM_TYPE_NON_AP_PORT_LAST_E" then
             --          print("fecAbility: ", zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].param.fecRequested, " ")
             --     --else
             --     --    print( zzz["portParamsStcPtr"].portParams.apPort.resolutionParamsPtr[0].modeParamsArr[index1].paramType, " ")
             --     end
             -- end
          end
      end

      -- print(string.format("%-11s%-40s%-7s%-7s \n", tostring(devNum).."/"..tostring(portNum),
      --                             alignLeftToCenterStr(localStatusPtr["portStagePtr"].portState,
      --                                                  16),
      --                             alignLeftToCenterStr(tostring(localStatusPtr["portStagePtr"].portUnderOperDisable), 5),
      --                             alignLeftToCenterStr(localStatusPtr["portStagePtr"].failure, 5)))

    end
  end

  -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

--------------------------------------------
-- command registration: ap-port
--------------------------------------------
CLI_addCommand("interface", "port-manager parameters-set regular-port", {
  func   = port_manager_parameters_set_func,
  help   = "Set port manager parameters",
  params = {
  {   type="named", { format="ifMode %port_interface_mode", name="ifMode"},
            { format="speed %port_speed", name="speed"},

            mandatory = {"speed","ifMode"},

            --8 bits per serdes [0:7] - sd0 [8:15] - sd1...
            { format="rx_squelch_packed %GT_U32", name="rx_squelch_packed", help="8bit value per serdes (hence maximum of 4 lane configuration is applicable)"},
            { format="rx_dcGain_packed %GT_U32", name="rx_dcGain_packed"},
            { format="rx_LB_packed %GT_U32", name="rx_LB_packed"},
            { format="rx_HF_packed %GT_U32", name="rx_HF_packed"},
            { format="rx_LF_packed %GT_U32", name="rx_LF_packed"},
            --which serdeses to take values from the packed values 0x3 means take values for sd0 and sd1. value 0x5 means take values of sd0 and sd2...
            -- it is needed in order to distinguish whether values of 0 in the packed values is the value to use or just unsued serdes value
            { format="rx_bitmap_packed %GT_U32", name="rx_bitmap_packed", help="which serdeses [8bit values] to use from packed values"},

            --8 bits per serdes [0:7] - sd0 [8:15] - sd1...
            { format="tx_amp_packed %GT_U32", name="tx_amp_packed", help="8bit value per serdes (hence maximum of 4 lane configuration is applicable)"},
            { format="tx_emph0_packed %GT_U32", name="tx_emph0_packed"},
            { format="tx_emph1_packed %GT_U32", name="tx_emph1_packed"},
            --which serdeses to take values from the packed values 0x3 means take values for sd0 and sd1. value 0x5 means take values of sd0 and sd2...
            -- it is needed in order to distinguish whether values of 0 in the packed values is the value to use or just unsued serdes value
            { format="tx_bitmap_packed %GT_U32", name="tx_bitmap_packed", help="which serdeses [8bit values] to use from packed values"},

            -- 1 bits per serdes [0] - sd0 [1] - sd1...
            { format="polarity_rx_packed %GT_U32", name="polarity_rx_packed", help="1bit value [true or false] per serdes "},
            { format="polarity_tx_packed %GT_U32", name="polarity_tx_packed"},

            { format="fec_ability %ap_port_fec_mode", name="fec_ability"},

            { format="tune_mode %auto_tune", name="tune_mode"},
            { format="adaptive_rx_supported %bool", name="adaptive_rx_supported"},
            { format="edge_detect_supported %bool", name="edge_detect_supported"}

         }}}
)

CLI_addCommand("interface", "port-manager event-set", {
  func   = port_manager_event_set_func,
  help   = "Set port manager event",
  params = {
  {   type="named", { format="event %port_manager_event", name="event"},

            mandatory = {"event"}
         }}}
)

CLI_addCommand("interface", "port-manager status-show",
  {
  func   = statusShowFunc,
  help   = "Show port-manager status",
  params = {}
  }
)

CLI_addCommand("interface", "port-manager parameters-show",
  {
  func   = parametersShowFunc,
  help   = "Show port-manager parameters",
  params = {}
  }
)

CLI_addCommand("interface", "port-manager parameters-set ap-port", {
  func   = port_manager_ap_port_parameters_set_func,
  help   = "Enable AP process on port",
  params = {
  {   type="named", { format="fc_pause %bool", name="fcPause"},
            { format="fc_asm_dir %ap_flow_control", name="fcAsmDir"},
            { format="fecSupported %bool", name="fecSupported"},
            { format="fecRequired %bool", name="fecRequired"},
            { format="lane_num %GT_U32", name="laneNum"},

            { format="mode1 %ap_port_interface_mode", name="mode1"},
            { format="speed1 %ap_port_speed", name="speed1"},

            { format="fec_ability1 %ap_port_fec_mode", name="fec_ability1"},
            { format="fec_requested1 %ap_port_fec_mode", name="fec_requested1"},

            mandatory = {"fcPause","fcAsmDir","fecSupported","fecRequired","laneNum","mode1","speed1", "fec_ability1", "fec_requested1"},

            { format="mode2 %ap_port_interface_mode", name="mode2"},
            { format="speed2 %ap_port_speed", name="speed2"},
          --{ format="rx_squelch_packed0 %GT_U32", name="rx_squelch_packed0"},
          --{ format="rx_dcGain_packed0 %GT_U32", name="rx_dcGain_packed0"},
          --{ format="rx_LB_packed0 %GT_U32", name="rx_LB_packed0"},
          --{ format="rx_HF_packed0 %GT_U32", name="rx_HF_packed0"},
          --{ format="rx_LF_packed0 %GT_U32", name="rx_LF_packed0"},
          --{ format="rx_bitmap_packed0 %GT_U32", name="rx_bitmap_packed0"},
          --{ format="tx_amp_packed0 %GT_U32", name="tx_amp_packed0"},
          --{ format="tx_emph0_packed0 %GT_U32", name="tx_emph0_packed0"},
          --{ format="tx_emph1_packed0 %GT_U32", name="tx_emph1_packed0"},
          --{ format="tx_bitmap_packed0 %GT_U32", name="tx_bitmap_packed0"},
          --{ format="polarity_rx_packed0 %GT_U32", name="polarity_rx_packed0"},
          --{ format="polarity_tx_packed0 %GT_U32", name="polarity_tx_packed0"},
            { format="fec_ability2 %ap_port_fec_mode", name="fec_ability2"},
            { format="fec_requested2 %ap_port_fec_mode", name="fec_requested2"},

            { format="mode3 %ap_port_interface_mode", name="mode3"},
            { format="speed3 %ap_port_speed", name="speed3"},
           -- { format="rx_squelch_packed1 %GT_U32", name="rx_squelch_packed1"},
           -- { format="rx_dcGain_packed1 %GT_U32", name="rx_dcGain_packed1"},
           -- { format="rx_LB_packed1 %GT_U32", name="rx_LB_packed1"},
           -- { format="rx_HF_packed1 %GT_U32", name="rx_HF_packed1"},
           -- { format="rx_LF_packed1 %GT_U32", name="rx_LF_packed1"},
           -- { format="rx_bitmap_packed1 %GT_U32", name="rx_bitmap_packed1"},
           -- { format="tx_amp_packed1 %GT_U32", name="tx_amp_packed1"},
           -- { format="tx_emph0_packed1 %GT_U32", name="tx_emph0_packed1"},
           -- { format="tx_emph1_packed1 %GT_U32", name="tx_emph1_packed1"},
           -- { format="tx_bitmap_packed1 %GT_U32", name="tx_bitmap_packed1"},
           -- { format="polarity_rx_packed1 %GT_U32", name="polarity_rx_packed1"},
           -- { format="polarity_tx_packed1 %GT_U32", name="polarity_tx_packed1"},
            { format="fec_ability3 %ap_port_fec_mode", name="fec_ability3"},
            { format="fec_requested3 %ap_port_fec_mode", name="fec_requested3"},

            { format="mode4 %ap_port_interface_mode", name="mode4"},
            { format="speed4 %ap_port_speed", name="speed4"},
          --  { format="rx_squelch_packed2 %GT_U32", name="rx_squelch_packed2"},
          --  { format="rx_dcGain_packed2 %GT_U32", name="rx_dcGain_packed2"},
          --  { format="rx_LB_packed2 %GT_U32", name="rx_LB_packed2"},
          --  { format="rx_HF_packed2 %GT_U32", name="rx_HF_packed2"},
          --  { format="rx_LF_packed2 %GT_U32", name="rx_LF_packed2"},
          --  { format="rx_bitmap_packed2 %GT_U32", name="rx_bitmap_packed2"},
          --  { format="tx_amp_packed2 %GT_U32", name="tx_amp_packed2"},
          --  { format="tx_emph0_packed2 %GT_U32", name="tx_emph0_packed2"},
          --  { format="tx_emph1_packed2 %GT_U32", name="tx_emph1_packed2"},
          --  { format="tx_bitmap_packed2 %GT_U32", name="tx_bitmap_packed2"},
          --  { format="polarity_rx_packed2 %GT_U32", name="polarity_rx_packed2"},
          --  { format="polarity_tx_packed2 %GT_U32", name="polarity_tx_packed2"},
            { format="fec_ability4 %ap_port_fec_mode", name="fec_ability4"},
            { format="fec_requested4 %ap_port_fec_mode", name="fec_requested4"},

            { format="mode5 %ap_port_interface_mode", name="mode5"},
            { format="speed5 %ap_port_speed", name="speed5"},
          --  { format="rx_squelch_packed3 %GT_U32", name="rx_squelch_packed3"},
          --  { format="rx_dcGain_packed3 %GT_U32", name="rx_dcGain_packed3"},
          --  { format="rx_LB_packed3 %GT_U32", name="rx_LB_packed3"},
          --  { format="rx_HF_packed3 %GT_U32", name="rx_HF_packed3"},
          --  { format="rx_LF_packed3 %GT_U32", name="rx_LF_packed3"},
          --  { format="rx_bitmap_packed3 %GT_U32", name="rx_bitmap_packed3"},
          --  { format="tx_amp_packed3 %GT_U32", name="tx_amp_packed3"},
          --  { format="tx_emph0_packed3 %GT_U32", name="tx_emph0_packed3"},
          --  { format="tx_emph1_packed3 %GT_U32", name="tx_emph1_packed3"},
          --  { format="tx_bitmap_packed3 %GT_U32", name="tx_bitmap_packed3"},
          --  { format="polarity_rx_packed3 %GT_U32", name="polarity_rx_packed3"},
          --  { format="polarity_tx_packed3 %GT_U32", name="polarity_tx_packed3"},
            { format="fec_ability5 %ap_port_fec_mode", name="fec_ability5"},
            { format="fec_requested5 %ap_port_fec_mode", name="fec_requested5"},

            { format="mode6 %ap_port_interface_mode", name="mode6"},
            { format="speed6 %ap_port_speed", name="speed6"},
          --  { format="rx_squelch_packed4 %GT_U32", name="rx_squelch_packed4"},
          --  { format="rx_dcGain_packed4 %GT_U32", name="rx_dcGain_packed4"},
          --  { format="rx_LB_packed4 %GT_U32", name="rx_LB_packed4"},
          --  { format="rx_HF_packed4 %GT_U32", name="rx_HF_packed4"},
          --  { format="rx_LF_packed4 %GT_U32", name="rx_LF_packed4"},
          --  { format="rx_bitmap_packed4 %GT_U32", name="rx_bitmap_packed4"},
          --  { format="tx_amp_packed4 %GT_U32", name="tx_amp_packed4"},
          --  { format="tx_emph0_packed4 %GT_U32", name="tx_emph0_packed4"},
          --  { format="tx_emph1_packed4 %GT_U32", name="tx_emph1_packed4"},
          --  { format="tx_bitmap_packed4 %GT_U32", name="tx_bitmap_packed4"},
          --  { format="polarity_rx_packed4 %GT_U32", name="polarity_rx_packed4"},
          --  { format="polarity_tx_packed4 %GT_U32", name="polarity_tx_packed4"},
            { format="fec_ability6 %ap_port_fec_mode", name="fec_ability6"},
            { format="fec_requested6 %ap_port_fec_mode", name="fec_requested6"},

            { format="mode7 %ap_port_interface_mode", name="mode7"},
            { format="speed7 %ap_port_speed", name="speed7"},
          --  { format="rx_squelch_packed5 %GT_U32", name="rx_squelch_packed5"},
          --  { format="rx_dcGain_packed5 %GT_U32", name="rx_dcGain_packed5"},
          --  { format="rx_LB_packed5 %GT_U32", name="rx_LB_packed5"},
          --  { format="rx_HF_packed5 %GT_U32", name="rx_HF_packed5"},
          --  { format="rx_LF_packed5 %GT_U32", name="rx_LF_packed5"},
          --  { format="rx_bitmap_packed5 %GT_U32", name="rx_bitmap_packed5"},
          --  { format="tx_amp_packed5 %GT_U32", name="tx_amp_packed5"},
          --  { format="tx_emph0_packed5 %GT_U32", name="tx_emph0_packed5"},
          --  { format="tx_emph1_packed5 %GT_U32", name="tx_emph1_packed5"},
          --  { format="tx_bitmap_packed5 %GT_U32", name="tx_bitmap_packed5"},
          --  { format="polarity_rx_packed5 %GT_U32", name="polarity_rx_packed5"},
          --  { format="polarity_tx_packed5 %GT_U32", name="polarity_tx_packed5"},
            { format="fec_ability7 %ap_port_fec_mode", name="fec_ability7"},
            { format="fec_requested7 %ap_port_fec_mode", name="fec_requested7"},

            { format="mode8 %ap_port_interface_mode", name="mode8"},
            { format="speed8 %ap_port_speed", name="speed8"},
          --  { format="rx_squelch_packed6 %GT_U32", name="rx_squelch_packed6"},
          --  { format="rx_dcGain_packed6 %GT_U32", name="rx_dcGain_packed6"},
          --  { format="rx_LB_packed6 %GT_U32", name="rx_LB_packed6"},
          --  { format="rx_HF_packed6 %GT_U32", name="rx_HF_packed6"},
          --  { format="rx_LF_packed6 %GT_U32", name="rx_LF_packed6"},
          --  { format="rx_bitmap_packed6 %GT_U32", name="rx_bitmap_packed6"},
          --  { format="tx_amp_packed6 %GT_U32", name="tx_amp_packed6"},
          --  { format="tx_emph0_packed6 %GT_U32", name="tx_emph0_packed6"},
          --  { format="tx_emph1_packed6 %GT_U32", name="tx_emph1_packed6"},
          --  { format="tx_bitmap_packed6 %GT_U32", name="tx_bitmap_packed6"},
          --  { format="polarity_rx_packed6 %GT_U32", name="polarity_rx_packed6"},
          --  { format="polarity_tx_packed6 %GT_U32", name="polarity_tx_packed6"},
            { format="fec_ability8 %ap_port_fec_mode", name="fec_ability8"},
            { format="fec_requested8 %ap_port_fec_mode", name="fec_requested8"},

            { format="mode9 %ap_port_interface_mode", name="mode9"},
            { format="speed9 %ap_port_speed", name="speed9"},
          --  { format="rx_squelch_packed7 %GT_U32", name="rx_squelch_packed7"},
          --  { format="rx_dcGain_packed7 %GT_U32", name="rx_dcGain_packed7"},
          --  { format="rx_LB_packed7 %GT_U32", name="rx_LB_packed7"},
          --  { format="rx_HF_packed7 %GT_U32", name="rx_HF_packed7"},
          --  { format="rx_LF_packed7 %GT_U32", name="rx_LF_packed7"},
          --  { format="rx_bitmap_packed7 %GT_U32", name="rx_bitmap_packed7"},
          --  { format="tx_amp_packed7 %GT_U32", name="tx_amp_packed7"},
          --  { format="tx_emph0_packed7 %GT_U32", name="tx_emph0_packed7"},
          --  { format="tx_emph1_packed7 %GT_U32", name="tx_emph1_packed7"},
          --  { format="tx_bitmap_packed7 %GT_U32", name="tx_bitmap_packed7"},
          --  { format="polarity_rx_packed7 %GT_U32", name="polarity_rx_packed7"},
          --  { format="polarity_tx_packed7 %GT_U32", name="polarity_tx_packed7"},
            { format="fec_ability9 %ap_port_fec_mode", name="fec_ability9"},
            { format="fec_requested9 %ap_port_fec_mode", name="fec_requested9"},

            { format="mode10 %ap_port_interface_mode", name="mode10"},
            { format="speed10 %ap_port_speed", name="speed10"},
          --  { format="rx_squelch_packed7 %GT_U32", name="rx_squelch_packed7"},
          --  { format="rx_dcGain_packed7 %GT_U32", name="rx_dcGain_packed7"},
          --  { format="rx_LB_packed7 %GT_U32", name="rx_LB_packed7"},
          --  { format="rx_HF_packed7 %GT_U32", name="rx_HF_packed7"},
          --  { format="rx_LF_packed7 %GT_U32", name="rx_LF_packed7"},
          --  { format="rx_bitmap_packed7 %GT_U32", name="rx_bitmap_packed7"},
          --  { format="tx_amp_packed7 %GT_U32", name="tx_amp_packed7"},
          --  { format="tx_emph0_packed7 %GT_U32", name="tx_emph0_packed7"},
          --  { format="tx_emph1_packed7 %GT_U32", name="tx_emph1_packed7"},
          --  { format="tx_bitmap_packed7 %GT_U32", name="tx_bitmap_packed7"},
          --  { format="polarity_rx_packed7 %GT_U32", name="polarity_rx_packed7"},
          --  { format="polarity_tx_packed7 %GT_U32", name="polarity_tx_packed7"},
            { format="fec_ability10 %ap_port_fec_mode", name="fec_ability10"},
            { format="fec_requested10 %ap_port_fec_mode", name="fec_requested10"},

            requirements={
            ["speed1"] = {"mode1"},
            ["fec_ability1"] = {"speed1"},
            ["fec_requested1"] = {"fec_ability1"},
            ["mode2"] = {"fec_requested1"},
            ["speed2"] = {"mode2"},
            ["fec_ability2"] = {"speed2"},
            ["fec_requested2"] = {"fec_ability2"},

            ["mode3"] = {"fec_requested2"},
            ["speed3"] = {"mode3"},
            ["fec_ability3"] = {"speed3"},
            ["fec_requested3"] = {"fec_ability3"},

            ["mode4"] = {"fec_requested3"},
            ["speed4"] = {"mode4"},
            ["fec_ability4"] = {"speed4"},
            ["fec_requested4"] = {"fec_ability4"},

            ["mode5"] = {"fec_requested4"},
            ["speed5"] = {"mode5"},
            ["fec_ability5"] = {"speed5"},
            ["fec_requested5"] = {"fec_ability5"},

            ["mode6"] = {"fec_requested5"},
            ["speed6"] = {"mode6"},
            ["fec_ability6"] = {"speed6"},
            ["fec_requested6"] = {"fec_ability6"},

            ["mode7"] = {"fec_requested6"},
            ["speed7"] = {"mode7"},
            ["fec_ability7"] = {"speed7"},
            ["fec_requested7"] = {"fec_ability7"},

            ["mode8"] = {"fec_requested9"},
            ["speed8"] = {"mode8"},
            ["fec_ability8"] = {"speed8"},
            ["fec_requested8"] = {"fec_ability8"},

            ["mode9"] = {"fec_requested8"},
            ["speed9"] = {"mode9"},
            ["fec_ability9"] = {"speed9"},
            ["fec_requested9"] = {"fec_ability9"},

            ["mode10"] = {"fec_requested9"},
            ["speed10"] = {"mode10"},
            ["fec_ability10"] = {"speed10"},
            ["fec_requested10"] = {"fec_ability10"}

            }
         }}}
)


