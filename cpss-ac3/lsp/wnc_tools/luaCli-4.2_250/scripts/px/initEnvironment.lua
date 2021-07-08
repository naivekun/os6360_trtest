--
-- px environment
--

cli_C_functions_registered("wrlCpssIsAsicSimulation")

devEnv      = {}
multiDevEnv = {}
boardEnv    = {}
 
local devPortPatterns =
{
        ["CPSS_PX_FAMILY_PIPE_E"] = 
             {0, 4, 5, 12, 1, 2 }
}
local portCPUPatterns =
{
        ["CPSS_PX_FAMILY_PIPE_E"] = 16
}

local ledIfPatterns =
{
        ["CPSS_PX_FAMILY_PIPE_E"] = 0
}



-- was called from CLI command of 'cpssInitSystem' but need to be called
-- also after 'fast boot' that doing 'second' cpssInitSystem
function ending_fillDeviceEnvironment()
    -- init board specific global parameters
    if fillBoardEnv ~= nil then fillBoardEnv() end
    
    -- run registered callbacks
    if updateCLItypes ~= nil then updateCLItypes() end
    
end

local function check_port_list(port_list, all_ports)
    local port_bmp = {};
    for dummy, port in pairs(all_ports) do
        port_bmp[port] = {};
    end
    for dummy, port in pairs(port_list) do
        if type(port) == "number" then
            if not port_bmp[port] then
                return nil;
            end
        end
    end
    return true;
end

-- check port is able to support loopback (needed by the tests)
local function check_port_loop_back_able(dev, port)
    local rc, enablePtr =  wrlPxPortLoopbackModeEnableGet(dev, port)

    return (rc == 0);
end

-- ************************************************************************
---
--  fillDeviceEnvironment
--        @description   - fill devEnv table with device-depended parameters
--
--        @param         - devEnv
--
function fillDeviceEnvironment()

--  devEnv.dev     =0;
--  devEnv.portCPU =16
--  devEnv.ledIf   =0
--  devEnv.port    ={0, 4, 5, 12, 1, 2 }

  local dev, ports, family, dev_index, port_list, p_lst;

  local sysInfo = luaCLI_getDevInfo();
  if type(sysInfo) ~= "table" then
    -- case of invoking before cpssInitSystem
    return ending_fillDeviceEnvironment();
  end

  dev_index = 1;
  multiDevEnv = {}
  devEnv      = {}
  for i = 0,127 do
    if sysInfo[i] then
      dev = i
      ports = sysInfo[dev]
      family = wrlCpssDeviceFamilyGet(dev)
      if family ~= nil then
          DeviceType = wrlCpssDeviceTypeGet(dev)
      end
      -- print("dev " .. to_string(dev) .." family ".. to_string(family));
      multiDevEnv[dev_index] = {dev = dev}
      -- CPU port
      multiDevEnv[dev_index].portCPU = portCPUPatterns[family]
      -- ports
-- multiDevEnv[dev_index].port = devPortPatterns[family]
      -- list of possible port lists
      -- find first that contains only ports present on device
      port_list = devPortPatterns[family];
      if port_list then
          if check_port_list(port_list, ports) then
              multiDevEnv[dev_index].port = port_list;
          end
      end 
      -- default for not found family 
      if not multiDevEnv[dev_index].port then
          p_lst = {};
          local ports_num = 0;
          for _, port in ipairs(ports) do
              if check_port_loop_back_able(dev, port) then
                  p_lst[#p_lst+1] = port
                  ports_num = ports_num + 1;
              end
              if ports_num >= defaultNumOfPorts then
                  break;
              end
          end
          multiDevEnv[dev_index].port = p_lst;
      end
      multiDevEnv[dev_index].ledIf = ledIfPatterns[family]
      
      devEnv = multiDevEnv[dev_index]
      
      dev_index = dev_index + 1
    end
  end
  
  
  
  print("devices, ports to be used by LUA tests , and configuration files ==> " .. to_string(multiDevEnv))

  -- TODO
  -- system_capability_managment({devID = devEnv.dev})
  
  return ending_fillDeviceEnvironment()
end

-- called from initSysem function that handles cpssInitSystem command ("exec" context)
-- but should also be automatically called at reloading LUA CLI
fillDeviceEnvironment()


-- ************************************************************************
---
--  fillBoardEnv
--        @description   - fill boardEnv table with board-depended parameters
--
--        @param         - boardEnv global struct
--
function fillBoardEnv()
  local family
  local sysInfo = luaCLI_getDevInfo();
  if type(sysInfo) ~= "table" then
    -- case of invoking before cpssInitSystem
    return;
  end

  boardEnv = {}
  boardEnv.FamiliesList = {}
  boardEnv.devList = {}

  for i = 0, 127 do
    if sysInfo[i] then
      family = wrlCpssDeviceFamilyGet(i);
      table.insert(boardEnv.devList, {dev = i, type = family})
      if not boardEnv.FamiliesList[family] then
        boardEnv.FamiliesList[family] = {isExist = True}
      end
    end
  end
  -- print("DBG: boardEnv: " .. to_string(boardEnv));
end



-- fill table with board specific info for multidevices configuration
-- called from initSysem function that handles cpssInitSystem command ("exec" context)
-- but should also be automatically called at reloading LUA CLI
fillBoardEnv()


-- global function to get tested port
-- INPUT  : portIndex  - the index of the port 1..4 or "cpu"
-- OUTPUT : pornNumber - the port number in the 'index'
function getTestPort(portIndex)
    if devEnv == nil then
        return nil -- will cause exception
    end

    if portIndex == "cpu" then
        return devEnv.portCPU
    end

    return devEnv.port[portIndex]
end

-- ************************************************************************
--
--  is_supported_feature
--        @description     check device for supporting one of the features in the list.
--                          return true  - if feature     supported
--                          return false - if feature not supported
--
--        @param devNum  - device number
--        @param singleFeature    - the required feature.
--          ""                            --> no specific feature
--          "MICRO_INIT_TOOL"             --> micro-init tool
-- ---------------------
--
--        @param extraParamName - the name of the 'extra param'
--          for list of supported 'names' : see function is_supported_feature
--        @param extraParamValue - the value of the 'extra param'
--             examples:
--              1. feature 'SIP_5' device but need to support EVLAN 4097 :
--                      SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum , "SIP_5" , "EVLAN" , 4097 )
--                  NOTE: Bobcat3 in 512 ports mode not supports EVLAN > 4095 ... so test not relevant.
--
--
function is_supported_feature(devNum, singleFeature , extraParamName , extraParamValue)
    local devFamily , subFamily = wrlCpssDeviceFamilyGet(devNum)
    local devRevision = wrlCpssDeviceRevisionGet(devNum)
    local isSupported = false
    local got_fail_reason = false
    if singleFeature == "" then
        isSupported = true
    end

    if singleFeature == "MICRO_INIT_TOOL" then
       isSupported = (devFamily == "CPSS_PX_FAMILY_PIPE_E")
    else
        -- unknown feature ?!
    end

    if singleFeature == "" then
        singleFeature = extraParamName .. "=[" .. extraParamValue .. "]"
    end

    if isSupported then
        if (isSimulationUsed() == true ) then
            print ("feature " .. singleFeature .. " is supported in WM")
        else --if (isSimulationUsed == true )
            printLog ("feature " .. singleFeature .. " is supported in BM")
        end --if (isSimulationUsed == true )
    else
        if got_fail_reason then
            -- add no more info
        elseif (isSimulationUsed() == true ) then
            print ("feature " .. singleFeature .. " is not supported in WM")
        else --if (isSimulationUsed == true )
            printLog ("feature " .. singleFeature .. " is not supported in BM")
        end --if (isSimulationUsed == true )
    end

    return isSupported
end


--[[ ************************************************************************
--
--  SUPPORTED_FEATURE_DECLARE
--        @description     check device for supporting one of the features in the list.
--                         and exit from test if not support any of the features
--
--        @param devNum  - device number
--        @param ...    - (list of)the required features.
--          for example : see function is_supported_feature
--
--         examples:
--          1. feature 'BPE_802_1_BR' for 'control bridge' and for 'port extender' :
--                  SUPPORTED_FEATURE_DECLARE(devNum , "BPE_802_1_BR_CONTROL_BRIDGE" , "BPE_802_1_BR_PORT_EXTENDER"    )
--
]]--
function SUPPORTED_FEATURE_DECLARE(devNum, ...)
    local devFamily , subFamily = wrlCpssDeviceFamilyGet(devNum)
    local featuresList = {...}

    local numInList = #featuresList
    local index

    for index = 1, numInList do
        -- we need that one of the features to be supported by the device
        if is_supported_feature(devNum, featuresList[index]) then
            return true
        end
    end

    -- if we got here
    -- the device not support any of the features

    if devFamily == nil then
        -- device not exists
        printLog("NOTE: device not exists so test skipped")
        isDeviceNotSupported = true
        error()
        return false
    end

    if(numInList > 1) then
        printLog("NOTE: for current test : device must support one of next features:" .. to_string(featuresList) .. " so test skipped")
    else
        printLog("NOTE: for current test : device must support next feature:" .. to_string(featuresList[1]) .. " so test skipped")
    end

    -- the device not supported
    isDeviceNotSupported = true
    error()

    return false

end

luaGlobalStore("rxDataFormat", "PX_RX_DATA_STC")
