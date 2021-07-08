--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* run.lua
--*
--* DESCRIPTION:
--*       "run " command for examples mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- ************************************************************************

-- ************************************************************************
-- register C functions used in <examples> suite
--
cmdLuaCLI_registerCfunction("prvLuaTgfPxTransmitPacketsWithCapture")
cmdLuaCLI_registerCfunction("prvLuaTgfTransmitPackets")
cmdLuaCLI_registerCfunction("prvLuaTgfRxCapturedPacket")
cmdLuaCLI_registerCfunction("prvLuaTgfStateTrgEPortInsteadPhyPort")
cmdLuaCLI_registerCfunction("prvLuaTgfPortForceLinkUpEnableSet")
cmdLuaCLI_registerCfunction("prvLuaTgfPortLoopbackModeEnableSet")
cmdLuaCLI_registerCfunction("wrlCpssIsAsicSimulation")
cmdLuaCLI_registerCfunction("wrlDevFilterRangeSet")
cmdLuaCLI_registerCfunction("prvLuaTgfLogBindDefaultFunctions")
if not cli_C_functions_registered("wrlSimulationSlanManipulations") then
    function wrlSimulationSlanManipulations()
        return 0x10 -- GT_NOT_SUPPORTED
    end
end

CLI_prompts["examples"] = "(examples)#"
CLI_addCommand("exec", "examples", {
    func=CLI_change_mode_push,
    constFuncArgs={"examples"},
    help="Switch to examples of tests"
})


-- global var to not force link UP automatically
global_do_not_force_link_UP = false

FAST_BOOT_TEST_NAME = "PX_fastBoot_trafficAfterEeprom"
MICRO_INIT_TOOL_TEST_NAME = "PX_micro_init_tool"
GRANULAR_LOCKING_PROFILER_TEST_NAME = "PX_granular_locking_profiler"

message_level_tbl =
{
    ["silent"      ] = 0,
    ["emergency"   ] = 1,
    ["alerts"      ] = 2,
    ["critical"    ] = 3,
    ["errors"      ] = 4,
    ["warnings"    ] = 5,
    ["notification"] = 6,
    ["information" ] = 7,
    ["debug"       ] = 8,
    ["verbose"     ] = 9,
}

-- 0 - Silent       mode for output"
-- 1 - Emergency    mode for output"
-- 2 - Alerts       mode for output"
-- 3 - Critical     mode for output"
-- 4 - Errors       mode for output"
-- 5 - Warnings     mode for output"
-- 6 - Notification mode for output"
-- 7 - Information  mode for output"
-- 8 - Debug        mode for output"
-- 9 - Verbose      mode for output"

VerboseLevel   = 4
VerboseLvlTxt  = "warnings"

-- 1 - Min   coverage mode for tests
-- 2 - Midle coverage mode for tests
-- 3 - Max   coverage mode for tests

coverage_level_tbl =
{
    ["min"   ] = 1,
    ["middle"] = 2,
    ["max"   ] = 3,
}

CoverageLvl    = 1
CoverageLvlTxt = "min"

--print("run.lua CoverageLvl  = "..to_string(CoverageLvl))
--print("run.lua CoverageLvlTxt  = "..CoverageLvlTxt)

--CoverageLvl    = 3
--CoverageLvlTxt = "max"

-- ************************************************************************

-- status of test
local isTestFailed = nil

-- Tx packets counter for <run> command
local TxPacketsNum = 0

-- is device relevant for test
isDeviceNotSupported = false

-- mark test as long
local isLongTest = false

-- skip long test
local skipLongTests = true

local unbindSlansAlreadyDone = false
local rebuild_called_from_cmd = false
local already_set_implicit_multi_dev_system = false

-- *****************************************************************
-- enable/disable mode for run last tests
local run_LastTests = false
local Last_Tests_List = {}

-- *****************************************************************
local function check_Last_Tests_List(testName)
    for ii = 1 ,#Last_Tests_List do
        if Last_Tests_List[ii]..".lua" == testName then
            return true
        end
    end
    return false
end


-- ************************************************************************
---
--  substituteVars
--        @description   - parse command-line for pattern \$\{*\} and
--                         substitute device-depend parameters from devEnv[] global table
--
--        @param str     - config/deconfig commandline
--
local function substituteVars(str)
    local s = ""
    local n, e, varName

    -- scan input string
    while str ~= "" do
        -- position of left pattern #{
        n = string.find(str,"${", 1, true)
        if n == nil then
            s = s .. str
            str = ""
        else
            -- position of right pattern }
            e = string.find(str, "}", n+2, true)
        end
        if n == nil or e == nil then
            s = s .. str
            str = ""
        else
            -- isolate parameters name
            varName = string.sub(str, n+2, e-1)
            s = s .. string.sub(str, 1, n-1)

            -- get value of parameters from devEnv
            -- or global_test_data_env
            local k = nil;

            -- check if first 2 characters of 'varName' are '@@'
            if (k == nil) and "@@" == string.sub(varName,1 , 2) then
                local expression = string.sub(varName,3)
                -- supposed use as ${any-expression}
                k = loadstring("return " .. expression )()
            end
            if (k == nil) and (type(devEnv) == "table") then
                k = loadstring("return devEnv." .. varName )()
            end
            if (k == nil) and (type(global_test_data_env) == "table") then
                -- supposed use as ${name}
                k = loadstring("return global_test_data_env." .. varName )()
            end

            -- replace parameter with calculated value
            if k ~= nil then
                s = s .. tostring(k)
            else
                s = s .. string.sub(str, n, e)
            end
            str = string.sub(str, e+1)
        end
    end
    return s
end

-- *****************************************************************
function substituteEnvValuesToString(str)
    return substituteVars(str)
end


-- ************************************************************************
---
--  getTrafficCounters
--        @description  read counters per devNum/portNum
--
--        @param devNum  - device number
--        @param portNum - port number
--
function getTrafficCounters(devNum, portNum)
    local result, values
    if portNum == 16 and DeviceCpuPortMode == "CPSS_NET_CPU_PORT_MODE_SDMA_E" then
        local i
        local pkts = nil
        local bytes = nil
        local rxerrs = nil
        for i=0,7 do
            result, values = myGenWrapper("cpssPxNetIfSdmaRxCountersGet", {
                { "IN",  "GT_U8",  "devNum", devNum },
                { "IN",  "GT_U8",  "queueIdx", i },
                { "OUT", "CPSS_PX_NET_SDMA_RX_COUNTERS_STC", "rxCounters" }
            })
            if result == 0 then
                if pkts == nil then
                    pkts = { l={[0] = 0, [1] = 0 }}
                    bytes = { l={[0] = 0, [1] = 0 }}
                end
                local p = values.rxCounters.rxInPkts
                local b = values.rxCounters.rxInOctets
                pkts = wrlCpssGtU64MathAdd(pkts, {l={[0]=p,[1]=0}})
                bytes = wrlCpssGtU64MathAdd(bytes, {l={[0]=b,[1]=0}})
            else
                return nil
            end
        end
        result, values = myGenWrapper("cpssPxNetIfSdmaRxErrorCountGet", {
            { "IN",  "GT_U8",  "devNum", devNum },
            { "OUT", "CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC", "rxErrCount" }
        })
        if result == 0 then
            rxerrs = { l={[0] = 0, [1] = 0 }}
            for i=0,7 do
                local e = values.rxErrCount.counterArray[i]
                rxerrs = wrlCpssGtU64MathAdd(rxerrs, {l={[0]=e,[1]=0}})
            end
        else
            return nil
        end
        if bytes == nil then
            return nil
        end
        return { goodOctetsRcv = bytes,
                ucPktsRcv = pkts,
                macRcvError = rxerrs,
                badOctetsRcv = {l={[0]=0,[1]=0}}
            }
    else
        result, values = cpssPerPortParamGet("cpssPxPortMacCountersOnPortGet",
                                                 devNum, portNum,
                                               "portMacCounterSetArray",
                                               "CPSS_PORT_MAC_COUNTER_SET_STC")
        if 0 ~= result then
            return nil
        end
        return values.portMacCounterSetArray
    end
end

-- ************************************************************************
---
--  setLogOutputMode
--        @description  set silent output mode for tests log
--
--        @param - mode name (silent or verbose)
--
function setLogOutputMode(param)
    if param["mode"] == nil then
      print("Error: undefined mode")
      return false
    end

    if param["mode"] == "silent" then
      trace.setLogVerboseMode(false)
    elseif param["mode"] == "verbose" then
      trace.setLogVerboseMode(true)
    end

    return true
end

-- ************************************************************************
---
--  prvLuaTgfTransmitPacketsInterceptor
--        @description  prvLuaTgfTransmitPackets wrapper for using in run command
--        @param ...
--
function prvLuaTgfTransmitPacketsInterceptor(...)
  local res, val
  res, val = prvLuaTgfTransmitPacketsOriginal(...)
  local params = {...}

  local count = params[4]
  if params[4] == 0xFFFFFFFF then
    -- indication of 'start generating wirespeed'
    count = "generating wirespeed from CPU"
  end

  printLog(
      "---------SENDING PACKET TO " .. to_string(params[1]) .. "/" .. to_string(params[2])
      .. ", BURST COUNT = " .. to_string(count))
  if type(TxPacketsNum) == "string" then
    -- do not modify TxPacketsNum
  elseif params[4] == 0xFFFFFFFF then
    TxPacketsNum = count
  else
    TxPacketsNum = TxPacketsNum + params[4]
  end
  return res, val
end


-- ************************************************************************
---
--  prvLuaTgfTransmitPacketsWithCaptureInterceptor
--        @description  prvLuaTgfPxTransmitPacketsWithCapture wrapper for using in run command
--        @param ...
--
function prvLuaTgfTransmitPacketsWithCaptureInterceptor(...)
  local res, val
  res, val = prvLuaTgfTransmitPacketsWithCaptureOriginal(...)
  local params = {...}
  printLog("---------SENDING PACKET WITH CAPTURING,  BURST COUNT = " .. to_string(params[4]))
  TxPacketsNum = TxPacketsNum + params[4]
  return res, val
end

-- ************************************************************************
---
--  delay
--        @description  delay by interval
--
--        @param mSec - interval in miliseconds
--
function delay(mSec)
  local ret
  ret = cpssGenWrapper("osTimerWkAfter",{{"IN","GT_U32","mils",mSec}})
end

-- ************************************************************************
---
--  resetPortCounters
--        @description  reset all packets counters on dev/port
--
--        @param   devNum  - device number
--        @param   portNum - port number
--
function resetPortCounters(devNum, portNum)
  getTrafficCounters(devNum, portNum)
  if global_do_not_force_link_UP == false then
     prvLuaTgfPortForceLinkUpEnableSet(devNum,portNum,true)
  end
end

-- ************************************************************************
---
--  getPortCounters
--        @description  get rx/tx packets number per dev/port
--
--        @param    devNum  - device number
--        @param    portNum - port number
--        @return   {Rx,Tx , RxBytes, TxBytes} - rx/tx packets number and bytes number since last call
--
function getPortCounters(devNum, portNum)
  local counters = nil

  counters = getTrafficCounters(devNum, portNum)
  if counters == nil then
    return nil
  end

  -- especial case for CPU port
  if portNum == 16 and DeviceCpuPortMode == "CPSS_NET_CPU_PORT_MODE_SDMA_E" then
    return { Rx = counters.ucPktsRcv["l"][0], Tx = 0 , RxBytes = counters.goodOctetsRcv["l"][0], TxBytes = 0}
  end

  if(counters.goodPktsRcv["l"][0] == 0) then
      counters.goodPktsRcv["l"][0] = counters.ucPktsRcv["l"][0] +
            counters.mcPktsRcv["l"][0] + counters.brdcPktsRcv["l"][0]
  end

  if(counters.goodPktsSent["l"][0] == 0) then
      counters.goodPktsSent["l"][0] = counters.ucPktsSent["l"][0] +
            counters.mcPktsSent["l"][0] + counters.brdcPktsSent["l"][0]
  end

  return { Rx = counters.goodPktsRcv["l"][0], Tx = counters.goodPktsSent["l"][0] ,
           RxBytes = counters.goodOctetsRcv["l"][0], TxBytes = counters.goodOctetsSent["l"][0]}
end

--  if Rx == stormingExpectedIndication then Rx,RxBytes checked to be 'not zero'
--  if Tx == stormingExpectedIndication then Tx,TxBytes checked to be 'not zero'
--  if isSrcOfStorming == true and Rx/Tx equal stormingExpectedIndication then
--      this port expected '2 or more packets' (and not just 'not zero')
stormingExpectedIndication = "storming expected (non zero)"
stormingExpectedIndicationTwoOrMore = "storming expected (more then 1)"

-- ************************************************************************
---
--  checkExpectedCounters
--        @description  compare expected counters with actual numbers of
--                      received/transmitted packets per dev/port.
--                      Print message about detected fail with line number
--                      of callers and marked test as failed.
--
--        @param    lineNum          - line number of caller procedure
--        @param    devPortCounters  - table with devNum's, port's and
--                                     expected counters in format:
--                         {[dev] = { [Port1]  = {Rx=X_1, Tx=Y_1, RxBytes=bX_1, TxBytes=bY_1 , isSrcOfStorming = nil/true/false},
--                                    [Port2] =  {Rx=X_2, Tx=Y_2, RxBytes=bX_2, TxBytes=bY_2 , isSrcOfStorming = nil/true/false},
--                                     ...
--                                    [PortN] =  {Rx=X_N, RxBytes=bX_n, TxBytes=bY_n , isSrcOfStorming = nil/true/false}     }})
--      NOTE : about stormingExpectedIndication
--              if Rx == stormingExpectedIndication then Rx,RxBytes checked to be 'not zero'
--              if Tx == stormingExpectedIndication then Tx,TxBytes checked to be 'not zero'
--              if isSrcOfStorming == true and Rx/Tx equal stormingExpectedIndication then
--              this port expected '2 or more packets' (and not just 'not zero')
--
function checkExpectedCounters(lineNum, devPortCounters)

  local isFailDetected = false
  local lineString, devPortString , middleString , RxString , TxString , counterInfoStr
  local RxBytesString , TxBytesString
  local txCheckDone , rxCheckDone
  local failThreshold
  local isSrcOfStorming

      -- support omition of the line number ... because without support of 'file name' it is not much info !
  if lineNum ~= nil then
      lineString = string.format("(at line #%05d) " , lineNum)
  else
      lineString = ""
  end

  middleString = "Packets Counter : "
  RxString = "ERROR : Rx "
  RxBytesString = RxString .. "Bytes "
  TxString = "ERROR : Tx "
  TxBytesString = TxString .. "Bytes "


  for dev,portCounters in pairs(devPortCounters) do
    for port, expectedCounters in pairs(portCounters) do
      local counters

      rxCheckDone = false
      txCheckDone = false

      devPortString = string.format("for dev#%02d  port#%02d: " , dev, port)

      counters = getPortCounters(dev, port)

      isSrcOfStorming = expectedCounters.isSrcOfStorming

      if expectedCounters.Rx == stormingExpectedIndication then
        if isSrcOfStorming == true then
            failThreshold = 1
            counterInfoStr = "actual="..to_string(counters.Rx).." , but " .. stormingExpectedIndicationTwoOrMore
        else
            failThreshold = 0
            counterInfoStr = "actual=ZERO , but " .. stormingExpectedIndication
        end

          if counters.Rx <= failThreshold then
              setFailState()
              isFailDetected = true
              printLog(RxString .. middleString .. lineString .. devPortString .. counterInfoStr)
          end
          if counters.RxBytes == 0 then
              setFailState()
              isFailDetected = true
              counterInfoStr = "actual=ZERO , but " .. stormingExpectedIndication
              printLog(RxBytesString .. middleString .. lineString .. devPortString .. counterInfoStr)
          end

          rxCheckDone = true
      end

      if expectedCounters.Tx == stormingExpectedIndication then
        if isSrcOfStorming == true then
            failThreshold = 1
            counterInfoStr = "actual="..to_string(counters.Tx).." , but " .. stormingExpectedIndicationTwoOrMore
        else
            failThreshold = 0
            counterInfoStr = "actual=ZERO , but " .. stormingExpectedIndication
        end
          if counters.Tx <= failThreshold then
              setFailState()
              isFailDetected = true
              printLog(TxString .. middleString .. lineString .. devPortString .. counterInfoStr)
          end
          if counters.TxBytes == 0 then
              setFailState()
              isFailDetected = true
              counterInfoStr = "actual=ZERO , but " .. stormingExpectedIndication
              printLog(TxBytesString .. middleString .. lineString .. devPortString .. counterInfoStr)
          end
          txCheckDone = true
      end

      if rxCheckDone == false then
          if expectedCounters.Rx ~= nil then
            if expectedCounters.Rx ~= counters.Rx then
              setFailState()
              isFailDetected = true
              counterInfoStr = (string.format("actual=%3d expected=%3d" ,
                                counters.Rx, expectedCounters.Rx))
              printLog(RxString .. middleString .. lineString .. devPortString .. counterInfoStr)
            end
          end

          if expectedCounters.RxBytes ~= nil then
              if expectedCounters.RxBytes ~= counters.RxBytes then
                  setFailState()
                  isFailDetected = true
                  counterInfoStr = (string.format("actual=%3d expected=%3d" ,
                      counters.RxBytes, expectedCounters.RxBytes))
                  printLog(RxBytesString .. middleString .. lineString .. devPortString .. counterInfoStr)
              end
          end
      end -- rxCheckDone == false

      if txCheckDone == false then
          if expectedCounters.Tx ~= nil then
            if expectedCounters.Tx ~= counters.Tx then
              setFailState()
              isFailDetected = true
              counterInfoStr = (string.format("actual=%3d expected=%3d" ,
                                counters.Tx, expectedCounters.Tx))
              printLog(TxString .. middleString .. lineString .. devPortString .. counterInfoStr)
            end
          end

          if expectedCounters.TxBytes ~= nil then
              if expectedCounters.TxBytes ~= counters.TxBytes then
                  setFailState()
                  isFailDetected = true
                  counterInfoStr = (string.format("actual=%3d expected=%3d" ,
                      counters.TxBytes, expectedCounters.TxBytes))
                  printLog(TxBytesString .. middleString .. lineString .. devPortString .. counterInfoStr)
              end
          end
      end -- txCheckDone == false
    end--pairs(portCounters)
  end-- pairs(devPortCounters)
  return isFailDetected
end

-- ************************************************************************
---
--  printExpectedCounters
--        @description  print the expected counters of
--                      received/transmitted packets per dev/port.
--
--        @param    devPortCounters  - table with devNum's, port's and
--                                     expected counters in format:
--                         {[dev] = { [Port1]  = {Rx=X_1, Tx=Y_1, RxBytes=bX_1, TxBytes=bY_1 , isSrcOfStorming = nil/true/false},
--                                    [Port2] =  {Rx=X_2, Tx=Y_2, RxBytes=bX_2, TxBytes=bY_2 , isSrcOfStorming = nil/true/false},
--                                     ...
--                                    [PortN] =  {Rx=X_N, RxBytes=bX_n, TxBytes=bY_n , isSrcOfStorming = nil/true/false}     }})
--      NOTE : about stormingExpectedIndication
--              if Rx == stormingExpectedIndication then Rx,RxBytes checked to be 'not zero'
--              if Tx == stormingExpectedIndication then Tx,TxBytes checked to be 'not zero'
--              if isSrcOfStorming == true and Rx/Tx equal stormingExpectedIndication then
--              this port expected '2 or more packets' (and not just 'not zero')
--
function printExpectedCounters(devPortCounters)
    local devPortString
    local RxBytesString , TxBytesString
    local RxPacketsString , TxPacketsString
    local isSrcOfStorming

    printLog("print Expected Counters : started")

    for dev,portCounters in pairs(devPortCounters) do
        for port, expectedCounters in pairs(portCounters) do

            devPortString = string.format("For dev/port %d/%d: " , dev, port)

            isSrcOfStorming = expectedCounters.isSrcOfStorming

            if expectedCounters.Rx == stormingExpectedIndication then
                if isSrcOfStorming == true then
                    RxPacketsString = " Rx_packets = " .. stormingExpectedIndicationTwoOrMore
                else
                    RxPacketsString = " Rx_packets = " .. stormingExpectedIndication
                end
            elseif expectedCounters.Rx ~= nil then
                RxPacketsString = " Rx_packets = " .. expectedCounters.Rx
            else
                RxPacketsString = ""
            end

            if expectedCounters.Rx == stormingExpectedIndication then
                RxBytesString = " Rx_bytes = " .. stormingExpectedIndication
            elseif expectedCounters.RxBytes ~= nil then
                RxBytesString = " Rx_bytes = " .. expectedCounters.RxBytes
            else
                RxBytesString = ""
            end

            if expectedCounters.Tx == stormingExpectedIndication then
                if isSrcOfStorming == true then
                    TxPacketsString = " Tx_packets = " .. stormingExpectedIndicationTwoOrMore
                else
                    TxPacketsString = " Tx_packets = " .. stormingExpectedIndication
                end
            elseif expectedCounters.Tx ~= nil then
                TxPacketsString = " Tx_packets = " .. expectedCounters.Tx
            else
                TxPacketsString = ""
            end

            if expectedCounters.Tx == stormingExpectedIndication then
                TxBytesString = " Tx_bytes = " .. stormingExpectedIndication
            elseif expectedCounters.TxBytes ~= nil then
                TxBytesString = " Tx_bytes = " .. expectedCounters.TxBytes
            else
                TxBytesString = ""
            end

            printLog(devPortString .. RxPacketsString .. RxBytesString .. TxPacketsString .. TxBytesString)

        end--pairs(portCounters)
    end-- pairs(devPortCounters)

    printLog("print Expected Counters : ended")

end

local function executeCpssAPICall(fd, str)
    local cpss_API_name = prefix_del("callCpssAPI ", str);
    --local cpss_API_params
    local params_str = ""
    local state = 0
    local e, str0, str1
    local retval = false
    while true do
        str0 = fs.gets(fd)
        if str0 == nil then
            printLog("error: callCpssAPI incomplete.")
            return false
        end
        str1 = substituteVars(str0)
        if state == 0 then
            if prefix_match("<<<PARAMS", str1) then
                state = 1
            end
        else
            if prefix_match(">>>", str1) then
                break
            end
            params_str = params_str .. str1
        end
    end
    e, str1 = pcall(loadstring("_cpss_API_params="..params_str))
    if not e then
        printLog("error: callCpssAPI params systax error: "..str1)
        return false
    end
    printLog("Performing: myGenWrapper(\""..cpss_API_name.."\", ...)")
    local result, values = myGenWrapper(cpss_API_name, _cpss_API_params)
    if type(result) ~= "number" then
        printLog("   failed: "..to_string(result))
    else
        str1 = returnCodes[result]
        if str1 == nil then
            str1 = to_string(result)
        end
        retval = (result == 0)
        printLog("   result="..str1)
        printLog("   values="..to_string(values))
    end
    _cpss_API_params = nil
    return retval
end

--************************************************************************
--
--  executeStringCliCommands
--        @description  execute string variable with CLI commands. Every command
--                      is ended with newline character. Generate an exception
--                      if an error occurred
--
--        @param        st - string with commands.
--
--
function executeStringCliCommands(st)
   -- split string on lines finished by newline character
    local ret = true
    for line in st:gmatch("[^\n]+") do
        -- delete leading and trailing spaces
        line = string.gsub(line, "^%s*(.-)%s*$", "%1")
        if string.sub(line, 1, 2) ~= "//" then
            line = substituteEnvValuesToString(line);
            printLog("Performing: " .. line)
            if not cmdLuaCLI_execute(line) then
                error("executeStringCliCommands " .. line)
            end
        end
    end
end

-- ************************************************************************
---
--  executeLocalConfig
--        @description  load and execute configuration file line by line
--
--        @param params.name  - name of configuration file
--
--        @return       true, if success, otherwise false
--
function executeLocalConfig(filename)
    local ret = true
    local line, fid
    printLog("Load configuration .. "..filename)
    fid = fs.open(filename, "r")

    if fid == nil then
        printLog("failed to open file " .. filename)
        return false,"failed to open file " .. filename
    end

    line = fs.gets(fid)

    while (line ~= nil) do
        local tline;
        tline = string.gsub(line, "^%s*(.-)%s*$", "%1") --delete spaces

        if not prefix_match("//", tline) then -- skip comments
            -- substitute device-depended global parameters(if found)
            tline = substituteVars(tline)

            if prefix_match("callCpssAPI ", tline) then
                if not executeCpssAPICall(fid, tline) then
                    setFailState()
                    ret = false
                end
            else
                printLog("Performing: (".. tostring(CLI_current_mode_get()) .. ")# ".. tline)

                -- skip do show command for silent mode
                if (trace.getLogVerboseMode() or (string.sub(tline, 1, 7) ~= "do show")) then
                    if not cmdLuaCLI_execute(tline) then
                        setFailState()
                        ret = false
                        printLog("Error in: " .. line)
                    end
                end
            end
        end
        -- read next line
        line = fs.gets(fid)
    end

    printLog("Config file <".. filename .."> executed.")
    fs.close(fid)

    return ret
end

-- ************************************************************************
---
--  executeConfigFile
--        @description  execute file from configuration directory and print log
--
--        @param params.name  - name of configuration file
--
local function executeConfigFile(params)

    local ret, status
    local name, fileName
    local fd, e
    local line

    if params.name == nil then
        print("Configuration name not specified. Nothing to do.")
        return false
    end

    name  = params.name
    fileName = "px/examples/configurations/"..name..".txt"

    fd, e = fs.open(fileName, "r")

    if fd == nil then
        print("Failed to open file " .. fileName .. ": "..e)
        return false
    end

    printMsg("Execute configuration .. "..fileName)

    line, e = fs.gets(fd)
    while (line ~= nil) do
        local xLine
        xLine = string.gsub(line, "^%s*(.-)%s*$", "%1") --delete spaces
        while string.sub(xLine,-1) == "\\" do
            xLine = string.gsub(string.sub(xLine,1,-2),"^%s*(.-)%s*$", "%1")
            line, e = fs.gets(fd)
            if line == nil then
                break
            end
            xLine = xLine .. " " .. string.gsub(line, "^%s*(.-)%s*$", "%1")
        end


        if not prefix_match("//", xLine) then -- skip comments
            -- substitute device-depended global parameters(if found)
            xLine = substituteVars(xLine)

            if prefix_match("callCpssAPI ", xLine) then
                executeCpssAPICall(fd, xLine, true)
            else
                printMsg("Performing: " .. xLine)
                cmdLuaCLI_execute(xLine)
            end
        end
        line, e = fs.gets(fd)
    end

    fs.close(fd)

    --if an error has occured
    if e ~= "<EOF>" then  print("Error reading file: " .. e)  end

    printMsg("Config file <".. fileName .."> executed.")

    CLI_change_mode_pop_all()
    CLI_change_mode_push("examples")
end

-- ************************************************************************
---
--  showConfigFile
--        @description  shows the contents of a file
--
--        @param params         - The parameters
--

local function showConfigFile(params)
    local str, e, fd
    local name, fileName

    if params.name == nil then
        print("Configuration name not specified. Nothing to do.")
        return false
    end

    name  = params["name"]
    fileName = "px/examples/configurations/"..name..".txt"

    fd,e=fs.open(fileName)

    if fd == nil then
        print("Failed to open file " .. name .. ": "..e)
        return false
    end

    str, e = fs.gets(fd)
    while (str~=nil) do
        str=string.gsub(str,"\n","")

        if not string.find(str, "^%s*//") then
            -- substitute device-depended global parameters(if found)
            str = substituteVars(str)
        end

        print(str)
        str, e = fs.gets(fd)
    end
    fs.close(fd)
    if e ~= "<EOF>" then  print("Error reading file: " .. e)  end
end

-- ************************************************************************
---
--  loadConfigFile
--        @description  load contents of a config file
--        @param name   - The name of config file
--

local function loadConfigFile(name)
    local str, e, fd
    local fileName
    local fileContent = {}

    if name == nil then
        return nil
    end

    fileName = "px/examples/configurations/"..name..".txt"
    fd,e=fs.open(fileName)

    if fd == nil then
        return nil
    end

    str, e = fs.gets(fd)
    while (str~=nil) do
        str=string.gsub(str,"\n","")
        table.insert(fileContent, str)
        str, e = fs.gets(fd)
    end
    fs.close(fd)
    if e ~= "<EOF>" then
      print("Error reading file: " .. e)
    end

    return fileContent
end

--if the item in table
local function inTable(tbl, key)
    local i
    for i = 1, #tbl do
        if tbl[i] == key then
            return true
        end
    end
    return false
end

--to provide return from the test
local function setTestResult(val)
    isTestFailed = val
end

--to provide return from the test
local function getTestResult()
    return isTestFailed
end

-- to set the test status as FAILED from a test
function setFailState()
    setTestResult(true)
end

function setTestStateSkipped()
    isDeviceNotSupported = true;
    setFailState();
end

function MARK_LONG_TEST()
    if (skipLongTests == true) then
        isLongTest = true;
        error();
        return false;
    end
    return true;
end

--returns the list of files from the directory
local function getFiles(directory)
    local fd,err,totalMem
    local res = {}
    local a

    fd,err = fs.opendir(" ")

    if (fd==nil) then
        print (err)
    else
        a = fs.readdir(fd)

        while (a~=nil) and (a~=0) do
            local path = to_string(a.d_name)
            --if it starts from the "directory"

            if string.sub(path, 2, #directory+1) == directory then
                table.insert(res, string.sub(path, #directory+3, #path-1))
            end
            a = fs.readdir(fd)
        end
        fs.closedir(fd)
    end

    return res
end

-- ************************************************************************
---
--  getTestNames
--        @description  return list of existed tests
--
--        @param none
--
local function getTestNames()
    local testnames = {}
    local test_files
    local testName

    test_files = getFiles("px/examples/tests")

    for i = 1, #test_files do
        testName = test_files[i]
        -- Last Tests that shouldn't run by 'run all'
        if(false == check_Last_Tests_List(testName)) then
            local name = string.sub(testName, 0, #testName-4)
            table.insert(testnames, name)
        end
    end

    return testnames
end

-- ************************************************************************
---
--  getConfigNames
--        @description  return list of existed configs
--
--        @param none
--
local function getConfigNames()
    local configList = {}
    local confNames = {}
    configList = getFiles("px/examples/configurations")

    for i = 1, #configList do
        local fullName = configList[i]
        local name = string.sub(fullName, 0, #fullName-4)
        if inTable(configList, name..".txt")  then
          table.insert(confNames, name)
        end
    end
    return confNames
end


local portsInTest = {port1,port2,port3,port4,port5}


-- init tested ports : force link up + reset mac counters
luaTgfConfigTestedPorts(devNum,portsInTest,true)

-- force link up for needed system ports
function luaTgfConfigSystemPorts(forceLinkUp)
    if(forceLinkUp == true) then
        print("System Ports of lua CLI tests:")
    end

    local devNum        = devEnv.dev
    local ports         = devEnv.port
    local portsInTest   = {}

    for dummy,port in pairs(ports) do
        local lastIndex = #portsInTest
        portsInTest[lastIndex + 1] = port
    end

    luaTgfConfigTestedPorts(devNum,portsInTest,forceLinkUp)
end

local function is_name_in_test(subTestName , fullTest)
    local val = string.match(fullTest , subTestName)

    if val then
        return true
    end

    return false
end


-- ************************************************************************
---
--  parseWildcardName
--        @description  return list of all the tests that
--                       'match' the name with wildcard
--
--        @param name - name of test(s) with * (wild-card)
--
local function parseWildcardName(name)
    local tests_to_run = {}
    local testName
    local testnames = getTestNames()

    if run_LastTests == true then
        local k,v
        for k,v in ipairs(Last_Tests_List) do
            table.insert(testnames, v)
        end
    end

    local wild_card_exists = string.find(name,"*", 1, true)

    -- nil if no wildcard
    if wild_card_exists == nil then
        return nil
    end

    -- replace '*' with '[%w_]+' for the use as pattern for string.match method
    local pattern = string.gsub(name , "*" , "%[%%w%_%]%+")

    -- print(to_string(pattern))

    for i = 1, #testnames do
        testName = testnames[i]
--        print(to_string(testName:match(pattern)))
--        if string.match(testName, pattern) then
        if testName:match("^" .. pattern) then
            table.insert(tests_to_run,testName)
        end
    end

--     print(to_string(tests_to_run))

    if #tests_to_run == 0 then
        return nil
    end

    return tests_to_run
end
local LONG_TEST_CNS = 30 --seconds
-- run specific test
local function internal_run_specific(name,totalResults)
    -- system print intercepting function
    local function dumbPrint(...)
    end
    -- system print handler
    local systemPrintHandler

    local res, err
    local startTime = os.time()
    local tastTime

    TxPacketsNum = 0
    printMsg("====== The test \""..name.."\" started")

    setTestResult(nil)
    isDeviceNotSupported = false
    isLongTest = false

    -- hide output for non-verbose mode
    if trace.getLogVerboseMode() == true then
      res, err = pcall(dofile,"px/examples/tests/"..name..".lua")
      if (not res) and (not isDeviceNotSupported) and (not isLongTest) then
        print("ERROR: ", err)
      end
    else
      systemPrintHandler = print
      print = dumbPrint
      res, err = pcall(dofile,"px/examples/tests/"..name..".lua")
      print = systemPrintHandler
    end

    -- declare fail state if internal error detected
    if not res then setFailState() end

    tastTime = os.difftime(os.time(), startTime)
    local runTime = string.format(" Running time is %d second(s)", tastTime)

    if (getTestResult() == true) then
      if (isDeviceNotSupported or isLongTest) then
        printMsg("====== The test \""..name.."\" SKIPPED." .. runTime)
        totalResults.skipNum = totalResults.skipNum + 1
        -- add name to list of skipped tests
        table.insert(totalResults.listOfSkippedTests, name)
      else
        printMsg("====== The test \""..name.."\" FAILED." .. runTime)
        totalResults.failNum = totalResults.failNum + 1
        -- add name to list of failed tests
        table.insert(totalResults.listOfFailedTests, name)
      end
    else
        printMsg("====== The test \""..name.."\" PASSED." .. runTime)
        totalResults.passNum = totalResults.passNum + 1
        table.insert(totalResults.listOfPassedTests, name)
    end

    if (tastTime >= LONG_TEST_CNS) then
        table.insert(totalResults.longTests,{name=name,runTime=runTime})
    end

    printMsg("====== Sent ".. TxPacketsNum .. " packets")
    printMsg("\n")
end

-- luaTgfConfigSystemPorts shouldn't run in 'fastBoot_trafficAfterEeprom'
local skipWhenPexOnly = {FAST_BOOT_TEST_NAME, MICRO_INIT_TOOL_TEST_NAME}

-- check if test need to be skipped when run in list of tests
local function check_skipWhenPexOnly(testName)
    for ii = 1 ,#skipWhenPexOnly do
        if skipWhenPexOnly[ii] == testName then
            return true
        end
    end
    return false
end


-- run specific test
local function run_specific(name)
    local totalResults = {  skipNum = 0 , failNum = 0 , passNum = 0 ,
                            listOfFailedTests  = {} ,
                            listOfSkippedTests = {} ,
                            listOfPassedTests  = {},
                            longTests = {}}

    if(check_skipWhenPexOnly(name)==false) then
        -- force system port to link UP .. so tests no worry about it
        luaTgfConfigSystemPorts(true)
    end

    internal_run_specific(name,totalResults)

    if(check_skipWhenPexOnly(name)==false)then
        -- restore force system port to link UP
        luaTgfConfigSystemPorts(false)
    end

end


-- *****************************************************************

-- Tests that shouldn't run by 'run all'
local skipTestsWhenNotExplicit = {FAST_BOOT_TEST_NAME,
                                  MICRO_INIT_TOOL_TEST_NAME,
                                  GRANULAR_LOCKING_PROFILER_TEST_NAME}

-- check if test need to be skipped when run in list of tests
local function check_skipTestsWhenNotExplicit(testName)
    for ii = 1 ,#skipTestsWhenNotExplicit do
        if skipTestsWhenNotExplicit[ii] == testName then
            return true
        end
    end

    return false
end

-- run list of tests
local function run_list(listNames)
    local totalResults = {  skipNum = 0 , failNum = 0 , passNum = 0 ,
                            listOfFailedTests  = {} ,
                            listOfSkippedTests = {} ,
                            listOfPassedTests  = {},
                            longTests = {}}
    local startTime = os.time()

    -- force system port to link UP .. so tests no worry about it
    luaTgfConfigSystemPorts(true)

    local valueGet
    local result, val
    local apiName
    local special
    local sim_debug_test = nil--"vtag_6_bytes_cb_and_pe"

    for i=1,#listNames do
        -- Tests that shouldn't run by 'run all'

        if(sim_debug_test == listNames[i]) then
            startSimulationLog()
        end

        if(false == check_skipTestsWhenNotExplicit(listNames[i])) then
            internal_run_specific(listNames[i],totalResults)
        end

        if(sim_debug_test == listNames[i]) then
            return
        end
        -- in case we see a configuration that is different from the default,
        -- we can use the following logic to find what test do not de-configure it.
        -- use the CPSS get API to read the proper configuration
--[[
        apiName = "cpssPxBrgFdbNaToCpuPerPortGet"
            result, val  = myGenWrapper(apiName, {
                { "IN",     "GT_U8",       "devNum",   0 },
                { "IN",     "GT_PORT_NUM",      "portNum",   8},
                { "OUT","GT_BOOL","updateEnable"},

            })
            valueGet  = val["updateEnable"]

        if(valueGet==false)then
            printLog("the name of the test that do not de-configure correctly is: " .. listNames[i])
            printLog("the value of the de-configured parameter is: ")
            printLog(valueGet)
            return
        end
--]]

    end

    local totalNum = #listNames

    -- restore force system port to link UP
    luaTgfConfigSystemPorts(false)

    printMsg("========== The Run ended ==========")

    if totalResults.skipNum ~=0 then
        printMsg("\n")
        printMsg("========== Summary list of skipped tests:")
        local i
        for i=1, #totalResults.listOfSkippedTests do
          printMsg(totalResults.listOfSkippedTests[i])
        end
        printMsg("")
    end

    if totalResults.failNum ~=0 then
        printMsg("\n")
        printMsg("========== Summary list of failed tests:")
        local i
        for i=1, #totalResults.listOfFailedTests do
          printMsg(totalResults.listOfFailedTests[i])
        end
        printMsg("")
    end

    if( #totalResults.longTests > 0) then
        printMsg("\n")
        printMsg("========== long tests (>="..LONG_TEST_CNS.." seconds):")
        local i
        for i=1, #totalResults.longTests do
          printMsg(totalResults.longTests[i].name,totalResults.longTests[i].runTime)
        end
        printMsg("")
    end


    printMsg("========== Summary statistics ==========")
    printMsg("PASS       FAIL       SKIP       TOTAL")
    printMsg("========================================")
    printMsg(string.format("[%d]        [%d]        [%d]        [%d]",
              totalResults.passNum, totalResults.failNum, totalResults.skipNum, totalNum))

    local ts = string.format("Total running time: %d seconds", os.difftime(os.time(), startTime))
    printMsg(ts)
    printMsg("\n")

end


-- ************************************************************************
---
--  listTests
--        @description  list all the tests that 'match' the name of parameter
--
--        @param name - name of test(s) , can be with * (wild-card)
--
local function listTests(params)
    local name = params["testname"]

    if name == "all" then name = "*" end

    local wild_card_list = parseWildcardName(name)
    if (wild_card_list) then

        if #wild_card_list == 1 then
            name = wild_card_list[1]
        else
            for ii=1,#wild_card_list do
                print("Test #".. ii .. " : ".. wild_card_list[ii])
            end

            print("Total of " .. #wild_card_list .. " tests")
            return
        end
    end

    print("(single test) Test : ".. name)
end


-- ************************************************************************
---
--  run
--        @description  execute specified luaCLI test
--                      (or all tests if testName "all" or "*")
--
--        @param name - name of test(s) , can be with * (wild-card)
--
local function run(params)
    local name
    local testnames
    local isStressMode = false

    local i, j, k


    -- list of lists of tests for stress mode
    local testslist = {}
    testslist[1] = {}
    testslist[2] = {}
    testslist[3] = {}

    local notFound = true

    if params["test"] == nil and params.allFlag == nil then
      print("Error: undefined test name")
      return false
    end

    name  = params["test"]

    -- is testName is valid and fill tables for stress mode
    -- fill tables for <run stress testName>
    if params.stressFlag ~= nil then
      isStressMode = true

      local testnames = getTestNames()
      local testName

      -- create 3 equal lists of tests to run (without specified test)
      for i = 1, #testnames do
        testName = testnames[i]
        if testName ~= name then
          table.insert(testslist[1], testName)
          table.insert(testslist[2], testName)
          table.insert(testslist[3], testName)
        else
          notFound = false
        end
      end

      -- is specified test name exists?
      if notFound then
        print("Error: undefined test name")
        return false
      end

      -- run after all tests to see results without impacting of specified test
      table.insert(testslist[1], #testslist[1]+1, name)
      -- run in middle
      table.insert(testslist[2], #testslist[2]/2, name)
      -- run in begin
      table.insert(testslist[3], 1, name)
    end --[[ if params["stress"] ~= nil ]]--


    if wrlCpssIsAsicSimulation() and
       params.allFlag ~= nil and
       not unbindSlansAlreadyDone
    then
        wrlSimulationSlanManipulations(2)--indication of unbind slans of all ports from all devices
        unbindSlansAlreadyDone = true
    end

    prvLuaTgfTransmitPacketsOriginal = prvLuaTgfTransmitPackets
    prvLuaTgfTransmitPackets = prvLuaTgfTransmitPacketsInterceptor
    prvLuaTgfTransmitPacketsWithCaptureOriginal = prvLuaTgfPxTransmitPacketsWithCapture
    prvLuaTgfPxTransmitPacketsWithCapture = prvLuaTgfTransmitPacketsWithCaptureInterceptor

    -- enable printing for TGF utils
    prvLuaTgfLogBindDefaultFunctions();

    -- do not let the 'show' commands break the run with ""
    ignore_all_nice_printings(true)


    -- run test or list of tests
    if params.allFlag == nil and params.stressFlag == nil then
        -- print ("NNN params " .. to_string(name))
        local testsList ={}

        -- in commandline specified only one test
        if #name == 1 then
          testsList = parseWildcardName(name[1])

          --check if wildmark used
          if (testsList) then
            run_list(testsList)
          else
            run_specific(name[1])
          end

        -- in commandline specified several names
        else
          local wList
          -- iterate thru all specified names
          for k = 1, #name do
            wList = parseWildcardName(name[k])
            if wList == nil then
              table.insert(testsList, name[k])
            else
              local l
              for l=1, #wList do
                table.insert(testsList, wList[l])
              end
            end
          end
          -- run list of tests
          run_list(testsList)
        end

    else --[[ if (params["testname"] ~= "all") and (isStressMode == false) ]]--

      if wild_card_list and isStressMode then
        print("Error: not implemented 'wild-card' with 'stress' mode ")
      end


      for j = 1, 3 do
        -- results counters

        if isStressMode then
          printMsg("====== run-stress "..name.." attempt#".. tostring(j))
          testnames = testslist[j]
        else
          testnames = getTestNames()
        end

        if run_LastTests == true then
            local m
            for m=1, #Last_Tests_List do
            table.insert(testnames, Last_Tests_List[m])
            end
        end

        run_list(testnames)


        if isStressMode == false then break end

      end

    end



    -- restore mode set earlier
    ignore_all_nice_printings(false)

    prvLuaTgfTransmitPackets = prvLuaTgfTransmitPacketsOriginal
    prvLuaTgfPxTransmitPacketsWithCapture = prvLuaTgfTransmitPacketsWithCaptureOriginal

    CLI_change_mode_pop_all()
    CLI_change_mode_push("examples")
end

-- ************************************************************************
---
--  checkParamTestName
--        @description  check testName is valid
--
--        @param testname - name of test
--
local function checkParamTestName(param)
    local testnames = getTestNames()

    local k,v
    for k,v in ipairs(Last_Tests_List) do
            table.insert(testnames, v)
    end

    if inTable(testnames, param) then
        return true, param
    end

    local wild_card_list = parseWildcardName(param)
    if (wild_card_list) then
        return true, param
    end

    return false, "invalid test name"
end

-- ************************************************************************
---
--  completeValueTestName
--        @description  return list of tests
--
--        @param none
--
local function completeValueTestName(param)
    local compl={}
    local help={}

    local testnames = getTestNames()
    local i

    local k,v
    for k,v in ipairs(Last_Tests_List) do
            table.insert(testnames, v)
    end

    for i=1,#testnames do
        if prefix_match(param,testnames[i]) then
            table.insert(compl, testnames[i])
            help[#compl] = "execute test"
        end
    end

    local wild_card_list = parseWildcardName(param)
    if (wild_card_list) then
        for i=1,#wild_card_list do
            table.insert(compl, wild_card_list[i])
            help[#compl] = "execute test"
        end
    end

    return compl, help
end

-- ************************************************************************
---
--  completeValueConfigName
--        @description  return list of configuration files
--
--        @param none
--
local function completeValueConfigName(param)
    local compl={}
    local help={}
    local configNames = getConfigNames()
    local i
    for i=1,#configNames do
        if prefix_match(param,configNames[i]) then
            table.insert(compl, configNames[i])
        end
    end
    return compl, help
end


-- ************************************************************************
---
--  getConfigDescription
--        @description  return tags, shortDescription and fullDescription of
--        configuration file
--
--        @param name - name of configuration file
--
local function getConfigDescription(name)
    local fileContent = {}
    local fileDescription = {}
    local dMode = 0
    local tagsList = {}
    local shortDescr
    local fullDescr = {}
    local testDescr = {}

    -- check name
    if name == nil then
        return nil
    end

    -- load file content
    fileContent = loadConfigFile(name)

    if fileContent == nil then
        return nil
    end

    -- select valid description section of file
    for i = 1, #fileContent do
      local line = fileContent[i]
      if string.sub(line, 1, 2) ~= "//" then
        break
      end
      table.insert(fileDescription, line)
    end

    if fileDescription == nil then
      -- no valid description found -> return empty lists
      return {tags = tagsList, shortDescr = shortDescr, fullDescr = fullDescr, testDescr = testDescr}
    end

    -- split description to section
    for i = 1, #fileDescription do
      local line = fileDescription[i]

      -- // TAG:
      if string.sub(line, 1, 7) == "// TAG:" then
        local allTags = string.sub(line, 8, #line)
        -- split allTag string by words
        for word in string.gmatch(allTags,"[%w-]+") do
          -- convert to lowercase and add to tags table
          table.insert(tagsList, string.lower(word))
        end
      end

      -- // Short Description:
      if string.sub(line, 1, 22) == "// Short Description: " then
        shortDescr = string.sub(line, 23, #line)
      end


      -- // Description:
      if (string.sub(line, 1, 16) == "// Description: ") or (dMode == 1) then
        dMode = 1
        table.insert(fullDescr, line)
      end

      -- // Test Description:
      if (string.sub(line, 1, 21) == "// Test Description: ") or (dMode == 2) then
        dMode = 2
        table.insert(testDescr, line)
      end
    end

    return {tags = tagsList, shortDescr = shortDescr, fullDescr = fullDescr, testDescr = testDescr}
end

-- ************************************************************************
---
--  searchByTag
--        @description  search for a configuration files in the RAMFS according
--                      to a string entered. For each file, a small description
--                      will be displayed.
--
--        @param tagString - string with tags
--
local function searchByTag(params)
    local configFiles = {}
    local tags = {}
    local tagLine

    if params["tagString"] == nil then
        print("Error: empty tag's line")
        return false
    end

    -- convert to lower case
    tagLine = string.lower(params["tagString"])

    -- split tag string by words
    for word in string.gmatch(tagLine,"[%w-]+") do
      table.insert(tags, word)
    end
    if tags == nil then
        print("Error: no tags specified")
        return false
    end

    -- load list of configuration files
    configFiles = getConfigNames()
    if configFiles == nil then
        print("Error: no configuration files found")
        return false
    end

    --
    for i = 1, #configFiles do
      local description
      description = getConfigDescription(configFiles[i])
      if description ~= nil and description["shortDescr"] ~= nil then

      --[[
        for i =1, #description["tags"] do
          print("T = " .. description["tags"][i])
        end
        ]]--


        local tagDetected = false
        for k = 1, #tags do
          for j =1, #description["tags"] do
            if string.find(description["tags"][j], tags[k], 1, true) ~= nil then
              -- print("DETECTED  DESCR=" .. description["tags"][j] .. "   TAG=" .. tags[k] .. "|")
              tagDetected = true
              break
            end
          end
        end
        if tagDetected == true then
          print(string.format("%-25.25s %s", configFiles[i], description["shortDescr"]))
        end
      end
    end

end



-- ************************************************************************
---
--  addTestFiles
--        @description  adding test files from localFS to RAMFS.
--                      for a given file name, it adds the test (seached in tests directory),
--                      the configuration file, the de-configuration file and its packet.
--
--        @param filename   - name of the test
--

function addTestFiles(params)
    local testName = params["filename"]
    local fileTbl = {}
    local k, v
    if wraplLoadFileFromLocalFS == nil then
        print("can't add test")
        return false
    end
    fileTbl["testFile"] = "px/examples/tests/" .. testName .. ".lua"
    fileTbl["confFile"] = "px/examples/configurations/" .. testName .. ".txt"
    fileTbl["deconfFile"] = "px/examples/configurations/" .. testName .. "_deconfig.txt"
    fileTbl["pckFile"] = "px/examples/packets/" .. testName .. ".lua"

    -- support common
    fileTbl["common"] = "px/examples/common/" .. testName .. ".lua"

    for k, v in pairs(fileTbl) do

        if wraplLoadFileFromLocalFS(v) then
            print("added file: " .. v)
        elseif (k ~= "common") then
            print("can't add ".. v .. ": file not exists")
        end
    end
end

-- ************************************************************************
---
--  showTags
--        @description  print all tags detected in a configuration files in the RAMFS
--
--        @param none
--
local function showTags(params)
    local configFiles = {}
    local tags = {}
    local tagLine
    local i, j, k
    -- load list of configuration files
    configFiles = getConfigNames()
    if configFiles == nil then
        print("Error: no configuration files found")
        return false
    end

    -- iterate thru all files and add all detected tags to table
    for i = 1, #configFiles do
      local description

      -- load TAGs list
      description = getConfigDescription(configFiles[i])
      if description ~= nil and description["tags"] ~= nil then
        -- if tag not in tags{} -> add to table
        for j =1, #description["tags"] do
          local tagDetected = false
          local key, val
          for key, val in pairs(tags) do
            if val == description["tags"][j] then
              tagDetected = true
              break
            end
          end
          if tagDetected == false then table.insert(tags, description["tags"][j]) end
        end

      end -- if
    end -- for

    table.sort(tags)

    -- find max width of tag
    local columnsWidth = 0
    for k = 1, #tags do
      j = #tags[k]
      if columnsWidth<j then columnsWidth = j end
    end

    -- exit if nothing to output
    if columnsWidth == 0 or #tags<1 then return end
    columnsWidth = columnsWidth+1

    -- calculate columns number
    local columnsNum = 80 / columnsWidth

    -- tags counter
    k = 1
    repeat
      -- output string
      local outstr = ""
      -- columns counter
      j = 1
      repeat
        -- add tags[k] to string
        outstr = string.format("%s%-"..tostring(columnsWidth).."s", outstr, tags[k])
        k = k + 1
        j = j + 1
      until k>#tags or j>columnsNum
      print(outstr)
    until k>#tags
end

--------------------------------------------------------------------------------
---
---  CLI types section
--
--
CLI_type_dict["test"] = {
    checker = checkParamTestName,
    complete = completeValueTestName
}

CLI_type_dict["configName"] = {
    complete = completeValueConfigName,
    help = ""
}

-- tagList format declaration
CLI_type_dict["tagList"] = {
    help = ""
}

--------------------------------------------------------------------------------
---
---  CLI commands section
--
--

-- run one or more tests separated by spaces
CLI_addCommand("examples", "run", {
    func=run,
    help="Execute configuration file and run test(s)",
    params={
        { type="values",
          {format="@test", name="test", help="executed test name"}
        }
    }
})

-- run all tests
CLI_addCommand("examples", "run all", {
    func=function(params)
      params.allFlag = true
      return run(params)
    end,
    help="Run all tests"
})

-- run specified test 3 times inside all tests: before all tests, in middle of all tests and at the end of all tests
CLI_addCommand("examples", "run stress", {
    func=function(params)
      params.stressFlag = true
      return run(params)
    end,
    help="Execute test in stress mode",
    params={
        { type="values",
          {format="%test", name="test", help="executed test name"},
        }
    }
})


-- to load (execute a configuration) from a file stored in the RAMFS
-- (configurations directory)
--
CLI_addCommand("examples", "load", {
    func=executeConfigFile,
    help="Load configuration from file",
    params={
        { type="named",
            {format="%configName", name="name" }
        }
    }
})


CLI_addHelp("examples", "show", "Show command")

-- display the contents of the file stored in the RAMFS,
-- in "px/examples/configurations/" directory
--
CLI_addCommand("examples", "show file", {
    func=showConfigFile,
    help="Show the contents of a configuration file in the RAMFS",
    params={
        { type="named",
            {format="%configName", name="name" }
        }
    }
})

-- print dynamic tag content detected in a configuration files
-- in "px/examples/configurations/" directory
--
CLI_addCommand("examples", "show search-keywords", {
    func=showTags,
    help="Show the tags(keywords) used in configuration files",
    params={
    }
})

-- search for a configuration file or a number of configuration files
-- in the RAMFS according to a string entered.
-- for each file, a small description will be displayed.
--
CLI_addCommand("examples", "search", {
    func=searchByTag,
    help="Search configuration files in the RAMFS",
    params={
        { type="named",
            {format="%tagList", name="tagString" }
        }
    }
})

CLI_addCommand("examples", "add-test-files", {
    func = addTestFiles,
    help = "add test with its conf file, deconf file, and its packet",
    params = {
        { type = "values", "%filename" }
    }
})

-- set mode for tests output log
--
CLI_addCommand("examples", "log-mode-select", {
    func = setLogOutputMode,
    help = "Set mode for tests output",
    params={
        { type="named",
            {format="silent",  help = "set silent mode for output"},
            {format="verbose", help = "set verbose mode for output"},
            alt = { mode = {"silent", "verbose"}},
            mandatory={"mode"}
        }
    }
})

CLI_addCommand("examples", "match-test-name", {
    func = listTests,
    help = "list all the tests that 'match' the name of parameter",
    params = {
        { type="values",
          {format="%test", name="testname", help="the name of the test to look for ,can use * (wild-card)"},
        }
    }
})


-- {port = {0, 18, 36, 58, 1, 2}, portCPU = 63}
-- manage global varables
-- preset_test_ports = nil;
-- preset_test_cpu_port = nil;
-- preset_test_device = nil;
-- preset_test_led_if = nil;
-- recall fillDeviceEnvironment on rebuild command
function setDeviceEnvironmentFromPrestValues()
    devEnv.dev     = 0;
    devEnv.portCPU = 16
    devEnv.ledIf   = 0
    devEnv.port    = {0, 4, 5, 12, 1, 2 }

    if preset_test_device then
        devEnv.dev     = preset_test_device;
    end
    if preset_test_cpu_port then
        devEnv.portCPU = preset_test_cpu_port;
    end
    if preset_test_ports then
        devEnv.port    = deepcopy(preset_test_ports);
    end
    if preset_test_led_if then
        devEnv.ledIf   = preset_test_led_if;
    end

    print(
        "devices, ports to be used by LUA tests , and configuration files ==> "
        .. to_string(devEnv))
    return
end

local function tests_cfg_set(params)
    -- print("params " .. to_string(params));
    if params.cmd == "ports" then
        for dev,ports in pairs(params.dev_ports) do
            if type(ports) == "table" and next(ports) then
                preset_test_ports = deepcopy(ports);
            end
            if params.cpu_port then
                preset_test_cpu_port = params.cpu_port;
            end
        end
        setDeviceEnvironmentFromPrestValues();
    elseif params.cmd == "device" then
        preset_test_device = params["tests_dev"];
        setDeviceEnvironmentFromPrestValues();
    elseif params.cmd == "led_if" then
        preset_test_led_if = params["led_if"];
        setDeviceEnvironmentFromPrestValues();
    elseif params.cmd == "reset" then
        preset_test_device = nil;
        preset_test_ports = nil;
        preset_test_cpu_port = nil;
        preset_test_led_if = nil;
        setDeviceEnvironmentFromPrestValues();
    elseif params.cmd == "print" then
        print("devEnv " .. to_string(devEnv));
    else
        print("wrong command");
    end
end

CLI_addHelp(
    "examples", "tests-cfg-set",
    "Set configuration to be used in tests after initSystem")

CLI_addCommand("examples", "tests-cfg-set ports", {
    help = "Set tests device ports to be used after initSystem",
    func = function(params)
        params.cmd = "ports";
        tests_cfg_set(params);
    end,
    params={
        { -- parameters node
            type = "values",
            mandatory={"dev_ports"},
            { format="%port-range", name="dev_ports",
                help="device ports used in tests"},
        },
        {
            type = "named",
            { format="cpu-port %GT_U16", name="cpu_port",
                help="device CPU port"},
                requirements = {cpu_port = {"dev_ports"},
            },
        },
    },
})

CLI_addCommand("examples", "tests-cfg-set device", {
    help = "Set tests device to be used after initSystem",
    func = function(params)
        params.cmd = "device";
        tests_cfg_set(params);
    end,
    params={
        { -- parameters node
            type = "values",
            mandatory={"tests_dev"},
            { format="%devID",name="tests_dev",
                help="device number for one-device tests",
            },
        },
    }
})

CLI_addCommand("examples", "tests-cfg-set led-if", {
    help = "Set tests led-if to be used after initSystem",
    func = function(params)
        params.cmd = "led_if";
        tests_cfg_set(params);
    end,
    params={
        { -- parameters node
            type = "values",
            mandatory={"led_if"},
            { format="%GT_U16",name="led_if",
                help="led-if number for one-device tests",
            },
        },
    }
})

CLI_addCommand("examples", "tests-cfg-set print", {
    help = "Print preseted configuration to be used after initSystem or after tests-cfg-set rebuild",
    func = function(params)
        params.cmd = "print";
        tests_cfg_set(params);
    end,
})

CLI_addCommand("examples", "tests-cfg-set reset", {
    help = "Erase preseted configuration to be used after initSystem or after tests-cfg-set rebuild",
    func = function(params)
        params.cmd = "reset";
        tests_cfg_set(params);
    end,
})

local errors_array = {}
local pass_array = {}
local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"
-- add 'error string' to errors_array .. to set test as fail on this 'section'
-- print results in testPrintResultSummary()
function testAddErrorString(errorString)
    errors_array[#errors_array+1] = errorString
    setFailState()
end
-- add 'passed string' to pass_array .. to set test as passed on this 'section'
-- print results in testPrintResultSummary()
function testAddPassString(passString)
    pass_array[#pass_array+1] = passString
end

-- check expected result compared to actual one
function testCheckExpectedValue(name , expected_value , actualValue)
    local compare_special = {string_case_insensitive = true}

    local descrption = "Comparing expected value(s) of " .. to_string(name)

    local diffValues = table_differences_get(expected_value, actualValue , nil, compare_special)

    if not diffValues then
        local pass_string = descrption .. " PASSED."
        printLog (pass_string)
            -- makes LOG too big when all is OK -->
            -- "With value: " .. to_string(expected_value))
        testAddPassString(pass_string)
    else
        -- not the expected value
        local error_string = "ERROR : " .. descrption .. " FAILED."
        printLog (error_string .. " due to: " , to_string(diffValues))
        testAddErrorString(error_string)
        setFailState()
    end

    return
end
-- function to check expected values compared to actual values and print and register the error for the test
-- note : this function not care about the format of 'arrayToCheck'
--
-- callBackFunction(command_data, entry) --> callback function that will
--          return with : isError , actualValue , expectedValue , description
--
-- arrayToCheck[] - array of 'entries' to check
--
function testCheckExpectedValues(command_data,callBackFunction,arrayToCheck)
    for index,entry in pairs(arrayToCheck) do
        local isError , actualValue , expectedValue , description = callBackFunction(command_data, entry)

        if isError then
            local error_string = "ERROR : FAILED to get " .. description
            -- use command_data:addErrorAndPrint to be able to see also errors that came from
            -- command_data that got it inside pip_get_info_drop_port_counter
            command_data:addErrorAndPrint(error_string) -- print and reset the errors string array

            --printLog ("ERROR : " .. error_string)
            testAddErrorString(error_string)
            setFailState()
        else
            testCheckExpectedValue(description,expectedValue,actualValue)
        end
    end
end
-- print results summary (and clear arrays of pass_array,errors_array)
function testPrintResultSummary(testName)
    local numPass = #pass_array

    printLog (delimiter)

    if numPass ~= 0 then
        printLog("-------- "..testName.." PASS summary : " .. numPass .. " tests: ")

        for ii = 1,numPass do
            local myString = "["..ii.."] " .. to_string(pass_array[ii])
            printLog(myString)
        end
    else
        printLog("-------- ALL "..testName.." TESTS FAILED .")
    end

    local numFail = #errors_array
    if numFail ~= 0 then
        printLog (delimiter)

        printLog("-------- "..testName.." FAIL summary : " .. numFail .. " tests: ")
        for ii = 1,numFail do
            local myString = "["..ii.."] " .. to_string(errors_array[ii])
            printLog(myString)
        end
    end
    pass_array = {}
    errors_array = {}

    if numPass == 0 and numFail == 0 then
        setFailState()

        printLog (delimiter)

        printLog("-------- ERROR no "..testName.." test passed or failed")
    end

    printLog (delimiter)
end




-- -- ************************************************************************
--
-- function setTestCoverageLevel(param)
-- local mode
--  mode = param["mode"]
--  if mode == nil then
--      print("Error: undefined mode")
--      return false
--  end
--
--     CoverageLvl    = coverage_level_tbl[mode]
--     CoverageLvlTxt = mode
--
--     printLog("CoverageLvl    = "..to_string(CoverageLvl))
--     printLog("CoverageLvlTxt = " .. CoverageLvlTxt)
--
--  return true
-- end
--
-- -- ************************************************************************
--
-- CLI_addCommand("examples", "coverage-test-level", {
--     func = setTestCoverageLevel,
--     help = "Set coverage level mode for tests",
--     params = {
--         {
--             type = "named",
--             { format = "min"    }, -- 1 - Min   coverage mode for tests
--             { format = "middle" }, -- 2 - Midle coverage mode for tests
--             { format = "max"    }, -- 3 - Max   coverage mode for tests
--
--             alt = { mode = { "min", "middle", "max" } },
--             mandatory = { "mode" }
--         }
--     }
-- })
--

-- ************************************************************************

function setTestCoverageLevel(param)
local mode
    mode = param["mode"]
    if mode == nil then
        print("Error: undefined mode")
        return false
    end

    CoverageLvl = tonumber(mode)

    return true
end

-- ************************************************************************

CLI_addCommand("examples", "coverage-test-level", {
    func = setTestCoverageLevel,
    help = "Set coverage level mode for tests",
    params = {
        {
            type = "named",
            { format = "1", help = "set 1 - Min    coverage level for tests" },
            { format = "2", help = "set 2 - Middle coverage level for tests" },
            { format = "3", help = "set 3 - Max    coverage level for tests" },

            alt = { mode = { "1", "2", "3" } },
            mandatory = { "mode" }
        }
    }
})


-- ************************************************************************

function setLogVerboseLevel(param)
    local mode
    mode = param["mode"]
    if mode == nil then
        print("Error: undefined mode")
        return false
    end

    VerboseLvlTxt = mode
    VerboseLevel = message_level_tbl[mode]

    printLog("VerboseLevel  = "..to_string(VerboseLevel))
    printLog("VerboseLvlTxt = " .. VerboseLvlTxt)

     return true
end

-- ************************************************************************

CLI_addCommand("examples", "msg-level-select", {
    func = setLogVerboseLevel,
    help = "Set message level verbose mode for output",
    params = {
        {
            type = "named",
            { format = "silent"      , help = "set level 0 for log messages" },
            { format = "emergency"   , help = "set level 1 for log messages" },
            { format = "alerts"      , help = "set level 2 for log messages" },
            { format = "critical"    , help = "set level 3 for log messages" },
            { format = "errors"      , help = "set level 4 for log messages" },
            { format = "warnings"    , help = "set level 5 for log messages" },
            { format = "notification", help = "set level 6 for log messages" },
            { format = "information" , help = "set level 7 for log messages" },
            { format = "debug"       , help = "set level 8 for log messages" },
            { format = "verbose"     , help = "set level 9 for log messages" },

            alt={mode={"silent","emergency","alerts","critical","errors","warnings","notification","information","debug","verbose"}},
            --alt = { mode = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" } },
            mandatory = { "mode" }
        }
    }
})

-- ************************************************************************
---
--  setSkipLongTests
--        @description  set skipping mode for long tests
--
--        @param - mode (enable or disable)
--
function setSkipLongTests(param)
    if param["mode"] == nil then
      print("Error: undefined mode")
      return false
    end

    if param["mode"] == "enable" then
      skipLongTests = true
    elseif param["mode"] == "disable" then
      skipLongTests = false
    end

    return true
end


-- ************************************************************************
-- skipping long tests
--
CLI_addCommand("examples", "skip-long-tests", {
    func = setSkipLongTests,
    help = "Set skipping mode for long tests",
    params={
        { type="named",
            {format="enable",  help = "set skip mode for skip long test"},
            {format="disable", help = "set disable mode to run long test"},
            alt = { mode = {"enable", "disable"}},
            mandatory={"mode"}
        }
    }
})

-- ************************************************************************
---
--  set_mode_for_run_last_tests
--        @description  set_mode_for_run_last_tests
--
--        @param - mode (enable or disable)
--
function set_mode_for_run_last_tests(param)
    if param["mode"] == nil then
      print("Error: undefined mode")
      return false
    end

    if param["mode"] == "enable" then
      run_LastTests = true
    elseif param["mode"] == "disable" then
      run_LastTests = false
    end

    return true
end


-- ************************************************************************
-- set enable/disable mode for run last tests
--
CLI_addCommand("examples", "last-tests-run", {
    func = set_mode_for_run_last_tests,
    help = "Whether to run last tests list when running <<run all>>",
    params={
        { type="named",
            {format="enable",  help = "include last tests in <<run all>>"},
            {format="disable", help = "exclude last tests from <<run all>>"},
            alt = { mode = {"enable", "disable"}},
            mandatory={"mode"}
        }
    }
})

local function device_range_filter(params)
    local min_dev_id = params["min_dev_id"];
    local max_dev_id = params["max_dev_id"];
    local enable = tonumber(params["enable"]);
    wrlDevFilterRangeSet(min_dev_id, max_dev_id, enable);
end

CLI_addCommand("config", "device-range-filter", {
    help = "Adds/removes range of device id-s to/from device filter",
    func = device_range_filter,
    params={
        { -- parameters node
            type = "values",
            { format="%GT_U8",name="min_dev_id",
                help="minimal device number in the range",
            },
            { format="%GT_U8",name="max_dev_id",
                help="maximal device number in the range",
            },
            { format="%bool",name="enable",
                help="enable or disable",
            },
        },
        mandatory={"enable"},
        requirements = {
            enable  = {"min_dev_id", "max_dev_id"},
            max_dev_id = {"min_dev_id"}
        },
    }
})

