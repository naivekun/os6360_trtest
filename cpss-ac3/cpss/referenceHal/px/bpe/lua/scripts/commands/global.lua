--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* global.lua
--*
--* DESCRIPTION:
--*       global BPE commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

-- ************************************************************************
---
--  init_func
--        @description  Initialize the BPE package
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function init_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeInit", {
            { "IN", "GT_SW_DEV_NUM",    "devNum",              params.device_id }
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeInit"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

-- ************************************************************************
---
--  reset_func
--        @description  Clean all BPE configurations
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function reset_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeReset", {
            { "IN", "GT_SW_DEV_NUM",    "devNum",              params.device_id }
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeReset"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

--------------------------------------------
-- command registration: 
--------------------------------------------

-- the command looks like :
-- Console(bpe)# init device 0
CLI_addCommand("bpe", "init", {
    func   = init_func,
    help   = "Initialize the BPE package",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" }                                                       
        }
    }
}) 

-- the command looks like :
-- Console(bpe)# reset device 0
CLI_addCommand("bpe", "reset", {
    func   = reset_func,
    help   = "Clean all BPE configurations",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" }                                                       
        }
    }
}) 
