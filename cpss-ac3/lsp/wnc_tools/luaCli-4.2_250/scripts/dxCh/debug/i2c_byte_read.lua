--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* i2c_byte_read.lua
--*
--* DESCRIPTION:
--*       Read a byte from i2c bus
--*
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssDxChHwIfTwsiReadByte")

-- ************************************************************************
---
--  i2c_byte_read_func
--        @description  read a byte from i2c bus
--
--        @param params         - inserted values                   
--
--        @return               - result
--
local function i2c_byte_read_func(params)
    local GT_OK     = 0
    local devSlvId  = params.devSlvId
    local regAddr   = params.regAddr
    
    local values    = wrLogWrapper("wrlCpssDxChHwIfTwsiReadByte", "(devSlvId, regAddr)", devSlvId, regAddr)
    local rc        = values["status"]
    
    if (rc ~= GT_OK) then
        local index = string.find(returnCodes[rc],"=")
        print("Error at i2c byte read:", string.sub(returnCodes[rc],index + 1))
        do return end
    end
    local data      = values["data"]
    print("Data:", data)
end


--------------------------------------------------------------------------------
-- command registration: i2c byte read
--------------------------------------------------------------------------------
CLI_addCommand("debug", "i2c byte read", {
    func = i2c_byte_read_func,
    help = "Read a byte from i2c bus",
    params = {
        {	type= "named", 	{ format="devSlvId %GT_U32"             ,name="devSlvId"       , help="Slave Device ID"                         },
                            { format="regAddr %GT_U32"              ,name="regAddr"        , help="Register address to read from"           },  
                            requirements = {
                                ["regAddr"]       = {"devSlvId"}
                                },
							mandatory={"devSlvId", "regAddr"}
		}
    }
})
