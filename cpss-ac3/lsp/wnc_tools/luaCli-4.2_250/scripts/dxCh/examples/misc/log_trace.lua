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
--*      trace control procedures
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

-- default value of log output mode
--logVerboseMode = true

--                   { "0",        "1",       "2",     "3",     "4",       "5",        "6",          "7",       "8",      "9" } 
TestLogMsgLevelList={"silent","emergency","alerts","critical","errors","warnings","notification","information","debug","verbose"};

-- ************************************************************************
---
-- printMsgLvlLog
-- @description print log wrapper for using in tests
--
-- @param string - data to print
--
function printMsgLvlLog_old(level, string)
    -- skip printing in silent mode
    --if logVerboseMode then
        if VerboseLevel >= level then
            printLog(string)
        end
    --end
end

-- ************************************************************************
function printMsgLvlLog(level_txt, string)

    local level = message_level_tbl[level_txt]
    if level ~= nil then
      if VerboseLevel >= level then
          printLog(string)
      end
    end --if level ~= nil then
end

-- ************************************************************************
--                    { "0",        "1",       "2",     "3",     "4",       "5",        "6",          "7",       "8",      "9" } 
--TestLogMsgLevelList={"silent","emergency","alerts","critical","errors","warnings","notification","information","debug","verbose"};

function error_handle(msg, ifPass, testStillOk)

    if (ifPass == false) then
        printMsgLvlLog("errors", string.format("%s: FAIL Total Test:FAIL\n", msg))
        return false;
    else
        if (testStillOk == false) then
            printMsgLvlLog("errors", string.format("%s: PASS Total Test:FAIL\n", msg))
            return false;
        else
            printMsgLvlLog("errors", string.format("%s: PASS Total Test:PASS\n", msg))
            return true;
        end
    end
    return testStillOk;
end


function warning_handle(msg, ifPass, testStillOk)

    if (ifPass == false) then
        printMsgLvlLog("errors", string.format("%s: FAIL Total Test:FAIL\n", msg))
        return false;
    else
        if (testStillOk == false) then
            printMsgLvlLog("errors", string.format("%s: PASS Total Test:FAIL\n", msg))
            return false;
        else
            printMsgLvlLog("information", string.format("%s: PASS Total Test:PASS\n", msg))
            return true;
        end
    end
    return testStillOk;
end
