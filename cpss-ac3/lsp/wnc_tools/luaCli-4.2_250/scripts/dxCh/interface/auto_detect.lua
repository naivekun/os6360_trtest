--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* auto_detect.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of the port's crc check
--*       auto-detecting of the port's actual mode and speed
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlDxChPortModeSpeedAutoDetectAndConfig")

-- ************************************************************************
---
--  auto_detect_mode_speed
--        @description  configures and get auto-dection of port and speed
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--
local function auto_detect_mode_speed(params)
  local all_ports, dev_ports
  local dev, k, port
  local rc, values
  local i
  local autos, entry, port_exists
  
    local mode_speed
    local GT_OK = 0
    local ad_array = {}
    local ifMode, speed
    
    autos = {}
    for i = 1, #params.autodetect do
        entry = {}
        mode_speed = params.autodetect[i]       
        -- configure interface and speed per entry
        if mode_speed == "XLG_40000"          then              -- XLG_40000
                        ad_array[i-1]                = 1
                         
        elseif mode_speed == "RXAUI_10000"    then              -- RXAUI_10000
                        ad_array[i-1]                = 2
                         
        elseif mode_speed == "XAUI_10000"     then              -- XAUI_10000
                        ad_array[i-1]                = 3
                         
        elseif mode_speed == "XAUI_20000"     then              -- XAUI_20000
                        ad_array[i-1]                = 4
                         
        elseif mode_speed == "1000BaseX_1000" then              -- 1000BaseX_1000
                        ad_array[i-1]                = 5
                         
        elseif mode_speed == "SGMII_1000"     then              -- SGMII_1000
                        ad_array[i-1]                = 6
                         
        elseif mode_speed == "SGMII_2500"     then              -- SGMII_2500
                        ad_array[i-1]                = 7
                         
        elseif mode_speed == "QSGMII_1000"    then              -- QSGMII_1000
                        ad_array[i-1]                = 8
        end  
    end
    
    all_ports = getGlobal("ifRange")

    print("")
  -- loop on all prots and 
  for dev, dev_ports in pairs(all_ports) do
    for k, port in pairs(dev_ports) do
            -- check if port exists in the ranges
            port_exists = does_port_exist (dev,port)
        
            local ad_0_ = ad_array[0]
            local ad_1_ = ad_array[1]
            local ad_2_ = ad_array[2]
            local ad_3_ = ad_array[3]
            local ad_4_ = ad_array[4]
            local ad_5_ = ad_array[5] 
            local ad_6_ = ad_array[6]
            local ad_7_ = ad_array[7]
            local autodetect_ = #params.autodetect
            
            if port_exists then
                values = wrLogWrapper("wrlDxChPortModeSpeedAutoDetectAndConfig",
                          "(dev, port, ad_0_, ad_1_, ad_2_, ad_3_, ad_4_, ad_5_, ad_6_, ad_7_, autodetect_)", 
                            dev, port, ad_0_, ad_1_, ad_2_, ad_3_, ad_4_, ad_5_, ad_6_, ad_7_, autodetect_)
                rc      = values["status"]
    
                if(rc ~= GT_OK)then
                    local index = string.find(returnCodes[rc],"=")
                    print("Error at auto-detect on port " .. tostring(dev) .. "/" .. tostring(port) .." : " .. string.sub(returnCodes[rc],index + 1))
                    do return end
                end           
                            
                ifMode  = values["ifMode"]
                speed   = values["speed"]
                print("auto-detect successfully detect and configure interface mode and speed on port " .. tostring(dev) .. "/" .. tostring(port) .. "\n" ..
                      "Port configuration:    Mode  : " .. ENUM["INTERFACE_MODE"][ifMode] .. "\n" ..
                      "                       Speed : " .. ENUM["PORT_SPEED"][speed]      .. "\n")     
            end
        end  
    end
end


-------------------------------------------------------
-- command registration: auto-detect
-------------------------------------------------------
CLI_addHelp("interface", "auto-detect", "Auto-detect port's actual mode and speed")
CLI_addCommand("interface", "auto-detect", {
    func = auto_detect_mode_speed,
    help = "Auto-detect port's actual mode and speed",
    params = {{type="values", "@autodetect" }}
})
