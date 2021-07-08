--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* i2c_byte_write.lua
--*
--* DESCRIPTION:
--*       Write a byte on i2c bus
--*
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssDxChHwIfTwsiWriteByte")

-- ************************************************************************
---
--  i2c_byte_write_func
--        @description  write a byte on i2c bus
--
--        @param params         - inserted values                   
--
--        @return               - result
--
local function i2c_byte_write_func(params)
    local GT_OK     = 0
    local devSlvId  = params.devSlvId
    local regAddr   = params.regAddr
    local value     = params.value
    
    local values    = wrLogWrapper("wrlCpssDxChHwIfTwsiWriteByte", "(devSlvId, regAddr, value)", devSlvId, regAddr, value)
    local rc        = values["status"]

    if (rc ~= GT_OK) then
        local index = string.find(returnCodes[rc],"=")
        print("Error at i2c byte write:", string.sub(returnCodes[rc],index + 1))
        do return end
    end
end


--------------------------------------------------------------------------------
-- command registration: i2c byte write
--------------------------------------------------------------------------------
CLI_addCommand("debug", "i2c byte write", {
    func = i2c_byte_write_func,
    help = "Write a byte from i2c bus",
    params = {
        {	type= "named", 	{ format="devSlvId %GT_U32"             ,name="devSlvId"       , help="Slave Device ID"                         },
                            { format="regAddr %GT_U32"              ,name="regAddr"        , help="Register address to read from"           }, 
                            { format="value %GT_U32"                ,name="value"          , help="Data to be written to register"          }, 
                            requirements = {
                                ["regAddr"]       = {"devSlvId"},
                                ["value"]         = {"regAddr"}
                                },
							mandatory={"devSlvId", "regAddr", "value"}
		}
    }
})
