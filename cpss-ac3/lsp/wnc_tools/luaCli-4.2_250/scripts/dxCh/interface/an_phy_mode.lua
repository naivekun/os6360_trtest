--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* an_phy_mode.lua
--*
--* DESCRIPTION:
--*       enabling transmission of periodic fc messages
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlDxChPortModeSpeedSet")


-- ************************************************************************
---
--  an_phy_mode_func
--        @description  configure Auto-Negotiation Phy Mode when there is no PHY
--                      between link partners
--
--        @usage __global       - __global["ifRange"]: iterface range 
--
--        @return       true on success, otherwise false and error message
--

local function an_phy_mode_func(params)
    local all_ports = getGlobal("ifRange")
    local devNum, portNum
    local result, values
    local portNums
    local portDuplexIndex
    local portSpeedIndex
    local portInterface = "CPSS_PORT_INTERFACE_MODE_SGMII_E"
    local isactivate 
    local portSpeed
    
    if nil == params.flagNo then
        portSpeed = params.port_an.e
        portSpeedIndex = params.port_an.speedIndex
        portDuplexIndex = params.port_an.duplexIndex
    end

    for devNum, portNums in pairs(all_ports) do
        for portIndex = 1, #portNums do    
            portNum = portNums[portIndex]
            if is_sip_5(devNum) then
                if nil == params.flagNo then
                    local speed_mode_ = "CPSS_PORT_INTERFACE_MODE_SGMII_E"
                    result, values = wrLogWrapper("wrlDxChPortModeSpeedSet",
                          "(devNum, portNum, true, speed_mode_, portSpeed)", 
                            devNum, portNum, true, speed_mode_, portSpeed)
                    
              if  0x10 == result then
                print("It is not allowed to set speed ", portSpeed," and interface mode CPSS_PORT_INTERFACE_MODE_SGMII_E",
                            "for device ", devNum, "and port ", portNum)
                        return false
              elseif 0 ~= result then
                print("Error for setting speed ", portSpeed," and interface mode CPSS_PORT_INTERFACE_MODE_SGMII_E",
                            "for device ", devNum, "and port ", portNum)
                        return false
              end

                    result, values = myGenWrapper("cpssDxChPortAutoNegMasterModeEnableSet",
                                              {{ "IN", "GT_U8", "devNum", devNum},
                                               { "IN", "GT_U32", "portNum", portNum},
                                               { "IN", "GT_BOOL", "enable", true}})                                            
               if 0 ~= result then 
                        print("Error at setting auto negotiation master mode enable \n" ..
                                          " on device: ", devNum, "on port: ", portNum, "return code: ", result)
                        return false
                    end
                    local portAnAdvertismentPtr = {}
                    portAnAdvertismentPtr.link = true
                    portAnAdvertismentPtr.speed = portSpeedIndex
                    portAnAdvertismentPtr.duplex = portDuplexIndex
                    result, values = myGenWrapper("cpssDxChPortAutoNegAdvertismentConfigSet",
                                                 {{ "IN", "GT_U8", "devNum", devNum},
                                                  { "IN", "GT_U32", "portNum", portNum},
                                                  { "IN", "CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC", "portAnAdvertismentPtr", portAnAdvertismentPtr}})                                            
                    if 0 ~= result then 
                        print("Error at setting auto negotiation advertisment configuration \n" ..
                                              " on device: ", devNum, "on port: ", portNum, "return code: ", result)
                        return false
                    end
                    result, values = myGenWrapper("cpssDxChPortInBandAutoNegBypassEnableSet",
                                                 {{ "IN", "GT_U8", "devNum", devNum},
                                                  { "IN", "GT_U32", "portNum", portNum},
                                                  { "IN", "GT_BOOL", "enable", true}})                                            
                    if 0 ~= result then 
                         print("Error at setting in band auto negotiation by pass enable \n" ..
                                               " on device: ", devNum, "on port: ", portNum, "return code: ", result)
                         return false
                    end
                    result, values = myGenWrapper("cpssDxChPortInbandAutoNegEnableSet",
                                                 {{ "IN", "GT_U8", "devNum", devNum},
                                                  { "IN", "GT_U32", "portNum", portNum},
                                                  { "IN", "GT_BOOL", "enable", true}})                                            
                    if 0 ~= result then 
                         print("Error at setting in band auto negotiation enable \n" ..
                                               " on device: ", devNum, "on port: ", portNum, "return code: ", result)
                        return false
                    end
                    result, values = myGenWrapper("cpssDxChPortInbandAutoNegRestart",
                                                 {{ "IN", "GT_U8", "devNum", devNum},
                                                  { "IN", "GT_U32", "portNum", portNum}})
                    if 0 ~= result then 
                         print("Error at in band auto negotiation restart \n" ..
                                               " on device: ", devNum, "on port: ", portNum, "return code: ", result)
                         return false
                    end 
                else
                    result, values = myGenWrapper("cpssDxChPortInbandAutoNegEnableSet",
                                                 {{ "IN", "GT_U8", "devNum", devNum},
                                                  { "IN", "GT_U32", "portNum", portNum},
                                                  { "IN", "GT_BOOL", "enable", false}})                                            
                    if 0 ~= result then 
                         print("Error at setting in band auto negotiation disable \n" ..
                                               " on device: ", devNum, "on port: ", portNum, "return code: ", result)
                        return false
                    end
                    local portAnAdvertismentPtr = {}
                    portAnAdvertismentPtr.link = true
                    portAnAdvertismentPtr.speed = 2
                    portAnAdvertismentPtr.duplex = 0
                    result, values = myGenWrapper("cpssDxChPortAutoNegAdvertismentConfigSet",
                                                 {{ "IN", "GT_U8", "devNum", devNum},
                                                  { "IN", "GT_U32", "portNum", portNum},
                                                  { "IN", "CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC", "portAnAdvertismentPtr", portAnAdvertismentPtr}})                                            
                    if 0 ~= result then 
                        print("Error at setting default auto negotiation advertisment configuration \n" ..
                                              " on device: ", devNum, "on port: ", portNum, "return code: ", result)
                        return false
                    end
                    result, values = myGenWrapper("cpssDxChPortAutoNegMasterModeEnableSet",
                                              {{ "IN", "GT_U8", "devNum", devNum},
                                               { "IN", "GT_U32", "portNum", portNum},
                                               { "IN", "GT_BOOL", "enable", false}})                                            
               if 0 ~= result then 
                        print("Error at setting auto negotiation master mode disable \n" ..
                                          " on device: ", devNum, "on port: ", portNum, "return code: ", result)
                        return false
                    end
               end
      else
                print("Family of device ", devNum, " does not supported.")
                return false
            end
        end
    end           
    return true
end

CLI_type_dict["sgmii_speed"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "SGMII port speed",
    enum = {
        ["10h"] =    { value={ e="CPSS_PORT_SPEED_10_E", speedIndex=0, duplexIndex=1 },
                                        help="Advertise 10 Mbps Half duplex" },
        ["10f"] =    { value={ e="CPSS_PORT_SPEED_10_E", speedIndex=0, duplexIndex=0 },
                                        help="Advertise 10 Mbps Full duplex" },
        ["100h"] =    { value={ e="CPSS_PORT_SPEED_100_E", speedIndex=1, duplexIndex=1 },
                                        help="Advertise 100 Mbps Half duplex" },
        ["100f"] =    { value={ e="CPSS_PORT_SPEED_100_E", speedIndex=1, duplexIndex=0 },
                                        help="Advertise 100 Mbps Full duplex" },
        ["1000h"] =    { value={ e="CPSS_PORT_SPEED_1000_E", speedIndex=2, duplexIndex=1 },
                                        help="Advertise 1000 Mbps Half duplex" },
        ["1000f"] =    { value={ e="CPSS_PORT_SPEED_1000_E", speedIndex=2, duplexIndex=0 },
                                        help="Advertise 1000 Mbps Full duplex" }
    }
} 

--------------------------------------------
-- command registration: an phy mode
--------------------------------------------
CLI_addHelp("interface", "auto-negotiation", "Configure auto negotiation phy mode")
CLI_addCommand("interface", "auto-negotiation phy-mode", {
  func=an_phy_mode_func,
  help="Configure auto negotiation phy mode",
  params={ { type="values",
      {format="%sgmii_speed", name="port_an"}
  } }
})

--------------------------------------------------
-- command registration: no an phy mode
--------------------------------------------------
CLI_addCommand("interface", "no auto-negotiation phy-mode", {
  func=function(params)
      params.flagNo=true
      return an_phy_mode_func(params)
  end,
  help="Disabling auto-negotiation phy mode"
})