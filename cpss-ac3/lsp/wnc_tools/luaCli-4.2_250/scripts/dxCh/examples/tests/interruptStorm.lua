--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* interruptStorm.lua
--*
--* DESCRIPTION:
--*       This test check for for storm interrupts.
--*       The flow :
--*         1.Get event counters
--*         2.Wait
--*         3.Get event counters again
--*         4.Compare values from step 1 to values from step 3,if there is a big difference - then fail the test.
--*         
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants

--registering c functions



local function eventTableCompare(eventType,devID,delayTimeInSec,tollarance)
    local currCounter2;
    local currEventType;
    
    print("Get interrupt counters")
    local countersTable1 = wrlCpssGenEventTableGet(devID, eventType)

 
    print("Delay for "..delayTimeInSec)
    delay(1000 * delayTimeInSec)
    
    print("Get interrupt counters again")
    local countersTable2 = wrlCpssGenEventTableGet(devID, eventType)
 
    for index,currCounter in pairs(countersTable1) do
         currCounter2 = countersTable2[index];
         currEventType = cpssGenWrapperCheckParam("CPSS_UNI_EV_CAUSE_ENT", tonumber(index))
         if(currCounter2>currCounter)then
             print(tostring(currEventType).." is different by "..currCounter2- currCounter);
             if(currCounter2- currCounter > tollarance)then
              print("Interrupt storm detected for "..tostring(currEventType) )
              setFailState()
             end  
         end
    end
    

    return true
    
end

local CPSS_PP_EVENTS = 0

eventTableCompare(--[[eventType]]CPSS_PP_EVENTS,--[[devID]]0,--[[delayTimeInSec]]5,--[[tollarance]]100);


