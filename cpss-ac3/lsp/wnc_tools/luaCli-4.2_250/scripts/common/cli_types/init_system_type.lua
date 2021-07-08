--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* init_system_type.lua
--*
--* DESCRIPTION:
--*       init_system_type type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes


--constants

--
-- Types defined:
--
-- channel-group   - The range of ports
--
-- ************************************************************************
---
--  check_param_port_range
--        @description  Check parameter that it is in the correct form
--
--        @param param          - Parameter string
--
--        @return       boolean check status
--        @return       number:paramList or string:error_string if failed
--        @return       The format of paramList
--                          paramList[devID][1]...paramList[devID][n]
--
function check_init_system(param)
    if string.sub(param,string.len(param),string.len(param))=="," then
        return false,err
    end	
    param=param..","
    local ret={}
    s,e,ret[1],ret[2],ret[3] = string.find(param,"(%d+),(%d+),(%d*)") 
    err = "Wrong patterh, required format is boardIdx,boardRevId,reloadEeprom, for example 19,1,0"
    if nil==ret[1] then
        return false,err
    end
    
    if string.len(param)>e+1 then
        return false,err
    end	
    if ""==ret[3] then
        ret[3]="0"
    end
    
    ret[1]=tonumber(ret[1])
    ret[2]=tonumber(ret[2])
    ret[3]=tonumber(ret[3])
    
    return true,ret
end	
    
CLI_type_dict["init_system"] = {	--(number,number,number)
    checker = check_init_system,
    help = "The init system configurations. required format is boardIdx,boardRevId,reloadEeprom"
}

