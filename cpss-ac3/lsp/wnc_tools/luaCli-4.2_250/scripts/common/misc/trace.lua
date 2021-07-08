--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* trace.lua
--*
--* DESCRIPTION:
--*      trace control procedures for logger output and <trace cpss-api> support
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************


-- save global print handler
local printSave = print

-- wrapper for global <print>
local function printOriginal( ... )

  printSave(...)
end


-- ************************************************************************
---
--  writeLog
--        @description  write log to temporary file in RAMFS
--
--
local function writeLog(...)
  local i
  local s = ""
  
  if traceCpssAPIfd ~= nil then
    fs.write(traceCpssAPIfd, trace.getLLine())
--    fs.write(traceCpssAPIfd, myCPSS_API_log_prefix)
    
    local len = select("#", ...)
    for i=1, len do
      s= select(i, ...)
      fs.write(traceCpssAPIfd, string.gsub(tostring(s), "\n", "\n     ".. trace.getLLine()))
    end

--    fs.write(traceCpssAPIfd, myCPSS_API_log_suffix .. "\n")
    fs.write(traceCpssAPIfd, "\n")
  end
  
  return false, nil
end


--Log levels
LOG_ERROR     = 0
LOG_WARNING   = 1
LOG_INFO      = 2
LOG_EXAMPLES  = 3
LOG_TRACE     = 4
LOG_DEBUG     = 5


-- ************************************************************************
---
--  trace object implementation
--
--  Note: cannot be initialized outside the module
--  
local traceObject = {}

function traceObject.new() 
  local self = {}

  local inProgress = false     -- trace cpss-api status
  local onScreen = true        -- output of <trace cpss-api> on screen (false =  in file only)
  
  local leftMargine = 1        -- loop level and margin multiplier
  local lLine = "  "           -- line of left margin
    
  local repCounter = 5         -- repetition counter for <trace cpss-api>
  local logVerboseMode = false -- trace mode for examples

  local logLevel = LOG_INFO    -- by default print ERR, WRN and INFO messages

  local skipTraceLine = false  -- for debug purposes

  -- for debug purposes
--[[
  function self.setSkipTraceStatus(b)
    skipTraceLine = b
   end

  function self.getSkipTraceStatus()
    return skipTraceLine
   end
]]--

--  local mode = "verbose"
--  local fileName = ""
--  local fileDescriptor = nil

  -- reset trace parameters to default state
  function self.reset()
    inProgress = false
    leftMargine = 1
    lLine = "  "
    onScreen = true
    repCounter = 5
    logVerboseMode = false -- default value for examples log output mode is silent
    logLevel = LOG_INFO
  end

  -- activate trace status
  function self.start()
    logLevel = LOG_TRACE
    inProgress = true
  end

  -- stop trace, restore previous level and flush buffers
  function self.stop()
    if logVerboseMode then
      logLevel = LOG_EXAMPLES
    else
      logLevel = LOG_TRACE
    end  
    inProgress = false
  end

  -- get trace status
  function self.getStatus()
    return inProgress
  end

  -- get loop level
  function self.getMargin()
    return leftMargine
  end

  -- get lLine 
  function self.getLLine()
    return lLine
  end


  -- get log level
  function self.getLevel()
    return logLevel
  end

  -- set log level
  function self.setLevel(l)
    logLevel = l
  end

  -- set trace repetition counter
  function self.setCounter(num)
    if num>0 then 
      repCounter = num
    end
  end

  -- get trace repetition counter
  function self.getCounter()
    return repCounter
  end

  -- get logVerboseMode
  function self.getLogVerboseMode()
    return logVerboseMode
  end

  -- set logVerboseMode
  function self.setLogVerboseMode(state)
    if state then 
      logVerboseMode = true 
      logLevel = LOG_EXAMPLES
    else 
      logVerboseMode = false 
      logLevel = LOG_INFO
    end
  end

  -- get onScreen status
  function self.getOnScreen()
    return onScreen
  end

  -- set onScreen status
  function self.setOnScreen(onScreenStatus)
    if onScreenStatus then 
      onScreen = true 
    else 
      onScreen = false 
    end
  end

  -- trace API in progress
  function self.startAPI(...)
-- for debug purposes ( A.K. REVERT WHEN DONE LOGGER)
--    skipTraceLine = false
    
    log.trace("=============================================================================================")
    log.trace(...)
    
    local debuginfo = debug.getinfo(2)
    log.trace(debuginfo.source)
    log.trace(debuginfo.currentline)
    
    log.trace("{")
    leftMargine = leftMargine + 1
    lLine = string.rep('  ', leftMargine)
  end

  -- leave API and reset counters
  function self.leaveAPI(...)
    log.trace("return")
    if leftMargine > 1 then 
      leftMargine = leftMargine - 1
    end
    lLine = string.rep('  ', leftMargine)
    log.trace("}")
    log.trace(...)
    local debuginfo = debug.getinfo(2)
    log.trace(debuginfo.source)
    log.trace(debuginfo.currentline)
    log.trace("=============================================================================================")
-- for debug purposes ( A.K. REVERT WHEN DONE LOGGER)
--    skipTraceLine = true
  end

  -- start loop and increase log level
  function self.startLoop(...)
    log.trace("=================================================================")
    local debuginfo = debug.getinfo(2)
    log.trace(debuginfo.source)
    log.trace(debuginfo.currentline)
    log.trace(...)
    log.trace("{")
    leftMargine = leftMargine + 1
    lLine = string.rep('  ', leftMargine)
  end
  
  -- stop loop and decrease log level
  function self.stopLoop(...)
    if leftMargine > 1 then 
      leftMargine = leftMargine - 1
    end
    lLine = string.rep('  ', leftMargine)
    log.trace("}")
    log.trace(...)
    local debuginfo = debug.getinfo(2)
    log.trace(debuginfo.source)
    log.trace(debuginfo.currentline)
    log.trace("=================================================================")
  end

  return self
end


-- ************************************************************************
---
--  log implementation
--
--  Note: cannot be initialized outside the module
--  
local logObject = {}

function logObject.new()

local  logPrefix = {
        [LOG_ERROR]     = "ERROR:   ",
        [LOG_WARNING]   = "WARNING: ",
        [LOG_INFO]      = " ",
        [LOG_EXAMPLES]  = " ",
        [LOG_TRACE]     = " ",
        [LOG_DEBUG]     = "DEBUG:   "
}


  local self = {}

  -- ERROR
  function self.err(...)
    if trace.getLevel() < LOG_ERROR then
      return
    end

    printOriginal(trace.getLLine() .. logPrefix[LOG_ERROR], ...)
    writeLog(logPrefix[LOG_ERROR], ...)
  end

  -- WARNING
  function self.warn(...)
    if trace.getLevel() < LOG_WARNING then
      return
    end

    printOriginal(trace.getLLine() .. logPrefix[LOG_WARNING], ...)
    writeLog(logPrefix[LOG_WARNING], ...)
  end

  -- REGULAR OUTPUT
  function self.info(...)
    if trace.getLevel() < LOG_INFO then
      return
    end

    printOriginal(trace.getLLine(), ...)
    writeLog(...)
  end

  -- DETAILED OUTPUT FOR VERBOSE MODE
  function self.examples(...)
    if trace.getLevel() < LOG_EXAMPLES then
      return
    end

    printOriginal(trace.getLLine(), ...)
    writeLog(...)
  end


  -- TRACE CPSS-API OUTPUT
  function self.trace(...)
  
    if trace.getLevel() < LOG_TRACE then
      return
    end

    if trace.getOnScreen() then
      printOriginal(trace.getLLine(), ...)
    end
    writeLog(...)

  end

  -- DBG MESSANGER FOR DEVELOPERS
  function self.dbg(...)
    if trace.getLevel() < LOG_DEBUG then
      return
    end
    printOriginal(logPrefix[LOG_DEBUG], ... )
  end
  
  return self
end

-- ************************************************************************
-- ************************************************************************
---
-- Initialization 
--  

--- create trace and log instances 
if trace == nil then
  trace = traceObject.new()
  log = logObject.new()
 end

trace.reset()

-- for debug purposes ( A.K. REVERT WHEN DONE LOGGER)
-- trace.setSkipTraceStatus(true)


-- if log == nil then
-- end
-- log.reset()


-- ************************************************************************
---
--  printLog
--        @description print log wrapper for using in tests
--
--        @param string - data to print
--
function printLog(...)

--  writeLog(...)
  -- skip printing in silent mode
  if trace.getLogVerboseMode() or trace.getStatus() then
    log.info(...)
--    print(...)
  end
end


-- ************************************************************************
---
--  printErr
--        @description  log.err() wrapper
--        Used for errors messages
--
function printErr(...)
  log.err(...)
end


-- ************************************************************************
---
--  printWrn
--        @description  log.warn() wrapper
--        Used for errors messages
--
function printWarn(...)
  log.warn(...)
end


-- ************************************************************************
---
--  printMsg
--        @description  log.info() wrapper
--        Used for regular printing
--
function printMsg(...)

  log.info(...)
  
--[[  print(...)
  writeLog(...)
]]--

end


-- ************************************************************************
---
--  printTrace
--        @description  log.trace() wrapper
--        
--        @note Logger for <trace cpss-api> (myGenWrapper only)
--        
--
function printTrace(...)
  
  log.trace(...)

--[[
  if trace.getOnScreen() then
    printOriginal(...)
  end
  writeLog(...)
  ]]--
end

